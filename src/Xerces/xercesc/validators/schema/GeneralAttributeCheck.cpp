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

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/schema/GeneralAttributeCheck.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xercesc/validators/schema/TraverseSchema.hpp>
#include <xercesc/validators/datatype/DatatypeValidatorFactory.hpp>
#include <xercesc/dom/DOMNamedNodeMap.hpp>
#include <xercesc/framework/XMLErrorCodes.hpp>
#include <xercesc/framework/psvi/XSAnnotation.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/Janitor.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLInitializer.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Local const data
// ---------------------------------------------------------------------------
static const XMLCh fgValueZero[] =
{
    chDigit_0, chNull
};

static const XMLCh fgValueOne[] =
{
    chDigit_1, chNull
};

static const XMLCh fgUnbounded[] =
{
    chLatin_u, chLatin_n, chLatin_b, chLatin_o, chLatin_u, chLatin_n, chLatin_d,
    chLatin_e, chLatin_d, chNull
};

// ---------------------------------------------------------------------------
//  Static member data initialization
// ---------------------------------------------------------------------------
ValueHashTableOf<unsigned short>* GeneralAttributeCheck::fAttMap = 0;
ValueHashTableOf<unsigned short>* GeneralAttributeCheck::fFacetsMap = 0;
DatatypeValidator*                GeneralAttributeCheck::fNonNegIntDV = 0;
DatatypeValidator*                GeneralAttributeCheck::fBooleanDV = 0;
DatatypeValidator*                GeneralAttributeCheck::fAnyURIDV = 0;

void XMLInitializer::initializeGeneralAttributeCheck()
{
    GeneralAttributeCheck::initialize ();
}

void XMLInitializer::terminateGeneralAttributeCheck()
{
    delete GeneralAttributeCheck::fFacetsMap;
    delete GeneralAttributeCheck::fAttMap;

    GeneralAttributeCheck::fAttMap = 0;
    GeneralAttributeCheck::fFacetsMap = 0;

    GeneralAttributeCheck::fNonNegIntDV = 0;
    GeneralAttributeCheck::fBooleanDV = 0;
    GeneralAttributeCheck::fAnyURIDV = 0;
}

