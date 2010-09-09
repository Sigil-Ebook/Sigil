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
 * $Id: DateTimeValidator.cpp 676911 2008-07-15 13:27:32Z amassari $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/datatype/DateTimeValidator.hpp>
#include <xercesc/validators/datatype/InvalidDatatypeFacetException.hpp>
#include <xercesc/validators/datatype/InvalidDatatypeValueException.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Macro
// ---------------------------------------------------------------------------
#define  REPORT_VALUE_ERROR(val1, val2, except_code, manager)    \
  ThrowXMLwithMemMgr2(InvalidDatatypeValueException               \
          , except_code                                 \
          , val1->getRawData()                          \
          , val2->getRawData()                          \
          , manager);

// ---------------------------------------------------------------------------
//  Constructors and Destructor
// ---------------------------------------------------------------------------
DateTimeValidator::~DateTimeValidator()
{
}

DateTimeValidator::DateTimeValidator(
                          DatatypeValidator*            const baseValidator
                        , RefHashTableOf<KVStringPair>* const facets
                        , const int                           finalSet
                        , const ValidatorType                 type
                        , MemoryManager* const                manager)
:AbstractNumericFacetValidator(baseValidator, facets, finalSet, type, manager)
{
    //do not invoke init() here !!!
}

void DateTimeValidator::validate(const XMLCh*             const content
                               ,       ValidationContext* const context
                               ,       MemoryManager*     const manager)
{
    checkContent(content, context, false, manager);
}

int DateTimeValidator::compare(const XMLCh* const value1
                             , const XMLCh* const value2
                             , MemoryManager* const manager)
{
    try
    {
        XMLDateTime *pDate1 = parse(value1, manager);
        Janitor<XMLDateTime> jName1(pDate1);
        XMLDateTime *pDate2 = parse(value2, manager);
        Janitor<XMLDateTime> jName2(pDate2);
        int result = compareDates(pDate1, pDate2, true);
        return (result==INDETERMINATE)? -1 : result;
    }
    catch(const OutOfMemoryException&)
    {
        throw;
    }
    catch (...) // RuntimeException e
    {
        return -1; // revisit after implement compareDates()
    }

}


void DateTimeValidator::checkContent(const XMLCh*             const content
                                   ,       ValidationContext* const context
                                   ,       bool                     asBase
                                   ,       MemoryManager*     const manager)
{
    //validate against base validator if any
    DateTimeValidator *pBaseValidator = (DateTimeValidator*) this->getBaseValidator();
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

    // the derived classes' parse() method constructs an
    // XMLDateTime object anc invokes appropriate XMLDateTime's
    // parser to parse the content.
    XMLDateTime dateTimeValue(content, manager);
    XMLDateTime* dateTime = &dateTimeValue;
    
    parse(dateTime);

    // must be < MaxExclusive
    if ((thisFacetsDefined & DatatypeValidator::FACET_MAXEXCLUSIVE) != 0)
    {
        if (compareValues(dateTime, getMaxExclusive()) != XMLDateTime::LESS_THAN)
        {
            REPORT_VALUE_ERROR( dateTime
                              , getMaxExclusive()
                              , XMLExcepts::VALUE_exceed_maxExcl
                              , manager)
        }
    } 	

    // must be <= MaxInclusive
    if ((thisFacetsDefined & DatatypeValidator::FACET_MAXINCLUSIVE) != 0)
    {
        int result = compareValues(dateTime, getMaxInclusive());
        if ( result == XMLDateTime::GREATER_THAN || result == XMLDateTime::INDETERMINATE )
        {
            REPORT_VALUE_ERROR( dateTime
                              , getMaxInclusive()
                              , XMLExcepts::VALUE_exceed_maxIncl
                              , manager)
        }
    }

    // must be >= MinInclusive
    if ((thisFacetsDefined & DatatypeValidator::FACET_MININCLUSIVE) != 0)
    {
        int result = compareValues(dateTime, getMinInclusive());
        if (result == XMLDateTime::LESS_THAN || result == XMLDateTime::INDETERMINATE)
        {
            REPORT_VALUE_ERROR( dateTime
                              , getMinInclusive()
                              , XMLExcepts::VALUE_exceed_minIncl
                              , manager)
        }
    }

    // must be > MinExclusive
    if ( (thisFacetsDefined & DatatypeValidator::FACET_MINEXCLUSIVE) != 0 )
    {
        if (compareValues(dateTime, getMinExclusive()) != XMLDateTime::GREATER_THAN)
        {
            REPORT_VALUE_ERROR( dateTime
                              , getMinExclusive()
                              , XMLExcepts::VALUE_exceed_minExcl
                              , manager)
        }
    }

    if ((thisFacetsDefined & DatatypeValidator::FACET_ENUMERATION) != 0 &&
        (getEnumeration() != 0))
    {
        XMLSize_t i=0;
        XMLSize_t enumLength = getEnumeration()->size();
        for ( ; i < enumLength; i++)
        {
            if (compareValues(dateTime, getEnumeration()->elementAt(i)) == XMLDateTime::EQUAL)
                break;
        }

        if (i == enumLength)
            ThrowXMLwithMemMgr1(InvalidDatatypeValueException, XMLExcepts::VALUE_NotIn_Enumeration, content, manager);
    }
}

