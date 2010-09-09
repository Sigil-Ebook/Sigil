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
 * $Id: AbstractNumericFacetValidator.cpp 471747 2006-11-06 14:31:56Z amassari $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/datatype/AbstractNumericFacetValidator.hpp>
#include <xercesc/validators/datatype/InvalidDatatypeFacetException.hpp>
#include <xercesc/util/NumberFormatException.hpp>

//since we need to dynamically created each and every derivatives 
//during deserialization by XSerializeEngine>>Derivative, we got
//to include all hpp
#include <xercesc/util/XMLFloat.hpp>
#include <xercesc/util/XMLDouble.hpp>
#include <xercesc/util/XMLBigDecimal.hpp>
#include <xercesc/util/XMLDateTime.hpp>
#include <xercesc/internal/XTemplateSerializer.hpp>

XERCES_CPP_NAMESPACE_BEGIN

const int AbstractNumericFacetValidator::INDETERMINATE = 2;

#define  REPORT_FACET_ERROR(val1, val2, except_code, manager)    \
  ThrowXMLwithMemMgr2(InvalidDatatypeFacetException               \
          , except_code                                 \
          , val1->getFormattedString()                  \
          , val2->getFormattedString()                  \
          , manager);

#define  FROM_BASE_VALUE_SPACE(val, facetFlag, except_code, manager)   \
  if ((thisFacetsDefined & facetFlag) != 0)                   \
{                                                             \
    try                                                       \
{                                                             \
        numBase->checkContent(val->getRawData(), (ValidationContext*)0, false, manager);      \
}                                                             \
    catch ( XMLException& )                                   \
{                                                             \
        ThrowXMLwithMemMgr1(InvalidDatatypeFacetException               \
                , except_code                                 \
                , val->getRawData()                           \
                , manager);                                   \
}                                                             \
}


// ---------------------------------------------------------------------------
//  Constructors and Destructor
// ---------------------------------------------------------------------------
AbstractNumericFacetValidator::~AbstractNumericFacetValidator()
{
    if (!fMaxInclusiveInherited && fMaxInclusive)
        delete fMaxInclusive;

    if (!fMaxExclusiveInherited && fMaxExclusive)
        delete fMaxExclusive;

    if (!fMinInclusiveInherited && fMinInclusive)
        delete fMinInclusive;

    if (!fMinExclusiveInherited && fMinExclusive)
        delete fMinExclusive;

    //~RefVectorOf will delete all adopted elements
    if (!fEnumerationInherited &&  fEnumeration)
        delete fEnumeration;

    if (!fEnumerationInherited &&  fStrEnumeration)
        delete fStrEnumeration;
}

AbstractNumericFacetValidator::AbstractNumericFacetValidator(
                          DatatypeValidator*            const baseValidator
                        , RefHashTableOf<KVStringPair>* const facets
                        , const int                           finalSet
                        , const ValidatorType                 type
                        , MemoryManager* const                manager)
:DatatypeValidator(baseValidator, facets, finalSet, type, manager)
, fMaxInclusiveInherited(false)
, fMaxExclusiveInherited(false)
, fMinInclusiveInherited(false)
, fMinExclusiveInherited(false)
, fEnumerationInherited(false)
, fMaxInclusive(0)
, fMaxExclusive(0)
, fMinInclusive(0)
, fMinExclusive(0)
, fEnumeration(0)
, fStrEnumeration(0)
{
    //do not invoke init() here !!!
}

//
//  P1. Enumeration
//
void AbstractNumericFacetValidator::init(RefArrayVectorOf<XMLCh>* const enums
                                         , MemoryManager* const manager)
{

    fStrEnumeration = enums; // save the literal value
	                         // which is needed for getEnumString()

    if (enums)
    {
        setFacetsDefined(DatatypeValidator::FACET_ENUMERATION);
    }

    assignFacet(manager);
    inspectFacet(manager);
    inspectFacetBase(manager);
    inheritFacet();
}

