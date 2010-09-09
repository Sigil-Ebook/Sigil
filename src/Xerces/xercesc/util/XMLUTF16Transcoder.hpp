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

/*
 * $Id: XMLUTF16Transcoder.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XMLUTF16TRANSCODER_HPP)
#define XERCESC_INCLUDE_GUARD_XMLUTF16TRANSCODER_HPP

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/util/TransService.hpp>

XERCES_CPP_NAMESPACE_BEGIN


//
//  This class provides an implementation of the XMLTranscoder interface
//  for a simple UTF16 transcoder. The parser does some encodings
//  intrinsically without depending upon external transcoding services.
//  To make everything more orthogonal, we implement these internal
//  transcoders using the same transcoder abstraction as the pluggable
//  transcoding services do.
//
class XMLUTIL_EXPORT XMLUTF16Transcoder : public XMLTranscoder
{
public :
    // -----------------------------------------------------------------------
    //  Public constructors and destructor
    // -----------------------------------------------------------------------
    XMLUTF16Transcoder
    (
        const   XMLCh* const    encodingName
        , const XMLSize_t       blockSize
        , const bool            swapped
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    virtual ~XMLUTF16Transcoder();


    // -----------------------------------------------------------------------
    //  Implementation of the XMLTranscoder interface
    // -----------------------------------------------------------------------
    virtual XMLSize_t transcodeFrom
    (
        const   XMLByte* const          srcData
        , const XMLSize_t               srcCount
        ,       XMLCh* const            toFill
        , const XMLSize_t               maxChars
        ,       XMLSize_t&              bytesEaten
        ,       unsigned char* const    charSizes
    );

    virtual XMLSize_t transcodeTo
    (
        const   XMLCh* const    srcData
        , const XMLSize_t       srcCount
        ,       XMLByte* const  toFill
        , const XMLSize_t       maxBytes
        ,       XMLSize_t&      charsEaten
        , const UnRepOpts       options
    );

    virtual bool canTranscodeTo
    (
        const   unsigned int    toCheck
    );


private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XMLUTF16Transcoder(const XMLUTF16Transcoder&);
    XMLUTF16Transcoder& operator=(const XMLUTF16Transcoder&);


    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fSwapped
    //      Indicates whether the encoding is of the opposite endianness from
    //      the local host.
    // -----------------------------------------------------------------------
    bool    fSwapped;
};

XERCES_CPP_NAMESPACE_END

#endif
