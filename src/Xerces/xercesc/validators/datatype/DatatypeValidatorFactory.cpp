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
 * $Id: DatatypeValidatorFactory.cpp 932887 2010-04-11 13:04:59Z borisk $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/datatype/DatatypeValidatorFactory.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/Janitor.hpp>
#include <xercesc/validators/datatype/StringDatatypeValidator.hpp>
#include <xercesc/validators/datatype/BooleanDatatypeValidator.hpp>
#include <xercesc/validators/datatype/DecimalDatatypeValidator.hpp>
#include <xercesc/validators/datatype/HexBinaryDatatypeValidator.hpp>
#include <xercesc/validators/datatype/Base64BinaryDatatypeValidator.hpp>
#include <xercesc/validators/datatype/IDDatatypeValidator.hpp>
#include <xercesc/validators/datatype/IDREFDatatypeValidator.hpp>
#include <xercesc/validators/datatype/NOTATIONDatatypeValidator.hpp>
#include <xercesc/validators/datatype/ENTITYDatatypeValidator.hpp>
#include <xercesc/validators/datatype/QNameDatatypeValidator.hpp>
#include <xercesc/validators/datatype/NameDatatypeValidator.hpp>
#include <xercesc/validators/datatype/NCNameDatatypeValidator.hpp>
#include <xercesc/validators/datatype/ListDatatypeValidator.hpp>
#include <xercesc/validators/datatype/UnionDatatypeValidator.hpp>
#include <xercesc/validators/datatype/DoubleDatatypeValidator.hpp>
#include <xercesc/validators/datatype/FloatDatatypeValidator.hpp>
#include <xercesc/validators/datatype/AnyURIDatatypeValidator.hpp>
#include <xercesc/validators/datatype/AnySimpleTypeDatatypeValidator.hpp>
#include <xercesc/validators/datatype/DateTimeDatatypeValidator.hpp>
#include <xercesc/validators/datatype/DateDatatypeValidator.hpp>
#include <xercesc/validators/datatype/TimeDatatypeValidator.hpp>
#include <xercesc/validators/datatype/DayDatatypeValidator.hpp>
#include <xercesc/validators/datatype/MonthDatatypeValidator.hpp>
#include <xercesc/validators/datatype/MonthDayDatatypeValidator.hpp>
#include <xercesc/validators/datatype/YearDatatypeValidator.hpp>
#include <xercesc/validators/datatype/YearMonthDatatypeValidator.hpp>
#include <xercesc/validators/datatype/DurationDatatypeValidator.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLInitializer.hpp>

#include <xercesc/internal/XTemplateSerializer.hpp>

XERCES_CPP_NAMESPACE_BEGIN


// ---------------------------------------------------------------------------
//  DatatypeValidatorFactory: Local const data
// ---------------------------------------------------------------------------
const XMLCh fgTokPattern[] =
{
    chBackSlash, chLatin_c, chPlus, chNull
};

//E2-43
//[+\-]?[0-9]+
const XMLCh fgIntegerPattern[] =
{
    chOpenSquare, chPlus, chBackSlash, chDash, chCloseSquare, chQuestion,
    chOpenSquare, chDigit_0, chDash, chDigit_9, chCloseSquare, chPlus, chNull
};

//"\\i\\c*"
const XMLCh fgNamePattern[] =
{
    chBackSlash, chLatin_i, chBackSlash, chLatin_c, chAsterisk, chNull
};

//"[\\i-[:]][\\c-[:]]*"
const XMLCh fgNCNamePattern[] =
{
    chOpenSquare, chBackSlash, chLatin_i, chDash, chOpenSquare, chColon, chCloseSquare,
    chCloseSquare, chOpenSquare, chBackSlash, chLatin_c, chDash, chOpenSquare,
    chColon, chCloseSquare, chCloseSquare, chAsterisk, chNull
};



const XMLCh fgP0Y[] =
{
    chLatin_P, chDigit_0, chLatin_Y, chNull
};

const XMLCh fgP1Y[] =
{
    chLatin_P, chDigit_1, chLatin_Y, chNull
};

const XMLCh fgP100Y[] =
{
    chLatin_P, chDigit_1, chDigit_0, chDigit_0, chLatin_Y, chNull
};

const XMLCh fgPT24H[] =
{
    chLatin_P, chLatin_T, chDigit_2, chDigit_4, chLatin_H, chNull
};

const XMLCh fgP1M[] =
{
    chLatin_P, chDigit_1, chLatin_M, chNull
};

// ---------------------------------------------------------------------------
//  DatatypeValidatorFactory: Static member data
// ---------------------------------------------------------------------------
RefHashTableOf<DatatypeValidator>* DatatypeValidatorFactory::fBuiltInRegistry = 0;
RefHashTableOf<XMLCanRepGroup, PtrHasher>* DatatypeValidatorFactory::fCanRepRegistry = 0;

void XMLInitializer::initializeDatatypeValidatorFactory()
{
    // @@ This is ugly. Need to make expandRegistryToFullSchemaSet
    // static.
    //
    DatatypeValidatorFactory *dvFactory = new DatatypeValidatorFactory();
    if (dvFactory) {
        dvFactory->expandRegistryToFullSchemaSet();
        delete dvFactory;
    }
}

