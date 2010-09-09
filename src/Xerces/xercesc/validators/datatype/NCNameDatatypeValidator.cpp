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
 * $Id: NCNameDatatypeValidator.cpp 471747 2006-11-06 14:31:56Z amassari $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/datatype/NCNameDatatypeValidator.hpp>
#include <xercesc/validators/datatype/InvalidDatatypeValueException.hpp>
#include <xercesc/util/XMLChar.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Constructors and Destructor
// ---------------------------------------------------------------------------
NCNameDatatypeValidator::NCNameDatatypeValidator(MemoryManager* const manager)
:StringDatatypeValidator(0, 0, 0, DatatypeValidator::NCName, manager)
{}

NCNameDatatypeValidator::~NCNameDatatypeValidator()
{}

NCNameDatatypeValidator::NCNameDatatypeValidator(
                          DatatypeValidator*            const baseValidator
                        , RefHashTableOf<KVStringPair>* const facets
                        , RefArrayVectorOf<XMLCh>*      const enums
                        , const int                           finalSet
                        , MemoryManager* const                manager)
:StringDatatypeValidator(baseValidator, facets, finalSet, DatatypeValidator::NCName, manager)
{
    init(enums, manager);
}

DatatypeValidator* NCNameDatatypeValidator::newInstance
(
      RefHashTableOf<KVStringPair>* const facets
    , RefArrayVectorOf<XMLCh>* const      enums
    , const int                           finalSet
    , MemoryManager* const                manager
)
{
    return (DatatypeValidator*) new (manager) NCNameDatatypeValidator(this, facets, enums, finalSet, manager);
}

NCNameDatatypeValidator::NCNameDatatypeValidator(
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
int NCNameDatatypeValidator::compare(const XMLCh* const lValue
                                   , const XMLCh* const rValue
                                   ,       MemoryManager*     const)
{
    return ( XMLString::equals(lValue, rValue)? 0 : -1);
}

void NCNameDatatypeValidator::validate(const XMLCh*             const content
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

void NCNameDatatypeValidator::checkValueSpace(const XMLCh* const content
                                              , MemoryManager* const manager)
{
    //
    // 3.3.7 check must: "NCName"
    //
    if ( !XMLChar1_0::isValidNCName(content, XMLString::stringLen(content)) )
    {
        ThrowXMLwithMemMgr1(InvalidDatatypeValueException
                , XMLExcepts::VALUE_Invalid_NCName
                , content
                , manager);
    }

}

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_TOCREATE(NCNameDatatypeValidator)

void NCNameDatatypeValidator::serialize(XSerializeEngine& serEng)
{
    StringDatatypeValidator::serialize(serEng);
}

XERCES_CPP_NAMESPACE_END

 /**
  * End of file NCNameDatatypeValidator.cpp
  */
