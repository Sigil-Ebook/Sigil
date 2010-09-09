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
 * $Id: XSAttributeGroupDefinition.cpp 471747 2006-11-06 14:31:56Z amassari $
 */

#include <xercesc/framework/psvi/XSAttributeGroupDefinition.hpp>
#include <xercesc/framework/psvi/XSAttributeUse.hpp>
#include <xercesc/validators/schema/XercesAttGroupInfo.hpp>
#include <xercesc/framework/psvi/XSModel.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  XSAttributeGroupDefinition: Constructors and Destructor
// ---------------------------------------------------------------------------
XSAttributeGroupDefinition::XSAttributeGroupDefinition
(
    XercesAttGroupInfo* const   xercesAttGroupInfo
    , XSAttributeUseList* const xsAttList
    , XSWildcard* const         xsWildcard
    , XSAnnotation* const       xsAnnot
    , XSModel* const            xsModel
    , MemoryManager * const     manager
)
    : XSObject(XSConstants::ATTRIBUTE_GROUP_DEFINITION, xsModel, manager)
    , fXercesAttGroupInfo(xercesAttGroupInfo)
    , fXSAttributeUseList(xsAttList)
    , fXSWildcard(xsWildcard)
    , fAnnotation(xsAnnot)
{
}

XSAttributeGroupDefinition::~XSAttributeGroupDefinition()
{
    if (fXSAttributeUseList)
        delete fXSAttributeUseList;

    // don't delete fXSWildcard - deleted by XSModel
}

// XSObject methods
const XMLCh *XSAttributeGroupDefinition::getName() const
{
    return fXSModel->getURIStringPool()->getValueForId(fXercesAttGroupInfo->getNameId());        
}

const XMLCh *XSAttributeGroupDefinition::getNamespace() 
{
    return fXSModel->getURIStringPool()->getValueForId(fXercesAttGroupInfo->getNamespaceId());
}

XSNamespaceItem *XSAttributeGroupDefinition::getNamespaceItem() 
{
    return fXSModel->getNamespaceItem(getNamespace());
}


XERCES_CPP_NAMESPACE_END


