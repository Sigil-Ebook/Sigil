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
 * $Id: XMLDateTime.cpp 932887 2010-04-11 13:04:59Z borisk $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <stdlib.h>
#include <assert.h>
#include <errno.h>

#include <xercesc/util/XMLDateTime.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/util/Janitor.hpp>
#include <xercesc/util/NumberFormatException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

//
// constants used to process raw data (fBuffer)
//
// [-]{CCYY-MM-DD}'T'{HH:MM:SS.MS}['Z']
//                                [{+|-}hh:mm']
//

static const XMLCh DURATION_STARTER     = chLatin_P;              // 'P'
static const XMLCh DURATION_Y           = chLatin_Y;              // 'Y'
static const XMLCh DURATION_M           = chLatin_M;              // 'M'
static const XMLCh DURATION_D           = chLatin_D;              // 'D'
static const XMLCh DURATION_H           = chLatin_H;              // 'H'
static const XMLCh DURATION_S           = chLatin_S;              // 'S'

static const XMLCh DATE_SEPARATOR       = chDash;                 // '-'
static const XMLCh TIME_SEPARATOR       = chColon;                // ':'
static const XMLCh TIMEZONE_SEPARATOR   = chColon;                // ':'
static const XMLCh DATETIME_SEPARATOR   = chLatin_T;              // 'T'
static const XMLCh MILISECOND_SEPARATOR = chPeriod;               // '.'

static const XMLCh UTC_STD_CHAR         = chLatin_Z;              // 'Z'
static const XMLCh UTC_POS_CHAR         = chPlus;                 // '+'
static const XMLCh UTC_NEG_CHAR         = chDash;                 // '-'

static const XMLCh UTC_SET[]            = {UTC_STD_CHAR           //"Z+-"
                                         , UTC_POS_CHAR
                                         , UTC_NEG_CHAR
                                         , chNull};

static const XMLSize_t YMD_MIN_SIZE    = 10;   // CCYY-MM-DD
static const XMLSize_t YMONTH_MIN_SIZE = 7;    // CCYY_MM
static const XMLSize_t TIME_MIN_SIZE   = 8;    // hh:mm:ss
static const XMLSize_t TIMEZONE_SIZE   = 5;    // hh:mm
static const XMLSize_t DAY_SIZE        = 5;    // ---DD
//static const XMLSize_t MONTH_SIZE      = 6;    // --MM--
static const XMLSize_t MONTHDAY_SIZE   = 7;    // --MM-DD
static const int NOT_FOUND       = -1;

//define constants to be used in assigning default values for
//all date/time excluding duration
static const int YEAR_DEFAULT  = 2000;
static const int MONTH_DEFAULT = 01;
static const int DAY_DEFAULT   = 15;

// order-relation on duration is a partial order. The dates below are used to
// for comparison of 2 durations, based on the fact that
// duration x and y is x<=y iff s+x<=s+y
// see 3.2.6 duration W3C schema datatype specs
//
// the dates are in format: {CCYY,MM,DD, H, S, M, MS, timezone}
static const int DATETIMES[][XMLDateTime::TOTAL_SIZE] =
{
    {1696, 9, 1, 0, 0, 0, 0, XMLDateTime::UTC_STD},
	{1697, 2, 1, 0, 0, 0, 0, XMLDateTime::UTC_STD},
	{1903, 3, 1, 0, 0, 0, 0, XMLDateTime::UTC_STD},
	{1903, 7, 1, 0, 0, 0, 0, XMLDateTime::UTC_STD}
};

// ---------------------------------------------------------------------------
//  local methods
// ---------------------------------------------------------------------------
static inline int fQuotient(int a, int b)
{
    div_t div_result = div(a, b);
    return div_result.quot;
}

static inline int fQuotient(int temp, int low, int high)
{
    return fQuotient(temp - low, high - low);
}

static inline int mod(int a, int b, int quotient)
{
	return (a - quotient*b) ;
}

static inline int modulo (int temp, int low, int high)
{
    //modulo(a - low, high - low) + low
    int a = temp - low;
    int b = high - low;
    return (mod (a, b, fQuotient(a, b)) + low) ;
}

static inline bool isLeapYear(int year)
{
    return((year%4 == 0) && ((year%100 != 0) || (year%400 == 0)));
}

static int maxDayInMonthFor(int year, int month)
{

    if ( month == 4 || month == 6 || month == 9 || month == 11 )
    {
        return 30;
    }
    else if ( month==2 )
    {
        if ( isLeapYear(year) )
            return 29;
        else
            return 28;
    }
    else
    {
        return 31;
    }

}

// ---------------------------------------------------------------------------
//  static methods : for duration
// ---------------------------------------------------------------------------
/**
 * Compares 2 given durations. (refer to W3C Schema Datatypes "3.2.6 duration")
 *
 * 3.2.6.2 Order relation on duration
 *
 *     In general, the order-relation on duration is a partial order since there is no
 *  determinate relationship between certain durations such as one month (P1M) and 30 days (P30D).
 *  The order-relation of two duration values x and y is x < y iff s+x < s+y for each qualified
 *  dateTime s in the list below.
 *
 *     These values for s cause the greatest deviations in the addition of dateTimes and durations
 *
 **/
int XMLDateTime::compare(const XMLDateTime* const pDate1
                       , const XMLDateTime* const pDate2
                       , bool  strict)
{
    //REVISIT: this is unoptimazed vs of comparing 2 durations
    //         Algorithm is described in 3.2.6.2 W3C Schema Datatype specs
    //

    int resultA, resultB = INDETERMINATE;

    //try and see if the objects are equal
    if ( (resultA = compareOrder(pDate1, pDate2)) == EQUAL)
        return EQUAL;

    //long comparison algorithm is required
    XMLDateTime tempA(XMLPlatformUtils::fgMemoryManager), *pTempA = &tempA;
    XMLDateTime tempB(XMLPlatformUtils::fgMemoryManager), *pTempB = &tempB;

    addDuration(pTempA, pDate1, 0);
    addDuration(pTempB, pDate2, 0);
    resultA = compareOrder(pTempA, pTempB);
    if ( resultA == INDETERMINATE )
        return INDETERMINATE;

    addDuration(pTempA, pDate1, 1);
    addDuration(pTempB, pDate2, 1);
    resultB = compareOrder(pTempA, pTempB);
    resultA = compareResult(resultA, resultB, strict);
    if ( resultA == INDETERMINATE )
        return INDETERMINATE;

    addDuration(pTempA, pDate1, 2);
    addDuration(pTempB, pDate2, 2);
    resultB = compareOrder(pTempA, pTempB);
    resultA = compareResult(resultA, resultB, strict);
    if ( resultA == INDETERMINATE )
        return INDETERMINATE;

    addDuration(pTempA, pDate1, 3);
    addDuration(pTempB, pDate2, 3);
    resultB = compareOrder(pTempA, pTempB);
    resultA = compareResult(resultA, resultB, strict);

    return resultA;

}

//
// Form a new XMLDateTime with duration and baseDate array
// Note: C++        Java
//       fNewDate   duration
//       fDuration  date
//

void XMLDateTime::addDuration(XMLDateTime*             fNewDate
                            , const XMLDateTime* const fDuration
                            , int index)

