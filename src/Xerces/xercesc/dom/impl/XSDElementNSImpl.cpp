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
 * $Id: XSDElementNSImpl.cpp 678381 2008-07-21 10:15:01Z borisk $
 */
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/dom/DOMException.hpp>

#include "DOMDocumentImpl.hpp"
#include "XSDElementNSImpl.hpp"

XERCES_CPP_NAMESPACE_BEGIN


XSDElementNSImpl::XSDElementNSImpl(DOMDocument *ownerDoc, const XMLCh *nam) :
    DOMElementNSImpl(ownerDoc, nam)
    , fLineNo(0)
    , fColumnNo(0)
{
}

//Introduced in DOM Level 2
XSDElementNSImpl::XSDElementNSImpl(DOMDocument *ownerDoc,
                                   const XMLCh *namespaceURI,
                                   const XMLCh *qualifiedName,
                                   const XMLFileLoc lineNo,
                                   const XMLFileLoc columnNo) :
    DOMElementNSImpl(ownerDoc, namespaceURI, qualifiedName)
    , fLineNo(lineNo)
    , fColumnNo(columnNo)
{
}

XSDElementNSImpl::XSDElementNSImpl(const XSDElementNSImpl &other, bool deep) :
    DOMElementNSImpl(other, deep)
{
    this->fLineNo = other.fLineNo;
    this->fColumnNo =other.fColumnNo;
}

DOMNode * XSDElementNSImpl::cloneNode(bool deep) const {
    DOMNode* newNode = new (fParent.fOwnerDocument) XSDElementNSImpl(*this, deep);
    fNode.callUserDataHandlers(DOMUserDataHandler::NODE_CLONED, this, newNode);
    return newNode;
}


XERCES_CPP_NAMESPACE_END
