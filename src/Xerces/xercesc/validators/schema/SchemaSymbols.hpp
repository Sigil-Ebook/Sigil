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
 * $Id: SchemaSymbols.hpp 802804 2009-08-10 14:21:48Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_SCHEMASYMBOLS_HPP)
#define XERCESC_INCLUDE_GUARD_SCHEMASYMBOLS_HPP

#include <xercesc/util/XercesDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/*
 * Collection of symbols used to parse a Schema Grammar
 */

class VALIDATORS_EXPORT SchemaSymbols
{
public :
    // -----------------------------------------------------------------------
    // Constant data
    // -----------------------------------------------------------------------
    static const XMLCh fgURI_XSI[];
    static const XMLCh fgURI_SCHEMAFORSCHEMA[];
    // deprecated (typo)
    static const XMLCh fgXSI_SCHEMALOCACTION[];
    // deprecated (typo)
    static const XMLCh fgXSI_NONAMESPACESCHEMALOCACTION[];
    static const XMLCh fgXSI_SCHEMALOCATION[];
    static const XMLCh fgXSI_NONAMESPACESCHEMALOCATION[];
    static const XMLCh fgXSI_TYPE[];
    static const XMLCh fgELT_ALL[];
    static const XMLCh fgELT_ANNOTATION[];
    static const XMLCh fgELT_ANY[];
    static const XMLCh fgELT_WILDCARD[];
    static const XMLCh fgELT_ANYATTRIBUTE[];
    static const XMLCh fgELT_APPINFO[];
    static const XMLCh fgELT_ATTRIBUTE[];
    static const XMLCh fgELT_ATTRIBUTEGROUP[];
    static const XMLCh fgELT_CHOICE[];
    static const XMLCh fgELT_COMPLEXTYPE[];
    static const XMLCh fgELT_CONTENT[];
    static const XMLCh fgELT_DOCUMENTATION[];
    static const XMLCh fgELT_DURATION[];
    static const XMLCh fgELT_ELEMENT[];
    static const XMLCh fgELT_ENCODING[];
    static const XMLCh fgELT_ENUMERATION[];
    static const XMLCh fgELT_FIELD[];
    static const XMLCh fgELT_WHITESPACE[];
    static const XMLCh fgELT_GROUP[];
    static const XMLCh fgELT_IMPORT[];
    static const XMLCh fgELT_INCLUDE[];
    static const XMLCh fgELT_REDEFINE[];
    static const XMLCh fgELT_KEY[];
    static const XMLCh fgELT_KEYREF[];
    static const XMLCh fgELT_LENGTH[];
    static const XMLCh fgELT_MAXEXCLUSIVE[];
    static const XMLCh fgELT_MAXINCLUSIVE[];
    static const XMLCh fgELT_MAXLENGTH[];
    static const XMLCh fgELT_MINEXCLUSIVE[];
    static const XMLCh fgELT_MININCLUSIVE[];
    static const XMLCh fgELT_MINLENGTH[];
    static const XMLCh fgELT_NOTATION[];
    static const XMLCh fgELT_PATTERN[];
    static const XMLCh fgELT_PERIOD[];
    static const XMLCh fgELT_TOTALDIGITS[];
    static const XMLCh fgELT_FRACTIONDIGITS[];
    static const XMLCh fgELT_SCHEMA[];
    static const XMLCh fgELT_SELECTOR[];
    static const XMLCh fgELT_SEQUENCE[];
    static const XMLCh fgELT_SIMPLETYPE[];
    static const XMLCh fgELT_UNION[];
    static const XMLCh fgELT_LIST[];
    static const XMLCh fgELT_UNIQUE[];
    static const XMLCh fgELT_COMPLEXCONTENT[];
    static const XMLCh fgELT_SIMPLECONTENT[];
    static const XMLCh fgELT_RESTRICTION[];
    static const XMLCh fgELT_EXTENSION[];
    static const XMLCh fgATT_ABSTRACT[];
    static const XMLCh fgATT_ATTRIBUTEFORMDEFAULT[];
    static const XMLCh fgATT_BASE[];
    static const XMLCh fgATT_ITEMTYPE[];
    static const XMLCh fgATT_MEMBERTYPES[];
    static const XMLCh fgATT_BLOCK[];
    static const XMLCh fgATT_BLOCKDEFAULT[];
    static const XMLCh fgATT_DEFAULT[];
    static const XMLCh fgATT_ELEMENTFORMDEFAULT[];
    static const XMLCh fgATT_SUBSTITUTIONGROUP[];
    static const XMLCh fgATT_FINAL[];
    static const XMLCh fgATT_FINALDEFAULT[];
    static const XMLCh fgATT_FIXED[];
    static const XMLCh fgATT_FORM[];
    static const XMLCh fgATT_ID[];
    static const XMLCh fgATT_MAXOCCURS[];
    static const XMLCh fgATT_MINOCCURS[];
    static const XMLCh fgATT_NAME[];
    static const XMLCh fgATT_NAMESPACE[];
    static const XMLCh fgATT_NILL[];
    static const XMLCh fgATT_NILLABLE[];
    static const XMLCh fgATT_PROCESSCONTENTS[];
    static const XMLCh fgATT_REF[];
    static const XMLCh fgATT_REFER[];
    static const XMLCh fgATT_SCHEMALOCATION[];
    static const XMLCh fgATT_SOURCE[];
    static const XMLCh fgATT_SYSTEM[];
    static const XMLCh fgATT_PUBLIC[];
    static const XMLCh fgATT_TARGETNAMESPACE[];
    static const XMLCh fgATT_TYPE[];
    static const XMLCh fgATT_USE[];
    static const XMLCh fgATT_VALUE[];
    static const XMLCh fgATT_MIXED[];
    static const XMLCh fgATT_VERSION[];
    static const XMLCh fgATT_XPATH[];
    static const XMLCh fgATTVAL_TWOPOUNDANY[];
    static const XMLCh fgATTVAL_TWOPOUNDLOCAL[];
    static const XMLCh fgATTVAL_TWOPOUNDOTHER[];
    static const XMLCh fgATTVAL_TWOPOUNDTRAGETNAMESPACE[];
    static const XMLCh fgATTVAL_POUNDALL[];
    static const XMLCh fgATTVAL_BASE64[];
    static const XMLCh fgATTVAL_BOOLEAN[];
    static const XMLCh fgATTVAL_DEFAULT[];
    static const XMLCh fgATTVAL_ELEMENTONLY[];
    static const XMLCh fgATTVAL_EMPTY[];
    static const XMLCh fgATTVAL_EXTENSION[];
    static const XMLCh fgATTVAL_FALSE[];
    static const XMLCh fgATTVAL_FIXED[];
    static const XMLCh fgATTVAL_HEX[];
    static const XMLCh fgATTVAL_ID[];
    static const XMLCh fgATTVAL_LAX[];
    static const XMLCh fgATTVAL_MAXLENGTH[];
    static const XMLCh fgATTVAL_MINLENGTH[];
    static const XMLCh fgATTVAL_MIXED[];
    static const XMLCh fgATTVAL_NCNAME[];
    static const XMLCh fgATTVAL_OPTIONAL[];
    static const XMLCh fgATTVAL_PROHIBITED[];
    static const XMLCh fgATTVAL_QNAME[];
    static const XMLCh fgATTVAL_QUALIFIED[];
    static const XMLCh fgATTVAL_REQUIRED[];
    static const XMLCh fgATTVAL_RESTRICTION[];
    static const XMLCh fgATTVAL_SKIP[];
    static const XMLCh fgATTVAL_STRICT[];
    static const XMLCh fgATTVAL_STRING[];
    static const XMLCh fgATTVAL_TEXTONLY[];
    static const XMLCh fgATTVAL_TIMEDURATION[];
    static const XMLCh fgATTVAL_TRUE[];
    static const XMLCh fgATTVAL_UNQUALIFIED[];
    static const XMLCh fgATTVAL_URI[];
    static const XMLCh fgATTVAL_URIREFERENCE[];
    static const XMLCh fgATTVAL_SUBSTITUTIONGROUP[];
    static const XMLCh fgATTVAL_SUBSTITUTION[];
    static const XMLCh fgATTVAL_ANYTYPE[];
    static const XMLCh fgWS_PRESERVE[];
    static const XMLCh fgWS_COLLAPSE[];
    static const XMLCh fgWS_REPLACE[];
    static const XMLCh fgDT_STRING[];
    static const XMLCh fgDT_TOKEN[];
    static const XMLCh fgDT_LANGUAGE[];
    static const XMLCh fgDT_NAME[];
    static const XMLCh fgDT_NCNAME[];
    static const XMLCh fgDT_INTEGER[];
    static const XMLCh fgDT_DECIMAL[];
    static const XMLCh fgDT_BOOLEAN[];
    static const XMLCh fgDT_NONPOSITIVEINTEGER[];
    static const XMLCh fgDT_NEGATIVEINTEGER[];
    static const XMLCh fgDT_LONG[];
    static const XMLCh fgDT_INT[];
    static const XMLCh fgDT_SHORT[];
    static const XMLCh fgDT_BYTE[];
	static const XMLCh fgDT_NONNEGATIVEINTEGER[];
    static const XMLCh fgDT_ULONG[];
    static const XMLCh fgDT_UINT[];
	static const XMLCh fgDT_USHORT[];
	static const XMLCh fgDT_UBYTE[];
    static const XMLCh fgDT_POSITIVEINTEGER[];
//datetime
    static const XMLCh fgDT_DATETIME[];
    static const XMLCh fgDT_DATE[];
	static const XMLCh fgDT_TIME[];
    static const XMLCh fgDT_DURATION[];
    static const XMLCh fgDT_DAY[];
    static const XMLCh fgDT_MONTH[];
    static const XMLCh fgDT_MONTHDAY[];
    static const XMLCh fgDT_YEAR[];
    static const XMLCh fgDT_YEARMONTH[];

