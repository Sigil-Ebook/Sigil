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
 * $Id: XMLBigInteger.cpp 555320 2007-07-11 16:05:13Z amassari $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/XMLBigInteger.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/NumberFormatException.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/Janitor.hpp>
#include <xercesc/util/XMLChar.hpp>

XERCES_CPP_NAMESPACE_BEGIN

XMLCh* XMLBigInteger::getCanonicalRepresentation(const XMLCh*         const rawData
                                               ,       MemoryManager* const memMgr
                                               ,       bool           /*      isNonPositiveInteger */)
{
    try 
    {
        XMLCh* retBuf = (XMLCh*) memMgr->allocate( (XMLString::stringLen(rawData) + 2) * sizeof(XMLCh));
        ArrayJanitor<XMLCh> jan(retBuf, memMgr);
        int    sign = 0;

        XMLBigInteger::parseBigInteger(rawData, retBuf, sign);

        if (sign == 0)
        {
           retBuf[0] = chDigit_0;
           retBuf[1] = chNull;           
        }
        else if (sign == -1)
        {
            XMLCh* retBuffer = (XMLCh*) memMgr->allocate( (XMLString::stringLen(retBuf) + 2) * sizeof(XMLCh));
            retBuffer[0] = chDash;
            XMLString::copyString(&(retBuffer[1]), retBuf);
            return retBuffer;
        }

        jan.release();
        return retBuf;

    }
    catch (const NumberFormatException&)
    {
        return 0;
    }

}

/***
   *
   *  Leading and trailing whitespaces are allowed, and trimmed
   *
   *  Only one and either of (+,-) after the leading whitespace, before
   *  any other characters are allowed, and trimmed
   *
   *  Leading zero, after leading whitespace, (+|-), before any other
   *  characters are allowed, and trimmed
   *
   *  '.' NOT allowed

   *  return status: void
   *  ret_buf: w/o leading and/or trailing whitespace
   *           w/o '+' and '-'
   *           w/o leading zero
   *
   *  see XMLString::parseInt();
   *      XMLString::textToBin();
   *
   *  "    +000203456"            "203456"
   *  "    -000203456"            "203456"
   *
***/

void XMLBigInteger::parseBigInteger(const XMLCh* const toConvert
                                  , XMLCh* const retBuffer
                                  , int&   signValue
                                  , MemoryManager* const manager)
{
    // If no string, then its a failure
    if ((!toConvert) || (!*toConvert))
        ThrowXMLwithMemMgr(NumberFormatException, XMLExcepts::XMLNUM_emptyString, manager);

    //
    // Note: in Java's BigInteger, it seems any leading and/or trailing
    // whitespaces are not allowed. If this is the case, we may
    // need to skip the trimming below.
    //

    // Scan past any whitespace. If we hit the end, then return failure
    const XMLCh* startPtr = toConvert;
    while (XMLChar1_0::isWhitespace(*startPtr))
        startPtr++;

    if (!*startPtr)
        ThrowXMLwithMemMgr(NumberFormatException, XMLExcepts::XMLNUM_WSString, manager);

    // Start at the end and work back through any whitespace
    const XMLCh* endPtr = toConvert + XMLString::stringLen(toConvert);
    while (XMLChar1_0::isWhitespace(*(endPtr - 1)))
        endPtr--;

    //
    //  Work through what remains and convert each char to a digit.
    //  anything other than '
    //
    XMLCh* retPtr = retBuffer;
    signValue = 1;

    //
    // '+' or '-' is allowed only at the first position
    //
    if (*startPtr == chDash)
    {
        signValue = -1;
        startPtr++;
        if (startPtr == endPtr)
        {
            ThrowXMLwithMemMgr(NumberFormatException, XMLExcepts::XMLNUM_Inv_chars, manager);
        }
    }
    else if (*startPtr == chPlus)
    {
        // skip the '+'
        startPtr++;
        if (startPtr == endPtr)
        {
            ThrowXMLwithMemMgr(NumberFormatException, XMLExcepts::XMLNUM_Inv_chars, manager);
        }
    }


    // Scan past any leading zero.
    while (*startPtr == chDigit_0)
        startPtr++;

    if (startPtr >= endPtr)
    {
        signValue = 0;
        // containning zero, only zero, nothing but zero
        // it is a zero, indeed
        return;
    }

    while (startPtr < endPtr)
    {
        // If not valid decimal digit, then an error
        if ((*startPtr < chDigit_0) || (*startPtr > chDigit_9))
            ThrowXMLwithMemMgr(NumberFormatException, XMLExcepts::XMLNUM_Inv_chars, manager);

        // copy over
        *retPtr = *startPtr;
        retPtr++;
        startPtr++;
    }

    *retPtr = 0;   //terminated
    return;
}


/**
	 * Translates a string containing an optional minus sign followed by a
	 * sequence of one or more digits into a BigInteger.
	 * Any extraneous characters (including whitespace),
	 * inclusive, will result in a NumberFormatException.
 */

XMLBigInteger::XMLBigInteger(const XMLCh* const strValue,
                             MemoryManager* const manager)
: fSign(0)
, fMagnitude(0)
, fRawData(0)
, fMemoryManager(manager)
{
    if (!strValue)
        ThrowXMLwithMemMgr(NumberFormatException, XMLExcepts::XMLNUM_emptyString, fMemoryManager);

    XMLCh* ret_value = (XMLCh*) fMemoryManager->allocate
    (
       (XMLString::stringLen(strValue) + 1) * sizeof(XMLCh)
    );//new XMLCh[XMLString::stringLen(strValue)+1];
    ArrayJanitor<XMLCh> janName(ret_value, fMemoryManager);

    parseBigInteger(strValue, ret_value, fSign, fMemoryManager);

    if (fSign == 0)
        fMagnitude = XMLString::replicate(XMLUni::fgZeroLenString, fMemoryManager);
    else
        fMagnitude = XMLString::replicate(ret_value, fMemoryManager);

	fRawData = XMLString::replicate(strValue, fMemoryManager);
}

