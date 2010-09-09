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
 * $Id: FieldValueMap.cpp 708224 2008-10-27 16:02:26Z amassari $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/schema/identity/FieldValueMap.hpp>
#include <xercesc/util/Janitor.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

typedef JanitorMemFunCall<FieldValueMap>    CleanupType;

// ---------------------------------------------------------------------------
//  FieldValueMap: Constructors and Destructor
// ---------------------------------------------------------------------------
FieldValueMap::FieldValueMap(MemoryManager* const manager)
    : fFields(0)
    , fValidators(0)
    , fValues(0)
    , fMemoryManager(manager)
{
}

FieldValueMap::FieldValueMap(const FieldValueMap& other)
    : XMemory(other)
    , fFields(0)
    , fValidators(0)
    , fValues(0)
    , fMemoryManager(other.fMemoryManager)
{
    if (other.fFields) {
        CleanupType cleanup(this, &FieldValueMap::cleanUp);

        try {

                XMLSize_t valuesSize = other.fValues->size();

                fFields = new (fMemoryManager) ValueVectorOf<IC_Field*>(*(other.fFields));
                fValidators = new (fMemoryManager) ValueVectorOf<DatatypeValidator*>(*(other.fValidators));
                fValues = new (fMemoryManager) RefArrayVectorOf<XMLCh>(other.fFields->curCapacity(), true, fMemoryManager);

                for (XMLSize_t i=0; i<valuesSize; i++) {
                    fValues->addElement(XMLString::replicate(other.fValues->elementAt(i), fMemoryManager));
                }
        }
        catch(const OutOfMemoryException&)
        {
            cleanup.release();

            throw;
        }

        cleanup.release();
    }
}

FieldValueMap::~FieldValueMap()
{
    cleanUp();
}

// ---------------------------------------------------------------------------
//  FieldValueMap: Private helper methods.
// ---------------------------------------------------------------------------
void FieldValueMap::cleanUp()
{
    delete fFields;
    delete fValidators;
    delete fValues;
}

// ---------------------------------------------------------------------------
//  FieldValueMap: Helper methods
// ---------------------------------------------------------------------------
bool FieldValueMap::indexOf(const IC_Field* const key, XMLSize_t& location) const {

    if (fFields) {

        XMLSize_t fieldSize = fFields->size();

        for (XMLSize_t i=0; i < fieldSize; i++) {
            if (fFields->elementAt(i) == key) {
                location=i;
                return true;
            }
        }
    }

    return false;
}

void FieldValueMap::clear()
{
    if(fFields)
        fFields->removeAllElements();
    if(fValidators)
        fValidators->removeAllElements();
    if(fValues)
        fValues->removeAllElements();
}

XERCES_CPP_NAMESPACE_END

/**
  * End of file FieldValueMap.cpp
  */

