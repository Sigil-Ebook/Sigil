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
 * $Id: DecimalDatatypeValidator.cpp 932887 2010-04-11 13:04:59Z borisk $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/datatype/DecimalDatatypeValidator.hpp>
#include <xercesc/validators/datatype/XMLCanRepGroup.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xercesc/validators/datatype/InvalidDatatypeFacetException.hpp>
#include <xercesc/validators/datatype/InvalidDatatypeValueException.hpp>

#include <xercesc/validators/datatype/DatatypeValidatorFactory.hpp>
#include <xercesc/util/NumberFormatException.hpp>
#include <xercesc/util/XMLBigDecimal.hpp>
#include <xercesc/util/XMLBigInteger.hpp>

XERCES_CPP_NAMESPACE_BEGIN

static const int BUF_LEN = 64;

// ---------------------------------------------------------------------------
//  Constructors and Destructor
// ---------------------------------------------------------------------------
DecimalDatatypeValidator::DecimalDatatypeValidator(MemoryManager* const manager)
:AbstractNumericValidator(0, 0, 0, DatatypeValidator::Decimal, manager)
, fTotalDigits(0)
, fFractionDigits(0)
{
    setOrdered(XSSimpleTypeDefinition::ORDERED_TOTAL);
    setNumeric(true);
}

DecimalDatatypeValidator::DecimalDatatypeValidator(
                          DatatypeValidator*            const baseValidator
                        , RefHashTableOf<KVStringPair>* const facets
                        , RefArrayVectorOf<XMLCh>*      const enums
                        , const int                           finalSet
                        , MemoryManager*                const manager)
:AbstractNumericValidator(baseValidator, facets, finalSet, DatatypeValidator::Decimal, manager)
, fTotalDigits(0)
, fFractionDigits(0)
{
    init(enums, manager);
}

DecimalDatatypeValidator::~DecimalDatatypeValidator()
{
}

// -----------------------------------------------------------------------
// Compare methods
// -----------------------------------------------------------------------
int DecimalDatatypeValidator::compare(const XMLCh* const lValue
                                    , const XMLCh* const rValue
                                    , MemoryManager* const manager)
{
    XMLBigDecimal lObj(lValue, manager);
    XMLBigDecimal rObj(rValue, manager);

    return compareValues(&lObj, &rObj);
}

DatatypeValidator* DecimalDatatypeValidator::newInstance
(
      RefHashTableOf<KVStringPair>* const facets
    , RefArrayVectorOf<XMLCh>* const      enums
    , const int                           finalSet
    , MemoryManager* const                manager
)
{
    return (DatatypeValidator*) new (manager) DecimalDatatypeValidator(this, facets, enums, finalSet, manager);
}

// -----------------------------------------------------------------------
// ctor provided to be used by derived classes
// -----------------------------------------------------------------------
DecimalDatatypeValidator::DecimalDatatypeValidator(DatatypeValidator*            const baseValidator
                                                 , RefHashTableOf<KVStringPair>* const facets
                                                 , const int                           finalSet
                                                 , const ValidatorType                 type
                                                 , MemoryManager* const                manager)
:AbstractNumericValidator(baseValidator, facets, finalSet, type, manager)
, fTotalDigits(0)
, fFractionDigits(0)
{
    //do not invoke init here !!!
}

void DecimalDatatypeValidator::assignAdditionalFacet(const XMLCh* const key
                                                   , const XMLCh* const value
                                                   , MemoryManager* const manager)
{
    if (XMLString::equals(key, SchemaSymbols::fgELT_TOTALDIGITS))
    {
        int val;
        try
        {
            val = XMLString::parseInt(value, manager);
        }
        catch (NumberFormatException&)
        {
            ThrowXMLwithMemMgr1(InvalidDatatypeFacetException, XMLExcepts::FACET_Invalid_TotalDigit, value, manager);
        }

        // check 4.3.11.c0 must: totalDigits > 0
        if ( val <= 0 )
            ThrowXMLwithMemMgr1(InvalidDatatypeFacetException, XMLExcepts::FACET_PosInt_TotalDigit, value, manager);

        setTotalDigits(val);
        setFacetsDefined(DatatypeValidator::FACET_TOTALDIGITS);
    }
    else if (XMLString::equals(key, SchemaSymbols::fgELT_FRACTIONDIGITS))
    {
        int val;
        try
        {
            val = XMLString::parseInt(value, manager);
        }
        catch (NumberFormatException&)
        {
            ThrowXMLwithMemMgr1(InvalidDatatypeFacetException, XMLExcepts::FACET_Invalid_FractDigit, value, manager);
        }

        // check 4.3.12.c0 must: fractionDigits > 0
        if ( val < 0 )
            ThrowXMLwithMemMgr1(InvalidDatatypeFacetException, XMLExcepts::FACET_NonNeg_FractDigit, value, manager);

        setFractionDigits(val);
        setFacetsDefined(DatatypeValidator::FACET_FRACTIONDIGITS);
    }
    else
    {
        ThrowXMLwithMemMgr1(InvalidDatatypeFacetException
                , XMLExcepts::FACET_Invalid_Tag
                , key
                , manager);
    }
}