void XMLInitializer::terminateDatatypeValidatorFactory()
{
    delete DatatypeValidatorFactory::fBuiltInRegistry;
    DatatypeValidatorFactory::fBuiltInRegistry = 0;

    delete DatatypeValidatorFactory::fCanRepRegistry;
    DatatypeValidatorFactory::fCanRepRegistry = 0;
}

// ---------------------------------------------------------------------------
//  DatatypeValidatorFactory: Constructors and Destructor
// ---------------------------------------------------------------------------
DatatypeValidatorFactory::DatatypeValidatorFactory(MemoryManager* const manager)
    : fUserDefinedRegistry(0)
    , fMemoryManager(manager)
{
}

DatatypeValidatorFactory::~DatatypeValidatorFactory()
{
    cleanUp();
}


// ---------------------------------------------------------------------------
//  DatatypeValidatorFactory: Reset methods
// ---------------------------------------------------------------------------
void DatatypeValidatorFactory::resetRegistry() {

    if (fUserDefinedRegistry != 0) {
        fUserDefinedRegistry->removeAll();
    }
}

// ---------------------------------------------------------------------------
//  DatatypeValidatorFactory: Registry initialization methods
// ---------------------------------------------------------------------------
void DatatypeValidatorFactory::expandRegistryToFullSchemaSet()
{
    //Initialize common Schema/DTD Datatype validator set
    fBuiltInRegistry = new RefHashTableOf<DatatypeValidator>(29);

    DatatypeValidator *dv = new StringDatatypeValidator();
    dv->setTypeName(SchemaSymbols::fgDT_STRING, SchemaSymbols::fgURI_SCHEMAFORSCHEMA);
    fBuiltInRegistry->put((void*) SchemaSymbols::fgDT_STRING, dv);

    dv = new NOTATIONDatatypeValidator();
    dv->setTypeName(XMLUni::fgNotationString, SchemaSymbols::fgURI_SCHEMAFORSCHEMA);
    fBuiltInRegistry->put((void*) XMLUni::fgNotationString, dv);

    dv = new AnySimpleTypeDatatypeValidator();
    dv->setTypeName(SchemaSymbols::fgDT_ANYSIMPLETYPE, SchemaSymbols::fgURI_SCHEMAFORSCHEMA);
    fBuiltInRegistry->put((void*) SchemaSymbols::fgDT_ANYSIMPLETYPE, dv);

    dv = new BooleanDatatypeValidator();
    dv->setTypeName(SchemaSymbols::fgDT_BOOLEAN, SchemaSymbols::fgURI_SCHEMAFORSCHEMA);
    fBuiltInRegistry->put((void*) SchemaSymbols::fgDT_BOOLEAN, dv);

    dv = new DecimalDatatypeValidator();
    dv->setTypeName(SchemaSymbols::fgDT_DECIMAL, SchemaSymbols::fgURI_SCHEMAFORSCHEMA);
    fBuiltInRegistry->put((void*) SchemaSymbols::fgDT_DECIMAL, dv);

    dv = new HexBinaryDatatypeValidator();
    dv->setTypeName(SchemaSymbols::fgDT_HEXBINARY, SchemaSymbols::fgURI_SCHEMAFORSCHEMA);
    fBuiltInRegistry->put((void*) SchemaSymbols::fgDT_HEXBINARY, dv);

    dv = new Base64BinaryDatatypeValidator();
    dv->setTypeName(SchemaSymbols::fgDT_BASE64BINARY, SchemaSymbols::fgURI_SCHEMAFORSCHEMA);
    fBuiltInRegistry->put((void*) SchemaSymbols::fgDT_BASE64BINARY, dv);

    dv = new DoubleDatatypeValidator();
    dv->setTypeName(SchemaSymbols::fgDT_DOUBLE, SchemaSymbols::fgURI_SCHEMAFORSCHEMA);
    fBuiltInRegistry->put((void*) SchemaSymbols::fgDT_DOUBLE, dv);

    dv = new FloatDatatypeValidator();
    dv->setTypeName(SchemaSymbols::fgDT_FLOAT, SchemaSymbols::fgURI_SCHEMAFORSCHEMA);
    fBuiltInRegistry->put((void*) SchemaSymbols::fgDT_FLOAT, dv);

    dv = new AnyURIDatatypeValidator();
    dv->setTypeName(SchemaSymbols::fgDT_ANYURI, SchemaSymbols::fgURI_SCHEMAFORSCHEMA);
    fBuiltInRegistry->put((void*) SchemaSymbols::fgDT_ANYURI, dv);

    dv = new QNameDatatypeValidator();
    dv->setTypeName(SchemaSymbols::fgDT_QNAME, SchemaSymbols::fgURI_SCHEMAFORSCHEMA);
    fBuiltInRegistry->put((void*) SchemaSymbols::fgDT_QNAME, dv);

    dv = new DateTimeDatatypeValidator();
    dv->setTypeName(SchemaSymbols::fgDT_DATETIME, SchemaSymbols::fgURI_SCHEMAFORSCHEMA);
    fBuiltInRegistry->put((void*) SchemaSymbols::fgDT_DATETIME, dv);

    dv = new DateDatatypeValidator();
    dv->setTypeName(SchemaSymbols::fgDT_DATE, SchemaSymbols::fgURI_SCHEMAFORSCHEMA);
    fBuiltInRegistry->put((void*) SchemaSymbols::fgDT_DATE, dv);

    dv = new TimeDatatypeValidator();
    dv->setTypeName(SchemaSymbols::fgDT_TIME, SchemaSymbols::fgURI_SCHEMAFORSCHEMA);
    fBuiltInRegistry->put((void*) SchemaSymbols::fgDT_TIME, dv);

    dv = new DayDatatypeValidator();
    dv->setTypeName(SchemaSymbols::fgDT_DAY, SchemaSymbols::fgURI_SCHEMAFORSCHEMA);
    fBuiltInRegistry->put((void*) SchemaSymbols::fgDT_DAY, dv);

    dv = new MonthDatatypeValidator();
    dv->setTypeName(SchemaSymbols::fgDT_MONTH, SchemaSymbols::fgURI_SCHEMAFORSCHEMA);
    fBuiltInRegistry->put((void*) SchemaSymbols::fgDT_MONTH, dv);

    dv = new MonthDayDatatypeValidator();
    dv->setTypeName(SchemaSymbols::fgDT_MONTHDAY, SchemaSymbols::fgURI_SCHEMAFORSCHEMA);
    fBuiltInRegistry->put((void*) SchemaSymbols::fgDT_MONTHDAY, dv);

    dv = new YearDatatypeValidator();
    dv->setTypeName(SchemaSymbols::fgDT_YEAR, SchemaSymbols::fgURI_SCHEMAFORSCHEMA);
    fBuiltInRegistry->put((void*) SchemaSymbols::fgDT_YEAR, dv);

    dv = new YearMonthDatatypeValidator();
    dv->setTypeName(SchemaSymbols::fgDT_YEARMONTH, SchemaSymbols::fgURI_SCHEMAFORSCHEMA);
    fBuiltInRegistry->put((void*) SchemaSymbols::fgDT_YEARMONTH, dv);

    dv = new DurationDatatypeValidator();
    dv->setTypeName(SchemaSymbols::fgDT_DURATION, SchemaSymbols::fgURI_SCHEMAFORSCHEMA);
    fBuiltInRegistry->put((void*) SchemaSymbols::fgDT_DURATION, dv);

    // REVISIT
    // We are creating a lot of Hashtables for the facets of the different
    // validators. It's better to have some kind of a memory pool and ask
    // the pool to give us a new instance of the hashtable.
    RefHashTableOf<KVStringPair>* facets = new RefHashTableOf<KVStringPair>(3);

    // Create 'normalizedString' datatype validator
    facets->put((void*) SchemaSymbols::fgELT_WHITESPACE,
                new KVStringPair(SchemaSymbols::fgELT_WHITESPACE, SchemaSymbols::fgWS_REPLACE));

    createDatatypeValidator(SchemaSymbols::fgDT_NORMALIZEDSTRING,
                            getDatatypeValidator(SchemaSymbols::fgDT_STRING),
                            facets, 0, false, 0, false);

    // Create 'token' datatype validator
    facets = new RefHashTableOf<KVStringPair>(3);
    facets->put((void*) SchemaSymbols::fgELT_WHITESPACE,
                new KVStringPair(SchemaSymbols::fgELT_WHITESPACE, SchemaSymbols::fgWS_COLLAPSE));

    createDatatypeValidator(SchemaSymbols::fgDT_TOKEN,
                            getDatatypeValidator(SchemaSymbols::fgDT_NORMALIZEDSTRING),
                            facets, 0, false, 0, false);


    dv = new NameDatatypeValidator(getDatatypeValidator(SchemaSymbols::fgDT_TOKEN), 0, 0, 0);
    dv->setTypeName(SchemaSymbols::fgDT_NAME, SchemaSymbols::fgURI_SCHEMAFORSCHEMA);
    fBuiltInRegistry->put((void*) SchemaSymbols::fgDT_NAME, dv);


    dv = new NCNameDatatypeValidator(getDatatypeValidator(SchemaSymbols::fgDT_NAME), 0, 0, 0);
    dv->setTypeName(SchemaSymbols::fgDT_NCNAME, SchemaSymbols::fgURI_SCHEMAFORSCHEMA);
    fBuiltInRegistry->put((void*) SchemaSymbols::fgDT_NCNAME, dv);

    // Create 'NMTOKEN' datatype validator
    facets = new RefHashTableOf<KVStringPair>(3);

    facets->put((void*) SchemaSymbols::fgELT_PATTERN ,
                new KVStringPair(SchemaSymbols::fgELT_PATTERN,fgTokPattern));
    facets->put((void*) SchemaSymbols::fgELT_WHITESPACE,
                new KVStringPair(SchemaSymbols::fgELT_WHITESPACE, SchemaSymbols::fgWS_COLLAPSE));

    createDatatypeValidator(XMLUni::fgNmTokenString,
                            getDatatypeValidator(SchemaSymbols::fgDT_TOKEN),facets, 0, false, 0, false);

    // Create 'NMTOKENS' datatype validator
    facets = new RefHashTableOf<KVStringPair>(2);
    facets->put((void*) SchemaSymbols::fgELT_MINLENGTH,
                new KVStringPair(SchemaSymbols::fgELT_MINLENGTH, XMLUni::fgValueOne));

    createDatatypeValidator(XMLUni::fgNmTokensString,
                            getDatatypeValidator(XMLUni::fgNmTokenString), facets, 0, true, 0, false);

    // Create 'language' datatype validator
    facets = new RefHashTableOf<KVStringPair>(3);

    facets->put((void*) SchemaSymbols::fgELT_PATTERN,
                new KVStringPair(SchemaSymbols::fgELT_PATTERN, XMLUni::fgLangPattern));

    createDatatypeValidator(SchemaSymbols::fgDT_LANGUAGE,
                            getDatatypeValidator(SchemaSymbols::fgDT_TOKEN),
                            facets, 0, false, 0, false);

    // Create 'integer' datatype validator
    facets = new RefHashTableOf<KVStringPair>(3);

    facets->put((void*) SchemaSymbols::fgELT_FRACTIONDIGITS,
                new KVStringPair(SchemaSymbols::fgELT_FRACTIONDIGITS, XMLUni::fgValueZero));

    facets->put((void*) SchemaSymbols::fgELT_PATTERN,
                new KVStringPair(SchemaSymbols::fgELT_PATTERN, fgIntegerPattern));

    createDatatypeValidator(SchemaSymbols::fgDT_INTEGER,
                            getDatatypeValidator(SchemaSymbols::fgDT_DECIMAL),
                            facets, 0, false, 0, false);

    // Create 'nonPositiveInteger' datatype validator
    facets = new RefHashTableOf<KVStringPair>(2);

    facets->put((void*) SchemaSymbols::fgELT_MAXINCLUSIVE,
                new KVStringPair(SchemaSymbols::fgELT_MAXINCLUSIVE, XMLUni::fgValueZero));

    createDatatypeValidator(SchemaSymbols::fgDT_NONPOSITIVEINTEGER,
                            getDatatypeValidator(SchemaSymbols::fgDT_INTEGER),
                            facets, 0, false, 0, false);

    // Create 'negativeInteger' datatype validator
    facets = new RefHashTableOf<KVStringPair>(2);

    facets->put((void*) SchemaSymbols::fgELT_MAXINCLUSIVE,
                new KVStringPair(SchemaSymbols::fgELT_MAXINCLUSIVE, XMLUni::fgNegOne));

    createDatatypeValidator(SchemaSymbols::fgDT_NEGATIVEINTEGER,
                            getDatatypeValidator(SchemaSymbols::fgDT_NONPOSITIVEINTEGER),
                            facets, 0, false, 0, false);

    // Create 'long' datatype validator
    facets = new RefHashTableOf<KVStringPair>(2);

    facets->put((void*) SchemaSymbols::fgELT_MAXINCLUSIVE,
                new KVStringPair(SchemaSymbols::fgELT_MAXINCLUSIVE, XMLUni::fgLongMaxInc));
    facets->put((void*) SchemaSymbols::fgELT_MININCLUSIVE,
                new KVStringPair(SchemaSymbols::fgELT_MININCLUSIVE, XMLUni::fgLongMinInc));

    createDatatypeValidator(SchemaSymbols::fgDT_LONG,
                            getDatatypeValidator(SchemaSymbols::fgDT_INTEGER),
                            facets, 0, false, 0, false);

    // Create 'int' datatype validator
    facets = new RefHashTableOf<KVStringPair>(2);

    facets->put((void*) SchemaSymbols::fgELT_MAXINCLUSIVE,
                new KVStringPair(SchemaSymbols::fgELT_MAXINCLUSIVE, XMLUni::fgIntMaxInc));
    facets->put((void*) SchemaSymbols::fgELT_MININCLUSIVE,
                new KVStringPair(SchemaSymbols::fgELT_MININCLUSIVE, XMLUni::fgIntMinInc));

    createDatatypeValidator(SchemaSymbols::fgDT_INT,
                            getDatatypeValidator(SchemaSymbols::fgDT_LONG),
                            facets, 0, false, 0, false);

    // Create 'short' datatype validator
    facets = new RefHashTableOf<KVStringPair>(2);

    facets->put((void*) SchemaSymbols::fgELT_MAXINCLUSIVE,
                new KVStringPair(SchemaSymbols::fgELT_MAXINCLUSIVE, XMLUni::fgShortMaxInc));
    facets->put((void*) SchemaSymbols::fgELT_MININCLUSIVE,
                new KVStringPair(SchemaSymbols::fgELT_MININCLUSIVE, XMLUni::fgShortMinInc));

    createDatatypeValidator(SchemaSymbols::fgDT_SHORT,
                            getDatatypeValidator(SchemaSymbols::fgDT_INT),
                            facets, 0, false, 0 ,false);

    // Create 'byte' datatype validator
    facets = new RefHashTableOf<KVStringPair>(2);

    facets->put((void*) SchemaSymbols::fgELT_MAXINCLUSIVE,
                new KVStringPair(SchemaSymbols::fgELT_MAXINCLUSIVE, XMLUni::fgByteMaxInc));
    facets->put((void*) SchemaSymbols::fgELT_MININCLUSIVE,
                new KVStringPair(SchemaSymbols::fgELT_MININCLUSIVE, XMLUni::fgByteMinInc));

    createDatatypeValidator(SchemaSymbols::fgDT_BYTE,
                            getDatatypeValidator(SchemaSymbols::fgDT_SHORT),
                            facets, 0, false, 0, false);

    // Create 'nonNegativeInteger' datatype validator
    facets = new RefHashTableOf<KVStringPair>(2);

    facets->put((void*) SchemaSymbols::fgELT_MININCLUSIVE,
                new KVStringPair(SchemaSymbols::fgELT_MININCLUSIVE, XMLUni::fgValueZero));

    createDatatypeValidator(SchemaSymbols::fgDT_NONNEGATIVEINTEGER,
                            getDatatypeValidator(SchemaSymbols::fgDT_INTEGER),
                            facets, 0, false, 0, false);

    // Create 'unsignedLong' datatype validator
    facets = new RefHashTableOf<KVStringPair>(2);

    facets->put((void*) SchemaSymbols::fgELT_MAXINCLUSIVE,
                new KVStringPair(SchemaSymbols::fgELT_MAXINCLUSIVE, XMLUni::fgULongMaxInc));

    createDatatypeValidator(SchemaSymbols::fgDT_ULONG,
                            getDatatypeValidator(SchemaSymbols::fgDT_NONNEGATIVEINTEGER),
                            facets, 0, false, 0, false);

    // Create 'unsignedInt' datatype validator
    facets = new RefHashTableOf<KVStringPair>(2);

    facets->put((void*) SchemaSymbols::fgELT_MAXINCLUSIVE,
                new KVStringPair(SchemaSymbols::fgELT_MAXINCLUSIVE, XMLUni::fgUIntMaxInc));

    createDatatypeValidator(SchemaSymbols::fgDT_UINT,
                            getDatatypeValidator(SchemaSymbols::fgDT_ULONG),
                            facets, 0, false, 0, false);

    // Create 'unsignedShort' datatypeValidator
    facets = new RefHashTableOf<KVStringPair>(2);

    facets->put((void*) SchemaSymbols::fgELT_MAXINCLUSIVE,
                new KVStringPair(SchemaSymbols::fgELT_MAXINCLUSIVE, XMLUni::fgUShortMaxInc));

    createDatatypeValidator(SchemaSymbols::fgDT_USHORT,
                            getDatatypeValidator(SchemaSymbols::fgDT_UINT),
                            facets, 0, false, 0, false);

    // Create 'unsignedByte' datatype validator
    facets = new RefHashTableOf<KVStringPair>(2);

    facets->put((void*) SchemaSymbols::fgELT_MAXINCLUSIVE,
                new KVStringPair(SchemaSymbols::fgELT_MAXINCLUSIVE, XMLUni::fgUByteMaxInc));

    createDatatypeValidator(SchemaSymbols::fgDT_UBYTE,
                            getDatatypeValidator(SchemaSymbols::fgDT_USHORT),
                            facets, 0, false, 0, false);

    // Create 'positiveInteger' datatype validator
    facets = new RefHashTableOf<KVStringPair>(2);

    facets->put((void*) SchemaSymbols::fgELT_MININCLUSIVE,
                new KVStringPair(SchemaSymbols::fgELT_MININCLUSIVE, XMLUni::fgValueOne));

    createDatatypeValidator(SchemaSymbols::fgDT_POSITIVEINTEGER,
                            getDatatypeValidator(SchemaSymbols::fgDT_NONNEGATIVEINTEGER),
                            facets, 0, false, 0, false);

    // Create 'ID', 'IDREF' and 'ENTITY' datatype validator
    dv = new IDDatatypeValidator(getDatatypeValidator(SchemaSymbols::fgDT_NCNAME), 0, 0, 0);
    dv->setTypeName(XMLUni::fgIDString, SchemaSymbols::fgURI_SCHEMAFORSCHEMA);
    fBuiltInRegistry->put((void*) XMLUni::fgIDString, dv);

    dv = new IDREFDatatypeValidator(getDatatypeValidator(SchemaSymbols::fgDT_NCNAME), 0, 0, 0);
    dv->setTypeName(XMLUni::fgIDRefString, SchemaSymbols::fgURI_SCHEMAFORSCHEMA);
    fBuiltInRegistry->put((void*) XMLUni::fgIDRefString, dv);

    dv = new ENTITYDatatypeValidator(getDatatypeValidator(SchemaSymbols::fgDT_NCNAME), 0, 0, 0);
    dv->setTypeName(XMLUni::fgEntityString, SchemaSymbols::fgURI_SCHEMAFORSCHEMA);
    fBuiltInRegistry->put((void*) XMLUni::fgEntityString, dv);

    facets = new RefHashTableOf<KVStringPair>(2);
    facets->put((void*) SchemaSymbols::fgELT_MINLENGTH,
                new KVStringPair(SchemaSymbols::fgELT_MINLENGTH, XMLUni::fgValueOne));

    // Create 'IDREFS' datatype validator
    createDatatypeValidator
      (
        XMLUni::fgIDRefsString
        , getDatatypeValidator(XMLUni::fgIDRefString)
        , facets
        , 0
        , true
        , 0
        , false
      );

    facets = new RefHashTableOf<KVStringPair>(2);

    facets->put((void*) SchemaSymbols::fgELT_MINLENGTH,
                new KVStringPair(SchemaSymbols::fgELT_MINLENGTH, XMLUni::fgValueOne));

    // Create 'ENTITIES' datatype validator
    createDatatypeValidator
      (
        XMLUni::fgEntitiesString
        , getDatatypeValidator(XMLUni::fgEntityString)
        , facets
        , 0
        , true
        , 0
        , false
      );

    initCanRepRegistory();
}


