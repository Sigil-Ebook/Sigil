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
 * $Id: XSValue.cpp 932887 2010-04-11 13:04:59Z borisk $
 */

#include <limits.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <float.h>

#include <xercesc/framework/psvi/XSValue.hpp>

#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLStringTokenizer.hpp>

#include <xercesc/util/XMLBigDecimal.hpp>
#include <xercesc/util/XMLBigInteger.hpp>
#include <xercesc/util/XMLFloat.hpp>
#include <xercesc/util/XMLDouble.hpp>
#include <xercesc/util/XMLDateTime.hpp>
#include <xercesc/util/HexBin.hpp>
#include <xercesc/util/Base64.hpp>
#include <xercesc/util/XMLUri.hpp>
#include <xercesc/util/XMLChar.hpp>
#include <xercesc/util/Janitor.hpp>
#include <xercesc/util/XMLInitializer.hpp>
#include <xercesc/util/regx/RegularExpression.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/util/NumberFormatException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/*** issues
 *
 *   1. For float, double, datetime family, the validation is almost similar to getActualValue
 *
 *
 *   DataType                   DataGroup
 *                             num  dtm  str             validation           canonical      actual-value
 *   ======================================================================================================
 *    dt_string                          str              [2] Char              NA             content
 *    dt_boolean                         str           {true, false, 1, 0}   {true, false}    bool
 *    dt_decimal               num                         lexical only         yes            double
 *    dt_float                 num                         lexical/value        yes            double
 *    dt_double                num                         lexical/value        yes            double
 *    ---------------------------------------------------------------------------------------------------------
 * 5  dt_duration                   dtm                    yes                  NA             struct datetime
 *    dt_dateTime                   dtm                    yes                  yes            struct datetime
 *    dt_time                       dtm                    yes                  yes            struct datetime
 *    dt_date                       dtm                    yes                  NA             struct datetime
 *    dt_gYearMonth                 dtm                    yes                  NA             struct datetime
 *    ---------------------------------------------------------------------------------------------------------
 * 10 dt_gYear                      dtm                    yes                  NA             struct datetime
 *    dt_gMonthDay                  dtm                    yes                  NA             struct datetime
 *    dt_gDay                       dtm                    yes                  NA             struct datetime
 *    dt_gMonth                     dtm                    yes                  NA             struct datetime
 *    dt_hexBinary                        str              decoding            ([a-f])         unsigned long ?
 *    ---------------------------------------------------------------------------------------------------------
 * 15 dt_base64Binary                     str              decoding             NA (errata?)   unsigned long ?
 *    dt_anyURI                           str              yes                  NA             content
 *    dt_QName                            str              a:b , [6]QName       NA             content
 *    dt_NOTATION                         str              [6]QName             NA             content
 *    dt_normalizedString                 str              no #xD #xA #x9       NA             content
 *    ---------------------------------------------------------------------------------------------------------
 * 20 dt_token                            str              no #xD #xA #x9 traling   NA         content
 *    dt_language                         str              language id          NA             content
 *    dt_NMTOKEN                          str              [7] Nmtoken          NA             content
 *    dt_NMTOKENS                         str              [8] Nmtokens         NA             content
 *    dt_Name                             str              [5] Name             NA             content
 *    ---------------------------------------------------------------------------------------------------------
 * 25 dt_NCName                           str              [4] NCName           NA             content
 *    dt_ID                               str              [4] NCName           NA             content
 *    dt_IDREF                            str              [4] NCName           NA             content
 *    dt_IDREFS                           str              ws seped IDREF       NA             content
 *    dt_ENTITY                           str              [4] NCName           NA             content
 *    ---------------------------------------------------------------------------------------------------------
 * 30 dt_ENTITIES                         str              ws seped ENTITY      NA             content
 *    dt_integer               num                         lexical              yes            long
 *    dt_nonPositiveInteger    num                         lexical              yes            long
 *    dt_negativeInteger       num                         lexical              yes            long
 *    dt_long                  num                         lexical              yes            long
 *    ---------------------------------------------------------------------------------------------------------
 * 35 dt_int                   num                         lexical              yes            int
 *    dt_short                 num                         lexical              yes            short
 *    dt_byte                  num                         lexical              yes            char
 *    dt_nonNegativeInteger    num                         lexical              yes            unsigned long
 *    dt_unsignedLong          num                         lexical              yes            unsigned long
 *    ---------------------------------------------------------------------------------------------------------
 * 40 dt_unsignedInt           num                         lexical              yes            unsigned int
 *    dt_unsignedShort         num                         lexical              yes            unsigned short
 *    dt_unsignedByte          num                         lexical              yes            unsigned char
 *    dt_positiveInteger       num                         lexical              yes            unsigned long
 *
 ***/

const XSValue::DataGroup XSValue::inGroup[XSValue::dt_MAXCOUNT] =
{
    dg_strings,   dg_strings,   dg_numerics,  dg_numerics,  dg_numerics,
    dg_datetimes, dg_datetimes, dg_datetimes, dg_datetimes, dg_datetimes,
    dg_datetimes, dg_datetimes, dg_datetimes, dg_datetimes, dg_strings,
    dg_strings,   dg_strings,   dg_strings,   dg_strings,   dg_strings,
    dg_strings,   dg_strings,   dg_strings,   dg_strings,   dg_strings,
    dg_strings,   dg_strings,   dg_strings,   dg_strings,   dg_strings,
    dg_strings,   dg_numerics,  dg_numerics,  dg_numerics,  dg_numerics,
    dg_numerics,  dg_numerics,  dg_numerics,  dg_numerics,  dg_numerics,
    dg_numerics,  dg_numerics,  dg_numerics,  dg_numerics
};

const bool XSValue::numericSign[XSValue::dt_MAXCOUNT] =
{
    true, true, true, true, true,
    true, true, true, true, true,
    true, true, true, true, true,
    true, true, true, true, true,
    true, true, true, true, true,
    true, true, true, true, true,
    true, true, true, true, true,
    true, true, true, false, false,
    false, false, false, false
};

// ---------------------------------------------------------------------------
//  Local static functions
// ---------------------------------------------------------------------------
static RegularExpression* sXSValueRegEx = 0;
ValueHashTableOf<XSValue::DataType>*  XSValue::fDataTypeRegistry = 0;

void XMLInitializer::initializeXSValue()
{
    sXSValueRegEx = new RegularExpression(
      XMLUni::fgLangPattern, SchemaSymbols::fgRegEx_XOption);

    XSValue::initializeRegistry();
}

void XMLInitializer::terminateXSValue()
{
    delete XSValue::fDataTypeRegistry;
    XSValue::fDataTypeRegistry = 0;

    delete sXSValueRegEx;
    sXSValueRegEx = 0;
}

XSValue::DataType  XSValue::getDataType(const XMLCh* const dtString)
{
    if (fDataTypeRegistry->containsKey(dtString)) {
        return fDataTypeRegistry->get(dtString);
    }

    return dt_MAXCOUNT;
}