void GeneralAttributeCheck::initialize()
{
    // Set up validators.
    //
    DatatypeValidatorFactory dvFactory;

    fNonNegIntDV = dvFactory.getDatatypeValidator(SchemaSymbols::fgDT_NONNEGATIVEINTEGER);
    fBooleanDV = dvFactory.getDatatypeValidator(SchemaSymbols::fgDT_BOOLEAN);
    fAnyURIDV = dvFactory.getDatatypeValidator(SchemaSymbols::fgDT_ANYURI);

    // TODO - add remaining valdiators

    // Map attributes.
    //
    fAttMap = new ValueHashTableOf<unsigned short>(A_Count);

    fAttMap->put((void*)SchemaSymbols::fgATT_ABSTRACT, A_Abstract);
    fAttMap->put((void*)SchemaSymbols::fgATT_ATTRIBUTEFORMDEFAULT, A_AttributeFormDefault);
    fAttMap->put((void*)SchemaSymbols::fgATT_BASE, A_Base);
    fAttMap->put((void*)SchemaSymbols::fgATT_BLOCK, A_Block);
    fAttMap->put((void*)SchemaSymbols::fgATT_BLOCKDEFAULT, A_BlockDefault);
    fAttMap->put((void*)SchemaSymbols::fgATT_DEFAULT, A_Default);
    fAttMap->put((void*)SchemaSymbols::fgATT_ELEMENTFORMDEFAULT, A_ElementFormDefault);
    fAttMap->put((void*)SchemaSymbols::fgATT_FINAL, A_Final);
    fAttMap->put((void*)SchemaSymbols::fgATT_FINALDEFAULT, A_FinalDefault);
    fAttMap->put((void*)SchemaSymbols::fgATT_FIXED, A_Fixed);
    fAttMap->put((void*)SchemaSymbols::fgATT_FORM, A_Form);
    fAttMap->put((void*)SchemaSymbols::fgATT_ID, A_ID);
    fAttMap->put((void*)SchemaSymbols::fgATT_ITEMTYPE, A_ItemType);
    fAttMap->put((void*)SchemaSymbols::fgATT_MAXOCCURS, A_MaxOccurs);
    fAttMap->put((void*)SchemaSymbols::fgATT_MEMBERTYPES, A_MemberTypes);
    fAttMap->put((void*)SchemaSymbols::fgATT_MINOCCURS, A_MinOccurs);
    fAttMap->put((void*)SchemaSymbols::fgATT_MIXED, A_Mixed);
    fAttMap->put((void*)SchemaSymbols::fgATT_NAME, A_Name);
    fAttMap->put((void*)SchemaSymbols::fgATT_NAMESPACE, A_Namespace);
    fAttMap->put((void*)SchemaSymbols::fgATT_NILLABLE, A_Nillable);
    fAttMap->put((void*)SchemaSymbols::fgATT_PROCESSCONTENTS, A_ProcessContents);
    fAttMap->put((void*)SchemaSymbols::fgATT_PUBLIC, A_Public);
    fAttMap->put((void*)SchemaSymbols::fgATT_REF, A_Ref);
    fAttMap->put((void*)SchemaSymbols::fgATT_REFER, A_Refer);
    fAttMap->put((void*)SchemaSymbols::fgATT_SCHEMALOCATION, A_SchemaLocation);
    fAttMap->put((void*)SchemaSymbols::fgATT_SOURCE, A_Source);
    fAttMap->put((void*)SchemaSymbols::fgATT_SUBSTITUTIONGROUP, A_SubstitutionGroup);
    fAttMap->put((void*)SchemaSymbols::fgATT_SYSTEM, A_System);
    fAttMap->put((void*)SchemaSymbols::fgATT_TARGETNAMESPACE, A_TargetNamespace);
    fAttMap->put((void*)SchemaSymbols::fgATT_TYPE, A_Type);
    fAttMap->put((void*)SchemaSymbols::fgATT_USE, A_Use);
    fAttMap->put((void*)SchemaSymbols::fgATT_VALUE, A_Value);
    fAttMap->put((void*)SchemaSymbols::fgATT_VERSION, A_Version);
    fAttMap->put((void*)SchemaSymbols::fgATT_XPATH, A_XPath);

    fFacetsMap = new ValueHashTableOf<unsigned short>(13);

    fFacetsMap->put((void*) SchemaSymbols::fgELT_MINEXCLUSIVE, E_MinExclusive);
    fFacetsMap->put((void*) SchemaSymbols::fgELT_MININCLUSIVE, E_MinInclusive);
    fFacetsMap->put((void*) SchemaSymbols::fgELT_MAXEXCLUSIVE, E_MaxExclusive);
    fFacetsMap->put((void*) SchemaSymbols::fgELT_MAXINCLUSIVE, E_MaxInclusive);
    fFacetsMap->put((void*) SchemaSymbols::fgELT_TOTALDIGITS, E_TotalDigits);
    fFacetsMap->put((void*) SchemaSymbols::fgELT_FRACTIONDIGITS, E_FractionDigits);
    fFacetsMap->put((void*) SchemaSymbols::fgELT_LENGTH, E_Length);
    fFacetsMap->put((void*) SchemaSymbols::fgELT_MINLENGTH, E_MinLength);
    fFacetsMap->put((void*) SchemaSymbols::fgELT_MAXLENGTH, E_MaxLength);
    fFacetsMap->put((void*) SchemaSymbols::fgELT_ENUMERATION, E_Enumeration);
    fFacetsMap->put((void*) SchemaSymbols::fgELT_WHITESPACE, E_WhiteSpace);
    fFacetsMap->put((void*) SchemaSymbols::fgELT_PATTERN, E_Pattern);
}

// ---------------------------------------------------------------------------
//  GeneralAttributeCheck: Constructors and Destructor
// ---------------------------------------------------------------------------
GeneralAttributeCheck::GeneralAttributeCheck(MemoryManager* const manager)
    : fMemoryManager(manager)
    , fIDValidator(manager)
{
}

GeneralAttributeCheck::~GeneralAttributeCheck()
{
}

