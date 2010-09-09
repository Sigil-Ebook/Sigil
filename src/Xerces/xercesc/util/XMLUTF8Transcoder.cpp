/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * $Id: XMLUTF8Transcoder.cpp 932887 2010-04-11 13:04:59Z borisk $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/TranscodingException.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/XMLUTF8Transcoder.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Local static data
//
//  gUTFBytes
//      A list of counts of trailing bytes for each initial byte in the input.
//
//  gUTFByteIndicator
//      For a UTF8 sequence of n bytes, n>=2, the first byte of the
//      sequence must contain n 1's followed by precisely 1 0 with the
//      rest of the byte containing arbitrary bits.  This array stores
//      the required bit pattern for validity checking.
//  gUTFByteIndicatorTest
//      When bitwise and'd with the observed value, if the observed
//      value is correct then a result matching gUTFByteIndicator will
//      be produced.
//
//  gUTFOffsets
//      A list of values to offset each result char type, according to how
//      many source bytes when into making it.
//
//  gFirstByteMark
//      A list of values to mask onto the first byte of an encoded sequence,
//      indexed by the number of bytes used to create the sequence.
// ---------------------------------------------------------------------------
static const XMLByte gUTFBytes[256] =
{
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    ,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    ,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    ,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    ,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    ,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    ,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    ,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    ,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    ,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    ,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    ,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    ,   0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
    ,   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
    ,   2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2
    ,   3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5
};

static const XMLByte gUTFByteIndicator[6] =
{
    0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC
};
static const XMLByte gUTFByteIndicatorTest[6] =
{
    0x80, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE
};

static const XMLUInt32 gUTFOffsets[6] =
{
    0, 0x3080, 0xE2080, 0x3C82080, 0xFA082080, 0x82082080
};

static const XMLByte gFirstByteMark[7] =
{
    0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC
};



// ---------------------------------------------------------------------------
//  XMLUTF8Transcoder: Constructors and Destructor
// ---------------------------------------------------------------------------
XMLUTF8Transcoder::XMLUTF8Transcoder(const  XMLCh* const    encodingName
                                    , const XMLSize_t       blockSize
                                    , MemoryManager* const  manager)
:XMLTranscoder(encodingName, blockSize, manager)
{
}

XMLUTF8Transcoder::~XMLUTF8Transcoder()
{
}


