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
 * $Id: DOMElementImpl.cpp 901107 2010-01-20 08:45:02Z borisk $
 */

#include "DOMElementImpl.hpp"

#include <xercesc/dom/DOMAttr.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/XMLUri.hpp>

#include "DOMAttrMapImpl.hpp"
#include "DOMAttrImpl.hpp"
#include "DOMDocumentImpl.hpp"
#include "DOMParentNode.hpp"
#include "DOMCasts.hpp"
#include "DOMElementNSImpl.hpp"
#include "DOMTypeInfoImpl.hpp"

#include "DOMDocumentTypeImpl.hpp"
#include <xercesc/util/OutOfMemoryException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class DOMAttr;

DOMElementImpl::DOMElementImpl(DOMDocument *ownerDoc, const XMLCh *eName)
    : fNode(ownerDoc), fParent(ownerDoc), fAttributes(0), fDefaultAttributes(0)
{
    DOMDocumentImpl *docImpl = (DOMDocumentImpl *)ownerDoc;
    fName = docImpl->getPooledString(eName);
    setupDefaultAttributes();
    if (!fDefaultAttributes) {
        fDefaultAttributes = new (docImpl) DOMAttrMapImpl(this);
        fAttributes = new (docImpl) DOMAttrMapImpl(this);
    }
    else {
      fAttributes = new (docImpl) DOMAttrMapImpl(this, fDefaultAttributes);
    }
}


DOMElementImpl::DOMElementImpl(const DOMElementImpl &other, bool deep)
    : DOMElement(other),
      fNode (other.fParent.fOwnerDocument),
      fParent (other.fParent.fOwnerDocument),
      fAttributes(0),
      fDefaultAttributes(0)
{
    fName = other.fName;

    if (deep)
        fParent.cloneChildren(&other);

    if (other.getAttributes())
    {
        fAttributes = ((DOMAttrMapImpl *)other.getAttributes())->cloneAttrMap(this);
    }

    if (other.getDefaultAttributes())
    {
        fDefaultAttributes = ((DOMAttrMapImpl *)other.getDefaultAttributes())->cloneAttrMap(this);
    }

    if (!fDefaultAttributes)
        setupDefaultAttributes();

    if (!fDefaultAttributes)
        fDefaultAttributes = new (fParent.fOwnerDocument) DOMAttrMapImpl(this);

    if (!fAttributes) {
        if (!fDefaultAttributes) {
            fAttributes = new (fParent.fOwnerDocument) DOMAttrMapImpl(this);
        }
        else {
            fAttributes = new (fParent.fOwnerDocument) DOMAttrMapImpl(this, fDefaultAttributes);
        }
    }

}


DOMElementImpl::~DOMElementImpl()
{
}


DOMNode *DOMElementImpl::cloneNode(bool deep) const
{
    DOMNode* newNode = new (fParent.fOwnerDocument, DOMMemoryManager::ELEMENT_OBJECT) DOMElementImpl(*this, deep);
    fNode.callUserDataHandlers(DOMUserDataHandler::NODE_CLONED, this, newNode);
    return newNode;
}




const XMLCh * DOMElementImpl::getNodeName() const {
    return fName;
}


DOMNode::NodeType DOMElementImpl::getNodeType() const {
    return DOMNode::ELEMENT_NODE;
}


const XMLCh * DOMElementImpl::getAttribute(const XMLCh *nam) const
{
    DOMNode * attr = fAttributes->getNamedItem(nam);
    if (attr)
        return attr->getNodeValue();

    return XMLUni::fgZeroLenString;
}



DOMAttr *DOMElementImpl::getAttributeNode(const XMLCh *nam) const
{
    return  (DOMAttr *)fAttributes->getNamedItem(nam);
}


DOMNamedNodeMap *DOMElementImpl::getAttributes() const
{
    DOMElementImpl *ncThis = (DOMElementImpl *)this;   // cast off const
    return ncThis->fAttributes;
}



DOMNodeList *DOMElementImpl::getElementsByTagName(const XMLCh *tagname) const
{
    DOMDocumentImpl *docImpl = (DOMDocumentImpl *)fParent.fOwnerDocument;
    return docImpl->getDeepNodeList(this,tagname);
}


const XMLCh * DOMElementImpl::getTagName() const
{
    return fName;
}


