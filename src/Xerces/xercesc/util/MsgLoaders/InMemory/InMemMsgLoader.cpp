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
 * $Id: InMemMsgLoader.cpp 570552 2007-08-28 19:57:36Z amassari $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/BitOps.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLMsgLoader.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUni.hpp>
#include "InMemMsgLoader.hpp"
#include "XercesMessages_en_US.hpp"

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Public Constructors and Destructor
// ---------------------------------------------------------------------------
InMemMsgLoader::InMemMsgLoader(const XMLCh* const msgDomain)
:fMsgDomain(0)
{
    if (!XMLString::equals(msgDomain, XMLUni::fgXMLErrDomain)
    &&  !XMLString::equals(msgDomain, XMLUni::fgExceptDomain)
    &&  !XMLString::equals(msgDomain, XMLUni::fgXMLDOMMsgDomain)
    &&  !XMLString::equals(msgDomain, XMLUni::fgValidityDomain))
    {
        XMLPlatformUtils::panic(PanicHandler::Panic_UnknownMsgDomain);
    }

    fMsgDomain = XMLString::replicate(msgDomain, XMLPlatformUtils::fgMemoryManager);
}

InMemMsgLoader::~InMemMsgLoader()
{
    XMLPlatformUtils::fgMemoryManager->deallocate(fMsgDomain);//delete [] fMsgDomain;
}


// ---------------------------------------------------------------------------
//  Implementation of the virtual message loader API
// ---------------------------------------------------------------------------
bool InMemMsgLoader::loadMsg(const  XMLMsgLoader::XMLMsgId  msgToLoad
                            ,       XMLCh* const            toFill
                            , const XMLSize_t               maxChars)
{
    //
    //  Just use the id to map into the correct array of messages. Then
    //  copy that to the caller's buffer.
    //
    //  NOTE:   The source text is in little endian form. So, if we are a
    //          big endian machine, flip them in the process.
    //
    XMLCh* endPtr = toFill + maxChars;
    XMLCh* outPtr = toFill;
    const XMLCh* srcPtr = 0;

    if (XMLString::equals(fMsgDomain, XMLUni::fgXMLErrDomain))
    {
        if ( msgToLoad > gXMLErrArraySize)
            return false;
        else
            srcPtr = gXMLErrArray[msgToLoad - 1];
    }
     else if (XMLString::equals(fMsgDomain, XMLUni::fgExceptDomain))
    {
         if ( msgToLoad > gXMLExceptArraySize)
            return false;
         else
             srcPtr = gXMLExceptArray[msgToLoad - 1];
    }
     else if (XMLString::equals(fMsgDomain, XMLUni::fgValidityDomain))
    {
         if ( msgToLoad > gXMLValidityArraySize)
            return false;
         else
             srcPtr = gXMLValidityArray[msgToLoad - 1];
    }
     else if (XMLString::equals(fMsgDomain, XMLUni::fgXMLDOMMsgDomain))
    {
         if ( msgToLoad > gXMLDOMMsgArraySize)
            return false;
         else
             srcPtr = gXMLDOMMsgArray[msgToLoad - 1];
    }

     while (*srcPtr && (outPtr < endPtr))
     {
         *outPtr++ = *srcPtr++;
     }
     *outPtr = 0;

     return true;
}


bool InMemMsgLoader::loadMsg(const  XMLMsgLoader::XMLMsgId  msgToLoad
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


bool InMemMsgLoader::loadMsg(const  XMLMsgLoader::XMLMsgId  msgToLoad
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
