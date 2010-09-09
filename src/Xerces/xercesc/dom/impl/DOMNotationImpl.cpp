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
 * $Id: DOMNotationImpl.cpp 671894 2008-06-26 13:29:21Z borisk $
 */

#include "DOMDocumentImpl.hpp"
#include "DOMNotationImpl.hpp"
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOMNode.hpp>

XERCES_CPP_NAMESPACE_BEGIN

DOMNotationImpl::DOMNotationImpl(DOMDocument *ownerDoc, const XMLCh *nName)
    : fNode(ownerDoc), fName(0), fPublicId(0), fSystemId(0), fBaseURI(0)
{
    fNode.setIsLeafNode(true);
    fName = ((DOMDocumentImpl *)ownerDoc)->getPooledString(nName);
}

DOMNotationImpl::DOMNotationImpl(const DOMNotationImpl &other, bool /*deep*/)
    : DOMNotation(other),
      fNode(other.fNode),
      fName(other.fName),
      fPublicId(other.fPublicId),
      fSystemId(other.fSystemId),
      fBaseURI(other.fBaseURI)
{
    fNode.setIsLeafNode(true);
}


DOMNotationImpl::~DOMNotationImpl()
{
}


DOMNode *DOMNotationImpl::cloneNode(bool deep) const
{
    DOMNode* newNode = new (getOwnerDocument(), DOMMemoryManager::NOTATION_OBJECT) DOMNotationImpl(*this, deep);
    fNode.callUserDataHandlers(DOMUserDataHandler::NODE_CLONED, this, newNode);
    return newNode;
}


const XMLCh * DOMNotationImpl::getNodeName() const {
    return fName;
}


DOMNode::NodeType DOMNotationImpl::getNodeType() const {
    return DOMNode::NOTATION_NODE;
}



const XMLCh * DOMNotationImpl::getPublicId() const
{
    return fPublicId;
}


const XMLCh * DOMNotationImpl::getSystemId() const
{
    return fSystemId;
}


void DOMNotationImpl::setNodeValue(const XMLCh *arg)
{
    fNode.setNodeValue(arg);
}


void DOMNotationImpl::setPublicId(const XMLCh *arg)
{
    if(fNode.isReadOnly())
        throw DOMException(
        DOMException::NO_MODIFICATION_ALLOWED_ERR,0, GetDOMNodeMemoryManager);

    fPublicId = ((DOMDocumentImpl *)getOwnerDocument())->cloneString(arg);
}


void DOMNotationImpl::setSystemId(const XMLCh *arg)
{
    if(fNode.isReadOnly())
        throw DOMException(
        DOMException::NO_MODIFICATION_ALLOWED_ERR,0, GetDOMNodeMemoryManager);

    fSystemId = ((DOMDocumentImpl *)getOwnerDocument())->cloneString(arg);
}

void DOMNotationImpl::release()
{
    if (fNode.isOwned() && !fNode.isToBeReleased())
        throw DOMException(DOMException::INVALID_ACCESS_ERR,0, GetDOMNodeMemoryManager);

    DOMDocumentImpl* doc = (DOMDocumentImpl*) getOwnerDocument();
    if (doc) {
        fNode.callUserDataHandlers(DOMUserDataHandler::NODE_DELETED, 0, 0);
        doc->release(this, DOMMemoryManager::NOTATION_OBJECT);
    }
    else {
        // shouldn't reach here
        throw DOMException(DOMException::INVALID_ACCESS_ERR,0, GetDOMNodeMemoryManager);
    }
}

void DOMNotationImpl::setBaseURI(const XMLCh* baseURI) {
    if (baseURI && *baseURI) {
        XMLCh* temp = (XMLCh*) ((DOMDocumentImpl *)getOwnerDocument())->allocate((XMLString::stringLen(baseURI) + 9)*sizeof(XMLCh));
        XMLString::fixURI(baseURI, temp);
        fBaseURI = temp;
    }
    else
        fBaseURI = 0;
}

