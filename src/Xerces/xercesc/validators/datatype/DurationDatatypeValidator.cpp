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
 * $Id: DurationDatatypeValidator.cpp 471747 2006-11-06 14:31:56Z amassari $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/datatype/DurationDatatypeValidator.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Constructors and Destructor
// ---------------------------------------------------------------------------
DurationDatatypeValidator::DurationDatatypeValidator(MemoryManager* const manager)
:DateTimeValidator(0, 0, 0, DatatypeValidator::Duration, manager)
{
    setOrdered(XSSimpleTypeDefinition::ORDERED_PARTIAL);
}

DurationDatatypeValidator::DurationDatatypeValidator(
                          DatatypeValidator*            const baseValidator
                        , RefHashTableOf<KVStringPair>* const facets
                        , RefArrayVectorOf<XMLCh>*      const enums
                        , const int                           finalSet
                        , MemoryManager* const                manager)
:DateTimeValidator(baseValidator, facets, finalSet, DatatypeValidator::Duration, manager)
{
    init(enums, manager);
}

DurationDatatypeValidator::~DurationDatatypeValidator()
{}

DatatypeValidator* DurationDatatypeValidator::newInstance
(
      RefHashTableOf<KVStringPair>* const facets
    , RefArrayVectorOf<XMLCh>* const      enums
    , const int                           finalSet
    , MemoryManager* const                manager
)
{
    return (DatatypeValidator*) new (manager) DurationDatatypeValidator(this, facets, enums, finalSet, manager);
}

//
// caller need to release the date created here
//
XMLDateTime* DurationDatatypeValidator::parse(const XMLCh* const content, MemoryManager* const manager)
{
    XMLDateTime *pRetDate = new (manager) XMLDateTime(content, manager);
    Janitor<XMLDateTime> jan(pRetDate);

    try
    {
        pRetDate->parseDuration();
    }
    catch(const OutOfMemoryException&)
    {
        jan.release();

        throw;
    }

    return jan.release();
}

void DurationDatatypeValidator::parse(XMLDateTime* const pDate)
{
    pDate->parseDuration();
}

int DurationDatatypeValidator::compareDates(const XMLDateTime* const date1
                                          , const XMLDateTime* const date2
                                          , bool strict)
{
    return XMLDateTime::compare(date1, date2, strict);
}

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_TOCREATE(DurationDatatypeValidator)

void DurationDatatypeValidator::serialize(XSerializeEngine& serEng)
{
    DateTimeValidator::serialize(serEng);
}

XERCES_CPP_NAMESPACE_END

/**
  * End of file DurationDatatypeValidator::cpp
  */

