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
 * $Id: DoubleDatatypeValidator.cpp 676911 2008-07-15 13:27:32Z amassari $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/datatype/DoubleDatatypeValidator.hpp>
#include <xercesc/validators/datatype/InvalidDatatypeFacetException.hpp>
#include <xercesc/validators/datatype/InvalidDatatypeValueException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Constructors and Destructor
// ---------------------------------------------------------------------------
DoubleDatatypeValidator::DoubleDatatypeValidator(MemoryManager* const manager)
:AbstractNumericValidator(0, 0, 0, DatatypeValidator::Double, manager)
{
    setOrdered(XSSimpleTypeDefinition::ORDERED_PARTIAL);
    setBounded(true);
    setFinite(true);
    setNumeric(true);
}

DoubleDatatypeValidator::DoubleDatatypeValidator(
                          DatatypeValidator*            const baseValidator
                        , RefHashTableOf<KVStringPair>* const facets
                        , RefArrayVectorOf<XMLCh>*      const enums
                        , const int                           finalSet
                        , MemoryManager* const                manager)
:AbstractNumericValidator(baseValidator, facets, finalSet, DatatypeValidator::Double, manager)
{
    init(enums, manager);
}

DoubleDatatypeValidator::~DoubleDatatypeValidator()
{}

// -----------------------------------------------------------------------
// Compare methods
// -----------------------------------------------------------------------
int DoubleDatatypeValidator::compare(const XMLCh* const lValue
                                   , const XMLCh* const rValue
                                   , MemoryManager* const manager)
{
    XMLDouble lObj(lValue, manager);
    XMLDouble rObj(rValue, manager);

    return compareValues(&lObj, &rObj);
}

DatatypeValidator* DoubleDatatypeValidator::newInstance
(
      RefHashTableOf<KVStringPair>* const facets
    , RefArrayVectorOf<XMLCh>* const      enums
    , const int                           finalSet
    , MemoryManager* const                manager
)
{
    return (DatatypeValidator*) new (manager) DoubleDatatypeValidator(this, facets, enums, finalSet, manager);
}

// -----------------------------------------------------------------------
// ctor provided to be used by derived classes
// -----------------------------------------------------------------------
DoubleDatatypeValidator::DoubleDatatypeValidator(DatatypeValidator*            const baseValidator
                                               , RefHashTableOf<KVStringPair>* const facets
                                               , const int                           finalSet
                                               , const ValidatorType                 type
                                               , MemoryManager* const                manager)
:AbstractNumericValidator(baseValidator, facets, finalSet, type, manager)
{
    //do not invoke init here !!!
}

int  DoubleDatatypeValidator::compareValues(const XMLNumber* const lValue
                                          , const XMLNumber* const rValue)
{
    return XMLDouble::compareValues((XMLDouble*) lValue, (XMLDouble*) rValue);
}

void  DoubleDatatypeValidator::setMaxInclusive(const XMLCh* const value)
{
    fMaxInclusive = new (fMemoryManager) XMLDouble(value, fMemoryManager);
}

void  DoubleDatatypeValidator::setMaxExclusive(const XMLCh* const value)
{
    fMaxExclusive = new (fMemoryManager) XMLDouble(value, fMemoryManager);
}

void  DoubleDatatypeValidator::setMinInclusive(const XMLCh* const value)
{
    fMinInclusive = new (fMemoryManager) XMLDouble(value, fMemoryManager);
}

void  DoubleDatatypeValidator::setMinExclusive(const XMLCh* const value)
{
    fMinExclusive = new (fMemoryManager) XMLDouble(value, fMemoryManager);
}

void  DoubleDatatypeValidator::setEnumeration(MemoryManager* const manager)
{
    // check 4.3.5.c0 must: enumeration values from the value space of base
    //
    // 1. shall be from base value space
    // 2. shall be from current value space as well ( shall go through boundsCheck() )
    //
    if (!fStrEnumeration)
        return;

    XMLSize_t i = 0;
    XMLSize_t enumLength = fStrEnumeration->size();

    DoubleDatatypeValidator *numBase = (DoubleDatatypeValidator*) getBaseValidator();
    if (numBase)
    {
        try
        {
            for ( i = 0; i < enumLength; i++)
            {
                numBase->checkContent(fStrEnumeration->elementAt(i), (ValidationContext*)0, false, manager);
            }
        }
        catch (XMLException&)
        {
            ThrowXMLwithMemMgr1(InvalidDatatypeFacetException
                    , XMLExcepts::FACET_enum_base
                    , fStrEnumeration->elementAt(i)
                    , manager);

        }
    }

#if 0
// spec says that only base has to checkContent          
    // We put the this->checkContent in a separate loop
    // to not block original message with in that method.
    //
    for ( i = 0; i < enumLength; i++)
    {
        checkContent(fStrEnumeration->elementAt(i), (ValidationContext*)0, false, manager);
    }
#endif

    fEnumeration = new (manager) RefVectorOf<XMLNumber>(enumLength, true, manager);
    fEnumerationInherited = false;

    for ( i = 0; i < enumLength; i++)
    {
        fEnumeration->insertElementAt(new (manager) XMLDouble(fStrEnumeration->elementAt(i), manager), i);
    }
}

// -----------------------------------------------------------------------
// Abstract interface from AbstractNumericValidator
// -----------------------------------------------------------------------

void DoubleDatatypeValidator::checkContent(const XMLCh*             const content
                                          ,      ValidationContext* const context
                                          ,      bool                     asBase
                                          ,      MemoryManager*     const manager)
{

    //validate against base validator if any
    DoubleDatatypeValidator *pBase = (DoubleDatatypeValidator*) this->getBaseValidator();
    if (pBase)
        pBase->checkContent(content, context, true, manager);

    // we check pattern first
    if ( (getFacetsDefined() & DatatypeValidator::FACET_PATTERN ) != 0 )
    {
        if (getRegex()->matches(content, manager) ==false)
        {
            ThrowXMLwithMemMgr2(InvalidDatatypeValueException
                    , XMLExcepts::VALUE_NotMatch_Pattern
                    , content
                    , getPattern()
                    , manager);
        }
    }

    // if this is a base validator, we only need to check pattern facet
    // all other facet were inherited by the derived type
    if (asBase)
        return;

    XMLDouble theValue(content, manager);
    XMLDouble *theData = &theValue;

    if (getEnumeration())
    {
        XMLSize_t i=0;
        XMLSize_t enumLength = getEnumeration()->size();
        for ( ; i < enumLength; i++)
        {
            if (compareValues(theData, (XMLDouble*) getEnumeration()->elementAt(i)) ==0 )
                break;
        }

        if (i == enumLength)
            ThrowXMLwithMemMgr1(InvalidDatatypeValueException, XMLExcepts::VALUE_NotIn_Enumeration, content, manager);
    }

    boundsCheck(theData, manager);
}

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_TOCREATE(DoubleDatatypeValidator)

void DoubleDatatypeValidator::serialize(XSerializeEngine& serEng)
{
    /***
     * Note: 
     *
     *     During storing, we need write the specific number
     *     type info before calling base::serialize().
     *
     *     While loading, we do nothing here
     ***/

    if (serEng.isStoring())
    {
        serEng<<(int) (XMLNumber::Double);
    }

    AbstractNumericValidator::serialize(serEng);

}

XERCES_CPP_NAMESPACE_END

/**
  * End of file DoubleDatatypeValidator::cpp
  */
