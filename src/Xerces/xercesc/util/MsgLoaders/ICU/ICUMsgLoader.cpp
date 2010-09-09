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
 * $Id: ICUMsgLoader.cpp 883612 2009-11-24 07:24:53Z borisk $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLMsgLoader.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/Janitor.hpp>
#include "ICUMsgLoader.hpp"
#include "unicode/putil.h"
#include "unicode/uloc.h"
#include "unicode/udata.h"

#include "string.h"
#include <stdio.h>
#include <stdlib.h>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Local static methods
// ---------------------------------------------------------------------------

/*
 *  Resource Data Reference.
 *
 *  The data is packaged as a dll (or .so or whatever, depending on the platform) that exports a data symbol.
 *  The application (this *.cpp) references that symbol here, and will pass the data address to ICU, which
 *  will then  be able to fetch resources from the data.
 */
#define ENTRY_POINT xercesc_messages_3_1_dat
#define BUNDLE_NAME "xercesc_messages_3_1"

extern "C" void U_IMPORT *ENTRY_POINT;

/*
 *  Tell ICU where our resource data is located in memory. The data lives in the xercesc_nessages dll, and we just
 *  pass the address of an exported symbol from that library to ICU.
 */
static bool setAppDataOK = false;

static void setAppData()
{
    static bool setAppDataDone = false;

    if (setAppDataDone)
    {
        return;
    }
    else
    {
        setAppDataDone = true;
        UErrorCode err = U_ZERO_ERROR;
        udata_setAppData(BUNDLE_NAME, &ENTRY_POINT, &err);
        if (U_SUCCESS(err))
        {
    	    setAppDataOK = true;
        }
    }

}

// ---------------------------------------------------------------------------
//  Public Constructors and Destructor
// ---------------------------------------------------------------------------
ICUMsgLoader::ICUMsgLoader(const XMLCh* const  msgDomain)
:fLocaleBundle(0)
,fDomainBundle(0)
{
    /***
	    Validate msgDomain
    ***/
    if (!XMLString::equals(msgDomain, XMLUni::fgXMLErrDomain)    &&
        !XMLString::equals(msgDomain, XMLUni::fgExceptDomain)    &&
        !XMLString::equals(msgDomain, XMLUni::fgXMLDOMMsgDomain) &&
        !XMLString::equals(msgDomain, XMLUni::fgValidityDomain)   )
    {
        XMLPlatformUtils::panic(PanicHandler::Panic_UnknownMsgDomain);
    }

    /***
	Resolve domainName
    ***/
    int     index = XMLString::lastIndexOf(msgDomain, chForwardSlash);
    char*   domainName = XMLString::transcode(&(msgDomain[index + 1]), XMLPlatformUtils::fgMemoryManager);
    ArrayJanitor<char> jan1(domainName, XMLPlatformUtils::fgMemoryManager);

    /***
        Location resolution priority

         1. XMLMsgLoader::getNLSHome(), set by user through
            XMLPlatformUtils::Initialize(), which provides user-specified
            location where the message loader shall retrieve error messages.

         2. environment var: XERCESC_NLS_HOME

         3. path $XERCESCROOT/msg
    ***/

    char locationBuf[1024];
    memset(locationBuf, 0, sizeof locationBuf);
    const char *nlsHome = XMLMsgLoader::getNLSHome();

    if (nlsHome)
    {
    	strcpy(locationBuf, nlsHome);
        strcat(locationBuf, U_FILE_SEP_STRING);
    }
    else
    {
        nlsHome = getenv("XERCESC_NLS_HOME");
        if (nlsHome)
        {
            strcpy(locationBuf, nlsHome);
            strcat(locationBuf, U_FILE_SEP_STRING);
        }
        else
        {
            nlsHome = getenv("XERCESCROOT");
            if (nlsHome)
            {
                strcpy(locationBuf, nlsHome);
                strcat(locationBuf, U_FILE_SEP_STRING);
                strcat(locationBuf, "msg");
                strcat(locationBuf, U_FILE_SEP_STRING);
            }
            else
            {
                /***
                 leave it to ICU to decide where to search
                 for the error message.
                 ***/
                 setAppData();
            }
        }
    }

    /***
	Open the locale-specific resource bundle
    ***/
    strcat(locationBuf, BUNDLE_NAME);
    UErrorCode err = U_ZERO_ERROR;
    uloc_setDefault("root", &err);   // in case user-specified locale unavailable
    err = U_ZERO_ERROR;
    fLocaleBundle = ures_open(locationBuf, XMLMsgLoader::getLocale(), &err);
    if (!U_SUCCESS(err) || fLocaleBundle == NULL)
    {
    	/***
    	   in case user specified location does not work
    	   try the dll
        ***/

        if (strcmp(locationBuf, BUNDLE_NAME) !=0 )
        {
            setAppData();
            err = U_ZERO_ERROR;
            fLocaleBundle = ures_open(BUNDLE_NAME, XMLMsgLoader::getLocale(), &err);
            if (!U_SUCCESS(err) || fLocaleBundle == NULL)
            {
                 XMLPlatformUtils::panic(PanicHandler::Panic_CantLoadMsgDomain);
            }
        }
        else
        {
            XMLPlatformUtils::panic(PanicHandler::Panic_CantLoadMsgDomain);
        }
    }

    /***
	Open the domain specific resource bundle within
	the locale-specific resource bundle
    ***/
    err = U_ZERO_ERROR;
    fDomainBundle = ures_getByKey(fLocaleBundle, domainName, NULL, &err);
    if (!U_SUCCESS(err) || fDomainBundle == NULL)
    {
        XMLPlatformUtils::panic(PanicHandler::Panic_CantLoadMsgDomain);
    }
}

