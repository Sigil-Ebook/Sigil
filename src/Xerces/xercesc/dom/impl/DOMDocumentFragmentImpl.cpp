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
 * $Id: DOMDocumentFragmentImpl.cpp 671894 2008-06-26 13:29:21Z borisk $
 */

#include "DOMDocumentFragmentImpl.hpp"
#include "DOMDocumentImpl.hpp"
#include "DOMCasts.hpp"
#include <xercesc/dom/DOMNode.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/util/XMLUniDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN

DOMDocumentFragmentImpl::DOMDocumentFragmentImpl(DOMDocument *masterDoc)
    : fNode(masterDoc), fParent(masterDoc)
{
}


DOMDocumentFragmentImpl::DOMDocumentFragmentImpl(const DOMDocumentFragmentImpl &other,
                                           bool deep)
    : fNode(other.fNode), fParent(other.fParent)
{
    if (deep)
        castToParentImpl(this)->cloneChildren(&other);
}


DOMDocumentFragmentImpl::~DOMDocumentFragmentImpl()
{
}



DOMNode *DOMDocumentFragmentImpl::cloneNode(bool deep) const
{
    DOMNode* newNode = new (castToNodeImpl(this)->getOwnerDocument(), DOMMemoryManager::DOCUMENT_FRAGMENT_OBJECT) DOMDocumentFragmentImpl(*this, deep);
    fNode.callUserDataHandlers(DOMUserDataHandler::NODE_CLONED, this, newNode);
    return newNode;
}


const XMLCh * DOMDocumentFragmentImpl::getNodeName() const {
    static const XMLCh name[] = {chPound, chLatin_d, chLatin_o, chLatin_c, chLatin_u, chLatin_m,
        chLatin_e, chLatin_n, chLatin_t, chDash,
        chLatin_f, chLatin_r, chLatin_a, chLatin_g, chLatin_m, chLatin_e, chLatin_n, chLatin_t, 0};
    return name;
}


DOMNode::NodeType DOMDocumentFragmentImpl::getNodeType() const {
    return DOMNode::DOCUMENT_FRAGMENT_NODE;
}


void DOMDocumentFragmentImpl::setNodeValue(const XMLCh *x)
{
    fNode.setNodeValue(x);
}


void DOMDocumentFragmentImpl::release()
{
    if (fNode.isOwned() && !fNode.isToBeReleased())
        throw DOMException(DOMException::INVALID_ACCESS_ERR,0, GetDOMNodeMemoryManager);

    DOMDocumentImpl* doc = (DOMDocumentImpl*) getOwnerDocument();
    if (doc) {
        fNode.callUserDataHandlers(DOMUserDataHandler::NODE_DELETED, 0, 0);
        fParent.release();
        doc->release(this, DOMMemoryManager::DOCUMENT_FRAGMENT_OBJECT);
    }
    else {
        // shouldn't reach here
        throw DOMException(DOMException::INVALID_ACCESS_ERR,0, GetDOMNodeMemoryManager);
    }
}

//
//  Delegation stubs for inherited functions.
//
           DOMNode*         DOMDocumentFragmentImpl::appendChild(DOMNode *newChild)          {return fParent.appendChild (newChild); }
           DOMNamedNodeMap* DOMDocumentFragmentImpl::getAttributes() const                   {return fNode.getAttributes (); }
           DOMNodeList*     DOMDocumentFragmentImpl::getChildNodes() const                   {return fParent.getChildNodes (); }
           DOMNode*         DOMDocumentFragmentImpl::getFirstChild() const                   {return fParent.getFirstChild (); }
           DOMNode*         DOMDocumentFragmentImpl::getLastChild() const                    {return fParent.getLastChild (); }
     const XMLCh*           DOMDocumentFragmentImpl::getLocalName() const                    {return fNode.getLocalName (); }
     const XMLCh*           DOMDocumentFragmentImpl::getNamespaceURI() const                 {return fNode.getNamespaceURI (); }
           DOMNode*         DOMDocumentFragmentImpl::getNextSibling() const                  {return fNode.getNextSibling (); }
     const XMLCh*           DOMDocumentFragmentImpl::getNodeValue() const                    {return fNode.getNodeValue (); }
           DOMDocument*     DOMDocumentFragmentImpl::getOwnerDocument() const                {return fParent.fOwnerDocument; }
     const XMLCh*           DOMDocumentFragmentImpl::getPrefix() const                       {return fNode.getPrefix (); }
           DOMNode*         DOMDocumentFragmentImpl::getParentNode() const                   {return fNode.getParentNode (); }
           DOMNode*         DOMDocumentFragmentImpl::getPreviousSibling() const              {return fNode.getPreviousSibling (); }
           bool             DOMDocumentFragmentImpl::hasChildNodes() const                   {return fParent.hasChildNodes (); }
           DOMNode*         DOMDocumentFragmentImpl::insertBefore(DOMNode *newChild, DOMNode *refChild)
                                                                                             {return fParent.insertBefore (newChild, refChild); }
           void             DOMDocumentFragmentImpl::normalize()                             {fParent.normalize (); }
           DOMNode*         DOMDocumentFragmentImpl::removeChild(DOMNode *oldChild)          {return fParent.removeChild (oldChild); }
           DOMNode*         DOMDocumentFragmentImpl::replaceChild(DOMNode *newChild, DOMNode *oldChild)
                                                                                             {return fParent.replaceChild (newChild, oldChild); }
           bool             DOMDocumentFragmentImpl::isSupported(const XMLCh *feature, const XMLCh *version) const
                                                                                             {return fNode.isSupported (feature, version); }
           void             DOMDocumentFragmentImpl::setPrefix(const XMLCh  *prefix)         {fNode.setPrefix(prefix); }
           bool             DOMDocumentFragmentImpl::hasAttributes() const                   {return fNode.hasAttributes(); }
           bool             DOMDocumentFragmentImpl::isSameNode(const DOMNode* other) const  {return fNode.isSameNode(other); }
           bool             DOMDocumentFragmentImpl::isEqualNode(const DOMNode* arg) const   {return fParent.isEqualNode(arg); }
           void*            DOMDocumentFragmentImpl::setUserData(const XMLCh* key, void* data, DOMUserDataHandler* handler)
                                                                                             {return fNode.setUserData(key, data, handler); }
           void*            DOMDocumentFragmentImpl::getUserData(const XMLCh* key) const     {return fNode.getUserData(key); }
           const XMLCh*     DOMDocumentFragmentImpl::getBaseURI() const                      {return fNode.getBaseURI(); }
           short            DOMDocumentFragmentImpl::compareDocumentPosition(const DOMNode* other) const {return fNode.compareDocumentPosition(other); }
           const XMLCh*     DOMDocumentFragmentImpl::getTextContent() const                  {return fNode.getTextContent(); }
           void             DOMDocumentFragmentImpl::setTextContent(const XMLCh* textContent){fNode.setTextContent(textContent); }
           const XMLCh*     DOMDocumentFragmentImpl::lookupPrefix(const XMLCh* namespaceURI) const  {return fNode.lookupPrefix(namespaceURI); }
           bool             DOMDocumentFragmentImpl::isDefaultNamespace(const XMLCh* namespaceURI) const {return fNode.isDefaultNamespace(namespaceURI); }
           const XMLCh*     DOMDocumentFragmentImpl::lookupNamespaceURI(const XMLCh* prefix) const  {return fNode.lookupNamespaceURI(prefix); }
           void*            DOMDocumentFragmentImpl::getFeature(const XMLCh* feature, const XMLCh* version) const {return fNode.getFeature(feature, version); }

XERCES_CPP_NAMESPACE_END