// ---------------------------------------------------------------------------
//  GeneralAttributeCheck: Validation methods
// ---------------------------------------------------------------------------
void
GeneralAttributeCheck::checkAttributes(const DOMElement* const elem,
                                       const unsigned short elemContext,
                                       TraverseSchema* const schema,
                                       const bool isTopLevel,
                                       ValueVectorOf<DOMNode*>* const nonXSAttList)
{
    if (nonXSAttList)
        nonXSAttList->removeAllElements();

    if (elem == 0 || !fAttMap || elemContext>=E_Count)
        return;

    const XMLCh* elemName = elem->getLocalName();
    if (!XMLString::equals(SchemaSymbols::fgURI_SCHEMAFORSCHEMA, elem->getNamespaceURI())) {
        schema->reportSchemaError
        (
            elem
            , XMLUni::fgXMLErrDomain
            , XMLErrs::ELTSchemaNS
            , elemName
        );
    }

    DOMNamedNodeMap* eltAttrs = elem->getAttributes();
    const XMLSize_t  attrCount = eltAttrs->getLength();
    XMLByte          attList[A_Count];

    memset(attList, 0, sizeof(attList));

    for (XMLSize_t i = 0; i < attrCount; i++) {

        DOMNode*     attribute = eltAttrs->item(i);
        const XMLCh* attName = attribute->getNodeName();

        // skip namespace declarations
        if (XMLString::equals(attName, XMLUni::fgXMLNSString)
            || XMLString::startsWith(attName, XMLUni::fgXMLNSColonString))
            continue;

        // Bypass attributes that start with xml
        // add this to the list of "non-schema" attributes
        if ((*attName == chLatin_X || *attName == chLatin_x)
           && (*(attName+1) == chLatin_M || *(attName+1) == chLatin_m)
           && (*(attName+2) == chLatin_L || *(attName+2) == chLatin_l)) {

           if (nonXSAttList)
                nonXSAttList->addElement(attribute);

            continue;
        }

        // for attributes with namespace prefix
        const XMLCh* attrURI = attribute->getNamespaceURI();

        if (attrURI != 0 && *attrURI) {

            // attributes with schema namespace are not allowed
            // and not allowed on "documentation" and "appInfo"
            if (XMLString::equals(attrURI, SchemaSymbols::fgURI_SCHEMAFORSCHEMA) ||
                XMLString::equals(elemName, SchemaSymbols::fgELT_APPINFO) ||
                XMLString::equals(elemName, SchemaSymbols::fgELT_DOCUMENTATION)) {

                schema->reportSchemaError(elem, XMLUni::fgXMLErrDomain,
                                          isTopLevel?XMLErrs::AttributeDisallowedGlobal:XMLErrs::AttributeDisallowedLocal, 
                                          attName, elemName);
            }
            else if (nonXSAttList)
            {
                nonXSAttList->addElement(attribute);
            }

            continue;
        }

        int attNameId = A_Invalid;
        attName = attribute->getLocalName();

        bool bContinue=false;   // workaround for Borland bug with 'continue' in 'catch'
        try {
            attNameId= fAttMap->get(attName, fMemoryManager);
        }
        catch(const OutOfMemoryException&)
        {
            throw;
        }
        catch(...) {

            schema->reportSchemaError(elem, XMLUni::fgXMLErrDomain,
                                      isTopLevel?XMLErrs::AttributeDisallowedGlobal:XMLErrs::AttributeDisallowedLocal, 
                                      attName, elemName);
            bContinue=true;
        }
        if(bContinue)
            continue;

        if (fgElemAttTable[elemContext][attNameId] & Att_Mask) {

            attList[attNameId] = 1;
            validate
            (
                elem
                , attName
                , attribute->getNodeValue()
                , fgElemAttTable[elemContext][attNameId] & DV_Mask
                , schema
            );
        }
        else {
            schema->reportSchemaError(elem, XMLUni::fgXMLErrDomain,
                                      isTopLevel?XMLErrs::AttributeDisallowedGlobal:XMLErrs::AttributeDisallowedLocal, 
                                      attName, elemName);
        }
    }

    // ------------------------------------------------------------------
    // Check for required attributes
    // ------------------------------------------------------------------
    for (unsigned int j=0; j < A_Count; j++) {

        if ((fgElemAttTable[elemContext][j] & Att_Required) && attList[j] == 0) {
            schema->reportSchemaError(elem, XMLUni::fgXMLErrDomain, 
                                      isTopLevel?XMLErrs::AttributeRequiredGlobal:XMLErrs::AttributeRequiredLocal, 
                                      fAttNames[j], elemName);
        }
    }
}