{

    //REVISIT: some code could be shared between normalize() and this method,
    //         however is it worth moving it? The structures are different...
    //

    fNewDate->reset();
    //add months (may be modified additionaly below)
    int temp = DATETIMES[index][Month] + fDuration->fValue[Month];
    fNewDate->fValue[Month] = modulo(temp, 1, 13);
    int carry = fQuotient(temp, 1, 13);
    if (fNewDate->fValue[Month] <= 0) {
        fNewDate->fValue[Month]+= 12;
        carry--;
    }

    //add years (may be modified additionaly below)
    fNewDate->fValue[CentYear] = DATETIMES[index][CentYear] + fDuration->fValue[CentYear] + carry;

    //add seconds
    temp = DATETIMES[index][Second] + fDuration->fValue[Second];
    carry = fQuotient (temp, 60);
    fNewDate->fValue[Second] =  mod(temp, 60, carry);
    if (fNewDate->fValue[Second] < 0) {
        fNewDate->fValue[Second]+= 60;
        carry--;
    }

    //add minutes
    temp = DATETIMES[index][Minute] + fDuration->fValue[Minute] + carry;
    carry = fQuotient(temp, 60);
    fNewDate->fValue[Minute] = mod(temp, 60, carry);
    if (fNewDate->fValue[Minute] < 0) {
        fNewDate->fValue[Minute]+= 60;
        carry--;
    }

    //add hours
    temp = DATETIMES[index][Hour] + fDuration->fValue[Hour] + carry;
    carry = fQuotient(temp, 24);
    fNewDate->fValue[Hour] = mod(temp, 24, carry);
    if (fNewDate->fValue[Hour] < 0) {
        fNewDate->fValue[Hour]+= 24;
        carry--;
    }

    fNewDate->fValue[Day] = DATETIMES[index][Day] + fDuration->fValue[Day] + carry;

    while ( true )
    {
        temp = maxDayInMonthFor(fNewDate->fValue[CentYear], fNewDate->fValue[Month]);
        if ( fNewDate->fValue[Day] < 1 )
        { //original fNewDate was negative
            fNewDate->fValue[Day] += maxDayInMonthFor(fNewDate->fValue[CentYear], fNewDate->fValue[Month]-1);
            carry = -1;
        }
        else if ( fNewDate->fValue[Day] > temp )
        {
            fNewDate->fValue[Day] -= temp;
            carry = 1;
        }
        else
        {
            break;
        }

        temp = fNewDate->fValue[Month] + carry;
        fNewDate->fValue[Month] = modulo(temp, 1, 13);
        if (fNewDate->fValue[Month] <= 0) {
            fNewDate->fValue[Month]+= 12;
            fNewDate->fValue[CentYear]--;
        }
        fNewDate->fValue[CentYear] += fQuotient(temp, 1, 13);
    }

    //fNewDate->fValue[utc] = UTC_STD_CHAR;
    fNewDate->fValue[utc] = UTC_STD;
}

int XMLDateTime::compareResult(int resultA
                             , int resultB
                             , bool strict)
{

    if ( resultB == INDETERMINATE )
    {
        return INDETERMINATE;
    }
    else if ( (resultA != resultB) &&
              strict                )
    {
        return INDETERMINATE;
    }
    else if ( (resultA != resultB) &&
              !strict               )
    {
        if ( (resultA != EQUAL) &&
             (resultB != EQUAL)  )
        {
            return INDETERMINATE;
        }
        else
        {
            return (resultA != EQUAL)? resultA : resultB;
        }
    }

    return resultA;

}

// ---------------------------------------------------------------------------
//  static methods : for others
// ---------------------------------------------------------------------------
int XMLDateTime::compare(const XMLDateTime* const pDate1
                       , const XMLDateTime* const pDate2)
{

    if (pDate1->fValue[utc] == pDate2->fValue[utc])
    {
        return XMLDateTime::compareOrder(pDate1, pDate2);
    }

    int c1, c2;

    if ( pDate1->isNormalized())
    {
        c1 = compareResult(pDate1, pDate2, false, UTC_POS);
        c2 = compareResult(pDate1, pDate2, false, UTC_NEG);
        return getRetVal(c1, c2);
    }
    else if ( pDate2->isNormalized())
    {
        c1 = compareResult(pDate1, pDate2, true, UTC_POS);
        c2 = compareResult(pDate1, pDate2, true, UTC_NEG);
        return getRetVal(c1, c2);
    }

    return INDETERMINATE;
}

int XMLDateTime::compareResult(const XMLDateTime* const pDate1
                             , const XMLDateTime* const pDate2
                             , bool  set2Left
                             , int   utc_type)
{
    XMLDateTime tmpDate = (set2Left ? *pDate1 : *pDate2);

    tmpDate.fTimeZone[hh] = 14;
    tmpDate.fTimeZone[mm] = 0;
    tmpDate.fValue[utc] = utc_type;
    tmpDate.normalize();

    return (set2Left? XMLDateTime::compareOrder(&tmpDate, pDate2) :
                      XMLDateTime::compareOrder(pDate1, &tmpDate));
}

int XMLDateTime::compareOrder(const XMLDateTime* const lValue
                            , const XMLDateTime* const rValue)
                            //, MemoryManager* const memMgr)
{
    //
    // If any of the them is not normalized() yet,
    // we need to do something here.
    //
    XMLDateTime lTemp = *lValue;
    XMLDateTime rTemp = *rValue;

    lTemp.normalize();
    rTemp.normalize();

    for ( int i = 0 ; i < TOTAL_SIZE; i++ )
    {
        if ( lTemp.fValue[i] < rTemp.fValue[i] )
        {
            return LESS_THAN;
        }
        else if ( lTemp.fValue[i] > rTemp.fValue[i] )
        {
            return GREATER_THAN;
        }
    }

    if ( lTemp.fHasTime)
    {
        if ( lTemp.fMilliSecond < rTemp.fMilliSecond )
        {
            return LESS_THAN;
        }
        else if ( lTemp.fMilliSecond > rTemp.fMilliSecond )
        {
            return GREATER_THAN;
        }
    }

    return EQUAL;
}

// ---------------------------------------------------------------------------
//  ctor and dtor
// ---------------------------------------------------------------------------
XMLDateTime::~XMLDateTime()
{
    if (fBuffer)
        fMemoryManager->deallocate(fBuffer);//delete[] fBuffer;
}

XMLDateTime::XMLDateTime(MemoryManager* const manager)
: fStart(0)
, fEnd(0)
, fBufferMaxLen(0)
, fMilliSecond(0)
, fHasTime(false)
, fBuffer(0)
, fMemoryManager(manager)
{
    reset();
}

XMLDateTime::XMLDateTime(const XMLCh* const aString,
                         MemoryManager* const manager)
: fStart(0)
, fEnd(0)
, fBufferMaxLen(0)
, fMilliSecond(0)
, fHasTime(false)
, fBuffer(0)
, fMemoryManager(manager)
{
    setBuffer(aString);
}

// -----------------------------------------------------------------------
// Copy ctor and Assignment operators
// -----------------------------------------------------------------------

XMLDateTime::XMLDateTime(const XMLDateTime &toCopy)
: XMLNumber(toCopy)
, fBufferMaxLen(0)
, fBuffer(0)
, fMemoryManager(toCopy.fMemoryManager)
{
    copy(toCopy);
}

XMLDateTime& XMLDateTime::operator=(const XMLDateTime& rhs)
{
    if (this == &rhs)
        return *this;

    copy(rhs);
    return *this;
}

// -----------------------------------------------------------------------
// Implementation of Abstract Interface
// -----------------------------------------------------------------------

//
// We may simply return the handle to fBuffer
//
XMLCh*  XMLDateTime::getRawData() const
{
    return fBuffer;
}

