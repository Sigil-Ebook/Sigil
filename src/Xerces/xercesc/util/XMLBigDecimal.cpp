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
 * $Id: XMLBigDecimal.cpp 557254 2007-07-18 13:28:54Z amassari $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/XMLBigDecimal.hpp>
#include <xercesc/util/XMLBigInteger.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/util/NumberFormatException.hpp>
#include <xercesc/util/XMLChar.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/util/Janitor.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/**
 * Constructs a BigDecimal from a string containing an optional (plus | minus)
 * sign followed by a sequence of zero or more decimal digits, optionally
 * followed by a fraction, which consists of a decimal point followed by
 * zero or more decimal digits.  The string must contain at least one
 * digit in the integer or fractional part.  The scale of the resulting
 * BigDecimal will be the number of digits to the right of the decimal
 * point in the string, or 0 if the string contains no decimal point.
 * Any extraneous characters (including whitespace) will result in
 * a NumberFormatException.

 * since parseBigDecimal()  may throw exception,
 * caller of XMLBigDecimal need to catch it.
//
**/

typedef JanitorMemFunCall<XMLBigDecimal>    CleanupType;

XMLBigDecimal::XMLBigDecimal(const XMLCh* const strValue,
                             MemoryManager* const manager)
: fSign(0)
, fTotalDigits(0)
, fScale(0)
, fRawDataLen(0)
, fRawData(0)
, fIntVal(0)
, fMemoryManager(manager)
{
    if ((!strValue) || (!*strValue))
        ThrowXMLwithMemMgr(NumberFormatException, XMLExcepts::XMLNUM_emptyString, fMemoryManager);

    CleanupType cleanup(this, &XMLBigDecimal::cleanUp);

    try
    {
        fRawDataLen = XMLString::stringLen(strValue);
        fRawData = (XMLCh*) fMemoryManager->allocate
        (
            ((fRawDataLen*2) + 2) * sizeof(XMLCh) //fRawData and fIntVal
        );
        memcpy(fRawData, strValue, fRawDataLen * sizeof(XMLCh));
        fRawData[fRawDataLen] = chNull;
        fIntVal = fRawData + fRawDataLen + 1;
        parseDecimal(strValue, fIntVal, fSign, (int&) fTotalDigits, (int&) fScale, fMemoryManager);
    }
    catch(const OutOfMemoryException&)
    {
        cleanup.release();

        throw;
    }

    cleanup.release();
}

XMLBigDecimal::~XMLBigDecimal()
{
    cleanUp();
}

void XMLBigDecimal::cleanUp()
{
    if (fRawData)
        fMemoryManager->deallocate(fRawData); //XMLString::release(&fRawData);
}

void XMLBigDecimal::setDecimalValue(const XMLCh* const strValue)
{
    fScale = fTotalDigits = 0;
    XMLSize_t valueLen = XMLString::stringLen(strValue);

    if (valueLen > fRawDataLen)
    {
        fMemoryManager->deallocate(fRawData);
        fRawData = (XMLCh*) fMemoryManager->allocate
        (
            ((valueLen * 2) + 4) * sizeof(XMLCh)
        );//XMLString::replicate(strValue, fMemoryManager);
    }

    memcpy(fRawData, strValue, valueLen * sizeof(XMLCh));
    fRawData[valueLen] = chNull;
    fRawDataLen = valueLen;
    fIntVal = fRawData + fRawDataLen + 1;
    parseDecimal(strValue, fIntVal, fSign, (int&) fTotalDigits, (int&) fScale, fMemoryManager);

}

/***
 * 3.2.3 decimal  
 *
 * . the preceding optional "+" sign is prohibited. 
 * . The decimal point is required. 
 * . Leading and trailing zeroes are prohibited subject to the following: 
 *   there must be at least one digit to the right and to the left of the decimal point which may be a zero.
 *
 ***/
XMLCh* XMLBigDecimal::getCanonicalRepresentation(const XMLCh*         const rawData
                                               ,       MemoryManager* const memMgr)
{

    XMLCh* retBuf = (XMLCh*) memMgr->allocate( (XMLString::stringLen(rawData)+1) * sizeof(XMLCh));
    ArrayJanitor<XMLCh> janName(retBuf, memMgr);
    int   sign, totalDigits, fractDigits;

    try
    {
        parseDecimal(rawData, retBuf, sign, totalDigits, fractDigits, memMgr);
    }
    catch (const NumberFormatException&)
    {
        return 0;
    }


    //Extra space reserved in case strLen is zero
    XMLSize_t strLen = XMLString::stringLen(retBuf);
    XMLCh* retBuffer = (XMLCh*) memMgr->allocate( (strLen + 4) * sizeof(XMLCh));

    if ( (sign == 0) || (totalDigits == 0))
    {
        retBuffer[0] = chDigit_0;
        retBuffer[1] = chPeriod;
        retBuffer[2] = chDigit_0;
        retBuffer[3] = chNull;
    }
    else
    {
        XMLCh* retPtr = retBuffer;

        if (sign == -1)
        {
            *retPtr++ = chDash;
        }

        if (fractDigits == totalDigits)   // no integer
        {           
            *retPtr++ = chDigit_0;
            *retPtr++ = chPeriod;
            XMLString::copyNString(retPtr, retBuf, strLen);
            retPtr += strLen;
            *retPtr = chNull;
        }
        else if (fractDigits == 0)        // no fraction
        {
            XMLString::copyNString(retPtr, retBuf, strLen);
            retPtr += strLen;
            *retPtr++ = chPeriod;
            *retPtr++ = chDigit_0;
            *retPtr   = chNull;
        }
        else  // normal
        {
            int intLen = totalDigits - fractDigits;
            XMLString::copyNString(retPtr, retBuf, intLen);
            retPtr += intLen;
            *retPtr++ = chPeriod;
            XMLString::copyNString(retPtr, &(retBuf[intLen]), fractDigits);
            retPtr += fractDigits;
            *retPtr = chNull;
        }

    }
            
    return retBuffer;
}

