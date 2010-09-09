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
 * $Id: DOMCommentImpl.cpp 678381 2008-07-21 10:15:01Z borisk $
 */

#include "DOMCommentImpl.hpp"
#include "DOMCharacterDataImpl.hpp"
#include "DOMStringPool.hpp"
#include "DOMCasts.hpp"
#include "DOMDocumentImpl.hpp"
#include "DOMRangeImpl.hpp"
#include <xercesc/dom/DOMNode.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/util/XMLUniDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN

DOMCommentImpl::DOMCommentImpl(DOMDocument *ownerDoc, const XMLCh *dat)
    : fNode(ownerDoc),  fCharacterData(ownerDoc, dat)
{
    fNode.setIsLeafNode(true);
}


DOMCommentImpl::DOMCommentImpl(const DOMCommentImpl &other, bool)

    : fNode(other.fNode),
    fChild(other.fChild),
    fCharacterData(other.fCharacterData)
{
    fNode.setIsLeafNode(true);
}


DOMCommentImpl::~DOMCommentImpl() {
}



DOMNode * DOMCommentImpl::cloneNode(bool deep) const
{
    DOMNode* newNode = new (getOwnerDocument(), DOMMemoryManager::COMMENT_OBJECT) DOMCommentImpl(*this, deep);
    fNode.callUserDataHandlers(DOMUserDataHandler::NODE_CLONED, this, newNode);
    return newNode;
}


const XMLCh * DOMCommentImpl::getNodeName() const {
    static const XMLCh gComment[] =
        {chPound, chLatin_c, chLatin_o, chLatin_m, chLatin_m, chLatin_e,chLatin_n, chLatin_t, 0};
    return gComment;
}

DOMNode::NodeType DOMCommentImpl::getNodeType() const {
    return DOMNode::COMMENT_NODE;
}

void DOMCommentImpl::release()
{
    if (fNode.isOwned() && !fNode.isToBeReleased())
        throw DOMException(DOMException::INVALID_ACCESS_ERR,0, GetDOMNodeMemoryManager);

    DOMDocumentImpl* doc = (DOMDocumentImpl*) getOwnerDocument();
    if (doc) {
        fNode.callUserDataHandlers(DOMUserDataHandler::NODE_DELETED, 0, 0);
        fCharacterData.releaseBuffer();
        doc->release(this, DOMMemoryManager::COMMENT_OBJECT);
    }
    else {
        // shouldn't reach here
        throw DOMException(DOMException::INVALID_ACCESS_ERR,0, GetDOMNodeMemoryManager);
    }
}


