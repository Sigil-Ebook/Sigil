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
 * $Id: DOMImplementationRegistry.cpp 676911 2008-07-15 13:27:32Z amassari $
 */

#include <xercesc/util/Mutexes.hpp>
#include <xercesc/util/RefVectorOf.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLInitializer.hpp>
#include <xercesc/dom/DOMImplementationRegistry.hpp>
#include <xercesc/dom/DOMImplementationSource.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include "DOMImplementationImpl.hpp"
#include "DOMImplementationListImpl.hpp"

XERCES_CPP_NAMESPACE_BEGIN

// Points to the singleton instance of a registry of DOMImplementationSource.
//
static RefVectorOf<DOMImplementationSource>* gDOMImplSrcVector = 0;

//  Global mutex that is used to synchronize access to the vector.
//
static XMLMutex* gDOMImplSrcVectorMutex = 0;

void XMLInitializer::initializeDOMImplementationRegistry()
{
    gDOMImplSrcVectorMutex = new XMLMutex(XMLPlatformUtils::fgMemoryManager);
    gDOMImplSrcVector = new RefVectorOf<DOMImplementationSource>(3, false);
}

void XMLInitializer::terminateDOMImplementationRegistry()
{
    delete gDOMImplSrcVector;
    gDOMImplSrcVector = 0;

    delete gDOMImplSrcVectorMutex;
    gDOMImplSrcVectorMutex = 0;
}

// -----------------------------------------------------------------------
//  DOMImplementationRegistry Functions
// -----------------------------------------------------------------------
DOMImplementation *DOMImplementationRegistry::getDOMImplementation(const XMLCh* features) {

    XMLMutexLock lock(gDOMImplSrcVectorMutex);

    XMLSize_t len = gDOMImplSrcVector->size();

    // Put our defined source there
    if (len == 0) {
        gDOMImplSrcVector->addElement((DOMImplementationSource*)DOMImplementationImpl::getDOMImplementationImpl());

        len = gDOMImplSrcVector->size();
    }

    for (XMLSize_t i = len; i > 0; i--) {
        DOMImplementationSource* source = gDOMImplSrcVector->elementAt(i-1);
        DOMImplementation* impl = source->getDOMImplementation(features);
        if (impl)
            return impl;
    }

    return 0;
}

DOMImplementationList* DOMImplementationRegistry::getDOMImplementationList(const XMLCh* features) {

    DOMImplementationListImpl* list = new DOMImplementationListImpl;

    XMLMutexLock lock(gDOMImplSrcVectorMutex);

    XMLSize_t len = gDOMImplSrcVector->size();

    // Put our defined source there
    if (len == 0)
        gDOMImplSrcVector->addElement((DOMImplementationSource*)DOMImplementationImpl::getDOMImplementationImpl());

    len = gDOMImplSrcVector->size();

    for (XMLSize_t i = len; i > 0; i--) {
        DOMImplementationSource* source = gDOMImplSrcVector->elementAt(i-1);
        DOMImplementationList* oneList = source->getDOMImplementationList(features);
        XMLSize_t oneListLen=oneList->getLength();
        for(XMLSize_t j=0; j<oneListLen; j++)
            list->add(oneList->item(j));
        oneList->release();
    }

    return list;
}

void DOMImplementationRegistry::addSource (DOMImplementationSource* source)
{
    XMLMutexLock lock(gDOMImplSrcVectorMutex);
    gDOMImplSrcVector->addElement(source);
}


XERCES_CPP_NAMESPACE_END

