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
 * $Id: HexBinaryDatatypeValidator.cpp 695949 2008-09-16 15:57:44Z borisk $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/datatype/HexBinaryDatatypeValidator.hpp>
#include <xercesc/validators/datatype/InvalidDatatypeFacetException.hpp>
#include <xercesc/validators/datatype/InvalidDatatypeValueException.hpp>
#include <xercesc/util/HexBin.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Constructors and Destructor
// ---------------------------------------------------------------------------
HexBinaryDatatypeValidator::HexBinaryDatatypeValidator(MemoryManager* const manager)
:AbstractStringValidator(0, 0, 0, DatatypeValidator::HexBinary, manager)
{}

HexBinaryDatatypeValidator::~HexBinaryDatatypeValidator()
{}

HexBinaryDatatypeValidator::HexBinaryDatatypeValidator(
                          DatatypeValidator*            const baseValidator
                        , RefHashTableOf<KVStringPair>* const facets
                        , RefArrayVectorOf<XMLCh>*      const enums
                        , const int                           finalSet
                        , MemoryManager* const                manager)
:AbstractStringValidator(baseValidator, facets, finalSet, DatatypeValidator::HexBinary, manager)
{
    init(enums, manager);
}

DatatypeValidator* HexBinaryDatatypeValidator::newInstance
(
      RefHashTableOf<KVStringPair>* const facets
    , RefArrayVectorOf<XMLCh>* const      enums
    , const int                           finalSet
    , MemoryManager* const                manager
)
{
    return (DatatypeValidator*) new (manager) HexBinaryDatatypeValidator(this, facets, enums, finalSet, manager);
}

// ---------------------------------------------------------------------------
//  Utilities
// ---------------------------------------------------------------------------

void HexBinaryDatatypeValidator::checkValueSpace(const XMLCh* const content,
                                                 MemoryManager* const manager)
{
    if (HexBin::getDataLength(content) < 0)
    {
        ThrowXMLwithMemMgr1(InvalidDatatypeValueException
                , XMLExcepts::VALUE_Not_HexBin
                , content
                , manager);
    }
}

XMLSize_t HexBinaryDatatypeValidator::getLength(const XMLCh* const content
                                      , MemoryManager* const) const
{
    return (XMLSize_t)HexBin::getDataLength(content);
}

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_TOCREATE(HexBinaryDatatypeValidator)

void HexBinaryDatatypeValidator::serialize(XSerializeEngine& serEng)
{
    AbstractStringValidator::serialize(serEng);
}

XERCES_CPP_NAMESPACE_END

/**
  * End of file HexBinaryDatatypeValidator.cpp
  */