void XSValue::initializeRegistry()
{
    //using the XMLPlatformUtils::fgMemoryManager
    fDataTypeRegistry  = new ValueHashTableOf<XSValue::DataType>(43);

    if (fDataTypeRegistry) {
        fDataTypeRegistry->put((void*) SchemaSymbols::fgDT_STRING,             XSValue::dt_string);
        fDataTypeRegistry->put((void*) SchemaSymbols::fgDT_BOOLEAN,            XSValue::dt_boolean);
        fDataTypeRegistry->put((void*) SchemaSymbols::fgDT_DECIMAL,            XSValue::dt_decimal);
        fDataTypeRegistry->put((void*) SchemaSymbols::fgDT_FLOAT,              XSValue::dt_float);
        fDataTypeRegistry->put((void*) SchemaSymbols::fgDT_DOUBLE,             XSValue::dt_double);
        fDataTypeRegistry->put((void*) SchemaSymbols::fgDT_DURATION,           XSValue::dt_duration);
        fDataTypeRegistry->put((void*) SchemaSymbols::fgDT_DATETIME,           XSValue::dt_dateTime);
        fDataTypeRegistry->put((void*) SchemaSymbols::fgDT_TIME,               XSValue::dt_time);
        fDataTypeRegistry->put((void*) SchemaSymbols::fgDT_DATE,               XSValue::dt_date);
        fDataTypeRegistry->put((void*) SchemaSymbols::fgDT_YEARMONTH,          XSValue::dt_gYearMonth);
        fDataTypeRegistry->put((void*) SchemaSymbols::fgDT_YEAR,               XSValue::dt_gYear);
        fDataTypeRegistry->put((void*) SchemaSymbols::fgDT_MONTHDAY,           XSValue::dt_gMonthDay);
        fDataTypeRegistry->put((void*) SchemaSymbols::fgDT_DAY,                XSValue::dt_gDay);
        fDataTypeRegistry->put((void*) SchemaSymbols::fgDT_MONTH,              XSValue::dt_gMonth);
        fDataTypeRegistry->put((void*) SchemaSymbols::fgDT_HEXBINARY,          XSValue::dt_hexBinary);
        fDataTypeRegistry->put((void*) SchemaSymbols::fgDT_BASE64BINARY,       XSValue::dt_base64Binary);
        fDataTypeRegistry->put((void*) SchemaSymbols::fgDT_ANYURI,             XSValue::dt_anyURI);
        fDataTypeRegistry->put((void*) SchemaSymbols::fgDT_QNAME,              XSValue::dt_QName);
        fDataTypeRegistry->put((void*) XMLUni::fgNotationString,               XSValue::dt_NOTATION);
        fDataTypeRegistry->put((void*) SchemaSymbols::fgDT_NORMALIZEDSTRING,   XSValue::dt_normalizedString);
        fDataTypeRegistry->put((void*) SchemaSymbols::fgDT_TOKEN,              XSValue::dt_token);
        fDataTypeRegistry->put((void*) SchemaSymbols::fgDT_LANGUAGE,           XSValue::dt_language);
        fDataTypeRegistry->put((void*) XMLUni::fgNmTokenString,                XSValue::dt_NMTOKEN);
        fDataTypeRegistry->put((void*) XMLUni::fgNmTokensString,               XSValue::dt_NMTOKENS);
        fDataTypeRegistry->put((void*) SchemaSymbols::fgDT_NAME,               XSValue::dt_Name);
        fDataTypeRegistry->put((void*) SchemaSymbols::fgDT_NCNAME,             XSValue::dt_NCName);
        fDataTypeRegistry->put((void*) XMLUni::fgIDString,                     XSValue::dt_ID);
        fDataTypeRegistry->put((void*) XMLUni::fgIDRefString,                  XSValue::dt_IDREF);
        fDataTypeRegistry->put((void*) XMLUni::fgIDRefsString,                 XSValue::dt_IDREFS);
        fDataTypeRegistry->put((void*) XMLUni::fgEntityString,                 XSValue::dt_ENTITY);
        fDataTypeRegistry->put((void*) XMLUni::fgEntitiesString,               XSValue::dt_ENTITIES);
        fDataTypeRegistry->put((void*) SchemaSymbols::fgDT_INTEGER,            XSValue::dt_integer);
        fDataTypeRegistry->put((void*) SchemaSymbols::fgDT_NONPOSITIVEINTEGER, XSValue::dt_nonPositiveInteger);
        fDataTypeRegistry->put((void*) SchemaSymbols::fgDT_NEGATIVEINTEGER,    XSValue::dt_negativeInteger);
        fDataTypeRegistry->put((void*) SchemaSymbols::fgDT_LONG,               XSValue::dt_long);
        fDataTypeRegistry->put((void*) SchemaSymbols::fgDT_INT,                XSValue::dt_int);
        fDataTypeRegistry->put((void*) SchemaSymbols::fgDT_SHORT,              XSValue::dt_short);
        fDataTypeRegistry->put((void*) SchemaSymbols::fgDT_BYTE,               XSValue::dt_byte);
        fDataTypeRegistry->put((void*) SchemaSymbols::fgDT_NONNEGATIVEINTEGER, XSValue::dt_nonNegativeInteger);
        fDataTypeRegistry->put((void*) SchemaSymbols::fgDT_ULONG,              XSValue::dt_unsignedLong);
        fDataTypeRegistry->put((void*) SchemaSymbols::fgDT_UINT,               XSValue::dt_unsignedInt);
        fDataTypeRegistry->put((void*) SchemaSymbols::fgDT_USHORT,             XSValue::dt_unsignedShort);
        fDataTypeRegistry->put((void*) SchemaSymbols::fgDT_UBYTE,              XSValue::dt_unsignedByte);
        fDataTypeRegistry->put((void*) SchemaSymbols::fgDT_POSITIVEINTEGER,    XSValue::dt_positiveInteger);
    }
}

static bool checkTimeZoneError(XSValue::DataType       const &datatype
                             , SchemaDateTimeException const &e       )
{
    return (((datatype == XSValue::dt_dateTime) || (datatype == XSValue::dt_time) || (datatype == XSValue::dt_date)) &&
            ((e.getCode() == XMLExcepts::DateTime_tz_noUTCsign)   ||
             (e.getCode() == XMLExcepts::DateTime_tz_stuffAfterZ) ||
             (e.getCode() == XMLExcepts::DateTime_tz_invalid)     ||
             (e.getCode() == XMLExcepts::DateTime_tz_hh_invalid)));
}

// ---------------------------------------------------------------------------
//  Local Data
// ---------------------------------------------------------------------------

static const XMLCh Separator_20[] = {chSpace, chNull};
static const XMLCh Separator_ws[] = {chSpace, chLF, chCR, chHTab, chNull};

// ---------------------------------------------------------------------------
//  XSValue: Constructors and Destructor
// ---------------------------------------------------------------------------
XSValue::XSValue(DataType        const dt
               , MemoryManager*  const manager)
    :fMemAllocated(false)
    ,fMemoryManager(manager)
{
    fData.f_datatype = dt;
}

XSValue::~XSValue()
{
    if (fMemAllocated)
        fMemoryManager->deallocate(fData.fValue.f_byteVal);
}

