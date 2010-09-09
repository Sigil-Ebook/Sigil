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
 * $Id: DOMNamedNodeMapImpl.cpp 678381 2008-07-21 10:15:01Z borisk $
 */


#include <xercesc/dom/DOMAttr.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/framework/XMLBuffer.hpp>
#include <xercesc/util/XMLUniDefs.hpp>

#include "DOMNodeVector.hpp"
#include "DOMNamedNodeMapImpl.hpp"
#include "DOMCasts.hpp"
#include "DOMDocumentImpl.hpp"
#include "DOMNodeImpl.hpp"

XERCES_CPP_NAMESPACE_BEGIN

DOMNamedNodeMapImpl::DOMNamedNodeMapImpl(DOMNode *ownerNod)
{
    fOwnerNode=ownerNod;
    memset(fBuckets,0,MAP_SIZE*sizeof(DOMNodeVector*));
}

DOMNamedNodeMapImpl::~DOMNamedNodeMapImpl()
{
}

bool DOMNamedNodeMapImpl::readOnly()
{
    return castToNodeImpl(fOwnerNode)->isReadOnly();
}

DOMNamedNodeMapImpl *DOMNamedNodeMapImpl::cloneMap(DOMNode *ownerNod)
{
    DOMDocumentImpl *doc = (DOMDocumentImpl *)(castToNodeImpl(ownerNod)->getOwnerDocument());
    DOMNamedNodeMapImpl *newmap = new (doc) DOMNamedNodeMapImpl(ownerNod);

    for(XMLSize_t index=0;index<MAP_SIZE;index++)
        if (fBuckets[index] != 0) {
            XMLSize_t size=fBuckets[index]->size();
            newmap->fBuckets[index] = new (doc) DOMNodeVector(doc, size);
            for (XMLSize_t i = 0; i < size; ++i) {
                DOMNode *s = fBuckets[index]->elementAt(i);
                DOMNode *n = s->cloneNode(true);
			    castToNodeImpl(n)->isSpecified(castToNodeImpl(s)->isSpecified());
                castToNodeImpl(n)->fOwnerNode = ownerNod;
                castToNodeImpl(n)->isOwned(true);
                newmap->fBuckets[index]->addElement(n);
            }
        }

    return newmap;
}


XMLSize_t DOMNamedNodeMapImpl::getLength() const
{
    XMLSize_t count=0;
    for(XMLSize_t index=0;index<MAP_SIZE;index++)
        count+=(fBuckets[index]==0?0:fBuckets[index]->size());
    return count;
}

DOMNode * DOMNamedNodeMapImpl::item(XMLSize_t index) const
{
    XMLSize_t count=0;
    for(XMLSize_t i=0;i<MAP_SIZE;i++) {
        if(fBuckets[i]==0)
            continue;
        XMLSize_t thisBucket=fBuckets[i]->size();
        if(index>=count && index<(count+thisBucket))
            return fBuckets[i]->elementAt(index-count);
        count+=thisBucket;
    }
    return NULL;
}


DOMNode * DOMNamedNodeMapImpl::getNamedItem(const XMLCh *name) const
{
    XMLSize_t hash=XMLString::hash(name,MAP_SIZE);
    if(fBuckets[hash]==0)
        return 0;

    XMLSize_t i = 0;
    XMLSize_t size = fBuckets[hash]->size();
    for (i = 0; i < size; ++i) {
        DOMNode *n=fBuckets[hash]->elementAt(i);
        if(XMLString::equals(name,n->getNodeName()))
            return n;
    }

    return 0;
}


