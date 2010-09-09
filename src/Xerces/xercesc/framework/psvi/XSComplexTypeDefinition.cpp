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
 * $Id: XSComplexTypeDefinition.cpp 594002 2007-11-12 01:05:09Z cargilld $
 */

#include <xercesc/framework/psvi/XSComplexTypeDefinition.hpp>
#include <xercesc/framework/psvi/XSWildcard.hpp>
#include <xercesc/framework/psvi/XSSimpleTypeDefinition.hpp>
#include <xercesc/framework/psvi/XSAttributeUse.hpp>
#include <xercesc/framework/psvi/XSModel.hpp>
#include <xercesc/framework/psvi/XSAnnotation.hpp>
#include <xercesc/framework/psvi/XSParticle.hpp>
#include <xercesc/validators/schema/ComplexTypeInfo.hpp>
#include <xercesc/validators/schema/SchemaElementDecl.hpp>
#include <xercesc/validators/schema/SchemaAttDefList.hpp>


XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  XSComplexTypeDefinition: Constructors and Destructor
// ---------------------------------------------------------------------------
XSComplexTypeDefinition::XSComplexTypeDefinition
(
    ComplexTypeInfo* const          complexTypeInfo
    , XSWildcard* const             xsWildcard
    , XSSimpleTypeDefinition* const xsSimpleType
    , XSAttributeUseList* const     xsAttList
    , XSTypeDefinition* const       xsBaseType
    , XSParticle* const             xsParticle
    , XSAnnotation* const           headAnnot
    , XSModel* const                xsModel
    , MemoryManager* const          manager
)
    : XSTypeDefinition(COMPLEX_TYPE, xsBaseType, xsModel, manager)
    , fComplexTypeInfo(complexTypeInfo)
    , fXSWildcard(xsWildcard)
    , fXSAttributeUseList(xsAttList)
    , fXSSimpleTypeDefinition(xsSimpleType)
    , fXSAnnotationList(0)
    , fParticle(xsParticle)
    , fProhibitedSubstitution(0)
{
    int blockset = fComplexTypeInfo->getBlockSet();
    if (blockset)
    {
        if (blockset & SchemaSymbols::XSD_EXTENSION)
            fProhibitedSubstitution |= XSConstants::DERIVATION_EXTENSION;

        if (blockset & SchemaSymbols::XSD_RESTRICTION)
            fProhibitedSubstitution |= XSConstants::DERIVATION_RESTRICTION;
    }

    int finalSet = fComplexTypeInfo->getFinalSet();
    if (finalSet)
    {
        if (finalSet & SchemaSymbols::XSD_EXTENSION)
            fFinal |= XSConstants::DERIVATION_EXTENSION;

        if (finalSet & SchemaSymbols::XSD_RESTRICTION)
            fFinal |= XSConstants::DERIVATION_RESTRICTION;
    }

    if (headAnnot)
    {
        fXSAnnotationList = new (manager) RefVectorOf<XSAnnotation>(1, false, manager);
        XSAnnotation* annot = headAnnot;

        do
        {
            fXSAnnotationList->addElement(annot);
            annot = annot->getNext();        
        } while (annot);
    }
}

XSComplexTypeDefinition::~XSComplexTypeDefinition() 
{
    // don't delete fXSWildcard - deleted by XSModel
    // don't delete fXSSimpleTypeDefinition - deleted by XSModel
    if (fXSAttributeUseList)
        delete fXSAttributeUseList;

    if (fXSAnnotationList)
        delete fXSAnnotationList;

    if (fParticle)
        delete fParticle;
}

// ---------------------------------------------------------------------------
//  XSComplexTypeDefinition: access methods
// ---------------------------------------------------------------------------
XSConstants::DERIVATION_TYPE XSComplexTypeDefinition::getDerivationMethod() const
{
    if(fComplexTypeInfo->getDerivedBy() == SchemaSymbols::XSD_EXTENSION)
        return XSConstants::DERIVATION_EXTENSION;    
    return XSConstants::DERIVATION_RESTRICTION;
}

bool XSComplexTypeDefinition::getAbstract() const
{
    return fComplexTypeInfo->getAbstract();
}


XSComplexTypeDefinition::CONTENT_TYPE XSComplexTypeDefinition::getContentType() const
{
    switch(fComplexTypeInfo->getContentType()) {
        case SchemaElementDecl::Simple:
            return CONTENTTYPE_SIMPLE;
        case SchemaElementDecl::Empty:
        case SchemaElementDecl::ElementOnlyEmpty:
            return CONTENTTYPE_EMPTY;
        case SchemaElementDecl::Children:        
            return CONTENTTYPE_ELEMENT;
        default:
            //case SchemaElementDecl::Mixed_Complex:
            //case SchemaElementDecl::Mixed_Simple:
            //case SchemaElementDecl::Any:
            return CONTENTTYPE_MIXED;
    }
}

bool XSComplexTypeDefinition::isProhibitedSubstitution(XSConstants::DERIVATION_TYPE toTest)                                                     
{
    if (fProhibitedSubstitution & toTest)
        return true;

    return false;
}

XSAnnotationList *XSComplexTypeDefinition::getAnnotations()
{    
    return fXSAnnotationList;
}

// ---------------------------------------------------------------------------
//  XSComplexTypeDefinition: virtual methods
// ---------------------------------------------------------------------------
const XMLCh *XSComplexTypeDefinition::getName() const
{
    return fComplexTypeInfo->getTypeLocalName();
}

const XMLCh *XSComplexTypeDefinition::getNamespace() 
{
    return fComplexTypeInfo->getTypeUri();
}

XSNamespaceItem *XSComplexTypeDefinition::getNamespaceItem() 
{
    return fXSModel->getNamespaceItem(getNamespace());
}

bool XSComplexTypeDefinition::getAnonymous() const
{
    return fComplexTypeInfo->getAnonymous(); 
}

XSTypeDefinition *XSComplexTypeDefinition::getBaseType() 
{
    return fBaseType;
}

bool XSComplexTypeDefinition::derivedFromType(const XSTypeDefinition * const ancestorType)
{
    if (!ancestorType)
        return false;

    XSTypeDefinition* type = (XSTypeDefinition*) ancestorType;

    if (ancestorType == type->getBaseType())
    {
        // ancestor is anytype
        return true;
    }

    type = this;
    XSTypeDefinition* lastType = 0;  // anytype has a basetype of anytype so will have infinite loop...

    while (type && (type != ancestorType) && (type != lastType))
    {
        lastType = type;
        type = type->getBaseType();
    }

    return (type == ancestorType);
}

XERCES_CPP_NAMESPACE_END