// ---------------------------------------------------------------------------
//  XSValue: Public Interface
//
//    No exception is thrown from these methods
//
// ---------------------------------------------------------------------------
bool XSValue::validate(const XMLCh*         const content
                      ,      DataType             datatype
                      ,      Status&              status
                      ,      XMLVersion           version
                      ,      MemoryManager* const manager)
{
    if (!content ||
        !*content ||
        ((version == ver_10) && (XMLChar1_0::isAllSpaces(content, XMLString::stringLen(content)))) ||
        ((version == ver_11) && (XMLChar1_1::isAllSpaces(content, XMLString::stringLen(content)))) ) {

        switch (datatype) {
        case XSValue::dt_string:
        case XSValue::dt_normalizedString:
        case XSValue::dt_token:
        case XSValue::dt_anyURI:
        case XSValue::dt_hexBinary:
        case XSValue::dt_base64Binary:
            status = st_Init;
            return true;
            break;
        default:
            status = st_NoContent;
            return false;
            break;
        }
    }

    status = st_Init;

    switch (inGroup[datatype]) {
    case XSValue::dg_numerics:
        return validateNumerics(content, datatype, status, manager);
        break;
    case XSValue::dg_datetimes:
        return validateDateTimes(content, datatype, status, manager);
        break;
    case XSValue::dg_strings:
        return validateStrings(content, datatype, status, version, manager);
        break;
    default:
        status = st_UnknownType;

        return false;
        break;
    }
    return false;
}

XMLCh*
XSValue::getCanonicalRepresentation(const XMLCh*         const content
                                   ,      DataType             datatype
                                   ,      Status&              status
                                   ,      XMLVersion           version
                                   ,      bool                 toValidate
                                   ,      MemoryManager* const manager)
{
    if (!content ||
        !*content ||
        ((version == ver_10) && (XMLChar1_0::isAllSpaces(content, XMLString::stringLen(content)))) ||
        ((version == ver_11) && (XMLChar1_1::isAllSpaces(content, XMLString::stringLen(content)))) ) {
        status = st_NoContent;
        return 0;
    }

    status = st_Init;

    switch (inGroup[datatype]) {
    case XSValue::dg_numerics:
        return getCanRepNumerics(content, datatype,  status, toValidate, manager);
        break;
    case XSValue::dg_datetimes:
        return getCanRepDateTimes(content, datatype,  status, toValidate, manager);
        break;
    case XSValue::dg_strings:
        return getCanRepStrings(content, datatype,  status, version, toValidate, manager);
        break;
    default:
        status = st_UnknownType;

        return 0;
        break;
    }
    return 0;
}

XSValue* XSValue::getActualValue(const XMLCh*         const content
                               ,       DataType             datatype
                               ,       Status&              status
                               ,       XMLVersion           version
                               ,       bool                 toValidate
                               ,       MemoryManager* const manager)
{
    if (!content ||
        !*content ||
        ((version == ver_10) && (XMLChar1_0::isAllSpaces(content, XMLString::stringLen(content)))) ||
        ((version == ver_11) && (XMLChar1_1::isAllSpaces(content, XMLString::stringLen(content)))) ) {
        status = st_NoContent;
        return 0;
    }

    status = st_Init;

    switch (inGroup[datatype]) {
    case XSValue::dg_numerics:
        return getActValNumerics(content, datatype,  status, toValidate, manager);
        break;
    case XSValue::dg_datetimes:
        return getActValDateTimes(content, datatype,  status, manager);
        break;
    case XSValue::dg_strings:
        return getActValStrings(content, datatype,  status, version, toValidate, manager);
        break;
    default:
        status = st_UnknownType;
        return 0;
        break;
    }
    return 0;
}

// ---------------------------------------------------------------------------
//  XSValue: Helpers
// ---------------------------------------------------------------------------

/***
 *
 *  Boundary checking is done against Schema Type's lexcial space
 ***/
bool
XSValue::validateNumerics(const XMLCh*         const content
                        ,       DataType             datatype
                        ,       Status&              status
                        ,       MemoryManager* const manager)
{

    try {

        switch (datatype) {
        case XSValue::dt_decimal:
            XMLBigDecimal::parseDecimal(content, manager);
            break;
        case XSValue::dt_float:
            {
                //XMLFloat takes care of 0, -0, -INF, INF and NaN
                //XMLFloat::checkBoundary() handles error and outofbound issues
                XMLFloat data(content, manager);
                break;
            }
        case XSValue::dt_double:
            {
                //XMLDouble takes care of 0, -0, -INF, INF and NaN
                //XMLDouble::checkBoundary() handles error and outofbound issues
                XMLDouble  data(content, manager);
                break;
            }
        /***
         *   For all potentially unrepresentable types
         *
         *   For dt_long, dt_unsignedLong, doing lexical space
         *   checking ensures consistent behaviour on 32/64 boxes
         *
         ***/
        case XSValue::dt_integer:
        case XSValue::dt_negativeInteger:
        case XSValue::dt_nonPositiveInteger:
        case XSValue::dt_nonNegativeInteger:
        case XSValue::dt_positiveInteger:
        case XSValue::dt_long:
        case XSValue::dt_unsignedLong:
            {
                XMLCh* compareData = (XMLCh*) manager->allocate((XMLString::stringLen(content) + 1) * sizeof(XMLCh));
                ArrayJanitor<XMLCh> janName(compareData, manager);
                int    signValue = 0;
                XMLBigInteger::parseBigInteger(content, compareData, signValue,  manager);

                switch (datatype) {
                case XSValue::dt_integer:
                    //error: no
                    break;
                case XSValue::dt_negativeInteger:
                    // error: > -1
                    if  (XMLBigInteger::compareValues(compareData
                                                    , signValue
                                                    , &(XMLUni::fgNegOne[1])
                                                    , -1
                                                    , manager)
                                                    == XMLNumber::GREATER_THAN)
                    {
                        status = st_FOCA0002;
                        return false;
                    }
                    break;
                case XSValue::dt_nonPositiveInteger:
                    // error: > 0
                     if (XMLBigInteger::compareValues(compareData
                                                   , signValue
                                                   , XMLUni::fgValueZero
                                                   , 0
                                                   , manager)
                                                   == XMLNumber::GREATER_THAN)
                    {
                        status = st_FOCA0002;
                        return false;
                    }
                    break;
                case XSValue::dt_nonNegativeInteger:
                    // error: < 0
                    if (XMLBigInteger::compareValues(compareData
                                                   , signValue
                                                   , XMLUni::fgValueZero
                                                   , 0
                                                   , manager)
                                                   == XMLNumber::LESS_THAN)
                    {
                        status = st_FOCA0002;
                        return false;
                    }
                    break;
                case XSValue::dt_positiveInteger:
                    // error: < 1
                    if (XMLBigInteger::compareValues(compareData
                                                   , signValue
                                                   , XMLUni::fgValueOne
                                                   , 1
                                                   , manager)
                                                   == XMLNumber::LESS_THAN)
                    {
                        status = st_FOCA0002;
                        return false;
                    }
                    break;
                case XSValue::dt_long:
                    // error: < -9223372036854775808 || > 9223372036854775807
                    if ((XMLBigInteger::compareValues(compareData
                                                    , signValue
                                                    , &(XMLUni::fgLongMinInc[1])
                                                    , -1
                                                    , manager)
                                                    == XMLNumber::LESS_THAN) ||
                        (XMLBigInteger::compareValues(compareData
                                                    , signValue
                                                    , XMLUni::fgLongMaxInc
                                                    , 1
                                                    , manager)
                                                    == XMLNumber::GREATER_THAN))
                    {
                        status = st_FOCA0002;
                        return false;
                    }
                    break;
                case XSValue::dt_unsignedLong:
                    // error: < 0 || > 18446744073709551615
                    if ((XMLBigInteger::compareValues(compareData
                                                    , signValue
                                                    , XMLUni::fgValueZero
                                                    , 0
                                                    , manager)
                                                    == XMLNumber::LESS_THAN) ||
                        (XMLBigInteger::compareValues(compareData
                                                    , signValue
                                                    , XMLUni::fgULongMaxInc
                                                    , 1
                                                    , manager)
                                                    == XMLNumber::GREATER_THAN))
                    {
                        status = st_FOCA0002;
                        return false;
                    }
                    break;
                default:
                    status = st_NotSupported;
                    return false;
                    break;
                }
                break;
            }
        case XSValue::dt_int:
        case XSValue::dt_short:
        case XSValue::dt_byte:
        case XSValue::dt_unsignedInt:
        case XSValue::dt_unsignedShort:
        case XSValue::dt_unsignedByte:
            {
                t_value   actVal;

                if ( !getActualNumericValue(
                                  content
                                , status
                                , actVal
                                , manager
                                , datatype
                                 )
                )
                {
                    return false;
                }
                break;
            }
        default:
            return false;
        } // end switch
    }
    catch (const NumberFormatException&)
    {
        //getActValue()/getCanonical() need to know the failure details
        //if validation is required
        status = st_FOCA0002;
        return false;
    }
    return true;  //both valid chars and within boundary
}

