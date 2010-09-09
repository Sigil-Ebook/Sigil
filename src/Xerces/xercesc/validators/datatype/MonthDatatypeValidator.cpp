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
 * $Id: MonthDatatypeValidator.cpp 471747 2006-11-06 14:31:56Z amassari $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/datatype/MonthDatatypeValidator.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Constructors and Destructor
// ---------------------------------------------------------------------------
MonthDatatypeValidator::MonthDatatypeValidator(MemoryManager* const manager)
:DateTimeValidator(0, 0, 0, DatatypeValidator::Month, manager)
{
    setOrdered(XSSimpleTypeDefinition::ORDERED_PARTIAL);
}

MonthDatatypeValidator::MonthDatatypeValidator(
                          DatatypeValidator*            const baseValidator
                        , RefHashTableOf<KVStringPair>* const facets
                        , RefArrayVectorOf<XMLCh>*      const enums
                        , const int                           finalSet
                        , MemoryManager* const                manager)
:DateTimeValidator(baseValidator, facets, finalSet, DatatypeValidator::Month, manager)
{
    init(enums, manager);
}

MonthDatatypeValidator::~MonthDatatypeValidator()
{}

DatatypeValidator* MonthDatatypeValidator::newInstance
(
      RefHashTableOf<KVStringPair>* const facets
    , RefArrayVectorOf<XMLCh>* const      enums
    , const int                           finalSet
    , MemoryManager* const                manager
)
{
    return (DatatypeValidator*) new (manager) MonthDatatypeValidator(this, facets, enums, finalSet, manager);
}

//
// caller need to release the date created here
//
XMLDateTime* MonthDatatypeValidator::parse(const XMLCh* const content, MemoryManager* const manager)
{
    XMLDateTime *pRetDate = new (manager) XMLDateTime(content, manager);
    Janitor<XMLDateTime> jan(pRetDate);

    try
    {
        pRetDate->parseMonth();
    }
    catch(const OutOfMemoryException&)
    {
        jan.release();

        throw;
    }

    return jan.release();
}

void MonthDatatypeValidator::parse(XMLDateTime* const pDate)
{
    pDate->parseMonth();
}


/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_TOCREATE(MonthDatatypeValidator)

void MonthDatatypeValidator::serialize(XSerializeEngine& serEng)
{
    DateTimeValidator::serialize(serEng);
}

XERCES_CPP_NAMESPACE_END

/**
  * End of file MonthDatatypeValidator::cpp
  */