void DOMElementImpl::removeAttribute(const XMLCh *nam)
{
    if (fNode.isReadOnly())
        throw DOMException(
             DOMException::NO_MODIFICATION_ALLOWED_ERR, 0, GetDOMNodeMemoryManager);

    int i = fAttributes->findNamePoint(nam);
    if (i >= 0)
    {
        DOMNode *att = fAttributes->removeNamedItemAt(i);
        ((DOMAttrImpl *)att)->removeAttrFromIDNodeMap();
        att->release();
    }
}



DOMAttr *DOMElementImpl::removeAttributeNode(DOMAttr *oldAttr)
{
    if (fNode.isReadOnly())
        throw DOMException(
        DOMException::NO_MODIFICATION_ALLOWED_ERR, 0, GetDOMNodeMemoryManager);

    DOMNode* found = 0;

    // Since there is no removeAttributeNodeNS, check if this oldAttr has NS or not
    const XMLCh* localName = oldAttr->getLocalName();
    int i = 0;
    if (localName)
        i = fAttributes->findNamePoint(oldAttr->getNamespaceURI(), localName);
    else
        i = fAttributes->findNamePoint(oldAttr->getName());

    if (i >= 0) {
        // If it is in fact the right object, remove it.
        found = fAttributes->item(i);
        if (found == oldAttr) {
            fAttributes->removeNamedItemAt(i);
            ((DOMAttrImpl *)oldAttr)->removeAttrFromIDNodeMap();
        }
        else
            throw DOMException(DOMException::NOT_FOUND_ERR, 0, GetDOMNodeMemoryManager);

    }
    else
        throw DOMException(DOMException::NOT_FOUND_ERR, 0, GetDOMNodeMemoryManager);

   return (DOMAttr *)found;
}



void DOMElementImpl::setAttribute(const XMLCh *nam, const XMLCh *val)
{
    if (fNode.isReadOnly())
        throw DOMException(
        DOMException::NO_MODIFICATION_ALLOWED_ERR, 0, GetDOMNodeMemoryManager);

    DOMAttr* newAttr = getAttributeNode(nam);
    if (!newAttr)
    {
        newAttr = fParent.fOwnerDocument->createAttribute(nam);
        fAttributes->setNamedItem(newAttr);
    }

    newAttr->setNodeValue(val);
}

void DOMElementImpl::setIdAttribute(const XMLCh* name, bool isId)
{
    if (fNode.isReadOnly())
        throw DOMException(
        DOMException::NO_MODIFICATION_ALLOWED_ERR, 0, GetDOMNodeMemoryManager);

    DOMAttr *attr = getAttributeNode(name);

    if (!attr)
        throw DOMException(DOMException::NOT_FOUND_ERR, 0, GetDOMNodeMemoryManager);

    if(isId)
        ((DOMAttrImpl *)attr)->addAttrToIDNodeMap();
    else
        ((DOMAttrImpl *)attr)->removeAttrFromIDNodeMap();
}

void DOMElementImpl::setIdAttributeNS(const XMLCh* namespaceURI, const XMLCh* localName, bool isId) {

    if (fNode.isReadOnly())
        throw DOMException(
        DOMException::NO_MODIFICATION_ALLOWED_ERR, 0, GetDOMNodeMemoryManager);

    DOMAttr *attr = getAttributeNodeNS(namespaceURI, localName);

    if (!attr)
        throw DOMException(DOMException::NOT_FOUND_ERR, 0, GetDOMNodeMemoryManager);

    if(isId)
        ((DOMAttrImpl *)attr)->addAttrToIDNodeMap();
    else
        ((DOMAttrImpl *)attr)->removeAttrFromIDNodeMap();
}


void DOMElementImpl::setIdAttributeNode(const DOMAttr *idAttr, bool isId) {

    if (fNode.isReadOnly())
        throw DOMException(
        DOMException::NO_MODIFICATION_ALLOWED_ERR, 0, GetDOMNodeMemoryManager);

    DOMAttr *attr;
    const XMLCh* localName = idAttr->getLocalName();
    if (localName)
        attr = getAttributeNodeNS(idAttr->getNamespaceURI(), idAttr->getLocalName());
    else
        attr = getAttributeNode(idAttr->getName());

    if(!attr)
        throw DOMException(DOMException::NOT_FOUND_ERR, 0, GetDOMNodeMemoryManager);

    if(isId)
        ((DOMAttrImpl *)attr)->addAttrToIDNodeMap();
    else
        ((DOMAttrImpl *)attr)->removeAttrFromIDNodeMap();
}