const XMLCh*  XMLDateTime::getFormattedString() const
{
    return getRawData();
}

int XMLDateTime::getSign() const
{
    return 0;
}

// ---------------------------------------------------------------------------
//  Parsers
// ---------------------------------------------------------------------------

//
// [-]{CCYY-MM-DD}'T'{HH:MM:SS.MS}[TimeZone]
//
void XMLDateTime::parseDateTime()
{
    if (!initParser())
        ThrowXMLwithMemMgr1(SchemaDateTimeException
                , XMLExcepts::DateTime_dt_invalid
                , fBuffer ? fBuffer : XMLUni::fgZeroLenString
                , fMemoryManager);

    getDate();

    //fStart is supposed to point to 'T'
    if (fBuffer[fStart++] != DATETIME_SEPARATOR)
          ThrowXMLwithMemMgr1(SchemaDateTimeException
                , XMLExcepts::DateTime_dt_missingT
                , fBuffer
                , fMemoryManager);

    getTime();
    validateDateTime();
    normalize();
    fHasTime = true;
}

//
// [-]{CCYY-MM-DD}[TimeZone]
//
void XMLDateTime::parseDate()
{
    if (!initParser())
      ThrowXMLwithMemMgr1(SchemaDateTimeException
                , XMLExcepts::DateTime_date_invalid
                , fBuffer ? fBuffer : XMLUni::fgZeroLenString
                , fMemoryManager);

    getDate();
    parseTimeZone();
    validateDateTime();
    normalize();
}

void XMLDateTime::parseTime()
{
    if (!initParser())
        ThrowXMLwithMemMgr1(SchemaDateTimeException
                , XMLExcepts::DateTime_time_invalid
                , fBuffer ? fBuffer : XMLUni::fgZeroLenString
                , fMemoryManager);

    // time initialize to default values
    fValue[CentYear]= YEAR_DEFAULT;
    fValue[Month]   = MONTH_DEFAULT;
    fValue[Day]     = DAY_DEFAULT;

    getTime();

    validateDateTime();
    normalize();
    fHasTime = true;
}

//
// {---DD}[TimeZone]
//  01234
//
void XMLDateTime::parseDay()
{
    if (!initParser())
        ThrowXMLwithMemMgr1(SchemaDateTimeException
                , XMLExcepts::DateTime_gDay_invalid
                , fBuffer ? fBuffer : XMLUni::fgZeroLenString
                , fMemoryManager);

    if (fBuffer[0] != DATE_SEPARATOR ||
        fBuffer[1] != DATE_SEPARATOR ||
        fBuffer[2] != DATE_SEPARATOR  )
    {
        ThrowXMLwithMemMgr1(SchemaDateTimeException
                , XMLExcepts::DateTime_gDay_invalid
                , fBuffer
                , fMemoryManager);
    }

    //initialize values
    fValue[CentYear] = YEAR_DEFAULT;
    fValue[Month]    = MONTH_DEFAULT;
    fValue[Day]      = parseInt(fStart+3, fStart+5);

    if ( DAY_SIZE < fEnd )
    {
        int pos = XMLString::indexOf(UTC_SET, fBuffer[DAY_SIZE]);
        if (pos == -1 )
        {
            ThrowXMLwithMemMgr1(SchemaDateTimeException
                    , XMLExcepts::DateTime_gDay_invalid
                    , fBuffer
                    , fMemoryManager);
        }
        else
        {
            fValue[utc] = pos+1;
            getTimeZone(DAY_SIZE);
        }
    }

    validateDateTime();
    normalize();
}

//
// {--MM--}[TimeZone]
// {--MM}[TimeZone]
//  012345
//
void XMLDateTime::parseMonth()
{
    if (!initParser())
        ThrowXMLwithMemMgr1(SchemaDateTimeException
                , XMLExcepts::DateTime_gMth_invalid
                , fBuffer ? fBuffer : XMLUni::fgZeroLenString
                , fMemoryManager);

    if (fBuffer[0] != DATE_SEPARATOR ||
        fBuffer[1] != DATE_SEPARATOR  )
    {
        ThrowXMLwithMemMgr1(SchemaDateTimeException
                , XMLExcepts::DateTime_gMth_invalid
                , fBuffer
                , fMemoryManager);
    }

    //set constants
    fValue[CentYear] = YEAR_DEFAULT;
    fValue[Day]      = DAY_DEFAULT;
    fValue[Month]    = parseInt(2, 4);

    // REVISIT: allow both --MM and --MM-- now.
    // need to remove the following lines to disallow --MM--
    // when the errata is officially in the rec.
    fStart = 4;
    if ( fEnd >= fStart+2 && fBuffer[fStart] == DATE_SEPARATOR && fBuffer[fStart+1] == DATE_SEPARATOR )
    {
        fStart += 2;
    }

    //
    // parse TimeZone if any
    //
    if ( fStart < fEnd )
    {
        int pos = XMLString::indexOf(UTC_SET, fBuffer[fStart]);
        if ( pos == NOT_FOUND )
        {
            ThrowXMLwithMemMgr1(SchemaDateTimeException
                    , XMLExcepts::DateTime_gMth_invalid
                    , fBuffer
                    , fMemoryManager);
        }
        else
        {
            fValue[utc] = pos+1;
            getTimeZone(fStart);
        }
    }

    validateDateTime();
    normalize();
}

//
//[-]{CCYY}[TimeZone]
// 0  1234
//
void XMLDateTime::parseYear()
{
    if (!initParser())
        ThrowXMLwithMemMgr1(SchemaDateTimeException
                , XMLExcepts::DateTime_year_invalid
                , fBuffer ? fBuffer : XMLUni::fgZeroLenString
                , fMemoryManager);

    // skip the first '-' and search for timezone
    //
    int sign = findUTCSign((fBuffer[0] == chDash) ? 1 : 0);

    if (sign == NOT_FOUND)
    {
        fValue[CentYear] = parseIntYear(fEnd);
    }
    else
    {
        fValue[CentYear] = parseIntYear(sign);
        getTimeZone(sign);
    }

    //initialize values
    fValue[Month] = MONTH_DEFAULT;
    fValue[Day]   = DAY_DEFAULT;   //java is 1

    validateDateTime();
    normalize();
}

//
//{--MM-DD}[TimeZone]
// 0123456
//
void XMLDateTime::parseMonthDay()
{
    if (!initParser())
        ThrowXMLwithMemMgr1(SchemaDateTimeException
                , XMLExcepts::DateTime_gMthDay_invalid
                , fBuffer ? fBuffer : XMLUni::fgZeroLenString
                , fMemoryManager);

    if (fBuffer[0] != DATE_SEPARATOR ||
        fBuffer[1] != DATE_SEPARATOR ||
        fBuffer[4] != DATE_SEPARATOR )
    {
        ThrowXMLwithMemMgr1(SchemaDateTimeException
                , XMLExcepts::DateTime_gMthDay_invalid
                , fBuffer
                , fMemoryManager);
    }


    //initialize
    fValue[CentYear] = YEAR_DEFAULT;
    fValue[Month]    = parseInt(2, 4);
    fValue[Day]      = parseInt(5, 7);

    if ( MONTHDAY_SIZE < fEnd )
    {
        int pos = XMLString::indexOf(UTC_SET, fBuffer[MONTHDAY_SIZE]);
        if ( pos == NOT_FOUND )
        {
            ThrowXMLwithMemMgr1(SchemaDateTimeException
                    , XMLExcepts::DateTime_gMthDay_invalid
                    , fBuffer
                    , fMemoryManager);
        }
        else
        {
            fValue[utc] = pos+1;
            getTimeZone(MONTHDAY_SIZE);
        }
    }

    validateDateTime();
    normalize();
}