bool XSValue::validateDateTimes(const XMLCh*         const input_content
                              ,       DataType             datatype
                              ,       Status&              status
                              ,       MemoryManager* const manager)
{
    XMLCh* content = XMLString::replicate(input_content, manager);
    ArrayJanitor<XMLCh> janTmpName(content, manager);
    XMLString::trim(content);
    try
    {
        XMLDateTime coreDate = XMLDateTime(content, manager);

        switch (datatype) {
        case XSValue::dt_duration:
            coreDate.parseDuration();
            break;
        case XSValue::dt_dateTime:
            coreDate.parseDateTime();
            break;
        case XSValue::dt_time:
            coreDate.parseTime();
            break;
        case XSValue::dt_date:
            coreDate.parseDate();
            break;
        case XSValue::dt_gYearMonth:
            coreDate.parseYearMonth();
            break;
        case XSValue::dt_gYear:
            coreDate.parseYear();
            break;
        case XSValue::dt_gMonthDay:
            coreDate.parseMonthDay();
            break;
        case XSValue::dt_gDay:
            coreDate.parseDay();
            break;
        case XSValue::dt_gMonth:
            coreDate.parseMonth();
            break;
        default:
            return false;
            break;
        }
    }

    catch (const SchemaDateTimeException &e)
    {
        status = checkTimeZoneError(datatype, e)? XSValue::st_FODT0003 : st_FOCA0002;
        return false;
    }
    catch (const NumberFormatException&)
    {
        //getActValue()/getCanonical() need to know the failure details
        //if validation is required
        status = st_FOCA0002;
        return false;
    }

    return true; //parsing succeed
}

