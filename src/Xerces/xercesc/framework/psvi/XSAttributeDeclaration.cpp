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
 * $Id: XSAttributeDeclaration.cpp 471747 2006-11-06 14:31:56Z amassari $
 */

#include <xercesc/framework/psvi/XSAttributeDeclaration.hpp>
#include <xercesc/framework/psvi/XSModel.hpp>
#include <xercesc/framework/psvi/XSNamespaceItem.hpp>
#include <xercesc/framework/psvi/XSAnnotation.hpp>
#include <xercesc/util/StringPool.hpp>
#include <xercesc/validators/schema/SchemaGrammar.hpp>
#include <xercesc/validators/schema/SchemaAttDef.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  XSAttributeDeclaration: Constructors and Destructor
// ---------------------------------------------------------------------------
XSAttributeDeclaration::XSAttributeDeclaration(SchemaAttDef* const           attDef,
                                               XSSimpleTypeDefinition* const typeDef,
                                               XSAnnotation* const           annot,
                                               XSModel* const                xsModel,
                                               XSConstants::SCOPE            scope,
                                               XSComplexTypeDefinition*      enclosingCTDefinition,
                                               MemoryManager * const         manager)
    : XSObject(XSConstants::ATTRIBUTE_DECLARATION, xsModel, manager)
    , fAttDef(attDef)
    , fTypeDefinition(typeDef)
    , fAnnotation(annot) 
    , fScope(scope)
    , fEnclosingCTDefinition(enclosingCTDefinition)        
{
}

XSAttributeDeclaration::~XSAttributeDeclaration() 
{
    // don't delete fTypeDefinition - deleted by XSModel
}

// ---------------------------------------------------------------------------
//  XSAttributeDeclaration: XSObject virtual methods
// ---------------------------------------------------------------------------
const XMLCh *XSAttributeDeclaration::getName() const
{
    return fAttDef->getAttName()->getLocalPart();
}

const XMLCh *XSAttributeDeclaration::getNamespace() 
{
    return fXSModel->getURIStringPool()->getValueForId(fAttDef->getAttName()->getURI());
}

XSNamespaceItem *XSAttributeDeclaration::getNamespaceItem() 
{
    return fXSModel->getNamespaceItem(getNamespace());
}

// ---------------------------------------------------------------------------
//  XSAttributeDeclaration: access methods
// ---------------------------------------------------------------------------

XSConstants::VALUE_CONSTRAINT XSAttributeDeclaration::getConstraintType() const
{
    if (fScope != XSConstants::SCOPE_GLOBAL)
        return XSConstants::VALUE_CONSTRAINT_NONE;

    if (fAttDef->getDefaultType() == XMLAttDef::Default)
        return XSConstants::VALUE_CONSTRAINT_DEFAULT;

    if ((fAttDef->getDefaultType() == XMLAttDef::Fixed) ||
        (fAttDef->getDefaultType() == XMLAttDef::Required_And_Fixed))
        return XSConstants::VALUE_CONSTRAINT_FIXED;

    return XSConstants::VALUE_CONSTRAINT_NONE;
}

const XMLCh *XSAttributeDeclaration::getConstraintValue()
{
    if (fScope == XSConstants::SCOPE_GLOBAL)
        return fAttDef->getValue();

    return 0;
}

bool XSAttributeDeclaration::getRequired() const
{
    if (fAttDef->getDefaultType() == XMLAttDef::Required ||
        fAttDef->getDefaultType() == XMLAttDef::Required_And_Fixed)
        return true;

    return false;
}

XERCES_CPP_NAMESPACE_END


