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
 * $Id: DOMProcessingInstructionImpl.cpp 678381 2008-07-21 10:15:01Z borisk $
 */

#include "DOMProcessingInstructionImpl.hpp"
#include "DOMDocumentImpl.hpp"
#include "DOMNodeImpl.hpp"
#include "DOMStringPool.hpp"
#include "DOMRangeImpl.hpp"

#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOMNode.hpp>

XERCES_CPP_NAMESPACE_BEGIN

DOMProcessingInstructionImpl::DOMProcessingInstructionImpl(DOMDocument *ownerDoc,
                                                     const XMLCh *targt,
                                                     const XMLCh *dat)
    : fNode(ownerDoc), fCharacterData(ownerDoc, dat), fBaseURI(0)
{
    fNode.setIsLeafNode(true);
    this->fTarget = ((DOMDocumentImpl *)ownerDoc)->cloneString(targt);
}


DOMProcessingInstructionImpl::DOMProcessingInstructionImpl(
                                        const DOMProcessingInstructionImpl &other,
                                        bool /*deep*/)
    : DOMProcessingInstruction(other),
      fNode(other.fNode),
      fChild(other.fChild),
      fCharacterData(other.fCharacterData),
      fTarget(other.fTarget),
      fBaseURI(other.fBaseURI)
{
    fNode.setIsLeafNode(true);
}


DOMProcessingInstructionImpl::~DOMProcessingInstructionImpl()
{
}


DOMNode *DOMProcessingInstructionImpl::cloneNode(bool deep) const
{
    DOMNode* newNode = new (getOwnerDocument(), DOMMemoryManager::PROCESSING_INSTRUCTION_OBJECT) DOMProcessingInstructionImpl(*this, deep);
    fNode.callUserDataHandlers(DOMUserDataHandler::NODE_CLONED, this, newNode);
    return newNode;
}


const XMLCh * DOMProcessingInstructionImpl::getNodeName() const
{
    return fTarget;
}


DOMNode::NodeType DOMProcessingInstructionImpl::getNodeType() const {
    return DOMNode::PROCESSING_INSTRUCTION_NODE;
}


/** A PI's "target" states what processor channel the PI's data
should be directed to. It is defined differently in HTML and XML.

  In XML, a PI's "target" is the first (whitespace-delimited) token
  following the "<?" token that begins the PI.

    In HTML, target is always 0.

      Note that getNodeName is aliased to getTarget.
*/
const XMLCh * DOMProcessingInstructionImpl::getTarget() const
{
    return fTarget;
}


void DOMProcessingInstructionImpl::release()
{
    if (fNode.isOwned() && !fNode.isToBeReleased())
        throw DOMException(DOMException::INVALID_ACCESS_ERR,0, GetDOMNodeMemoryManager);

    DOMDocumentImpl* doc = (DOMDocumentImpl*) getOwnerDocument();
    if (doc) {
        fNode.callUserDataHandlers(DOMUserDataHandler::NODE_DELETED, 0, 0);
        fCharacterData.releaseBuffer();
        doc->release(this, DOMMemoryManager::PROCESSING_INSTRUCTION_OBJECT);
    }
    else {
        // shouldn't reach here
        throw DOMException(DOMException::INVALID_ACCESS_ERR,0, GetDOMNodeMemoryManager);
    }
}

void DOMProcessingInstructionImpl::setBaseURI(const XMLCh* baseURI) {
    this->fBaseURI = ((DOMDocumentImpl *)getOwnerDocument())->cloneString(baseURI);
}

const XMLCh* DOMProcessingInstructionImpl::getBaseURI() const
{
    return fBaseURI? fBaseURI : fNode.fOwnerNode->getBaseURI();
}

// Non standard extension for the range to work
DOMProcessingInstruction *DOMProcessingInstructionImpl::splitText(XMLSize_t offset)
{
    if (fNode.isReadOnly())
    {
        throw DOMException(
            DOMException::NO_MODIFICATION_ALLOWED_ERR, 0, GetDOMNodeMemoryManager);
    }
    XMLSize_t len = fCharacterData.fDataBuf->getLen();
    if (offset > len)
        throw DOMException(DOMException::INDEX_SIZE_ERR, 0,  GetDOMNodeMemoryManager);

    DOMDocumentImpl *doc = (DOMDocumentImpl *)getOwnerDocument();
    DOMProcessingInstruction *newText =
      doc->createProcessingInstruction(
        fTarget, this->substringData(offset, len - offset));

    DOMNode *parent = getParentNode();
    if (parent != 0)
        parent->insertBefore(newText, getNextSibling());

    fCharacterData.fDataBuf->chop(offset);

    if (doc != 0) {
        Ranges* ranges = doc->getRanges();
        if (ranges != 0) {
            XMLSize_t sz = ranges->size();
            if (sz != 0) {
                for (XMLSize_t i =0; i<sz; i++) {
                    ranges->elementAt(i)->updateSplitInfo( this, newText, offset);
                }
            }
        }
    }

    return newText;
}