void DecimalDatatypeValidator::inheritAdditionalFacet()
{

    DecimalDatatypeValidator *numBase = (DecimalDatatypeValidator*) getBaseValidator();

    if (!numBase)
        return;

    int thisFacetsDefined = getFacetsDefined();
    int baseFacetsDefined = numBase->getFacetsDefined();

    // inherit totalDigits
    if ((( baseFacetsDefined & DatatypeValidator::FACET_TOTALDIGITS) != 0) &&
        (( thisFacetsDefined & DatatypeValidator::FACET_TOTALDIGITS) == 0) )
    {
        setTotalDigits(numBase->fTotalDigits);
        setFacetsDefined(DatatypeValidator::FACET_TOTALDIGITS);
    }

    // inherit fractionDigits
    if ((( baseFacetsDefined & DatatypeValidator::FACET_FRACTIONDIGITS) != 0) &&
        (( thisFacetsDefined & DatatypeValidator::FACET_FRACTIONDIGITS) == 0) )
    {
        setFractionDigits(numBase->fFractionDigits);
        setFacetsDefined(DatatypeValidator::FACET_FRACTIONDIGITS);
    }
}

void DecimalDatatypeValidator::checkAdditionalFacetConstraints(MemoryManager* const manager) const
{
    int thisFacetsDefined = getFacetsDefined();

    // check 4.3.12.c1 must: fractionDigits <= totalDigits
    if ( ((thisFacetsDefined & DatatypeValidator::FACET_FRACTIONDIGITS) != 0) &&
         ((thisFacetsDefined & DatatypeValidator::FACET_TOTALDIGITS) != 0) )
    {
        if ( fFractionDigits > fTotalDigits )
        {
            XMLCh value1[BUF_LEN+1];
            XMLCh value2[BUF_LEN+1];
            XMLString::binToText(getFractionDigits(), value1, BUF_LEN, 10, manager);
            XMLString::binToText(getTotalDigits(), value2, BUF_LEN, 10, manager);
            ThrowXMLwithMemMgr2(InvalidDatatypeFacetException
                                 , XMLExcepts::FACET_TotDigit_FractDigit
                                 , value2
                                 , value1
                                 , manager);
        }
    }

}

