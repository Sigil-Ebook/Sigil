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
 * $Id: ValueStoreCache.cpp 708224 2008-10-27 16:02:26Z amassari $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/schema/identity/ValueStoreCache.hpp>
#include <xercesc/validators/schema/identity/ValueStore.hpp>
#include <xercesc/validators/schema/SchemaElementDecl.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

typedef JanitorMemFunCall<ValueStoreCache>    CleanupType;

// ---------------------------------------------------------------------------
//  ValueStoreCache: Constructors and Destructor
// ---------------------------------------------------------------------------
ValueStoreCache::ValueStoreCache(MemoryManager* const manager)
    : fValueStores(0)
    , fGlobalICMap(0)
    , fIC2ValueStoreMap(0)
    , fGlobalMapStack(0)
    , fScanner(0)
    , fMemoryManager(manager)
{
    CleanupType cleanup(this, &ValueStoreCache::cleanUp);

    try {
        init();
    }
    catch(const OutOfMemoryException&)
    {
        cleanup.release();

        throw;
    }

    cleanup.release();
}


ValueStoreCache::~ValueStoreCache()
{
    cleanUp();
}

// ---------------------------------------------------------------------------
//  ValueStoreCache: Document handling methods
// ---------------------------------------------------------------------------
void ValueStoreCache::startDocument() {

    fIC2ValueStoreMap->removeAll();
    fGlobalICMap->removeAll();
    fValueStores->removeAllElements();
    fGlobalMapStack->removeAllElements();
}

void ValueStoreCache::startElement() {

    fGlobalMapStack->push(fGlobalICMap);
    fGlobalICMap = new (fMemoryManager) RefHashTableOf<ValueStore, PtrHasher>
    (
        13
        , false
        , fMemoryManager
    );
}

void ValueStoreCache::endElement() {

    if (fGlobalMapStack->empty()) {
        return; // must be an invalid doc!
    }

    RefHashTableOf<ValueStore, PtrHasher>* oldMap = fGlobalMapStack->pop();
    RefHashTableOfEnumerator<ValueStore, PtrHasher> mapEnum(oldMap, false, fMemoryManager);
//    Janitor<RefHashTableOf<ValueStore> > janMap(oldMap);

    while (mapEnum.hasMoreElements()) {

        ValueStore& oldVal = mapEnum.nextElement();
        IdentityConstraint* ic = oldVal.getIdentityConstraint();
        ValueStore* currVal = fGlobalICMap->get(ic);

        if (!currVal) {
            fGlobalICMap->put(ic, &oldVal);
        }
        else {
            currVal->append(&oldVal);
        }
    }
    delete oldMap;
}

// ---------------------------------------------------------------------------
//  ValueStoreCache: Helper methods
// ---------------------------------------------------------------------------
void ValueStoreCache::cleanUp() {

    delete fIC2ValueStoreMap;
    delete fGlobalICMap;
    delete fGlobalMapStack;
    delete fValueStores;
}

void ValueStoreCache::init() {

    fValueStores = new (fMemoryManager) RefVectorOf<ValueStore>(8, false, fMemoryManager);
    fGlobalICMap = new (fMemoryManager) RefHashTableOf<ValueStore, PtrHasher>
    (
        13
        , false
        , fMemoryManager
    );
    fIC2ValueStoreMap = new (fMemoryManager) RefHash2KeysTableOf<ValueStore, PtrHasher>
    (
        13
        , true
        , fMemoryManager
    );
    fGlobalMapStack = new (fMemoryManager) RefStackOf<RefHashTableOf<ValueStore, PtrHasher> >(8, true, fMemoryManager);
}

void ValueStoreCache::initValueStoresFor(SchemaElementDecl* const elemDecl,
                                         const int initialDepth) {

    // initialize value stores for unique fields
    XMLSize_t icCount = elemDecl->getIdentityConstraintCount();

    for (XMLSize_t i=0; i<icCount; i++) {

        IdentityConstraint* ic = elemDecl->getIdentityConstraintAt(i);
        ValueStore* valueStore=fIC2ValueStoreMap->get(ic, initialDepth);
        if(valueStore==0)
        {
            valueStore = new (fMemoryManager) ValueStore(ic, fScanner, fMemoryManager);
            fIC2ValueStoreMap->put(ic, initialDepth, valueStore);
        }
        else
            valueStore->clear();
        fValueStores->addElement(valueStore);
    }
}

void ValueStoreCache::transplant(IdentityConstraint* const ic, const int initialDepth) {

    if (ic->getType() == IdentityConstraint::ICType_KEYREF) {
        return;
    }

    ValueStore* newVals = fIC2ValueStoreMap->get(ic, initialDepth);
    ValueStore* currVals = fGlobalICMap->get(ic);

    if (currVals) {
        currVals->append(newVals);
    } else {
        fGlobalICMap->put(ic, newVals);
    }
}

XERCES_CPP_NAMESPACE_END

/**
  * End of file ValueStoreCache.cpp
  */
