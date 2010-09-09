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
 * $Id: XMLAbstractDoubleFloat.cpp 673155 2008-07-01 17:55:39Z dbertoni $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/XMLAbstractDoubleFloat.hpp>
#include <xercesc/util/XMLBigDecimal.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/NumberFormatException.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/Janitor.hpp>

#include <locale.h>
#include <float.h>
#include <errno.h>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  local data member
// ---------------------------------------------------------------------------
static const int BUF_LEN = 64;

static XMLCh expSign[] = {chLatin_e, chLatin_E, chNull};

// ---------------------------------------------------------------------------
//  ctor/dtor
// ---------------------------------------------------------------------------
XMLAbstractDoubleFloat::XMLAbstractDoubleFloat(MemoryManager* const manager)
: fValue(0)
, fType(Normal)
, fDataConverted(false)
, fDataOverflowed(false)
, fSign(0)
, fRawData(0)
, fFormattedString(0)
, fMemoryManager(manager)
{
}

XMLAbstractDoubleFloat::~XMLAbstractDoubleFloat()
{
     fMemoryManager->deallocate(fRawData);//delete [] fRawData;
     fMemoryManager->deallocate(fFormattedString);//delete [] fFormattedString;
}

void XMLAbstractDoubleFloat::init(const XMLCh* const strValue)
{
    if ((!strValue) || (!*strValue))
        ThrowXMLwithMemMgr(NumberFormatException, XMLExcepts::XMLNUM_emptyString, fMemoryManager);

    fRawData = XMLString::replicate(strValue, fMemoryManager);   // preserve the raw data form

    XMLCh* tmpStrValue = XMLString::replicate(strValue, fMemoryManager);
    ArrayJanitor<XMLCh> janTmpName(tmpStrValue, fMemoryManager);
    XMLString::trim(tmpStrValue);

    if (!*tmpStrValue) 
        ThrowXMLwithMemMgr(NumberFormatException, XMLExcepts::XMLNUM_emptyString, fMemoryManager);

    normalizeZero(tmpStrValue);

    if (XMLString::equals(tmpStrValue, XMLUni::fgNegINFString) )
    {
        fType = NegINF;
        fSign = -1;
    }
    else if (XMLString::equals(tmpStrValue, XMLUni::fgPosINFString) )
    {
        fType = PosINF;
        fSign = 1;
    }
    else if (XMLString::equals(tmpStrValue, XMLUni::fgNaNString) )
    {
        fType = NaN;
        fSign = 1;
    }
    else
        //
        // Normal case
        //
    {
        // Use a stack-based buffer when possible.  Since all
        // valid doubles or floats will only contain ASCII
        // digits, a decimal point,  or the exponent character,
        // they will all be single byte characters, and this will
        // work.
        static const XMLSize_t maxStackSize = 100;

        XMLSize_t lenTempStrValue = 0;

        
        // Need to check that the string only contains valid schema characters
        // since the call to strtod may allow other values.  For example, AIX
        // allows "infinity" and "+INF"
        XMLCh curChar;
        while ((curChar = tmpStrValue[lenTempStrValue])!=0) {
            if (!((curChar >= chDigit_0 &&
                   curChar <= chDigit_9) ||
                  curChar == chPeriod  ||
                  curChar == chDash    ||
                  curChar == chPlus    ||
                  curChar == chLatin_E ||
                  curChar == chLatin_e)) {                
                ThrowXMLwithMemMgr(
                    NumberFormatException,
                    XMLExcepts::XMLNUM_Inv_chars,
                    getMemoryManager());
            }            
            lenTempStrValue++;
        }
       
        if (lenTempStrValue < maxStackSize)
        {
            char    buffer[maxStackSize + 1];

            XMLString::transcode(
                tmpStrValue,
                buffer,
                sizeof(buffer) - 1,
                getMemoryManager());

            // Do this for safety, because we've
            // no guarantee we didn't overrun the
            // capacity of the buffer when transcoding
            // a bogus value.
            buffer[maxStackSize] = '\0';

            // If they aren't the same length, then some
            // non-ASCII multibyte character was present.
            // This will only happen in the case where the
            // string has a bogus character, and it's long
            // enough to overrun this buffer, but we need
            // to check, even if it's unlikely to happen.
            if (XMLString::stringLen(buffer) != lenTempStrValue)
            {
                ThrowXMLwithMemMgr(
                    NumberFormatException,
                    XMLExcepts::XMLNUM_Inv_chars,
                    getMemoryManager());
            }

            checkBoundary(buffer);
        }
        else
        {
            char *nptr = XMLString::transcode(tmpStrValue, getMemoryManager());
            const ArrayJanitor<char> janStr(nptr, fMemoryManager);

            checkBoundary(nptr);
        }
    }

}

