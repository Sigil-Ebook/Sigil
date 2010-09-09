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
#include <xercesc/util/XMLChTranscoder.hpp>
#include <string.h>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  XMLChTranscoder: Constructors and Destructor
// ---------------------------------------------------------------------------
XMLChTranscoder::XMLChTranscoder(const  XMLCh* const   encodingName
                                , const XMLSize_t      blockSize
                                , MemoryManager* const manager) :

    XMLTranscoder(encodingName, blockSize, manager)
{
}


XMLChTranscoder::~XMLChTranscoder()
{
}


// ---------------------------------------------------------------------------
//  XMLChTranscoder: Implementation of the transcoder API
// ---------------------------------------------------------------------------
XMLSize_t
XMLChTranscoder::transcodeFrom( const   XMLByte* const          srcData
                                , const XMLSize_t               srcCount
                                ,       XMLCh* const            toFill
                                , const XMLSize_t               maxChars
                                ,       XMLSize_t&              bytesEaten
                                ,       unsigned char* const    charSizes)
{
    //
    //  Calculate the max chars we can do here. Its the lesser of the
    //  max output chars and the number of chars in the source.
    //
    const XMLSize_t srcChars = srcCount / sizeof(XMLCh);
    const XMLSize_t countToDo = srcChars < maxChars ? srcChars : maxChars;

    //
    //  Copy over the count of chars that we precalculated. Notice we
    //  convert char count to byte count here!!!
    //
    memcpy(toFill, srcData, countToDo * sizeof(XMLCh));

    // Set the bytes eaten
    bytesEaten = countToDo * sizeof(XMLCh);

    // Set the character sizes to the fixed size
    memset(charSizes, sizeof(XMLCh), countToDo);

    // Return the chars we transcoded
    return countToDo;
}


XMLSize_t
XMLChTranscoder::transcodeTo(const  XMLCh* const    srcData
                            , const XMLSize_t       srcCount
                            ,       XMLByte* const  toFill
                            , const XMLSize_t       maxBytes
                            ,       XMLSize_t&      charsEaten
                                , const UnRepOpts)
{
    //
    //  Calculate the max chars we can do here. Its the lesser of the
    //  max chars we can store in the output byte buffer, and the number
    //  of chars in the source.
    //
    const XMLSize_t maxOutChars  = maxBytes / sizeof(XMLCh);
    const XMLSize_t countToDo    = maxOutChars < srcCount
                                    ? maxOutChars : srcCount;

    //
    //  Copy over the number of chars we calculated. Note that we have
    //  to convert the char count to a byte count!!
    //
    memcpy(toFill, srcData, countToDo * sizeof(XMLCh));

    // Set the chars eaten
    charsEaten = countToDo;

    // Return the bytes we transcoded
    return countToDo * sizeof(XMLCh);
}


bool XMLChTranscoder::canTranscodeTo(const unsigned int)
{
    // We can handle anything
    return true;
}

XERCES_CPP_NAMESPACE_END
