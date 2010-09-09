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
 * $Id: DOMCDATASectionImpl.cpp 678709 2008-07-22 10:56:56Z borisk $
 */

#include "DOMCDATASectionImpl.hpp"
#include "DOMNodeImpl.hpp"
#include "DOMRangeImpl.hpp"
#include "DOMDocumentImpl.hpp"
#include "DOMCasts.hpp"
#include "DOMStringPool.hpp"
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOMNodeFilter.hpp>
#include <xercesc/dom/DOMTreeWalker.hpp>
#include <xercesc/util/XMLUniDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN

DOMCDATASectionImpl::DOMCDATASectionImpl(DOMDocument *ownerDoc,
                                   const XMLCh *dat)
    : fNode(ownerDoc), fCharacterData(ownerDoc, dat)
{
    fNode.setIsLeafNode(true);
}

DOMCDATASectionImpl::
DOMCDATASectionImpl(DOMDocument *ownerDoc, const XMLCh* data, XMLSize_t n)
    : fNode(ownerDoc), fCharacterData(ownerDoc, data, n)
{
    fNode.setIsLeafNode(true);
}

DOMCDATASectionImpl::DOMCDATASectionImpl(const DOMCDATASectionImpl &other, bool /*deep*/)
    : DOMCDATASection(other),
    fNode(*castToNodeImpl(&other)),
    fChild(*castToChildImpl(&other)),
    fCharacterData(other.fCharacterData)
{
    // revisit.  Something nees to make "deep" work.
}


DOMCDATASectionImpl::~DOMCDATASectionImpl()
{
}


DOMNode  *DOMCDATASectionImpl::cloneNode(bool deep) const
{
    DOMNode* newNode = new (this->getOwnerDocument(), DOMMemoryManager::CDATA_SECTION_OBJECT) DOMCDATASectionImpl(*this, deep);
    fNode.callUserDataHandlers(DOMUserDataHandler::NODE_CLONED, this, newNode);
    return newNode;
}


const XMLCh * DOMCDATASectionImpl::getNodeName() const {
    static const XMLCh gcdata_section[] = {chPound, chLatin_c, chLatin_d, chLatin_a, chLatin_t, chLatin_a,
        chDash, chLatin_s, chLatin_e, chLatin_c, chLatin_t, chLatin_i, chLatin_o, chLatin_n, 0};
    return gcdata_section;
}


DOMNode::NodeType DOMCDATASectionImpl::getNodeType() const {
    return DOMNode::CDATA_SECTION_NODE;
}


bool DOMCDATASectionImpl::isIgnorableWhitespace() const
{
    return fNode.ignorableWhitespace();
}