// ---------------------------------------------------------------------------
//  XMLUTF8Transcoder: Implementation of the transcoder API
// ---------------------------------------------------------------------------
XMLSize_t
XMLUTF8Transcoder::transcodeFrom(const  XMLByte* const          srcData
                                , const XMLSize_t               srcCount
                                ,       XMLCh* const            toFill
                                , const XMLSize_t               maxChars
                                ,       XMLSize_t&              bytesEaten
                                ,       unsigned char* const    charSizes)
{
    // Watch for pathological scenario. Shouldn't happen, but...
    if (!srcCount || !maxChars)
        return 0;

    //
    //  Get pointers to our start and end points of the input and output
    //  buffers.
    //
    const XMLByte*  srcPtr = srcData;
    const XMLByte*  srcEnd = srcPtr + srcCount;
    XMLCh*          outPtr = toFill;
    XMLCh*          outEnd = outPtr + maxChars;
    unsigned char*  sizePtr = charSizes;



    //
    //  We now loop until we either run out of input data, or room to store
    //  output chars.
    //
    while ((srcPtr < srcEnd) && (outPtr < outEnd))
    {
        // Special-case ASCII, which is a leading byte value of <= 127
        if (*srcPtr <= 127)
        {
            // Handle ASCII in groups instead of single character at a time.
            const XMLByte* srcPtr_save = srcPtr;
            const XMLSize_t chunkSize = (srcEnd-srcPtr)<(outEnd-outPtr)?(srcEnd-srcPtr):(outEnd-outPtr);
            for(XMLSize_t i=0;i<chunkSize && *srcPtr <= 127;++i)
                *outPtr++ = XMLCh(*srcPtr++);
            memset(sizePtr,1,srcPtr - srcPtr_save);
            sizePtr += srcPtr - srcPtr_save;
            if (srcPtr == srcEnd || outPtr == outEnd)
                break;
        }

        // See how many trailing src bytes this sequence is going to require
        const unsigned int trailingBytes = gUTFBytes[*srcPtr];

        //
        //  If there are not enough source bytes to do this one, then we
        //  are done. Note that we done >= here because we are implicitly
        //  counting the 1 byte we get no matter what.
        //
        //  If we break out here, then there is nothing to undo since we
        //  haven't updated any pointers yet.
        //
        if (srcPtr + trailingBytes >= srcEnd)
            break;

        // Looks ok, so lets build up the value
        // or at least let's try to do so--remembering that
        // we cannot assume the encoding to be valid:

        // first, test first byte
        if((gUTFByteIndicatorTest[trailingBytes] & *srcPtr) != gUTFByteIndicator[trailingBytes]) {
            char pos[2] = {(char)0x31, 0}; 
            char len[2] = {(char)(trailingBytes+0x31), 0};
            char byte[2] = {*srcPtr,0};
            ThrowXMLwithMemMgr3(UTFDataFormatException, XMLExcepts::UTF8_FormatError, pos, byte, len, getMemoryManager());
        }

        /***
         * http://www.unicode.org/reports/tr27/
         *
         * Table 3.1B. lists all of the byte sequences that are legal in UTF-8. 
         * A range of byte values such as A0..BF indicates that any byte from A0 to BF (inclusive) 
         * is legal in that position. 
         * Any byte value outside of the ranges listed is illegal. 
         * For example, 
         * the byte sequence <C0 AF> is illegal  since C0 is not legal in the 1st Byte column. 
         * The byte sequence <E0 9F 80> is illegal since in the row 
         *    where E0 is legal as a first byte, 
         *    9F is not legal as a second byte. 
         * The byte sequence <F4 80 83 92> is legal, since every byte in that sequence matches 
         * a byte range in a row of the table (the last row). 
         *
         *
         * Table 3.1B. Legal UTF-8 Byte Sequences  
         * Code Points              1st Byte    2nd Byte    3rd Byte    4th Byte 
         * =========================================================================
         * U+0000..U+007F            00..7F       
         * -------------------------------------------------------------------------
         * U+0080..U+07FF            C2..DF      80..BF      
         *
         * -------------------------------------------------------------------------
         * U+0800..U+0FFF            E0          A0..BF     80..BF   
         *                                       -- 
         *                          
         * U+1000..U+FFFF            E1..EF      80..BF     80..BF    
         *
         * --------------------------------------------------------------------------
         * U+10000..U+3FFFF          F0          90..BF     80..BF       80..BF 
         *                                       --
         * U+40000..U+FFFFF          F1..F3      80..BF     80..BF       80..BF 
         * U+100000..U+10FFFF        F4          80..8F     80..BF       80..BF 
         *                                           --
         * ==========================================================================
         *
         *  Cases where a trailing byte range is not 80..BF are underlined in the table to 
         *  draw attention to them. These occur only in the second byte of a sequence.
         *
         ***/

        XMLUInt32 tmpVal = 0;

        switch(trailingBytes)
        {
            case 1 :
                // UTF-8:   [110y yyyy] [10xx xxxx]
                // Unicode: [0000 0yyy] [yyxx xxxx]
                //
                // 0xC0, 0xC1 has been filtered out             
                checkTrailingBytes(*(srcPtr+1), 1, 1);

                tmpVal = *srcPtr++;
                tmpVal <<= 6;
                tmpVal += *srcPtr++;

                break;
            case 2 :
                // UTF-8:   [1110 zzzz] [10yy yyyy] [10xx xxxx]
                // Unicode: [zzzz yyyy] [yyxx xxxx]
                //
                if (( *srcPtr == 0xE0) && ( *(srcPtr+1) < 0xA0)) 
                {
                    char byte0[2] = {*srcPtr    ,0};
                    char byte1[2] = {*(srcPtr+1),0};

                    ThrowXMLwithMemMgr2(UTFDataFormatException
                                      , XMLExcepts::UTF8_Invalid_3BytesSeq
                                      , byte0
                                      , byte1
                                      , getMemoryManager());
                }

                checkTrailingBytes(*(srcPtr+1), 2, 1);
                checkTrailingBytes(*(srcPtr+2), 2, 2);

                //
                // D36 (a) UTF-8 is the Unicode Transformation Format that serializes 
                //         a Unicode code point as a sequence of one to four bytes, 
                //         as specified in Table 3.1, UTF-8 Bit Distribution.
                //     (b) An illegal UTF-8 code unit sequence is any byte sequence that 
                //         does not match the patterns listed in Table 3.1B, Legal UTF-8 
                //         Byte Sequences.
                //     (c) An irregular UTF-8 code unit sequence is a six-byte sequence 
                //         where the first three bytes correspond to a high surrogate, 
                //         and the next three bytes correspond to a low surrogate. 
                //         As a consequence of C12, these irregular UTF-8 sequences shall 
                //         not be generated by a conformant process. 
                //
                //irregular three bytes sequence
                // that is zzzzyy matches leading surrogate tag 110110 or 
                //                       trailing surrogate tag 110111
                // *srcPtr=1110 1101 
                // *(srcPtr+1)=1010 yyyy or 
                // *(srcPtr+1)=1011 yyyy
                //
                // 0xED 1110 1101
                // 0xA0 1010 0000

                if ((*srcPtr == 0xED) && (*(srcPtr+1) >= 0xA0))
                {
                    char byte0[2] = {*srcPtr,    0};
                    char byte1[2] = {*(srcPtr+1),0};

                     ThrowXMLwithMemMgr2(UTFDataFormatException
                              , XMLExcepts::UTF8_Irregular_3BytesSeq
                              , byte0
                              , byte1
                              , getMemoryManager());
                }

                tmpVal = *srcPtr++;
                tmpVal <<= 6;
                tmpVal += *srcPtr++;
                tmpVal <<= 6;
                tmpVal += *srcPtr++;

                break;
            case 3 : 
                // UTF-8:   [1111 0uuu] [10uu zzzz] [10yy yyyy] [10xx xxxx]*
                // Unicode: [1101 10ww] [wwzz zzyy] (high surrogate)
                //          [1101 11yy] [yyxx xxxx] (low surrogate)
                //          * uuuuu = wwww + 1
                //
                if (((*srcPtr == 0xF0) && (*(srcPtr+1) < 0x90)) ||
                    ((*srcPtr == 0xF4) && (*(srcPtr+1) > 0x8F))  )
                {
                    char byte0[2] = {*srcPtr    ,0};
                    char byte1[2] = {*(srcPtr+1),0};

                    ThrowXMLwithMemMgr2(UTFDataFormatException
                                      , XMLExcepts::UTF8_Invalid_4BytesSeq
                                      , byte0
                                      , byte1
                                      , getMemoryManager());
                }

                checkTrailingBytes(*(srcPtr+1), 3, 1);
                checkTrailingBytes(*(srcPtr+2), 3, 2);
                checkTrailingBytes(*(srcPtr+3), 3, 3);
                
                tmpVal = *srcPtr++;
                tmpVal <<= 6;
                tmpVal += *srcPtr++;
                tmpVal <<= 6;
                tmpVal += *srcPtr++;
                tmpVal <<= 6;
                tmpVal += *srcPtr++;

                break;
            default: // trailingBytes > 3

                /***
                 * The definition of UTF-8 in Annex D of ISO/IEC 10646-1:2000 also allows 
                 * for the use of five- and six-byte sequences to encode characters that 
                 * are outside the range of the Unicode character set; those five- and 
                 * six-byte sequences are illegal for the use of UTF-8 as a transformation 
                 * of Unicode characters. ISO/IEC 10646 does not allow mapping of unpaired 
                 * surrogates, nor U+FFFE and U+FFFF (but it does allow other noncharacters).
                 ***/
                char len[2]  = {(char)(trailingBytes+0x31), 0};
                char byte[2] = {*srcPtr,0};

                ThrowXMLwithMemMgr2(UTFDataFormatException
                                  , XMLExcepts::UTF8_Exceeds_BytesLimit
                                  , byte
                                  , len
                                  , getMemoryManager());

                break;
        }


        // since trailingBytes comes from an array, this logic is redundant
        //  default :
        //      ThrowXMLwithMemMgr(TranscodingException, XMLExcepts::Trans_BadSrcSeq);
        //}
        tmpVal -= gUTFOffsets[trailingBytes];

        //
        //  If it will fit into a single char, then put it in. Otherwise
        //  encode it as a surrogate pair. If its not valid, use the
        //  replacement char.
        //
        if (!(tmpVal & 0xFFFF0000))
        {
            *sizePtr++ = trailingBytes + 1;
            *outPtr++ = XMLCh(tmpVal);
        }
         else if (tmpVal > 0x10FFFF)
        {
            //
            //  If we've gotten more than 32 chars so far, then just break
            //  out for now and lets process those. When we come back in
            //  here again, we'll get no chars and throw an exception. This
            //  way, the error will have a line and col number closer to
            //  the real problem area.
            //
            if ((outPtr - toFill) > 32)
                break;

            ThrowXMLwithMemMgr(TranscodingException, XMLExcepts::Trans_BadSrcSeq, getMemoryManager());
        }
         else
        {
            //
            //  If we have enough room to store the leading and trailing
            //  chars, then lets do it. Else, pretend this one never
            //  happened, and leave it for the next time. Since we don't
            //  update the bytes read until the bottom of the loop, by
            //  breaking out here its like it never happened.
            //
            if (outPtr + 1 >= outEnd)
                break;

            // Store the leading surrogate char
            tmpVal -= 0x10000;
            *sizePtr++ = trailingBytes + 1;
            *outPtr++ = XMLCh((tmpVal >> 10) + 0xD800);

            //
            //  And then the trailing char. This one accounts for no
            //  bytes eaten from the source, so set the char size for this
            //  one to be zero.
            //
            *sizePtr++ = 0;
            *outPtr++ = XMLCh((tmpVal & 0x3FF) + 0xDC00);
        }
    }

    // Update the bytes eaten
    bytesEaten = srcPtr - srcData;

    // Return the characters read
    return outPtr - toFill;
}


