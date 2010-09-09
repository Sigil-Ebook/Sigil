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
 * $Id: DatatypeValidator.cpp 555320 2007-07-11 16:05:13Z amassari $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/datatype/DatatypeValidator.hpp>
#include <xercesc/framework/MemoryManager.hpp>

//since we need to dynamically created each and every derivatives 
//during deserialization by XSerializeEngine>>Derivative, we got
//to include all hpp

#include <xercesc/validators/datatype/StringDatatypeValidator.hpp>
#include <xercesc/validators/datatype/AnyURIDatatypeValidator.hpp>
#include <xercesc/validators/datatype/QNameDatatypeValidator.hpp>
#include <xercesc/validators/datatype/NameDatatypeValidator.hpp>
#include <xercesc/validators/datatype/NCNameDatatypeValidator.hpp>
#include <xercesc/validators/datatype/BooleanDatatypeValidator.hpp>
#include <xercesc/validators/datatype/FloatDatatypeValidator.hpp>
#include <xercesc/validators/datatype/DoubleDatatypeValidator.hpp>
#include <xercesc/validators/datatype/DecimalDatatypeValidator.hpp>
#include <xercesc/validators/datatype/HexBinaryDatatypeValidator.hpp>
#include <xercesc/validators/datatype/Base64BinaryDatatypeValidator.hpp>
#include <xercesc/validators/datatype/DurationDatatypeValidator.hpp>
#include <xercesc/validators/datatype/DateTimeDatatypeValidator.hpp>
#include <xercesc/validators/datatype/DateDatatypeValidator.hpp>
#include <xercesc/validators/datatype/TimeDatatypeValidator.hpp>
#include <xercesc/validators/datatype/MonthDayDatatypeValidator.hpp>
#include <xercesc/validators/datatype/YearMonthDatatypeValidator.hpp>
#include <xercesc/validators/datatype/YearDatatypeValidator.hpp>
#include <xercesc/validators/datatype/MonthDatatypeValidator.hpp>
#include <xercesc/validators/datatype/DayDatatypeValidator.hpp>
#include <xercesc/validators/datatype/IDDatatypeValidator.hpp>
#include <xercesc/validators/datatype/IDREFDatatypeValidator.hpp>
#include <xercesc/validators/datatype/ENTITYDatatypeValidator.hpp>
#include <xercesc/validators/datatype/NOTATIONDatatypeValidator.hpp>
#include <xercesc/validators/datatype/ListDatatypeValidator.hpp>
#include <xercesc/validators/datatype/UnionDatatypeValidator.hpp>
#include <xercesc/validators/datatype/AnySimpleTypeDatatypeValidator.hpp>

#include <xercesc/internal/XTemplateSerializer.hpp>

XERCES_CPP_NAMESPACE_BEGIN

static const int DV_BUILTIN = -1;
static const int DV_NORMAL  = -2;
static const int DV_ZERO    = -3;

static const int TYPENAME_ZERO    = -1;
static const int TYPENAME_S4S     = -2;
static const int TYPENAME_NORMAL  = -3;

// ---------------------------------------------------------------------------
//  DatatypeValidator: Constructors and Destructor
// ---------------------------------------------------------------------------
DatatypeValidator::DatatypeValidator(DatatypeValidator* const baseValidator,
                                     RefHashTableOf<KVStringPair>* const facets,
                                     const int finalSet,
                                     const ValidatorType type,
                                     MemoryManager* const manager)
    : fAnonymous(false)
    , fFinite(false)
    , fBounded(false)
    , fNumeric(false)
    , fWhiteSpace(COLLAPSE)
    , fFinalSet(finalSet)
    , fFacetsDefined(0)
    , fFixed(0)
    , fType(type)
    , fOrdered(XSSimpleTypeDefinition::ORDERED_FALSE)
    , fBaseValidator(baseValidator)
    , fFacets(facets)
    , fPattern(0)
    , fRegex(0)
    , fTypeName(0)
    , fTypeLocalName(XMLUni::fgZeroLenString)
    , fTypeUri(XMLUni::fgZeroLenString)
    , fMemoryManager(manager)
{
}