XMLCh*  XMLAbstractDoubleFloat::getRawData() const
{
    return fRawData;
}

const XMLCh*  XMLAbstractDoubleFloat::getFormattedString() const
{
    if (!fDataConverted)
    {
        return fRawData;
    }
    else 
    {
        if (!fFormattedString)    	
        {
            XMLAbstractDoubleFloat *temp = (XMLAbstractDoubleFloat *) this;
            temp->formatString();
        }

        return fFormattedString;           
    }

}

void XMLAbstractDoubleFloat::formatString()
{

    XMLSize_t rawDataLen = XMLString::stringLen(fRawData);
    fFormattedString = (XMLCh*) fMemoryManager->allocate
    (
        (rawDataLen + 8) * sizeof(XMLCh)
    );//new XMLCh [ rawDataLen + 8];
    for (XMLSize_t i = 0; i < rawDataLen + 8; i++)
        fFormattedString[i] = chNull;

    XMLString::copyString(fFormattedString, fRawData);

    fFormattedString[rawDataLen] = chSpace;
    fFormattedString[rawDataLen + 1] = chOpenParen;

    switch (fType)
    {
    case NegINF:       
        XMLString::catString(fFormattedString, XMLUni::fgNegINFString);
        break;
    case PosINF:
        XMLString::catString(fFormattedString, XMLUni::fgPosINFString);
        break;
    case NaN:
        XMLString::catString(fFormattedString, XMLUni::fgNaNString);
        break;
    default:
        // its zero
        XMLString::catString(fFormattedString, XMLUni::fgPosZeroString);
        break;
    }

    fFormattedString[XMLString::stringLen(fFormattedString)] = chCloseParen;

}

int XMLAbstractDoubleFloat::getSign() const
{
    return fSign;
}

//
//
//
int XMLAbstractDoubleFloat::compareValues(const XMLAbstractDoubleFloat* const lValue
                                        , const XMLAbstractDoubleFloat* const rValue
                                        , MemoryManager* const manager)
{
    //
    // case#1: lValue normal
    //         rValue normal
    //
    if ((!lValue->isSpecialValue()) &&
        (!rValue->isSpecialValue())  )
    {
        if (lValue->fValue == rValue->fValue)
            return EQUAL;
        else
            return (lValue->fValue > rValue->fValue) ? GREATER_THAN : LESS_THAN;

    }
    //
    // case#2: lValue special
    //         rValue special
    //
    // Schema Errata E2-40
    // 
    // Positive Infinity is greater than all other non-NAN value.
    // Nan equals itself but is not comparable with (neither greater than nor less than)
    //     any other value in the value space
    // Negative Infinity is less than all other non-NAN values.
    //
    else
    if ((lValue->isSpecialValue()) &&
        (rValue->isSpecialValue())  )
    {
        if (lValue->fType == rValue->fType)
            return EQUAL;
        else
        {
            if ((lValue->fType == NaN) ||
                (rValue->fType == NaN)  )
            {
                return INDETERMINATE;
            }
            else
            {
                return (lValue->fType > rValue->fType) ? GREATER_THAN : LESS_THAN;
            }
        }

    }
    //
    // case#3: lValue special
    //         rValue normal
    //
    else
    if ((lValue->isSpecialValue()) &&
        (!rValue->isSpecialValue())  )
    {
        return compareSpecial(lValue, manager);
    }
    //
    // case#4: lValue normal
    //         rValue special
    //
    else
    {
        return (-1) * compareSpecial(rValue, manager);
    }
}