bool XSValue::validateStrings(const XMLCh*         const content
                            ,       DataType             datatype
                            ,       Status&              status
                            ,       XMLVersion           version
                            ,       MemoryManager* const manager)
{
    bool isValid = true;

    switch (datatype) {
        case XSValue::dt_boolean:
            {
                XMLSize_t i = 0;
                XMLCh* tmpStrValue = XMLString::replicate(content, manager);
                ArrayJanitor<XMLCh> janTmpName(tmpStrValue, manager);
                XMLString::trim(tmpStrValue);
                for (; i < XMLUni::fgBooleanValueSpaceArraySize; i++) {
                    if (XMLString::equals(tmpStrValue, XMLUni::fgBooleanValueSpace[i]))
                        break;
                }

                if (XMLUni::fgBooleanValueSpaceArraySize == i) {
                    isValid = false;
                }
                break;
            }
        case XSValue::dt_hexBinary:
            {
            XMLCh* tmpStrValue = XMLString::replicate(content, manager);
            ArrayJanitor<XMLCh> janTmpName(tmpStrValue, manager);
            XMLString::trim(tmpStrValue);
            if (HexBin::getDataLength(tmpStrValue) == -1) {
               isValid = false;
            }
            }
            break;
        case XSValue::dt_base64Binary:
            if (Base64::getDataLength(content, manager) == -1) {
                isValid = false;
            }
            break;
        case XSValue::dt_anyURI:
            if (XMLUri::isValidURI(true, content, true) == false) {
                isValid = false;
            }
            break;
        case XSValue::dt_QName:
            {
            XMLCh* tmpStrValue = XMLString::replicate(content, manager);
            ArrayJanitor<XMLCh> janTmpName(tmpStrValue, manager);
            XMLString::trim(tmpStrValue);
            isValid = (version == ver_10) ?
                XMLChar1_0::isValidQName(tmpStrValue, XMLString::stringLen(tmpStrValue)) :
                XMLChar1_1::isValidQName(tmpStrValue, XMLString::stringLen(tmpStrValue));
            }
            break;
        case XSValue::dt_NOTATION:
            {
            XMLCh* tmpStrValue = XMLString::replicate(content, manager);
            ArrayJanitor<XMLCh> janTmpName(tmpStrValue, manager);
            XMLString::trim(tmpStrValue);
            if ( XMLString::isValidNOTATION(tmpStrValue, manager) == false) {
            	isValid = false;
            }
            }
            break;
        case XSValue::dt_string:
            {
                const XMLCh*   rawPtr = content;

                if (version == ver_10) {
                    while (*rawPtr)
                        if (!XMLChar1_0::isXMLChar(*rawPtr++)) {
                            isValid = false;
                            break;
                        }
                }
                else {
                    while (*rawPtr)
                        if (!XMLChar1_1::isXMLChar(*rawPtr++)) {
                            isValid = false;
                            break;
                        }
                }
                break;
            }
        case XSValue::dt_normalizedString:
            {
                const XMLCh*   rawPtr = content;

                if (version == ver_10) {
                    while (*rawPtr) {
                        if (!XMLChar1_0::isXMLChar(*rawPtr)) {
                            isValid = false;
                            break;
                        }
                        else if (*rawPtr == chCR || *rawPtr == chLF || *rawPtr == chHTab) {
                            isValid = false;
                            break;
                        }
                        else {
                            rawPtr++;
                        }
                    }
                }
                else {
                    while (*rawPtr) {
                        if (!XMLChar1_1::isXMLChar(*rawPtr)) {
                            isValid = false;
                            break;
                        }
                        else if (*rawPtr == chCR || *rawPtr == chLF || *rawPtr == chHTab) {
                            isValid = false;
                            break;
                        }
                        else {
                            rawPtr++;
                        }

                    }
                }
                break;
            }
        case XSValue::dt_token:
        case XSValue::dt_language:
            {
                XMLSize_t     strLen = XMLString::stringLen(content);
                const XMLCh*  rawPtr = content;
                bool     inWS = false;

                if (version == ver_10) {
                    // Check leading/Trailing white space
                    if (XMLChar1_0::isWhitespace(content[0])      ||
                        XMLChar1_0::isWhitespace(content[strLen - 1])  ) {
                        isValid = false;
                    }
                    else {
                        while (*rawPtr) {
                            if (!XMLChar1_0::isXMLChar(*rawPtr)) {
                                isValid = false;
                                break;
                            }
                            else if (*rawPtr == chCR || *rawPtr == chLF || *rawPtr == chHTab) {
                                isValid = false;
                                break;
                            }
                            else if (XMLChar1_0::isWhitespace(*rawPtr)) {
                                if (inWS) {
                                    isValid = false;
                                    break;
                                }
                                else {
                                    inWS = true;
                                }
                            }
                            else {
                                inWS = false;
                            }

                            rawPtr++;
                        }
                    }
                }
                else {
                    // Check leading/Trailing white space
                    if (XMLChar1_1::isWhitespace(content[0])      ||
                        XMLChar1_1::isWhitespace(content[strLen - 1])  ) {
                        isValid = false;
                    }
                    else {
                        while (*rawPtr) {
                            if (!XMLChar1_1::isXMLChar(*rawPtr)) {
                                isValid = false;
                                break;
                            }
                            else if (*rawPtr == chCR || *rawPtr == chLF || *rawPtr == chHTab) {
                                isValid = false;
                                break;
                            }
                            else if (XMLChar1_1::isWhitespace(*rawPtr)) {
                                if (inWS) {
                                    isValid = false;
                                    break;
                                }
                                else {
                                    inWS = true;
                                }
                            }
                            else {
                                inWS = false;
                            }
                            rawPtr++;
                        }
                    }
                }
                if (isValid == true && datatype == XSValue::dt_language) {
                    if (!sXSValueRegEx) {
                        status = st_CantCreateRegEx;
                        isValid = false;
                    }
                    else
                    {
                        if (sXSValueRegEx->matches(content, manager) == false)
                        {
                            isValid = false;
                        }
                    }
                }
                break;
            }
        case XSValue::dt_NMTOKEN:
            isValid = (version == ver_10) ?
                XMLChar1_0::isValidNmtoken(content, XMLString::stringLen(content)) :
                XMLChar1_1::isValidNmtoken(content, XMLString::stringLen(content));
            break;
        case XSValue::dt_NMTOKENS:
            // [8]    Nmtokens   ::=    Nmtoken (#x20 Nmtoken)*
            {
                XMLStringTokenizer tokenizer(content, Separator_20, manager);

                if (version ==  ver_10) {
                    while (tokenizer.hasMoreTokens()) {
                        const XMLCh* token = tokenizer.nextToken();

                        if (!XMLChar1_0::isValidNmtoken(token, XMLString::stringLen(token))) {
                            isValid = false;
                            break;
                        }
                    }
                }
                else {
                    while (tokenizer.hasMoreTokens()) {
                        const XMLCh* token = tokenizer.nextToken();

                        if (!XMLChar1_1::isValidNmtoken(token, XMLString::stringLen(token))) {
                            isValid = false;
                            break;
                        }
                    }
                }
                break;
            }
        case XSValue::dt_Name:
            isValid = (version == ver_10) ?
                XMLChar1_0::isValidName(content) :
                XMLChar1_1::isValidName(content);
            break;
        case XSValue::dt_NCName:
        case XSValue::dt_ID:
        case XSValue::dt_IDREF:
        case XSValue::dt_ENTITY:
            isValid = (version == ver_10) ?
                XMLChar1_0::isValidNCName(content, XMLString::stringLen(content)) :
                XMLChar1_1::isValidNCName(content, XMLString::stringLen(content));
            break;
        case XSValue::dt_ENTITIES:
        case XSValue::dt_IDREFS:
            {
                XMLStringTokenizer tokenizer(content, Separator_ws, manager);

                if (version ==  ver_10 ) {
                    while (tokenizer.hasMoreTokens()) {
                        const XMLCh* token = tokenizer.nextToken();

                        if (!XMLChar1_0::isValidNCName(token, XMLString::stringLen(token))) {
                            isValid = false;
                            break;
                        }
                    }
                }
                else {
                    while (tokenizer.hasMoreTokens()) {
                        const XMLCh* token = tokenizer.nextToken();

                        if (!XMLChar1_1::isValidNCName(token, XMLString::stringLen(token))) {
                            isValid = false;
                            break;
                        }
                    }
                }
            }
            break;
        default:
            status = st_NotSupported;
            isValid = false;
            break;
    }


    if (isValid == false && status == st_Init) {
        status = st_FOCA0002;
    }

    return isValid;
}


XMLCh* XSValue::getCanRepNumerics(const XMLCh*         const content
                                ,       DataType             datatype
                                ,       Status&              status
                                ,       bool                 toValidate
                                ,       MemoryManager* const manager)
{
    try
    {

        // getCanonicalRepresentation does lexical space validation only
        // (no range checking), therefore if validation is required,
        // we need to pass the content to the validate interface for complete checking
        if (toValidate && !validateNumerics(content, datatype, status, manager))
            return 0;

        XMLCh* retVal = 0;

        if (datatype == XSValue::dt_decimal)
        {
            retVal = XMLBigDecimal::getCanonicalRepresentation(content, manager);

            if (!retVal)
                status = st_FOCA0002;

            return retVal;

        }
        else if (datatype == XSValue::dt_float || datatype == XSValue::dt_double  )
        {
            // In XML4C, no float or double is treated as out of range
            // it gets converted to INF, -INF or zero.
            // The getCanonical method should treat double & float the
            // same way as the rest of XML4C for consistentcy so need
            // to getActualValue and see if it was converted.
            XSValue* xsval = getActValNumerics(content, datatype, status, false, manager);
            if (!xsval) {
                status = st_FOCA0002;
                return retVal;
            }

            DoubleFloatType enumVal;
            if (datatype == XSValue::dt_float) {
                enumVal = xsval->fData.fValue.f_floatType.f_floatEnum;
            }
            else {
                enumVal = xsval->fData.fValue.f_doubleType.f_doubleEnum;
            }
            delete xsval;

            switch(enumVal) {
            case DoubleFloatType_NegINF:
                retVal = XMLString::replicate(XMLUni::fgNegINFString, manager);
                break;
            case DoubleFloatType_PosINF:
                retVal = XMLString::replicate(XMLUni::fgPosINFString, manager);
                break;
            case DoubleFloatType_NaN:
                retVal = XMLString::replicate(XMLUni::fgNaNString, manager);
                break;
            case DoubleFloatType_Zero:
                retVal = XMLString::replicate(XMLUni::fgPosZeroString, manager);
                break;
            default: //DoubleFloatType_Normal
                retVal = XMLAbstractDoubleFloat::getCanonicalRepresentation(content, manager);

                if (!retVal)
                    status = st_FOCA0002;
                break;
            }
            return retVal;
        }
        else
        {
            retVal = XMLBigInteger::getCanonicalRepresentation(content, manager, datatype == XSValue::dt_nonPositiveInteger);

            if (!retVal)
                status = st_FOCA0002;

            return retVal;
        }
    }
    catch (const NumberFormatException&)
    {
        status = st_FOCA0002;
    }

    return 0;
}

