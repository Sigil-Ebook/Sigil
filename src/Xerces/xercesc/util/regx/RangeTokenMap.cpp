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
 * $Id: RangeTokenMap.cpp 678879 2008-07-22 20:05:05Z amassari $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/regx/RangeTokenMap.hpp>
#include <xercesc/util/regx/RangeToken.hpp>
#include <xercesc/util/regx/RegxDefs.hpp>
#include <xercesc/util/regx/TokenFactory.hpp>
#include <xercesc/util/regx/XMLRangeFactory.hpp>
#include <xercesc/util/regx/ASCIIRangeFactory.hpp>
#include <xercesc/util/regx/UnicodeRangeFactory.hpp>
#include <xercesc/util/regx/BlockRangeFactory.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLExceptMsgs.hpp>
#include <xercesc/util/StringPool.hpp>
#include <xercesc/util/XMLInitializer.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

RangeTokenMap* RangeTokenMap::fInstance = 0;

void XMLInitializer::initializeRangeTokenMap()
{
    RangeTokenMap::fInstance = new RangeTokenMap(
      XMLPlatformUtils::fgMemoryManager);

    if (RangeTokenMap::fInstance)
      RangeTokenMap::fInstance->buildTokenRanges();
}

void XMLInitializer::terminateRangeTokenMap()
{
    delete RangeTokenMap::fInstance;
    RangeTokenMap::fInstance = 0;
}


// ---------------------------------------------------------------------------
//  RangeTokenElemMap: Constructors and Destructor
// ---------------------------------------------------------------------------
RangeTokenElemMap::RangeTokenElemMap(unsigned int categoryId) :
    fCategoryId(categoryId)
    , fRange(0)
    , fNRange(0)
{

}

RangeTokenElemMap::~RangeTokenElemMap()
{

}

// ---------------------------------------------------------------------------
//  RangeTokenMap: Constructors and Destructor
// ---------------------------------------------------------------------------

typedef JanitorMemFunCall<RangeTokenMap>    CleanupType;

RangeTokenMap::RangeTokenMap(MemoryManager* manager) :
    fTokenRegistry(0)
    , fRangeMap(0)
    , fCategories(0)
    , fTokenFactory(0)
    , fMutex(manager)
{
    CleanupType cleanup(this, &RangeTokenMap::cleanUp);

    try {
        fTokenRegistry = new (manager) RefHashTableOf<RangeTokenElemMap>(109, manager);
        fRangeMap = new (manager) RefHashTableOf<RangeFactory>(29, manager);
        fCategories = new (manager) XMLStringPool(109, manager);
        fTokenFactory = new (manager) TokenFactory(manager);
        initializeRegistry();
    }
    catch(const OutOfMemoryException&)
    {
        cleanup.release();

        throw;
    }

    cleanup.release();
}

RangeTokenMap::~RangeTokenMap() {

    cleanUp();
}

// ---------------------------------------------------------------------------
//  RangeTokenMap: Getter methods
// ---------------------------------------------------------------------------
RangeToken* RangeTokenMap::getRange(const XMLCh* const keyword,
                                    const bool complement) {

    if (!fTokenRegistry->containsKey(keyword))
        return 0;

    RangeTokenElemMap* elemMap = fTokenRegistry->get(keyword);
    RangeToken* rangeTok = elemMap->getRangeToken(complement);

    if (!rangeTok)
    {
        XMLMutexLock lockInit(&fMutex);

        // make sure that it was not created while we were locked
        rangeTok = elemMap->getRangeToken(complement);

        if (!rangeTok)
        {
            unsigned int categId = elemMap->getCategoryId();
            const XMLCh* categName = fCategories->getValueForId(categId);
            RangeFactory* rangeFactory = fRangeMap->get(categName);

            if (rangeFactory)
            {
                rangeFactory->buildRanges(this);
                rangeTok = elemMap->getRangeToken(complement);

                // see if we are complementing an existing range
                if (!rangeTok && complement)
                {
                    rangeTok = elemMap->getRangeToken();
                    if (rangeTok)
                    {
                        rangeTok = RangeToken::complementRanges(rangeTok, fTokenFactory, fTokenRegistry->getMemoryManager());
                        elemMap->setRangeToken(rangeTok , complement);
                    }
                }
            }
        }
    }

    return rangeTok;
}