int XMLAbstractDoubleFloat::compareSpecial(const XMLAbstractDoubleFloat* const specialValue                                         
                                         , MemoryManager* const manager)
{
    switch (specialValue->fType)
    {
    case NegINF:
        return LESS_THAN;
    case PosINF:
        return GREATER_THAN;
    case NaN:
        // NaN is not comparable to any other value
        return INDETERMINATE;

    default:
        XMLCh value1[BUF_LEN+1];
        XMLString::binToText(specialValue->fType, value1, 16, 10, manager);
        ThrowXMLwithMemMgr1(NumberFormatException
                , XMLExcepts::XMLNUM_DBL_FLT_InvalidType
                , value1, manager);
        //internal error
        return 0;
    }
}

//
//  Assumption: no leading space
//
//  1. The valid char set is "+-.0"
//  2. There shall be only one sign at the first position, if there is one.
//  3. There shall be only one dot '.', if there is one.
//
//  Return:
//
//  for input comforming to [+]? [0]* '.'? [0]*,
//            normalize the input to positive zero string
//  for input comforming to '-' [0]* '.'? [0]*,
//            normalize the input to negative zero string
//  otherwise, do nothing
//
void XMLAbstractDoubleFloat::normalizeZero(XMLCh* const inData)
{

	// do a quick check
	if (!inData  ||
		!*inData ||
        (XMLString::equals(inData, XMLUni::fgNegZeroString) ) ||
        (XMLString::equals(inData, XMLUni::fgPosZeroString) )  )
        return;

    XMLCh*   srcStr = inData;
	bool     minusSeen = false;
    bool     dotSeen = false;

	// process sign if any
	if (*srcStr == chDash)
	{
		minusSeen = true;
		srcStr++;
        if (!*srcStr)
        {
            ThrowXMLwithMemMgr(NumberFormatException, XMLExcepts::XMLNUM_Inv_chars, getMemoryManager());
        }
	}
	else if (*srcStr == chPlus)
	{
		srcStr++;
        if (!*srcStr)
        {
            ThrowXMLwithMemMgr(NumberFormatException, XMLExcepts::XMLNUM_Inv_chars, getMemoryManager());
        }
	}
    else if (*srcStr == chPeriod)
    {
        dotSeen = true;
        srcStr++;
        if (!*srcStr)
        {
            ThrowXMLwithMemMgr(NumberFormatException, XMLExcepts::XMLNUM_Inv_chars, getMemoryManager());
        }
    }

	// scan the string
	
	bool  isValidStr = true;
    XMLCh theChar;
	while ((theChar=*srcStr++)!=0 && isValidStr)
	{
		if ( theChar != chPeriod && theChar != chDigit_0 )
			isValidStr = false;           		// invalid char
        else if (theChar == chPeriod)           // process dot
			dotSeen ? isValidStr = false : dotSeen = true;
	}

	// need not to worry about the memory problem
	// since either fgNegZeroString or fgPosZeroString
	// is the canonical form (meaning the shortest in length)
	// of their category respectively.
	if (isValidStr)
	{
		if (minusSeen)
			XMLString::copyString(inData, XMLUni::fgNegZeroString);
		else
			XMLString::copyString(inData, XMLUni::fgPosZeroString);
	}
    else
    {
        // we got to set the sign first, since this string may
        // eventaully turn out to be beyond the minimum representable 
        // number and reduced to -0 or +0.
        fSign = minusSeen ? -1 : 1;
    }

    return;
} 