/***
 *
 *   For Decimal-derivated, an instance of DecimalDatatypeValidator
 *   can be used by the primitive datatype, decimal, or any one of
 *   the derivated datatypes, such as int, long, unsighed short, just
 *   name a few, or any user-defined datatypes, which may derivate from
 *   either the primitive datatype, decimal, or from any one of those
 *   decimal derivated datatypes, or other user-defined datatypes, which
 *   in turn, indirectly derivate from decimal or decimal-derived.
 *
 *   fCanRepRegisty captures deciaml dv and its derivatives.
 *
 ***/
void DatatypeValidatorFactory::initCanRepRegistory()
{

     /***
      * key:  dv
      * data: XMLCanRepGroup
      ***/
     fCanRepRegistry  = new RefHashTableOf<XMLCanRepGroup, PtrHasher>(29, true);

     fCanRepRegistry->put((void*) getDatatypeValidator(SchemaSymbols::fgDT_DECIMAL),
                        new  XMLCanRepGroup(XMLCanRepGroup::Decimal));

     fCanRepRegistry->put((void*) getDatatypeValidator(SchemaSymbols::fgDT_INTEGER),
                        new  XMLCanRepGroup(XMLCanRepGroup::Decimal_Derived_signed));
     fCanRepRegistry->put((void*) getDatatypeValidator(SchemaSymbols::fgDT_LONG),
                        new  XMLCanRepGroup(XMLCanRepGroup::Decimal_Derived_signed));
     fCanRepRegistry->put((void*) getDatatypeValidator(SchemaSymbols::fgDT_INT),
                        new  XMLCanRepGroup(XMLCanRepGroup::Decimal_Derived_signed));
     fCanRepRegistry->put((void*) getDatatypeValidator(SchemaSymbols::fgDT_SHORT),
                        new  XMLCanRepGroup(XMLCanRepGroup::Decimal_Derived_signed));
     fCanRepRegistry->put((void*) getDatatypeValidator(SchemaSymbols::fgDT_BYTE),
                        new  XMLCanRepGroup(XMLCanRepGroup::Decimal_Derived_signed));
     fCanRepRegistry->put((void*) getDatatypeValidator(SchemaSymbols::fgDT_NONNEGATIVEINTEGER),
                        new  XMLCanRepGroup(XMLCanRepGroup::Decimal_Derived_signed));
     fCanRepRegistry->put((void*) getDatatypeValidator(SchemaSymbols::fgDT_POSITIVEINTEGER),
                        new  XMLCanRepGroup(XMLCanRepGroup::Decimal_Derived_signed));

     fCanRepRegistry->put((void*) getDatatypeValidator(SchemaSymbols::fgDT_NEGATIVEINTEGER),
                        new  XMLCanRepGroup(XMLCanRepGroup::Decimal_Derived_unsigned));
     fCanRepRegistry->put((void*) getDatatypeValidator(SchemaSymbols::fgDT_ULONG),
                        new  XMLCanRepGroup(XMLCanRepGroup::Decimal_Derived_unsigned));
     fCanRepRegistry->put((void*) getDatatypeValidator(SchemaSymbols::fgDT_UINT),
                        new  XMLCanRepGroup(XMLCanRepGroup::Decimal_Derived_unsigned));
     fCanRepRegistry->put((void*) getDatatypeValidator(SchemaSymbols::fgDT_USHORT),
                        new  XMLCanRepGroup(XMLCanRepGroup::Decimal_Derived_unsigned));
     fCanRepRegistry->put((void*) getDatatypeValidator(SchemaSymbols::fgDT_UBYTE),
                        new  XMLCanRepGroup(XMLCanRepGroup::Decimal_Derived_unsigned));

     fCanRepRegistry->put((void*) getDatatypeValidator(SchemaSymbols::fgDT_NONPOSITIVEINTEGER),
                        new  XMLCanRepGroup(XMLCanRepGroup::Decimal_Derived_npi));
}

