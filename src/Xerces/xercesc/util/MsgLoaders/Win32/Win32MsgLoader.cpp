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
 * $Id: Win32MsgLoader.cpp 570552 2007-08-28 19:57:36Z amassari $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <windows.h>

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLMsgLoader.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUni.hpp>
#include "Win32MsgLoader.hpp"


//	Function prototypes
BOOL APIENTRY DllMain(HINSTANCE hModule,
                             DWORD  ul_reason_for_call,
                             LPVOID lpReserved);


// ---------------------------------------------------------------------------
//  Public Constructors and Destructor
// ---------------------------------------------------------------------------
HINSTANCE globalModuleHandle;

BOOL APIENTRY DllMain(HINSTANCE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID /*lpReserved*/)
{
  switch (ul_reason_for_call) {
  case DLL_PROCESS_ATTACH:
    globalModuleHandle = hModule;
    break;
  case DLL_THREAD_ATTACH:
    break;
  case DLL_THREAD_DETACH:
    break;
  case DLL_PROCESS_DETACH:
    break;
  }
  return TRUE;
}

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Global module handle
// ---------------------------------------------------------------------------
Win32MsgLoader::Win32MsgLoader(const XMLCh* const msgDomain) :

    fDomainOfs(0)
    , fModHandle(0)
    , fMsgDomain(0)
{
    // Try to get the module handle
    fModHandle = globalModuleHandle;
    if (!fModHandle)
    {
        //
        //  If we didn't find it, its probably because its a development
        //  build which is built as separate DLLs, so lets look for the DLL
        //  that we are part of.
        //
        static const char* const privDLLName = "IXUTIL";
        fModHandle = ::GetModuleHandleA(privDLLName);

        // If neither exists, then we give up
        if (!fModHandle)
        {
            // Probably have to call panic here
        }
    }

    // Store the domain name
    fMsgDomain = XMLString::replicate(msgDomain, XMLPlatformUtils::fgMemoryManager);

    // And precalc the id offset we use for this domain
    if (XMLString::equals(fMsgDomain, XMLUni::fgXMLErrDomain))
        fDomainOfs = 0;
    else if (XMLString::equals(fMsgDomain, XMLUni::fgExceptDomain))
        fDomainOfs = 0x2000;
    else if (XMLString::equals(fMsgDomain, XMLUni::fgValidityDomain))
        fDomainOfs = 0x4000;
    else if (XMLString::equals(fMsgDomain, XMLUni::fgXMLDOMMsgDomain))
        fDomainOfs = 0x6000;
    else
        XMLPlatformUtils::panic(PanicHandler::Panic_UnknownMsgDomain);
}

Win32MsgLoader::~Win32MsgLoader()
{
    XMLPlatformUtils::fgMemoryManager->deallocate(fMsgDomain);//delete [] fMsgDomain;
}


// ---------------------------------------------------------------------------
//  Implementation of the virtual message loader API
// ---------------------------------------------------------------------------

//
//  This is the method that actually does the work of loading a message from
//  the attached resources. Note that we don't use LoadStringW here, since it
//  won't work on Win98. So we go the next level down and do what LoadStringW
//  would have done, since this will work on either platform.
//
bool Win32MsgLoader::loadMsg(const  XMLMsgLoader::XMLMsgId  msgToLoad
                            ,       XMLCh* const            toFill
                            , const XMLSize_t               maxChars)
{
    // In case we error return, and they don't check it...
    toFill[0] = 0;

    // Adjust the message id by the domain offset
    const unsigned int theMsgId = msgToLoad + fDomainOfs;

    //
    //  Figure out the actual id the id, adjusting it by the domain offset.
    //  Then first we calculate the particular 16 string block that this id
    //  is in, and the offset within that block of the string in question.
    //
    const unsigned int theBlock = (theMsgId >> 4) + 1;
    const unsigned int theOfs   = theMsgId & 0x000F;

    // Try to find this resource. If we fail to find it, return false
    HRSRC hMsgRsc = ::FindResourceEx
    (
        fModHandle
        , RT_STRING
        , MAKEINTRESOURCE(theBlock)
        , MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL)
    );
    if (!hMsgRsc)
        return false;

    // We found it, so load the block. If this fails, also return a false
    HGLOBAL hGbl = ::LoadResource(fModHandle, hMsgRsc);
    if (!hGbl)
        return false;

    // Lock this resource into memory. Again, if it fails, just return false
    const XMLCh* pBlock = (const XMLCh*)::LockResource(hGbl);
    if (!pBlock)
        return false;

    //
    //  Look through the block for our desired message. Its stored such that
    //  the zeroth entry has the length minus the separator null.
    //
    for (unsigned int index = 0; index < theOfs; index++)
        pBlock += *pBlock + 1;

    // Calculate how many actual chars we will end up with
    const XMLSize_t actualChars = ((maxChars < (XMLSize_t)*pBlock) ? maxChars : (XMLSize_t)*pBlock);

    // Ok, finally now copy as much as we can into the caller's buffer
    wcsncpy(toFill, pBlock + 1, actualChars);
    toFill[actualChars] = 0;

    return true;
}


bool Win32MsgLoader::loadMsg(const  XMLMsgLoader::XMLMsgId  msgToLoad
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


bool Win32MsgLoader::loadMsg(const  XMLMsgLoader::XMLMsgId  msgToLoad
                            ,       XMLCh* const            toFill
                            , const XMLSize_t               maxChars
                            , const char* const             repText1
                            , const char* const             repText2
                            , const char* const             repText3
                            , const char* const             repText4
                            , MemoryManager* const          manager)
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
