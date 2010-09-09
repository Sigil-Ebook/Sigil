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
 * $Id: DOMAttrMapImpl.cpp 678709 2008-07-22 10:56:56Z borisk $
 */

#include "DOMCasts.hpp"
#include "DOMNodeImpl.hpp"
#include "DOMNodeVector.hpp"
#include "DOMAttrMapImpl.hpp"
#include "DOMAttrImpl.hpp"
#include "DOMElementImpl.hpp"

#include <xercesc/dom/DOMAttr.hpp>
#include <xercesc/dom/DOMException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

DOMAttrMapImpl::DOMAttrMapImpl(DOMNode *ownerNod)
{
    this->fOwnerNode=ownerNod;
    this->fNodes = 0;
	hasDefaults(false);
}

DOMAttrMapImpl::DOMAttrMapImpl(DOMNode *ownerNod, const DOMAttrMapImpl *defaults)
{
    this->fOwnerNode=ownerNod;
    this->fNodes = 0;
	hasDefaults(false);
	if (defaults != 0)
	{
		if (defaults->getLength() > 0)
		{
			hasDefaults(true);
			cloneContent(defaults);
		}
	}
}

DOMAttrMapImpl::~DOMAttrMapImpl()
{
}

void DOMAttrMapImpl::cloneContent(const DOMAttrMapImpl *srcmap)
{
    if ((srcmap != 0) && (srcmap->fNodes != 0))
    {
        if (fNodes != 0)
            fNodes->reset();
        else
        {
            XMLSize_t size = srcmap->fNodes->size();
            if(size > 0) {
                DOMDocumentImpl *doc = (DOMDocumentImpl*)fOwnerNode->getOwnerDocument();
                fNodes = new (doc) DOMNodeVector(doc, size);
            }
        }

        for (XMLSize_t i = 0; i < srcmap->fNodes->size(); i++)
        {
            DOMNode *n = srcmap->fNodes->elementAt(i);
            DOMNode *clone = n->cloneNode(true);
            castToNodeImpl(clone)->isSpecified(castToNodeImpl(n)->isSpecified());
            castToNodeImpl(clone)->fOwnerNode = fOwnerNode;
            castToNodeImpl(clone)->isOwned(true);
            fNodes->addElement(clone);
        }
    }
}

DOMAttrMapImpl *DOMAttrMapImpl::cloneAttrMap(DOMNode *ownerNode_p)
{
	DOMAttrMapImpl *newmap = new (castToNodeImpl(ownerNode_p)->getOwnerDocument()) DOMAttrMapImpl(ownerNode_p);
	newmap->cloneContent(this);
	// newmap->attrDefaults = this->attrDefaults;  // revisit
	return newmap;
}

void DOMAttrMapImpl::setReadOnly(bool readOnl, bool deep)
{
    // this->fReadOnly=readOnl;
    if(deep && fNodes!=0)
    {
        XMLSize_t sz = fNodes->size();
        for (XMLSize_t i=0; i<sz; ++i) {
            castToNodeImpl(fNodes->elementAt(i))->setReadOnly(readOnl, deep);
        }
    }
}

bool DOMAttrMapImpl::readOnly() {
    return castToNodeImpl(fOwnerNode)->isReadOnly();
}

int DOMAttrMapImpl::findNamePoint(const XMLCh *name) const
{
    // Binary search
    int i=0;
    if(fNodes!=0)
    {
        int first=0,last=(int)fNodes->size()-1;

        while(first<=last)
        {
            i=(first+last)/2;
            int test = XMLString::compareString(name, fNodes->elementAt(i)->getNodeName());
            if(test==0)
                return i; // Name found
            else if(test<0)
                last=i-1;
            else
                first=i+1;
        }
        if(first>i) i=first;
    }
    /********************
    // Linear search
    int i = 0;
    if (fNodes != 0)
    for (i = 0; i < fNodes.size(); ++i)
    {
    int test = name.compareTo(((NodeImpl *) (fNodes.elementAt(i))).getNodeName());
    if (test == 0)
    return i;
    else
    if (test < 0)
    {
    break; // Found insertpoint
    }
    }

    *******************/
    return -1 - i; // not-found has to be encoded.
}

DOMNode * DOMAttrMapImpl::getNamedItem(const XMLCh *name) const
{
    int i=findNamePoint(name);
    return (i<0) ? 0 : fNodes->elementAt(i);
}