DOMAttr * DOMElementImpl::setAttributeNode(DOMAttr *newAttr)
{
    if (fNode.isReadOnly())
        throw DOMException(
        DOMException::NO_MODIFICATION_ALLOWED_ERR, 0, GetDOMNodeMemoryManager);

    if (newAttr->getNodeType() != DOMNode::ATTRIBUTE_NODE)
        throw DOMException(DOMException::WRONG_DOCUMENT_ERR, 0, GetDOMNodeMemoryManager);
        // revisit.  Exception doesn't match test.

    // This will throw INUSE if necessary
    DOMAttr *oldAttr = (DOMAttr *) fAttributes->setNamedItem(newAttr);

    return oldAttr;
}


void DOMElementImpl::setNodeValue(const XMLCh *x)
{
    fNode.setNodeValue(x);
}



void DOMElementImpl::setReadOnly(bool readOnl, bool deep)
{
    fNode.setReadOnly(readOnl,deep);
    fAttributes->setReadOnly(readOnl,true);
}


//Introduced in DOM Level 2
const XMLCh * DOMElementImpl::getAttributeNS(const XMLCh *fNamespaceURI,
    const XMLCh *fLocalName) const
{
    DOMAttr * attr=
      (DOMAttr *)(fAttributes->getNamedItemNS(fNamespaceURI, fLocalName));
    return (attr==0) ? XMLUni::fgZeroLenString : attr->getValue();
}


void DOMElementImpl::setAttributeNS(const XMLCh *fNamespaceURI,
    const XMLCh *qualifiedName, const XMLCh *fValue)
{
    if (fNode.isReadOnly())
        throw DOMException(DOMException::NO_MODIFICATION_ALLOWED_ERR, 0, GetDOMNodeMemoryManager);

    int index = DOMDocumentImpl::indexofQualifiedName(qualifiedName);
    if (index < 0)
        throw DOMException(DOMException::NAMESPACE_ERR, 0, GetDOMNodeMemoryManager);

    DOMAttr* newAttr = getAttributeNodeNS(fNamespaceURI, qualifiedName+index);
    if (!newAttr)
    {
        newAttr = fParent.fOwnerDocument->createAttributeNS(fNamespaceURI, qualifiedName);
        fAttributes->setNamedItemNS(newAttr);
    }

    newAttr->setNodeValue(fValue);
}


void DOMElementImpl::removeAttributeNS(const XMLCh *fNamespaceURI,
    const XMLCh *fLocalName)
{
    if (fNode.isReadOnly())
        throw DOMException(
        DOMException::NO_MODIFICATION_ALLOWED_ERR, 0, GetDOMNodeMemoryManager);

    int i = fAttributes->findNamePoint(fNamespaceURI, fLocalName);
    if (i >= 0)
    {
        DOMNode *att = fAttributes->removeNamedItemAt(i);
        att->release();
    }
}


DOMAttr *DOMElementImpl::getAttributeNodeNS(const XMLCh *fNamespaceURI,
    const XMLCh *fLocalName) const
{
    return (DOMAttr *)fAttributes->getNamedItemNS(fNamespaceURI, fLocalName);
}


DOMAttr *DOMElementImpl::setAttributeNodeNS(DOMAttr *newAttr)
{
    if (fNode.isReadOnly())
        throw DOMException(
            DOMException::NO_MODIFICATION_ALLOWED_ERR, 0, GetDOMNodeMemoryManager);

    if (newAttr -> getOwnerDocument() != fParent.fOwnerDocument)
        throw DOMException(DOMException::WRONG_DOCUMENT_ERR, 0, GetDOMNodeMemoryManager);

    // This will throw INUSE if necessary
    DOMAttr *oldAttr = (DOMAttr *) fAttributes->setNamedItemNS(newAttr);

    return oldAttr;
}


DOMNodeList *DOMElementImpl::getElementsByTagNameNS(const XMLCh *namespaceURI,
    const XMLCh *localName) const
{
    DOMDocumentImpl *docImpl = (DOMDocumentImpl *)fParent.fOwnerDocument;
    return docImpl->getDeepNodeList(this, namespaceURI, localName);
}

bool DOMElementImpl::hasAttributes() const
{
    return (fAttributes != 0 && fAttributes->getLength() != 0);
}


