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
 * $Id: DOMTextImpl.cpp 678709 2008-07-22 10:56:56Z borisk $
 */


#include <xercesc/util/XMLUniDefs.hpp>

#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOMNode.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMCDATASection.hpp>
#include <xercesc/dom/DOMNodeFilter.hpp>
#include <xercesc/dom/DOMTreeWalker.hpp>

#include "DOMDocumentImpl.hpp"
#include "DOMStringPool.hpp"
#include "DOMTextImpl.hpp"
#include "DOMCharacterDataImpl.hpp"
#include "DOMChildNode.hpp"
#include "DOMRangeImpl.hpp"
#include "DOMCasts.hpp"

#include <assert.h>

XERCES_CPP_NAMESPACE_BEGIN

class DOMDocument;

DOMTextImpl::DOMTextImpl(DOMDocument *ownerDoc, const XMLCh *dat)
    : fNode(ownerDoc), fCharacterData(ownerDoc, dat)
{
    fNode.setIsLeafNode(true);
}

DOMTextImpl::
DOMTextImpl(DOMDocument *ownerDoc, const XMLCh* dat, XMLSize_t n)
    : fNode(ownerDoc), fCharacterData(ownerDoc, dat, n)
{
    fNode.setIsLeafNode(true);
}

DOMTextImpl::DOMTextImpl(const DOMTextImpl &other, bool)
    : DOMText(other)
    , fNode(other.fNode)
    , fCharacterData(other.fCharacterData)
{
    fNode.setIsLeafNode(true);
}

DOMTextImpl::~DOMTextImpl()
{
}


DOMNode *DOMTextImpl::cloneNode(bool deep) const
{
    DOMNode* newNode = new (getOwnerDocument(), DOMMemoryManager::TEXT_OBJECT) DOMTextImpl(*this, deep);
    fNode.callUserDataHandlers(DOMUserDataHandler::NODE_CLONED, this, newNode);
    return newNode;
}


const XMLCh * DOMTextImpl::getNodeName() const {
    static const XMLCh gtext[] = {chPound, chLatin_t, chLatin_e, chLatin_x, chLatin_t, chNull};
    return gtext;
}

DOMNode::NodeType DOMTextImpl::getNodeType() const {
    return DOMNode::TEXT_NODE;
}


DOMText *DOMTextImpl::splitText(XMLSize_t offset)
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
    DOMText *newText = doc->createTextNode(
      this->substringData(offset, len - offset));

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


bool DOMTextImpl::isIgnorableWhitespace() const
{
    return fNode.ignorableWhitespace();
}



void DOMTextImpl::setIgnorableWhitespace(bool ignorable)
{
    fNode.ignorableWhitespace(ignorable);
}


bool DOMTextImpl::getIsElementContentWhitespace() const
{
    return isIgnorableWhitespace();
}

const XMLCh* DOMTextImpl::getWholeText() const
{
    DOMDocument *doc = getOwnerDocument();
    DOMTreeWalker* pWalker=doc->createTreeWalker(doc->getDocumentElement(), DOMNodeFilter::SHOW_ALL, NULL, true);
    pWalker->setCurrentNode((DOMNode*)this);
    // Logically-adjacent text nodes are Text or CDATASection nodes that can be visited sequentially in document order or in
    // reversed document order without entering, exiting, or passing over Element, Comment, or ProcessingInstruction nodes.
	DOMNode* prevNode;
    while((prevNode=pWalker->previousNode())!=NULL)
    {
        if(prevNode->getNodeType()==ELEMENT_NODE || prevNode->getNodeType()==COMMENT_NODE || prevNode->getNodeType()==PROCESSING_INSTRUCTION_NODE)
            break;
    }
	XMLBuffer buff(1023, GetDOMNodeMemoryManager);
	DOMNode* nextNode;
    while((nextNode=pWalker->nextNode())!=NULL)
    {
        if(nextNode->getNodeType()==ELEMENT_NODE || nextNode->getNodeType()==COMMENT_NODE || nextNode->getNodeType()==PROCESSING_INSTRUCTION_NODE)
            break;
        if(nextNode->getNodeType()==TEXT_NODE || nextNode->getNodeType()==CDATA_SECTION_NODE)
    		buff.append(nextNode->getNodeValue());
    }
    pWalker->release();

    XMLCh* wholeString = (XMLCh*) (GetDOMNodeMemoryManager->allocate((buff.getLen()+1)*sizeof(XMLCh)));
	XMLString::copyString(wholeString, buff.getRawBuffer());
	return wholeString;
}