//
//   Assign facets
//        assign common facets
//        assign additional facet
//
void AbstractNumericFacetValidator::assignFacet(MemoryManager* const manager)
{

    RefHashTableOf<KVStringPair>* facets = getFacets();

    if (!facets)     // no facets defined
        return;

    XMLCh* key;

    RefHashTableOfEnumerator<KVStringPair> e(facets, false, manager);

    while (e.hasMoreElements())
    {
        KVStringPair pair = e.nextElement();
        key = pair.getKey();
        XMLCh* value = pair.getValue();

        if (XMLString::equals(key, SchemaSymbols::fgELT_PATTERN))
        {
            setPattern(value);
            if (getPattern())
                setFacetsDefined(DatatypeValidator::FACET_PATTERN);
            // do not construct regex until needed
        }
        else if (XMLString::equals(key, SchemaSymbols::fgELT_MAXINCLUSIVE))
        {
            try
            {
                setMaxInclusive(value);
            }
            catch (NumberFormatException&)
            {
                ThrowXMLwithMemMgr1(InvalidDatatypeFacetException, XMLExcepts::FACET_Invalid_MaxIncl, value, manager);
            }
            setFacetsDefined(DatatypeValidator::FACET_MAXINCLUSIVE);
        }
        else if (XMLString::equals(key, SchemaSymbols::fgELT_MAXEXCLUSIVE))
        {
            try
            {
                setMaxExclusive(value);
            }
            catch (NumberFormatException&)
            {
                ThrowXMLwithMemMgr1(InvalidDatatypeFacetException, XMLExcepts::FACET_Invalid_MaxExcl, value, manager);
            }
            setFacetsDefined(DatatypeValidator::FACET_MAXEXCLUSIVE);
        }
        else if (XMLString::equals(key, SchemaSymbols::fgELT_MININCLUSIVE))
        {
            try
            {
                setMinInclusive(value);
            }
            catch (NumberFormatException&)
            {
                ThrowXMLwithMemMgr1(InvalidDatatypeFacetException, XMLExcepts::FACET_Invalid_MinIncl, value, manager);
            }
            setFacetsDefined(DatatypeValidator::FACET_MININCLUSIVE);
        }
        else if (XMLString::equals(key, SchemaSymbols::fgELT_MINEXCLUSIVE))
        {
            try
            {
                setMinExclusive(value);
            }
            catch (NumberFormatException&)
            {
                ThrowXMLwithMemMgr1(InvalidDatatypeFacetException, XMLExcepts::FACET_Invalid_MinExcl, value, manager);
            }
            setFacetsDefined(DatatypeValidator::FACET_MINEXCLUSIVE);
        }
        else if (XMLString::equals(key, SchemaSymbols::fgATT_FIXED))
        {
            unsigned int val;
            bool         retStatus;
            try
            {
                retStatus = XMLString::textToBin(value, val, fMemoryManager);
            }
            catch (RuntimeException&)
            {
                ThrowXMLwithMemMgr(InvalidDatatypeFacetException, XMLExcepts::FACET_internalError_fixed, manager);
            }

            if (!retStatus)
            {
                ThrowXMLwithMemMgr(InvalidDatatypeFacetException, XMLExcepts::FACET_internalError_fixed, manager);
            }

            setFixed(val);
            //no setFacetsDefined here

        }
        else
        {
            assignAdditionalFacet(key, value, manager);
        }

    }//while

}// end of assigneFacet()