bool DOMElementImpl::hasAttribute(const XMLCh *name) const
{
    return (getAttributeNode(name) != 0);
}


bool DOMElementImpl::hasAttributeNS(const XMLCh *namespaceURI,
    const XMLCh *localName) const
{
    return (getAttributeNodeNS(namespaceURI, localName) != 0);
}


// util functions for default attributes
// returns the default attribute map for this node from the owner document
DOMAttrMapImpl *DOMElementImpl::getDefaultAttributes() const
{
    return fDefaultAttributes;
}

// initially set up the default attribute information based on doctype information
void DOMElementImpl::setupDefaultAttributes()
{
    DOMDocument *tmpdoc = fParent.fOwnerDocument;
    if ((fNode.fOwnerNode == 0) || (tmpdoc == 0) || (tmpdoc->getDoctype() == 0))
        return;

    DOMNode *eldef = ((DOMDocumentTypeImpl*)tmpdoc->getDoctype())->getElements()->getNamedItem(getNodeName());
    DOMAttrMapImpl* defAttrs = (eldef == 0) ? 0 : (DOMAttrMapImpl *)(eldef->getAttributes());

    if (defAttrs)
        fDefaultAttributes = new (tmpdoc) DOMAttrMapImpl(this, defAttrs);
}

DOMAttr * DOMElementImpl::setDefaultAttributeNode(DOMAttr *newAttr)
{
    if (fNode.isReadOnly())
        throw DOMException(
        DOMException::NO_MODIFICATION_ALLOWED_ERR, 0, GetDOMNodeMemoryManager);

    if (newAttr->getNodeType() != DOMNode::ATTRIBUTE_NODE)
        throw DOMException(DOMException::WRONG_DOCUMENT_ERR, 0, GetDOMNodeMemoryManager);
        // revisit.  Exception doesn't match test.

    // This will throw INUSE if necessary
    DOMAttr *oldAttr = (DOMAttr *) fDefaultAttributes->setNamedItem(newAttr);
    fAttributes->hasDefaults(true);

    return oldAttr;
}


DOMAttr *DOMElementImpl::setDefaultAttributeNodeNS(DOMAttr *newAttr)
{
    if (fNode.isReadOnly())
        throw DOMException(
            DOMException::NO_MODIFICATION_ALLOWED_ERR, 0, GetDOMNodeMemoryManager);

    if (newAttr -> getOwnerDocument() != fParent.fOwnerDocument)
        throw DOMException(DOMException::WRONG_DOCUMENT_ERR, 0, GetDOMNodeMemoryManager);

    // This will throw INUSE if necessary
    DOMAttr *oldAttr = (DOMAttr *) fDefaultAttributes->setNamedItemNS(newAttr);
    fAttributes->hasDefaults(true);

    return oldAttr;
}

void DOMElementImpl::release()
{
    if (fNode.isOwned() && !fNode.isToBeReleased())
        throw DOMException(DOMException::INVALID_ACCESS_ERR,0, GetDOMNodeMemoryManager);

    DOMDocumentImpl* doc = (DOMDocumentImpl*) fParent.fOwnerDocument;
    if (doc) {
        fNode.callUserDataHandlers(DOMUserDataHandler::NODE_DELETED, 0, 0);
        // release children
        fParent.release();
        // release attributes
        fAttributes->hasDefaults(false);
        XMLSize_t count;
        while((count = fAttributes->getLength()) != 0)
        {
            DOMNode* attr = fAttributes->removeNamedItemAt(count-1);
            attr->release();
        }

        doc->release(this, DOMMemoryManager::ELEMENT_OBJECT);
    }
    else {
        // shouldn't reach here
        throw DOMException(DOMException::INVALID_ACCESS_ERR,0, GetDOMNodeMemoryManager);
    }
}