DatatypeValidator::~DatatypeValidator()
{
	cleanUp();
}

const XMLCh* DatatypeValidator::getWSstring(const short theType) const
{
    switch (theType)
    {
    case PRESERVE:
         return SchemaSymbols::fgWS_PRESERVE;
    case REPLACE:
         return SchemaSymbols::fgWS_REPLACE;
    case COLLAPSE:
         return SchemaSymbols::fgWS_COLLAPSE;
    default:
         return SchemaSymbols::fgWS_PRESERVE;
    }

}

void DatatypeValidator::setTypeName(const XMLCh* const name, const XMLCh* const uri)
{
    if (fTypeName) {

        fMemoryManager->deallocate(fTypeName);
        fTypeName = 0;
    }

    if (name || uri) {

        XMLSize_t nameLen = XMLString::stringLen(name);
        XMLSize_t uriLen = XMLString::stringLen(uri);

        fTypeName = (XMLCh*) fMemoryManager->allocate
        (
            (nameLen + uriLen + 2)*sizeof(XMLCh)
        );
        fTypeUri = fTypeName;
        fTypeLocalName = &fTypeName[uriLen+1];

        if (uri)
			XMLString::moveChars(fTypeName, uri, uriLen+1);
        else
			fTypeName[0] = chNull;

        if (name)
            XMLString::moveChars(&fTypeName[uriLen+1], name, nameLen+1);
        else
            fTypeName[uriLen+1] = chNull;
    }
    else
    {
        fTypeUri = fTypeLocalName = XMLUni::fgZeroLenString;
    }
}

void DatatypeValidator::setTypeName(const XMLCh* const typeName)
{
    if (fTypeName)
    {
        fMemoryManager->deallocate(fTypeName);
        fTypeName = 0;
    }

    if (typeName)
    {
        XMLSize_t nameLen = XMLString::stringLen(typeName);
        int commaOffset = XMLString::indexOf(typeName, chComma);

        fTypeName = (XMLCh*) fMemoryManager->allocate
        (
            (nameLen + 1) * sizeof(XMLCh)
        );
	    XMLString::moveChars(fTypeName, typeName, nameLen+1);

        if ( commaOffset == -1) {
            fTypeUri = SchemaSymbols::fgURI_SCHEMAFORSCHEMA;
            fTypeLocalName = fTypeName;
        }
        else {
            fTypeUri = fTypeName;
            fTypeLocalName = &fTypeName[commaOffset+1];
            fTypeName[commaOffset] = chNull;
        }
    }
    else
    {
        fTypeUri = fTypeLocalName = XMLUni::fgZeroLenString;
    }
}

// ---------------------------------------------------------------------------
//  DatatypeValidator: CleanUp methods
// ---------------------------------------------------------------------------
void DatatypeValidator::cleanUp() {

	delete fFacets;
    delete fRegex;
    if (fPattern)
        fMemoryManager->deallocate(fPattern);//delete [] fPattern;
    if (fTypeName)
        fMemoryManager->deallocate(fTypeName);
}

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_NOCREATE(DatatypeValidator)