void XMLDateTime::parseYearMonth()
{
    if (!initParser())
        ThrowXMLwithMemMgr1(SchemaDateTimeException
                , XMLExcepts::DateTime_ym_invalid
                , fBuffer ? fBuffer : XMLUni::fgZeroLenString
                , fMemoryManager);

    // get date
    getYearMonth();
    fValue[Day] = DAY_DEFAULT;
    parseTimeZone();

    validateDateTime();
    normalize();
}

//
//PnYn MnDTnH nMnS: -P1Y2M3DT10H30M
//
// [-]{'P'{[n'Y'][n'M'][n'D']['T'][n'H'][n'M'][n'S']}}
//
//  Note: the n above shall be >= 0
//        if no time element found, 'T' shall be absent
//
void XMLDateTime::parseDuration()
{
    if (!initParser())
        ThrowXMLwithMemMgr1(SchemaDateTimeException
                , XMLExcepts::DateTime_dur_invalid
                , fBuffer ? fBuffer : XMLUni::fgZeroLenString
                , fMemoryManager);

    // must start with '-' or 'P'
    //
    XMLCh c = fBuffer[fStart++];
    if ( (c != DURATION_STARTER) &&
         (c != chDash)            )
    {
        ThrowXMLwithMemMgr1(SchemaDateTimeException
                , XMLExcepts::DateTime_dur_Start_dashP
                , fBuffer
                , fMemoryManager);
    }

    // 'P' must ALWAYS be present in either case
    if ( (c == chDash) &&
         (fBuffer[fStart++]!= DURATION_STARTER ))
    {
        ThrowXMLwithMemMgr1(SchemaDateTimeException
                , XMLExcepts::DateTime_dur_noP
                , fBuffer
                , fMemoryManager);
    }

    // java code
    //date[utc]=(c=='-')?'-':0;
    //fValue[utc] = UTC_STD;
    fValue[utc] = (fBuffer[0] == chDash? UTC_NEG : UTC_STD);

    int negate = ( fBuffer[0] == chDash ? -1 : 1);

    //
    // No negative value is allowed after 'P'
    //
    // eg P-1234, invalid
    //
    if (indexOf(fStart, fEnd, chDash) != NOT_FOUND)
    {
        ThrowXMLwithMemMgr1(SchemaDateTimeException
                , XMLExcepts::DateTime_dur_DashNotFirst
                , fBuffer
                , fMemoryManager);
    }

    //at least one number and designator must be seen after P
    bool designator = false;

    int endDate = indexOf(fStart, fEnd, DATETIME_SEPARATOR);
    if ( endDate == NOT_FOUND )
    {
        endDate = (int)fEnd;  // 'T' absent
    }

    //find 'Y'
    int end = indexOf(fStart, endDate, DURATION_Y);
    if ( end != NOT_FOUND )
    {
        //scan year
        fValue[CentYear] = negate * parseInt(fStart, end);
        fStart = end+1;
        designator = true;
    }

    end = indexOf(fStart, endDate, DURATION_M);
    if ( end != NOT_FOUND )
    {
        //scan month
        fValue[Month] = negate * parseInt(fStart, end);
        fStart = end+1;
        designator = true;
    }

    end = indexOf(fStart, endDate, DURATION_D);
    if ( end != NOT_FOUND )
    {
        //scan day
        fValue[Day] = negate * parseInt(fStart,end);
        fStart = end+1;
        designator = true;
    }

    if ( (fEnd == XMLSize_t (endDate)) &&   // 'T' absent
         (fStart != fEnd)   )   // something after Day
    {
        ThrowXMLwithMemMgr1(SchemaDateTimeException
                , XMLExcepts::DateTime_dur_inv_b4T
                , fBuffer
                , fMemoryManager);
    }

    if ( fEnd != XMLSize_t (endDate) ) // 'T' present
    {
        //scan hours, minutes, seconds
        //

        // skip 'T' first
        end = indexOf(++fStart, fEnd, DURATION_H);
        if ( end != NOT_FOUND )
        {
            //scan hours
            fValue[Hour] = negate * parseInt(fStart, end);
            fStart = end+1;
            designator = true;
        }

        end = indexOf(fStart, fEnd, DURATION_M);
        if ( end != NOT_FOUND )
        {
            //scan min
            fValue[Minute] = negate * parseInt(fStart, end);
            fStart = end+1;
            designator = true;
        }

        end = indexOf(fStart, fEnd, DURATION_S);
        if ( end != NOT_FOUND )
        {
            //scan seconds
            int mlsec = indexOf (fStart, end, MILISECOND_SEPARATOR);

            /***
             * Schema Errata: E2-23
             * at least one digit must follow the decimal point if it appears.
             * That is, the value of the seconds component must conform
             * to the following pattern: [0-9]+(.[0-9]+)?
             */
            if ( mlsec != NOT_FOUND )
            {
                /***
                 * make usure there is something after the '.' and before the end.
                 */
                if ( mlsec+1 == end )
                {
                    ThrowXMLwithMemMgr1(SchemaDateTimeException
                            , XMLExcepts::DateTime_dur_inv_seconds
                            , fBuffer
                            , fMemoryManager);
                }

                fValue[Second]     = negate * parseInt(fStart, mlsec);
                fMilliSecond        = negate * parseMiliSecond(mlsec+1, end);
            }
            else
            {
                fValue[Second] = negate * parseInt(fStart,end);
            }

            fStart = end+1;
            designator = true;
        }

        // no additional data should appear after last item
        // P1Y1M1DT is illigal value as well
        if ( (fStart != fEnd) ||
              fBuffer[--fStart] == DATETIME_SEPARATOR )
        {
            ThrowXMLwithMemMgr1(SchemaDateTimeException
                    , XMLExcepts::DateTime_dur_NoTimeAfterT
                    , fBuffer
                    , fMemoryManager);
        }
    }

    if ( !designator )
    {
        ThrowXMLwithMemMgr1(SchemaDateTimeException
                , XMLExcepts::DateTime_dur_NoElementAtAll
                , fBuffer
                , fMemoryManager);
    }

}

// ---------------------------------------------------------------------------
//  Scanners
// ---------------------------------------------------------------------------

//
// [-]{CCYY-MM-DD}
//
// Note: CCYY could be more than 4 digits
//       Assuming fStart point to the beginning of the Date Section
//       fStart updated to point to the position right AFTER the second 'D'
//       Since the lenght of CCYY might be variable, we can't check format upfront
//
void XMLDateTime::getDate()
{

    // Ensure enough chars in buffer
    if ( (fStart+YMD_MIN_SIZE) > fEnd)
        ThrowXMLwithMemMgr1(SchemaDateTimeException
                , XMLExcepts::DateTime_date_incomplete
                , fBuffer
                , fMemoryManager);

    getYearMonth();    // Scan YearMonth and
                       // fStart point to the next '-'

    if (fBuffer[fStart++] != DATE_SEPARATOR)
    {
        ThrowXMLwithMemMgr1(SchemaDateTimeException
                , XMLExcepts::DateTime_date_invalid
                , fBuffer
                , fMemoryManager);
        //("CCYY-MM must be followed by '-' sign");
    }

    fValue[Day] = parseInt(fStart, fStart+2);
    fStart += 2 ;  //fStart points right after the Day

    return;
}