const XMLCh* DOMElementImpl::getBaseURI() const
{
    const XMLCh* baseURI = fNode.fOwnerNode->getBaseURI();
    if (fAttributes) {
        const XMLCh baseString[] =
        {
            chLatin_b, chLatin_a, chLatin_s, chLatin_e, chNull
        };
        DOMNode* attrNode = fAttributes->getNamedItemNS(DOMNodeImpl::getXmlURIString(), baseString);
        if (attrNode==NULL) {
            const XMLCh xmlBaseString[] =
            {
                chLatin_x, chLatin_m, chLatin_l, chColon, chLatin_b, chLatin_a, chLatin_s, chLatin_e, chNull
            };
            attrNode = fAttributes->getNamedItem(xmlBaseString);
        }
        if (attrNode) {
            const XMLCh* uri =  attrNode->getNodeValue();
            if (uri && *uri) {// attribute value is always empty string
                // if there is a base URI for the parent node, use it to resolve relative URI
                if(baseURI)
                {
                    try {
                      DOMDocumentImpl* doc = (DOMDocumentImpl *)fParent.fOwnerDocument;
                      XMLUri temp(baseURI, doc->getMemoryManager());
                      XMLUri temp2(&temp, uri, doc->getMemoryManager());
                      uri = doc->cloneString(temp2.getUriText());
                    }
                    catch(const OutOfMemoryException&)
                    {
                        throw;
                    }
                    catch (...){
                        // REVISIT: what should happen in this case?
                        return 0;
                    }
                }
                return uri;
            }
        }
    }
    return baseURI;
}



//
//   Functions inherited from Node
//
           DOMNode*         DOMElementImpl::appendChild(DOMNode *newChild)          {return fParent.appendChild (newChild); }
           DOMNodeList*     DOMElementImpl::getChildNodes() const                   {return fParent.getChildNodes (); }
           DOMNode*         DOMElementImpl::getFirstChild() const                   {return fParent.getFirstChild (); }
           DOMNode*         DOMElementImpl::getLastChild() const                    {return fParent.getLastChild (); }
     const XMLCh*           DOMElementImpl::getLocalName() const                    {return fNode.getLocalName (); }
     const XMLCh*           DOMElementImpl::getNamespaceURI() const                 {return fNode.getNamespaceURI (); }
           DOMNode*         DOMElementImpl::getNextSibling() const                  {return fChild.getNextSibling (); }
     const XMLCh*           DOMElementImpl::getNodeValue() const                    {return fNode.getNodeValue (); }
           DOMDocument*     DOMElementImpl::getOwnerDocument() const                {return fParent.fOwnerDocument; }
     const XMLCh*           DOMElementImpl::getPrefix() const                       {return fNode.getPrefix (); }
           DOMNode*         DOMElementImpl::getParentNode() const                   {return fChild.getParentNode (this); }
           DOMNode*         DOMElementImpl::getPreviousSibling() const              {return fChild.getPreviousSibling (this); }
           bool             DOMElementImpl::hasChildNodes() const                   {return fParent.hasChildNodes (); }
           DOMNode*         DOMElementImpl::insertBefore(DOMNode *newChild, DOMNode *refChild)
                                                                                    {return fParent.insertBefore (newChild, refChild); }
           void             DOMElementImpl::normalize()                             {fParent.normalize (); }
           DOMNode*         DOMElementImpl::removeChild(DOMNode *oldChild)          {return fParent.removeChild (oldChild); }
           DOMNode*         DOMElementImpl::replaceChild(DOMNode *newChild, DOMNode *oldChild)
                                                                                    {return fParent.replaceChild (newChild, oldChild); }
           bool             DOMElementImpl::isSupported(const XMLCh *feature, const XMLCh *version) const
                                                                                    {return fNode.isSupported (feature, version); }
           void             DOMElementImpl::setPrefix(const XMLCh  *prefix)         {fNode.setPrefix(prefix); }
           bool             DOMElementImpl::isSameNode(const DOMNode* other) const  {return fNode.isSameNode(other); }
           void*            DOMElementImpl::setUserData(const XMLCh* key, void* data, DOMUserDataHandler* handler)
                                                                                    {return fNode.setUserData(key, data, handler); }
           void*            DOMElementImpl::getUserData(const XMLCh* key) const     {return fNode.getUserData(key); }
           short            DOMElementImpl::compareDocumentPosition(const DOMNode* other) const {return fNode.compareDocumentPosition(other); }
           const XMLCh*     DOMElementImpl::getTextContent() const                  {return fNode.getTextContent(); }
           void             DOMElementImpl::setTextContent(const XMLCh* textContent){fNode.setTextContent(textContent); }
           const XMLCh*     DOMElementImpl::lookupPrefix(const XMLCh* namespaceURI) const  {return fNode.lookupPrefix(namespaceURI); }
           bool             DOMElementImpl::isDefaultNamespace(const XMLCh* namespaceURI) const {return fNode.isDefaultNamespace(namespaceURI); }
           const XMLCh*     DOMElementImpl::lookupNamespaceURI(const XMLCh* prefix) const  {return fNode.lookupNamespaceURI(prefix); }
           void*            DOMElementImpl::getFeature(const XMLCh* feature, const XMLCh* version) const {return fNode.getFeature(feature, version); }



