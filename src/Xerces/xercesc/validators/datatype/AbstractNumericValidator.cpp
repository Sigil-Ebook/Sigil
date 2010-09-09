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
 * $Id: AbstractNumericValidator.cpp 471747 2006-11-06 14:31:56Z amassari $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/datatype/AbstractNumericValidator.hpp>
#include <xercesc/validators/datatype/InvalidDatatypeValueException.hpp>
#include <xercesc/util/XMLAbstractDoubleFloat.hpp>

XERCES_CPP_NAMESPACE_BEGIN

#define  REPORT_VALUE_ERROR(val1, val2, except_code, manager)    \
  ThrowXMLwithMemMgr2(InvalidDatatypeValueException               \
          , except_code                                 \
          , val1->getFormattedString()                  \
          , val2->getFormattedString()                  \
          , manager);

// ---------------------------------------------------------------------------
//  Constructors and Destructor
// ---------------------------------------------------------------------------
AbstractNumericValidator::~AbstractNumericValidator()
{}

AbstractNumericValidator::AbstractNumericValidator(
                          DatatypeValidator*            const baseValidator
                        , RefHashTableOf<KVStringPair>* const facets
                        , const int                           finalSet
                        , const ValidatorType                 type
                        , MemoryManager* const                manager)
:AbstractNumericFacetValidator(baseValidator, facets, finalSet, type, manager)
{
    //do not invoke init() here !!!
}

void AbstractNumericValidator::validate(const XMLCh*             const content
                                       ,      ValidationContext* const context
                                       ,      MemoryManager*     const manager)
{
    checkContent(content, context, false, manager);
}

void AbstractNumericValidator::boundsCheck(const XMLNumber*         const theData
                                          ,      MemoryManager*     const manager)
{
    int thisFacetsDefined = getFacetsDefined();
    int result;

    if (thisFacetsDefined == 0)
        return;

    // must be < MaxExclusive
    if ( (thisFacetsDefined & DatatypeValidator::FACET_MAXEXCLUSIVE) != 0 )
    {
        result = compareValues(theData, getMaxExclusive());
        if ( result != -1)
        {
            REPORT_VALUE_ERROR(theData
                                 , getMaxExclusive()
                                 , XMLExcepts::VALUE_exceed_maxExcl
                                 , manager)
        }
    } 	

    // must be <= MaxInclusive
    if ( (thisFacetsDefined & DatatypeValidator::FACET_MAXINCLUSIVE) != 0 )
    {
        result = compareValues(theData, getMaxInclusive());
        if (result == 1)
        {
            REPORT_VALUE_ERROR(theData
                             , getMaxInclusive()
                             , XMLExcepts::VALUE_exceed_maxIncl
                             , manager)
        }
    }

    // must be >= MinInclusive
    if ( (thisFacetsDefined & DatatypeValidator::FACET_MININCLUSIVE) != 0 )
    {
        result = compareValues(theData, getMinInclusive());
        if (result == -1)
        {
            REPORT_VALUE_ERROR(theData
                             , getMinInclusive()
                             , XMLExcepts::VALUE_exceed_minIncl
                             , manager)
        }
    }

    // must be > MinExclusive
    if ( (thisFacetsDefined & DatatypeValidator::FACET_MINEXCLUSIVE) != 0 )
    {
        result = compareValues(theData, getMinExclusive());
        if (result != 1)
        {
            REPORT_VALUE_ERROR(theData
                             , getMinExclusive()
                             , XMLExcepts::VALUE_exceed_minExcl
                             , manager)
        }
    }
}

const XMLCh* AbstractNumericValidator::getCanonicalRepresentation(const XMLCh*         const rawData
                                                                 ,      MemoryManager* const memMgr
                                                                 ,      bool                 toValidate) const
{
    MemoryManager* toUse = memMgr? memMgr : fMemoryManager;

    if (toValidate)
    {
        AbstractNumericValidator* temp = (AbstractNumericValidator*) this;

        try 
        {
            temp->checkContent(rawData, 0, false, toUse);
        }
        catch (...)
        {
            return 0;
        }
    }

    // XMLAbstractDoubleFloat::getCanonicalRepresentation handles
    // exceptional cases
    return XMLAbstractDoubleFloat::getCanonicalRepresentation(rawData, toUse);

}

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_NOCREATE(AbstractNumericValidator)

void AbstractNumericValidator::serialize(XSerializeEngine& serEng)
{
    AbstractNumericFacetValidator::serialize(serEng);

    /***
     * Need not to do anything else here
     *
     * Note: its derivatives, Doubledv, Floatdv and Decimaldv writes
     *       number type info into the binary data stream to be read
     *       by AbstractNumericFacetVaildator during loading, therefore
     *       this class can NOT write/read anything into/from the binary
     *       data stream.
     *
     *       Later on, if this class has to write/read something into/from
     *       the binary data stream, we need to add a numberType data
     *       to XMLNumber and let AbstractNumericFacetValidator to write/read
     *       this number type info.
     ***/
}

XERCES_CPP_NAMESPACE_END

/**
  * End of file AbstractNumericValidator::cpp
  */