//
// hh:mm:ss[.msssss]['Z']
// hh:mm:ss[.msssss][['+'|'-']hh:mm]
// 012345678
//
// Note: Assuming fStart point to the beginning of the Time Section
//       fStart updated to point to the position right AFTER the second 's'
//                                                  or ms if any
//
void XMLDateTime::getTime()
{

    // Ensure enough chars in buffer
    if ( (fStart+TIME_MIN_SIZE) > fEnd)
        ThrowXMLwithMemMgr1(SchemaDateTimeException
                , XMLExcepts::DateTime_time_incomplete
                , fBuffer
                , fMemoryManager);
        //"Imcomplete Time Format"

    // check (fixed) format first
    if ((fBuffer[fStart + 2] != TIME_SEPARATOR) ||
        (fBuffer[fStart + 5] != TIME_SEPARATOR)  )
    {
        ThrowXMLwithMemMgr1(SchemaDateTimeException
                , XMLExcepts::DateTime_time_invalid
                , fBuffer
                , fMemoryManager);
        //("Error in parsing time" );
    }

    //
    // get hours, minute and second
    //
    fValue[Hour]   = parseInt(fStart + 0, fStart + 2);
    fValue[Minute] = parseInt(fStart + 3, fStart + 5);
    fValue[Second] = parseInt(fStart + 6, fStart + 8);
    fStart += 8;

    // to see if any ms and/or utc part after that
    if (fStart >= fEnd)
        return;

    //find UTC sign if any
    int sign = findUTCSign(fStart);

    //parse miliseconds
    int milisec = (fBuffer[fStart] == MILISECOND_SEPARATOR)? (int)fStart : NOT_FOUND;
    if ( milisec != NOT_FOUND )
    {
        fStart++;   // skip the '.'
        // make sure we have some thing between the '.' and fEnd
        if (fStart >= fEnd)
        {
            ThrowXMLwithMemMgr1(SchemaDateTimeException
                    , XMLExcepts::DateTime_ms_noDigit
                    , fBuffer
                    , fMemoryManager);
            //("ms shall be present once '.' is present" );
        }

        if ( sign == NOT_FOUND )
        {
            fMilliSecond = parseMiliSecond(fStart, fEnd);  //get ms between '.' and fEnd
            fStart = fEnd;
        }
        else
        {
            fMilliSecond = parseMiliSecond(fStart, sign);  //get ms between UTC sign and fEnd
        }
	}
    else if(sign == 0 || XMLSize_t (sign) != fStart)
    {
        // seconds has more than 2 digits
        ThrowXMLwithMemMgr1(SchemaDateTimeException
                , XMLExcepts::DateTime_min_invalid
                , fBuffer
                , fMemoryManager);
    }

    //parse UTC time zone (hh:mm)
    if ( sign > 0 ) {
        getTimeZone(sign);
    }

}

//
// [-]{CCYY-MM}
//
// Note: CCYY could be more than 4 digits
//       fStart updated to point AFTER the second 'M' (probably meet the fEnd)
//
void XMLDateTime::getYearMonth()
{

    // Ensure enough chars in buffer
    if ( (fStart+YMONTH_MIN_SIZE) > fEnd)
        ThrowXMLwithMemMgr1(SchemaDateTimeException
                , XMLExcepts::DateTime_ym_incomplete
                , fBuffer
                , fMemoryManager);
        //"Imcomplete YearMonth Format";

    // skip the first leading '-'
    XMLSize_t start = ( fBuffer[0] == chDash ) ? fStart + 1 : fStart;

    //
    // search for year separator '-'
    //
    int yearSeparator = indexOf(start, fEnd, DATE_SEPARATOR);
    if ( yearSeparator == NOT_FOUND)
        ThrowXMLwithMemMgr1(SchemaDateTimeException
                , XMLExcepts::DateTime_ym_invalid
                , fBuffer
                , fMemoryManager);
        //("Year separator is missing or misplaced");

    fValue[CentYear] = parseIntYear(yearSeparator);
    fStart = yearSeparator + 1;  //skip the '-' and point to the first M

    //
    //gonna check we have enough byte for month
    //
    if ((fStart + 2) > fEnd )
        ThrowXMLwithMemMgr1(SchemaDateTimeException
                , XMLExcepts::DateTime_ym_noMonth
                , fBuffer
                , fMemoryManager);
        //"no month in buffer"

    fValue[Month] = parseInt(fStart, yearSeparator + 3);
    fStart += 2;  //fStart points right after the MONTH

    return;
}

void XMLDateTime::parseTimeZone()
{
    //fStart points right after the date
  	if ( fStart < fEnd ) {
        int pos = XMLString::indexOf(UTC_SET, fBuffer[fStart]);
    	if (pos == NOT_FOUND) {
            ThrowXMLwithMemMgr1(SchemaDateTimeException
                    , XMLExcepts::DateTime_tz_noUTCsign
                    , fBuffer
                    , fMemoryManager);
   		}
   		else {
            fValue[utc] = pos+1;
  	        getTimeZone(fStart);
   		}
    }

    return;
}

//
// 'Z'
// ['+'|'-']hh:mm
//
// Note: Assuming fStart points to the beginning of TimeZone section
//       fStart updated to meet fEnd
//
void XMLDateTime::getTimeZone(const XMLSize_t sign)
{

    if ( fBuffer[sign] == UTC_STD_CHAR )
    {
        if ((sign + 1) != fEnd )
        {
            ThrowXMLwithMemMgr1(SchemaDateTimeException
                    , XMLExcepts::DateTime_tz_stuffAfterZ
                    , fBuffer
                    , fMemoryManager);
            //"Error in parsing time zone");
        }

        return;
    }

    //
    // otherwise, it has to be this format
    // '[+|-]'hh:mm
    //    1   23456 7
    //   sign      fEnd
    //
    if ( ( ( sign + TIMEZONE_SIZE + 1) != fEnd )      ||
         ( fBuffer[sign + 3] != TIMEZONE_SEPARATOR ) )
    {
        ThrowXMLwithMemMgr1(SchemaDateTimeException
                , XMLExcepts::DateTime_tz_invalid
                , fBuffer
                , fMemoryManager);
        //("Error in parsing time zone");
    }

    fTimeZone[hh] = parseInt(sign+1, sign+3);
    fTimeZone[mm] = parseInt(sign+4, fEnd);

    return;
}

// ---------------------------------------------------------------------------
//  Validator and normalizer
// ---------------------------------------------------------------------------

/**
 * If timezone present - normalize dateTime  [E Adding durations to dateTimes]
 *
 * @param date   CCYY-MM-DDThh:mm:ss+03
 * @return CCYY-MM-DDThh:mm:ssZ
 */