void XMLAbstractDoubleFloat::normalizeDecimalPoint(char* const toNormal)
{
    // find the locale-specific decimal point delimiter
    lconv* lc = localeconv();
    char delimiter = *lc->decimal_point;

    // replace '.' with the locale-specific decimal point delimiter
    if ( delimiter != '.' )
    {
        char* period = strchr( toNormal, '.' );
        if ( period )
        {
            *period = delimiter;
        }
    }
}


void
XMLAbstractDoubleFloat::convert(char* const strValue)
{
    normalizeDecimalPoint(strValue);

    char *endptr = 0;
    errno = 0;
    fValue = strtod(strValue, &endptr);

    // check if all chars are valid char.  If they are, endptr will
    // pointer to the null terminator.
    if (*endptr != '\0')
    {
        ThrowXMLwithMemMgr(NumberFormatException, XMLExcepts::XMLNUM_Inv_chars, getMemoryManager());
    }

    // check if overflow/underflow occurs
    if (errno == ERANGE)
    {
            
        fDataConverted = true;

        if ( fValue < 0 )
        {
            if (fValue > (-1)*DBL_MIN)
            {
                fValue = 0;
            }
            else
            {
                fType = NegINF;
                fDataOverflowed = true;
            }
        }
        else if ( fValue > 0)
        {
            if (fValue < DBL_MIN )
            {
                fValue = 0;
            }
            else
            {
                fType = PosINF;
                fDataOverflowed = true;
            }
        }
    }
}



/***
 * E2-40
 *
 *   3.2.4 float
 *   3.2.5 double
 *
 * . the exponent must be indicated by "E". 
 *   if the exponent is zero, it must be indicated by "E0". 
 *
 * . For the mantissa, 
 *      the preceding optional "+" sign is prohibited and 
 *      the decimal point is required. 
 *
 * . For the exponent, 
 *      the preceding optional "+" sign is prohibited. 
 *      Leading zeroes are prohibited.
 *      
 * . Leading and trailing zeroes are prohibited subject to the following: 
 *   number representations must be normalized such that 
 *     . there is a single digit, which is non-zero, to the left of the decimal point and
 *     . at least a single digit to the right of the decimal point.
 *     . unless the value being represented is zero. 
 *       The canonical representation for zero is 0.0E0
 *
 ***/     