DOMNode *DOMAttrMapImpl::setNamedItem(DOMNode *arg)
{
    if (arg->getNodeType() != DOMNode::ATTRIBUTE_NODE)
        throw DOMException(DOMException::HIERARCHY_REQUEST_ERR, 0, GetDOMNamedNodeMapMemoryManager);

    DOMDocument *doc = fOwnerNode->getOwnerDocument();
    DOMNodeImpl *argImpl = castToNodeImpl(arg);
    if(argImpl->getOwnerDocument() != doc)
        throw DOMException(DOMException::WRONG_DOCUMENT_ERR, 0, GetDOMNamedNodeMapMemoryManager);
    if (this->readOnly())
        throw DOMException(DOMException::NO_MODIFICATION_ALLOWED_ERR, 0, GetDOMNamedNodeMapMemoryManager);
    if ((arg->getNodeType() == DOMNode::ATTRIBUTE_NODE) && argImpl->isOwned() && (argImpl->fOwnerNode != fOwnerNode))
        throw DOMException(DOMException::INUSE_ATTRIBUTE_ERR,0, GetDOMNamedNodeMapMemoryManager);

    argImpl->fOwnerNode = fOwnerNode;
    argImpl->isOwned(true);
    int i=findNamePoint(arg->getNodeName());
    DOMNode * previous=0;
    if(i>=0)
    {
        previous = fNodes->elementAt(i);
        fNodes->setElementAt(arg,i);
    }
    else
    {
        i=-1-i; // Insert point (may be end of list)
        if(0==fNodes)
        {
            fNodes=new ((DOMDocumentImpl*)doc) DOMNodeVector(doc);
        }
        fNodes->insertElementAt(arg,i);
    }
    if (previous != 0) {
        castToNodeImpl(previous)->fOwnerNode = doc;
        castToNodeImpl(previous)->isOwned(false);
    }

    return previous;
}

//Introduced in DOM Level 2

int DOMAttrMapImpl::findNamePoint(const XMLCh *namespaceURI,
	const XMLCh *localName) const
{
    if (fNodes == 0)
	return -1;
    // This is a linear search through the same fNodes Vector.
    // The Vector is sorted on the DOM Level 1 nodename.
    // The DOM Level 2 NS keys are namespaceURI and Localname,
    // so we must linear search thru it.
    // In addition, to get this to work with fNodes without any namespace
    // (namespaceURI and localNames are both 0) we then use the nodeName
    // as a secondary key.
    const XMLSize_t len = fNodes -> size();
    for (XMLSize_t i = 0; i < len; ++i) {
        DOMNode *node = fNodes -> elementAt(i);
        const XMLCh * nNamespaceURI = node->getNamespaceURI();
        const XMLCh * nLocalName = node->getLocalName();
        if (!XMLString::equals(nNamespaceURI, namespaceURI))    //URI not match
            continue;
        else {
            if (XMLString::equals(localName, nLocalName)
                ||
                (nLocalName == 0 && XMLString::equals(localName, node->getNodeName())))
                return (int)i;
        }
    }
    return -1;	//not found
}

DOMNode *DOMAttrMapImpl::getNamedItemNS(const XMLCh *namespaceURI,
	const XMLCh *localName) const
{
    int i = findNamePoint(namespaceURI, localName);
    return i < 0 ? 0 : fNodes -> elementAt(i);
}

DOMNode *DOMAttrMapImpl::setNamedItemNS(DOMNode* arg)
{
    if (arg->getNodeType() != DOMNode::ATTRIBUTE_NODE)
        throw DOMException(DOMException::HIERARCHY_REQUEST_ERR, 0, GetDOMNamedNodeMapMemoryManager);

    DOMDocument *doc = fOwnerNode->getOwnerDocument();
    DOMNodeImpl *argImpl = castToNodeImpl(arg);
    if (argImpl->getOwnerDocument() != doc)
        throw DOMException(DOMException::WRONG_DOCUMENT_ERR,0, GetDOMNamedNodeMapMemoryManager);
    if (this->readOnly())
        throw DOMException(DOMException::NO_MODIFICATION_ALLOWED_ERR, 0, GetDOMNamedNodeMapMemoryManager);
    if (argImpl->isOwned())
        throw DOMException(DOMException::INUSE_ATTRIBUTE_ERR,0, GetDOMNamedNodeMapMemoryManager);

    argImpl->fOwnerNode = fOwnerNode;
    argImpl->isOwned(true);
    int i=findNamePoint(arg->getNamespaceURI(), arg->getLocalName());
    DOMNode *previous=0;
    if(i>=0) {
        previous = fNodes->elementAt(i);
        fNodes->setElementAt(arg,i);
    } else {
        i=findNamePoint(arg->getNodeName()); // Insert point (may be end of list)
        if (i<0)
          i = -1 - i;
        if(0==fNodes)
            fNodes=new ((DOMDocumentImpl*)doc) DOMNodeVector(doc);
        fNodes->insertElementAt(arg,i);
    }
    if (previous != 0) {
        castToNodeImpl(previous)->fOwnerNode = doc;
        castToNodeImpl(previous)->isOwned(false);
    }

    return previous;
}