void  XMLBigDecimal::parseDecimal(const XMLCh* const toParse
                               ,        XMLCh* const retBuffer
                               ,        int&         sign
                               ,        int&         totalDigits
                               ,        int&         fractDigits
                               ,        MemoryManager* const manager)
{
    //init
    retBuffer[0] = chNull;
    totalDigits = 0;
    fractDigits = 0;

    // Strip leading white space, if any. 
    const XMLCh* startPtr = toParse;
    while (XMLChar1_0::isWhitespace(*startPtr))
        startPtr++;

    // If we hit the end, then return failure
    if (!*startPtr)
        ThrowXMLwithMemMgr(NumberFormatException, XMLExcepts::XMLNUM_WSString, manager);

    // Strip tailing white space, if any.
    const XMLCh* endPtr = toParse + XMLString::stringLen(toParse);
    while (XMLChar1_0::isWhitespace(*(endPtr - 1)))
        endPtr--;

    // '+' or '-' is allowed only at the first position
    // and is NOT included in the return parsed string
    sign = 1;
    if (*startPtr == chDash)
    {
        sign = -1;
        startPtr++;
        if (startPtr == endPtr)
        {
            ThrowXMLwithMemMgr(NumberFormatException, XMLExcepts::XMLNUM_Inv_chars, manager);
        }
    }
    else if (*startPtr == chPlus)
    {
        startPtr++;         
        if (startPtr == endPtr)
        {
            ThrowXMLwithMemMgr(NumberFormatException, XMLExcepts::XMLNUM_Inv_chars, manager);
        }
    }

    // Strip leading zeros
    while (*startPtr == chDigit_0)
        startPtr++;

    // containning zero, only zero, nothing but zero
    // it is a zero, indeed
    if (startPtr >= endPtr)
    {
        sign = 0;
        return;
    }

    XMLCh* retPtr = (XMLCh*) retBuffer;

    // Scan data
    bool   dotSignFound = false;
    while (startPtr < endPtr)
    {
        if (*startPtr == chPeriod)
        {
            if (!dotSignFound)
            {
                dotSignFound = true;
                fractDigits = (int)(endPtr - startPtr - 1);
                startPtr++;
                continue;
            }
            else  // '.' is allowed only once
                ThrowXMLwithMemMgr(NumberFormatException, XMLExcepts::XMLNUM_2ManyDecPoint, manager);
        }

        // If not valid decimal digit, then an error
        if ((*startPtr < chDigit_0) || (*startPtr > chDigit_9))
            ThrowXMLwithMemMgr(NumberFormatException, XMLExcepts::XMLNUM_Inv_chars, manager);

        // copy over
        *retPtr++ = *startPtr++;
        totalDigits++;
    }

    /***
    E2-44 totalDigits

     ... by restricting it to numbers that are expressible as i x 10^-n
     where i and n are integers such that |i| < 10^totalDigits and 0 <= n <= totalDigits. 

        normalization: remove all trailing zero after the '.'
                       and adjust the scaleValue as well.
    ***/
    while ((fractDigits > 0) && (*(retPtr-1) == chDigit_0))          
    {
        retPtr--;
        fractDigits--;
        totalDigits--;
    }
    // 0.0 got past the check for zero because of the decimal point, so we need to double check it here
    if(totalDigits==0)
        sign = 0;

    *retPtr = chNull;   //terminated
    return;
}

