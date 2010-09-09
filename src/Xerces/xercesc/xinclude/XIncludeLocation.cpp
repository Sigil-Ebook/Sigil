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
 * $Id: XIncludeLocation.cpp 932949 2010-04-11 17:40:33Z borisk $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/xinclude/XIncludeLocation.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XMLUniDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN

const XMLCh *allocate(const XMLCh *href){
    XMLCh *allocated;
    XMLSize_t length = XMLString::stringLen(href);
    allocated = (XMLCh*)XMLPlatformUtils::fgMemoryManager->allocate((length+1) * sizeof(XMLCh));
    XMLString::copyString(allocated, href);
    XMLPlatformUtils::removeDotDotSlash(allocated);

    return allocated;
}

void deallocate(void *ptr){
    if (ptr)
        XMLPlatformUtils::fgMemoryManager->deallocate((void *)ptr);
}

// ---------------------------------------------------------------------------
//  Destructor and Constructor
// ---------------------------------------------------------------------------
XIncludeLocation::XIncludeLocation(const XMLCh *href){
    fHref = allocate(href);
}

XIncludeLocation::~XIncludeLocation(){
    deallocate((void *)fHref);
}

const XMLCh *
XIncludeLocation::prependPath(const XMLCh *baseToAdd){
    XMLCh *relativeHref = NULL;
    if (fHref == NULL){
        return NULL;
    }

    if (baseToAdd == NULL){
        return fHref;
    }

    XMLPlatformUtils::removeDotDotSlash((XMLCh*)baseToAdd);
    XMLSize_t baseLength = XMLString::stringLen(baseToAdd);

    int lastSlash = XMLString::lastIndexOf(baseToAdd, chForwardSlash);
    if (lastSlash == -1){
        /* not found, try another platform */
        lastSlash = XMLString::lastIndexOf(baseToAdd, chBackSlash);
    }

    // Skip the scheme (e.g., file://) if fHref has one. Ideally we
    // should detect also if the URI is absolute.
    //
    const XMLCh* hrefPath = findEndOfProtocol (fHref);
    XMLSize_t hrefPathLength = XMLString::stringLen(hrefPath);

    relativeHref = (XMLCh *)XMLPlatformUtils::fgMemoryManager->allocate((hrefPathLength + baseLength + 2) * sizeof(XMLCh));
    if (relativeHref == NULL){
        return NULL;
    }
    XMLString::copyNString(relativeHref, baseToAdd, lastSlash + 1);
    relativeHref[lastSlash + 1] = chNull;
    XMLString::catString(relativeHref, hrefPath);

    /* free the old reference */
    deallocate((void *)fHref);

    fHref = relativeHref;
    return fHref;
}

const XMLCh *
XIncludeLocation::findEndOfProtocol(const XMLCh *URI){
    if ( URI[0] == chLatin_f &&
        URI[1] == chLatin_i &&
        URI[2] == chLatin_l &&
        URI[3] == chLatin_e &&
        URI[4] == chColon &&
        URI[5] == chForwardSlash &&
        URI[6] == chForwardSlash &&
        URI[7] == chForwardSlash )
    {
        return URI + 8;
    }

    if ( URI[0] == chLatin_f &&
        URI[1] == chLatin_t &&
        URI[2] == chLatin_p &&
        URI[3] == chColon &&
        URI[4] == chForwardSlash &&
        URI[5] == chForwardSlash &&
        URI[6] == chForwardSlash )
    {
        return URI + 7;
    }

    if ( URI[0] == chLatin_h &&
        URI[1] == chLatin_t &&
        URI[2] == chLatin_t &&
        URI[3] == chLatin_p &&
        URI[4] == chColon &&
        URI[5] == chForwardSlash &&
        URI[6] == chForwardSlash &&
        URI[7] == chForwardSlash )
    {
        return URI + 8;
    }

    /* if method fails, simply return the URI and let the problem be detected
     * and reported down the line (it may not have a protocol of course) */
    return URI;
}

XERCES_CPP_NAMESPACE_END