DOMNode *DOMAttrMapImpl::removeNamedItem(const XMLCh *name)
{
    if (this->readOnly())
        throw DOMException(
            DOMException::NO_MODIFICATION_ALLOWED_ERR, 0, GetDOMNamedNodeMapMemoryManager);
    int i=findNamePoint(name);
    DOMNode *removed = 0;

    if(i<0)
        throw DOMException(DOMException::NOT_FOUND_ERR, 0, GetDOMNamedNodeMapMemoryManager);

    removed = fNodes->elementAt(i);
    fNodes->removeElementAt(i);
    castToNodeImpl(removed)->fOwnerNode = fOwnerNode->getOwnerDocument();
    castToNodeImpl(removed)->isOwned(false);

    // Replace it if it had a default value
    // (DOM spec level 1 - Element Interface)
    if (hasDefaults() && (removed != 0))
    {
        DOMAttrMapImpl* defAttrs = ((DOMElementImpl*)fOwnerNode)->getDefaultAttributes();
        DOMAttr* attr = (DOMAttr*)(defAttrs->getNamedItem(name));
        if (attr != 0)
        {
            DOMAttr* newAttr = (DOMAttr*)attr->cloneNode(true);
            setNamedItem(newAttr);
        }
    }

    return removed;
}

DOMNode *DOMAttrMapImpl::removeNamedItemNS(const XMLCh *namespaceURI, const XMLCh *localName)
{
    if (this->readOnly())
        throw DOMException(
        DOMException::NO_MODIFICATION_ALLOWED_ERR, 0, GetDOMNamedNodeMapMemoryManager);
    int i = findNamePoint(namespaceURI, localName);
    if (i < 0)
        throw DOMException(DOMException::NOT_FOUND_ERR, 0, GetDOMNamedNodeMapMemoryManager);

    DOMNode * removed = fNodes -> elementAt(i);
    fNodes -> removeElementAt(i);	//remove n from nodes
    castToNodeImpl(removed)->fOwnerNode = fOwnerNode->getOwnerDocument();
    castToNodeImpl(removed)->isOwned(false);

    // Replace it if it had a default value
    // (DOM spec level 2 - Element Interface)

    if (hasDefaults() && (removed != 0))
    {
        DOMAttrMapImpl* defAttrs = ((DOMElementImpl*)fOwnerNode)->getDefaultAttributes();
        DOMAttr* attr = (DOMAttr*)(defAttrs->getNamedItemNS(namespaceURI, localName));
        if (attr != 0)
        {
            DOMAttr* newAttr = (DOMAttr*)attr->cloneNode(true);
            setNamedItemNS(newAttr);
        }
    }

    return removed;
}

// remove the name using index
// avoid calling findNamePoint again if the index is already known
DOMNode * DOMAttrMapImpl::removeNamedItemAt(XMLSize_t index)
{
    if (this->readOnly())
        throw DOMException(
            DOMException::NO_MODIFICATION_ALLOWED_ERR, 0, GetDOMNamedNodeMapMemoryManager);

    DOMNode *removed = item(index);
    if(!removed)
        throw DOMException(DOMException::NOT_FOUND_ERR, 0, GetDOMNamedNodeMapMemoryManager);

    fNodes->removeElementAt(index);
    castToNodeImpl(removed)->fOwnerNode = fOwnerNode->getOwnerDocument();
    castToNodeImpl(removed)->isOwned(false);

    // Replace it if it had a default value
    // (DOM spec level 1 - Element Interface)
    if (hasDefaults() && (removed != 0))
    {
        DOMAttrMapImpl* defAttrs = ((DOMElementImpl*)fOwnerNode)->getDefaultAttributes();

        const XMLCh* localName = removed->getLocalName();
        DOMAttr* attr = 0;
        if (localName)
            attr = (DOMAttr*)(defAttrs->getNamedItemNS(removed->getNamespaceURI(), localName));
        else
            attr = (DOMAttr*)(defAttrs->getNamedItem(((DOMAttr*)removed)->getName()));

        if (attr != 0)
        {
            DOMAttr* newAttr = (DOMAttr*)attr->cloneNode(true);
            setNamedItem(newAttr);
        }
    }

    return removed;
}

