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
 * $Id: NOTATIONDatatypeValidator.cpp 676911 2008-07-15 13:27:32Z amassari $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/datatype/NOTATIONDatatypeValidator.hpp>
#include <xercesc/util/XMLUri.hpp>
#include <xercesc/validators/datatype/InvalidDatatypeFacetException.hpp>
#include <xercesc/validators/datatype/InvalidDatatypeValueException.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Constructors and Destructor
// ---------------------------------------------------------------------------
NOTATIONDatatypeValidator::NOTATIONDatatypeValidator(MemoryManager* const manager)
:AbstractStringValidator(0, 0, 0, DatatypeValidator::NOTATION, manager)
{}

NOTATIONDatatypeValidator::~NOTATIONDatatypeValidator()
{}

NOTATIONDatatypeValidator::NOTATIONDatatypeValidator(
                          DatatypeValidator*            const baseValidator
                        , RefHashTableOf<KVStringPair>* const facets
                        , RefArrayVectorOf<XMLCh>*           const enums
                        , const int                           finalSet
                        , MemoryManager* const manager)
:AbstractStringValidator(baseValidator, facets, finalSet, DatatypeValidator::NOTATION, manager)
{
    init(enums, manager);
}

DatatypeValidator* NOTATIONDatatypeValidator::newInstance
(
      RefHashTableOf<KVStringPair>* const facets
    , RefArrayVectorOf<XMLCh>* const      enums
    , const int                           finalSet
    , MemoryManager* const                manager
)
{
    return (DatatypeValidator*) new (manager) NOTATIONDatatypeValidator(this, facets, enums, finalSet, manager);
}

// ---------------------------------------------------------------------------
//  Utilities
// ---------------------------------------------------------------------------

void NOTATIONDatatypeValidator::checkValueSpace(const XMLCh* const content
                                                , MemoryManager* const manager)
{

    if (!XMLString::isValidNOTATION(content, manager))
    {
        ThrowXMLwithMemMgr1(InvalidDatatypeValueException
                , XMLExcepts::VALUE_NOTATION_Invalid
                , content
                , manager);
    }

}

void NOTATIONDatatypeValidator::checkContent( const XMLCh*             const content
                                          ,       ValidationContext* const context
                                          ,       bool                     asBase
                                          ,       MemoryManager*     const manager
                                          )
{

    //validate against base validator if any
    NOTATIONDatatypeValidator *pBaseValidator = (NOTATIONDatatypeValidator*) this->getBaseValidator();
    if (pBaseValidator)
        pBaseValidator->checkContent(content, context, true, manager);

    int thisFacetsDefined = getFacetsDefined();

    // we check pattern first
    if ( (thisFacetsDefined & DatatypeValidator::FACET_PATTERN ) != 0 )
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

    checkValueSpace(content, manager);

    if ((thisFacetsDefined & DatatypeValidator::FACET_ENUMERATION) != 0 &&
        (getEnumeration() != 0))
    {
        XMLCh* normContent = XMLString::replicate(content, manager);
        ArrayJanitor<XMLCh>  jan(normContent, manager);
        normalizeContent(normContent, manager);

        XMLSize_t i=0;
        XMLSize_t enumLength = getEnumeration()->size();
        for ( ; i < enumLength; i++)
        {
            if (XMLString::equals(normContent, getEnumeration()->elementAt(i)))
                break;
        }

        if (i == enumLength)
            ThrowXMLwithMemMgr1(InvalidDatatypeValueException, XMLExcepts::VALUE_NotIn_Enumeration, content, manager);
    }

    checkAdditionalFacet(content, manager);
}

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_TOCREATE(NOTATIONDatatypeValidator)

void NOTATIONDatatypeValidator::serialize(XSerializeEngine& serEng)
{
    AbstractStringValidator::serialize(serEng);
}

XERCES_CPP_NAMESPACE_END

/**
  * End of file NOTATIONDatatypeValidator.cpp
  */