// Non standard extension for the range to work
DOMComment *DOMCommentImpl::splitText(XMLSize_t offset)
{
    if (fNode.isReadOnly())
    {
        throw DOMException(
            DOMException::NO_MODIFICATION_ALLOWED_ERR, 0, GetDOMNodeMemoryManager);
    }
    XMLSize_t len = fCharacterData.fDataBuf->getLen();
    if (offset > len)
        throw DOMException(DOMException::INDEX_SIZE_ERR, 0, GetDOMNodeMemoryManager);

    DOMDocumentImpl *doc = (DOMDocumentImpl *)getOwnerDocument();
    DOMComment *newText =
      doc->createComment(this->substringData(offset, len - offset));

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


           DOMNode*         DOMCommentImpl::appendChild(DOMNode *newChild)          {return fNode.appendChild (newChild); }
           DOMNamedNodeMap* DOMCommentImpl::getAttributes() const                   {return fNode.getAttributes (); }
           DOMNodeList*     DOMCommentImpl::getChildNodes() const                   {return fNode.getChildNodes (); }
           DOMNode*         DOMCommentImpl::getFirstChild() const                   {return fNode.getFirstChild (); }
           DOMNode*         DOMCommentImpl::getLastChild() const                    {return fNode.getLastChild (); }
     const XMLCh*           DOMCommentImpl::getLocalName() const                    {return fNode.getLocalName (); }
     const XMLCh*           DOMCommentImpl::getNamespaceURI() const                 {return fNode.getNamespaceURI (); }
           DOMNode*         DOMCommentImpl::getNextSibling() const                  {return fChild.getNextSibling (); }
     const XMLCh*           DOMCommentImpl::getNodeValue() const                    {return fCharacterData.getNodeValue (); }
           DOMDocument*     DOMCommentImpl::getOwnerDocument() const                {return fNode.getOwnerDocument (); }
     const XMLCh*           DOMCommentImpl::getPrefix() const                       {return fNode.getPrefix (); }
           DOMNode*         DOMCommentImpl::getParentNode() const                   {return fChild.getParentNode (this); }
           DOMNode*         DOMCommentImpl::getPreviousSibling() const              {return fChild.getPreviousSibling (this); }
           bool             DOMCommentImpl::hasChildNodes() const                   {return fNode.hasChildNodes (); }
           DOMNode*         DOMCommentImpl::insertBefore(DOMNode *newChild, DOMNode *refChild)
                                                                                    {return fNode.insertBefore (newChild, refChild); }
           void             DOMCommentImpl::normalize()                             {fNode.normalize (); }
           DOMNode*         DOMCommentImpl::removeChild(DOMNode *oldChild)          {return fNode.removeChild (oldChild); }
           DOMNode*         DOMCommentImpl::replaceChild(DOMNode *newChild, DOMNode *oldChild)
                                                                                    {return fNode.replaceChild (newChild, oldChild); }
           bool             DOMCommentImpl::isSupported(const XMLCh *feature, const XMLCh *version) const
                                                                                    {return fNode.isSupported (feature, version); }
           void             DOMCommentImpl::setPrefix(const XMLCh  *prefix)         {fNode.setPrefix(prefix); }
           bool             DOMCommentImpl::hasAttributes() const                   {return fNode.hasAttributes(); }
           bool             DOMCommentImpl::isSameNode(const DOMNode* other) const  {return fNode.isSameNode(other); }
           bool             DOMCommentImpl::isEqualNode(const DOMNode* arg) const   {return fNode.isEqualNode(arg); }
           void*            DOMCommentImpl::setUserData(const XMLCh* key, void* data, DOMUserDataHandler* handler)
                                                                                    {return fNode.setUserData(key, data, handler); }
           void*            DOMCommentImpl::getUserData(const XMLCh* key) const     {return fNode.getUserData(key); }
           const XMLCh*     DOMCommentImpl::getBaseURI() const                      {return fNode.getBaseURI(); }
           short            DOMCommentImpl::compareDocumentPosition(const DOMNode* other) const {return fNode.compareDocumentPosition(other); }
           const XMLCh*     DOMCommentImpl::getTextContent() const                  {return fNode.getTextContent(); }
           void             DOMCommentImpl::setTextContent(const XMLCh* textContent){fNode.setTextContent(textContent); }
           const XMLCh*     DOMCommentImpl::lookupPrefix(const XMLCh* namespaceURI) const  {return fNode.lookupPrefix(namespaceURI); }
           bool             DOMCommentImpl::isDefaultNamespace(const XMLCh* namespaceURI) const {return fNode.isDefaultNamespace(namespaceURI); }
           const XMLCh*     DOMCommentImpl::lookupNamespaceURI(const XMLCh* prefix) const  {return fNode.lookupNamespaceURI(prefix); }
           void*            DOMCommentImpl::getFeature(const XMLCh* feature, const XMLCh* version) const {return fNode.getFeature(feature, version); }



//
//   Delegation of CharacerData functions.
//


           const XMLCh*     DOMCommentImpl::getData() const                         {return fCharacterData.getData();}
           XMLSize_t        DOMCommentImpl::getLength() const                       {return fCharacterData.getLength();}
           const XMLCh*     DOMCommentImpl::substringData(XMLSize_t offset, XMLSize_t count) const
                                                                                    {return fCharacterData.substringData(this, offset, count);}
           void             DOMCommentImpl::appendData(const XMLCh *arg)            {fCharacterData.appendData(this, arg);}
           void             DOMCommentImpl::insertData(XMLSize_t offset, const  XMLCh *arg)
                                                                                    {fCharacterData.insertData(this, offset, arg);}
           void             DOMCommentImpl::deleteData(XMLSize_t offset, XMLSize_t count)
                                                                                    {fCharacterData.deleteData(this, offset, count);}
           void             DOMCommentImpl::replaceData(XMLSize_t offset, XMLSize_t count, const XMLCh *arg)
                                                                                    {fCharacterData.replaceData(this, offset, count, arg);}
           void             DOMCommentImpl::setData(const XMLCh *data)              {fCharacterData.setData(this, data);}
           void             DOMCommentImpl::setNodeValue(const XMLCh  *nodeValue)   {fCharacterData.setNodeValue (this, nodeValue); }

XERCES_CPP_NAMESPACE_END