/**
 * Get this AttributeMap in sync with the given "defaults" map.
 * @param defaults The default attributes map to sync with.
 */
void DOMAttrMapImpl::reconcileDefaultAttributes(const DOMAttrMapImpl* defaults) {

    // remove any existing default
    XMLSize_t nsize = getLength();
    for (XMLSize_t i = nsize; i > 0; i--) {
        DOMAttr* attr = (DOMAttr*)item(i-1);
        if (!attr->getSpecified()) {
            removeNamedItemAt(i-1);
        }
    }

    hasDefaults(false);

    // add the new defaults
    if (defaults) {
        hasDefaults(true);

        if (nsize == 0) {
            cloneContent(defaults);
        }
        else {
            XMLSize_t dsize = defaults->getLength();
            for (XMLSize_t n = 0; n < dsize; n++) {
                DOMAttr* attr = (DOMAttr*)defaults->item(n);

                DOMAttr* newAttr = (DOMAttr*)attr->cloneNode(true);
                setNamedItemNS(newAttr);
                DOMAttrImpl* newAttrImpl = (DOMAttrImpl*) newAttr;
                newAttrImpl->setSpecified(false);
            }
        }
    }
} // reconcileDefaults()


/**
 * Move specified attributes from the given map to this one
 */
void DOMAttrMapImpl::moveSpecifiedAttributes(DOMAttrMapImpl* srcmap) {
    XMLSize_t nsize = srcmap->getLength();

    for (XMLSize_t i = nsize; i > 0; i--) {
        DOMAttr* attr = (DOMAttr*)srcmap->item(i-1);
        if (attr->getSpecified()) {
            srcmap->removeNamedItemAt(i-1);
        }

        if (attr->getLocalName())
            setNamedItemNS(attr);
        else
            setNamedItem(attr);
    }
} // moveSpecifiedAttributes(AttributeMap):void

XMLSize_t DOMAttrMapImpl::getLength() const
{
    return (fNodes != 0) ? fNodes->size() : 0;
}

DOMNode * DOMAttrMapImpl::item(XMLSize_t index) const
{
    return (fNodes != 0 && index < fNodes->size()) ?
        fNodes->elementAt(index) : 0;
}

void DOMAttrMapImpl::setNamedItemFast(DOMNode *arg)
{
    DOMNodeImpl *argImpl = castToNodeImpl(arg);

    argImpl->fOwnerNode = fOwnerNode;
    argImpl->isOwned(true);
    int i = findNamePoint(arg->getNodeName());

    if(i >= 0)
      fNodes->setElementAt(arg, i);
    else
    {
      i= -1 -i;
      fNodes->insertElementAt(arg, i);
    }
}

void DOMAttrMapImpl::setNamedItemNSFast(DOMNode* arg)
{
    DOMNodeImpl *argImpl = castToNodeImpl(arg);

    argImpl->fOwnerNode = fOwnerNode;
    argImpl->isOwned(true);
    int i=findNamePoint(arg->getNamespaceURI(), arg->getLocalName());

    if(i >= 0)
    {
        fNodes->setElementAt(arg,i);
    }
    else
    {
        i = findNamePoint(arg->getNodeName());

        if (i < 0)
          i = -1 - i;

        fNodes->insertElementAt(arg,i);
    }
}

void DOMAttrMapImpl::reserve (XMLSize_t n)
{
  if (fNodes == 0)
  {
    DOMDocumentImpl* doc = (DOMDocumentImpl*)fOwnerNode->getOwnerDocument();
    fNodes = new (doc) DOMNodeVector(doc, n);
  }
}

XERCES_CPP_NAMESPACE_END
