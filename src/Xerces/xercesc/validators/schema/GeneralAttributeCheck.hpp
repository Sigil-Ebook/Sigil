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
 * $Id: GeneralAttributeCheck.hpp 729944 2008-12-29 17:03:32Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_GENERALATTRIBUTECHECK_HPP)
#define XERCESC_INCLUDE_GUARD_GENERALATTRIBUTECHECK_HPP

/**
  * A general purpose class to check for valid values of attributes, as well
  * as check for proper association with corresponding schema elements.
  */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/RefHashTableOf.hpp>
#include <xercesc/util/ValueHashTableOf.hpp>
#include <xercesc/validators/datatype/IDDatatypeValidator.hpp>
#include <xercesc/framework/ValidationContext.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Forward declaration
// ---------------------------------------------------------------------------
class TraverseSchema;
class DOMElement;
class DOMNode;

class VALIDATORS_EXPORT GeneralAttributeCheck : public XMemory
{
public:
    // -----------------------------------------------------------------------
    //  Constants
    // -----------------------------------------------------------------------
    //Elements
    enum
    {
        E_All,
        E_Annotation,
        E_Any,
        E_AnyAttribute,
        E_Appinfo,
        E_AttributeGlobal,
        E_AttributeLocal,
        E_AttributeRef,
        E_AttributeGroupGlobal,
        E_AttributeGroupRef,
        E_Choice,
        E_ComplexContent,
        E_ComplexTypeGlobal,
        E_ComplexTypeLocal,
        E_Documentation,
        E_ElementGlobal,
        E_ElementLocal,
        E_ElementRef,
        E_Enumeration,
        E_Extension,
        E_Field,
        E_FractionDigits,
        E_GroupGlobal,
        E_GroupRef,
        E_Import,
        E_Include,
        E_Key,
        E_KeyRef,
        E_Length,
        E_List,
        E_MaxExclusive,
        E_MaxInclusive,
        E_MaxLength,
        E_MinExclusive,
        E_MinInclusive,
        E_MinLength,
        E_Notation,
        E_Pattern,
        E_Redefine,
        E_Restriction,
        E_Schema,
        E_Selector,
        E_Sequence,
        E_SimpleContent,
        E_SimpleTypeGlobal,
        E_SimpleTypeLocal,
        E_TotalDigits,
        E_Union,
        E_Unique,
        E_WhiteSpace,

        E_Count,
        E_Invalid = -1
    };

    //Attributes
    enum
    {
        A_Abstract,
        A_AttributeFormDefault,
        A_Base,
        A_Block,
        A_BlockDefault,
        A_Default,
        A_ElementFormDefault,
        A_Final,
        A_FinalDefault,
        A_Fixed,
        A_Form,
        A_ID,
        A_ItemType,
        A_MaxOccurs,
        A_MemberTypes,
        A_MinOccurs,
        A_Mixed,
        A_Name,
        A_Namespace,
        A_Nillable,
        A_ProcessContents,
        A_Public,
        A_Ref,
        A_Refer,
        A_SchemaLocation,
        A_Source,
        A_SubstitutionGroup,
        A_System,
        A_TargetNamespace,
        A_Type,
        A_Use,
        A_Value,
        A_Version,
        A_XPath,

        A_Count,
        A_Invalid = -1
    };

    //Validators
    enum {

        DV_String = 0,
        DV_AnyURI = 4,
        DV_NonNegInt = 8,
        DV_Boolean = 16,
        DV_ID = 32,
        DV_Form = 64,
        DV_MaxOccurs = 128,
        DV_MaxOccurs1 = 256,
        DV_MinOccurs1 = 512,
        DV_ProcessContents = 1024,
        DV_Use = 2048,
        DV_WhiteSpace = 4096,

        DV_Mask = (DV_AnyURI | DV_NonNegInt | DV_Boolean | DV_ID | DV_Form |
                   DV_MaxOccurs | DV_MaxOccurs1 | DV_MinOccurs1 |
                   DV_ProcessContents | DV_Use | DV_WhiteSpace)
    };

    // generate element-attributes map table
#if defined(NEED_TO_GEN_ELEM_ATT_MAP_TABLE)
    static void initCharFlagTable();
#endif

    // -----------------------------------------------------------------------
    //  Constructor/Destructor
    // -----------------------------------------------------------------------
    GeneralAttributeCheck(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    ~GeneralAttributeCheck();

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    unsigned short getFacetId(const XMLCh* const facetName, MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    // -----------------------------------------------------------------------
    //  Validation methods
    // -----------------------------------------------------------------------
    void checkAttributes(const DOMElement* const elem,
                         const unsigned short elemContext,
                         TraverseSchema* const schema,
                         const bool isTopLevel = false,
                         ValueVectorOf<DOMNode*>* const nonXSAttList = 0);

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    GeneralAttributeCheck(const GeneralAttributeCheck&);
    GeneralAttributeCheck& operator=(const GeneralAttributeCheck&);

    // -----------------------------------------------------------------------
    //  Validation methods
    // -----------------------------------------------------------------------
    void validate(const DOMElement* const elem, const XMLCh* const attName, const XMLCh* const attValue,
                  const short dvIndex, TraverseSchema* const schema);

    // -----------------------------------------------------------------------
    //  Private Constants
    // -----------------------------------------------------------------------
    // optional vs. required attribute
    enum {
        Att_Required = 1,
        Att_Optional = 2,
        Att_Mask = 3
    };

    // -----------------------------------------------------------------------
    //  Private data members
    // -----------------------------------------------------------------------
    static ValueHashTableOf<unsigned short>* fAttMap;
    static ValueHashTableOf<unsigned short>* fFacetsMap;
    static DatatypeValidator*                fNonNegIntDV;
    static DatatypeValidator*                fBooleanDV;
    static DatatypeValidator*                fAnyURIDV;
    static unsigned short                    fgElemAttTable[E_Count][A_Count];
    static const XMLCh*                      fAttNames[A_Count];
    MemoryManager*                           fMemoryManager;
    IDDatatypeValidator                      fIDValidator;

private:
    static void initialize();

    friend class XMLInitializer;
};


// ---------------------------------------------------------------------------
//  GeneralAttributeCheck: Getter methods
// ---------------------------------------------------------------------------
inline unsigned short
GeneralAttributeCheck::getFacetId(const XMLCh* const facetName, MemoryManager* const manager) {

    return fFacetsMap->get(facetName, manager);
}

XERCES_CPP_NAMESPACE_END

#endif

/**
  * End of file GeneralAttributeCheck.hpp
  */

