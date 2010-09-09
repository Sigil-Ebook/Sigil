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
 *  $Id: XMLMsgLoader.cpp 482395 2006-12-04 22:44:40Z dbertoni $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/XMLMsgLoader.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUniDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/***
 *   The PlatformUtils::initialize() would set fLocale
 *   to either a user-provided string or 0
 *
 ***/
char* XMLMsgLoader::fLocale = 0;

char* XMLMsgLoader::fPath = 0;

/***
 *  if localeToAdopt is 0, that is to release memory for
 *  the user defined locale string
 *
 ***/
void  XMLMsgLoader::setLocale(const char* const localeToAdopt)
{
    /***
     * Release the current setting's memory, if any
     ***/
    if (fLocale)
    {
        XMLPlatformUtils::fgMemoryManager->deallocate(fLocale);//delete [] fLocale;
        fLocale = 0;
    }

    /***
     *  
     *  REVISIT: later we may do locale format checking
     * 
     *           refer to phttp://oss.software.ibm.com/icu/userguide/locale.html
     *           for details.
     */
    if (localeToAdopt && (strlen(localeToAdopt) == 2 || (strlen(localeToAdopt) > 3 && localeToAdopt[2]=='_')))
    {
        fLocale   = XMLString::replicate(localeToAdopt, XMLPlatformUtils::fgMemoryManager);                   
    }

}

const char* XMLMsgLoader::getLocale()
{
    return fLocale;
}

/***
 *  if nlsHomeToAdopt is 0, that is to release memory for
 *  the user defined NLSHome string
 *
 ***/
void  XMLMsgLoader::setNLSHome(const char* const nlsHomeToAdopt)
{
    /***
     * Release the current setting's memory, if any
     ***/
    if (fPath)
    {
        XMLPlatformUtils::fgMemoryManager->deallocate(fPath);//delete [] fPath;
        fPath = 0;
    }

    if (nlsHomeToAdopt)
    {
        fPath = XMLString::replicate(nlsHomeToAdopt, XMLPlatformUtils::fgMemoryManager);
    }

}

const char* XMLMsgLoader::getNLSHome()
{
    return fPath;
}

XERCES_CPP_NAMESPACE_END