XMLBigInteger::~XMLBigInteger()
{
    fMemoryManager->deallocate(fMagnitude);//delete[] fMagnitude;
	if (fRawData)
		fMemoryManager->deallocate(fRawData);//delete[] fRawData;
}

XMLBigInteger::XMLBigInteger(const XMLBigInteger& toCopy)
: XMemory(toCopy)
, fSign(toCopy.fSign)
, fMagnitude(0)
, fRawData(0)
, fMemoryManager(toCopy.fMemoryManager)
{
    fMagnitude = XMLString::replicate(toCopy.fMagnitude, fMemoryManager);
	fRawData = XMLString::replicate(toCopy.fRawData, fMemoryManager);
}

/**
 * Returns -1, 0 or 1 as lValue is less than, equal to, or greater
 * than rValue.
*/
int  XMLBigInteger::compareValues(const XMLBigInteger* const lValue
                                , const XMLBigInteger* const rValue
                                , MemoryManager* const manager)
{
    if ((!lValue) || (!rValue) )
        ThrowXMLwithMemMgr(NumberFormatException, XMLExcepts::XMLNUM_null_ptr, manager);

    int lSign = lValue->getSign();
    int rSign = rValue->getSign();

    //
    // different sign
    //
    if (lSign != rSign)
        return(lSign > rSign ? 1 : -1);

    //
    // same sign
    //
    if (lSign == 0)    // optimization
        return 0;

    XMLSize_t lStrLen = XMLString::stringLen(lValue->fMagnitude);
    XMLSize_t rStrLen = XMLString::stringLen(rValue->fMagnitude);

    //
    // different length
    //
    if (lStrLen > rStrLen)
        return ( lSign > 0 ? 1 : -1 );
    else if (lStrLen < rStrLen)
        return ( lSign > 0 ? -1 : 1 );

    //
    // same length
    // XMLString::compareString() return > 0, 0 and <0
    // we need to convert it to 1, 0, and -1
    //
    int retVal = XMLString::compareString(lValue->fMagnitude, rValue->fMagnitude);

    if ( retVal > 0 )
    {
        return ( lSign > 0 ? 1 : -1 );
    }
    else if ( retVal < 0 )
    {
        return ( lSign > 0 ? -1 : 1 );
    }
    else
        return 0;

}

int XMLBigInteger::compareValues(const XMLCh*         const lString
                               , const int&                 lSign
                               , const XMLCh*         const rString
                               , const int&                 rSign
                               ,       MemoryManager* const manager)
{
    if ((!lString) || (!rString) )
        ThrowXMLwithMemMgr(NumberFormatException, XMLExcepts::XMLNUM_null_ptr, manager);

    //
    // different sign
    //
    if (lSign != rSign)
        return(lSign > rSign ? 1 : -1);

    //
    // same sign
    //
    if (lSign == 0)    // optimization
        return 0;

    XMLSize_t lStrLen = XMLString::stringLen(lString);
    XMLSize_t rStrLen = XMLString::stringLen(rString);

    //
    // different length
    //
    if (lStrLen > rStrLen)
        return ( lSign > 0 ? 1 : -1 );
    else if (lStrLen < rStrLen)
        return ( lSign > 0 ? -1 : 1 );

    //
    // same length
    // XMLString::compareString() return > 0, 0 and <0
    // we need to convert it to 1, 0, and -1
    //
    int retVal = XMLString::compareString(lString, rString);

    if ( retVal > 0 )
    {
        return ( lSign > 0 ? 1 : -1 );
    }
    else if ( retVal < 0 )
    {
        return ( lSign > 0 ? -1 : 1 );
    }
    else
        return 0;

}

/**
 * Shift the fMagnitude to the left
 */

void XMLBigInteger::multiply(const unsigned int byteToShift)
{
    if (byteToShift <= 0)
        return;

    XMLSize_t strLen = XMLString::stringLen(fMagnitude);
    XMLCh* tmp = (XMLCh*) fMemoryManager->allocate
    (
        (strLen + byteToShift + 1) * sizeof(XMLCh)
    );//new XMLCh[strLen+byteToShift+1];
    XMLString::moveChars(tmp, fMagnitude, strLen);

    unsigned int i = 0;
    for ( ; i < byteToShift; i++)
        tmp[strLen+i] = chDigit_0;

    tmp[strLen+i] = chNull;

    fMemoryManager->deallocate(fMagnitude);//delete[] fMagnitude;
    fMagnitude = tmp;
}

/**
 * Shift the fMagnitude to the right
 * by doing this, we lose precision.
 */
void XMLBigInteger::divide(const unsigned int byteToShift)
{
    if (byteToShift <= 0)
        return;

    XMLSize_t strLen = XMLString::stringLen(fMagnitude);
    XMLCh* tmp = (XMLCh*) fMemoryManager->allocate
    (
        (strLen - byteToShift + 1) * sizeof(XMLCh)
    );//new XMLCh[strLen-byteToShift+1];
    XMLString::moveChars(tmp, fMagnitude, strLen-byteToShift);

    tmp[strLen-byteToShift] = chNull;

    fMemoryManager->deallocate(fMagnitude);//delete[] fMagnitude;
    fMagnitude = tmp;
}

//
//
//
int XMLBigInteger::intValue() const
{
    unsigned int retVal;
    XMLString::textToBin(fMagnitude, retVal, fMemoryManager);
    return retVal * getSign();
}

XERCES_CPP_NAMESPACE_END
