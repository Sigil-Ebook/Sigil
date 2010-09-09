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
 * $Id: XMLEBCDICTranscoder.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XMLEBCDICTRANSCODER_HPP)
#define XERCESC_INCLUDE_GUARD_XMLEBCDICTRANSCODER_HPP

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/util/XML256TableTranscoder.hpp>

XERCES_CPP_NAMESPACE_BEGIN

//
//  This class provides an implementation of the XMLTranscoder interface
//  for a simple EBCDIC-US transcoder. The parser does some encodings
//  intrinsically without depending upon external transcoding services.
//  To make everything more orthogonal, we implement these internal
//  transcoders using the same transcoder abstraction as the pluggable
//  transcoding services do.
//
//  EBCDIC-US is the same as IBM037, CP37, EBCDIC-CP-US, etc...
//
class XMLUTIL_EXPORT XMLEBCDICTranscoder : public XML256TableTranscoder
{
public :
    // -----------------------------------------------------------------------
    //  Public, static methods
    // -----------------------------------------------------------------------
    static XMLCh xlatThisOne(const XMLByte toXlat);


    // -----------------------------------------------------------------------
    //  Public constructors and destructor
    // -----------------------------------------------------------------------
    XMLEBCDICTranscoder
    (
        const   XMLCh* const   encodingName
        , const XMLSize_t      blockSize
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    virtual ~XMLEBCDICTranscoder();


private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XMLEBCDICTranscoder();
    XMLEBCDICTranscoder(const XMLEBCDICTranscoder&);
    XMLEBCDICTranscoder& operator=(const XMLEBCDICTranscoder&);
};

XERCES_CPP_NAMESPACE_END

#endif