DOMText* DOMTextImpl::replaceWholeText(const XMLCh* newText)
{
    DOMDocument *doc = getOwnerDocument();
    DOMTreeWalker* pWalker=doc->createTreeWalker(doc->getDocumentElement(), DOMNodeFilter::SHOW_ALL, NULL, true);
    pWalker->setCurrentNode((DOMNode*)this);
    // Logically-adjacent text nodes are Text or CDATASection nodes that can be visited sequentially in document order or in
    // reversed document order without entering, exiting, or passing over Element, Comment, or ProcessingInstruction nodes.
    DOMNode* pFirstTextNode=this;
	DOMNode* prevNode;
    while((prevNode=pWalker->previousNode())!=NULL)
    {
        if(prevNode->getNodeType()==ELEMENT_NODE || prevNode->getNodeType()==COMMENT_NODE || prevNode->getNodeType()==PROCESSING_INSTRUCTION_NODE)
            break;
        pFirstTextNode=prevNode;
    }
    // before doing any change we need to check if we are going to remove an entity reference that doesn't contain just text
    DOMNode* pCurrentNode=pWalker->getCurrentNode();
	DOMNode* nextNode;
    while((nextNode=pWalker->nextNode())!=NULL)
    {
        if(nextNode->getNodeType()==ELEMENT_NODE || nextNode->getNodeType()==COMMENT_NODE || nextNode->getNodeType()==PROCESSING_INSTRUCTION_NODE)
            break;
        if(nextNode->getNodeType()==ENTITY_REFERENCE_NODE)
        {
            DOMTreeWalker* pInnerWalker=doc->createTreeWalker(nextNode, DOMNodeFilter::SHOW_ALL, NULL, true);
            while(pInnerWalker->nextNode())
            {
                short nodeType=pInnerWalker->getCurrentNode()->getNodeType();
                if(nodeType!=ENTITY_REFERENCE_NODE && nodeType!=TEXT_NODE && nodeType!=CDATA_SECTION_NODE)
                    throw DOMException(DOMException::NO_MODIFICATION_ALLOWED_ERR, 0, GetDOMNodeMemoryManager);
            }
            pInnerWalker->release();
        }
    }
    DOMText* retVal=NULL;
    // if the first node in the chain is a text node, replace its content, otherwise create a new node
    if(newText && *newText)
    {
        if(!castToNodeImpl(pFirstTextNode)->isReadOnly() && (pFirstTextNode->getNodeType()==TEXT_NODE || pFirstTextNode->getNodeType()==CDATA_SECTION_NODE))
        {
            pFirstTextNode->setNodeValue(newText);
            retVal=(DOMText*)pFirstTextNode;
        }
        else
        {
            if(getNodeType()==TEXT_NODE)
                retVal=doc->createTextNode(newText);
            else
                retVal=doc->createCDATASection(newText);
            pFirstTextNode->getParentNode()->insertBefore(retVal, pFirstTextNode);
        }
    }
    // now delete all the following text nodes
    pWalker->setCurrentNode(pCurrentNode);
    while((nextNode=pWalker->nextNode())!=NULL)
    {
        if(nextNode->getNodeType()==ELEMENT_NODE || nextNode->getNodeType()==COMMENT_NODE || nextNode->getNodeType()==PROCESSING_INSTRUCTION_NODE)
            break;
        if(nextNode!=retVal)
        {
            // keep the tree walker valid
            pWalker->previousNode();
            nextNode->getParentNode()->removeChild(nextNode);
            nextNode->release();
        }
    }
    pWalker->release();
    return retVal;
}


void DOMTextImpl::release()
{
    if (fNode.isOwned() && !fNode.isToBeReleased())
        throw DOMException(DOMException::INVALID_ACCESS_ERR,0, GetDOMNodeMemoryManager);

    DOMDocumentImpl* doc = (DOMDocumentImpl*) getOwnerDocument();
    if (doc) {
        fNode.callUserDataHandlers(DOMUserDataHandler::NODE_DELETED, 0, 0);
        fCharacterData.releaseBuffer();
        doc->release(this, DOMMemoryManager::TEXT_OBJECT);
    }
    else {
        // shouldn't reach here
        throw DOMException(DOMException::INVALID_ACCESS_ERR,0, GetDOMNodeMemoryManager);
    }
}