bool DOMElementImpl::isEqualNode(const DOMNode* arg) const
{
    if (isSameNode(arg)) {
        return true;
    }

    if (!fNode.isEqualNode(arg)) {
        return false;
    }

    bool hasAttrs = hasAttributes();

    if (hasAttrs != arg->hasAttributes()) {
        return false;
    }

    if (hasAttrs) {
        DOMNamedNodeMap* map1 = getAttributes();
        DOMNamedNodeMap* map2 = arg->getAttributes();

        XMLSize_t len = map1->getLength();
        if (len != map2->getLength()) {
            return false;
        }
        for (XMLSize_t i = 0; i < len; i++) {
            DOMNode* n1 = map1->item(i);
            if (!n1->getLocalName()) { // DOM Level 1 Node
                DOMNode* n2 = map2->getNamedItem(n1->getNodeName());
                if (!n2 || !n1->isEqualNode(n2)) {
                    return false;
                }
            }
            else {
                DOMNode* n2 = map2->getNamedItemNS(n1->getNamespaceURI(),
                                              n1->getLocalName());
                if (!n2 || !n1->isEqualNode(n2)) {
                    return false;
                }
            }
        }
    }

    return fParent.isEqualNode(arg);
}

DOMNode* DOMElementImpl::rename(const XMLCh* namespaceURI, const XMLCh* name)
{
    DOMDocumentImpl* doc = (DOMDocumentImpl*) fParent.fOwnerDocument;

    if (!namespaceURI || !*namespaceURI) {
        fName = doc->getPooledString(name);
        fAttributes->reconcileDefaultAttributes(getDefaultAttributes());

        // and fire user data NODE_RENAMED event
        castToNodeImpl(this)->callUserDataHandlers(DOMUserDataHandler::NODE_RENAMED, this, this);

        return this;
    }
    else {

        // create a new ElementNS
        DOMElementNSImpl* newElem = (DOMElementNSImpl*)doc->createElementNS(namespaceURI, name);

        // transfer the userData
        doc->transferUserData(castToNodeImpl(this), castToNodeImpl(newElem));

        // remove old node from parent if any
        DOMNode* parent = getParentNode();
        DOMNode* nextSib = getNextSibling();
        if (parent) {
            parent->removeChild(this);
        }

        // move children to new node
        DOMNode* child = getFirstChild();
        while (child) {
            removeChild(child);
            newElem->appendChild(child);
            child = getFirstChild();
        }

        // insert new node where old one was
        if (parent) {
            parent->insertBefore(newElem, nextSib);
        }

        // move specified attributes to new node
        newElem->fAttributes->moveSpecifiedAttributes(fAttributes);

        // and fire user data NODE_RENAMED event
        castToNodeImpl(newElem)->callUserDataHandlers(DOMUserDataHandler::NODE_RENAMED, this, newElem);

        return newElem;
    }
}

const DOMTypeInfo *DOMElementImpl::getSchemaTypeInfo() const
{
    return &DOMTypeInfoImpl::g_DtdValidatedElement;
}

// DOMElementTraversal
DOMElement * DOMElementImpl::getFirstElementChild() const
{
    DOMNode* n = getFirstChild();
    while (n != NULL) {
        switch (n->getNodeType()) {
            case DOMNode::ELEMENT_NODE:
                return (DOMElement*) n;
            case DOMNode::ENTITY_REFERENCE_NODE:
                {
                    DOMElement* e = getFirstElementChild(n);
                    if (e != NULL)
                        return e;
                }
                break;
            default:
                break;
        }
        n = n->getNextSibling();
    }
    return NULL;
}

DOMElement * DOMElementImpl::getLastElementChild() const
{
    DOMNode* n = getLastChild();
    while (n != NULL) {
        switch (n->getNodeType()) {
            case DOMNode::ELEMENT_NODE:
                return (DOMElement*) n;
            case DOMNode::ENTITY_REFERENCE_NODE:
                {
                    DOMElement* e = getLastElementChild(n);
                    if (e != NULL)
                        return e;
                }
                break;
            default:
                break;
        }
        n = n->getPreviousSibling();
    }
    return NULL;
}