XMLCh* XSValue::getCanRepDateTimes(const XMLCh*         const input_content
                                 ,       DataType             datatype
                                 ,       Status&              status
                                 ,       bool                 toValidate
                                 ,       MemoryManager* const manager)
{
    XMLCh* content = XMLString::replicate(input_content, manager);
    ArrayJanitor<XMLCh> janTmpName(content, manager);
    XMLString::trim(content);
    try
    {

        XMLDateTime coreDate = XMLDateTime(content, manager);

        switch (datatype) {
        case XSValue::dt_dateTime:
            //we need this parsing
            coreDate.parseDateTime();
            return coreDate.getDateTimeCanonicalRepresentation(manager);
            break;
        case XSValue::dt_time:
            // we need this parsing
            coreDate.parseTime();
            return coreDate.getTimeCanonicalRepresentation(manager);
            break;
        case XSValue::dt_date:
            // we need this parsing
            coreDate.parseDate();
            return coreDate.getDateCanonicalRepresentation(manager);
            break;
        case XSValue::dt_duration:
        case XSValue::dt_gYearMonth:
        case XSValue::dt_gYear:
        case XSValue::dt_gMonthDay:
        case XSValue::dt_gDay:
        case XSValue::dt_gMonth:
            {
                if (!(toValidate && !validateDateTimes(content, datatype, status, manager)))
                    status = st_NoCanRep;

                return 0;
            }
            break;
        default:
            return 0;
            break;
        }
    }
    catch (SchemaDateTimeException &e)
    {
        status = checkTimeZoneError(datatype, e)? XSValue::st_FODT0003 : st_FOCA0002;
    }
    catch (const NumberFormatException&)
    {
        status = st_FOCA0002;
    }
    return 0;
}

XMLCh* XSValue::getCanRepStrings(const XMLCh*         const content
                               ,       DataType             datatype
                               ,       Status&              status
                               ,       XMLVersion           version
                               ,       bool                 toValidate
                               ,       MemoryManager* const manager)
{
    switch (datatype) {
        case XSValue::dt_boolean:
            {
            XMLCh* tmpStrValue = XMLString::replicate(content, manager);
            ArrayJanitor<XMLCh> janTmpName(tmpStrValue, manager);
            XMLString::trim(tmpStrValue);
            //always validate before getting canRep
            if (XMLString::equals(tmpStrValue, XMLUni::fgBooleanValueSpace[0]) ||
                XMLString::equals(tmpStrValue, XMLUni::fgBooleanValueSpace[2])  )
            {
                return XMLString::replicate(XMLUni::fgBooleanValueSpace[0], manager);
            }
            else if (XMLString::equals(tmpStrValue, XMLUni::fgBooleanValueSpace[1]) ||
                     XMLString::equals(tmpStrValue, XMLUni::fgBooleanValueSpace[3])  )
            {
                return XMLString::replicate(XMLUni::fgBooleanValueSpace[1], manager);
            }
            else
            {
                status = st_FOCA0002;
                return 0;
            }
            }
            break;
        case XSValue::dt_hexBinary:
            {
                //HexBin::getCanonicalRepresentation does validation automatically
                XMLCh* tmpStrValue = XMLString::replicate(content, manager);
                ArrayJanitor<XMLCh> janTmpName(tmpStrValue, manager);
                XMLString::trim(tmpStrValue);

                XMLCh* canRep = HexBin::getCanonicalRepresentation(tmpStrValue, manager);
                if (!canRep)
                    status = st_FOCA0002;

                return canRep;
                break;
            }
        case XSValue::dt_base64Binary:
            {
                //Base64::getCanonicalRepresentation does validation automatically
                XMLCh* canRep = Base64::getCanonicalRepresentation(content, manager);
                if (!canRep)
                    status = st_FOCA0002;

                return canRep;
                break;
            }
        case XSValue::dt_anyURI:
        case XSValue::dt_QName:
        case XSValue::dt_NOTATION:
        case XSValue::dt_string:
        case XSValue::dt_normalizedString:
        case XSValue::dt_token:
        case XSValue::dt_language:
        case XSValue::dt_NMTOKEN:
        case XSValue::dt_NMTOKENS:
        case XSValue::dt_Name:
        case XSValue::dt_NCName:
        case XSValue::dt_ID:
        case XSValue::dt_IDREF:
        case XSValue::dt_ENTITY:
        case XSValue::dt_ENTITIES:
        case XSValue::dt_IDREFS:
            if (toValidate && !validateStrings(content, datatype, status, version, manager))
                status = st_FOCA0002;
            else
                status = st_NoCanRep;

            return 0;
            break;
        default:
            return 0;
            break;
    }

    return 0;
}