void GeneralAttributeCheck::validate(const DOMElement* const elem,
                                     const XMLCh* const attName,
                                     const XMLCh* const attValue,
                                     const short dvIndex,
                                     TraverseSchema* const schema)
{
    bool isInvalid = false;
    DatatypeValidator* dv = 0;

    ValidationContext* fValidationContext = schema->fSchemaInfo->getValidationContext();
    switch (dvIndex) {
    case DV_Form:
        if (!XMLString::equals(attValue, SchemaSymbols::fgATTVAL_QUALIFIED)
            && !XMLString::equals(attValue, SchemaSymbols::fgATTVAL_UNQUALIFIED)) {
            isInvalid = true;
        }
        break;
    case DV_MaxOccurs:
            // maxOccurs = (nonNegativeInteger | unbounded)
        if (!XMLString::equals(attValue, fgUnbounded)) {
            dv = fNonNegIntDV;
        }
        break;
    case DV_MaxOccurs1:
        if (!XMLString::equals(attValue, fgValueOne)) {
            isInvalid = true;
        }
        break;
    case DV_MinOccurs1:
        if (!XMLString::equals(attValue, fgValueZero)
            && !XMLString::equals(attValue, fgValueOne)) {
            isInvalid = true;
        }
        break;
    case DV_ProcessContents:
        if (!XMLString::equals(attValue, SchemaSymbols::fgATTVAL_SKIP)
            && !XMLString::equals(attValue, SchemaSymbols::fgATTVAL_LAX)
            && !XMLString::equals(attValue, SchemaSymbols::fgATTVAL_STRICT)) {
            isInvalid = true;
        }
        break;
    case DV_Use:
        if (!XMLString::equals(attValue, SchemaSymbols::fgATTVAL_OPTIONAL)
            && !XMLString::equals(attValue, SchemaSymbols::fgATTVAL_PROHIBITED)
            && !XMLString::equals(attValue, SchemaSymbols::fgATTVAL_REQUIRED)) {
            isInvalid = true;
        }
        break;
    case DV_WhiteSpace:
        if (!XMLString::equals(attValue, SchemaSymbols::fgWS_PRESERVE)
            && !XMLString::equals(attValue, SchemaSymbols::fgWS_REPLACE)
            && !XMLString::equals(attValue, SchemaSymbols::fgWS_COLLAPSE)) {
            isInvalid = true;
        }
        break;
    case DV_Boolean:
        dv = fBooleanDV;
        break;
    case DV_NonNegInt:
        dv = fNonNegIntDV;
        break;
    case DV_AnyURI:
        dv = fAnyURIDV;
        break;
    case DV_ID:
        if (fValidationContext)
        {
            dv = &fIDValidator;
        }
        break;
    }

    if (dv) {
        try {
            dv->validate(attValue, fValidationContext, fMemoryManager);
        }
        catch(const XMLException& excep) {
            schema->reportSchemaError(elem, excep);
        }
        catch(const OutOfMemoryException&)
        {
            throw;
        }
        catch(...) {
            isInvalid = true;
        }
    }

    if (isInvalid) {
        schema->reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::InvalidAttValue,
                                  attValue, attName);
    }
}


// ---------------------------------------------------------------------------
//  Conditional methods for building the table
// ---------------------------------------------------------------------------

//
//  This code will set up the character flags table. Its defined out since
//  this table is now written out and hard coded (at the bottom of this
//  file) into the code itself. This code is retained in case there is
//  any need to recreate it later.
//

#if defined(NEED_TO_GEN_ELEM_ATT_MAP_TABLE)

#include <stdio.h>

