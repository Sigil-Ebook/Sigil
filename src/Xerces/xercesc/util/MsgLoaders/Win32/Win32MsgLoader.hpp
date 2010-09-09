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
 * $Id: Win32MsgLoader.hpp 570552 2007-08-28 19:57:36Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_WIN32MSGLOADER_HPP)
#define XERCESC_INCLUDE_GUARD_WIN32MSGLOADER_HPP

#include <windows.h>
#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/util/XMLMsgLoader.hpp>

XERCES_CPP_NAMESPACE_BEGIN

//
//  This is a simple in Win32 RC message loader implementation.
//
class XMLUTIL_EXPORT Win32MsgLoader : public XMLMsgLoader
{
public :
    // -----------------------------------------------------------------------
    //  Public Constructors and Destructor
    // -----------------------------------------------------------------------
    Win32MsgLoader(const XMLCh* const msgDomain);
    ~Win32MsgLoader();


    // -----------------------------------------------------------------------
    //  Implementation of the virtual message loader API
    // -----------------------------------------------------------------------
    virtual bool loadMsg
    (
        const   XMLMsgLoader::XMLMsgId  msgToLoad
        ,       XMLCh* const            toFill
        , const XMLSize_t               maxChars
    );

    virtual bool loadMsg
    (
        const   XMLMsgLoader::XMLMsgId  msgToLoad
        ,       XMLCh* const            toFill
        , const XMLSize_t               maxChars
        , const XMLCh* const            repText1
        , const XMLCh* const            repText2 = 0
        , const XMLCh* const            repText3 = 0
        , const XMLCh* const            repText4 = 0
        , MemoryManager* const          manger   = XMLPlatformUtils::fgMemoryManager
    );

    virtual bool loadMsg
    (
        const   XMLMsgLoader::XMLMsgId  msgToLoad
        ,       XMLCh* const            toFill
        , const XMLSize_t               maxChars
        , const char* const             repText1
        , const char* const             repText2 = 0
        , const char* const             repText3 = 0
        , const char* const             repText4 = 0
        , MemoryManager* const          manager  = XMLPlatformUtils::fgMemoryManager
    );


private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    Win32MsgLoader();
    Win32MsgLoader(const Win32MsgLoader&);
    Win32MsgLoader& operator=(const Win32MsgLoader&);


    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fDomainOfs
    //      This is the id offset for the current domain. Its used to bias
    //      the zero based id of each domain, since they are stored in the
    //      same file and have to have unique ids internally. This is set
    //      in the ctor from the domain name. We just have to agree with
    //      what our formatter in the NLSXlat program does.
    //
    //  fModHandle
    //      This is our DLL module handle that we need in order to load
    //      resource messages. This is set during construction.
    //
    //  fMsgDomain
    //      This is the name of the error domain that this loader is for.
    // -----------------------------------------------------------------------
    unsigned int    fDomainOfs;
    HINSTANCE       fModHandle;
    XMLCh*          fMsgDomain;
};

XERCES_CPP_NAMESPACE_END

#endif