void  XMLBigDecimal::parseDecimal(const XMLCh*         const toParse
                               ,        MemoryManager* const manager)
{

    // Strip leading white space, if any. 
    const XMLCh* startPtr = toParse;
    while (XMLChar1_0::isWhitespace(*startPtr))
        startPtr++;

    // If we hit the end, then return failure
    if (!*startPtr)
        ThrowXMLwithMemMgr(NumberFormatException, XMLExcepts::XMLNUM_WSString, manager);

    // Strip tailing white space, if any.
    const XMLCh* endPtr = toParse + XMLString::stringLen(toParse);
    while (XMLChar1_0::isWhitespace(*(endPtr - 1)))
        endPtr--;

    // '+' or '-' is allowed only at the first position
    // and is NOT included in the return parsed string

    if (*startPtr == chDash)
    {
        startPtr++;
        if (startPtr == endPtr)
        {
            ThrowXMLwithMemMgr(NumberFormatException, XMLExcepts::XMLNUM_Inv_chars, manager);
        }
    }
    else if (*startPtr == chPlus)
    {
        startPtr++;
        if (startPtr == endPtr)
        {
            ThrowXMLwithMemMgr(NumberFormatException, XMLExcepts::XMLNUM_Inv_chars, manager);
        }
    }

    // Strip leading zeros
    while (*startPtr == chDigit_0)
        startPtr++;

    // containning zero, only zero, nothing but zero
    // it is a zero, indeed
    if (startPtr >= endPtr)
    {
        return;
    }

    // Scan data
    bool   dotSignFound = false;
    while (startPtr < endPtr)
    {
        if (*startPtr == chPeriod)
        {
            if (!dotSignFound)
            {
                dotSignFound = true;
                startPtr++;
                continue;
            }
            else  // '.' is allowed only once
                ThrowXMLwithMemMgr(NumberFormatException, XMLExcepts::XMLNUM_2ManyDecPoint, manager);
        }

        // If not valid decimal digit, then an error
        if ((*startPtr < chDigit_0) || (*startPtr > chDigit_9))
            ThrowXMLwithMemMgr(NumberFormatException, XMLExcepts::XMLNUM_Inv_chars, manager);

        startPtr++;

    }

    return;
}

int XMLBigDecimal::compareValues( const XMLBigDecimal* const lValue
                                , const XMLBigDecimal* const rValue
                                , MemoryManager* const manager)
{
    if ((!lValue) || (!rValue) )
        ThrowXMLwithMemMgr(NumberFormatException, XMLExcepts::XMLNUM_null_ptr, manager);
        	
    return lValue->toCompare(*rValue);
}                                

/**
 * Returns -1, 0 or 1 as this is less than, equal to, or greater
 * than rValue.  
 * 
 * This method is based on the fact, that parsebigDecimal() would eliminate
 * unnecessary leading/trailing zeros. 
**/
int XMLBigDecimal::toCompare(const XMLBigDecimal& other) const
{
    /***
     * different sign
     */
    int lSign = this->getSign();
    if (lSign != other.getSign())
        return (lSign > other.getSign() ? 1 : -1);

    /***
     * same sign, zero
     */
    if (lSign == 0)    // optimization
        return 0;

    /***
     * same sign, non-zero
     */
    unsigned int lIntDigit = this->getTotalDigit() - this->getScale();
    unsigned int rIntDigit = other.getTotalDigit() - other.getScale();

    if (lIntDigit > rIntDigit)
    {
        return 1 * lSign;
    }
    else if (lIntDigit < rIntDigit)
    {
        return -1 * lSign;
    }
    else  // compare fraction
    {
        int res = XMLString::compareString
        ( this->getValue()
        , other.getValue()
        );

        if (res > 0)
            return 1 * lSign;
        else if (res < 0)
            return -1 * lSign;
        else
            return 0;
    }

}


/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_TOCREATE(XMLBigDecimal)

XMLBigDecimal::XMLBigDecimal(MemoryManager* const manager)
: fSign(0)
, fTotalDigits(0)
, fScale(0)
, fRawDataLen(0)
, fRawData(0)
, fIntVal(0)
, fMemoryManager(manager)
{
}

void XMLBigDecimal::serialize(XSerializeEngine& serEng)
{
    //REVISIT: may not need to call base since it does nothing
    XMLNumber::serialize(serEng);

    if (serEng.isStoring())
    {
        serEng<<fSign;
        serEng<<fTotalDigits;
        serEng<<fScale;

        serEng.writeString(fRawData);
        serEng.writeString(fIntVal);

    }
    else
    {
        serEng>>fSign;
        serEng>>fTotalDigits;
        serEng>>fScale;

        XMLCh* rawdataStr;
        serEng.readString(rawdataStr);
        ArrayJanitor<XMLCh> rawdataName(rawdataStr, serEng.getMemoryManager());
        fRawDataLen = XMLString::stringLen(rawdataStr);

        XMLCh* intvalStr;
        serEng.readString(intvalStr);
        ArrayJanitor<XMLCh> intvalName(intvalStr, serEng.getMemoryManager());
        XMLSize_t intvalStrLen = XMLString::stringLen(intvalStr);

        if (fRawData)
            fMemoryManager->deallocate(fRawData);

        fRawData = (XMLCh*) fMemoryManager->allocate
        (
            ((fRawDataLen + intvalStrLen) + 4) * sizeof(XMLCh)
        );

        memcpy(fRawData, rawdataStr, fRawDataLen * sizeof(XMLCh));
        fRawData[fRawDataLen] = chNull;
        fIntVal = fRawData + fRawDataLen + 1;
        memcpy(fIntVal, intvalStr,  intvalStrLen * sizeof(XMLCh));
        fIntVal[intvalStrLen] = chNull;

    }

}

XERCES_CPP_NAMESPACE_END