void DecimalDatatypeValidator::checkAdditionalFacetConstraintsBase(MemoryManager* const manager) const
{

    DecimalDatatypeValidator *numBase = (DecimalDatatypeValidator*) getBaseValidator();

    if (!numBase)
        return;

    int thisFacetsDefined = getFacetsDefined();
    int baseFacetsDefined = numBase->getFacetsDefined();

    // check 4.3.11.c1 error: totalDigits > base.totalDigits
    // totalDigits != base.totalDigits if (base.fixed)
    if (( thisFacetsDefined & DatatypeValidator::FACET_TOTALDIGITS) != 0)
    {
        if ( (( baseFacetsDefined & DatatypeValidator::FACET_TOTALDIGITS) != 0) &&
            ( fTotalDigits > numBase->fTotalDigits ))
        {
            XMLCh value1[BUF_LEN+1];
            XMLCh value2[BUF_LEN+1];
            XMLString::binToText(fTotalDigits, value1, BUF_LEN, 10, manager);
            XMLString::binToText(numBase->fTotalDigits, value2, BUF_LEN, 10, manager);
            ThrowXMLwithMemMgr2(InvalidDatatypeFacetException
                                 , XMLExcepts::FACET_totalDigit_base_totalDigit
                                 , value1
                                 , value2
                                 , manager);
        }

        if ( (( baseFacetsDefined & DatatypeValidator::FACET_TOTALDIGITS) != 0) &&
            (( numBase->getFixed() & DatatypeValidator::FACET_TOTALDIGITS) != 0) &&
            ( fTotalDigits != numBase->fTotalDigits ))
        {
            XMLCh value1[BUF_LEN+1];
            XMLCh value2[BUF_LEN+1];
            XMLString::binToText(fTotalDigits, value1, BUF_LEN, 10, manager);
            XMLString::binToText(numBase->fTotalDigits, value2, BUF_LEN, 10, manager);
            ThrowXMLwithMemMgr2(InvalidDatatypeFacetException
                                 , XMLExcepts::FACET_totalDigit_base_fixed
                                 , value1
                                 , value2
                                 , manager);
        }
    }

    if (( thisFacetsDefined & DatatypeValidator::FACET_FRACTIONDIGITS) != 0)
    {
        // check question error: fractionDigits > base.fractionDigits ???
        if ( (( baseFacetsDefined & DatatypeValidator::FACET_FRACTIONDIGITS) != 0) &&
            ( fFractionDigits > numBase->fFractionDigits ))
        {
            XMLCh value1[BUF_LEN+1];
            XMLCh value2[BUF_LEN+1];
            XMLString::binToText(fFractionDigits, value1, BUF_LEN, 10, manager);
            XMLString::binToText(numBase->fFractionDigits, value2, BUF_LEN, 10, manager);
            ThrowXMLwithMemMgr2(InvalidDatatypeFacetException
                                 , XMLExcepts::FACET_fractDigit_base_fractDigit
                                 , value1
                                 , value2
                                 , manager);
                        }

        // check question error: fractionDigits > base.totalDigits ???
        if ( (( baseFacetsDefined & DatatypeValidator::FACET_TOTALDIGITS) != 0) &&
            ( fFractionDigits > numBase->fTotalDigits ))
        {
            XMLCh value1[BUF_LEN+1];
            XMLCh value2[BUF_LEN+1];
            XMLString::binToText(fFractionDigits, value1, BUF_LEN, 10, manager);
            XMLString::binToText(numBase->fTotalDigits, value2, BUF_LEN, 10, manager);
            ThrowXMLwithMemMgr2(InvalidDatatypeFacetException
                                 , XMLExcepts::FACET_fractDigit_base_totalDigit
                                 , value1
                                 , value2
                                 , manager);
        }

        // fractionDigits != base.fractionDigits if (base.fixed)
        if ( (( baseFacetsDefined & DatatypeValidator::FACET_FRACTIONDIGITS) != 0) &&
            (( numBase->getFixed() & DatatypeValidator::FACET_FRACTIONDIGITS) != 0) &&
            ( fFractionDigits != numBase->fFractionDigits ))
        {
            XMLCh value1[BUF_LEN+1];
            XMLCh value2[BUF_LEN+1];
            XMLString::binToText(fFractionDigits, value1, BUF_LEN, 10, manager);
            XMLString::binToText(numBase->fFractionDigits, value2, BUF_LEN, 10, manager);
            ThrowXMLwithMemMgr2(InvalidDatatypeFacetException
                                 , XMLExcepts::FACET_fractDigit_base_fixed
                                 , value1
                                 , value2
                                 , manager);
        }
    }

}

int  DecimalDatatypeValidator::compareValues(const XMLNumber* const lValue
                                           , const XMLNumber* const rValue)
{
    return XMLBigDecimal::compareValues((XMLBigDecimal*) lValue, (XMLBigDecimal*) rValue,
                                        ((XMLBigDecimal*)lValue)->getMemoryManager());
}

void  DecimalDatatypeValidator::setMaxInclusive(const XMLCh* const value)
{
    fMaxInclusive = new (fMemoryManager) XMLBigDecimal(value, fMemoryManager);
}

void  DecimalDatatypeValidator::setMaxExclusive(const XMLCh* const value)
{
    fMaxExclusive = new (fMemoryManager) XMLBigDecimal(value, fMemoryManager);
}

void  DecimalDatatypeValidator::setMinInclusive(const XMLCh* const value)
{
    fMinInclusive = new (fMemoryManager) XMLBigDecimal(value, fMemoryManager);
}

void  DecimalDatatypeValidator::setMinExclusive(const XMLCh* const value)
{
    fMinExclusive = new (fMemoryManager) XMLBigDecimal(value, fMemoryManager);
}

