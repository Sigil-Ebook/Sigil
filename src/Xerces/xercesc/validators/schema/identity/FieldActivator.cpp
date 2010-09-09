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
 * $Id: FieldActivator.cpp 679340 2008-07-24 10:28:29Z borisk $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/schema/identity/FieldActivator.hpp>
#include <xercesc/validators/schema/identity/ValueStore.hpp>
#include <xercesc/validators/schema/identity/ValueStoreCache.hpp>
#include <xercesc/validators/schema/identity/XPathMatcherStack.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  FieldActivator: Constructors and Destructor
// ---------------------------------------------------------------------------
FieldActivator::FieldActivator(ValueStoreCache* const valueStoreCache,
                               XPathMatcherStack* const matcherStack,
                               MemoryManager* const manager)
    : fValueStoreCache(valueStoreCache)
    , fMatcherStack(matcherStack)
    , fMayMatch(0)
    , fMemoryManager(manager)
{
    fMayMatch = new (manager) ValueHashTableOf<bool, PtrHasher>(29, manager);
}

FieldActivator::FieldActivator(const FieldActivator& other)
    : XMemory(other)
    , fValueStoreCache(other.fValueStoreCache)
    , fMatcherStack(other.fMatcherStack)
    , fMayMatch(0)
    , fMemoryManager(other.fMemoryManager)
{
    fMayMatch = new (fMemoryManager) ValueHashTableOf<bool, PtrHasher>(29, fMemoryManager);
    ValueHashTableOfEnumerator<bool, PtrHasher> mayMatchEnum(other.fMayMatch, false, fMemoryManager);

    // Build key set
    while (mayMatchEnum.hasMoreElements())
    {
        IC_Field* field = (IC_Field*) mayMatchEnum.nextElementKey();
        fMayMatch->put(field, other.fMayMatch->get(field));
    }
}


FieldActivator::~FieldActivator()
{
    delete fMayMatch;
}

// ---------------------------------------------------------------------------
//  FieldActivator: Operator methods
// ---------------------------------------------------------------------------
FieldActivator& FieldActivator::operator =(const FieldActivator& other) {

    if (this == &other) {
        return *this;
    }

    fValueStoreCache = other.fValueStoreCache;
    fMatcherStack = other.fMatcherStack;
    return *this;
}

// ---------------------------------------------------------------------------
//  FieldActivator: Operator methods
// ---------------------------------------------------------------------------
XPathMatcher* FieldActivator::activateField(IC_Field* const field, const int initialDepth) {

    ValueStore* valueStore = fValueStoreCache->getValueStoreFor(field, initialDepth);
    XPathMatcher* matcher = field->createMatcher(this, valueStore, fMemoryManager);

    setMayMatch(field, true);
    fMatcherStack->addMatcher(matcher);
    matcher->startDocumentFragment();

    return matcher;
}

void FieldActivator::startValueScopeFor(const IdentityConstraint* const ic,
                                        const int initialDepth) {

    XMLSize_t fieldCount = ic->getFieldCount();

    for(XMLSize_t i=0; i<fieldCount; i++) {

        const IC_Field* field = ic->getFieldAt(i);
        ValueStore* valueStore = fValueStoreCache->getValueStoreFor(field, initialDepth);

        valueStore->startValueScope();
    }
}

void FieldActivator::endValueScopeFor(const IdentityConstraint* const ic, const int initialDepth) {

    ValueStore* valueStore = fValueStoreCache->getValueStoreFor(ic, initialDepth);

    valueStore->endValueScope();
}

XERCES_CPP_NAMESPACE_END

/**
  * End of file FieldActivator.cpp
  */
