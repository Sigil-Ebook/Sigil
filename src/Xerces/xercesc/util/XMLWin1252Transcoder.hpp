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
 * $Id: XMLWin1252Transcoder.hpp 570552 2007-08-28 19:57:36Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XMLWIN1252TRANSCODER_HPP)
#define XERCESC_INCLUDE_GUARD_XMLWIN1252TRANSCODER_HPP

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/util/XML256TableTranscoder.hpp>

XERCES_CPP_NAMESPACE_BEGIN


//
//  This class provides an implementation of the XMLTranscoder interface
//  for the Windows variant of Latin1, called Windows-1252. Its close to
//  Latin1, but is somewhat different.
//
class XMLUTIL_EXPORT XMLWin1252Transcoder : public XML256TableTranscoder
{
public :
    // -----------------------------------------------------------------------
    //  Public constructors and destructor
    // -----------------------------------------------------------------------
    XMLWin1252Transcoder
    (
        const   XMLCh* const   encodingName
        , const XMLSize_t      blockSize
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    virtual ~XMLWin1252Transcoder();


private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XMLWin1252Transcoder();
    XMLWin1252Transcoder(const XMLWin1252Transcoder&);
    XMLWin1252Transcoder& operator=(const XMLWin1252Transcoder&);
};

XERCES_CPP_NAMESPACE_END

#endif