void XMLDateTime::normalize()
{

    if ((fValue[utc] == UTC_UNKNOWN) ||
        (fValue[utc] == UTC_STD)      )
        return;

    int negate = (fValue[utc] == UTC_POS)? -1: 1;
    int temp;
    int carry;


    // we normalize a duration so could have 200M...
    //update months (may be modified additionaly below)
    temp = fValue[Month];
    fValue[Month] = modulo(temp, 1, 13);
    carry = fQuotient(temp, 1, 13);
    if (fValue[Month] <= 0) {
        fValue[Month]+= 12;
        carry--;
    }

    //add years (may be modified additionaly below)
    fValue[CentYear] += carry;

    // add mins
    temp = fValue[Minute] + negate * fTimeZone[mm];
    carry = fQuotient(temp, 60);
    fValue[Minute] = mod(temp, 60, carry);
    if (fValue[Minute] < 0) {
        fValue[Minute] += 60;
        carry--;
    }

    //add hours
    temp = fValue[Hour] + negate * fTimeZone[hh] + carry;
    carry = fQuotient(temp, 24);
    fValue[Hour] = mod(temp, 24, carry);
    if (fValue[Hour] < 0) {
        fValue[Hour] += 24;
        carry--;
    }

    fValue[Day] += carry;

    while (1)
    {
        temp = maxDayInMonthFor(fValue[CentYear], fValue[Month]);
        if (fValue[Day] < 1)
        {
            fValue[Day] += maxDayInMonthFor(fValue[CentYear], fValue[Month] - 1);
            carry = -1;
        }
        else if ( fValue[Day] > temp )
        {
            fValue[Day] -= temp;
            carry = 1;
        }
        else
        {
            break;
        }

        temp = fValue[Month] + carry;
        fValue[Month] = modulo(temp, 1, 13);
        if (fValue[Month] <=0) {
            fValue[Month]+= 12;
            fValue[CentYear]--;
        }
        fValue[CentYear] += fQuotient(temp, 1, 13);
    }

    // set to normalized
    fValue[utc] = UTC_STD;

    return;
}

void XMLDateTime::validateDateTime() const
{

    //REVISIT: should we throw an exception for not valid dates
    //          or reporting an error message should be sufficient?
    if ( fValue[CentYear] == 0 )
    {
        ThrowXMLwithMemMgr1(SchemaDateTimeException
                , XMLExcepts::DateTime_year_zero
                , fBuffer
                , fMemoryManager);
        //"The year \"0000\" is an illegal year value");
    }

    if ( fValue[Month] < 1  ||
         fValue[Month] > 12  )
    {
        ThrowXMLwithMemMgr1(SchemaDateTimeException
                , XMLExcepts::DateTime_mth_invalid
                , fBuffer
                , fMemoryManager);
		//"The month must have values 1 to 12");
    }

    //validate days
    if ( fValue[Day] > maxDayInMonthFor( fValue[CentYear], fValue[Month]) ||
         fValue[Day] == 0 )
    {
        XMLCh szMaxDay[3];
        XMLString::binToText(maxDayInMonthFor( fValue[CentYear], fValue[Month]), szMaxDay, 3, 10, fMemoryManager);
        ThrowXMLwithMemMgr2(SchemaDateTimeException
                , XMLExcepts::DateTime_day_invalid
                , fBuffer
                , szMaxDay
                , fMemoryManager);
        //"The day must have values 1 to 31");
    }

    //validate hours
    if ((fValue[Hour] < 0)  ||
        (fValue[Hour] > 24) ||
        ((fValue[Hour] == 24) && ((fValue[Minute] !=0) ||
                                  (fValue[Second] !=0) ||
                                  (fMilliSecond    !=0))))
    {
        ThrowXMLwithMemMgr1(SchemaDateTimeException
                , XMLExcepts::DateTime_hour_invalid
                , fBuffer
                , fMemoryManager);
        //("Hour must have values 0-23");
    }

    //validate minutes
    if ( fValue[Minute] < 0 ||
         fValue[Minute] > 59 )
    {
        ThrowXMLwithMemMgr1(SchemaDateTimeException
                , XMLExcepts::DateTime_min_invalid
                , fBuffer
                , fMemoryManager);
        //"Minute must have values 0-59");
    }

    //validate seconds
    if ( fValue[Second] < 0 ||
         fValue[Second] > 60 )
    {
        ThrowXMLwithMemMgr1(SchemaDateTimeException
                , XMLExcepts::DateTime_second_invalid
                , fBuffer
                , fMemoryManager);
        //"Second must have values 0-60");
    }

    //validate time-zone hours
    if ( (abs(fTimeZone[hh]) > 14) ||
         ((abs(fTimeZone[hh]) == 14) && (fTimeZone[mm] != 0)) )
    {
        ThrowXMLwithMemMgr1(SchemaDateTimeException
                , XMLExcepts::DateTime_tz_hh_invalid
                , fBuffer
                , fMemoryManager);
        //"Time zone should have range -14..+14");
    }

    //validate time-zone minutes
    if ( abs(fTimeZone[mm]) > 59 )
    {
        ThrowXMLwithMemMgr1(SchemaDateTimeException
                , XMLExcepts::DateTime_min_invalid
                , fBuffer
                , fMemoryManager);
        //("Minute must have values 0-59");
    }

    return;
}

// -----------------------------------------------------------------------
// locator and converter
// -----------------------------------------------------------------------
int XMLDateTime::indexOf(const XMLSize_t start, const XMLSize_t end, const XMLCh ch) const
{
    for ( XMLSize_t i = start; i < end; i++ )
        if ( fBuffer[i] == ch )
            return (int)i;

    return NOT_FOUND;
}

int XMLDateTime::findUTCSign (const XMLSize_t start)
{
    int  pos;
    for ( XMLSize_t index = start; index < fEnd; index++ )
    {
        pos = XMLString::indexOf(UTC_SET, fBuffer[index]);
        if ( pos != NOT_FOUND)
        {
            fValue[utc] = pos+1;   // refer to utcType, there is 1 diff
            return (int)index;
        }
    }

    return NOT_FOUND;
}

//
// Note:
//    start: starting point in fBuffer
//    end:   ending point in fBuffer (exclusive)
//    fStart NOT updated
//
int XMLDateTime::parseInt(const XMLSize_t start, const XMLSize_t end) const
{
    unsigned int retVal = 0;
    for (XMLSize_t i=start; i < end; i++) {

        if (fBuffer[i] < chDigit_0 || fBuffer[i] > chDigit_9)
            ThrowXMLwithMemMgr(NumberFormatException, XMLExcepts::XMLNUM_Inv_chars, fMemoryManager);

        retVal = (retVal * 10) + (unsigned int) (fBuffer[i] - chDigit_0);
    }

    return (int) retVal;
}

//
// Note:
//    start: pointing to the first digit after the '.'
//    end:   pointing to one position after the last digit
//    fStart NOT updated
//
double XMLDateTime::parseMiliSecond(const XMLSize_t start, const XMLSize_t end) const
{
    double div = 10;
    double retval = 0;

    for (XMLSize_t i=start; i < end; i++) {

        if (fBuffer[i] < chDigit_0 || fBuffer[i] > chDigit_9)
            ThrowXMLwithMemMgr(NumberFormatException, XMLExcepts::XMLNUM_Inv_chars, fMemoryManager);

        retval += (fBuffer[i] == chDigit_0) ? 0 : ((double) (fBuffer[i] - chDigit_0)/div);
        div *= 10;
    }

    // we don't check underflow occurs since
    // nothing we can do about it.
    return retval;
}

//
// [-]CCYY
//
// Note: start from fStart
//       end (exclusive)
//       fStart NOT updated
//
int XMLDateTime::parseIntYear(const XMLSize_t end) const
{
    // skip the first leading '-'
    XMLSize_t start = ( fBuffer[0] == chDash ) ? fStart + 1 : fStart;

    XMLSize_t length = end - start;
    if (length < 4)
    {
        ThrowXMLwithMemMgr1(SchemaDateTimeException
                , XMLExcepts::DateTime_year_tooShort
                , fBuffer
                , fMemoryManager);
        //"Year must have 'CCYY' format");
    }
    else if (length > 4 &&
             fBuffer[start] == chDigit_0)
    {
        ThrowXMLwithMemMgr1(SchemaDateTimeException
                , XMLExcepts::DateTime_year_leadingZero
                , fBuffer
                , fMemoryManager);
        //"Leading zeros are required if the year value would otherwise have fewer than four digits;
        // otherwise they are forbidden");
    }

    bool negative = (fBuffer[0] == chDash);
    int  yearVal = parseInt((negative ? 1 : 0), end);
    return ( negative ? (-1) * yearVal : yearVal );
}

