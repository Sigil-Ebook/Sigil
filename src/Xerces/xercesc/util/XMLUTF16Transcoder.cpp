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
#include <xercesc/util/XMLUTF16Transcoder.hpp>
#include <xercesc/util/TranscodingException.hpp>
#include <string.h>

XERCES_CPP_NAMESPACE_BEGIN



// ---------------------------------------------------------------------------
//  XMLUTF16Transcoder: Constructors and Destructor
// ---------------------------------------------------------------------------
XMLUTF16Transcoder::XMLUTF16Transcoder( const   XMLCh* const    encodingName
                                        , const XMLSize_t       blockSize
                                        , const bool            swapped
                                        , MemoryManager* const manager) :

    XMLTranscoder(encodingName, blockSize, manager)
    , fSwapped(swapped)
{
}


XMLUTF16Transcoder::~XMLUTF16Transcoder()
{
}


// ---------------------------------------------------------------------------
//  XMLUTF16Transcoder: Implementation of the transcoder API
// ---------------------------------------------------------------------------
XMLSize_t
XMLUTF16Transcoder::transcodeFrom(  const   XMLByte* const       srcData
                                    , const XMLSize_t            srcCount
                                    ,       XMLCh* const         toFill
                                    , const XMLSize_t            maxChars
                                    ,       XMLSize_t&           bytesEaten
                                    ,       unsigned char* const charSizes)
{
    //
    //  Calculate the max chars we can do here. Its the lesser of the
    //  max output chars and the number of chars in the source.
    //
    const XMLSize_t srcChars = srcCount / sizeof(UTF16Ch);
    const XMLSize_t countToDo = srcChars < maxChars ? srcChars : maxChars;

    // Look at the source data as UTF16 chars
    const UTF16Ch* asUTF16 = (const UTF16Ch*)srcData;

    // And get a mutable pointer to the output
    XMLCh* outPtr = toFill;

    //
    //  If its swapped, we have to do a char by char swap and cast. Else
    //  we have to check whether our XMLCh and UTF16Ch types are the same
    //  size or not. If so, we can optimize by just doing a buffer copy.
    //
    if (fSwapped)
    {
        //
        //  And then do the swapping loop for the count we precalculated. Note
        //  that this also handles size conversion as well if XMLCh is not the
        //  same size as UTF16Ch.
        //
        for (XMLSize_t index = 0; index < countToDo; index++)
            *outPtr++ = BitOps::swapBytes(*asUTF16++);
    }
     else
    {
        //
        //  If the XMLCh type is the same size as a UTF16 value on this
        //  platform, then we can do just a buffer copy straight to the target
        //  buffer since our source chars are UTF-16 chars. If its not, then
        //  we still have to do a loop and assign each one, in order to
        //  implicitly convert.
        //
        if (sizeof(XMLCh) == sizeof(UTF16Ch))
        {
            //  Notice we convert char count to byte count here!!!
            memcpy(toFill, srcData, countToDo * sizeof(UTF16Ch));
        }
         else
        {
            for (XMLSize_t index = 0; index < countToDo; index++)
                *outPtr++ = XMLCh(*asUTF16++);
        }
    }

    // Set the bytes eaten
    bytesEaten = countToDo * sizeof(UTF16Ch);

    // Set the character sizes to the fixed size
    memset(charSizes, sizeof(UTF16Ch), countToDo);

    // Return the chars we transcoded
    return countToDo;
}


XMLSize_t
XMLUTF16Transcoder::transcodeTo(const   XMLCh* const    srcData
                                , const XMLSize_t       srcCount
                                ,       XMLByte* const  toFill
                                , const XMLSize_t       maxBytes
                                ,       XMLSize_t&      charsEaten
                                , const UnRepOpts)
{
    //
    //  Calculate the max chars we can do here. Its the lesser of the
    //  chars that we can fit into the output buffer, and the source
    //  chars available.
    //
    const XMLSize_t maxOutChars = maxBytes / sizeof(UTF16Ch);
    const XMLSize_t countToDo = srcCount < maxOutChars ? srcCount : maxOutChars;

    //
    //  Get a pointer tot he output buffer in the UTF-16 character format
    //  that we need to work with. And get a mutable pointer to the source
    //  character buffer.
    //
    UTF16Ch*        outPtr = (UTF16Ch*)toFill;
    const XMLCh*    srcPtr = srcData;

    //
    //  If the target format is swapped from our native format, then handle
    //  it one way, else handle it another.
    //
    if (fSwapped)
    {
        //
        //  And then do the swapping loop for the count we precalculated. Note
        //  that this also handles size conversion as well if XMLCh is not the
        //  same size as UTF16Ch.
        //
        for (XMLSize_t index = 0; index < countToDo; index++)
        {
            // To avoid flakey compilers, use a temp
            const UTF16Ch tmpCh = UTF16Ch(*srcPtr++);
            *outPtr++ = BitOps::swapBytes(tmpCh);
        }
    }
     else
    {
        //
        //  If XMLCh and UTF16Ch are the same size, we can just do a fast
        //  memory copy. Otherwise, we have to do a loop and downcast each
        //  character into its new 16 bit storage.
        //
        if (sizeof(XMLCh) == sizeof(UTF16Ch))
        {
            //  Notice we convert char count to byte count here!!!
            memcpy(toFill, srcData, countToDo * sizeof(UTF16Ch));
        }
         else
        {
            for (XMLSize_t index = 0; index < countToDo; index++)
                *outPtr++ = UTF16Ch(*srcPtr++);
        }
    }

    // Set the chars eaten to the calculated number we ate
    charsEaten = countToDo;

    //Return the bytes we ate. Note we convert to a byte count here!
    return countToDo * sizeof(UTF16Ch);
}


bool XMLUTF16Transcoder::canTranscodeTo(const unsigned int)
{
    // We can handle anything
    return true;
}

XERCES_CPP_NAMESPACE_END