void DecimalDatatypeValidator::setEnumeration(MemoryManager* const manager)
{
    // check 4.3.5.c0 must: enumeration values from the value space of base
    //
    // 1. shall be from base value space
    // 2. shall be from current value space as well ( shall go through boundsCheck() )
    //
    if (!fStrEnumeration)
        return;

    XMLSize_t i = 0;
    XMLSize_t enumLength = fStrEnumeration->size();

    DecimalDatatypeValidator *numBase = (DecimalDatatypeValidator*) getBaseValidator();
    if (numBase)
    {
        try
        {
            for ( i = 0; i < enumLength; i++)
            {
                numBase->checkContent(fStrEnumeration->elementAt(i), (ValidationContext*)0, false, manager);
            }
        }
        catch (XMLException&)
        {
            ThrowXMLwithMemMgr1(InvalidDatatypeFacetException
                    , XMLExcepts::FACET_enum_base
                    , fStrEnumeration->elementAt(i)
                    , manager);
        }
    }
#if 0
// spec says that only base has to checkContent
    // We put the this->checkContent in a separate loop
    // to not block original message with in that method.
    //
    for ( i = 0; i < enumLength; i++)
    {
        checkContent(fStrEnumeration->elementAt(i), (ValidationContext*)0, false, manager);
    }
#endif
    fEnumeration = new (manager) RefVectorOf<XMLNumber>(enumLength, true, manager);
    fEnumerationInherited = false;

    for ( i = 0; i < enumLength; i++)
    {
        fEnumeration->insertElementAt(new (manager) XMLBigDecimal(fStrEnumeration->elementAt(i), manager), i);
    }

}

// -----------------------------------------------------------------------
// Abstract interface from AbstractNumericValidator
// -----------------------------------------------------------------------
void DecimalDatatypeValidator::checkContent(const XMLCh*             const content
                                           ,      ValidationContext* const context
                                           ,      bool                     asBase
                                           ,      MemoryManager*     const manager)
{
    //validate against base validator if any
    DecimalDatatypeValidator *pBase = (DecimalDatatypeValidator*) this->getBaseValidator();
    if (pBase)
        pBase->checkContent(content, context, true, manager);

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

    XMLBigDecimal  compareDataValue(content, manager);
    XMLBigDecimal* compareData = &compareDataValue;

    if (getEnumeration())
    {
        XMLSize_t i=0;
        XMLSize_t enumLength = getEnumeration()->size();
        for ( ; i < enumLength; i++)
        {
            if (compareValues(compareData, (XMLBigDecimal*) getEnumeration()->elementAt(i)) ==0 )
                break;
        }

        if (i == enumLength)
            ThrowXMLwithMemMgr1(InvalidDatatypeValueException, XMLExcepts::VALUE_NotIn_Enumeration, content, manager);
    }

    boundsCheck(compareData, manager);

    if ( (thisFacetsDefined & DatatypeValidator::FACET_FRACTIONDIGITS) != 0 )
    {
        if ( compareData->getScale() > fFractionDigits )
        {
            XMLCh value1[BUF_LEN+1];
            XMLCh value2[BUF_LEN+1];
            XMLString::binToText(compareData->getScale(), value1, BUF_LEN, 10, manager);
            XMLString::binToText(fFractionDigits, value2, BUF_LEN, 10, manager);
            ThrowXMLwithMemMgr3(InvalidDatatypeFacetException
                              , XMLExcepts::VALUE_exceed_fractDigit
                              , compareData->getRawData()
                              , value1
                              , value2
                              , manager);
        }
    }

    if ( (thisFacetsDefined & DatatypeValidator::FACET_TOTALDIGITS) != 0 )
    {
        if ( compareData->getTotalDigit() > fTotalDigits )
        {
            XMLCh value1[BUF_LEN+1];
            XMLCh value2[BUF_LEN+1];
            XMLString::binToText(compareData->getTotalDigit(), value1, BUF_LEN, 10, manager);
            XMLString::binToText(fTotalDigits, value2, BUF_LEN, 10, manager);
            ThrowXMLwithMemMgr3(InvalidDatatypeFacetException
                              , XMLExcepts::VALUE_exceed_totalDigit
                              , compareData->getRawData()
                              , value1
                              , value2
                              , manager);
        }

        /***
         E2-44 totalDigits
         ... by restricting it to numbers that are expressible as i x 10^-n
         where i and n are integers such that |i| < 10^totalDigits and 0 <= n <= totalDigits.
         ***/

        if ( compareData->getScale() > fTotalDigits )
        {
            XMLCh value1[BUF_LEN+1];
            XMLCh value2[BUF_LEN+1];
            XMLString::binToText(compareData->getScale(), value1, BUF_LEN, 10, manager);
            XMLString::binToText(fTotalDigits, value2, BUF_LEN, 10, manager);
            ThrowXMLwithMemMgr3(InvalidDatatypeFacetException
                              , XMLExcepts::VALUE_exceed_totalDigit
                              , compareData->getRawData()
                              , value1
                              , value2
                              , manager);
        }
    }
}