ICUMsgLoader::~ICUMsgLoader()
{
    ures_close(fDomainBundle);
    ures_close(fLocaleBundle);
}


// ---------------------------------------------------------------------------
//  Implementation of the virtual message loader API
// ---------------------------------------------------------------------------
bool ICUMsgLoader::loadMsg( const   XMLMsgLoader::XMLMsgId  msgToLoad
                          ,         XMLCh* const            toFill
                          , const   XMLSize_t               maxChars)
{
    UErrorCode   err = U_ZERO_ERROR;
    int32_t      strLen = 0;

    // Assuming array format
    const UChar *name = ures_getStringByIndex(fDomainBundle, (int32_t)msgToLoad-1, &strLen, &err);

    if (!U_SUCCESS(err) || (name == NULL))
    {
        return false;
    }

    int retStrLen = strLen > (int32_t)maxChars ? maxChars : strLen;

    if (sizeof(UChar)==sizeof(XMLCh))
    {
        XMLString::moveChars(toFill, (XMLCh*)name, retStrLen);
        toFill[retStrLen] = (XMLCh) 0;
    }
    else
    {
        XMLCh* retStr = toFill;
        const UChar *srcPtr = name;

        while (retStrLen--)
           *retStr++ = *srcPtr++;

        *retStr = 0;
    }

    return true;
}


bool ICUMsgLoader::loadMsg( const   XMLMsgLoader::XMLMsgId  msgToLoad
                            ,       XMLCh* const            toFill
                            , const XMLSize_t               maxChars
                            , const XMLCh* const            repText1
                            , const XMLCh* const            repText2
                            , const XMLCh* const            repText3
                            , const XMLCh* const            repText4
                            , MemoryManager* const          manager   )
{
    // Call the other version to load up the message
    if (!loadMsg(msgToLoad, toFill, maxChars))
        return false;

    // And do the token replacement
    XMLString::replaceTokens(toFill, maxChars, repText1, repText2, repText3, repText4, manager);
    return true;
}


bool ICUMsgLoader::loadMsg( const   XMLMsgLoader::XMLMsgId  msgToLoad
                            ,       XMLCh* const            toFill
                            , const XMLSize_t               maxChars
                            , const char* const             repText1
                            , const char* const             repText2
                            , const char* const             repText3
                            , const char* const             repText4
                            , MemoryManager * const         manager)
{
    //
    //  Transcode the provided parameters and call the other version,
    //  which will do the replacement work.
    //
    XMLCh* tmp1 = 0;
    XMLCh* tmp2 = 0;
    XMLCh* tmp3 = 0;
    XMLCh* tmp4 = 0;

    bool bRet = false;
    if (repText1)
        tmp1 = XMLString::transcode(repText1, manager);
    if (repText2)
        tmp2 = XMLString::transcode(repText2, manager);
    if (repText3)
        tmp3 = XMLString::transcode(repText3, manager);
    if (repText4)
        tmp4 = XMLString::transcode(repText4, manager);

    bRet = loadMsg(msgToLoad, toFill, maxChars, tmp1, tmp2, tmp3, tmp4, manager);

    if (tmp1)
        manager->deallocate(tmp1);//delete [] tmp1;
    if (tmp2)
        manager->deallocate(tmp2);//delete [] tmp2;
    if (tmp3)
        manager->deallocate(tmp3);//delete [] tmp3;
    if (tmp4)
        manager->deallocate(tmp4);//delete [] tmp4;

    return bRet;
}

XERCES_CPP_NAMESPACE_END