//
// Check facet among self
//         check common facets
//         check Additional Facet Constraint
//
void AbstractNumericFacetValidator::inspectFacet(MemoryManager* const manager)
{

    int thisFacetsDefined = getFacetsDefined();
    XMLNumber *thisMaxInclusive = getMaxInclusive();
    XMLNumber *thisMaxExclusive = getMaxExclusive();
    XMLNumber *thisMinInclusive = getMinInclusive();
    XMLNumber *thisMinExclusive = getMinExclusive();

    if (!thisFacetsDefined)
        return;

    // non co-existence checking
    // check 4.3.8.c1 error: maxInclusive + maxExclusive
    if (((thisFacetsDefined & DatatypeValidator::FACET_MAXEXCLUSIVE) != 0) &&
        ((thisFacetsDefined & DatatypeValidator::FACET_MAXINCLUSIVE) != 0) )
        ThrowXMLwithMemMgr(InvalidDatatypeFacetException, XMLExcepts::FACET_max_Incl_Excl, manager);

    // non co-existence checking
    // check 4.3.9.c1 error: minInclusive + minExclusive
    if (((thisFacetsDefined & DatatypeValidator::FACET_MINEXCLUSIVE) != 0) &&
        ((thisFacetsDefined & DatatypeValidator::FACET_MININCLUSIVE) != 0) )
        ThrowXMLwithMemMgr(InvalidDatatypeFacetException, XMLExcepts::FACET_min_Incl_Excl, manager);

    //
    // minExclusive < minInclusive <= maxInclusive < maxExclusive
    //
    // check 4.3.7.c1 must: minInclusive <= maxInclusive
    if (((thisFacetsDefined & DatatypeValidator::FACET_MAXINCLUSIVE) != 0) &&
        ((thisFacetsDefined & DatatypeValidator::FACET_MININCLUSIVE) != 0) )
    {
        int result = compareValues(thisMinInclusive, thisMaxInclusive);
        if ( result == 1 || result == INDETERMINATE )
        {
            REPORT_FACET_ERROR(thisMinInclusive
                             , thisMaxInclusive
                             , XMLExcepts::FACET_maxIncl_minIncl
                             , manager)
        }
    }

    // check 4.3.8.c2 must: minExclusive <= maxExclusive
    if ( ((thisFacetsDefined & DatatypeValidator::FACET_MAXEXCLUSIVE) != 0) &&
        ((thisFacetsDefined & DatatypeValidator::FACET_MINEXCLUSIVE) != 0) )
    {
        int result = compareValues(getMinExclusive(), getMaxExclusive());
        if ( result == 1 || result == INDETERMINATE )
        {
            REPORT_FACET_ERROR(thisMinExclusive
                             , thisMaxExclusive
                             , XMLExcepts::FACET_maxExcl_minExcl
                             , manager)
        }
    }

    // check 4.3.9.c2 must: minExclusive < maxInclusive
    if ( ((thisFacetsDefined & DatatypeValidator::FACET_MAXINCLUSIVE) != 0) &&
        ((thisFacetsDefined & DatatypeValidator::FACET_MINEXCLUSIVE) != 0) )
    {
        int result = compareValues(getMinExclusive(), getMaxInclusive());
        if ( result != -1 )
        {
            REPORT_FACET_ERROR(thisMinExclusive
                             , thisMaxInclusive
                             , XMLExcepts::FACET_maxIncl_minExcl
                             , manager)
        }
    }

    // check 4.3.10.c1 must: minInclusive < maxExclusive
    if ( ((thisFacetsDefined & DatatypeValidator::FACET_MAXEXCLUSIVE) != 0) &&
        ((thisFacetsDefined & DatatypeValidator::FACET_MININCLUSIVE) != 0) )
    {
        int result = compareValues(getMinInclusive(), getMaxExclusive());
        if ( result != -1)
        {
            REPORT_FACET_ERROR(thisMinInclusive
                             , thisMaxExclusive
                             , XMLExcepts::FACET_maxExcl_minIncl
                             , manager)
        }
    }

    checkAdditionalFacetConstraints(manager);

}// end of inspectFacet()