    static const XMLCh fgDT_BASE64BINARY[];
    static const XMLCh fgDT_HEXBINARY[];
    static const XMLCh fgDT_FLOAT[];
    static const XMLCh fgDT_DOUBLE[];
    static const XMLCh fgDT_URIREFERENCE[];
    static const XMLCh fgDT_ANYURI[];
    static const XMLCh fgDT_QNAME[];
    static const XMLCh fgDT_NORMALIZEDSTRING[];
    static const XMLCh fgDT_ANYSIMPLETYPE[];
    static const XMLCh fgRegEx_XOption[];
    static const XMLCh fgRedefIdentifier[];
    static const int   fgINT_MIN_VALUE;
    static const int   fgINT_MAX_VALUE;

    enum {
        XSD_EMPTYSET = 0,
        XSD_SUBSTITUTION = 1,
        XSD_EXTENSION = 2,
        XSD_RESTRICTION = 4,
        XSD_LIST = 8,
        XSD_UNION = 16,
        XSD_ENUMERATION = 32
    };

    // group orders
    enum {
        XSD_CHOICE = 0,
        XSD_SEQUENCE= 1,
        XSD_ALL = 2
    };

    enum {
        XSD_UNBOUNDED = -1,
        XSD_NILLABLE = 1,
        XSD_ABSTRACT = 2,
        XSD_FIXED = 4
    };

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    SchemaSymbols();
};

XERCES_CPP_NAMESPACE_END

#endif

/**
  * End of file SchemaSymbols.hpp
  */