//
// removeNamedItem() - Remove the named item, and return it.
//                      The caller can release the
//                      returned item if it's not used
//                      we can't do it here because the caller would
//                      never see the returned node.
//
DOMNode * DOMNamedNodeMapImpl::removeNamedItem(const XMLCh *name)
{
    if (this->readOnly())
        throw DOMException(
            DOMException::NO_MODIFICATION_ALLOWED_ERR, 0, GetDOMNamedNodeMapMemoryManager);

    XMLSize_t hash=XMLString::hash(name,MAP_SIZE);
    if(fBuckets[hash]==0)
        throw DOMException(DOMException::NOT_FOUND_ERR, 0, GetDOMNamedNodeMapMemoryManager);

    DOMDocument *doc = fOwnerNode->getOwnerDocument();

    XMLSize_t i = 0;
    XMLSize_t size = fBuckets[hash]->size();
    for (i = 0; i < size; ++i) {
        DOMNode *n=fBuckets[hash]->elementAt(i);
        if(XMLString::equals(name,n->getNodeName())) {
            fBuckets[hash]->removeElementAt(i);
            castToNodeImpl(n)->fOwnerNode = doc;
            castToNodeImpl(n)->isOwned(false);
            return n;
        }
    }

    throw DOMException(DOMException::NOT_FOUND_ERR, 0, GetDOMNamedNodeMapMemoryManager);
    return 0;
}



//
// setNamedItem()  Put the item into the NamedNodeList by name.
//                  If an item with the same name already was
//                  in the list, replace it.  Return the old
//                  item, if there was one.
//                  Caller is responsible for arranging for
//                  deletion of the old item if its ref count is
//                  zero.
//
DOMNode * DOMNamedNodeMapImpl::setNamedItem(DOMNode * arg)
{
    DOMDocument *doc = fOwnerNode->getOwnerDocument();
    DOMNodeImpl *argImpl = castToNodeImpl(arg);
    if(argImpl->getOwnerDocument() != doc)
        throw DOMException(DOMException::WRONG_DOCUMENT_ERR,0, GetDOMNamedNodeMapMemoryManager);
    if (this->readOnly())
        throw DOMException(DOMException::NO_MODIFICATION_ALLOWED_ERR, 0, GetDOMNamedNodeMapMemoryManager);
    if ((arg->getNodeType() == DOMNode::ATTRIBUTE_NODE) && argImpl->isOwned() && (argImpl->fOwnerNode != fOwnerNode))
        throw DOMException(DOMException::INUSE_ATTRIBUTE_ERR,0, GetDOMNamedNodeMapMemoryManager);

    argImpl->fOwnerNode = fOwnerNode;
    argImpl->isOwned(true);

    const XMLCh* name=arg->getNodeName();
    XMLSize_t hash=XMLString::hash(name,MAP_SIZE);
    if(fBuckets[hash]==0)
        fBuckets[hash] = new (doc) DOMNodeVector(doc, 3);

    XMLSize_t i = 0;
    XMLSize_t size = fBuckets[hash]->size();
    for (i = 0; i < size; ++i) {
        DOMNode *n=fBuckets[hash]->elementAt(i);
        if(XMLString::equals(name,n->getNodeName())) {
            fBuckets[hash]->setElementAt(arg,i);
            castToNodeImpl(n)->fOwnerNode = doc;
            castToNodeImpl(n)->isOwned(false);
            return n;
        }
    }
    fBuckets[hash]->addElement(arg);
    return 0;
}


void DOMNamedNodeMapImpl::setReadOnly(bool readOnl, bool deep)
{
    // this->fReadOnly=readOnl;
    if(deep) {
        for (XMLSize_t index = 0; index < MAP_SIZE; index++) {
            if(fBuckets[index]==0)
                continue;
            XMLSize_t sz = fBuckets[index]->size();
            for (XMLSize_t i=0; i<sz; ++i)
                castToNodeImpl(fBuckets[index]->elementAt(i))->setReadOnly(readOnl, deep);
        }
    }
}


//Introduced in DOM Level 2

