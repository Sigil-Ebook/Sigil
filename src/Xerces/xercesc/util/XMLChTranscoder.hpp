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
 * $Id: XMLChTranscoder.hpp 635560 2008-03-10 14:10:09Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XMLCHTRANSCODER_HPP)
#define XERCESC_INCLUDE_GUARD_XMLCHTRANSCODER_HPP

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/util/TransService.hpp>

XERCES_CPP_NAMESPACE_BEGIN

//
//  This class provides an implementation of the XMLTranscoder interface
//  for a simple XMLCh transcoder. This is used for internal entities, which
//  are already in the native XMLCh format.
//
class XMLUTIL_EXPORT XMLChTranscoder : public XMLTranscoder
{
public :
    // -----------------------------------------------------------------------
    //  Public constructors and destructor
    // -----------------------------------------------------------------------
    XMLChTranscoder
    (
        const   XMLCh* const   encodingName
        , const XMLSize_t      blockSize
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    virtual ~XMLChTranscoder();


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
    XMLChTranscoder(const XMLChTranscoder&);
    XMLChTranscoder& operator=(const XMLChTranscoder&);
};

XERCES_CPP_NAMESPACE_END

#endif