// ---------------------------------------------------------------------------
//  RangeTokenMap: Putter methods
// ---------------------------------------------------------------------------
void RangeTokenMap::addCategory(const XMLCh* const categoryName) {

    fCategories->addOrFind(categoryName);
}

void RangeTokenMap::addRangeMap(const XMLCh* const categoryName,
                                RangeFactory* const rangeFactory) {

    fRangeMap->put((void*)categoryName, rangeFactory);
}

void RangeTokenMap::addKeywordMap(const XMLCh* const keyword,
                                 const XMLCh* const categoryName) {

    unsigned int categId = fCategories->getId(categoryName);

    if (categId == 0) {
        ThrowXMLwithMemMgr1(RuntimeException, XMLExcepts::Regex_InvalidCategoryName, categoryName, fTokenRegistry->getMemoryManager());
    }

    if (fTokenRegistry->containsKey(keyword)) {

        RangeTokenElemMap* elemMap = fTokenRegistry->get(keyword);

        if (elemMap->getCategoryId() != categId)
            elemMap->setCategoryId(categId);

        return;
    }

    fTokenRegistry->put((void*) keyword, new RangeTokenElemMap(categId));
}

// ---------------------------------------------------------------------------
//  RangeTokenMap: Setter methods
// ---------------------------------------------------------------------------
void RangeTokenMap::setRangeToken(const XMLCh* const keyword,
                                  RangeToken* const tok,const bool complement) {

    if (fTokenRegistry->containsKey(keyword)) {
        fTokenRegistry->get(keyword)->setRangeToken(tok, complement);
    }
    else {
        ThrowXMLwithMemMgr1(RuntimeException, XMLExcepts::Regex_KeywordNotFound, keyword, fTokenRegistry->getMemoryManager());
    }
}


// ---------------------------------------------------------------------------
//  RangeTokenMap: Initialization methods
// ---------------------------------------------------------------------------
void RangeTokenMap::initializeRegistry() {

    // Add categories
    fCategories->addOrFind(fgXMLCategory);
    fCategories->addOrFind(fgASCIICategory);
    fCategories->addOrFind(fgUnicodeCategory);
    fCategories->addOrFind(fgBlockCategory);

    // Add xml range factory
    RangeFactory* rangeFact = new XMLRangeFactory();
    fRangeMap->put((void*)fgXMLCategory, rangeFact);
    rangeFact->initializeKeywordMap(this);

    // Add ascii range factory
    rangeFact = new ASCIIRangeFactory();
    fRangeMap->put((void*)fgASCIICategory, rangeFact);
    rangeFact->initializeKeywordMap(this);

    // Add unicode range factory
    rangeFact = new UnicodeRangeFactory();
    fRangeMap->put((void*)fgUnicodeCategory, rangeFact);
    rangeFact->initializeKeywordMap(this);

    // Add block range factory
    rangeFact = new BlockRangeFactory();
    fRangeMap->put((void*)fgBlockCategory, rangeFact);
    rangeFact->initializeKeywordMap(this);
}

void RangeTokenMap::buildTokenRanges()
{
    // Build ranges */
    RangeFactory* rangeFactory = fRangeMap->get(fgXMLCategory);
    rangeFactory->buildRanges(this);

    rangeFactory = fRangeMap->get(fgASCIICategory);
    rangeFactory->buildRanges(this);

    rangeFactory = fRangeMap->get(fgUnicodeCategory);
    rangeFactory->buildRanges(this);

    rangeFactory = fRangeMap->get(fgBlockCategory);
    rangeFactory->buildRanges(this);
}

// ---------------------------------------------------------------------------
//  RangeTokenMap: Instance methods
// ---------------------------------------------------------------------------
RangeTokenMap* RangeTokenMap::instance()
{
    return fInstance;
}

// ---------------------------------------------------------------------------
//  RangeTokenMap: helper methods
// ---------------------------------------------------------------------------
void RangeTokenMap::cleanUp()
{
    delete fTokenRegistry;
    fTokenRegistry = 0;

    delete fRangeMap;
    fRangeMap = 0;

    delete fCategories;
    fCategories = 0;

    delete fTokenFactory;
    fTokenFactory = 0;
}

XERCES_CPP_NAMESPACE_END

/**
  * End of file RangeTokenMap.cpp
  */
