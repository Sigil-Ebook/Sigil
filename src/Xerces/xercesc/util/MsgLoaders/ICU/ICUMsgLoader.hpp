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
 * $Id: ICUMsgLoader.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_ICUMSGLOADER_HPP)
#define XERCESC_INCLUDE_GUARD_ICUMSGLOADER_HPP

#include <xercesc/util/XMLMsgLoader.hpp>
#include "unicode/ures.h"

XERCES_CPP_NAMESPACE_BEGIN

//
//  This is the ICU specific implementation of the XMLMsgLoader interface.
//  This one uses ICU resource bundles to store its messages.
//
class XMLUTIL_EXPORT ICUMsgLoader : public XMLMsgLoader
{
public :
    // -----------------------------------------------------------------------
    //  Public Constructors and Destructor
    // -----------------------------------------------------------------------
    ICUMsgLoader(const XMLCh* const  msgDomain);
    ~ICUMsgLoader();


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
    ICUMsgLoader();
    ICUMsgLoader(const ICUMsgLoader&);
    ICUMsgLoader& operator=(const ICUMsgLoader&);


    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fLocaleBundle
    //      pointer to the required locale specific resource bundle,
	//           or to the default locale resource bundle in case the required
	//              locale specific resource bundle unavailable.
    //
    //  fDomainBundle
    //      pointer to the domain specific resource bundle with in the
	//              required locale specific (or default locale) resource bundle.
    //
    // -----------------------------------------------------------------------
    UResourceBundle*      fLocaleBundle;
    UResourceBundle*      fDomainBundle;
};

XERCES_CPP_NAMESPACE_END

#endif
