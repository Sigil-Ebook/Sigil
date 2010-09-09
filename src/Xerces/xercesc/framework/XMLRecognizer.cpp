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
 *  $Id: XMLRecognizer.cpp 555320 2007-07-11 16:05:13Z amassari $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/framework/XMLRecognizer.hpp>
#include <xercesc/util/RuntimeException.hpp>
#include <xercesc/util/XMLString.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Local data
//
//  gEncodingNameMap
//      This array maps the Encodings enum values to their canonical names.
//      Be sure to keep this in sync with that enum!
// ---------------------------------------------------------------------------
static const XMLCh* gEncodingNameMap[XMLRecognizer::Encodings_Count] =
{
    XMLUni::fgEBCDICEncodingString
    , XMLUni::fgUCS4BEncodingString
    , XMLUni::fgUCS4LEncodingString
    , XMLUni::fgUSASCIIEncodingString
    , XMLUni::fgUTF8EncodingString
    , XMLUni::fgUTF16BEncodingString
    , XMLUni::fgUTF16LEncodingString
    , XMLUni::fgXMLChEncodingString
};



// ---------------------------------------------------------------------------
//  XMLRecognizer: Public, const static data
//
//  gXXXPre
//  gXXXPreLen
//      The byte sequence prefixes for all of the encodings that we can
//      auto sense. Also included is the length of each sequence.
// ---------------------------------------------------------------------------
const char           XMLRecognizer::fgASCIIPre[]  = { 0x3C, 0x3F, 0x78, 0x6D, 0x6C, 0x20 };
const XMLSize_t      XMLRecognizer::fgASCIIPreLen = 6;
const XMLByte        XMLRecognizer::fgEBCDICPre[] = { 0x4C, 0x6F, 0xA7, 0x94, 0x93, 0x40 };
const XMLSize_t      XMLRecognizer::fgEBCDICPreLen = 6;
const XMLByte        XMLRecognizer::fgUTF16BPre[] = { 0x00, 0x3C, 0x00, 0x3F, 0x00, 0x78, 0x00, 0x6D, 0x00, 0x6C, 0x00, 0x20 };
const XMLByte        XMLRecognizer::fgUTF16LPre[] = { 0x3C, 0x00, 0x3F, 0x00, 0x78, 0x00, 0x6D, 0x00, 0x6C, 0x00, 0x20, 0x00 };
const XMLSize_t      XMLRecognizer::fgUTF16PreLen = 12;
const XMLByte        XMLRecognizer::fgUCS4BPre[]  =
{
        0x00, 0x00, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x3F
    ,   0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x6D
    ,   0x00, 0x00, 0x00, 0x6C, 0x00, 0x00, 0x00, 0x20
};
const XMLByte        XMLRecognizer::fgUCS4LPre[]  =
{
        0x3C, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00
    ,   0x78, 0x00, 0x00, 0x00, 0x6D, 0x00, 0x00, 0x00
    ,   0x6C, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00
};
const XMLSize_t      XMLRecognizer::fgUCS4PreLen = 24;

const char           XMLRecognizer::fgUTF8BOM[] = {(char)0xEF, (char)0xBB, (char)0xBF};
const XMLSize_t      XMLRecognizer::fgUTF8BOMLen = 3;

// ---------------------------------------------------------------------------
//  XMLRecognizer: Encoding recognition methods
// ---------------------------------------------------------------------------
XMLRecognizer::Encodings
XMLRecognizer::basicEncodingProbe(  const   XMLByte* const  rawBuffer
                                    , const XMLSize_t       rawByteCount)
{
    //
    //  As an optimization to check the 90% case, check first for the ASCII
    //  sequence '<?xml', which means its either US-ASCII, UTF-8, or some
    //  other encoding that we don't do manually but which happens to share
    //  the US-ASCII code points for these characters. So just return UTF-8
    //  to get us through the first line.
    //
    if (rawByteCount >= fgASCIIPreLen)
    {
        if (!memcmp(rawBuffer, fgASCIIPre, fgASCIIPreLen))
            return UTF_8;
    }

    //
    //  If the count of raw bytes is less than 2, it cannot be anything
    //  we understand, so return UTF-8 as a fallback.
    //
    if (rawByteCount < 2)
        return UTF_8;
         
    //  
    //  We have two to four bytes, so lets check for a UTF-16 BOM. That
    //  is quick to check and enough to identify two major encodings.   
    // 

    if (rawByteCount < 4)
    {
        if ((rawBuffer[0] == 0xFE) && (rawBuffer[1] == 0xFF))
            return UTF_16B;
        else if ((rawBuffer[0] == 0xFF) && (rawBuffer[1] == 0xFE))
            return UTF_16L;
        else 
            return UTF_8;
    }

    /***
     *    F.1 Detection Without External Encoding Information
     *
     *    Because each XML entity not accompanied by external encoding information and 
     *    not in UTF-8 or UTF-16 encoding must begin with an XML encoding declaration, 
     *    in which the first characters must be '<?xml', any conforming processor can detect, 
     *    after two to four octets of input, which of the following cases apply. 
     *
     *    In reading this list, it may help to know that in UCS-4, '<' is "#x0000003C" and 
     *    '?' is "#x0000003F", and the Byte Order Mark required of UTF-16 data streams is 
     *    "#xFEFF". The notation ## is used to denote any byte value except that two consecutive 
     *    ##s cannot be both 00.
     *
     *    With a Byte Order Mark:
     *
     *    00 00 FE FF           UCS-4,    big-endian machine    (1234 order) 
     *    FF FE 00 00           UCS-4,    little-endian machine (4321 order) 
     *    00 00 FF FE           UCS-4,    unusual octet order   (2143) 
     *    FE FF 00 00           UCS-4,    unusual octet order   (3412) 
     *    FE FF ## ##           UTF-16,   big-endian 
     *    FF FE ## ##           UTF-16,   little-endian 
     *    EF BB BF              UTF-8 
     *
     ***/

    //
    //  We have at least four bytes, so we can check all BOM
    //  for UCS-4BE, UCS-4LE, UTF-16BE and UTF-16LE as well.
    //
    if ((rawBuffer[0] == 0x00) && (rawBuffer[1] == 0x00) && (rawBuffer[2] == 0xFE) && (rawBuffer[3] == 0xFF))
        return UCS_4B;
    else if ((rawBuffer[0] == 0xFF) && (rawBuffer[1] == 0xFE) && (rawBuffer[2] == 0x00) && (rawBuffer[3] == 0x00))
        return UCS_4L;
    else if ((rawBuffer[0] == 0xFE) && (rawBuffer[1] == 0xFF))
        return UTF_16B;
    else if ((rawBuffer[0] == 0xFF) && (rawBuffer[1] == 0xFE))
        return UTF_16L;

    //
    //  We have at least 4 bytes. So lets check the 4 byte sequences that
    //  indicate other UTF-16 and UCS encodings.
    //
    if ((rawBuffer[0] == 0x00) || (rawBuffer[0] == 0x3C))
    {
        if (rawByteCount >= fgUCS4PreLen && !memcmp(rawBuffer, fgUCS4BPre, fgUCS4PreLen))
            return UCS_4B;
        else if (rawByteCount >= fgUCS4PreLen && !memcmp(rawBuffer, fgUCS4LPre, fgUCS4PreLen))
            return UCS_4L;
        else if (rawByteCount >= fgUTF16PreLen && !memcmp(rawBuffer, fgUTF16BPre, fgUTF16PreLen))
            return UTF_16B;
        else if (rawByteCount >= fgUTF16PreLen && !memcmp(rawBuffer, fgUTF16LPre, fgUTF16PreLen))
            return UTF_16L;
    }

    //
    //  See if we have enough bytes to possibly match the EBCDIC prefix.
    //  If so, try it.
    //
    if (rawByteCount > fgEBCDICPreLen)
    {
        if (!memcmp(rawBuffer, fgEBCDICPre, fgEBCDICPreLen))
            return EBCDIC;
    }

    //
    //  Does not seem to be anything we know, so go with UTF-8 to get at
    //  least through the first line and see what it really is.
    //
    return UTF_8;
}