//
//  Delegation functions
//
           DOMNode*         DOMTextImpl::appendChild(DOMNode *newChild)          {return fNode.appendChild (newChild); }
           DOMNamedNodeMap* DOMTextImpl::getAttributes() const                   {return fNode.getAttributes (); }
           DOMNodeList*     DOMTextImpl::getChildNodes() const                   {return fNode.getChildNodes (); }
           DOMNode*         DOMTextImpl::getFirstChild() const                   {return fNode.getFirstChild (); }
           DOMNode*         DOMTextImpl::getLastChild() const                    {return fNode.getLastChild (); }
     const XMLCh*           DOMTextImpl::getLocalName() const                    {return fNode.getLocalName (); }
     const XMLCh*           DOMTextImpl::getNamespaceURI() const                 {return fNode.getNamespaceURI (); }
           DOMNode*         DOMTextImpl::getNextSibling() const                  {return fChild.getNextSibling (); }
     const XMLCh*           DOMTextImpl::getNodeValue() const                    {return fCharacterData.getNodeValue (); }
           DOMDocument*     DOMTextImpl::getOwnerDocument() const                {return fNode.getOwnerDocument (); }
     const XMLCh*           DOMTextImpl::getPrefix() const                       {return fNode.getPrefix (); }
           DOMNode*         DOMTextImpl::getParentNode() const                   {return fChild.getParentNode (this); }
           DOMNode*         DOMTextImpl::getPreviousSibling() const              {return fChild.getPreviousSibling (this); }
           bool             DOMTextImpl::hasChildNodes() const                   {return fNode.hasChildNodes (); }
           DOMNode*         DOMTextImpl::insertBefore(DOMNode *newChild, DOMNode *refChild)
                                                                                 {return fNode.insertBefore (newChild, refChild); }
           void             DOMTextImpl::normalize()                             {fNode.normalize (); }
           DOMNode*         DOMTextImpl::removeChild(DOMNode *oldChild)          {return fNode.removeChild (oldChild); }
           DOMNode*         DOMTextImpl::replaceChild(DOMNode *newChild, DOMNode *oldChild)
                                                                                 {return fNode.replaceChild (newChild, oldChild); }
           bool             DOMTextImpl::isSupported(const XMLCh *feature, const XMLCh *version) const
                                                                                 {return fNode.isSupported (feature, version); }
           void             DOMTextImpl::setPrefix(const XMLCh  *prefix)         {fNode.setPrefix(prefix); }
           bool             DOMTextImpl::hasAttributes() const                   {return fNode.hasAttributes(); }
           bool             DOMTextImpl::isSameNode(const DOMNode* other) const  {return fNode.isSameNode(other); }
           bool             DOMTextImpl::isEqualNode(const DOMNode* arg) const   {return fNode.isEqualNode(arg); }
           void*            DOMTextImpl::setUserData(const XMLCh* key, void* data, DOMUserDataHandler* handler)
                                                                                 {return fNode.setUserData(key, data, handler); }
           void*            DOMTextImpl::getUserData(const XMLCh* key) const     {return fNode.getUserData(key); }
           const XMLCh*     DOMTextImpl::getBaseURI() const                      {return fNode.getBaseURI(); }
           short            DOMTextImpl::compareDocumentPosition(const DOMNode* other) const {return fNode.compareDocumentPosition(other); }
           const XMLCh*     DOMTextImpl::getTextContent() const                  {return fNode.getTextContent(); }
           void             DOMTextImpl::setTextContent(const XMLCh* textContent){fNode.setTextContent(textContent); }
           const XMLCh*     DOMTextImpl::lookupPrefix(const XMLCh* namespaceURI) const  {return fNode.lookupPrefix(namespaceURI); }
           bool             DOMTextImpl::isDefaultNamespace(const XMLCh* namespaceURI) const {return fNode.isDefaultNamespace(namespaceURI); }
           const XMLCh*     DOMTextImpl::lookupNamespaceURI(const XMLCh* prefix) const  {return fNode.lookupNamespaceURI(prefix); }
           void*            DOMTextImpl::getFeature(const XMLCh* feature, const XMLCh* version) const {return fNode.getFeature(feature, version); }



//
//   Delegation of CharacerData functions.
//


          const XMLCh*      DOMTextImpl::getData() const                         {return fCharacterData.getData();}
          XMLSize_t         DOMTextImpl::getLength() const                       {return fCharacterData.getLength();}
          const XMLCh*      DOMTextImpl::substringData(XMLSize_t offset, XMLSize_t count) const
                                                                                 {return fCharacterData.substringData(this, offset, count);}
          void              DOMTextImpl::appendData(const XMLCh *arg)            {fCharacterData.appendData(this, arg);}
          void              DOMTextImpl::insertData(XMLSize_t offset, const  XMLCh *arg)
                                                                                 {fCharacterData.insertData(this, offset, arg);}
          void              DOMTextImpl::deleteData(XMLSize_t offset, XMLSize_t count)
                                                                                 {fCharacterData.deleteData(this, offset, count);}
          void              DOMTextImpl::replaceData(XMLSize_t offset, XMLSize_t count, const XMLCh *arg)
                                                                                 {fCharacterData.replaceData(this, offset, count, arg);}
          void              DOMTextImpl::setData(const XMLCh *data)              {fCharacterData.setData(this, data);}
          void              DOMTextImpl::setNodeValue(const XMLCh  *nodeValue)   {fCharacterData.setNodeValue (this, nodeValue); }

          void              DOMTextImpl::appendData(const XMLCh *arg, XMLSize_t n) {fCharacterData.appendData(this, arg, n);}

XERCES_CPP_NAMESPACE_END
