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
 * $Id: DOMEntityImpl.cpp 678381 2008-07-21 10:15:01Z borisk $
 */

#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOMNode.hpp>
#include <xercesc/dom/DOMEntityReference.hpp>
#include "DOMEntityImpl.hpp"
#include "DOMDocumentImpl.hpp"

XERCES_CPP_NAMESPACE_BEGIN

DOMEntityImpl::DOMEntityImpl(DOMDocument *ownerDoc, const XMLCh *eName)
   : fNode(ownerDoc),
     fParent(ownerDoc),
     fPublicId(0),
     fSystemId(0),
     fNotationName(0),
     fRefEntity(0),
     fInputEncoding(0),
     fXmlEncoding(0),
     fXmlVersion(0),
     fBaseURI(0),
     fEntityRefNodeCloned(false)
{
    fName        = ((DOMDocumentImpl *)ownerDoc)->getPooledString(eName);
    fNode.setReadOnly(true, true);
}


DOMEntityImpl::DOMEntityImpl(const DOMEntityImpl &other, bool deep)
    : DOMEntity(other),
      fNode(other.fNode),
      fParent(other.fParent),
      fName(other.fName),
      fPublicId(other.fPublicId),
      fSystemId(other.fSystemId),
      fNotationName(other.fNotationName),
      fRefEntity(other.fRefEntity),
      fInputEncoding(other.fInputEncoding),
      fXmlEncoding(other.fXmlEncoding),
      fXmlVersion(other.fXmlVersion),
      fBaseURI(other.fBaseURI),
      fEntityRefNodeCloned(false)
{
    if (deep)
        fParent.cloneChildren(&other);
    fNode.setReadOnly(true, true);
}


DOMEntityImpl::~DOMEntityImpl() {
}


DOMNode *DOMEntityImpl::cloneNode(bool deep) const
{
    DOMNode* newNode = new (fParent.fOwnerDocument, DOMMemoryManager::ENTITY_OBJECT) DOMEntityImpl(*this, deep);
    fNode.callUserDataHandlers(DOMUserDataHandler::NODE_CLONED, this, newNode);
    return newNode;
}


const XMLCh * DOMEntityImpl::getNodeName() const {
    return fName;
}


DOMNode::NodeType DOMEntityImpl::getNodeType() const {
    return DOMNode::ENTITY_NODE;
}


const XMLCh * DOMEntityImpl::getNotationName() const
{
    return fNotationName;
}


const XMLCh * DOMEntityImpl::getPublicId() const {
    return fPublicId;
}


const XMLCh * DOMEntityImpl::getSystemId() const
{
    return fSystemId;
}


const XMLCh* DOMEntityImpl::getBaseURI() const
{
    return fBaseURI;
}


void DOMEntityImpl::setNodeValue(const XMLCh *arg)
{
    fNode.setNodeValue(arg);
}


void DOMEntityImpl::setNotationName(const XMLCh *arg)
{
    DOMDocumentImpl *doc = (DOMDocumentImpl *)fParent.fOwnerDocument;
    fNotationName = doc->cloneString(arg);
}


void DOMEntityImpl::setPublicId(const XMLCh *arg)
{
    DOMDocumentImpl *doc = (DOMDocumentImpl *)fParent.fOwnerDocument;
    fPublicId = doc->cloneString(arg);
}


void DOMEntityImpl::setSystemId(const XMLCh *arg)
{
    DOMDocumentImpl *doc = (DOMDocumentImpl *)fParent.fOwnerDocument;
    fSystemId = doc->cloneString(arg);
}


void DOMEntityImpl::setBaseURI(const XMLCh* baseURI) {
    if (baseURI && *baseURI) {
        XMLCh* temp = (XMLCh*) ((DOMDocumentImpl *)fParent.fOwnerDocument)->allocate((XMLString::stringLen(baseURI) + 9)*sizeof(XMLCh));
        XMLString::fixURI(baseURI, temp);
        fBaseURI = temp;
    }
    else
        fBaseURI = 0;
}


void   DOMEntityImpl::setEntityRef(DOMEntityReference* other)
{
    fRefEntity = other;
}


DOMEntityReference*  DOMEntityImpl::getEntityRef() const
{
    return fRefEntity;
}

void  DOMEntityImpl::cloneEntityRefTree() const
{
    if (fEntityRefNodeCloned)
        return;

    // cast off const.  This method is const because it is
    //   called from a bunch of logically const methods, like
    //   getFirstChild().
    DOMEntityImpl *ncThis = (DOMEntityImpl *)this;

    //lazily clone the entityRef tree to this entity
    if (fParent.fFirstChild != 0)
        return;

    if (!fRefEntity)
        return;

    ncThis->fEntityRefNodeCloned = true;
    ncThis->fNode.setReadOnly(false, true);
    ncThis->fParent.cloneChildren(fRefEntity);
    ncThis->fNode.setReadOnly(true, true);
}

DOMNode * DOMEntityImpl::getFirstChild() const
{
    cloneEntityRefTree();
    return fParent.fFirstChild;
}

DOMNode *   DOMEntityImpl::getLastChild() const
{
    cloneEntityRefTree();
    return fParent.getLastChild();
}

DOMNodeList* DOMEntityImpl::getChildNodes() const
{
    cloneEntityRefTree();
    return this->fParent.getChildNodes();

}

bool DOMEntityImpl::hasChildNodes() const
{
    cloneEntityRefTree();
    return fParent.fFirstChild!=0;
}


