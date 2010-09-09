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
 * $Id: InMemMsgLoader.hpp 570552 2007-08-28 19:57:36Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_INMEMMSGLOADER_HPP)
#define XERCESC_INCLUDE_GUARD_INMEMMSGLOADER_HPP

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/util/XMLMsgLoader.hpp>

XERCES_CPP_NAMESPACE_BEGIN

//
//  This is a simple in memory message loader implementation. For those
//  folks who just want a single language and want something very fast and
//  efficient, can basically just provide a couple of arrays of Unicode
//  strings that can be looked up by the message id.
//
class XMLUTIL_EXPORT InMemMsgLoader : public XMLMsgLoader
{
public :
    // -----------------------------------------------------------------------
    //  Public Constructors and Destructor
    // -----------------------------------------------------------------------
    InMemMsgLoader(const XMLCh* const msgDomain);
    ~InMemMsgLoader();


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
        , MemoryManager* const          manager  = XMLPlatformUtils::fgMemoryManager
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
        , MemoryManager * const         manager  = XMLPlatformUtils::fgMemoryManager
    );


private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    InMemMsgLoader();
    InMemMsgLoader(const InMemMsgLoader&);
    InMemMsgLoader& operator=(const InMemMsgLoader&);


    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fMsgDomain
    //      This is the message domain that we are for loading message from.
    // -----------------------------------------------------------------------
    XMLCh*  fMsgDomain;
};

XERCES_CPP_NAMESPACE_END

#endif
