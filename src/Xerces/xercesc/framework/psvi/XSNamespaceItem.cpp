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
 * $Id: XSNamespaceItem.cpp 674012 2008-07-04 11:18:21Z borisk $
 */

#include <xercesc/framework/psvi/XSNamespaceItem.hpp>
#include <xercesc/validators/schema/SchemaGrammar.hpp>
#include <xercesc/framework/psvi/XSModel.hpp>
#include <xercesc/framework/psvi/XSAnnotation.hpp>
#include <xercesc/validators/schema/XMLSchemaDescriptionImpl.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  XSNamespaceItem: Constructors and Destructors
// ---------------------------------------------------------------------------
XSNamespaceItem::XSNamespaceItem(XSModel* const       xsModel,
                                 SchemaGrammar* const grammar,
                                 MemoryManager* const manager)
    : fMemoryManager(manager)
    , fGrammar(grammar)
    , fXSModel(xsModel)
    , fXSAnnotationList(0)
    , fSchemaNamespace(grammar->getTargetNamespace())
{
    // Populate XSNamedMaps by going through the components
    for (XMLSize_t i=0; i<XSConstants::MULTIVALUE_FACET; i++)
    {
        switch (i+1)
        {
            case XSConstants::ATTRIBUTE_DECLARATION:
            case XSConstants::ELEMENT_DECLARATION:
            case XSConstants::TYPE_DEFINITION:
            case XSConstants::ATTRIBUTE_GROUP_DEFINITION:
            case XSConstants::MODEL_GROUP_DEFINITION:
            case XSConstants::NOTATION_DECLARATION:
                fComponentMap[i] = new (fMemoryManager) XSNamedMap<XSObject>
                (
                    20,     // size
                    29,     // modulus
                    fXSModel->getURIStringPool(),
                    false,  // adoptElems
                    fMemoryManager
                );
                fHashMap[i] = new (fMemoryManager) RefHashTableOf<XSObject>
                (
                    29,
                    false,
                    fMemoryManager
                );
                break;
            default:
                // ATTRIBUTE_USE
                // MODEL_GROUP
                // PARTICLE
                // IDENTITY_CONSTRAINT
                // WILDCARD
                // ANNOTATION
                // FACET
                // MULTIVALUE
                fComponentMap[i] = 0;
                fHashMap[i] = 0;
                break;
        }
    }

    fXSAnnotationList = new (manager) RefVectorOf <XSAnnotation> (5, false, manager);
}

XSNamespaceItem::XSNamespaceItem(XSModel* const       xsModel,
                                 const XMLCh* const   schemaNamespace,
                                 MemoryManager* const manager)
    : fMemoryManager(manager)
    , fGrammar(0)
    , fXSModel(xsModel)
    , fXSAnnotationList(0)
    , fSchemaNamespace(schemaNamespace)
{
    // Populate XSNamedMaps by going through the components
    for (XMLSize_t i=0; i<XSConstants::MULTIVALUE_FACET; i++)
    {
        switch (i+1)
        {
            case XSConstants::ATTRIBUTE_DECLARATION:
            case XSConstants::ELEMENT_DECLARATION:
            case XSConstants::TYPE_DEFINITION:
            case XSConstants::ATTRIBUTE_GROUP_DEFINITION:
            case XSConstants::MODEL_GROUP_DEFINITION:
            case XSConstants::NOTATION_DECLARATION:
                fComponentMap[i] = new (fMemoryManager) XSNamedMap<XSObject>
                (
                    20,     // size
                    29,     // modulus
                    fXSModel->getURIStringPool(),
                    false,  // adoptElems
                    fMemoryManager
                );
                fHashMap[i] = new (fMemoryManager) RefHashTableOf<XSObject>
                (
                    29,
                    false,
                    fMemoryManager
                );
                break;
            default:
                // ATTRIBUTE_USE
                // MODEL_GROUP
                // PARTICLE
                // IDENTITY_CONSTRAINT
                // WILDCARD
                // ANNOTATION
                // FACET
                // MULTIVALUE
                fComponentMap[i] = 0;
                fHashMap[i] = 0;
                break;
        }
    }

    fXSAnnotationList = new (manager) RefVectorOf <XSAnnotation> (5, false, manager);
}

XSNamespaceItem::~XSNamespaceItem()
{
    for (XMLSize_t i=0; i<XSConstants::MULTIVALUE_FACET; i++)
    {
        switch (i+1)
        {
            case XSConstants::ATTRIBUTE_DECLARATION:
            case XSConstants::ELEMENT_DECLARATION:
            case XSConstants::TYPE_DEFINITION:
            case XSConstants::ATTRIBUTE_GROUP_DEFINITION:
            case XSConstants::MODEL_GROUP_DEFINITION:
            case XSConstants::NOTATION_DECLARATION:
                delete fComponentMap[i];
                delete fHashMap[i];
                break;
        }
    }

    delete fXSAnnotationList;
}

// ---------------------------------------------------------------------------
//  XSNamespaceItem: access methods
// ---------------------------------------------------------------------------
XSNamedMap<XSObject> *XSNamespaceItem::getComponents(XSConstants::COMPONENT_TYPE objectType)
{
    return fComponentMap[objectType -1];
}

XSElementDeclaration *XSNamespaceItem::getElementDeclaration(const XMLCh *name)
{
    if (name)
        return (XSElementDeclaration*) fHashMap[XSConstants::ELEMENT_DECLARATION -1]->get(name);
    return 0;
}

XSAttributeDeclaration *XSNamespaceItem::getAttributeDeclaration(const XMLCh *name)
{
    if (name)
        return (XSAttributeDeclaration*) fHashMap[XSConstants::ATTRIBUTE_DECLARATION -1]->get(name);
    return 0;
}

XSTypeDefinition *XSNamespaceItem::getTypeDefinition(const XMLCh *name)
{
    if (name)
        return (XSTypeDefinition*) fHashMap[XSConstants::TYPE_DEFINITION -1]->get(name);
    return 0;
}

XSAttributeGroupDefinition *XSNamespaceItem::getAttributeGroup(const XMLCh *name)
{
    if (name)
        return (XSAttributeGroupDefinition*) fHashMap[XSConstants::ATTRIBUTE_GROUP_DEFINITION -1]->get(name);
    return 0;
}

XSModelGroupDefinition *XSNamespaceItem::getModelGroupDefinition(const XMLCh *name)
{
    if (name)
        return (XSModelGroupDefinition*) fHashMap[XSConstants::MODEL_GROUP_DEFINITION -1]->get(name);
    return 0;
}

XSNotationDeclaration *XSNamespaceItem::getNotationDeclaration(const XMLCh *name)
{
    if (name)
        return (XSNotationDeclaration*) fHashMap[XSConstants::NOTATION_DECLARATION -1]->get(name);
    return 0;
}

const StringList *XSNamespaceItem::getDocumentLocations()
{
    if (fGrammar)
        return ((XMLSchemaDescriptionImpl*) fGrammar->getGrammarDescription())->getLocationHints();

    return 0;
}

XERCES_CPP_NAMESPACE_END
