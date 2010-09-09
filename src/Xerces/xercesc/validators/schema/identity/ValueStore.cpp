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
 * $Id: ValueStore.cpp 804209 2009-08-14 13:15:05Z amassari $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/internal/XMLScanner.hpp>
#include <xercesc/framework/XMLValidator.hpp>
#include <xercesc/validators/datatype/DatatypeValidator.hpp>
#include <xercesc/validators/schema/identity/FieldActivator.hpp>
#include <xercesc/validators/schema/identity/ValueStore.hpp>
#include <xercesc/validators/schema/identity/IC_Field.hpp>
#include <xercesc/validators/schema/identity/IC_KeyRef.hpp>
#include <xercesc/validators/schema/identity/ValueStoreCache.hpp>

XERCES_CPP_NAMESPACE_BEGIN

//
// ---------------------------------------------------------------------------
// ICValueHasher: the hasher for identity constraints values
// ---------------------------------------------------------------------------
XMLSize_t ICValueHasher::getHashVal(const void* key, XMLSize_t mod) const
{
    const FieldValueMap* valueMap=(const FieldValueMap*)key;
    XMLSize_t hashVal = 0;

    XMLSize_t size = valueMap->size();
    for (XMLSize_t j=0; j<size; j++) {
        // reach the most generic datatype validator
        DatatypeValidator* dv = valueMap->getDatatypeValidatorAt(j);
        while(dv && dv->getBaseValidator())
            dv = dv->getBaseValidator();
        const XMLCh* const val = valueMap->getValueAt(j);
        const XMLCh* canonVal = (dv && val)?dv->getCanonicalRepresentation(val, fMemoryManager):0;
        if(canonVal)
        {
            hashVal += XMLString::hash(canonVal, mod);
            fMemoryManager->deallocate((void*)canonVal);
        }
        else if(val)
            hashVal += XMLString::hash(val, mod);
    }

    return hashVal % mod;
}

bool ICValueHasher::equals(const void *const key1, const void *const key2) const
{
    const FieldValueMap* left=(const FieldValueMap*)key1;
    const FieldValueMap* right=(const FieldValueMap*)key2;

    XMLSize_t lSize = left->size();
    XMLSize_t rSize = right->size();
    if (lSize == rSize) 
    {
        bool matchFound = true;

        for (XMLSize_t j=0; j<rSize; j++) {
            if (!isDuplicateOf(left->getDatatypeValidatorAt(j), left->getValueAt(j),
                               right->getDatatypeValidatorAt(j), right->getValueAt(j))) {
                matchFound = false;
                break;
            }
        }

        if (matchFound) { // found it
            return true;
        }
    }
    return false;
}

bool ICValueHasher::isDuplicateOf(DatatypeValidator* const dv1, const XMLCh* const val1,
                                  DatatypeValidator* const dv2, const XMLCh* const val2) const 
{

    // if either validator's null, fall back on string comparison
    if(!dv1 || !dv2) {
        return (XMLString::equals(val1, val2));
    }

    bool val1IsEmpty = (val1==0 || *val1==0);
    bool val2IsEmpty = (val2==0 || *val2==0);

    if (val1IsEmpty && val2IsEmpty) {

        if (dv1 == dv2) {
            return true;
        }

        return false;
    }

    if (val1IsEmpty || val2IsEmpty) {
        return false;
    }

    // find the common ancestor, if there is one
    DatatypeValidator* tempVal1 = dv1;
    while(tempVal1)
    {
        DatatypeValidator* tempVal2 = dv2;
        for(; tempVal2 != NULL && tempVal2 != tempVal1; tempVal2 = tempVal2->getBaseValidator()) ;
        if (tempVal2) 
            return ((tempVal2->compare(val1, val2, fMemoryManager)) == 0);
        tempVal1=tempVal1->getBaseValidator();
    }

    // if we're here it means the types weren't related. They are different:
    return false;
}

// ---------------------------------------------------------------------------
//  ValueStore: Constructors and Destructor
// ---------------------------------------------------------------------------
ValueStore::ValueStore(IdentityConstraint* const ic,
                       XMLScanner* const scanner,
                       MemoryManager* const manager)
    : fDoReportError(false)
    , fValuesCount(0)
    , fIdentityConstraint(ic)
    , fValues(manager)
    , fValueTuples(0)
    , fScanner(scanner)
    , fMemoryManager(manager)
{
    fDoReportError = (scanner && (scanner->getValidationScheme() == XMLScanner::Val_Always));
}


ValueStore::~ValueStore()
{
    delete fValueTuples;
}