void GeneralAttributeCheck::initCharFlagTable()
{
    unsigned short attList[E_Count][A_Count];

    for (unsigned int i=0; i < E_Count; i++) {
        for (unsigned int j=0; j < A_Count; j++) {
            attList[i][j] = 0;
        }
    }

    //
    //  Write it out to a temp file to be read back into this source later.
    //
    FILE* outFl = fopen("ea_table.out", "wt+");
    fprintf(outFl, "unsigned short GeneralAttributeCheck::fgElemAttTable[GeneralAttributeCheck::E_Count][GeneralAttributeCheck::A_Count] =\n{\n    {");

    //"all"
    attList[E_All][A_ID] = Att_Optional | DV_ID;
    attList[E_All][A_MaxOccurs] = Att_Optional | DV_MaxOccurs1;
    attList[E_All][A_MinOccurs] = Att_Optional | DV_MinOccurs1;

    // "annotation"
    attList[E_Annotation][A_ID] = Att_Optional | DV_ID;

    // "any"
    attList[E_Any][A_ID] = Att_Optional | DV_ID;
    attList[E_Any][A_MaxOccurs] = Att_Optional | DV_MaxOccurs;
    attList[E_Any][A_MinOccurs] = Att_Optional | DV_NonNegInt;
    attList[E_Any][A_Namespace] = Att_Optional;
    attList[E_Any][A_ProcessContents] = Att_Optional | DV_ProcessContents;

    // "anyAttribute"
    attList[E_AnyAttribute][A_ID] = Att_Optional | DV_ID;
    attList[E_AnyAttribute][A_Namespace] = Att_Optional;
    attList[E_AnyAttribute][A_ProcessContents] = Att_Optional | DV_ProcessContents;

    // "appinfo"
    attList[E_Appinfo][A_Source]= Att_Optional | DV_AnyURI;

    // attribute - global"
    attList[E_AttributeGlobal][A_Default] = Att_Optional;
    attList[E_AttributeGlobal][A_Fixed] = Att_Optional;
    attList[E_AttributeGlobal][A_ID] = Att_Optional | DV_ID;
    attList[E_AttributeGlobal][A_Name] = Att_Required;
    attList[E_AttributeGlobal][A_Type] = Att_Optional;

    // "attribute - local"
    attList[E_AttributeLocal][A_Default] = Att_Optional;
    attList[E_AttributeLocal][A_Fixed] = Att_Optional;
    attList[E_AttributeLocal][A_Form]= Att_Optional | DV_Form;
    attList[E_AttributeLocal][A_ID] = Att_Optional | DV_ID;
    attList[E_AttributeLocal][A_Name] = Att_Required;
    attList[E_AttributeLocal][A_Type] = Att_Optional;
    attList[E_AttributeLocal][A_Use] = Att_Optional | DV_Use;

    // "attribute - ref"
    attList[E_AttributeRef][A_Default] = Att_Optional;
    attList[E_AttributeRef][A_Fixed] = Att_Optional;
    attList[E_AttributeRef][A_ID] = Att_Optional | DV_ID;
    attList[E_AttributeRef][A_Ref]= Att_Required;
    attList[E_AttributeRef][A_Use] = Att_Optional | DV_Use;

    // "attributeGroup - global"
    attList[E_AttributeGroupGlobal][A_ID] = Att_Optional | DV_ID;
    attList[E_AttributeGroupGlobal][A_Name] = Att_Required;

    // "attributeGroup - ref"
    attList[E_AttributeGroupRef][A_ID] = Att_Optional | DV_ID;
    attList[E_AttributeGroupRef][A_Ref]= Att_Required;

    // "choice"
    attList[E_Choice][A_ID] = Att_Optional | DV_ID;
    attList[E_Choice][A_MaxOccurs] = Att_Optional | DV_MaxOccurs;
    attList[E_Choice][A_MinOccurs] = Att_Optional | DV_NonNegInt;

    // "complexContent"
    attList[E_ComplexContent][A_ID] = Att_Optional | DV_ID;
    attList[E_ComplexContent][A_Mixed] = Att_Optional | DV_Boolean;

    // "complexType - global"
    attList[E_ComplexTypeGlobal][A_Abstract] = Att_Optional | DV_Boolean;
    attList[E_ComplexTypeGlobal][A_Block] = Att_Optional;
    attList[E_ComplexTypeGlobal][A_Final] = Att_Optional;
    attList[E_ComplexTypeGlobal][A_ID] = Att_Optional | DV_ID;
    attList[E_ComplexTypeGlobal][A_Mixed] = Att_Optional | DV_Boolean;
    attList[E_ComplexTypeGlobal][A_Name] = Att_Required;

    // "complexType - local"
    attList[E_ComplexTypeLocal][A_ID] = Att_Optional | DV_ID;
    attList[E_ComplexTypeLocal][A_Mixed] = Att_Optional | DV_Boolean;

    // "documentation"
    attList[E_Documentation][A_Source] = Att_Optional | DV_AnyURI;

    // "element - global"
    attList[E_ElementGlobal][A_Abstract] = Att_Optional | DV_Boolean;
    attList[E_ElementGlobal][A_Block] = Att_Optional;
    attList[E_ElementGlobal][A_Default] = Att_Optional;
    attList[E_ElementGlobal][A_Final] = Att_Optional;
    attList[E_ElementGlobal][A_Fixed] = Att_Optional;
    attList[E_ElementGlobal][A_ID] = Att_Optional | DV_ID;
    attList[E_ElementGlobal][A_Name] = Att_Required;
    attList[E_ElementGlobal][A_Nillable] = Att_Optional | DV_Boolean;
    attList[E_ElementGlobal][A_SubstitutionGroup] = Att_Optional;
    attList[E_ElementGlobal][A_Type] = Att_Optional;

    // "element - local"
    attList[E_ElementLocal][A_Block]= Att_Optional;
    attList[E_ElementLocal][A_Default] = Att_Optional;
    attList[E_ElementLocal][A_Fixed] = Att_Optional;
    attList[E_ElementLocal][A_Form] = Att_Optional | DV_Form;
    attList[E_ElementLocal][A_ID] = Att_Optional | DV_ID;
    attList[E_ElementLocal][A_MaxOccurs] = Att_Optional | DV_MaxOccurs;
    attList[E_ElementLocal][A_MinOccurs] = Att_Optional | DV_NonNegInt;
    attList[E_ElementLocal][A_Name] = Att_Required;
    attList[E_ElementLocal][A_Nillable] = Att_Optional | DV_Boolean;
    attList[E_ElementLocal][A_Type] = Att_Optional;

    //"element - ref"
    attList[E_ElementRef][A_ID] = Att_Optional | DV_ID;
    attList[E_ElementRef][A_MaxOccurs] = Att_Optional | DV_MaxOccurs;
    attList[E_ElementRef][A_MinOccurs] = Att_Optional | DV_NonNegInt;
    attList[E_ElementRef][A_Ref] = Att_Required;

    // "enumeration"
    attList[E_Enumeration][A_ID] = Att_Optional | DV_ID;
    attList[E_Enumeration][A_Value] = Att_Optional;

    // "extension"
    attList[E_Extension][A_Base] = Att_Required;
    attList[E_Extension][A_ID] = Att_Optional | DV_ID;

    //"field"
    attList[E_Field][A_ID] = Att_Optional | DV_ID;
    attList[E_Field][A_XPath] = Att_Required;

    // "fractionDigits"
    attList[E_FractionDigits][A_ID] = Att_Optional | DV_ID;
    attList[E_FractionDigits][A_Value] = Att_Optional | DV_NonNegInt;
    attList[E_FractionDigits][A_Fixed] = Att_Optional | DV_Boolean;

    // "group - global"
    attList[E_GroupGlobal][A_ID] = Att_Optional | DV_ID;
    attList[E_GroupGlobal][A_Name] = Att_Required;

    // "group - ref"
    attList[E_GroupRef][A_ID] = Att_Optional | DV_ID;
    attList[E_GroupRef][A_MaxOccurs] = Att_Optional | DV_MaxOccurs;
    attList[E_GroupRef][A_MinOccurs] = Att_Optional | DV_NonNegInt;
    attList[E_GroupRef][A_Ref] = Att_Required;

    // "import"
    attList[E_Import][A_ID] = Att_Optional | DV_ID;
    attList[E_Import][A_Namespace] = Att_Optional;
    attList[E_Import][A_SchemaLocation] = Att_Optional;

    // "include"
    attList[E_Include][A_ID] = Att_Optional | DV_ID;
    attList[E_Include][A_SchemaLocation] = Att_Required;

    // "key"
    attList[E_Key][A_ID] = Att_Optional | DV_ID;
    attList[E_Key][A_Name] = Att_Required;

    // "keyref"
    attList[E_KeyRef][A_ID] = Att_Optional | DV_ID;
    attList[E_KeyRef][A_Name] = Att_Required;
    attList[E_KeyRef][A_Refer] = Att_Required;

    // "length"
    attList[E_Length][A_ID] = Att_Optional | DV_ID;
    attList[E_Length][A_Value] = Att_Optional | DV_NonNegInt;
    attList[E_Length][A_Fixed] = Att_Optional | DV_Boolean;

    // "list"
    attList[E_List][A_ID] = Att_Optional | DV_ID;
    attList[E_List][A_ItemType] = Att_Optional;

    // "maxExclusive"
    attList[E_MaxExclusive][A_ID] = Att_Optional | DV_ID;
    attList[E_MaxExclusive][A_Value] = Att_Optional;
    attList[E_MaxExclusive][A_Fixed] = Att_Optional | DV_Boolean;

    // "maxInclusive"
    attList[E_MaxInclusive][A_ID] = Att_Optional | DV_ID;
    attList[E_MaxInclusive][A_Value] = Att_Optional;
    attList[E_MaxInclusive][A_Fixed] = Att_Optional | DV_Boolean;

    // "maxLength"
    attList[E_MaxLength][A_ID] = Att_Optional | DV_ID;
    attList[E_MaxLength][A_Value] = Att_Optional | DV_NonNegInt;
    attList[E_MaxLength][A_Fixed] = Att_Optional | DV_Boolean;

    // "minExclusive"
    attList[E_MinExclusive][A_ID] = Att_Optional | DV_ID;
    attList[E_MinExclusive][A_Value] = Att_Optional;
    attList[E_MinExclusive][A_Fixed] = Att_Optional | DV_Boolean;

    // "minInclusive"
    attList[E_MinInclusive][A_ID] = Att_Optional | DV_ID;
    attList[E_MinInclusive][A_Value] = Att_Optional;
    attList[E_MinInclusive][A_Fixed] = Att_Optional | DV_Boolean;

    // "minLength"
    attList[E_MinLength][A_ID] = Att_Optional | DV_ID;
    attList[E_MinLength][A_Value] = Att_Optional | DV_NonNegInt;
    attList[E_MinLength][A_Fixed] = Att_Optional | DV_Boolean;

    // "notation"
    attList[E_Notation][A_ID] = Att_Optional | DV_ID;
    attList[E_Notation][A_Name] = Att_Required;
    attList[E_Notation][A_Public] = Att_Optional;
    attList[E_Notation][A_System] = Att_Optional | DV_AnyURI;

    // "pattern"
    attList[E_Pattern][A_ID] = Att_Optional;
    attList[E_Pattern][A_Value] = Att_Optional;

    // "redefine"
    attList[E_Redefine][A_ID] = Att_Optional | DV_ID;
    attList[E_Redefine][A_SchemaLocation] = Att_Required;

    // "restriction"
    attList[E_Restriction][A_Base] = Att_Optional;
    attList[E_Restriction][A_ID] = Att_Optional | DV_ID;

    // "schema"
    attList[E_Schema][A_AttributeFormDefault] = Att_Optional | DV_Form;
    attList[E_Schema][A_BlockDefault] = Att_Optional;
    attList[E_Schema][A_ElementFormDefault] = Att_Optional | DV_Form;
    attList[E_Schema][A_FinalDefault] = Att_Optional;
    attList[E_Schema][A_ID] = Att_Optional | DV_ID;
    attList[E_Schema][A_TargetNamespace] = Att_Optional;
    attList[E_Schema][A_Version] = Att_Optional;

    // "selector"
    attList[E_Selector][A_ID] = Att_Optional | DV_ID;
    attList[E_Selector][A_XPath] = Att_Required;

    // "sequence"
    attList[E_Sequence][A_ID] = Att_Optional | DV_ID;
    attList[E_Sequence][A_MaxOccurs] = Att_Optional | DV_MaxOccurs;
    attList[E_Sequence][A_MinOccurs] = Att_Optional | DV_NonNegInt;

    // "simpleContent"
    attList[E_SimpleContent][A_ID] = Att_Optional | DV_ID;

    // "simpleType - global"
    attList[E_SimpleTypeGlobal][A_Final] = Att_Optional;
    attList[E_SimpleTypeGlobal][A_ID] = Att_Optional | DV_ID;
    attList[E_SimpleTypeGlobal][A_Name] = Att_Required;

    // "simpleType - local"
    attList[E_SimpleTypeLocal][A_Final] = Att_Optional;
    attList[E_SimpleTypeLocal][A_ID] = Att_Optional | DV_ID;

    // "totalDigits"
    attList[E_TotalDigits][A_ID] = Att_Optional | DV_ID;
    attList[E_TotalDigits][A_Value] = Att_Optional | DV_NonNegInt;
    attList[E_TotalDigits][A_Fixed] = Att_Optional | DV_Boolean;

    // "union"
    attList[E_Union][A_ID] = Att_Optional | DV_ID;
    attList[E_Union][A_MemberTypes] = Att_Optional;

    // "unique"
    attList[E_Unique][A_ID] = Att_Optional | DV_ID;
    attList[E_Unique][A_Name] = Att_Required;

    // "whitespace"
    attList[E_WhiteSpace][A_ID] = Att_Optional | DV_ID;
    attList[E_WhiteSpace][A_Value] = Att_Optional | DV_WhiteSpace;
    attList[E_WhiteSpace][A_Fixed] = Att_Optional | DV_Boolean;

    for (unsigned int j=0; j < E_Count; j++) {

        for (unsigned int index = 0; index < A_Count-1; index++)
        {
            fprintf(outFl, " %d,", attList[j][index]);
        }

        fprintf(outFl, " %d", attList[j][A_Count - 1]);

        if (j + 1 == E_Count)
            fprintf(outFl, "}\n};");
        else
            fprintf(outFl, "},\n    {");
    }

    fclose(outFl);
}