/***
 * 3.2.3 decimal
 *
 * . the preceding optional "+" sign is prohibited.
 * . The decimal point is required.
 * . Leading and trailing zeroes are prohibited subject to the following:
 *   there must be at least one digit to the right and to the left of the decimal point which may be a zero.
 *
 *
 *  3.3.13 integer
 *  3.3.16 long
 *  3.3.17 int
 *  3.3.18 short
 *  3.3.19 byte
 *  3.3.20 nonNegativeInteger
 *  3.3.25 positiveInteger
 *
 *   . the preceding optional "+" sign is prohibited and
 *   . leading zeroes are prohibited.
 *
 *
 *  E2-27
 *  3.3.14 nonPositiveInteger
 *
 *   . In the canonical form for zero, the sign must be omitted.
 *   . leading zeroes are prohibited.
 *
 *  3.3.15 negativeInteger
 *  3.3.21 unsignedLong
 *  3.3.22 unsignedInt
 *  3.3.23 unsignedShort
 *  3.3.24 unsignedByte
 *
 *  . leading zeroes are prohibited.
 *
 *  Summary:
 *  . leading zeros are prohibited for all three groups
 *  . '-' is required for nonPositiveInteger if it is zero
 *
 ***/

const XMLCh* DecimalDatatypeValidator::getCanonicalRepresentation(const XMLCh*         const rawData
                                                                 ,      MemoryManager* const memMgr
                                                                 ,      bool                 toValidate) const
{
    MemoryManager* toUse = memMgr? memMgr : fMemoryManager;
    DecimalDatatypeValidator* temp = (DecimalDatatypeValidator*) this;

    if (toValidate)
    {
        try
        {
            temp->checkContent(rawData, 0, false, toUse);
        }
        catch (...)
        {
            return 0;
        }
    }

    // XMLBigInteger::getCanonicalRepresentation and
    // XMLBigDecimal::getCanonicalRepresentation will handle exceptional cases
    XMLCanRepGroup::CanRepGroup dvType = DatatypeValidatorFactory::getCanRepGroup(temp);

    if ((dvType == XMLCanRepGroup::Decimal_Derived_signed)   ||
        (dvType == XMLCanRepGroup::Decimal_Derived_unsigned) ||
        (dvType == XMLCanRepGroup::Decimal_Derived_npi)        )
    {
        return XMLBigInteger::getCanonicalRepresentation(rawData, toUse, dvType == XMLCanRepGroup::Decimal_Derived_npi);
    }
    else if (dvType == XMLCanRepGroup::Decimal)
    {
        return XMLBigDecimal::getCanonicalRepresentation(rawData, toUse);
    }
    else //in case?
    {
        return XMLString::replicate(rawData, toUse);
    }

}

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_TOCREATE(DecimalDatatypeValidator)

void DecimalDatatypeValidator::serialize(XSerializeEngine& serEng)
{
    /***
     * Note:
     *
     *     During storing, we need write the specific number
     *     type info before calling base::serialize().
     *
     *     While loading, we do nothing here
     ***/

    if (serEng.isStoring())
    {
        serEng<<(int) (XMLNumber::BigDecimal);
    }

    AbstractNumericValidator::serialize(serEng);

    //don't serialize XMLBigDecimal*
    if (serEng.isStoring())
    {
        serEng<<fTotalDigits;
        serEng<<fFractionDigits;
    }
    else
    {
        serEng>>fTotalDigits;
        serEng>>fFractionDigits;
    }

}

XERCES_CPP_NAMESPACE_END

/**
  * End of file DecimalDatatypeValidator::cpp
  */
