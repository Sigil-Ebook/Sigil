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
 * $Id: XSSimpleTypeDefinition.cpp 674012 2008-07-04 11:18:21Z borisk $
 */

#include <xercesc/framework/psvi/XSSimpleTypeDefinition.hpp>
#include <xercesc/framework/psvi/XSFacet.hpp>
#include <xercesc/framework/psvi/XSMultiValueFacet.hpp>
#include <xercesc/framework/psvi/XSAnnotation.hpp>
#include <xercesc/framework/psvi/XSModel.hpp>
#include <xercesc/validators/datatype/DatatypeValidator.hpp>
#include <xercesc/validators/datatype/UnionDatatypeValidator.hpp>
#include <xercesc/util/XMLStringTokenizer.hpp>
#include <xercesc/util/XMLUniDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Local, static functions
// ---------------------------------------------------------------------------
static bool XSSimpleTypeDefinitionTestFlag(int flag)
{
    if (flag)
        return true;
    return false;
}


// ---------------------------------------------------------------------------
//  XSSimpleTypeDefinition: Constructors and Destructors
// ---------------------------------------------------------------------------
XSSimpleTypeDefinition::XSSimpleTypeDefinition
(
    DatatypeValidator* const            datatypeValidator
    , VARIETY                           stVariety
    , XSTypeDefinition* const           xsBaseType
    , XSSimpleTypeDefinition* const     primitiveOrItemType
    , XSSimpleTypeDefinitionList* const memberTypes
    , XSAnnotation*                     headAnnot
    , XSModel* const                    xsModel
    , MemoryManager* const              manager
)
    : XSTypeDefinition(SIMPLE_TYPE, xsBaseType, xsModel, manager)
    , fDefinedFacets(0)
    , fFixedFacets(0)
    , fVariety(stVariety)
    , fDatatypeValidator(datatypeValidator)
    , fXSFacetList(0)
    , fXSMultiValueFacetList(0)
    , fPatternList(0)
    , fPrimitiveOrItemType(primitiveOrItemType)
    , fMemberTypes(memberTypes)
    , fXSAnnotationList(0)
{
    int finalset = fDatatypeValidator->getFinalSet();
    if (finalset)
    {
        if (finalset & SchemaSymbols::XSD_EXTENSION)
            fFinal |= XSConstants::DERIVATION_EXTENSION;

        if (finalset & SchemaSymbols::XSD_RESTRICTION)
            fFinal |= XSConstants::DERIVATION_RESTRICTION;

        if (finalset & SchemaSymbols::XSD_LIST)
            fFinal |= XSConstants::DERIVATION_LIST;

        if (finalset & SchemaSymbols::XSD_UNION)
            fFinal |= XSConstants::DERIVATION_UNION;
    }

    if (headAnnot)
    {
        XSAnnotation* annot = headAnnot;

        fXSAnnotationList = new (manager) RefVectorOf<XSAnnotation>(3, false, manager);
        do
        {
            fXSAnnotationList->addElement(annot);
            annot = annot->getNext();
        } while (annot);
    }
}

XSSimpleTypeDefinition::~XSSimpleTypeDefinition()
{
    if (fXSFacetList)
        delete fXSFacetList;

    if (fXSMultiValueFacetList)
        delete fXSMultiValueFacetList;

    if (fPatternList)
        delete fPatternList;

    // don't delete fPrimitiveOrItemType -> deleted by XSModel
    if (fMemberTypes)
        delete fMemberTypes;

    if (fXSAnnotationList)
        delete fXSAnnotationList;
}


// ---------------------------------------------------------------------------
//  XSSimpleTypeDefinition: access methods
// ---------------------------------------------------------------------------
bool XSSimpleTypeDefinition::isDefinedFacet(FACET facetName)
{
    return XSSimpleTypeDefinitionTestFlag(fDefinedFacets & facetName);
}

bool XSSimpleTypeDefinition::isFixedFacet(FACET facetName)
{
    return XSSimpleTypeDefinitionTestFlag(fFixedFacets & facetName);
}

const XMLCh *XSSimpleTypeDefinition::getLexicalFacetValue(FACET facetName)
{
    XMLSize_t size = fXSFacetList->size();
    for (XMLSize_t i=0; i<size; i++)
    {
        if (((fXSFacetList->elementAt(i))->getFacetKind()) == facetName)
            return (fXSFacetList->elementAt(i))->getLexicalFacetValue();
    }
    return 0;
}

StringList *XSSimpleTypeDefinition::getLexicalEnumeration()
{
    return (RefArrayVectorOf<XMLCh>*) fDatatypeValidator->getEnumString();
}

XSSimpleTypeDefinition::ORDERING XSSimpleTypeDefinition::getOrdered() const
{
    return fDatatypeValidator->getOrdered();
}

bool XSSimpleTypeDefinition::getFinite() const
{
    return fDatatypeValidator->getFinite();
}

bool XSSimpleTypeDefinition::getBounded() const
{
    return fDatatypeValidator->getBounded();
}

bool XSSimpleTypeDefinition::getNumeric() const
{
    return fDatatypeValidator->getNumeric();
}


// ---------------------------------------------------------------------------
//  XSSimpleTypeDefinition: virtual methods
// ---------------------------------------------------------------------------
const XMLCh *XSSimpleTypeDefinition::getName() const
{
    return fDatatypeValidator->getTypeLocalName();
}

const XMLCh *XSSimpleTypeDefinition::getNamespace()
{
    return fDatatypeValidator->getTypeUri();
}

XSNamespaceItem *XSSimpleTypeDefinition::getNamespaceItem()
{
    return fXSModel->getNamespaceItem(getNamespace());
}

bool XSSimpleTypeDefinition::getAnonymous() const
{
    return fDatatypeValidator->getAnonymous();
}

XSTypeDefinition *XSSimpleTypeDefinition::getBaseType()
{
    return fBaseType;
}

bool XSSimpleTypeDefinition::derivedFromType(const XSTypeDefinition * const ancestorType)
{
    if (!ancestorType)
        return false;

    XSTypeDefinition* type;

    if (ancestorType->getTypeCategory() == XSTypeDefinition::COMPLEX_TYPE)
    {
        type = (XSTypeDefinition*) ancestorType;
        if (ancestorType == type->getBaseType())
        {
            // ancestor is anytype
            return true;
        }
        return false;
    }

    type = this;
    XSTypeDefinition* lastType = 0;  // anysimple type has a base type of anytype
                                     // anytype has a basetype of anytype so will have infinite loop...

    while (type && (type != ancestorType) && (type != lastType))
    {
        lastType = type;
        type = type->getBaseType();
    }

    return (type == ancestorType);
}

// ---------------------------------------------------------------------------
//  XSSimpleTypeDefinition: helper methods
// ---------------------------------------------------------------------------
void XSSimpleTypeDefinition::setFacetInfo
(
    int                            definedFacets
    , int                          fixedFacets
    , XSFacetList* const           xsFacetList
    , XSMultiValueFacetList* const xsMultiValueFacetList
    , StringList* const            patternList
)
{
    fDefinedFacets = definedFacets;
    fFixedFacets = fixedFacets;
    fXSFacetList = xsFacetList;
    fXSMultiValueFacetList = xsMultiValueFacetList;
    fPatternList = patternList;
}


XERCES_CPP_NAMESPACE_END