XSValue*
XSValue::getActValNumerics(const XMLCh*         const content
                         ,       DataType             datatype
                         ,       Status&              status
                         ,       bool                 toValidate
                         ,       MemoryManager* const manager)
{

    try {

        switch (datatype) {
        case XSValue::dt_decimal:
        {
            if (toValidate) {
                XMLBigDecimal::parseDecimal(content, manager);
            }
            //Prepare the double value
            XMLDouble  data(content, manager);
            if (data.isDataConverted())
            {
                status = st_FOCA0001;
                return 0;
            }

            XSValue* retVal = new (manager) XSValue(dt_decimal, manager);
            retVal->fData.fValue.f_decimal.f_dvalue = data.getValue();

            return retVal;
            break;
        }
        case XSValue::dt_float:
        {
            //XMLFloat takes care of 0, -0, -INF, INF and NaN
            //XMLFloat::checkBoundary() handles error and outofbound issues
            XMLFloat data(content, manager);
            XSValue* retVal = new (manager) XSValue(dt_float, manager);

            if (data.isDataConverted())
            {
                retVal->fData.fValue.f_floatType.f_float = 0.0;
                retVal->fData.fValue.f_floatType.f_floatEnum = DoubleFloatType_Zero;

                switch(data.getType()) {
                    case XMLAbstractDoubleFloat::NegINF:
                        retVal->fData.fValue.f_floatType.f_floatEnum = DoubleFloatType_NegINF;
                        break;
                    case XMLAbstractDoubleFloat::PosINF:
                        retVal->fData.fValue.f_floatType.f_floatEnum = DoubleFloatType_PosINF;
                        break;
                    case XMLAbstractDoubleFloat::NaN:
                        retVal->fData.fValue.f_floatType.f_floatEnum = DoubleFloatType_NaN;
                        break;
                    default:
                        break;
                }
            }
            else {
                retVal->fData.fValue.f_floatType.f_floatEnum = DoubleFloatType_Normal;
                retVal->fData.fValue.f_floatType.f_float = (float) data.getValue();
            }
            return retVal;
            break;
        }
        case XSValue::dt_double:
        {
            //XMLDouble takes care of 0, -0, -INF, INF and NaN
            //XMLDouble::checkBoundary() handles error and outofbound issues
            XMLDouble  data(content, manager);
            XSValue* retVal = new (manager) XSValue(dt_double, manager);

            if (data.isDataConverted())
            {
                retVal->fData.fValue.f_doubleType.f_double = 0.0;
                retVal->fData.fValue.f_doubleType.f_doubleEnum = DoubleFloatType_Zero;

                switch(data.getType()) {
                    case XMLAbstractDoubleFloat::NegINF:
                        retVal->fData.fValue.f_doubleType.f_doubleEnum = DoubleFloatType_NegINF;
                        break;
                    case XMLAbstractDoubleFloat::PosINF:
                        retVal->fData.fValue.f_doubleType.f_doubleEnum = DoubleFloatType_PosINF;
                        break;
                    case XMLAbstractDoubleFloat::NaN:
                        retVal->fData.fValue.f_doubleType.f_doubleEnum = DoubleFloatType_NaN;
                        break;
                    default:
                        break;
                }
            }
            else {
                retVal->fData.fValue.f_doubleType.f_doubleEnum = DoubleFloatType_Normal;
                retVal->fData.fValue.f_doubleType.f_double = data.getValue();
            }
            return retVal;
            break;
        }
        case XSValue::dt_integer:
        case XSValue::dt_negativeInteger:
        case XSValue::dt_nonPositiveInteger:
        case XSValue::dt_nonNegativeInteger:
        case XSValue::dt_positiveInteger:
        case XSValue::dt_long:
        case XSValue::dt_int:
        case XSValue::dt_short:
        case XSValue::dt_byte:
        case XSValue::dt_unsignedLong:
        case XSValue::dt_unsignedInt:
        case XSValue::dt_unsignedShort:
        case XSValue::dt_unsignedByte:
        {
            t_value   actVal;

            if ( !getActualNumericValue(
                                  content
                                , status
                                , actVal
                                , manager
                                , datatype
                                )
                )
            {
                //status has been set by getActualNumericValue
                return 0;
            }

            XSValue* retVal = new (manager) XSValue(datatype, manager);

            switch (datatype) {
                case XSValue::dt_integer:
                    retVal->fData.fValue.f_long = actVal.f_long;
                    break;
                case XSValue::dt_negativeInteger:
                    retVal->fData.fValue.f_long = actVal.f_long;
                    break;
                case XSValue::dt_nonPositiveInteger:
                    retVal->fData.fValue.f_long = actVal.f_long;
                    break;
                case XSValue::dt_nonNegativeInteger:
                    retVal->fData.fValue.f_long = actVal.f_ulong;
                    break;
                case XSValue::dt_positiveInteger:
                    retVal->fData.fValue.f_long = actVal.f_ulong;
                    break;
                case XSValue::dt_long:
                    retVal->fData.fValue.f_long = actVal.f_long;
                    break;
                case XSValue::dt_int:
                    retVal->fData.fValue.f_int = (int) actVal.f_long;
                    break;
                case XSValue::dt_short:
                    retVal->fData.fValue.f_short = (short) actVal.f_long;
                    break;
                case XSValue::dt_byte:
                    retVal->fData.fValue.f_char = (char) actVal.f_long;
                    break;
                case XSValue::dt_unsignedLong:
                    retVal->fData.fValue.f_ulong = actVal.f_ulong;
                    break;
                case XSValue::dt_unsignedInt:
                    retVal->fData.fValue.f_uint = (unsigned int) actVal.f_ulong;
                    break;
                case XSValue::dt_unsignedShort:
                    retVal->fData.fValue.f_ushort = (unsigned short) actVal.f_ulong;
                    break;
                case XSValue::dt_unsignedByte:
                    retVal->fData.fValue.f_uchar = (unsigned char) actVal.f_ulong;
                    break;
                default:
                    return 0;
                    break;
            }
            return retVal;
            break;
        }
        default:
            return 0;
            break;
        } // end switch
    }
    catch (const NumberFormatException&)
    {
        status = st_FOCA0002;
    }
    return 0;
}

XSValue*
XSValue::getActValDateTimes(const XMLCh*         const input_content
                          ,       DataType             datatype
                          ,       Status&              status
                          ,       MemoryManager* const manager)
{
    XMLCh* content = XMLString::replicate(input_content, manager);
    ArrayJanitor<XMLCh> janTmpName(content, manager);
    XMLString::trim(content);
    try
    {
        //Need not check if validation is requested since
        //parsing functions below does the validation automatically
        XMLDateTime coreDate = XMLDateTime(content, manager);

        switch (datatype) {
        case XSValue::dt_duration:
            coreDate.parseDuration();
            break;
        case XSValue::dt_dateTime:
            coreDate.parseDateTime();
            break;
        case XSValue::dt_time:
            coreDate.parseTime();
            coreDate.fValue[XMLDateTime::CentYear] = 0;
            coreDate.fValue[XMLDateTime::Month] = 0;
            coreDate.fValue[XMLDateTime::Day] = 0;
            break;
        case XSValue::dt_date:
            coreDate.parseDate();
            coreDate.fValue[XMLDateTime::Hour] = 0;
            coreDate.fValue[XMLDateTime::Minute] = 0;
            break;
        case XSValue::dt_gYearMonth:
            coreDate.parseYearMonth();
            coreDate.fValue[XMLDateTime::Day] = 0;
            coreDate.fValue[XMLDateTime::Hour] = 0;
            coreDate.fValue[XMLDateTime::Minute] = 0;
            break;
        case XSValue::dt_gYear:
            coreDate.parseYear();
            coreDate.fValue[XMLDateTime::Month] = 0;
            coreDate.fValue[XMLDateTime::Day] = 0;
            coreDate.fValue[XMLDateTime::Hour] = 0;
            coreDate.fValue[XMLDateTime::Minute] = 0;
            break;
        case XSValue::dt_gMonthDay:
            coreDate.parseMonthDay();
            coreDate.fValue[XMLDateTime::CentYear] = 0;
            coreDate.fValue[XMLDateTime::Hour] = 0;
            coreDate.fValue[XMLDateTime::Minute] = 0;
            break;
        case XSValue::dt_gDay:
            coreDate.parseDay();
            coreDate.fValue[XMLDateTime::CentYear] = 0;
            coreDate.fValue[XMLDateTime::Month] = 0;
            coreDate.fValue[XMLDateTime::Hour] = 0;
            coreDate.fValue[XMLDateTime::Minute] = 0;
            break;
        case XSValue::dt_gMonth:
            coreDate.parseMonth();
            coreDate.fValue[XMLDateTime::CentYear] = 0;
            coreDate.fValue[XMLDateTime::Day] = 0;
            coreDate.fValue[XMLDateTime::Hour] = 0;
            coreDate.fValue[XMLDateTime::Minute] = 0;
            break;
        default:
            return 0;
            break;
        }

        XSValue* retVal = new (manager) XSValue(datatype, manager);

        retVal->fData.fValue.f_datetime.f_year    = coreDate.fValue[XMLDateTime::CentYear];
        retVal->fData.fValue.f_datetime.f_month   = coreDate.fValue[XMLDateTime::Month];
        retVal->fData.fValue.f_datetime.f_day     = coreDate.fValue[XMLDateTime::Day];
        retVal->fData.fValue.f_datetime.f_hour    = coreDate.fValue[XMLDateTime::Hour];
        retVal->fData.fValue.f_datetime.f_min     = coreDate.fValue[XMLDateTime::Minute];
        retVal->fData.fValue.f_datetime.f_second  = coreDate.fValue[XMLDateTime::Second];
        retVal->fData.fValue.f_datetime.f_milisec = coreDate.fMilliSecond;

        return retVal;
    }
    catch (SchemaDateTimeException const &e)
    {
        status = checkTimeZoneError(datatype, e)? XSValue::st_FODT0003 : st_FOCA0002;
    }
    catch (const NumberFormatException&)
    {
        status = st_FOCA0002;
    }
    return 0;

}