//
//  Check vs base
//         check common facets
//         check enumeration
//         check Additional Facet Constraint
//
void AbstractNumericFacetValidator::inspectFacetBase(MemoryManager* const manager)
{

    AbstractNumericFacetValidator* numBase = (AbstractNumericFacetValidator*) getBaseValidator();
    int thisFacetsDefined = getFacetsDefined();

    if ( (!thisFacetsDefined && !fEnumeration) ||
         !numBase           )
        return;

    int baseFacetsDefined = numBase->getFacetsDefined();

    XMLNumber *thisMaxInclusive = getMaxInclusive();
    XMLNumber *thisMaxExclusive = getMaxExclusive();
    XMLNumber *thisMinInclusive = getMinInclusive();
    XMLNumber *thisMinExclusive = getMinExclusive();

    XMLNumber *baseMaxInclusive = numBase->getMaxInclusive();
    XMLNumber *baseMaxExclusive = numBase->getMaxExclusive();
    XMLNumber *baseMinInclusive = numBase->getMinInclusive();
    XMLNumber *baseMinExclusive = numBase->getMinExclusive();
    int       baseFixed = numBase->getFixed();

                //                                     this
                //                 minExclusive                          maxExclusive
                //                    minInclusive                  maxInclusive
                //
                //                                     base
                //  minExclusive                                                          maxExclusive
                //      minInclusive                                                   maxInclusive
                //

    // check 4.3.7.c2 error:
    // maxInclusive > base.maxInclusive && maxInclusive != base.maxInclusive if (base.fixed)
    // maxInclusive >= base.maxExclusive
    // maxInclusive < base.minInclusive
    // maxInclusive <= base.minExclusive

    if ((thisFacetsDefined & DatatypeValidator::FACET_MAXINCLUSIVE) != 0)
    {
        if ((baseFacetsDefined & DatatypeValidator::FACET_MAXINCLUSIVE) != 0)
        {
            int result = compareValues(thisMaxInclusive, baseMaxInclusive);

            if (((baseFixed & DatatypeValidator::FACET_MAXINCLUSIVE) != 0) &&
                 (result != 0 ))
            {
                REPORT_FACET_ERROR(thisMaxInclusive
                                 , baseMaxInclusive
                                 , XMLExcepts::FACET_maxIncl_base_fixed
                                 , manager)
            }

            if (result == 1 || result == INDETERMINATE)
            {
                REPORT_FACET_ERROR(thisMaxInclusive
                                 , baseMaxInclusive
                                 , XMLExcepts::FACET_maxIncl_base_maxIncl
                                 , manager)
            }

        }

        if ((baseFacetsDefined & DatatypeValidator::FACET_MAXEXCLUSIVE) != 0)
        {
            int result = compareValues(thisMaxInclusive, baseMaxExclusive);
            if (result != -1 )
            {
                REPORT_FACET_ERROR(thisMaxInclusive
                                 , baseMaxExclusive
                                 , XMLExcepts::FACET_maxIncl_base_maxExcl
                                 , manager)
            }
        }


        if ((baseFacetsDefined & DatatypeValidator::FACET_MININCLUSIVE) != 0)
        {
            int result = compareValues(thisMaxInclusive, baseMinInclusive);
            if (result == -1 || result == INDETERMINATE)
            {
                REPORT_FACET_ERROR(thisMaxInclusive
                                 , baseMinInclusive
                                 , XMLExcepts::FACET_maxIncl_base_minIncl
                                 , manager)
            }
        }

        if ((baseFacetsDefined & DatatypeValidator::FACET_MINEXCLUSIVE) != 0)
        {
            int result = compareValues(thisMaxInclusive, baseMinExclusive);
            if (result != 1 )
            {
                REPORT_FACET_ERROR(thisMaxInclusive
                                 , baseMinExclusive
                                 , XMLExcepts::FACET_maxIncl_base_minExcl
                                 , manager)
            }
        }

    }

    // check 4.3.8.c3 error:
    // maxExclusive > base.maxExclusive  && maxExclusive != base.maxExclusive if (base.fixed)
    // maxExclusive > base.maxInclusive
    // maxExclusive <= base.minInclusive
    // maxExclusive <= base.minExclusive

    if ((thisFacetsDefined & DatatypeValidator::FACET_MAXEXCLUSIVE) != 0)
    {
        if (( baseFacetsDefined & DatatypeValidator::FACET_MAXEXCLUSIVE) != 0)
        {
            int result = compareValues(thisMaxExclusive, baseMaxExclusive);

            if (((baseFixed & DatatypeValidator::FACET_MAXEXCLUSIVE) != 0) &&
                 (result != 0 ))
            {
                REPORT_FACET_ERROR(thisMaxExclusive
                                 , baseMaxExclusive
                                 , XMLExcepts::FACET_maxExcl_base_fixed
                                 , manager)
             }

            if (result == 1 || result == INDETERMINATE)
            {
                REPORT_FACET_ERROR(thisMaxExclusive
                                 , baseMaxExclusive
                                 , XMLExcepts::FACET_maxExcl_base_maxExcl
                                 , manager)
            }

            /**
             * Schema Errata
             * E2-16 maxExclusive
             *
             *   derived type's maxExclusive must either be
             *   1) equal to base' maxExclusive or
             *   2) from the base type value space
             *
             */
            if (result != 0)
            {
                FROM_BASE_VALUE_SPACE(thisMaxExclusive
                        , DatatypeValidator::FACET_MAXEXCLUSIVE
                        , XMLExcepts::FACET_maxExcl_notFromBase
                        , manager)
            }
        }
        else  // base has no maxExclusive
        {
            FROM_BASE_VALUE_SPACE(thisMaxExclusive
                        , DatatypeValidator::FACET_MAXEXCLUSIVE
                        , XMLExcepts::FACET_maxExcl_notFromBase
                        , manager)
        }

        if (( baseFacetsDefined & DatatypeValidator::FACET_MAXINCLUSIVE) != 0)
        {
            int result = compareValues(thisMaxExclusive, baseMaxInclusive);
            if (result == 1 || result == INDETERMINATE)
            {
                REPORT_FACET_ERROR(thisMaxExclusive
                                 , baseMaxInclusive
                                 , XMLExcepts::FACET_maxExcl_base_maxIncl
                                 , manager)
            }
        }

        if (( baseFacetsDefined & DatatypeValidator::FACET_MINEXCLUSIVE) != 0)
        {
            int result = compareValues(thisMaxExclusive, baseMinExclusive);
            if (result != 1)
            {
                REPORT_FACET_ERROR(thisMaxExclusive
                                 , baseMinExclusive
                                 , XMLExcepts::FACET_maxExcl_base_minExcl
                                 , manager)
            }
        }

        if (( baseFacetsDefined & DatatypeValidator::FACET_MININCLUSIVE) != 0)
        {
            int result = compareValues(thisMaxExclusive, baseMinInclusive);
            if (result != 1)
            {
                REPORT_FACET_ERROR(thisMaxExclusive
                                 , baseMinInclusive
                                 , XMLExcepts::FACET_maxExcl_base_minIncl
                                 , manager)
            }
        }
    }

    // check 4.3.9.c3 error:
    // minExclusive < base.minExclusive     minExclusive != base.minExclusive if (base.fixed)
    // minExclusive > base.maxInclusive ??? minExclusive >= base.maxInclusive
    // minExclusive < base.minInclusive
    // minExclusive >= base.maxExclusive

    if ((thisFacetsDefined & DatatypeValidator::FACET_MINEXCLUSIVE) != 0)
    {
        if (( baseFacetsDefined & DatatypeValidator::FACET_MINEXCLUSIVE) != 0)
        {
            int result = compareValues(thisMinExclusive, baseMinExclusive);

            if (((baseFixed & DatatypeValidator::FACET_MINEXCLUSIVE) != 0) &&
                 (result != 0 ))
            {
                REPORT_FACET_ERROR(thisMinExclusive
                                 , baseMinExclusive
                                 , XMLExcepts::FACET_minExcl_base_fixed
                                 , manager)
            }

            if (result == -1 || result == INDETERMINATE)
            {
                REPORT_FACET_ERROR(thisMinExclusive
                                 , baseMinExclusive
                                 , XMLExcepts::FACET_minExcl_base_minExcl
                                 , manager)
            }

            /**
             * Schema Errata
             * E2-16 maxExclusive
             *
             *   derived type's minExclusive must either be
             *   1) equal to base' minxExclusive or
             *   2) from the base type value space
             *
             *  Note: deduced from, not explicitly expressed in the Errata
             */
            if (result != 0)
            {
                FROM_BASE_VALUE_SPACE(thisMinExclusive
                        , DatatypeValidator::FACET_MINEXCLUSIVE
                        , XMLExcepts::FACET_minExcl_notFromBase
                        , manager)
            }
        }
        else // base has no minExclusive
        {

            FROM_BASE_VALUE_SPACE(thisMinExclusive
                        , DatatypeValidator::FACET_MINEXCLUSIVE
                        , XMLExcepts::FACET_minExcl_notFromBase
                        , manager)
        }

        if (( baseFacetsDefined & DatatypeValidator::FACET_MAXINCLUSIVE) != 0)
        {
            int result = compareValues(thisMinExclusive, baseMaxInclusive);
            if (result == 1 || result == INDETERMINATE)
            {
                REPORT_FACET_ERROR(thisMinExclusive
                                 , baseMaxInclusive
                                 , XMLExcepts::FACET_minExcl_base_maxIncl
                                 , manager)
            }
        }

        if (( baseFacetsDefined & DatatypeValidator::FACET_MININCLUSIVE) != 0)
        {
            int result = compareValues(thisMinExclusive, baseMinInclusive);
            if (result == -1 || result == INDETERMINATE)
            {
                REPORT_FACET_ERROR(thisMinExclusive
                                 , baseMinInclusive
                                 , XMLExcepts::FACET_minExcl_base_minIncl
                                 , manager)
            }
        }

        if (( baseFacetsDefined & DatatypeValidator::FACET_MAXEXCLUSIVE) != 0)
        {
            int result = compareValues(thisMinExclusive, baseMaxExclusive);
            if (result != -1)
            {
                REPORT_FACET_ERROR(thisMinExclusive
                                 , baseMaxExclusive
                                 , XMLExcepts::FACET_minExcl_base_maxExcl
                                 , manager)
            }
        }

    }

    // check 4.3.10.c2 error:
    // minInclusive < base.minInclusive   minInclusive != base.minInclusive if (base.fixed)
    // minInclusive > base.maxInclusive
    // minInclusive <= base.minExclusive
    // minInclusive >= base.maxExclusive


    if ((thisFacetsDefined & DatatypeValidator::FACET_MININCLUSIVE) != 0)
    {
        if ((baseFacetsDefined & DatatypeValidator::FACET_MININCLUSIVE) != 0)
        {
            int result = compareValues(thisMinInclusive, baseMinInclusive);

            if (((baseFixed & DatatypeValidator::FACET_MININCLUSIVE) != 0) &&
                 (result != 0 ))
            {
                REPORT_FACET_ERROR(thisMinInclusive
                                 , baseMinInclusive
                                 , XMLExcepts::FACET_minIncl_base_fixed
                                 , manager)
            }

            if (result == -1 || result == INDETERMINATE)
            {
                REPORT_FACET_ERROR(thisMinInclusive
                                 , baseMinInclusive
                                 , XMLExcepts::FACET_minIncl_base_minIncl
                                 , manager)
            }
        }

        if (( baseFacetsDefined & DatatypeValidator::FACET_MAXINCLUSIVE) != 0)
        {
            int result = compareValues(thisMinInclusive, baseMaxInclusive);
            if (result == 1 || result == INDETERMINATE)
            {
                REPORT_FACET_ERROR(thisMinInclusive
                                 , baseMaxInclusive
                                 , XMLExcepts::FACET_minIncl_base_maxIncl
                                 , manager)
            }
        }

        if (( baseFacetsDefined & DatatypeValidator::FACET_MINEXCLUSIVE) != 0)
        {
            int result = compareValues(thisMinInclusive, baseMinExclusive);
            if (result != 1)
            {
                REPORT_FACET_ERROR(thisMinInclusive
                                 , baseMinExclusive
                                 , XMLExcepts::FACET_minIncl_base_minExcl
                                 , manager)
            }
        }

        if (( baseFacetsDefined & DatatypeValidator::FACET_MAXEXCLUSIVE) != 0)
        {
            int result = compareValues(thisMinInclusive, baseMaxExclusive);
            if (result != -1)
            {
                REPORT_FACET_ERROR(thisMinInclusive
                                 , baseMaxExclusive
                                 , XMLExcepts::FACET_minIncl_base_maxExcl
                                 , manager)
            }
        }

    }

    checkAdditionalFacetConstraintsBase(manager);

    // check 4.3.5.c0 must: enumeration values from the value space of base
    //
    // In fact, the values in the enumeration shall go through validation
    // of this class as well.
    // this->checkContent(value, false);
    //
    if ( ((thisFacetsDefined & DatatypeValidator::FACET_ENUMERATION) != 0) &&
        ( fStrEnumeration ))
    {
        setEnumeration(manager);
    }

    //
    // maxInclusive, maxExclusive, minInclusive and minExclusive
    // shall come from the base's value space as well
    //

    FROM_BASE_VALUE_SPACE(thisMaxInclusive
                        , DatatypeValidator::FACET_MAXINCLUSIVE
                        , XMLExcepts::FACET_maxIncl_notFromBase
                        , manager)

    FROM_BASE_VALUE_SPACE(thisMinInclusive
                        , DatatypeValidator::FACET_MININCLUSIVE
                        , XMLExcepts::FACET_minIncl_notFromBase
                        , manager)

} //end of inspectFacetBase