/***
 * E2-41
 *
 *  3.2.7.2 Canonical representation
 *
 *  Except for trailing fractional zero digits in the seconds representation,
 *  '24:00:00' time representations, and timezone (for timezoned values),
 *  the mapping from literals to values is one-to-one. Where there is more
 *  than one possible representation, the canonical representation is as follows:
 *  redundant trailing zero digits in fractional-second literals are prohibited.
 *  An hour representation of '24' is prohibited. Timezoned values are canonically
 *  represented by appending 'Z' to the nontimezoned representation. (All
 *  timezoned dateTime values are UTC.)
 *
 *  .'24:00:00' -> '00:00:00'
 *  .milisecond: trailing zeros removed
 *  .'Z'
 *
 ***/
XMLCh* XMLDateTime::getDateTimeCanonicalRepresentation(MemoryManager* const memMgr) const
{
    XMLCh *miliStartPtr, *miliEndPtr;
    searchMiliSeconds(miliStartPtr, miliEndPtr);
    XMLSize_t miliSecondsLen = miliEndPtr - miliStartPtr;
    int utcSize = (fValue[utc] == UTC_UNKNOWN) ? 0 : 1;

    MemoryManager* toUse = memMgr? memMgr : fMemoryManager;
    XMLCh* retBuf = (XMLCh*) toUse->allocate( (21 + miliSecondsLen + utcSize + 1) * sizeof(XMLCh));
    XMLCh* retPtr = retBuf;

    // (-?) cc+yy-mm-dd'T'hh:mm:ss'Z'    ('.'s+)?
    //      2+  8       1      8   1
    //
    int additionalLen = fillYearString(retPtr, fValue[CentYear]);
    if(additionalLen != 0)
    {
        // very bad luck; have to resize the buffer...
        XMLCh *tmpBuf = (XMLCh*) toUse->allocate( (additionalLen+21+miliSecondsLen +2) * sizeof(XMLCh));
        XMLString::moveChars(tmpBuf, retBuf, 4+additionalLen);
        retPtr = tmpBuf+(retPtr-retBuf);
        toUse->deallocate(retBuf);
        retBuf = tmpBuf;
    }
    *retPtr++ = DATE_SEPARATOR;
    fillString(retPtr, fValue[Month], 2);
    *retPtr++ = DATE_SEPARATOR;
    fillString(retPtr, fValue[Day], 2);
    *retPtr++ = DATETIME_SEPARATOR;

    fillString(retPtr, fValue[Hour], 2);
    if (fValue[Hour] == 24)
    {
        *(retPtr - 2) = chDigit_0;
        *(retPtr - 1) = chDigit_0;
    }
    *retPtr++ = TIME_SEPARATOR;
    fillString(retPtr, fValue[Minute], 2);
    *retPtr++ = TIME_SEPARATOR;
    fillString(retPtr, fValue[Second], 2);

    if (miliSecondsLen)
    {
        *retPtr++ = chPeriod;
        XMLString::copyNString(retPtr, miliStartPtr, miliSecondsLen);
        retPtr += miliSecondsLen;
    }

    if (utcSize)
        *retPtr++ = UTC_STD_CHAR;
    *retPtr = chNull;

    return retBuf;
}

/***
 * E2-41
 *
 *  3.2.9.2 Canonical representation
 *
 * Given a member of the date value space, the date
 * portion of the canonical representation (the entire
 * representation for nontimezoned values, and all but
 * the timezone representation for timezoned values)
 * is always the date portion of the dateTime canonical
 * representation of the interval midpoint (the
 * dateTime representation, truncated on the right
 * to eliminate 'T' and all following characters).
 * For timezoned values, append the canonical
 * representation of the recoverable timezone.
 *
 ***/
XMLCh* XMLDateTime::getDateCanonicalRepresentation(MemoryManager* const memMgr) const
{
    /*
     * Case Date               Actual Value    Canonical Value
     *    1 yyyy-mm-dd         yyyy-mm-dd          yyyy-mm-dd
     *    2 yyyy-mm-ddZ        yyyy-mm-ddT00:00Z   yyyy-mm-ddZ
     *    3 yyyy-mm-dd+00:00   yyyy-mm-ddT00:00Z   yyyy-mm-ddZ
     *    4 yyyy-mm-dd+00:01   YYYY-MM-DCT23:59Z   yyyy-mm-dd+00:01
     *    5 yyyy-mm-dd+12:00   YYYY-MM-DCT12:00Z   yyyy-mm-dd+12:00
     *    6 yyyy-mm-dd+12:01   YYYY-MM-DCT11:59Z   YYYY-MM-DC-11:59
     *    7 yyyy-mm-dd+14:00   YYYY-MM-DCT10:00Z   YYYY-MM-DC-10:00
     *    8 yyyy-mm-dd-00:00   yyyy-mm-ddT00:00Z   yyyy-mm-ddZ
     *    9 yyyy-mm-dd-00:01   yyyy-mm-ddT00:01Z   yyyy-mm-dd-00:01
     *   11 yyyy-mm-dd-11:59   yyyy-mm-ddT11:59Z   YYYY-MM-DD-11:59
     *   10 yyyy-mm-dd-12:00   yyyy-mm-ddT12:00Z   YYYY-MM-DD+12:00
     *   12 yyyy-mm-dd-14:00   yyyy-mm-ddT14:00Z   YYYY-MM-DD+10:00
     */
    int utcSize = (fValue[utc] == UTC_UNKNOWN) ? 0 : 1;
    // YYYY-MM-DD  + chNull
    // 1234567890  + 1
    int memLength = 10 + 1 + utcSize;

    if (fTimeZone[hh] != 0 || fTimeZone[mm] != 0) {
        // YYYY-MM-DD+HH:MM  (utcSize will be 1 so drop that)
        // 1234567890123456
        memLength += 5; // 6 - 1 for utcSize
    }

    MemoryManager* toUse = memMgr? memMgr : fMemoryManager;
    XMLCh* retBuf = (XMLCh*) toUse->allocate( (memLength) * sizeof(XMLCh));
    XMLCh* retPtr = retBuf;

    if (fValue[Hour] < 12) {

        int additionalLen = fillYearString(retPtr, fValue[CentYear]);
        if (additionalLen != 0) {
            // very bad luck; have to resize the buffer...
            XMLCh *tmpBuf = (XMLCh*) toUse->allocate( (additionalLen + memLength ) * sizeof(XMLCh));
            XMLString::moveChars(tmpBuf, retBuf, 4+additionalLen);
            retPtr = tmpBuf+(retPtr-retBuf);
            toUse->deallocate(retBuf);
            retBuf = tmpBuf;
        }
        *retPtr++ = DATE_SEPARATOR;
        fillString(retPtr, fValue[Month], 2);
        *retPtr++ = DATE_SEPARATOR;
        fillString(retPtr, fValue[Day], 2);

        if (utcSize) {
            if (fTimeZone[hh] != 0 || fTimeZone[mm] != 0) {
                *retPtr++ = UTC_NEG_CHAR;
                fillString(retPtr, fValue[Hour], 2);
                *retPtr++ = TIME_SEPARATOR;
                fillString(retPtr, fValue[Minute], 2);
            }
            else {
                *retPtr++ = UTC_STD_CHAR;
            }
        }
        *retPtr = chNull;
    }
    else {
        /*
         * Need to reconvert things to get a recoverable time zone between
         * +12:00 and -11:59
         */
        int carry;
        int minute;
        int hour;
        int day;
        int month;
        int year;
        if (fValue[Minute] == 0) {
            minute = 0;
            carry = 0;
        }
        else {
            minute = 60 - fValue[Minute];
            carry = 1;
        }
        hour  = 24 - fValue[Hour] - carry;
        day   = fValue[Day] + 1;
        month = fValue[Month];
        year  = fValue[CentYear];

        while (1) {
            int temp = maxDayInMonthFor(year, month);
            if (day < 1) {
                day += maxDayInMonthFor(year, month - 1);
                carry = -1;
            }
            else if (day > temp) {
                day -= temp;
                carry = 1;
            }
            else {
                break;
            }

            temp = month + carry;
            month = modulo(temp, 1, 13);
            if (month <= 0) {
                month+= 12;
                year--;
            }
            year += fQuotient(temp, 1, 13);
        }

        int additionalLen = fillYearString(retPtr, year);
        if (additionalLen != 0) {
            // very bad luck; have to resize the buffer...
            XMLCh *tmpBuf = (XMLCh*) toUse->allocate( (additionalLen + memLength ) * sizeof(XMLCh));
            XMLString::moveChars(tmpBuf, retBuf, 4+additionalLen);
            retPtr = tmpBuf+(retPtr-retBuf);
            toUse->deallocate(retBuf);
            retBuf = tmpBuf;
        }
        *retPtr++ = DATE_SEPARATOR;
        fillString(retPtr, month, 2);
        *retPtr++ = DATE_SEPARATOR;
        fillString(retPtr, day, 2);

        *retPtr++ = UTC_POS_CHAR;
        fillString(retPtr, hour, 2);
        *retPtr++ = TIME_SEPARATOR;
        fillString(retPtr, minute, 2);
        *retPtr = chNull;
    }
    return retBuf;
}