DOMElement * DOMElementImpl::getNextElementSibling() const
{
    DOMNode* n = getNextLogicalSibling(this);
    while (n != NULL) {
        switch (n->getNodeType()) {
            case DOMNode::ELEMENT_NODE:
                return (DOMElement*) n;
            case DOMNode::ENTITY_REFERENCE_NODE:
                {
                    DOMElement* e = getFirstElementChild(n);
                    if (e != NULL)
                        return e;
                }
                break;
            default:
                break;
        }
        n = getNextLogicalSibling(n);
    }
    return NULL;
}

DOMElement * DOMElementImpl::getPreviousElementSibling() const
{
    DOMNode* n = getPreviousLogicalSibling(this);
    while (n != NULL) {
        switch (n->getNodeType()) {
            case DOMNode::ELEMENT_NODE:
                return (DOMElement*) n;
            case DOMNode::ENTITY_REFERENCE_NODE:
                {
                    DOMElement* e = getLastElementChild(n);
                    if (e != NULL)
                        return e;
                }
                break;
            default:
                break;
        }
        n = getPreviousLogicalSibling(n);
    }
    return NULL;
}

XMLSize_t DOMElementImpl::getChildElementCount() const
{
    XMLSize_t count = 0;
    DOMElement* child = getFirstElementChild();
    while (child != NULL) {
        ++count;
        child = child->getNextElementSibling();
    }
    return count;
}

// Returns the first element node found from a
// non-recursive in order traversal of the given node.
DOMElement* DOMElementImpl::getFirstElementChild(const DOMNode* n) const
{
    const DOMNode* top = n;
    while (n != NULL) {
        if (n->getNodeType() == DOMNode::ELEMENT_NODE) {
            return (DOMElement*) n;
        }
        DOMNode* next = n->getFirstChild();
        while (next == NULL) {
            if (top == n) {
                break;
            }
            next = n->getNextSibling();
            if (next == NULL) {
                n = n->getParentNode();
                if (n == NULL || top == n) {
                    return NULL;
                }
            }
        }
        n = next;
    }
    return NULL;
}

// Returns the first element node found from a
// non-recursive reverse order traversal of the given node.
DOMElement* DOMElementImpl::getLastElementChild(const DOMNode* n) const
{
    const DOMNode* top = n;
    while (n != NULL) {
        if (n->getNodeType() == DOMNode::ELEMENT_NODE) {
            return (DOMElement*) n;
        }
        DOMNode* next = n->getLastChild();
        while (next == NULL) {
            if (top == n) {
                break;
            }
            next = n->getPreviousSibling();
            if (next == NULL) {
                n = n->getParentNode();
                if (n == NULL || top == n) {
                    return NULL;
                }
            }
        }
        n = next;
    }
    return NULL;
}

// Returns the next logical sibling with respect to the given node.
DOMNode* DOMElementImpl::getNextLogicalSibling(const DOMNode* n) const
{
    DOMNode* next = n->getNextSibling();
    // If "n" has no following sibling and its parent is an entity reference node we
    // need to continue the search through the following siblings of the entity
    // reference as these are logically siblings of the given node.
    if (next == NULL) {
        DOMNode* parent = n->getParentNode();
        while (parent != NULL && parent->getNodeType() == DOMNode::ENTITY_REFERENCE_NODE) {
            next = parent->getNextSibling();
            if (next != NULL) {
                break;
            }
            parent = parent->getParentNode();
        }
    }
    return next;
}

// Returns the previous logical sibling with respect to the given node.
DOMNode* DOMElementImpl::getPreviousLogicalSibling(const DOMNode* n) const
{
    DOMNode* prev = n->getPreviousSibling();
    // If "n" has no previous sibling and its parent is an entity reference node we
    // need to continue the search through the previous siblings of the entity
    // reference as these are logically siblings of the given node.
    if (prev == NULL) {
        DOMNode* parent = n->getParentNode();
        while (parent != NULL && parent->getNodeType() == DOMNode::ENTITY_REFERENCE_NODE) {
            prev = parent->getPreviousSibling();
            if (prev != NULL) {
                break;
            }
            parent = parent->getParentNode();
        }
    }
    return prev;
}


XERCES_CPP_NAMESPACE_END