#endif


unsigned short GeneralAttributeCheck::fgElemAttTable[GeneralAttributeCheck::E_Count][GeneralAttributeCheck::A_Count] =
{
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 34, 0, 258, 0, 514, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 34, 0, 130, 0, 10, 0, 0, 2, 0, 1026, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 2, 0, 1026, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 2, 0, 0, 0, 2, 0, 34, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 2, 0, 0, 0, 2, 66, 34, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2050, 0, 0, 0},
  { 0, 0, 0, 0, 0, 2, 0, 0, 0, 2, 0, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 2050, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 34, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 34, 0, 130, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 34, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  { 18, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 34, 0, 0, 0, 0, 18, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 34, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0},
  { 18, 0, 0, 2, 0, 2, 0, 2, 0, 2, 0, 34, 0, 0, 0, 0, 0, 1, 0, 18, 0, 0, 0, 0, 0, 0, 2, 0, 0, 2, 0, 0, 0, 0},
  { 0, 0, 0, 2, 0, 2, 0, 0, 0, 2, 66, 34, 0, 130, 0, 10, 0, 1, 0, 18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 34, 0, 130, 0, 10, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0},
  { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 34, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 34, 0, 130, 0, 10, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 34, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 34, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 34, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 34, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 66, 0, 0, 2, 0, 66, 0, 2, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 2, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 34, 0, 130, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 34, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 34, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 34, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4098, 0, 0}
};

