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


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/BitOps.hpp>
#include <xercesc/util/XMLUCS4Transcoder.hpp>
#include <xercesc/util/TranscodingException.hpp>
#include <string.h>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  XMLUCS4Transcoder: Constructors and Destructor
// ---------------------------------------------------------------------------
XMLUCS4Transcoder::XMLUCS4Transcoder(const  XMLCh* const    encodingName
                                    , const XMLSize_t       blockSize
                                    , const bool            swapped
                                    , MemoryManager* const manager) :

    XMLTranscoder(encodingName, blockSize, manager)
    , fSwapped(swapped)
{
}


XMLUCS4Transcoder::~XMLUCS4Transcoder()
{
}


// ---------------------------------------------------------------------------
//  XMLUCS4Transcoder: Implementation of the transcoder API
// ---------------------------------------------------------------------------
XMLSize_t
XMLUCS4Transcoder::transcodeFrom(const  XMLByte* const          srcData
                                , const XMLSize_t               srcCount
                                ,       XMLCh* const            toFill
                                , const XMLSize_t               maxChars
                                ,       XMLSize_t&              bytesEaten
                                ,       unsigned char* const    charSizes)
{
    //
    //  Get pointers to the start and end of the source buffer in terms of
    //  UCS-4 characters.
    //
    const UCS4Ch*   srcPtr = (const UCS4Ch*)srcData;
    const UCS4Ch*   srcEnd = srcPtr + (srcCount / sizeof(UCS4Ch));

    //
    //  Get pointers to the start and end of the target buffer, which is
    //  in terms of the XMLCh chars we output.
    //
    XMLCh*  outPtr = toFill;
    XMLCh*  outEnd = toFill + maxChars;

    //
    //  And get a pointer into the char sizes buffer. We will run this
    //  up as we put chars into the output buffer.
    //
    unsigned char* sizePtr = charSizes;

    //
    //  Now process chars until we either use up all our source or all of
    //  our output space.
    //
    while ((outPtr < outEnd) && (srcPtr < srcEnd))
    {
        //
        //  Get the next UCS char out of the buffer. Don't bump the ptr
        //  yet since we might not have enough storage for it in the target
        //  (if its causes a surrogate pair to be created.
        //
        UCS4Ch nextVal = *srcPtr;

        // If it needs to be swapped, then do it
        if (fSwapped)
            nextVal = BitOps::swapBytes(nextVal);

        // Handle a surrogate pair if needed
        if (nextVal & 0xFFFF0000)
        {
            //
            //  If we don't have room for both of the chars, then we
            //  bail out now.
            //
            if (outPtr + 1 == outEnd)
                break;

            const XMLInt32 LEAD_OFFSET = 0xD800 - (0x10000 >> 10);
	        const XMLCh ch1 = XMLCh(LEAD_OFFSET + (nextVal >> 10));
	        const XMLCh ch2 = XMLCh(0xDC00 + (nextVal & 0x3FF));

            //
            //  We have room so store them both. But note that the
            //  second one took up no source bytes!
            //
            *sizePtr++ = sizeof(UCS4Ch);
            *outPtr++ = ch1;
            *sizePtr++ = 0;
            *outPtr++ = ch2;
        }
         else
        {
            //
            //  No surrogate, so just store it and bump the count of chars
            //  read. Update the char sizes buffer for this char's entry.
            //
            *sizePtr++ = sizeof(UCS4Ch);
            *outPtr++ = XMLCh(nextVal);
        }

        // Indicate that we ate another UCS char's worth of bytes
        srcPtr++;
    }

    // Set the bytes eaten parameter
    bytesEaten = ((const XMLByte*)srcPtr) - srcData;

    // And return the chars written into the output buffer
    return outPtr - toFill;
}


XMLSize_t
XMLUCS4Transcoder::transcodeTo( const   XMLCh* const    srcData
                                , const XMLSize_t       srcCount
                                ,       XMLByte* const  toFill
                                , const XMLSize_t       maxBytes
                                ,       XMLSize_t&      charsEaten
                                , const UnRepOpts)
{
    //
    //  Get pointers to the start and end of the source buffer, which
    //  is in terms of XMLCh chars.
    //
    const XMLCh*  srcPtr = srcData;
    const XMLCh*  srcEnd = srcData + srcCount;

    //
    //  Get pointers to the start and end of the target buffer, in terms
    //  of UCS-4 chars.
    //
    UCS4Ch*   outPtr = (UCS4Ch*)toFill;
    UCS4Ch*   outEnd = outPtr + (maxBytes / sizeof(UCS4Ch));

    //
    //  Now loop until we either run out of source characters or we
    //  fill up our output buffer.
    //
    XMLCh trailCh;
    while ((outPtr < outEnd) && (srcPtr < srcEnd))
    {
        //
        //  Get out an XMLCh char from the source. Don't bump up the
        //  pointer yet, since it might be a leading for which we don't
        //  have the trailing.
        //
        const XMLCh curCh = *srcPtr;

        //
        //  If its a leading char of a surrogate pair handle it one way,
        //  else just cast it over into the target.
        //
        if ((curCh >= 0xD800) && (curCh <= 0xDBFF))
        {
            //
            //  Ok, we have to have another source char available or we
            //  just give up without eating the leading char.
            //
            if (srcPtr + 1 == srcEnd)
                break;

            //
            //  We have the trailing char, so eat the first char and the
            //  trailing char from the source.
            //
            srcPtr++;
            trailCh = *srcPtr++;

            //
            //  Then make sure its a legal trailing char. If not, throw
            //  an exception.
            //
            if ( !( (trailCh >= 0xDC00) && (trailCh <= 0xDFFF) ) )
                ThrowXMLwithMemMgr(TranscodingException, XMLExcepts::Trans_BadTrailingSurrogate, getMemoryManager());

            // And now combine the two into a single output char
            const XMLInt32 SURROGATE_OFFSET = 0x10000 - (0xD800 << 10) - 0xDC00;
            *outPtr++ = (curCh << 10) + trailCh + SURROGATE_OFFSET;
        }
         else
        {
            //
            //  Its just a char, so we can take it as is. If we need to
            //  swap it, then swap it. Because of flakey compilers, use
            //  a temp first.
            //
            const UCS4Ch tmpCh = UCS4Ch(curCh);
            if (fSwapped)
                *outPtr++ = BitOps::swapBytes(tmpCh);
            else
                *outPtr++ = tmpCh;

            // Bump the source pointer
            srcPtr++;
        }
    }

    // Set the chars we ate from the source
    charsEaten = srcPtr - srcData;

    // Return the bytes we wrote to the output
    return ((XMLByte*)outPtr) - toFill;
}


bool XMLUCS4Transcoder::canTranscodeTo(const unsigned int)
{
    // We can handle anything
    return true;
}

XERCES_CPP_NAMESPACE_END
