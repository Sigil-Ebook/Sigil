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
 * $Id: XSNotationDeclaration.cpp 471747 2006-11-06 14:31:56Z amassari $
 */

#include <xercesc/framework/psvi/XSNotationDeclaration.hpp>
#include <xercesc/framework/psvi/XSModel.hpp>
#include <xercesc/framework/XMLNotationDecl.hpp>
#include <xercesc/framework/psvi/XSModel.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  XSNotationDeclaration: Constructors and Destructors
// ---------------------------------------------------------------------------
XSNotationDeclaration::XSNotationDeclaration
(
    XMLNotationDecl* const  xmlNotationDecl
    , XSAnnotation* const   annot
    , XSModel* const        xsModel
    , MemoryManager * const manager
)
    : XSObject(XSConstants::NOTATION_DECLARATION, xsModel, manager)
    , fXMLNotationDecl(xmlNotationDecl)
    , fAnnotation(annot)
{
}

XSNotationDeclaration::~XSNotationDeclaration()
{
}

// ---------------------------------------------------------------------------
//  XSNotationDeclaration: XSModel virtual methods
// ---------------------------------------------------------------------------
const XMLCh *XSNotationDeclaration::getName() const
{
    return fXMLNotationDecl->getName();
}

const XMLCh *XSNotationDeclaration::getNamespace() 
{
    return fXSModel->getURIStringPool()->getValueForId(fXMLNotationDecl->getNameSpaceId());
}

XSNamespaceItem *XSNotationDeclaration::getNamespaceItem() 
{
    return fXSModel->getNamespaceItem(getNamespace());
}


// ---------------------------------------------------------------------------
//  XSNotationDeclaration: access methods
// ---------------------------------------------------------------------------
const XMLCh *XSNotationDeclaration::getSystemId()
{
    return fXMLNotationDecl->getSystemId();
}

const XMLCh *XSNotationDeclaration::getPublicId()
{
   return fXMLNotationDecl->getPublicId();
}

XERCES_CPP_NAMESPACE_END