XSValue*
XSValue::getActValStrings(const XMLCh*         const content
                        ,       DataType             datatype
                        ,       Status&              status
                        ,       XMLVersion           version
                        ,       bool                 toValidate
                        ,       MemoryManager* const manager)
{
    switch (datatype) {
        case XSValue::dt_boolean:
            {
            XMLCh* tmpStrValue = XMLString::replicate(content, manager);
            ArrayJanitor<XMLCh> janTmpName(tmpStrValue, manager);
            XMLString::trim(tmpStrValue);
            //do validation here more efficiently
            if (XMLString::equals(tmpStrValue, XMLUni::fgBooleanValueSpace[0]) ||
                XMLString::equals(tmpStrValue, XMLUni::fgBooleanValueSpace[2])  )
            {
                XSValue* retVal = new (manager) XSValue(dt_boolean, manager);
                retVal->fData.fValue.f_bool = false;
                return retVal;
            }
            else if (XMLString::equals(tmpStrValue, XMLUni::fgBooleanValueSpace[1]) ||
                     XMLString::equals(tmpStrValue, XMLUni::fgBooleanValueSpace[3])  )
            {
                XSValue* retVal = new (manager) XSValue(dt_boolean, manager);
                retVal->fData.fValue.f_bool = true;
                return retVal;
            }
            else
            {
                status = st_FOCA0002;
                return 0;
            }
            }
            break;
        case XSValue::dt_hexBinary:
            {
                XMLCh* tmpStrValue = XMLString::replicate(content, manager);
                ArrayJanitor<XMLCh> janTmpName(tmpStrValue, manager);
                XMLString::trim(tmpStrValue);

                XMLByte* decodedData = HexBin::decodeToXMLByte(tmpStrValue, manager);

                if (!decodedData)
                {
                    status = st_FOCA0002;
                    return 0;
                }

                XSValue* retVal = new (manager) XSValue(dt_hexBinary, manager);
                retVal->fData.fValue.f_byteVal = decodedData;
                retVal->fMemAllocated = true;
                return retVal;
                break;
            }
        case XSValue::dt_base64Binary:
            {
                XMLSize_t len = 0;
                XMLByte* decodedData = Base64::decodeToXMLByte(content, &len, manager);

                if (!decodedData)
                {
                    status = st_FOCA0002;
                    return 0;
                }

                XSValue* retVal = new (manager) XSValue(dt_base64Binary, manager);
                retVal->fData.fValue.f_byteVal = decodedData;
                retVal->fMemAllocated = true;
                return retVal;
                break;
            }
        case XSValue::dt_anyURI:
        case XSValue::dt_QName:
        case XSValue::dt_NOTATION:
        case XSValue::dt_string:
        case XSValue::dt_normalizedString:
        case XSValue::dt_token:
        case XSValue::dt_language:
        case XSValue::dt_NMTOKEN:
        case XSValue::dt_NMTOKENS:
        case XSValue::dt_Name:
        case XSValue::dt_NCName:
        case XSValue::dt_ID:
        case XSValue::dt_IDREF:
        case XSValue::dt_ENTITY:
        case XSValue::dt_ENTITIES:
        case XSValue::dt_IDREFS:
            if (toValidate && !validateStrings(content, datatype, status, version, manager))
                status = st_FOCA0002;
            else
                status = st_NoActVal;

            return 0;
            break;
        default:
            return 0;
            break;
    }

    return 0;
}

// ---------------------------------------------------------------------------
//  Utilities
// ---------------------------------------------------------------------------
bool XSValue::getActualNumericValue(const XMLCh*  const content
                           ,       Status&              status
                           ,       t_value&             retVal
                           ,       MemoryManager* const manager
                           ,       DataType             datatype)
{
    char *nptr = XMLString::transcode(content, manager);
    ArrayJanitor<char> jan(nptr, manager);
    char *endptr = 0;
    errno = 0;

    if (XSValue::numericSign[datatype])
    {
        retVal.f_long = strtol(nptr, &endptr, (int)10);
    }
    else
    {
        if (XMLString::indexOf(content, chDash) != -1)
        {
            status = st_FOCA0002; //invalid lexcial value
            return false;
        }

        retVal.f_ulong = strtoul(nptr, &endptr, (int)10);
    }

    // need to check out-of-bounds before checking erange...
    switch (datatype) {
        case XSValue::dt_nonPositiveInteger:
            if (retVal.f_long > 0)
            {
                status = st_FOCA0002;
                return false;
            }
            break;
        case XSValue::dt_negativeInteger:
            if (retVal.f_long >= 0)
            {
                status = st_FOCA0002;
                return false;
            }
            break;
        case XSValue::dt_int:
            // strtol will set value to LONG_MIN/LONG_MAX if ERANGE error
            if ((retVal.f_long < INT_MIN) ||
                (retVal.f_long > INT_MAX) ||
                (errno == ERANGE))
            {
                status = st_FOCA0002;
                return false;
            }
            break;
        case XSValue::dt_short:
            if ((retVal.f_long < SHRT_MIN) ||
                (retVal.f_long > SHRT_MAX))
            {
                status = st_FOCA0002;
                return false;
            }
            break;
        case XSValue::dt_byte:
            if ((retVal.f_long < SCHAR_MIN) ||
                (retVal.f_long > SCHAR_MAX))
            {
                status = st_FOCA0002;
                return false;
            }
            break;
        case XSValue::dt_unsignedInt:
            // strtoul will set value to LONG_INT if ERANGE error
            if ((retVal.f_ulong > UINT_MAX)  ||
                (errno == ERANGE))
            {
                status = st_FOCA0002;
                return false;
            }
            break;
        case XSValue::dt_unsignedShort:
            if (retVal.f_ulong > USHRT_MAX)
            {
                status = st_FOCA0002;
                return false;
            }
            break;
        case XSValue::dt_unsignedByte:
            if (retVal.f_ulong > UCHAR_MAX)
            {
                status = st_FOCA0002;
                return false;
            }
            break;
        case XSValue::dt_positiveInteger:
            if (retVal.f_ulong == 0)
            {
                status = st_FOCA0002;
                return false;
            }
            break;
        default:
            break;
    } // end switch
    // check if overflow/underflow occurs
    if (errno == ERANGE)
    {
        status = st_FOCA0003;
        return false;
    }

    // check if all chars are valid char.  If they are, endptr will
    // pointer to the null terminator, or all of the remaining
    // characters will be whitespace characters.
    while (*endptr != '\0')
    {
        const char  ch = *endptr;

        if (ch == '\t' || ch == '\n' || ch == '\r' || ch == ' ')
        {
            ++endptr;
        }
        else
        {
            status = st_FOCA0002;
            return false;
        }

    }
    return true;
}

XERCES_CPP_NAMESPACE_END
