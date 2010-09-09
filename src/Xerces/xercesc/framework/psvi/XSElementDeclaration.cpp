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
 * $Id: XSElementDeclaration.cpp 471747 2006-11-06 14:31:56Z amassari $
 */

#include <xercesc/framework/psvi/XSElementDeclaration.hpp>
#include <xercesc/framework/psvi/XSSimpleTypeDefinition.hpp>
#include <xercesc/framework/psvi/XSComplexTypeDefinition.hpp>
#include <xercesc/framework/psvi/XSIDCDefinition.hpp>
#include <xercesc/framework/psvi/XSModel.hpp>
#include <xercesc/validators/schema/SchemaElementDecl.hpp>
#include <xercesc/util/StringPool.hpp>

XERCES_CPP_NAMESPACE_BEGIN
   
// ---------------------------------------------------------------------------
//  XSElementDeclaration: Constructors and Destructor
// ---------------------------------------------------------------------------
XSElementDeclaration::XSElementDeclaration
(
    SchemaElementDecl* const             schemaElementDecl
    , XSTypeDefinition* const            typeDefinition
    , XSElementDeclaration* const        substitutionGroupAffiliation
    , XSAnnotation* const                annot
    , XSNamedMap<XSIDCDefinition>* const identityConstraints
    , XSModel* const                     xsModel
    , XSConstants::SCOPE                 elemScope
    , XSComplexTypeDefinition* const     enclosingTypeDefinition
    , MemoryManager* const               manager
)
    : XSObject(XSConstants::ELEMENT_DECLARATION, xsModel, manager)
    , fDisallowedSubstitutions(0)
    , fSubstitutionGroupExclusions(0)
    , fScope(elemScope)
    , fSchemaElementDecl(schemaElementDecl)
    , fTypeDefinition(typeDefinition)
    , fEnclosingTypeDefinition(enclosingTypeDefinition)
    , fSubstitutionGroupAffiliation(substitutionGroupAffiliation)
    , fAnnotation(annot)
    , fIdentityConstraints(identityConstraints)
{
    // set block and final information
    // NOTE: rest of setup will be taken care of in construct()
    int blockFinalSet = fSchemaElementDecl->getBlockSet();
    if (blockFinalSet) 
    {
        if (blockFinalSet & SchemaSymbols::XSD_EXTENSION)
            fDisallowedSubstitutions |= XSConstants::DERIVATION_EXTENSION;

        if (blockFinalSet & SchemaSymbols::XSD_RESTRICTION)
            fDisallowedSubstitutions |= XSConstants::DERIVATION_RESTRICTION;

        if (blockFinalSet & SchemaSymbols::XSD_SUBSTITUTION)
            fDisallowedSubstitutions |= XSConstants::DERIVATION_SUBSTITUTION;
    }
    
    if (0 != (blockFinalSet = fSchemaElementDecl->getFinalSet()))
    {
        if (blockFinalSet & SchemaSymbols::XSD_EXTENSION)
            fSubstitutionGroupExclusions |= XSConstants::DERIVATION_EXTENSION;

        if (blockFinalSet & SchemaSymbols::XSD_RESTRICTION)
            fSubstitutionGroupExclusions |= XSConstants::DERIVATION_RESTRICTION;
    }
}

XSElementDeclaration::~XSElementDeclaration() 
{
    // don't delete fTypeDefinition - deleted by XSModel
    // don't delete fSubstitutionGroupAffiliation - deleted by XSModel
    if (fIdentityConstraints)
        delete fIdentityConstraints;
}

// ---------------------------------------------------------------------------
//  XSElementDeclaration: XSObject virtual methods
// ---------------------------------------------------------------------------
const XMLCh *XSElementDeclaration::getName() const
{
    return fSchemaElementDecl->getElementName()->getLocalPart();
}

const XMLCh *XSElementDeclaration::getNamespace() 
{
    return fXSModel->getURIStringPool()->getValueForId(fSchemaElementDecl->getURI());
}

XSNamespaceItem *XSElementDeclaration::getNamespaceItem() 
{
    return fXSModel->getNamespaceItem(getNamespace());
}


// ---------------------------------------------------------------------------
//  XSElementDeclaration: access methods
// ---------------------------------------------------------------------------
XSConstants::VALUE_CONSTRAINT XSElementDeclaration::getConstraintType() const
{
    if (fSchemaElementDecl->getMiscFlags() & SchemaSymbols::XSD_FIXED)
        return XSConstants::VALUE_CONSTRAINT_FIXED;

    if (fSchemaElementDecl->getDefaultValue())
        return XSConstants::VALUE_CONSTRAINT_DEFAULT;

    return XSConstants::VALUE_CONSTRAINT_NONE;
}

const XMLCh *XSElementDeclaration::getConstraintValue()
{
    return fSchemaElementDecl->getDefaultValue();
}

bool XSElementDeclaration::getNillable() const
{
    if (fSchemaElementDecl->getMiscFlags() & SchemaSymbols::XSD_NILLABLE)
        return true;

    return false;
}

bool XSElementDeclaration::isSubstitutionGroupExclusion(XSConstants::DERIVATION_TYPE exclusion)
{
    if (fSubstitutionGroupExclusions & exclusion)
        return true;

    return false;
}


bool XSElementDeclaration::isDisallowedSubstitution(XSConstants::DERIVATION_TYPE disallowed)
{
    if (fDisallowedSubstitutions & disallowed)
        return true;

    return false;
}


bool XSElementDeclaration::getAbstract() const
{
    if (fSchemaElementDecl->getMiscFlags() & SchemaSymbols::XSD_ABSTRACT)
        return true;

    return false;
}

XERCES_CPP_NAMESPACE_END