/***
 *
 *   For any dv other than Decimaldv, return String only.
 *   Later on if support to dv other than Decimaldv arise, we may
 *   add them fCanRepRegistry during DatatypeValidatorFactory::initCanRepRegistory()
 *
 ***/
XMLCanRepGroup::CanRepGroup DatatypeValidatorFactory::getCanRepGroup(const DatatypeValidator* const dv)
{
    if (!dv)
        return XMLCanRepGroup::String;

    DatatypeValidator *curdv = (DatatypeValidator*) dv;

    while (curdv)
    {
        if (fCanRepRegistry->containsKey(curdv))
            return fCanRepRegistry->get(curdv)->getGroup();
        else
            curdv = curdv->getBaseValidator();
    }

    return XMLCanRepGroup::String;
}

DatatypeValidator* DatatypeValidatorFactory::getBuiltInBaseValidator(const DatatypeValidator* const dv)
{
    DatatypeValidator *curdv = (DatatypeValidator*)dv;

    while (curdv)
    {
        if (curdv == getBuiltInRegistry()->get(curdv->getTypeLocalName()))
            return curdv;
        else
            curdv = curdv->getBaseValidator();
     }

     return 0;
}

// ---------------------------------------------------------------------------
//  DatatypeValidatorFactory: factory methods
// ---------------------------------------------------------------------------
DatatypeValidator* DatatypeValidatorFactory::createDatatypeValidator
(
      const XMLCh* const                  typeName
	, DatatypeValidator* const            baseValidator
    , RefHashTableOf<KVStringPair>* const facets
    , RefArrayVectorOf<XMLCh>* const      enums
    , const bool                          isDerivedByList
    , const int                           finalSet
    , const bool                          isUserDefined
    , MemoryManager* const                userManager
)
{
	if (baseValidator == 0) {

        if (facets) {
            Janitor<KVStringPairHashTable> janFacets(facets);
        }

        if (enums) {
            Janitor<XMLChRefVector> janEnums(enums);
        }

        return 0;
    }

	DatatypeValidator* datatypeValidator = 0;
    MemoryManager* const manager = (isUserDefined)
        ? userManager : XMLPlatformUtils::fgMemoryManager;

    if (isDerivedByList) {
        datatypeValidator = new (manager) ListDatatypeValidator(baseValidator, facets, enums, finalSet, manager);

        // Set PSVI information for Ordered, Numeric, Bounded & Finite
        datatypeValidator->setOrdered(XSSimpleTypeDefinition::ORDERED_FALSE);
        datatypeValidator->setNumeric(false);
        if (facets &&
             ((facets->get(SchemaSymbols::fgELT_LENGTH) ||
              (facets->get(SchemaSymbols::fgELT_MINLENGTH) && facets->get(SchemaSymbols::fgELT_MAXLENGTH)))))
        {
            datatypeValidator->setBounded(true);
            datatypeValidator->setFinite(true);
        }
        else
        {
            datatypeValidator->setBounded(false);
            datatypeValidator->setFinite(false);
        }
    }
    else {

        if ((baseValidator->getType() != DatatypeValidator::String) && facets) {

            KVStringPair* value = facets->get(SchemaSymbols::fgELT_WHITESPACE);

            if (value != 0) {
                facets->removeKey(SchemaSymbols::fgELT_WHITESPACE);
            }
        }

        datatypeValidator = baseValidator->newInstance
        (
            facets
            , enums
            , finalSet
            , manager
        );

        // Set PSVI information for Ordered, Numeric, Bounded & Finite
        datatypeValidator->setOrdered(baseValidator->getOrdered());
        datatypeValidator->setNumeric(baseValidator->getNumeric());
        RefHashTableOf<KVStringPair>* baseFacets = baseValidator->getFacets();
        if (facets  &&
            ((facets->get(SchemaSymbols::fgELT_MININCLUSIVE) ||
              facets->get(SchemaSymbols::fgELT_MINEXCLUSIVE) ||
              (baseFacets && (baseFacets->get(SchemaSymbols::fgELT_MININCLUSIVE) ||
                              baseFacets->get(SchemaSymbols::fgELT_MINEXCLUSIVE))))) &&
             (facets->get(SchemaSymbols::fgELT_MAXINCLUSIVE) ||
              facets->get(SchemaSymbols::fgELT_MAXEXCLUSIVE) ||
              (baseFacets && ((baseFacets->get(SchemaSymbols::fgELT_MAXINCLUSIVE) ||
                               baseFacets->get(SchemaSymbols::fgELT_MAXEXCLUSIVE))))))
        {
            datatypeValidator->setBounded(true);
        }
        else
        {
            datatypeValidator->setBounded(false);
        }
        if (baseValidator->getFinite())
        {
            datatypeValidator->setFinite(true);
        }
        else if (!facets)
        {
            datatypeValidator->setFinite(false);
        }
        else if (facets->get(SchemaSymbols::fgELT_LENGTH) || facets->get(SchemaSymbols::fgELT_MAXLENGTH) ||
                 facets->get(SchemaSymbols::fgELT_TOTALDIGITS))
        {
            datatypeValidator->setFinite(true);
        }
        //for efficiency use this instead of rechecking...
        //else if ((facets->get(SchemaSymbols::fgELT_MININCLUSIVE) || facets->get(SchemaSymbols::fgELT_MINEXCLUSIVE)) &&
        //         (facets->get(SchemaSymbols::fgELT_MAXINCLUSIVE) || facets->get(SchemaSymbols::fgELT_MAXEXCLUSIVE)))
        else if (datatypeValidator->getBounded() ||
                 datatypeValidator->getType() == DatatypeValidator::Date      ||
                 datatypeValidator->getType() == DatatypeValidator::YearMonth ||
                 datatypeValidator->getType() == DatatypeValidator::Year      ||
                 datatypeValidator->getType() == DatatypeValidator::MonthDay  ||
                 datatypeValidator->getType() == DatatypeValidator::Day       ||
                 datatypeValidator->getType() == DatatypeValidator::Month)
        {
            if (facets->get(SchemaSymbols::fgELT_FRACTIONDIGITS))
            {
                datatypeValidator->setFinite(true);
            }
            else
            {
                datatypeValidator->setFinite(false);
            }
        }
        else
        {
            datatypeValidator->setFinite(false);
        }
    }

    if (datatypeValidator != 0) {

        if (isUserDefined) {

            if (!fUserDefinedRegistry) {
                fUserDefinedRegistry = new (userManager) RefHashTableOf<DatatypeValidator>(29, userManager);
            }

            fUserDefinedRegistry->put((void *)typeName, datatypeValidator);
        }
        else {
            fBuiltInRegistry->put((void *)typeName, datatypeValidator);
        }

        datatypeValidator->setTypeName(typeName);
    }

    return datatypeValidator;
}