//
//  Inherit facet from base
//    a. inherit common facets
//    b. inherit additional facet
//
void AbstractNumericFacetValidator::inheritFacet()
{

    AbstractNumericFacetValidator* numBase = (AbstractNumericFacetValidator*) getBaseValidator();
    if (!numBase)
        return;

    int thisFacetsDefined = getFacetsDefined();
    int baseFacetsDefined = numBase->getFacetsDefined();

    // inherit enumeration
    if ((( baseFacetsDefined & DatatypeValidator::FACET_ENUMERATION) != 0) &&
        (( thisFacetsDefined & DatatypeValidator::FACET_ENUMERATION) == 0))
    {
        fEnumeration = numBase->fEnumeration;
        fEnumerationInherited = true;
        setFacetsDefined(DatatypeValidator::FACET_ENUMERATION);
    }

    // inherit maxInclusive
    if ((( baseFacetsDefined & DatatypeValidator::FACET_MAXINCLUSIVE) != 0) &&
        (( thisFacetsDefined & DatatypeValidator::FACET_MAXEXCLUSIVE) == 0) &&
        (( thisFacetsDefined & DatatypeValidator::FACET_MAXINCLUSIVE) == 0) )
    {
        fMaxInclusive = numBase->getMaxInclusive();
        fMaxInclusiveInherited = true;
        setFacetsDefined(DatatypeValidator::FACET_MAXINCLUSIVE);
    }

    // inherit maxExclusive
    if ((( baseFacetsDefined & DatatypeValidator::FACET_MAXEXCLUSIVE) != 0) &&
        (( thisFacetsDefined & DatatypeValidator::FACET_MAXEXCLUSIVE) == 0) &&
        (( thisFacetsDefined & DatatypeValidator::FACET_MAXINCLUSIVE) == 0) )
    {
        fMaxExclusive = numBase->getMaxExclusive();
        fMaxExclusiveInherited = true;
        setFacetsDefined(DatatypeValidator::FACET_MAXEXCLUSIVE);
    }

    // inherit minExclusive
    if ((( baseFacetsDefined & DatatypeValidator::FACET_MININCLUSIVE) != 0) &&
        (( thisFacetsDefined & DatatypeValidator::FACET_MINEXCLUSIVE) == 0) &&
        (( thisFacetsDefined & DatatypeValidator::FACET_MININCLUSIVE) == 0) )
    {
        fMinInclusive = numBase->getMinInclusive();
        fMinInclusiveInherited = true;
        setFacetsDefined(DatatypeValidator::FACET_MININCLUSIVE);
    }

    // inherit minExclusive
    if ((( baseFacetsDefined & DatatypeValidator::FACET_MINEXCLUSIVE) != 0) &&
        (( thisFacetsDefined & DatatypeValidator::FACET_MINEXCLUSIVE) == 0) &&
        (( thisFacetsDefined & DatatypeValidator::FACET_MININCLUSIVE) == 0) )
    {
        fMinExclusive = numBase->getMinExclusive();
        fMinExclusiveInherited = true;
        setFacetsDefined(DatatypeValidator::FACET_MINEXCLUSIVE);
    }

    inheritAdditionalFacet();

    // inherit "fixed" option
    setFixed(getFixed() | numBase->getFixed());

}

