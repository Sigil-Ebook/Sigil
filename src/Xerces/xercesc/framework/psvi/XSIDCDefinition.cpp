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
 * $Id: XSIDCDefinition.cpp 471747 2006-11-06 14:31:56Z amassari $
 */

#include <xercesc/framework/psvi/XSIDCDefinition.hpp>
#include <xercesc/validators/schema/identity/IC_KeyRef.hpp>
#include <xercesc/validators/schema/identity/IC_Selector.hpp>
#include <xercesc/validators/schema/identity/XercesXPath.hpp>
#include <xercesc/framework/psvi/XSModel.hpp>
#include <xercesc/framework/psvi/XSAnnotation.hpp>
#include <xercesc/util/StringPool.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  XSIDCDefinition: Constructors and Destructor
// ---------------------------------------------------------------------------
XSIDCDefinition::XSIDCDefinition(IdentityConstraint* const identityConstraint,
                                 XSIDCDefinition*  const   keyIC,
                                 XSAnnotation* const       headAnnot,
                                 StringList* const         stringList,
                                 XSModel* const            xsModel,
                                 MemoryManager* const      manager)
    : XSObject(XSConstants::IDENTITY_CONSTRAINT, xsModel, manager)
    , fIdentityConstraint(identityConstraint)
    , fKey(keyIC)
    , fStringList(stringList)
    , fXSAnnotationList(0)
{
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

XSIDCDefinition::~XSIDCDefinition()
{
    if (fStringList)
        delete fStringList;

    // don't delete fKey - deleted by XSModel
    if (fXSAnnotationList)
        delete fXSAnnotationList;
}

// ---------------------------------------------------------------------------
//  XSIDCDefinition: XSObject virtual methods
// ---------------------------------------------------------------------------
const XMLCh *XSIDCDefinition::getName() const
{
    return fIdentityConstraint->getIdentityConstraintName();
}

const XMLCh *XSIDCDefinition::getNamespace() 
{
    return fXSModel->getURIStringPool()->getValueForId(fIdentityConstraint->getNamespaceURI());
}

XSNamespaceItem *XSIDCDefinition::getNamespaceItem() 
{
    return fXSModel->getNamespaceItem(getNamespace());
}

// ---------------------------------------------------------------------------
//  XSIDCDefinition: access methods
// ---------------------------------------------------------------------------
XSIDCDefinition::IC_CATEGORY XSIDCDefinition::getCategory() const
{
    switch(fIdentityConstraint->getType()) {
        case IdentityConstraint::ICType_UNIQUE:
            return IC_UNIQUE;
        case IdentityConstraint::ICType_KEY:
            return IC_KEY;
        case IdentityConstraint::ICType_KEYREF:
            return IC_KEYREF;
        default:
            // REVISIT:
            // should never really get here... IdentityConstraint::Unknown is the other
            // choice so need a default case for completeness; should issues error?
            return IC_KEY;
    }
}

const XMLCh *XSIDCDefinition::getSelectorStr()
{
    return fIdentityConstraint->getSelector()->getXPath()->getExpression();
}


XSAnnotationList *XSIDCDefinition::getAnnotations()
{
    return fXSAnnotationList;
}

XERCES_CPP_NAMESPACE_END