void DatatypeValidator::serialize(XSerializeEngine& serEng)
{

    if (serEng.isStoring())
    {
        serEng<<fAnonymous;
        serEng<<fFinite;
        serEng<<fBounded;
        serEng<<fNumeric;

        serEng<<fWhiteSpace;
        serEng<<fFinalSet;
        serEng<<fFacetsDefined;
        serEng<<fFixed;

        serEng<<(int)fType;
        serEng<<(int)fOrdered;

        storeDV(serEng, fBaseValidator);

        /***
         *  Serialize RefHashTableOf<KVStringPair>
         ***/
        XTemplateSerializer::storeObject(fFacets, serEng);

        serEng.writeString(fPattern);

        if (fTypeUri==XMLUni::fgZeroLenString)
        {
            serEng<<TYPENAME_ZERO;
        }
        else if (fTypeUri == SchemaSymbols::fgURI_SCHEMAFORSCHEMA)
        {
            serEng<<TYPENAME_S4S;
            serEng.writeString(fTypeLocalName);
        }
        else
        {        
            serEng<<TYPENAME_NORMAL;
            serEng.writeString(fTypeLocalName);
            serEng.writeString(fTypeUri);
        }

        /***
         * don't serialize 
         *       fRegex
         ***/    
    }
    else
    {
        serEng>>fAnonymous;
        serEng>>fFinite;
        serEng>>fBounded;
        serEng>>fNumeric;

        serEng>>fWhiteSpace;
        serEng>>fFinalSet;
        serEng>>fFacetsDefined;
        serEng>>fFixed;

        int type;
        serEng>>type;
        fType=(ValidatorType)type;

        serEng>>type;
        fOrdered = (XSSimpleTypeDefinition::ORDERING)type;


        fBaseValidator = loadDV(serEng);

        /***
         *
         *  Deserialize RefHashTableOf<KVStringPair>
         *
         ***/
        XTemplateSerializer::loadObject(&fFacets, 29, true, serEng);
        serEng.readString(fPattern);       

        /***
         *   Recreate through setTypeName()
         *       fTypeName
         ***/

        int flag;
        serEng>>flag;

        if ( TYPENAME_ZERO == flag )
        {
            setTypeName(0);
        }
        else if ( TYPENAME_S4S == flag )
        {
            XMLCh* typeLocalName;
            serEng.readString(typeLocalName);
            ArrayJanitor<XMLCh> janName(typeLocalName, fMemoryManager);

            setTypeName(typeLocalName);
        }
        else // TYPENAME_NORMAL
        {
            XMLCh* typeLocalName;
            serEng.readString(typeLocalName);
            ArrayJanitor<XMLCh> janName(typeLocalName, fMemoryManager);

            XMLCh* typeUri;
            serEng.readString(typeUri);
            ArrayJanitor<XMLCh> janUri(typeUri, fMemoryManager);

            setTypeName(typeLocalName, typeUri);
        }

        /***
         * don't serialize fRegex
         ***/
        fRegex = new (fMemoryManager) RegularExpression(fPattern, SchemaSymbols::fgRegEx_XOption, fMemoryManager);

    }

}

/***
 *
 *  When deserialized, we need to know, exactly what
 *  validator was serialized here.
 *
 *  Design Issue:
 *    
 *    This extra type information is only necessary when
 *  we need to create and deserialize an DatatypeValidator 
 *  derivative by operator >>, but not by object.serialize().
 *  Therefore it is appropriate to save this type info by
 *  hosting object rather than by derivative.serialize().
 *
 *
 ***/
void DatatypeValidator::storeDV(XSerializeEngine&        serEng
                              , DatatypeValidator* const dv)
{
    if (dv)
    {
        //builtIndv
        if (dv == DatatypeValidatorFactory::getBuiltInRegistry()->get(dv->getTypeLocalName()))
        {
            serEng<<DV_BUILTIN;
            serEng.writeString(dv->getTypeLocalName());
        }
        else
        {
            serEng<<DV_NORMAL;
            serEng<<(int) dv->getType();
            serEng<<dv;
        }
    }
    else
    {
        serEng<<DV_ZERO;
    }

}