XMLCh* XMLAbstractDoubleFloat::getCanonicalRepresentation(const XMLCh*         const rawData
                                                        ,       MemoryManager* const memMgr)
{
    // before anything, let's look for special tokens since that
    // breaks the calls to parse below.
    if(XMLString::equals(rawData, XMLUni::fgNegINFString) || 
       XMLString::equals(rawData, XMLUni::fgPosINFString) || 
       XMLString::equals(rawData, XMLUni::fgNaNString)     )
    {
        return XMLString::replicate(rawData, memMgr);
    }

    try 
    {
        XMLSize_t strLen = XMLString::stringLen(rawData);
        XMLCh* manStr = (XMLCh*) memMgr->allocate((strLen + 1) * sizeof(XMLCh));
        ArrayJanitor<XMLCh> janManStr(manStr, memMgr);
        XMLCh* manBuf = (XMLCh*) memMgr->allocate((strLen + 1) * sizeof(XMLCh));
        ArrayJanitor<XMLCh> janManBuf(manBuf, memMgr);
        XMLCh* expStr = (XMLCh*) memMgr->allocate((strLen + 1) * sizeof(XMLCh));
        ArrayJanitor<XMLCh> janExpStr(expStr, memMgr);
        XMLCh* retBuffer = (XMLCh*) memMgr->allocate((strLen + 8) * sizeof(XMLCh));
        ArrayJanitor<XMLCh> janRetBuffer(retBuffer, memMgr);
        retBuffer[0] = 0;

        int sign, totalDigits, fractDigits, expValue = 0;

        const XMLCh* ePosition = XMLString::findAny(rawData, expSign);

        /***
         *  parse mantissa and exp separately
        ***/
        if (!ePosition)
        {
            XMLBigDecimal::parseDecimal(rawData, manBuf, sign, totalDigits, fractDigits, memMgr);
            expValue = 0;
        }
        else
        {
            XMLSize_t manLen = ePosition - rawData;
            XMLString::copyNString(manStr, rawData, manLen);
            *(manStr + manLen) = chNull;
            XMLBigDecimal::parseDecimal(manStr, manBuf, sign, totalDigits, fractDigits, memMgr);

            XMLSize_t expLen = strLen - manLen - 1;
            ePosition++;
            XMLString::copyNString(expStr, ePosition, expLen);
            *(expStr + expLen) = chNull;
            expValue = XMLString::parseInt(expStr); 
        }

        if ( (sign == 0) || (totalDigits == 0) )
        {
            retBuffer[0] = chDigit_0;
            retBuffer[1] = chPeriod;
            retBuffer[2] = chDigit_0;
            retBuffer[3] = chLatin_E;
            retBuffer[4] = chDigit_0;
            retBuffer[5] = chNull;
        }
        else
        {
            XMLCh* retPtr = retBuffer;

            if (sign == -1)
            {
                *retPtr++ = chDash;
            }

            *retPtr++ = manBuf[0];
            *retPtr++ = chPeriod;

            //XMLBigDecimal::parseDecimal() will eliminate trailing zeros
            // iff there is a decimal points
            // eg. 56.7800e0  -> manBuf = 5678, totalDigits = 4, fractDigits = 2
            // we print it as 5.678e1
            //
            // but it wont remove trailing zeros if there is no decimal point.
            // eg.  567800e0 -> manBuf = 567800, totalDigits = 6, fractDigits = 0
            // we print it 5.67800e5
            //
            // for the latter, we need to print it as 5.678e5 instead
            //
            XMLCh* endPtr = manBuf + totalDigits;

            if (fractDigits == 0)
            {
                while(*(endPtr - 1) == chDigit_0)
                    endPtr--;
            }

            XMLSize_t remainLen = endPtr - &(manBuf[1]);

            if (remainLen)
            {
                XMLString::copyNString(retPtr, &(manBuf[1]), remainLen);
                retPtr += remainLen;
            }
            else
            {
                *retPtr++ = chDigit_0;
            }

            /***
             * 
             *  . adjust expValue
             *   
             *  new_fractDigits = totalDigits - 1  
             *  new_expValue = old_expValue + (new_fractDigits - fractDigits)
             *
             ***/
            expValue += (totalDigits - 1) - fractDigits ;
            XMLString::binToText(expValue, expStr, strLen, 10, memMgr);
            *retPtr++  = chLatin_E;
            *retPtr = chNull;

            XMLString::catString(&(retBuffer[0]), expStr);
        }

        janRetBuffer.release();
        return retBuffer;
    }
    catch (const NumberFormatException&)
    {
        return 0;
    }
}        

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_NOCREATE(XMLAbstractDoubleFloat)

void XMLAbstractDoubleFloat::serialize(XSerializeEngine& serEng)
{
    //REVISIT: may not need to call base since it does nothing
    XMLNumber::serialize(serEng);

    if (serEng.isStoring())
    {
        serEng << fValue;
        serEng << fType;
        serEng << fDataConverted;
        serEng << fDataOverflowed;
        serEng << fSign;

        serEng.writeString(fRawData);

        // Do not serialize fFormattedString

    }
    else
    {
        serEng >> fValue;

        int type = 0;
        serEng >> type;
        fType = (LiteralType) type;

        serEng >> fDataConverted;
        serEng >> fDataOverflowed;
        serEng >> fSign;

        serEng.readString(fRawData);

        // Set it to 0 force it to re-format if needed
        fFormattedString = 0;

    }

}

XERCES_CPP_NAMESPACE_END