void DOMEntityImpl::release()
{
    if (fNode.isOwned() && !fNode.isToBeReleased())
        throw DOMException(DOMException::INVALID_ACCESS_ERR,0, GetDOMNodeMemoryManager);

    DOMDocumentImpl* doc = (DOMDocumentImpl*) fParent.fOwnerDocument;
    if (doc) {
        fNode.callUserDataHandlers(DOMUserDataHandler::NODE_DELETED, 0, 0);
        fParent.release();
        doc->release(this, DOMMemoryManager::ENTITY_OBJECT);
    }
    else {
        // shouldn't reach here
        throw DOMException(DOMException::INVALID_ACCESS_ERR,0, GetDOMNodeMemoryManager);
    }
}

//
//  Functions inherited from Node
//

           DOMNode*         DOMEntityImpl::appendChild(DOMNode *newChild)          {cloneEntityRefTree(); return fParent.appendChild (newChild); }
           DOMNamedNodeMap* DOMEntityImpl::getAttributes() const                   {return fNode.getAttributes (); }
     const XMLCh*           DOMEntityImpl::getLocalName() const                    {return fNode.getLocalName (); }
     const XMLCh*           DOMEntityImpl::getNamespaceURI() const                 {return fNode.getNamespaceURI (); }
           DOMNode*         DOMEntityImpl::getNextSibling() const                  {return fNode.getNextSibling (); }
     const XMLCh*           DOMEntityImpl::getNodeValue() const                    {return fNode.getNodeValue (); }
           DOMDocument*     DOMEntityImpl::getOwnerDocument() const                {return fParent.fOwnerDocument; }
     const XMLCh*           DOMEntityImpl::getPrefix() const                       {return fNode.getPrefix (); }
           DOMNode*         DOMEntityImpl::getParentNode() const                   {return fNode.getParentNode (); }
           DOMNode*         DOMEntityImpl::getPreviousSibling() const              {return fNode.getPreviousSibling (); }
           DOMNode*         DOMEntityImpl::insertBefore(DOMNode *newChild, DOMNode *refChild)
                                                                                   {cloneEntityRefTree(); return fParent.insertBefore (newChild, refChild); }
           void             DOMEntityImpl::normalize()                             {cloneEntityRefTree(); fParent.normalize (); }
           DOMNode*         DOMEntityImpl::removeChild(DOMNode *oldChild)          {cloneEntityRefTree(); return fParent.removeChild (oldChild); }
           DOMNode*         DOMEntityImpl::replaceChild(DOMNode *newChild, DOMNode *oldChild)
                                                                                   {cloneEntityRefTree(); return fParent.replaceChild (newChild, oldChild); }
           bool             DOMEntityImpl::isSupported(const XMLCh *feature, const XMLCh *version) const
                                                                                   {return fNode.isSupported (feature, version); }
           void             DOMEntityImpl::setPrefix(const XMLCh  *prefix)         {fNode.setPrefix(prefix); }
           bool             DOMEntityImpl::hasAttributes() const                   {return fNode.hasAttributes(); }
           bool             DOMEntityImpl::isSameNode(const DOMNode* other) const  {return fNode.isSameNode(other); }
           bool             DOMEntityImpl::isEqualNode(const DOMNode* arg) const   {cloneEntityRefTree(); return fParent.isEqualNode(arg); }
           void*            DOMEntityImpl::setUserData(const XMLCh* key, void* data, DOMUserDataHandler* handler)
                                                                                   {return fNode.setUserData(key, data, handler); }
           void*            DOMEntityImpl::getUserData(const XMLCh* key) const     {return fNode.getUserData(key); }
           short            DOMEntityImpl::compareDocumentPosition(const DOMNode* other) const {return fNode.compareDocumentPosition(other); }
           const XMLCh*     DOMEntityImpl::getTextContent() const                  {return fNode.getTextContent(); }
           void             DOMEntityImpl::setTextContent(const XMLCh* textContent){fNode.setTextContent(textContent); }
           const XMLCh*     DOMEntityImpl::lookupPrefix(const XMLCh* namespaceURI) const  {return fNode.lookupPrefix(namespaceURI); }
           bool             DOMEntityImpl::isDefaultNamespace(const XMLCh* namespaceURI) const {return fNode.isDefaultNamespace(namespaceURI); }
           const XMLCh*     DOMEntityImpl::lookupNamespaceURI(const XMLCh* prefix) const  {return fNode.lookupNamespaceURI(prefix); }
           void*            DOMEntityImpl::getFeature(const XMLCh* feature, const XMLCh* version) const {return fNode.getFeature(feature, version); }


//Introduced in DOM Level 3
const XMLCh* DOMEntityImpl::getInputEncoding() const {
    return fInputEncoding;
}

void DOMEntityImpl::setInputEncoding(const XMLCh* actualEncoding){
    DOMDocumentImpl *doc = (DOMDocumentImpl *)fParent.fOwnerDocument;
    fInputEncoding = doc->cloneString(actualEncoding);
}

const XMLCh* DOMEntityImpl::getXmlEncoding() const {
    return fXmlEncoding;
}

void DOMEntityImpl::setXmlEncoding(const XMLCh* encoding){
    DOMDocumentImpl *doc = (DOMDocumentImpl *)fParent.fOwnerDocument;
    fXmlEncoding = doc->cloneString(encoding);
}

const XMLCh* DOMEntityImpl::getXmlVersion() const {
    return fXmlVersion;
}

void DOMEntityImpl::setXmlVersion(const XMLCh* version){
    DOMDocumentImpl *doc = (DOMDocumentImpl *)fParent.fOwnerDocument;
    fXmlVersion = doc->cloneString(version);
}

XERCES_CPP_NAMESPACE_END
