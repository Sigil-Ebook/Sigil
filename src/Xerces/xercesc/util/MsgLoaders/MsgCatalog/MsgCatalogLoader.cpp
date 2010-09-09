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
 * $Id: MsgCatalogLoader.cpp 614259 2008-01-22 16:59:21Z amassari $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLMsgLoader.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/XMLUni.hpp>
#include "MsgCatalogLoader.hpp"
#include "XMLMsgCat_Ids.hpp"

#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Public Constructors and Destructor
// ---------------------------------------------------------------------------
MsgCatalogLoader::MsgCatalogLoader(const XMLCh* const msgDomain)
:fCatalogHandle(0)
,fMsgSet(0)
{
    if (!XMLString::equals(msgDomain, XMLUni::fgXMLErrDomain)
    &&  !XMLString::equals(msgDomain, XMLUni::fgExceptDomain)
    &&  !XMLString::equals(msgDomain, XMLUni::fgXMLDOMMsgDomain)
    &&  !XMLString::equals(msgDomain, XMLUni::fgValidityDomain))
    {
        XMLPlatformUtils::panic(PanicHandler::Panic_UnknownMsgDomain);
    }

    // Prepare the path info
    char locationBuf[1024];
    memset(locationBuf, 0, sizeof locationBuf);
    const char *nlsHome = XMLMsgLoader::getNLSHome();
    
    if (nlsHome)
    {
    	strcpy(locationBuf, nlsHome);
        strcat(locationBuf, "/");
    }
    else
    {
        nlsHome = getenv("XERCESC_NLS_HOME");
        if (nlsHome)
        {
            strcpy(locationBuf, nlsHome);
            strcat(locationBuf, "/");
        }
        else
        {
            nlsHome = getenv("XERCESCROOT");
            if (nlsHome)
            {                       	
                strcpy(locationBuf, nlsHome);
                strcat(locationBuf, "/msg/");
            }
        }    
    }
    
    // Prepare user-specified locale specific cat file
    char catuser[1024];
    memset(catuser, 0, sizeof catuser);
    strcpy(catuser, locationBuf);
    strcat(catuser, "XercesMessages_");
    strcat(catuser, XMLMsgLoader::getLocale());
    strcat(catuser, ".cat");
        
    char catdefault[1024];
    memset(catdefault, 0, sizeof catdefault);
    strcpy(catdefault, locationBuf);
    strcat(catdefault, "XercesMessages_en_US.cat");

   /**
    * To open user-specified locale specific cat file
    * and default cat file if necessary
    */
    if ( ((fCatalogHandle=catopen(catuser, 0)) == (nl_catd)-1) &&
         ((fCatalogHandle=catopen(catdefault, 0)) == (nl_catd)-1)   )
    {
        // Probably have to call panic here
        printf("Could not open catalog:\n %s\n or %s\n", catuser, catdefault);
        XMLPlatformUtils::panic(PanicHandler::Panic_CantLoadMsgDomain);
    }

    if (XMLString::equals(msgDomain, XMLUni::fgXMLErrDomain))
        fMsgSet = CatId_XMLErrs;
    else if (XMLString::equals(msgDomain, XMLUni::fgExceptDomain))
        fMsgSet = CatId_XMLExcepts;
    else if (XMLString::equals(msgDomain, XMLUni::fgValidityDomain))
        fMsgSet = CatId_XMLValid;
    else if (XMLString::equals(msgDomain, XMLUni::fgXMLDOMMsgDomain))
        fMsgSet = CatId_XMLDOMMsg;
}

MsgCatalogLoader::~MsgCatalogLoader()
{
    catclose(fCatalogHandle);	
}


// ---------------------------------------------------------------------------
//  Implementation of the virtual message loader API
// ---------------------------------------------------------------------------
bool MsgCatalogLoader::loadMsg(const  XMLMsgLoader::XMLMsgId  msgToLoad
                              ,       XMLCh*   const          toFill
                              , const XMLSize_t               maxChars)
{
    char msgString[100];
    sprintf(msgString, "Could not find message ID %d from message set %d\n", msgToLoad, fMsgSet);
    char* catMessage = catgets( fCatalogHandle, fMsgSet, (int)msgToLoad, msgString);

    // catgets returns a pointer to msgString if it fails to locate the message
    // from the message catalog
    if (XMLString::equals(catMessage, msgString))
        return false;
    else
    {
        XMLString::transcode(catMessage, toFill, maxChars);
        return true;
    }
	
}

bool MsgCatalogLoader::loadMsg(const  XMLMsgLoader::XMLMsgId  msgToLoad
                            ,       XMLCh* const            toFill
                            , const XMLSize_t               maxChars
                            , const XMLCh* const            repText1
                            , const XMLCh* const            repText2
                            , const XMLCh* const            repText3
                            , const XMLCh* const            repText4
                            , MemoryManager* const          manager)
{
    // Call the other version to load up the message
    if (!loadMsg(msgToLoad, toFill, maxChars))
        return false;

    // And do the token replacement
    XMLString::replaceTokens(toFill, maxChars, repText1, repText2, repText3, repText4, manager);
    return true;
}


bool MsgCatalogLoader::loadMsg(const  XMLMsgLoader::XMLMsgId  msgToLoad
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