XMLRecognizer::Encodings
XMLRecognizer::encodingForName(const XMLCh* const encName)
{
    //
    //  Compare the passed string, assume input string is already uppercased,
    //  to the variations that we recognize.
    //
    //  !!NOTE: Note that we don't handle EBCDIC here because we don't handle
    //  that one ourselves. It is allowed to fall into 'other'.
    //
    if (encName == XMLUni::fgXMLChEncodingString ||
        !XMLString::compareString(encName, XMLUni::fgXMLChEncodingString))
    {
        return XMLRecognizer::XERCES_XMLCH;
    }
    else if (!XMLString::compareString(encName, XMLUni::fgUTF8EncodingString)
         ||  !XMLString::compareString(encName, XMLUni::fgUTF8EncodingString2))
    {
        return XMLRecognizer::UTF_8;
    }
    else if (!XMLString::compareString(encName, XMLUni::fgUSASCIIEncodingString)
         ||  !XMLString::compareString(encName, XMLUni::fgUSASCIIEncodingString2)
         ||  !XMLString::compareString(encName, XMLUni::fgUSASCIIEncodingString3)
         ||  !XMLString::compareString(encName, XMLUni::fgUSASCIIEncodingString4))
    {
        return XMLRecognizer::US_ASCII;
    }
    else if (!XMLString::compareString(encName, XMLUni::fgUTF16LEncodingString)
         ||  !XMLString::compareString(encName, XMLUni::fgUTF16LEncodingString2))
    {
        return XMLRecognizer::UTF_16L;
    }
    else if (!XMLString::compareString(encName, XMLUni::fgUTF16BEncodingString)
         ||  !XMLString::compareString(encName, XMLUni::fgUTF16BEncodingString2))
    {
        return XMLRecognizer::UTF_16B;
    }
    else if (!XMLString::compareString(encName, XMLUni::fgUTF16EncodingString))
    {
        return XMLPlatformUtils::fgXMLChBigEndian?XMLRecognizer::UTF_16B:XMLRecognizer::UTF_16L;
    }
    else if (!XMLString::compareString(encName, XMLUni::fgUCS4LEncodingString)
         ||  !XMLString::compareString(encName, XMLUni::fgUCS4LEncodingString2))
    {
        return XMLRecognizer::UCS_4L;
    }
    else if (!XMLString::compareString(encName, XMLUni::fgUCS4BEncodingString)
         ||  !XMLString::compareString(encName, XMLUni::fgUCS4BEncodingString2))
    {
        return XMLRecognizer::UCS_4B;
    }
    else if (!XMLString::compareString(encName, XMLUni::fgUCS4EncodingString))
    {
        return XMLPlatformUtils::fgXMLChBigEndian?XMLRecognizer::UCS_4B:XMLRecognizer::UCS_4L;
    }

    // Return 'other' since we don't recognizer it
    return XMLRecognizer::OtherEncoding;
}


const XMLCh*
XMLRecognizer::nameForEncoding(const XMLRecognizer::Encodings theEncoding
                               , MemoryManager* const manager)
{
    if (theEncoding >= Encodings_Count)
        ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::XMLRec_UnknownEncoding, manager);

    return gEncodingNameMap[theEncoding];
}

XERCES_CPP_NAMESPACE_END