const RefArrayVectorOf<XMLCh>* AbstractNumericFacetValidator::getEnumString() const
{
	return (fEnumerationInherited? getBaseValidator()->getEnumString() : fStrEnumeration );
}


void AbstractNumericFacetValidator::checkAdditionalFacetConstraints(MemoryManager* const) const
{
    return;
}

void AbstractNumericFacetValidator::checkAdditionalFacetConstraintsBase(MemoryManager* const) const
{
    return;
}

void AbstractNumericFacetValidator::inheritAdditionalFacet()
{
    return;
}

void AbstractNumericFacetValidator::assignAdditionalFacet( const XMLCh* const key
                                                   , const XMLCh* const
                                                   , MemoryManager* const manager)
{
    ThrowXMLwithMemMgr1(InvalidDatatypeFacetException
            , XMLExcepts::FACET_Invalid_Tag
            , key
            , manager);
}



/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_NOCREATE(AbstractNumericFacetValidator)

/***
 *  This dv needs to serialize/deserialize four boundary data members
 *  which are derivatives of XMLlNumber.
 *  The derivatives of this class, namely, Deciamldv, Doubledv, Floatdv and
 *  DateTimedv needs to write a typeEnum into the binary data stream, so
 *  during loading, this method reads the typeEnum first, and then instantiate
 *  the right type of objects, say XMLDouble, XMLFloat, XMLBigDecimal and 
 *  XMLDateTime.
 *
 *  
 ***/