//
//  splitText.   revist - factor into a common function for use
//                             here and in DOMTextImpl
//
DOMText *DOMCDATASectionImpl::splitText(XMLSize_t offset)
{
    if (fNode.isReadOnly())
    {
        throw DOMException(DOMException::NO_MODIFICATION_ALLOWED_ERR, 0, GetDOMNodeMemoryManager);
    }
    XMLSize_t len = fCharacterData.fDataBuf->getLen();
    if (offset > len)
        throw DOMException(DOMException::INDEX_SIZE_ERR, 0, GetDOMNodeMemoryManager);

    DOMDocumentImpl *doc = (DOMDocumentImpl *)getOwnerDocument();
    DOMText *newText =
      doc->createCDATASection(this->substringData(offset, len - offset));

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


bool DOMCDATASectionImpl::getIsElementContentWhitespace() const
{
    return isIgnorableWhitespace();
}

const XMLCh* DOMCDATASectionImpl::getWholeText() const
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

DOMText* DOMCDATASectionImpl::replaceWholeText(const XMLCh* newText)
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


void DOMCDATASectionImpl::release()
{
    if (fNode.isOwned() && !fNode.isToBeReleased())
        throw DOMException(DOMException::INVALID_ACCESS_ERR,0, GetDOMNodeMemoryManager);

    DOMDocumentImpl* doc = (DOMDocumentImpl*) getOwnerDocument();

    if (doc) {
        fNode.callUserDataHandlers(DOMUserDataHandler::NODE_DELETED, 0, 0);
        fCharacterData.releaseBuffer();
        doc->release(this, DOMMemoryManager::CDATA_SECTION_OBJECT);
    }
    else {
        // shouldn't reach here
        throw DOMException(DOMException::INVALID_ACCESS_ERR,0, GetDOMNodeMemoryManager);
    }
}


//
//  Delegation stubs for other DOM_Node inherited functions.
//
           DOMNode*         DOMCDATASectionImpl::appendChild(DOMNode *newChild)          {return fNode.appendChild (newChild); }
           DOMNamedNodeMap* DOMCDATASectionImpl::getAttributes() const                   {return fNode.getAttributes (); }
           DOMNodeList*     DOMCDATASectionImpl::getChildNodes() const                   {return fNode.getChildNodes (); }
           DOMNode*         DOMCDATASectionImpl::getFirstChild() const                   {return fNode.getFirstChild (); }
           DOMNode*         DOMCDATASectionImpl::getLastChild() const                    {return fNode.getLastChild (); }
     const XMLCh*           DOMCDATASectionImpl::getLocalName() const                    {return fNode.getLocalName (); }
     const XMLCh*           DOMCDATASectionImpl::getNamespaceURI() const                 {return fNode.getNamespaceURI (); }
           DOMNode*         DOMCDATASectionImpl::getNextSibling() const                  {return fChild.getNextSibling (); }
     const XMLCh*           DOMCDATASectionImpl::getNodeValue() const                    {return fCharacterData.getNodeValue (); }
           DOMDocument*     DOMCDATASectionImpl::getOwnerDocument() const                {return fNode.getOwnerDocument(); }
     const XMLCh*           DOMCDATASectionImpl::getPrefix() const                       {return fNode.getPrefix (); }
           DOMNode*         DOMCDATASectionImpl::getParentNode() const                   {return fChild.getParentNode (this); }
           DOMNode*         DOMCDATASectionImpl::getPreviousSibling() const              {return fChild.getPreviousSibling (this); }
           bool             DOMCDATASectionImpl::hasChildNodes() const                   {return fNode.hasChildNodes (); }
           DOMNode*         DOMCDATASectionImpl::insertBefore(DOMNode *newChild, DOMNode *refChild)
                                                                                         {return fNode.insertBefore (newChild, refChild); }
           void             DOMCDATASectionImpl::normalize()                             {fNode.normalize (); }
           DOMNode*         DOMCDATASectionImpl::removeChild(DOMNode *oldChild)          {return fNode.removeChild (oldChild); }
           DOMNode*         DOMCDATASectionImpl::replaceChild(DOMNode *newChild, DOMNode *oldChild)
                                                                                         {return fNode.replaceChild (newChild, oldChild); }
           bool             DOMCDATASectionImpl::isSupported(const XMLCh *feature, const XMLCh *version) const
                                                                                         {return fNode.isSupported (feature, version); }
           void             DOMCDATASectionImpl::setPrefix(const XMLCh  *prefix)         {fNode.setPrefix(prefix); }
           bool             DOMCDATASectionImpl::hasAttributes() const                   {return fNode.hasAttributes(); }
           bool             DOMCDATASectionImpl::isSameNode(const DOMNode* other) const  {return fNode.isSameNode(other); }
           bool             DOMCDATASectionImpl::isEqualNode(const DOMNode* arg) const   {return fNode.isEqualNode(arg); }
           void*            DOMCDATASectionImpl::setUserData(const XMLCh* key, void* data, DOMUserDataHandler* handler)
                                                                                         {return fNode.setUserData(key, data, handler); }
           void*            DOMCDATASectionImpl::getUserData(const XMLCh* key) const     {return fNode.getUserData(key); }
           const XMLCh*     DOMCDATASectionImpl::getBaseURI() const                      {return fNode.getBaseURI(); }
           short            DOMCDATASectionImpl::compareDocumentPosition(const DOMNode* other) const {return fNode.compareDocumentPosition(other); }
           const XMLCh*     DOMCDATASectionImpl::getTextContent() const                  {return fNode.getTextContent(); }
           void             DOMCDATASectionImpl::setTextContent(const XMLCh* textContent){fNode.setTextContent(textContent); }
           const XMLCh*     DOMCDATASectionImpl::lookupPrefix(const XMLCh* namespaceURI) const  {return fNode.lookupPrefix(namespaceURI); }
           bool             DOMCDATASectionImpl::isDefaultNamespace(const XMLCh* namespaceURI) const {return fNode.isDefaultNamespace(namespaceURI); }
           const XMLCh*     DOMCDATASectionImpl::lookupNamespaceURI(const XMLCh* prefix) const  {return fNode.lookupNamespaceURI(prefix); }
           void*            DOMCDATASectionImpl::getFeature(const XMLCh* feature, const XMLCh* version) const {return fNode.getFeature(feature, version); }



//
//   Delegation of CharacerData functions.
//


           const XMLCh*     DOMCDATASectionImpl::getData() const                         {return fCharacterData.getData();}
           XMLSize_t        DOMCDATASectionImpl::getLength() const                       {return fCharacterData.getLength();}
           const XMLCh*     DOMCDATASectionImpl::substringData(XMLSize_t offset, XMLSize_t count) const
                                                                                         {return fCharacterData.substringData(this, offset, count);}
           void             DOMCDATASectionImpl::appendData(const XMLCh *arg)            {fCharacterData.appendData(this, arg);}
           void             DOMCDATASectionImpl::insertData(XMLSize_t offset, const  XMLCh *arg)
                                                                                         {fCharacterData.insertData(this, offset, arg);}
           void             DOMCDATASectionImpl::deleteData(XMLSize_t offset, XMLSize_t count)
                                                                                         {fCharacterData.deleteData(this, offset, count);}
           void             DOMCDATASectionImpl::replaceData(XMLSize_t offset, XMLSize_t count, const XMLCh *arg)
                                                                                         {fCharacterData.replaceData(this, offset, count, arg);}
           void             DOMCDATASectionImpl::setData(const XMLCh *data)              {fCharacterData.setData(this, data);}
           void             DOMCDATASectionImpl::setNodeValue(const XMLCh  *nodeValue)   {fCharacterData.setNodeValue (this, nodeValue); }

XERCES_CPP_NAMESPACE_END