//
// Comparision methods
//
int DateTimeValidator::compareValues(const XMLNumber* const lValue
                                   , const XMLNumber* const rValue)
{
    return compareDates((XMLDateTime*) lValue, (XMLDateTime*) rValue, true);
}

/**
 * Compare algorithm described in dateDime (3.2.7).
 * Duration datatype overwrites this method
 *
 * @param date1  normalized date representation of the first value
 * @param date2  normalized date representation of the second value
 * @param strict
 * @return less, greater, less_equal, greater_equal, equal
 */
int DateTimeValidator::compareDates(const XMLDateTime* const date1
                                  , const XMLDateTime* const date2
                                  , bool)
{
    return XMLDateTime::compare(date1, date2);
}

//
// In fact, the proper way of the following set*() shall be
// {
// if (fMaxInclusive)
//     delete fMaxInclusive;
//
//    fMaxInclusive = parse(value);
//
// }
//
// But we know this function is invoked once and only once
// since there is no duplicated facet passed in, therefore
// fMaxInclusive is alwasy zero before, so for the
// sake of performance, we do not do the checking/delete.
//

void DateTimeValidator::setMaxInclusive(const XMLCh* const value)
{
    fMaxInclusive = parse(value, fMemoryManager);
}

void DateTimeValidator::setMaxExclusive(const XMLCh* const value)
{
    fMaxExclusive = parse(value, fMemoryManager);
}

void DateTimeValidator::setMinInclusive(const XMLCh* const value)
{
    fMinInclusive = parse(value, fMemoryManager);
}

void DateTimeValidator::setMinExclusive(const XMLCh* const value)
{
    fMinExclusive = parse(value, fMemoryManager);
}

void DateTimeValidator::setEnumeration(MemoryManager* const)
{
// to do: do we need to check against base value space???

    if (!fStrEnumeration)
        return;

    XMLSize_t enumLength = fStrEnumeration->size();
    fEnumeration = new (fMemoryManager) RefVectorOf<XMLNumber>(enumLength, true, fMemoryManager);
    fEnumerationInherited = false;

    for ( XMLSize_t i = 0; i < enumLength; i++)
        fEnumeration->insertElementAt(parse(fStrEnumeration->elementAt(i), fMemoryManager), i);

}

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_NOCREATE(DateTimeValidator)

void DateTimeValidator::serialize(XSerializeEngine& serEng)
{
    /***
     *
     * Note: All its derivatives share the same number type, that is
     *       XMLNumber::DateTime, so this class would write it.
     ***/

    if (serEng.isStoring())
    {
        serEng<<(int) XMLNumber::DateTime;
    }

    AbstractNumericFacetValidator::serialize(serEng);

    //dateTime can be instantiated during checkContent(), so don't serialize it.
}

XERCES_CPP_NAMESPACE_END

/**
  * End of file DateTimeValidator::cpp
  */