void AbstractNumericFacetValidator::serialize(XSerializeEngine& serEng)
{

    if (serEng.isStoring())
    {

        /***
         * don't move this line out of the if statement,
         * it is done purposely to allow AbstractNumericFacetValidator
         * read the number type information before DatatypeValidator
         * during loading
         ***/
        DatatypeValidator::serialize(serEng);

        // need not write type info for the XMLNumber since
        // the derivative class has done that       
        storeClusive(serEng, fMaxInclusiveInherited, fMaxInclusive);
        storeClusive(serEng, fMaxExclusiveInherited, fMaxExclusive);
        storeClusive(serEng, fMinInclusiveInherited, fMinInclusive);
        storeClusive(serEng, fMinExclusiveInherited, fMinExclusive);

        serEng<<fEnumerationInherited;

        /***
         * Serialize RefArrayVectorOf<XMLCh>
         * Serialize RefVectorOf<XMLNumber>
         ***/
        XTemplateSerializer::storeObject(fStrEnumeration, serEng);
        XTemplateSerializer::storeObject(fEnumeration, serEng);
   
    }
    else
    {
        // Read the number type info for the XMLNumber FIRST!!!
        int                     nType;
        XMLNumber::NumberType   numType;
        serEng>>nType;
        numType = (XMLNumber::NumberType) nType;

        DatatypeValidator::serialize(serEng);

        loadClusive(serEng, fMaxInclusiveInherited, fMaxInclusive, numType, 1);
        loadClusive(serEng, fMaxExclusiveInherited, fMaxExclusive, numType, 2);
        loadClusive(serEng, fMinInclusiveInherited, fMinInclusive, numType, 3);
        loadClusive(serEng, fMinExclusiveInherited, fMinExclusive, numType, 4);

        serEng>>fEnumerationInherited;

        /***
         *  Deserialize RefArrayVectorOf<XMLCh>         
         *  Deserialize RefVectorOf<XMLNumber>   
         ***/
        XTemplateSerializer::loadObject(&fStrEnumeration, 8, true, serEng);
        XTemplateSerializer::loadObject(&fEnumeration, 8, true, numType, serEng);

    }

}