DOMNode *DOMNamedNodeMapImpl::getNamedItemNS(const XMLCh *namespaceURI, const XMLCh *localName) const
{
    // the map is indexed using the full name of nodes; to search given a namespace and a local name
    // we have to do a linear search
    for (XMLSize_t index = 0; index < MAP_SIZE; index++) {
        if(fBuckets[index]==0)
            continue;

        XMLSize_t i = 0;
        XMLSize_t size = fBuckets[index]->size();
        for (i = 0; i < size; ++i) {
            DOMNode *n=fBuckets[index]->elementAt(i);
            const XMLCh * nNamespaceURI = n->getNamespaceURI();
            const XMLCh * nLocalName = n->getLocalName();
            if (!XMLString::equals(nNamespaceURI, namespaceURI))    //URI not match
                continue;
            else {
                if (XMLString::equals(localName, nLocalName)
                    ||
                    (nLocalName == 0 && XMLString::equals(localName, n->getNodeName())))
                    return n;
            }
        }
    }
    return 0;
}


//
// setNamedItemNS()  Put the item into the NamedNodeList by name.
//                  If an item with the same name already was
//                  in the list, replace it.  Return the old
//                  item, if there was one.
//                  Caller is responsible for arranging for
//                  deletion of the old item if its ref count is
//                  zero.
//
DOMNode * DOMNamedNodeMapImpl::setNamedItemNS(DOMNode *arg)
{
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

    const XMLCh* namespaceURI=arg->getNamespaceURI();
    const XMLCh* localName=arg->getLocalName();
    // the map is indexed using the full name of nodes; to search given a namespace and a local name
    // we have to do a linear search
    for (XMLSize_t index = 0; index < MAP_SIZE; index++) {
        if(fBuckets[index]==0)
            continue;

        XMLSize_t i = 0;
        XMLSize_t size = fBuckets[index]->size();
        for (i = 0; i < size; ++i) {
            DOMNode *n=fBuckets[index]->elementAt(i);
            const XMLCh * nNamespaceURI = n->getNamespaceURI();
            const XMLCh * nLocalName = n->getLocalName();
            if (!XMLString::equals(nNamespaceURI, namespaceURI))    //URI not match
                continue;
            else {
                if (XMLString::equals(localName, nLocalName)
                    ||
                    (nLocalName == 0 && XMLString::equals(localName, n->getNodeName()))) {
                    fBuckets[index]->setElementAt(arg,i);
                    castToNodeImpl(n)->fOwnerNode = doc;
                    castToNodeImpl(n)->isOwned(false);
                    return n;
                }
            }
        }
    }
    // if not found, add it using the full name as key
    return setNamedItem(arg);
}


// removeNamedItemNS() - Remove the named item, and return it.
//                      The caller can release the
//                      returned item if it's not used
//                      we can't do it here because the caller would
//                      never see the returned node.
DOMNode *DOMNamedNodeMapImpl::removeNamedItemNS(const XMLCh *namespaceURI,
                                                 const XMLCh *localName)
{
    if (this->readOnly())
        throw DOMException(
        DOMException::NO_MODIFICATION_ALLOWED_ERR, 0, GetDOMNamedNodeMapMemoryManager);

    // the map is indexed using the full name of nodes; to search given a namespace and a local name
    // we have to do a linear search
    for (XMLSize_t index = 0; index < MAP_SIZE; index++) {
        if(fBuckets[index]==0)
            continue;

        DOMDocument *doc = fOwnerNode->getOwnerDocument();
        XMLSize_t i = 0;
        XMLSize_t size = fBuckets[index]->size();
        for (i = 0; i < size; ++i) {
            DOMNode *n=fBuckets[index]->elementAt(i);
            const XMLCh * nNamespaceURI = n->getNamespaceURI();
            const XMLCh * nLocalName = n->getLocalName();
            if (!XMLString::equals(nNamespaceURI, namespaceURI))    //URI not match
                continue;
            else {
                if (XMLString::equals(localName, nLocalName)
                    ||
                    (nLocalName == 0 && XMLString::equals(localName, n->getNodeName()))) {
                    fBuckets[index]->removeElementAt(i);
                    castToNodeImpl(n)->fOwnerNode = doc;
                    castToNodeImpl(n)->isOwned(false);
                    return n;
                }
            }
        }
    }
    throw DOMException(DOMException::NOT_FOUND_ERR, 0, GetDOMNamedNodeMapMemoryManager);
    return 0;
}

XERCES_CPP_NAMESPACE_END