//
//    Delegation stubs for inherited functions
//
           DOMNode*         DOMProcessingInstructionImpl::appendChild(DOMNode *newChild)          {return fNode.appendChild (newChild); }
           DOMNamedNodeMap* DOMProcessingInstructionImpl::getAttributes() const                   {return fNode.getAttributes (); }
           DOMNodeList*     DOMProcessingInstructionImpl::getChildNodes() const                   {return fNode.getChildNodes (); }
           DOMNode*         DOMProcessingInstructionImpl::getFirstChild() const                   {return fNode.getFirstChild (); }
           DOMNode*         DOMProcessingInstructionImpl::getLastChild() const                    {return fNode.getLastChild (); }
     const XMLCh*           DOMProcessingInstructionImpl::getLocalName() const                    {return fNode.getLocalName (); }
     const XMLCh*           DOMProcessingInstructionImpl::getNamespaceURI() const                 {return fNode.getNamespaceURI (); }
           DOMNode*         DOMProcessingInstructionImpl::getNextSibling() const                  {return fChild.getNextSibling (); }
     const XMLCh*           DOMProcessingInstructionImpl::getNodeValue() const                    {return fCharacterData.getNodeValue (); }
           DOMDocument*     DOMProcessingInstructionImpl::getOwnerDocument() const                {return fNode.getOwnerDocument (); }
     const XMLCh*           DOMProcessingInstructionImpl::getPrefix() const                       {return fNode.getPrefix (); }
           DOMNode*         DOMProcessingInstructionImpl::getParentNode() const                   {return fChild.getParentNode (this); }
           DOMNode*         DOMProcessingInstructionImpl::getPreviousSibling() const              {return fChild.getPreviousSibling (this); }
           bool             DOMProcessingInstructionImpl::hasChildNodes() const                   {return fNode.hasChildNodes (); }
           DOMNode*         DOMProcessingInstructionImpl::insertBefore(DOMNode *newChild, DOMNode *refChild)
                                                                                                  {return fNode.insertBefore (newChild, refChild); }
           void             DOMProcessingInstructionImpl::normalize()                             {fNode.normalize (); }
           DOMNode*         DOMProcessingInstructionImpl::removeChild(DOMNode *oldChild)          {return fNode.removeChild (oldChild); }
           DOMNode*         DOMProcessingInstructionImpl::replaceChild(DOMNode *newChild, DOMNode *oldChild)
                                                                                                  {return fNode.replaceChild (newChild, oldChild); }
           bool             DOMProcessingInstructionImpl::isSupported(const XMLCh *feature, const XMLCh *version) const
                                                                                                  {return fNode.isSupported (feature, version); }
           void             DOMProcessingInstructionImpl::setPrefix(const XMLCh  *prefix)         {fNode.setPrefix(prefix); }
           bool             DOMProcessingInstructionImpl::hasAttributes() const                   {return fNode.hasAttributes(); }
           bool             DOMProcessingInstructionImpl::isSameNode(const DOMNode* other) const  {return fNode.isSameNode(other); }
           bool             DOMProcessingInstructionImpl::isEqualNode(const DOMNode* arg) const   {return fNode.isEqualNode(arg); }
           void*            DOMProcessingInstructionImpl::setUserData(const XMLCh* key, void* data, DOMUserDataHandler* handler)
                                                                                                  {return fNode.setUserData(key, data, handler); }
           void*            DOMProcessingInstructionImpl::getUserData(const XMLCh* key) const     {return fNode.getUserData(key); }
           short            DOMProcessingInstructionImpl::compareDocumentPosition(const DOMNode* other) const {return fNode.compareDocumentPosition(other); }
           const XMLCh*     DOMProcessingInstructionImpl::getTextContent() const                  {return fNode.getTextContent(); }
           void             DOMProcessingInstructionImpl::setTextContent(const XMLCh* textContent){fNode.setTextContent(textContent); }
           const XMLCh*     DOMProcessingInstructionImpl::lookupPrefix(const XMLCh* namespaceURI) const  {return fNode.lookupPrefix(namespaceURI); }
           bool             DOMProcessingInstructionImpl::isDefaultNamespace(const XMLCh* namespaceURI) const {return fNode.isDefaultNamespace(namespaceURI); }
           const XMLCh*     DOMProcessingInstructionImpl::lookupNamespaceURI(const XMLCh* prefix) const  {return fNode.lookupNamespaceURI(prefix); }
           void*            DOMProcessingInstructionImpl::getFeature(const XMLCh* feature, const XMLCh* version) const {return fNode.getFeature(feature, version); }

//
//   Delegation of CharacerData functions.
//


           const XMLCh*     DOMProcessingInstructionImpl::getData() const                         {return fCharacterData.getData();}
           void             DOMProcessingInstructionImpl::deleteData(XMLSize_t offset, XMLSize_t count)
                                                                                    {fCharacterData.deleteData(this, offset, count);}
           const XMLCh*     DOMProcessingInstructionImpl::substringData(XMLSize_t offset, XMLSize_t count) const
                                                                                    {return fCharacterData.substringData(this, offset, count);}
           void             DOMProcessingInstructionImpl::setData(const XMLCh *data)              {fCharacterData.setData(this, data);}
           void             DOMProcessingInstructionImpl::setNodeValue(const XMLCh  *nodeValue)   {fCharacterData.setNodeValue (this, nodeValue); }


XERCES_CPP_NAMESPACE_END
