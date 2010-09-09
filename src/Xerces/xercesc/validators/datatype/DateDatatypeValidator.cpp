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
 * $Id: DateDatatypeValidator.cpp 471747 2006-11-06 14:31:56Z amassari $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/datatype/DateDatatypeValidator.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Constructors and Destructor
// ---------------------------------------------------------------------------
DateDatatypeValidator::DateDatatypeValidator(MemoryManager* const manager)
:DateTimeValidator(0, 0, 0, DatatypeValidator::Date, manager)
{
    setOrdered(XSSimpleTypeDefinition::ORDERED_PARTIAL);
}

DateDatatypeValidator::DateDatatypeValidator(
                          DatatypeValidator*            const baseValidator
                        , RefHashTableOf<KVStringPair>* const facets
                        , RefArrayVectorOf<XMLCh>*      const enums
                        , const int                           finalSet
                        , MemoryManager* const                manager)
:DateTimeValidator(baseValidator, facets, finalSet, DatatypeValidator::Date, manager)
{
    init(enums, manager);
}

DateDatatypeValidator::~DateDatatypeValidator()
{}

DatatypeValidator* DateDatatypeValidator::newInstance
(
      RefHashTableOf<KVStringPair>* const facets
    , RefArrayVectorOf<XMLCh>* const      enums
    , const int                           finalSet
    , MemoryManager* const                manager
)
{
    return (DatatypeValidator*) new (manager) DateDatatypeValidator(this, facets, enums, finalSet, manager);
}

//
// caller need to release the date created here
//
XMLDateTime* DateDatatypeValidator::parse(const XMLCh* const content, MemoryManager* const manager)
{
    XMLDateTime *pRetDate = new (manager) XMLDateTime(content, manager);
    Janitor<XMLDateTime> jan(pRetDate);

    try
    {
        pRetDate->parseDate();
    }
    catch(const OutOfMemoryException&)
    {
        jan.release();

        throw;
    }

    return jan.release();
}

void DateDatatypeValidator::parse(XMLDateTime* const pDate)
{
    pDate->parseDate();
}

const XMLCh* DateDatatypeValidator::getCanonicalRepresentation(const XMLCh*         const rawData
                                                              ,      MemoryManager* const memMgr
                                                              ,      bool                 toValidate) const
{
    MemoryManager* toUse = memMgr? memMgr : fMemoryManager;

    if (toValidate)
    {
        DateDatatypeValidator* temp = (DateDatatypeValidator*) this;

        try
        {
            temp->checkContent(rawData, 0, false, toUse);   
        }
        catch (...)
        {
            return 0;
        }
    }
    
    try
    {
        XMLDateTime aDateTime(rawData, toUse);
        aDateTime.parseDate();
        return aDateTime.getDateCanonicalRepresentation(toUse);
    }
    catch (...)
    {
        return 0;
    }

}

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_TOCREATE(DateDatatypeValidator)

void DateDatatypeValidator::serialize(XSerializeEngine& serEng)
{
    DateTimeValidator::serialize(serEng);
}

XERCES_CPP_NAMESPACE_END

/**
  * End of file DateDatatypeValidator::cpp
  */