DatatypeValidator* DatatypeValidator::loadDV(XSerializeEngine& serEng)
{

    int flag;
    serEng>>flag;

    if (DV_BUILTIN == flag)
    {
        XMLCh* dvName;
        serEng.readString(dvName);
        ArrayJanitor<XMLCh> janName(dvName, serEng.getMemoryManager());

        return DatatypeValidatorFactory::getBuiltInRegistry()->get(dvName);
    }
    else if (DV_ZERO == flag)
    {
        return 0;
    }

    int type;
    serEng>>type;

    switch((ValidatorType)type)
    {
    case String: 
        StringDatatypeValidator* stringdv;
        serEng>>stringdv;
        return stringdv;        
    case AnyURI:
        AnyURIDatatypeValidator* anyuridv;
        serEng>>anyuridv;
        return anyuridv;        
    case QName: 
        QNameDatatypeValidator* qnamedv;
        serEng>>qnamedv;
        return qnamedv;        
    case Name: 
        NameDatatypeValidator* namedv;
        serEng>>namedv;
        return namedv;        
    case NCName:  
        NCNameDatatypeValidator* ncnamedv;
        serEng>>ncnamedv;
        return ncnamedv;        
    case Boolean: 
        BooleanDatatypeValidator* booleandv;
        serEng>>booleandv;
        return booleandv;        
    case Float: 
        FloatDatatypeValidator* floatdv;
        serEng>>floatdv;
        return floatdv;        
    case Double: 
        DoubleDatatypeValidator* doubledv;
        serEng>>doubledv;
        return doubledv;        
    case Decimal: 
        DecimalDatatypeValidator* decimaldv;
        serEng>>decimaldv;
        return decimaldv;        
    case HexBinary:  
        HexBinaryDatatypeValidator* hexbinarydv;
        serEng>>hexbinarydv;
        return hexbinarydv;       
    case Base64Binary: 
        Base64BinaryDatatypeValidator* base64binarydv;
        serEng>>base64binarydv;
        return base64binarydv;      
    case Duration:     
        DurationDatatypeValidator* durationdv;
        serEng>>durationdv;
        return durationdv;
    case DateTime:       
        DateTimeDatatypeValidator* datetimedv;
        serEng>>datetimedv;
        return datetimedv; 
    case Date:          
        DateDatatypeValidator* datedv;
        serEng>>datedv;
        return datedv;
    case Time:         
        TimeDatatypeValidator* timedv;
        serEng>>timedv;
        return timedv;
    case MonthDay:      
        MonthDayDatatypeValidator* monthdaydv;
        serEng>>monthdaydv;
        return monthdaydv;
    case YearMonth:     
        YearMonthDatatypeValidator* yearmonthdv;
        serEng>>yearmonthdv;
        return yearmonthdv;
    case Year:          
        YearDatatypeValidator* yeardv;
        serEng>>yeardv;
        return yeardv;
    case Month:        
        MonthDatatypeValidator* monthdv;
        serEng>>monthdv;
        return monthdv;
    case Day:           
        DayDatatypeValidator* daydv;
        serEng>>daydv;
        return daydv;
    case ID:           
        IDDatatypeValidator* iddv;
        serEng>>iddv;
        return iddv;
    case IDREF:         
        IDREFDatatypeValidator* idrefdv;
        serEng>>idrefdv;
        return idrefdv;
    case ENTITY:       
        ENTITYDatatypeValidator* entitydv;
        serEng>>entitydv;
        return entitydv;
    case NOTATION:     
        NOTATIONDatatypeValidator* notationdv;
        serEng>>notationdv;
        return notationdv;
    case List:          
        ListDatatypeValidator* listdv;
        serEng>>listdv;
        return listdv;
    case Union:         
        UnionDatatypeValidator* uniondv;
        serEng>>uniondv;
        return uniondv;
    case AnySimpleType:  
        AnySimpleTypeDatatypeValidator* anysimpletypedv;
        serEng>>anysimpletypedv;
        return anysimpletypedv;
    case UnKnown:
        return 0;
    default: //we treat this same as UnKnown
        return 0;
    }

}

/**
 * Canonical Representation
 *
 */
const XMLCh* DatatypeValidator::getCanonicalRepresentation(const XMLCh*         const rawData
                                                          ,      MemoryManager* const memMgr
                                                          ,      bool                 toValidate) const
{
    MemoryManager* toUse = memMgr? memMgr : fMemoryManager;

    if (toValidate)
    {
        DatatypeValidator *temp = (DatatypeValidator*) this;

        try
        {
            temp->validate(rawData, 0, toUse);    
        }
        catch (...)
        {
            return 0;
        }
    }

    return XMLString::replicate(rawData, toUse);
}


XERCES_CPP_NAMESPACE_END

/**
  * End of file DatatypeValidator.cpp
  */