static DatatypeValidator::ValidatorType getPrimitiveDV(DatatypeValidator::ValidatorType validationDV)
{
    if (validationDV == DatatypeValidator::ID    ||
        validationDV == DatatypeValidator::IDREF ||
        validationDV == DatatypeValidator::ENTITY)
    {
        return DatatypeValidator::String;
    }
    return validationDV;
}

DatatypeValidator* DatatypeValidatorFactory::createDatatypeValidator
(
      const XMLCh* const                    typeName
    , RefVectorOf<DatatypeValidator>* const validators
    , const int                             finalSet
    , const bool                            userDefined
    , MemoryManager* const                  userManager
)
{
    if (validators == 0)
        return 0;

    DatatypeValidator* datatypeValidator = 0;
    MemoryManager* const manager = (userDefined)
        ? userManager : XMLPlatformUtils::fgMemoryManager;

    datatypeValidator = new (manager) UnionDatatypeValidator(validators, finalSet, manager);

    if (datatypeValidator != 0) {

        if (userDefined) {

            if (!fUserDefinedRegistry) {
                fUserDefinedRegistry = new (userManager) RefHashTableOf<DatatypeValidator>(29, userManager);
            }

            fUserDefinedRegistry->put((void *)typeName, datatypeValidator);
        }
        else {
            fBuiltInRegistry->put((void *)typeName, datatypeValidator);
        }
        datatypeValidator->setTypeName(typeName);

        // Set PSVI information for Ordered, Numeric, Bounded & Finite
        XMLSize_t valSize = validators->size();
        if (valSize)
        {
            DatatypeValidator::ValidatorType ancestorId = getPrimitiveDV(validators->elementAt(0)->getType());

            // For a union the ORDERED {value} is partial unless one of the following:
            // 1. If every member of {member type definitions} is derived from a common ancestor other than the simple ur-type, then {value} is the same as that ancestor's ordered facet.
            // 2. If every member of {member type definitions} has a {value} of false for the ordered facet, then {value} is false.
            bool allOrderedFalse = true;
            bool commonAnc = ancestorId != DatatypeValidator::AnySimpleType;
            bool allNumeric = true;
            bool allBounded = true;
            bool allFinite  = true;

            for(XMLSize_t i = 0 ; (i < valSize) && (commonAnc || allOrderedFalse || allNumeric || allBounded || allFinite); i++)
            {
                // for the other member types, check whether the value is false
                // and whether they have the same ancestor as the first one
                if (commonAnc)
                    commonAnc = ancestorId == getPrimitiveDV(validators->elementAt(i)->getType());
                if (allOrderedFalse)
                    allOrderedFalse = validators->elementAt(i)->getOrdered() == XSSimpleTypeDefinition::ORDERED_FALSE;

                if (allNumeric && !validators->elementAt(i)->getNumeric())
                {
                    allNumeric = false;
                }
                if (allBounded && (!validators->elementAt(i)->getBounded() ||
                                   ancestorId != getPrimitiveDV(validators->elementAt(i)->getType())))
                {
                    allBounded = false;
                }
                if (allFinite && !validators->elementAt(i)->getFinite())
                {
                    allFinite = false;
                }
            }
            if (commonAnc)
            {
                datatypeValidator->setOrdered(validators->elementAt(0)->getOrdered());
            }
            else if (allOrderedFalse)
            {
                datatypeValidator->setOrdered(XSSimpleTypeDefinition::ORDERED_FALSE);
            }
            else
            {
                datatypeValidator->setOrdered(XSSimpleTypeDefinition::ORDERED_PARTIAL);
            }
            datatypeValidator->setNumeric(allNumeric);
            datatypeValidator->setBounded(allBounded);
            datatypeValidator->setFinite(allFinite);
        }
        else // size = 0
        {
            datatypeValidator->setOrdered(XSSimpleTypeDefinition::ORDERED_PARTIAL);
            datatypeValidator->setNumeric(true);
            datatypeValidator->setBounded(true);
            datatypeValidator->setFinite(true);
        }
    }
    return datatypeValidator;
}

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_TOCREATE(DatatypeValidatorFactory)

void DatatypeValidatorFactory::serialize(XSerializeEngine& serEng)
{

    // Need not to serialize static data member, fBuiltInRegistry

    if (serEng.isStoring())
    {
        /***
         * Serialize RefHashTableOf<DatatypeValidator>
         ***/
        XTemplateSerializer::storeObject(fUserDefinedRegistry, serEng);
    }
    else
    {
        /***
         * Deserialize RefHashTableOf<DatatypeValidator>
         ***/
        XTemplateSerializer::loadObject(&fUserDefinedRegistry, 29, true, serEng);
    }

}

XERCES_CPP_NAMESPACE_END

/**
  * End of file DatatypeValidatorFactory.cpp
  */
