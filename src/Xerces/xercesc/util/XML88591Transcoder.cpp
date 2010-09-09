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
#include <xercesc/util/TranscodingException.hpp>
#include <xercesc/util/XML88591Transcoder.hpp>
#include <xercesc/util/XMLString.hpp>
#include <string.h>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  XML88591Transcoder: Constructors and Destructor
// ---------------------------------------------------------------------------
XML88591Transcoder::XML88591Transcoder( const   XMLCh* const    encodingName
                                        , const XMLSize_t       blockSize
                                        , MemoryManager* const  manager) :

    XMLTranscoder(encodingName, blockSize, manager)
{
}


XML88591Transcoder::~XML88591Transcoder()
{
}


// ---------------------------------------------------------------------------
//  XML88591Transcoder: Implementation of the transcoder API
// ---------------------------------------------------------------------------
XMLSize_t
XML88591Transcoder::transcodeFrom(  const   XMLByte* const       srcData
                                    , const XMLSize_t            srcCount
                                    ,       XMLCh* const         toFill
                                    , const XMLSize_t            maxChars
                                    ,       XMLSize_t&           bytesEaten
                                    ,       unsigned char* const charSizes)
{
    //
    //  Calculate the max chars we can do here. Its the lesser of the
    //  max output chars and the number of bytes in the source.
    //
    const XMLSize_t countToDo = srcCount < maxChars ? srcCount : maxChars;

    //
    //  Loop through the bytes to do and convert over each byte. Its just
    //  a cast to the wide char type.
    //
    const XMLByte*  srcPtr = srcData;
    XMLCh*          destPtr = toFill;
    const XMLByte*  srcEnd = srcPtr + countToDo;
    while (srcPtr < srcEnd)
        *destPtr++ = XMLCh(*srcPtr++);

    // Set the bytes eaten, and set the char size array to the fixed size
    bytesEaten = countToDo;
    memset(charSizes, 1, countToDo);

    // Return the chars we transcoded
    return countToDo;
}


XMLSize_t
XML88591Transcoder::transcodeTo(const   XMLCh* const    srcData
                                , const XMLSize_t       srcCount
                                ,       XMLByte* const  toFill
                                , const XMLSize_t       maxBytes
                                ,       XMLSize_t&      charsEaten
                                , const UnRepOpts       options)
{
    //
    //  Calculate the max chars we can do here. Its the lesser of the
    //  max output bytes and the number of chars in the source.
    //
    const XMLSize_t countToDo = srcCount < maxBytes ? srcCount : maxBytes;

    //
    //  Loop through the bytes to do and convert over each byte. Its just
    //  a downcast of the wide char, checking for unrepresentable chars.
    //
    const XMLCh*    srcPtr  = srcData;
    const XMLCh*    srcEnd  = srcPtr + countToDo;
    XMLByte*        destPtr = toFill;
    while (srcPtr < srcEnd)
    {
        // If its legal, take it and jump back to top
        if (*srcPtr < 256)
        {
            *destPtr++ = XMLByte(*srcPtr++);
            continue;
        }

        //
        //  Its not representable so use a replacement char. According to
        //  the options, either throw or use the replacement.
        //
        if (options == UnRep_Throw)
        {
            XMLCh tmpBuf[17];
            XMLString::binToText((unsigned int)*srcPtr, tmpBuf, 16, 16, getMemoryManager());
            ThrowXMLwithMemMgr2
            (
                TranscodingException
                , XMLExcepts::Trans_Unrepresentable
                , tmpBuf
                , getEncodingName()
                , getMemoryManager()
            );
        }
        *destPtr++ = 0x1A;
        srcPtr++;
    }

    // Set the chars eaten
    charsEaten = countToDo;

    // Return the bytes we transcoded
    return countToDo;
}


bool XML88591Transcoder::canTranscodeTo(const unsigned int toCheck)
{
    return (toCheck < 256);
}

XERCES_CPP_NAMESPACE_END
