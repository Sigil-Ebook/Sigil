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
 * $Id: XSObject.cpp 674012 2008-07-04 11:18:21Z borisk $
 */

#include <xercesc/framework/psvi/XSObject.hpp>
#include <xercesc/framework/psvi/XSModel.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  XSObject: Constructors and Destructor
// ---------------------------------------------------------------------------
XSObject::XSObject(XSConstants::COMPONENT_TYPE compType,
                   XSModel* const xsModel,
                   MemoryManager* const manager)
    : fComponentType(compType)
    , fXSModel(xsModel)
    , fMemoryManager(manager)
    , fId(0)
{
    if (xsModel)
    {
        xsModel->addComponentToIdVector(this, compType-1);
    }
}

XSObject::~XSObject()
{
}

// ---------------------------------------------------------------------------
//  XSObject: Virtual interface methods
// ---------------------------------------------------------------------------
const XMLCh *XSObject::getName() const
{
    return 0;
}

const XMLCh *XSObject::getNamespace()
{
    return 0;
}

XSNamespaceItem *XSObject::getNamespaceItem()
{
    return 0;
}

XMLSize_t XSObject::getId() const
{
    return fId;
}

void XSObject::setId(XMLSize_t id)
{
    fId = id;
}

XERCES_CPP_NAMESPACE_END
