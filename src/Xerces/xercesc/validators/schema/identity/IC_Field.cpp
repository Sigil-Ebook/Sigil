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
 * $Id: IC_Field.cpp 471747 2006-11-06 14:31:56Z amassari $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/schema/identity/FieldActivator.hpp>
#include <xercesc/validators/schema/identity/IC_Field.hpp>
#include <xercesc/validators/schema/identity/ValueStore.hpp>
#include <xercesc/validators/schema/identity/XercesXPath.hpp>
#include <xercesc/validators/schema/identity/IdentityConstraint.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  FieldMatcher: Constructors and Destructor
// ---------------------------------------------------------------------------
FieldMatcher::FieldMatcher(XercesXPath* const xpath,
                           IC_Field* const aField,
                           ValueStore* const valueStore,
                           FieldActivator* const fieldActivator,
                           MemoryManager* const manager)
    : XPathMatcher(xpath, (IdentityConstraint*) 0, manager)
    , fValueStore(valueStore)
    , fField(aField)    
    , fFieldActivator(fieldActivator)
{
}

// ---------------------------------------------------------------------------
//  FieldMatcher: Match methods
// ---------------------------------------------------------------------------
void FieldMatcher::matched(const XMLCh* const content,
                           DatatypeValidator* const dv,
                           const bool isNil) {

    if(isNil) {
        fValueStore->reportNilError(fField->getIdentityConstraint());
    }

    fValueStore->addValue(fFieldActivator, fField, dv, content);

    // once we've stored the value for this field, we set the mayMatch
    // member to false so that, in the same scope, we don't match any more
    // values (and throw an error instead).
    fFieldActivator->setMayMatch(fField, false);
}

// ---------------------------------------------------------------------------
//  IC_Field: Constructors and Destructor
// ---------------------------------------------------------------------------
IC_Field::IC_Field(XercesXPath* const xpath,
                   IdentityConstraint* const identityConstraint)
    : fXPath(xpath)
    , fIdentityConstraint(identityConstraint)
{
}


IC_Field::~IC_Field()
{
    delete fXPath;
}

// ---------------------------------------------------------------------------
//  IC_Field: operators
// ---------------------------------------------------------------------------
bool IC_Field::operator== (const IC_Field& other) const {

    return (*fXPath == *(other.fXPath));
}

bool IC_Field::operator!= (const IC_Field& other) const {

    return !operator==(other);
}

// ---------------------------------------------------------------------------
//  IC_Field: Factory methods
// ---------------------------------------------------------------------------
XPathMatcher* IC_Field::createMatcher(FieldActivator* const fieldActivator,
                                      ValueStore* const valueStore,
                                      MemoryManager* const manager)
{
    return new (manager) FieldMatcher(fXPath, this, valueStore, fieldActivator, manager);
}

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_TOCREATE(IC_Field)

void IC_Field::serialize(XSerializeEngine& serEng)
{

    if (serEng.isStoring())
    {
        serEng<<fXPath;
        
        IdentityConstraint::storeIC(serEng, fIdentityConstraint);
    }
    else
    {
        serEng>>fXPath;

        fIdentityConstraint = IdentityConstraint::loadIC(serEng);
    }

}

IC_Field::IC_Field(MemoryManager* const )
:fXPath(0)
,fIdentityConstraint(0)
{
}

XERCES_CPP_NAMESPACE_END

/**
  * End of file IC_Field.cpp
  */