const XMLCh* DOMNotationImpl::getBaseURI() const
{
    return fBaseURI;
}


           DOMNode*         DOMNotationImpl::appendChild(DOMNode *newChild)          {return fNode.appendChild (newChild); }
           DOMNamedNodeMap* DOMNotationImpl::getAttributes() const                   {return fNode.getAttributes (); }
           DOMNodeList*     DOMNotationImpl::getChildNodes() const                   {return fNode.getChildNodes (); }
           DOMNode*         DOMNotationImpl::getFirstChild() const                   {return fNode.getFirstChild (); }
           DOMNode*         DOMNotationImpl::getLastChild() const                    {return fNode.getLastChild (); }
     const XMLCh*           DOMNotationImpl::getLocalName() const                    {return fNode.getLocalName (); }
     const XMLCh*           DOMNotationImpl::getNamespaceURI() const                 {return fNode.getNamespaceURI (); }
           DOMNode*         DOMNotationImpl::getNextSibling() const                  {return fNode.getNextSibling (); }
     const XMLCh*           DOMNotationImpl::getNodeValue() const                    {return fNode.getNodeValue (); }
           DOMDocument*     DOMNotationImpl::getOwnerDocument() const                {return fNode.getOwnerDocument (); }
     const XMLCh*           DOMNotationImpl::getPrefix() const                       {return fNode.getPrefix (); }
           DOMNode*         DOMNotationImpl::getParentNode() const                   {return fNode.getParentNode (); }
           DOMNode*         DOMNotationImpl::getPreviousSibling() const              {return fNode.getPreviousSibling (); }
           bool             DOMNotationImpl::hasChildNodes() const                   {return fNode.hasChildNodes (); }
           DOMNode*         DOMNotationImpl::insertBefore(DOMNode *newChild, DOMNode *refChild)
                                                                                     {return fNode.insertBefore (newChild, refChild); }
           void             DOMNotationImpl::normalize()                             {fNode.normalize (); }
           DOMNode*         DOMNotationImpl::removeChild(DOMNode *oldChild)          {return fNode.removeChild (oldChild); }
           DOMNode*         DOMNotationImpl::replaceChild(DOMNode *newChild, DOMNode *oldChild)
                                                                                     {return fNode.replaceChild (newChild, oldChild); }
           bool             DOMNotationImpl::isSupported(const XMLCh *feature, const XMLCh *version) const
                                                                                     {return fNode.isSupported (feature, version); }
           void             DOMNotationImpl::setPrefix(const XMLCh  *prefix)         {fNode.setPrefix(prefix); }
           bool             DOMNotationImpl::hasAttributes() const                   {return fNode.hasAttributes(); }
           bool             DOMNotationImpl::isSameNode(const DOMNode* other) const  {return fNode.isSameNode(other); }
           bool             DOMNotationImpl::isEqualNode(const DOMNode* arg) const   {return fNode.isEqualNode(arg); }
           void*            DOMNotationImpl::setUserData(const XMLCh* key, void* data, DOMUserDataHandler* handler)
                                                                                     {return fNode.setUserData(key, data, handler); }
           void*            DOMNotationImpl::getUserData(const XMLCh* key) const     {return fNode.getUserData(key); }
           short            DOMNotationImpl::compareDocumentPosition(const DOMNode* other) const {return fNode.compareDocumentPosition(other); }
           const XMLCh*     DOMNotationImpl::getTextContent() const                  {return fNode.getTextContent(); }
           void             DOMNotationImpl::setTextContent(const XMLCh* textContent){fNode.setTextContent(textContent); }
           const XMLCh*     DOMNotationImpl::lookupPrefix(const XMLCh* namespaceURI) const  {return fNode.lookupPrefix(namespaceURI); }
           bool             DOMNotationImpl::isDefaultNamespace(const XMLCh* namespaceURI) const {return fNode.isDefaultNamespace(namespaceURI); }
           const XMLCh*     DOMNotationImpl::lookupNamespaceURI(const XMLCh* prefix) const  {return fNode.lookupNamespaceURI(prefix); }
           void*            DOMNotationImpl::getFeature(const XMLCh* feature, const XMLCh* version) const {return fNode.getFeature(feature, version); }


XERCES_CPP_NAMESPACE_END
