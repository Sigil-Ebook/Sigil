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
 * $Id: BooleanDatatypeValidator.cpp 471747 2006-11-06 14:31:56Z amassari $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/datatype/BooleanDatatypeValidator.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xercesc/validators/datatype/InvalidDatatypeFacetException.hpp>
#include <xercesc/validators/datatype/InvalidDatatypeValueException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Constructors and Destructor
// ---------------------------------------------------------------------------
BooleanDatatypeValidator::BooleanDatatypeValidator(
                          DatatypeValidator*            const baseValidator
                        , RefHashTableOf<KVStringPair>* const facets
                        , RefArrayVectorOf<XMLCh>*      const enums
                        , const int                           finalSet
                        , MemoryManager* const                manager)
:DatatypeValidator(baseValidator, facets, finalSet, DatatypeValidator::Boolean, manager)
{

    // Set Facets if any defined
    if ( facets )
    {

        // Boolean shall NOT have enumeration
        if (enums) {
            delete enums;
            ThrowXMLwithMemMgr1(InvalidDatatypeFacetException
                    , XMLExcepts::FACET_Invalid_Tag
                    , "enumeration"
                    , manager);
        }

        XMLCh* key;
        XMLCh* value;
        RefHashTableOfEnumerator<KVStringPair> e(facets, false, manager);

        while (e.hasMoreElements())
        {
            KVStringPair pair = e.nextElement();
            key = pair.getKey();
            value = pair.getValue();

            if (XMLString::equals(key, SchemaSymbols::fgELT_PATTERN))
            {
                setPattern(value);
                setFacetsDefined(DatatypeValidator::FACET_PATTERN);
            }
            else
            {
                ThrowXMLwithMemMgr1(InvalidDatatypeFacetException
                        , XMLExcepts::FACET_Invalid_Tag
                        , key
                        , manager);
            }

        }

    }// End of facet setting
}

void BooleanDatatypeValidator::checkContent( const XMLCh*             const content
                                           ,       ValidationContext* const context
                                           ,       bool                     asBase
                                           ,       MemoryManager*     const manager)
{

    //validate against base validator if any
    BooleanDatatypeValidator *pBaseValidator = (BooleanDatatypeValidator*) this->getBaseValidator();
    if (pBaseValidator !=0)
        pBaseValidator->checkContent(content, context, true, manager);

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

    unsigned int   i = 0;
    for ( ; i < XMLUni::fgBooleanValueSpaceArraySize; i++ )
    {
        if ( XMLString::equals(content, XMLUni::fgBooleanValueSpace[i]))
            break;
    }

    if (i == XMLUni::fgBooleanValueSpaceArraySize)
        ThrowXMLwithMemMgr2(InvalidDatatypeValueException
                           , XMLExcepts::VALUE_Invalid_Name
                           , content
                           , SchemaSymbols::fgDT_BOOLEAN
                           , manager);
        //Not valid boolean type

}

int BooleanDatatypeValidator::compare(const XMLCh* const lValue
                                    , const XMLCh* const rValue
                                    , MemoryManager* const)
{
    // need to check by bool semantics
    // 1 == true
    // 0 == false

    if (XMLString::equals(lValue, XMLUni::fgBooleanValueSpace[0])||
        XMLString::equals(lValue, XMLUni::fgBooleanValueSpace[2]))
    {
        if (XMLString::equals(rValue, XMLUni::fgBooleanValueSpace[0]) ||
            XMLString::equals(rValue, XMLUni::fgBooleanValueSpace[2]))
            return 0;
    }
    else
    if (XMLString::equals(lValue, XMLUni::fgBooleanValueSpace[1]) ||
        XMLString::equals(lValue, XMLUni::fgBooleanValueSpace[3]))
    {
        if (XMLString::equals(rValue, XMLUni::fgBooleanValueSpace[1]) ||
            XMLString::equals(rValue, XMLUni::fgBooleanValueSpace[3]))
            return 0;
    }

    return 1;
}

const RefArrayVectorOf<XMLCh>* BooleanDatatypeValidator::getEnumString() const
{
	return 0;
}

/***
 * 3.2.2.2 Canonical representation
 *
 * The canonical representation for boolean is the set of literals {true, false}.
 ***/
const XMLCh* BooleanDatatypeValidator::getCanonicalRepresentation(const XMLCh*         const rawData
                                                                ,       MemoryManager* const memMgr
                                                                ,       bool           toValidate) const
{

    MemoryManager* toUse = memMgr? memMgr : getMemoryManager();
    
    if (toValidate)
    {
        BooleanDatatypeValidator *temp = (BooleanDatatypeValidator*) this;

        try
        {
            temp->checkContent(rawData, 0, false, toUse);   
        }
        catch (...)
        {
            return 0;
        }
    }

    return ( XMLString::equals(rawData, XMLUni::fgBooleanValueSpace[0]) ||
             XMLString::equals(rawData, XMLUni::fgBooleanValueSpace[2])  ) ?
             XMLString::replicate(XMLUni::fgBooleanValueSpace[0], toUse) :
             XMLString::replicate(XMLUni::fgBooleanValueSpace[1], toUse) ;

}

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_TOCREATE(BooleanDatatypeValidator)

void BooleanDatatypeValidator::serialize(XSerializeEngine& serEng)
{
    DatatypeValidator::serialize(serEng);
}

XERCES_CPP_NAMESPACE_END

/**
  * End of file BooleanDatatypeValidator.cpp
  */