//
// A user defined dv may inherit any of the Max/Min/Inc/Exc from a
// built dv, which will create its own Max/Min/Inc/Exc during the
// loading. Therefore if the user defined store and load this 
// facet, and does not own it, that will cause leakage.
//
// To avoid checking if the facet belongs to a builtIn dv or not, we
// do this way, for any inherited *clusive, we will not store it, and later 
// on during loading, we get it from the base dv.
//
void AbstractNumericFacetValidator::storeClusive(XSerializeEngine&       serEng
                                               , bool                    inherited
                                               , XMLNumber*              data)
{
    serEng<<inherited;

    //store only if own it
    if (!inherited)
        serEng<<data;

}

// it is guranteed that the base dv is loaded before this dv
//
void AbstractNumericFacetValidator::loadClusive(XSerializeEngine&       serEng
                                              , bool&                   inherited
                                              , XMLNumber*&             data
                                              , XMLNumber::NumberType   numType
                                              , int                     flag)
{
    serEng>>inherited;

    if (!inherited)
        data = XMLNumber::loadNumber(numType, serEng);
    else
    {
        AbstractNumericFacetValidator* basedv = (AbstractNumericFacetValidator*) getBaseValidator();

        switch(flag)
        {
        case 1: 
            data = basedv->getMaxInclusive();
            break;
        case 2:
            data = basedv->getMaxExclusive();
            break;
        case 3:
            data = basedv->getMinInclusive();
            break;
        case 4:
            data = basedv->getMinExclusive();
            break;
        default:
            break;
        }

    }

}

XERCES_CPP_NAMESPACE_END

/**
  * End of file AbstractNumericFacetValidator::cpp
  */