// ---------------------------------------------------------------------------
//  ValueStore: Helper methods
// ---------------------------------------------------------------------------
void ValueStore::addValue(FieldActivator* const fieldActivator,
                          IC_Field* const field,
                          DatatypeValidator* const dv,
                          const XMLCh* const value) {

    if (!fieldActivator->getMayMatch(field) && fDoReportError) {
        fScanner->getValidator()->emitError(XMLValid::IC_FieldMultipleMatch);
    }

    // do we even know this field?
    XMLSize_t index;
    bool bFound = fValues.indexOf(field, index);

    if (!bFound) {

        if (fDoReportError) {
           fScanner->getValidator()->emitError(XMLValid::IC_UnknownField);
        }

        return;
    }

    // store value
    if (!fValues.getDatatypeValidatorAt(index) &&
        !fValues.getValueAt(index)) {
        fValuesCount++;
    }

    fValues.put(field, dv, value);

    if (fValuesCount == fValues.size()) {

        // is this value as a group duplicated?
        if (contains(&fValues)) {
            duplicateValue();
        }

        // store values
        if (!fValueTuples) {
            fValueTuples = new (fMemoryManager) RefHashTableOf<FieldValueMap, ICValueHasher>(107, true, ICValueHasher(fMemoryManager), fMemoryManager);
        }

        FieldValueMap* pICItem = new (fMemoryManager) FieldValueMap(fValues);
        fValueTuples->put(pICItem, pICItem);
    }
}

void ValueStore::append(const ValueStore* const other) {

    if (!other->fValueTuples) {
        return;
    }

    RefHashTableOfEnumerator<FieldValueMap, ICValueHasher> iter(other->fValueTuples, false, fMemoryManager);
    while(iter.hasMoreElements())
    {
        FieldValueMap& valueMap = iter.nextElement();

        if (!contains(&valueMap)) {

            if (!fValueTuples) {
                fValueTuples = new (fMemoryManager) RefHashTableOf<FieldValueMap, ICValueHasher>(107, true, ICValueHasher(fMemoryManager), fMemoryManager);
            }

            FieldValueMap* pICItem = new (fMemoryManager) FieldValueMap(valueMap);
            fValueTuples->put(pICItem, pICItem);
        }
    }
}

void ValueStore::startValueScope() {

    fValuesCount = 0;

    XMLSize_t count = fIdentityConstraint->getFieldCount();

    for (XMLSize_t i = 0; i < count; i++) {
        fValues.put(fIdentityConstraint->getFieldAt(i), 0, 0);
    }
}

void ValueStore::endValueScope() {

    if (fValuesCount == 0) {

        if (fIdentityConstraint->getType() == IdentityConstraint::ICType_KEY && fDoReportError) {
            fScanner->getValidator()->emitError(XMLValid::IC_AbsentKeyValue,
                fIdentityConstraint->getElementName());
        }

        return;
    }

    // do we have enough values?
    if ((fValuesCount != fIdentityConstraint->getFieldCount()) && fDoReportError) {

        if(fIdentityConstraint->getType()==IdentityConstraint::ICType_KEY)
        {
			fScanner->getValidator()->emitError(XMLValid::IC_KeyNotEnoughValues,
                fIdentityConstraint->getElementName(), fIdentityConstraint->getIdentityConstraintName());
        }
    }
}

bool ValueStore::contains(const FieldValueMap* const other) {

    if (fValueTuples)
        return fValueTuples->get(other)!=0;

    return false;
}

void ValueStore::clear()
{
    fValuesCount=0;
    fValues.clear();
    if(fValueTuples)
        fValueTuples->removeAll();
}

// ---------------------------------------------------------------------------
//  ValueStore: Document handling methods
// ---------------------------------------------------------------------------
void ValueStore::endDocumentFragment(ValueStoreCache* const valueStoreCache) {

    if (fIdentityConstraint->getType() == IdentityConstraint::ICType_KEYREF) {

        // verify references
        // get the key store corresponding (if it exists):
        ValueStore* keyValueStore = valueStoreCache->getGlobalValueStoreFor(((IC_KeyRef*) fIdentityConstraint)->getKey());

        if (!keyValueStore) {

            if (fDoReportError) {
                fScanner->getValidator()->emitError(XMLValid::IC_KeyRefOutOfScope,
                    fIdentityConstraint->getIdentityConstraintName());
            }

            return;
        }

        if(fValueTuples)
        {
            RefHashTableOfEnumerator<FieldValueMap, ICValueHasher> iter(fValueTuples, false, fMemoryManager);
            while(iter.hasMoreElements())
            {
                FieldValueMap& valueMap = iter.nextElement();

                if (!keyValueStore->contains(&valueMap) && fDoReportError) {

                    fScanner->getValidator()->emitError(XMLValid::IC_KeyNotFound,
                        fIdentityConstraint->getElementName());
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
//  ValueStore: Error reporting methods
// ---------------------------------------------------------------------------
void ValueStore::reportNilError(IdentityConstraint* const ic) {

    if (fDoReportError && ic->getType() == IdentityConstraint::ICType_KEY) {
        fScanner->getValidator()->emitError(XMLValid::IC_KeyMatchesNillable,
                                            ic->getElementName());
    }
}

void ValueStore::duplicateValue() {

    if (fDoReportError) {

        switch (fIdentityConstraint->getType()) {
        case IdentityConstraint::ICType_UNIQUE:
            {
                fScanner->getValidator()->emitError(XMLValid::IC_DuplicateUnique,
                    fIdentityConstraint->getElementName());
                break;
            }
        case IdentityConstraint::ICType_KEY:
            {
                fScanner->getValidator()->emitError(XMLValid::IC_DuplicateKey,
                    fIdentityConstraint->getElementName());
                break;
            }
        }
    }
}

XERCES_CPP_NAMESPACE_END

/**
  * End of file ValueStore.cpp
  */