const XMLCh* GeneralAttributeCheck::fAttNames[GeneralAttributeCheck::A_Count] =
{
    SchemaSymbols::fgATT_ABSTRACT,
    SchemaSymbols::fgATT_ATTRIBUTEFORMDEFAULT,
    SchemaSymbols::fgATT_BASE,
    SchemaSymbols::fgATT_BLOCK,
    SchemaSymbols::fgATT_BLOCKDEFAULT,
    SchemaSymbols::fgATT_DEFAULT,
    SchemaSymbols::fgATT_ELEMENTFORMDEFAULT,
    SchemaSymbols::fgATT_FINAL,
    SchemaSymbols::fgATT_FINALDEFAULT,
    SchemaSymbols::fgATT_FIXED,
    SchemaSymbols::fgATT_FORM,
    SchemaSymbols::fgATT_ID,
    SchemaSymbols::fgATT_ITEMTYPE,
    SchemaSymbols::fgATT_MAXOCCURS,
    SchemaSymbols::fgATT_MEMBERTYPES,
    SchemaSymbols::fgATT_MINOCCURS,
    SchemaSymbols::fgATT_MIXED,
    SchemaSymbols::fgATT_NAME,
    SchemaSymbols::fgATT_NAMESPACE,
    SchemaSymbols::fgATT_NILLABLE,
    SchemaSymbols::fgATT_PROCESSCONTENTS,
    SchemaSymbols::fgATT_PUBLIC,
    SchemaSymbols::fgATT_REF,
    SchemaSymbols::fgATT_REFER,
    SchemaSymbols::fgATT_SCHEMALOCATION,
    SchemaSymbols::fgATT_SOURCE,
    SchemaSymbols::fgATT_SUBSTITUTIONGROUP,
    SchemaSymbols::fgATT_SYSTEM,
    SchemaSymbols::fgATT_TARGETNAMESPACE,
    SchemaSymbols::fgATT_TYPE,
    SchemaSymbols::fgATT_USE,
    SchemaSymbols::fgATT_VALUE,
    SchemaSymbols::fgATT_VERSION,
    SchemaSymbols::fgATT_XPATH,
};

XERCES_CPP_NAMESPACE_END

/**
  * End of file GeneralAttributeCheck.cpp
  */