/***
 * 3.2.8 time
 *
 *  . either the time zone must be omitted or,
 *    if present, the time zone must be Coordinated Universal Time (UTC) indicated by a "Z".
 *
 *  . Additionally, the canonical representation for midnight is 00:00:00.
 *
***/
XMLCh* XMLDateTime::getTimeCanonicalRepresentation(MemoryManager* const memMgr) const
{
    XMLCh *miliStartPtr, *miliEndPtr;
    searchMiliSeconds(miliStartPtr, miliEndPtr);
    XMLSize_t miliSecondsLen = miliEndPtr - miliStartPtr;
    int utcSize = (fValue[utc] == UTC_UNKNOWN) ? 0 : 1;

    MemoryManager* toUse = memMgr? memMgr : fMemoryManager;
    XMLCh* retBuf = (XMLCh*) toUse->allocate( (10 + miliSecondsLen + utcSize + 1) * sizeof(XMLCh));
    XMLCh* retPtr = retBuf;

    // 'hh:mm:ss'Z'    ('.'s+)?
    //      8    1
    //

    fillString(retPtr, fValue[Hour], 2);
    if (fValue[Hour] == 24)
    {
        *(retPtr - 2) = chDigit_0;
        *(retPtr - 1) = chDigit_0;
    }
    *retPtr++ = TIME_SEPARATOR;
    fillString(retPtr, fValue[Minute], 2);
    *retPtr++ = TIME_SEPARATOR;
    fillString(retPtr, fValue[Second], 2);

    if (miliSecondsLen)
    {
        *retPtr++ = chPeriod;
        XMLString::copyNString(retPtr, miliStartPtr, miliSecondsLen);
        retPtr += miliSecondsLen;
    }

    if (utcSize)
        *retPtr++ = UTC_STD_CHAR;
    *retPtr = chNull;

    return retBuf;
}

void XMLDateTime::fillString(XMLCh*& ptr, int value, XMLSize_t expLen) const
{
    XMLCh strBuffer[16];
    assert(expLen < 16);
    XMLString::binToText(value, strBuffer, expLen, 10, fMemoryManager);
    XMLSize_t actualLen = XMLString::stringLen(strBuffer);
    XMLSize_t i;
    //append leading zeros
    for (i = 0; i < expLen - actualLen; i++)
    {
        *ptr++ = chDigit_0;
    }

    for (i = 0; i < actualLen; i++)
    {
        *ptr++ = strBuffer[i];
    }

}

int XMLDateTime::fillYearString(XMLCh*& ptr, int value) const
{
    XMLCh strBuffer[16];
    // let's hope we get no years of 15 digits...
    XMLString::binToText(value, strBuffer, 15, 10, fMemoryManager);
    XMLSize_t actualLen = XMLString::stringLen(strBuffer);
    // don't forget that years can be negative...
    XMLSize_t negativeYear = 0;
    if(strBuffer[0] == chDash)
    {
        *ptr++ = strBuffer[0];
        negativeYear = 1;
    }
    XMLSize_t i;
    //append leading zeros
    if(actualLen+negativeYear < 4)
        for (i = 0; i < 4 - actualLen+negativeYear; i++)
            *ptr++ = chDigit_0;

    for (i = negativeYear; i < actualLen; i++)
        *ptr++ = strBuffer[i];

    if(actualLen > 4)
        return (int)actualLen-4;
    return 0;
}

/***
 *
 *   .check if the rawData has the mili second component
 *   .capture the substring
 *
 ***/
void XMLDateTime::searchMiliSeconds(XMLCh*& miliStartPtr, XMLCh*& miliEndPtr) const
{
    miliStartPtr = miliEndPtr = 0;

    int milisec = XMLString::indexOf(fBuffer, MILISECOND_SEPARATOR);
    if (milisec == -1)
        return;

    miliStartPtr = fBuffer + milisec + 1;
    miliEndPtr   = miliStartPtr;
    while (*miliEndPtr)
    {
        if ((*miliEndPtr < chDigit_0) || (*miliEndPtr > chDigit_9))
            break;

        miliEndPtr++;
    }

    //remove trailing zeros
    while( *(miliEndPtr - 1) == chDigit_0)
        miliEndPtr--;

    return;
}

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_TOCREATE(XMLDateTime)

void XMLDateTime::serialize(XSerializeEngine& serEng)
{
    //REVISIT: may not need to call base since it does nothing
    XMLNumber::serialize(serEng);

    int i = 0;

    if (serEng.isStoring())
    {
        for (i = 0; i < TOTAL_SIZE; i++)
        {
            serEng<<fValue[i];
        }

        for (i = 0; i < TIMEZONE_ARRAYSIZE; i++)
        {
            serEng<<fTimeZone[i];
        }

        serEng<<(unsigned long)fStart;
        serEng<<(unsigned long)fEnd;

        serEng.writeString(fBuffer, fBufferMaxLen, XSerializeEngine::toWriteBufferLen);
    }
    else
    {
        for (i = 0; i < TOTAL_SIZE; i++)
        {
            serEng>>fValue[i];
        }

        for (i = 0; i < TIMEZONE_ARRAYSIZE; i++)
        {
            serEng>>fTimeZone[i];
        }

        serEng>>(unsigned long&)fStart;
        serEng>>(unsigned long&)fEnd;

        XMLSize_t dataLen = 0;
        serEng.readString(fBuffer, fBufferMaxLen, dataLen ,XSerializeEngine::toReadBufferLen);

    }

}

XERCES_CPP_NAMESPACE_END
