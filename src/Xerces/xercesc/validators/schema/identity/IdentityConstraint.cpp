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
 * $Id: IdentityConstraint.cpp 676911 2008-07-15 13:27:32Z amassari $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/schema/identity/IdentityConstraint.hpp>
#include <xercesc/validators/schema/identity/IC_Selector.hpp>
#include <xercesc/validators/schema/identity/IC_Field.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

//since we need to dynamically created each and every derivatives 
//during deserialization by XSerializeEngine>>Derivative, we got
//to include all hpp
#include <xercesc/validators/schema/identity/IC_Unique.hpp>
#include <xercesc/validators/schema/identity/IC_Key.hpp>
#include <xercesc/validators/schema/identity/IC_KeyRef.hpp>

#include <xercesc/internal/XTemplateSerializer.hpp>

XERCES_CPP_NAMESPACE_BEGIN

typedef JanitorMemFunCall<IdentityConstraint>   CleanupType;

// ---------------------------------------------------------------------------
//  IdentityConstraint: Constructors and Destructor
// ---------------------------------------------------------------------------
IdentityConstraint::IdentityConstraint(const XMLCh* const identityConstraintName,
                                       const XMLCh* const elemName,
                                       MemoryManager* const manager)
    : fIdentityConstraintName(0)
    , fElemName(0)
    , fSelector(0)
    , fFields(0)
    , fMemoryManager(manager)
    , fNamespaceURI(-1)
{
    CleanupType cleanup(this, &IdentityConstraint::cleanUp);

    try {
        fIdentityConstraintName = XMLString::replicate(identityConstraintName, fMemoryManager);
        fElemName = XMLString::replicate(elemName, fMemoryManager);
    }
    catch(const OutOfMemoryException&)
    {
        cleanup.release();

        throw;
    }

    cleanup.release();
}


IdentityConstraint::~IdentityConstraint()
{
    cleanUp();
}

// ---------------------------------------------------------------------------
//  IdentityConstraint: operators
// ---------------------------------------------------------------------------
bool IdentityConstraint::operator ==(const IdentityConstraint& other) const {

    if (getType() != other.getType())
        return false;

    if (!XMLString::equals(fIdentityConstraintName, other.fIdentityConstraintName))
        return false;

    if (*fSelector != *(other.fSelector))
        return false;

    XMLSize_t fieldCount = fFields->size();

    if (fieldCount != other.fFields->size())
        return false;

    for (XMLSize_t i = 0; i < fieldCount; i++) {
        if (*(fFields->elementAt(i)) != *(other.fFields->elementAt(i)))
            return false;
    }

    return true;
}

bool IdentityConstraint::operator !=(const IdentityConstraint& other) const {

    return !operator==(other);
}

// ---------------------------------------------------------------------------
//  IdentityConstraint: Setter methods
// ---------------------------------------------------------------------------
void IdentityConstraint::setSelector(IC_Selector* const selector) {

    if (fSelector) {
        delete fSelector;
    }

    fSelector = selector;
}


// ---------------------------------------------------------------------------
//  IdentityConstraint: cleanUp methods
// ---------------------------------------------------------------------------
void IdentityConstraint::cleanUp() {

    fMemoryManager->deallocate(fIdentityConstraintName);//delete [] fIdentityConstraintName;
    fMemoryManager->deallocate(fElemName);//delete [] fElemName;
    delete fFields;
    delete fSelector;
}

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_NOCREATE(IdentityConstraint)

void IdentityConstraint::serialize(XSerializeEngine& serEng)
{

    if (serEng.isStoring())
    {
        serEng.writeString(fIdentityConstraintName);
        serEng.writeString(fElemName);

        serEng<<fSelector;
        serEng<<fNamespaceURI;
        /***
         *
         * Serialize RefVectorOf<IC_Field>* fFields;
         *
         ***/
        XTemplateSerializer::storeObject(fFields, serEng);
    
    }
    else
    {

        serEng.readString(fIdentityConstraintName);
        serEng.readString(fElemName);

        serEng>>fSelector;
        serEng>>fNamespaceURI;
        /***
         *
         * Deserialize RefVectorOf<IC_Field>* fFields;
         *
         ***/
        XTemplateSerializer::loadObject(&fFields, 4, true, serEng);

    }

}  

void IdentityConstraint::storeIC(XSerializeEngine&         serEng
                               , IdentityConstraint* const ic)
{
    if (ic)
    {
        serEng<<(int) ic->getType();
        serEng<<ic;
    }
    else
    {
        serEng<<(int) ICType_UNKNOWN;
    }

}

IdentityConstraint* IdentityConstraint::loadIC(XSerializeEngine& serEng)
{

    int type;
    serEng>>type;

    switch((ICType)type)
    {
    case ICType_UNIQUE: 
        IC_Unique* ic_unique;
        serEng>>ic_unique;
        return ic_unique;      
    case ICType_KEY:
        IC_Key* ic_key;
        serEng>>ic_key;
        return ic_key;
    case ICType_KEYREF: 
        IC_KeyRef* ic_keyref;
        serEng>>ic_keyref;
        return ic_keyref;
    case ICType_UNKNOWN:
        return 0;
    default: //we treat this same as UnKnown
        return 0;
    }

}

XERCES_CPP_NAMESPACE_END

/**
  * End of file IdentityConstraint.cpp
  */

