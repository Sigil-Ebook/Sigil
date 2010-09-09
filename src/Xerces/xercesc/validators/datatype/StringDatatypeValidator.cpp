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
 * $Id: StringDatatypeValidator.cpp 471747 2006-11-06 14:31:56Z amassari $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/datatype/StringDatatypeValidator.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xercesc/validators/datatype/InvalidDatatypeFacetException.hpp>
#include <xercesc/validators/datatype/InvalidDatatypeValueException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Constructors and Destructor
// ---------------------------------------------------------------------------
StringDatatypeValidator::StringDatatypeValidator(MemoryManager* const manager)
:AbstractStringValidator(0, 0, 0, DatatypeValidator::String, manager)
{
    setWhiteSpace(DatatypeValidator::PRESERVE);
}

StringDatatypeValidator::StringDatatypeValidator(
                          DatatypeValidator*            const baseValidator
                        , RefHashTableOf<KVStringPair>* const facets
                        , RefArrayVectorOf<XMLCh>*      const enums
                        , const int                           finalSet
                        , MemoryManager* const                manager)
:AbstractStringValidator(baseValidator, facets, finalSet, DatatypeValidator::String, manager)
{
    setWhiteSpace(DatatypeValidator::PRESERVE);
    init(enums, manager);
}

StringDatatypeValidator::~StringDatatypeValidator()
{}

DatatypeValidator* StringDatatypeValidator::newInstance
(
      RefHashTableOf<KVStringPair>* const facets
    , RefArrayVectorOf<XMLCh>* const      enums
    , const int                           finalSet
    , MemoryManager* const                manager
)
{
    return (DatatypeValidator*) new (manager) StringDatatypeValidator(this, facets, enums, finalSet, manager);
}

StringDatatypeValidator::StringDatatypeValidator(
                          DatatypeValidator*            const baseValidator
                        , RefHashTableOf<KVStringPair>* const facets
                        , const int                           finalSet
                        , const ValidatorType                 type
                        , MemoryManager* const                manager)
:AbstractStringValidator(baseValidator, facets, finalSet, type, manager)
{
    setWhiteSpace(DatatypeValidator::PRESERVE);
    // do not invoke init() here!!!
}

// ---------------------------------------------------------------------------
//  Utilities
// ---------------------------------------------------------------------------
void StringDatatypeValidator::assignAdditionalFacet(const XMLCh* const key
                                                  , const XMLCh* const value
                                                  , MemoryManager* const manager)
{
    if (XMLString::equals(key, SchemaSymbols::fgELT_WHITESPACE))
    {
        // whiteSpace = preserve | replace | collapse
        if (XMLString::equals(value, SchemaSymbols::fgWS_PRESERVE))
            setWhiteSpace(DatatypeValidator::PRESERVE);
        else if (XMLString::equals(value, SchemaSymbols::fgWS_REPLACE))
            setWhiteSpace(DatatypeValidator::REPLACE);
        else if (XMLString::equals(value, SchemaSymbols::fgWS_COLLAPSE))
            setWhiteSpace(DatatypeValidator::COLLAPSE);
        else
            ThrowXMLwithMemMgr1(InvalidDatatypeFacetException, XMLExcepts::FACET_Invalid_WS, value, manager);
        //("whiteSpace value '" + ws + "' must be one of 'preserve', 'replace', 'collapse'.");

        setFacetsDefined(DatatypeValidator::FACET_WHITESPACE);
    }
    else
    {
        ThrowXMLwithMemMgr1(InvalidDatatypeFacetException
                , XMLExcepts::FACET_Invalid_Tag
                , key
                , manager);
    }
}

void StringDatatypeValidator::inheritAdditionalFacet()
{
    StringDatatypeValidator *pBaseValidator = (StringDatatypeValidator*) getBaseValidator();

    if (!pBaseValidator)
        return;

    // inherit whitespace
    if (((pBaseValidator->getFacetsDefined() & DatatypeValidator::FACET_WHITESPACE) !=0) &&
        ((getFacetsDefined() & DatatypeValidator::FACET_WHITESPACE) == 0))
    {
        setWhiteSpace(getBaseValidator()->getWSFacet());
        setFacetsDefined(DatatypeValidator::FACET_WHITESPACE);
    }
}

void StringDatatypeValidator::checkAdditionalFacetConstraints(MemoryManager* const manager) const
{

    StringDatatypeValidator *pBaseValidator = (StringDatatypeValidator*) getBaseValidator();

    if (!pBaseValidator)
        return;

    short    thisWSFacet = getWSFacet();
    short    baseWSFacet = pBaseValidator->getWSFacet();

    // check 4.3.6.c1 error: whitespace
    if (((getFacetsDefined() & DatatypeValidator::FACET_WHITESPACE) != 0) &&
        ((pBaseValidator->getFacetsDefined() & DatatypeValidator::FACET_WHITESPACE) != 0 ))
    {
        if ((baseWSFacet == DatatypeValidator::COLLAPSE) &&
            ((thisWSFacet == DatatypeValidator::PRESERVE) ||
             (thisWSFacet == DatatypeValidator::REPLACE)))
             ThrowXMLwithMemMgr(InvalidDatatypeFacetException, XMLExcepts::FACET_WS_collapse, manager);

        if ((baseWSFacet == DatatypeValidator::REPLACE) &&
            (thisWSFacet == DatatypeValidator::PRESERVE))
            ThrowXMLwithMemMgr(InvalidDatatypeFacetException, XMLExcepts::FACET_WS_replace, manager);

        if (((pBaseValidator->getFixed() & DatatypeValidator::FACET_WHITESPACE) !=0) &&
            ( thisWSFacet != baseWSFacet))
        {
            ThrowXMLwithMemMgr2(InvalidDatatypeFacetException
                        , XMLExcepts::FACET_whitespace_base_fixed
                        , getWSstring(thisWSFacet)
                        , getWSstring(baseWSFacet)
                        , manager);
        }
    }

}

void StringDatatypeValidator::checkAdditionalFacet(const XMLCh* const content
                                                   , MemoryManager* const manager) const
{
    //
    // check WhiteSpace
    //
    if ((getFacetsDefined() & DatatypeValidator::FACET_WHITESPACE) != 0 )
    {
        if ( getWSFacet() == DatatypeValidator::REPLACE )
        {
            if (!XMLString::isWSReplaced(content))
                ThrowXMLwithMemMgr1(InvalidDatatypeValueException, XMLExcepts::VALUE_WS_replaced, content, manager);
        }
        else if ( getWSFacet() == DatatypeValidator::COLLAPSE )
        {
            if (!XMLString::isWSCollapsed(content))
                ThrowXMLwithMemMgr1(InvalidDatatypeValueException, XMLExcepts::VALUE_WS_collapsed, content, manager);
        }

    }
}

void StringDatatypeValidator::checkValueSpace(const XMLCh* const
                                              , MemoryManager* const)
{}

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_TOCREATE(StringDatatypeValidator)

void StringDatatypeValidator::serialize(XSerializeEngine& serEng)
{
    AbstractStringValidator::serialize(serEng);
}

XERCES_CPP_NAMESPACE_END

/**
  * End of file StringDatatypeValidator.cpp
  */