XMLSize_t
XMLUTF8Transcoder::transcodeTo( const   XMLCh* const    srcData
                                , const XMLSize_t       srcCount
                                ,       XMLByte* const  toFill
                                , const XMLSize_t       maxBytes
                                ,       XMLSize_t&      charsEaten
                                , const UnRepOpts       options)
{
    // Watch for pathological scenario. Shouldn't happen, but...
    if (!srcCount || !maxBytes)
        return 0;

    //
    //  Get pointers to our start and end points of the input and output
    //  buffers.
    //
    const XMLCh*    srcPtr = srcData;
    const XMLCh*    srcEnd = srcPtr + srcCount;
    XMLByte*        outPtr = toFill;
    XMLByte*        outEnd = toFill + maxBytes;

    while (srcPtr < srcEnd)
    {
        //
        //  Tentatively get the next char out. We have to get it into a
        //  32 bit value, because it could be a surrogate pair.
        //
        XMLUInt32 curVal = *srcPtr;

        //
        //  If its a leading surrogate, then lets see if we have the trailing
        //  available. If not, then give up now and leave it for next time.
        //
        unsigned int srcUsed = 1;
        if ((curVal >= 0xD800) && (curVal <= 0xDBFF))
        {
            if (srcPtr + 1 >= srcEnd)
                break;

            // Create the composite surrogate pair
            curVal = ((curVal - 0xD800) << 10)
                    + ((*(srcPtr + 1) - 0xDC00) + 0x10000);

            // And indicate that we ate another one
            srcUsed++;
        }

        // Figure out how many bytes we need
        unsigned int encodedBytes;
        if (curVal < 0x80)
            encodedBytes = 1;
        else if (curVal < 0x800)
            encodedBytes = 2;
        else if (curVal < 0x10000)
            encodedBytes = 3;
        else if (curVal < 0x110000)
            encodedBytes = 4;
        else
        {
            // If the options say to throw, then throw
            if (options == UnRep_Throw)
            {
                XMLCh tmpBuf[17];
                XMLString::binToText(curVal, tmpBuf, 16, 16, getMemoryManager());
                ThrowXMLwithMemMgr2
                (
                    TranscodingException
                    , XMLExcepts::Trans_Unrepresentable
                    , tmpBuf
                    , getEncodingName()
                    , getMemoryManager()
                );
            }

            // Else, use the replacement character
            *outPtr++ = chSpace;
            srcPtr += srcUsed;
            continue;
        }

        //
        //  If we cannot fully get this char into the output buffer,
        //  then leave it for the next time.
        //
        if (outPtr + encodedBytes > outEnd)
            break;

        // We can do it, so update the source index
        srcPtr += srcUsed;

        //
        //  And spit out the bytes. We spit them out in reverse order
        //  here, so bump up the output pointer and work down as we go.
        //
        outPtr += encodedBytes;
        switch(encodedBytes)
        {
            case 6 : *--outPtr = XMLByte((curVal | 0x80UL) & 0xBFUL);
                     curVal >>= 6;
            case 5 : *--outPtr = XMLByte((curVal | 0x80UL) & 0xBFUL);
                     curVal >>= 6;
            case 4 : *--outPtr = XMLByte((curVal | 0x80UL) & 0xBFUL);
                     curVal >>= 6;
            case 3 : *--outPtr = XMLByte((curVal | 0x80UL) & 0xBFUL);
                     curVal >>= 6;
            case 2 : *--outPtr = XMLByte((curVal | 0x80UL) & 0xBFUL);
                     curVal >>= 6;
            case 1 : *--outPtr = XMLByte
                     (
                        curVal | gFirstByteMark[encodedBytes]
                     );
        }

        // Add the encoded bytes back in again to indicate we've eaten them
        outPtr += encodedBytes;
    }

    // Fill in the chars we ate
    charsEaten = (srcPtr - srcData);

    // And return the bytes we filled in
    return (outPtr - toFill);
}


bool XMLUTF8Transcoder::canTranscodeTo(const unsigned int toCheck)
{
    // We can represent anything in the Unicode (with surrogates) range
    return (toCheck <= 0x10FFFF);
}

XERCES_CPP_NAMESPACE_END

