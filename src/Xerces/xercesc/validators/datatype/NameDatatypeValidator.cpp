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
 * $Id: NameDatatypeValidator.cpp 558817 2007-07-23 18:12:54Z cargilld $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/datatype/NameDatatypeValidator.hpp>
#include <xercesc/validators/datatype/InvalidDatatypeValueException.hpp>
#include <xercesc/util/XMLChar.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Constructors and Destructor
// ---------------------------------------------------------------------------
NameDatatypeValidator::NameDatatypeValidator(MemoryManager* const manager)
:StringDatatypeValidator(0, 0, 0, DatatypeValidator::Name, manager)
{}

NameDatatypeValidator::~NameDatatypeValidator()
{}

NameDatatypeValidator::NameDatatypeValidator(
                          DatatypeValidator*            const baseValidator
                        , RefHashTableOf<KVStringPair>* const facets
                        , RefArrayVectorOf<XMLCh>*      const enums
                        , const int                           finalSet
                        , MemoryManager* const                manager)
:StringDatatypeValidator(baseValidator, facets, finalSet, DatatypeValidator::Name, manager)
{
    init(enums, manager);
}

DatatypeValidator* NameDatatypeValidator::newInstance
(
      RefHashTableOf<KVStringPair>* const facets
    , RefArrayVectorOf<XMLCh>* const      enums
    , const int                           finalSet
    , MemoryManager* const                manager
)
{
    return (DatatypeValidator*) new (manager) NameDatatypeValidator(this, facets, enums, finalSet, manager);
}

NameDatatypeValidator::NameDatatypeValidator(
                          DatatypeValidator*            const baseValidator
                        , RefHashTableOf<KVStringPair>* const facets
                        , const int                           finalSet
                        , const ValidatorType                 type
                        , MemoryManager* const                manager)
:StringDatatypeValidator(baseValidator, facets, finalSet, type, manager)
{
    // do not invoke init() here!!!
}
// -----------------------------------------------------------------------
// Compare methods
// -----------------------------------------------------------------------
int NameDatatypeValidator::compare(const XMLCh* const lValue
                                   , const XMLCh* const rValue
                                   ,       MemoryManager*     const)
{
    return ( XMLString::equals(lValue, rValue)? 0 : -1);
}

void NameDatatypeValidator::validate(const XMLCh*             const content
                                   ,       ValidationContext* const context
                                   ,       MemoryManager*     const manager)
{
    // use StringDatatypeValidator (which in turn, invoke
    // the baseValidator) to validate content against
    // facets if any.
    //
    StringDatatypeValidator::validate(content, context, manager);

    return;
}

void NameDatatypeValidator::checkValueSpace(const XMLCh* const content
                                            , MemoryManager* const manager)
{
    //
    // 3.3.6 check must: "Name"
    //
    if ( !XMLChar1_0::isValidName(content))
    {
        ThrowXMLwithMemMgr2(InvalidDatatypeValueException
                , XMLExcepts::VALUE_Invalid_Name
                , content
                , SchemaSymbols::fgDT_NAME
                , manager);
    }

}

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_TOCREATE(NameDatatypeValidator)

void NameDatatypeValidator::serialize(XSerializeEngine& serEng)
{
    StringDatatypeValidator::serialize(serEng);
}

XERCES_CPP_NAMESPACE_END

 /**
  * End of file NameDatatypeValidator.cpp
  */
