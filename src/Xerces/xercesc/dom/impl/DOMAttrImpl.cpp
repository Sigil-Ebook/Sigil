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
 * $Id: DOMAttrImpl.cpp 678709 2008-07-22 10:56:56Z borisk $
 */

#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMException.hpp>

#include "DOMAttrImpl.hpp"
#include "DOMStringPool.hpp"
#include "DOMDocumentImpl.hpp"
#include "DOMCasts.hpp"
#include "DOMTypeInfoImpl.hpp"

XERCES_CPP_NAMESPACE_BEGIN

DOMAttrImpl::DOMAttrImpl(DOMDocument *ownerDoc, const XMLCh *aName)
    : fNode(ownerDoc), fParent (ownerDoc), fSchemaType(0)
{
    DOMDocumentImpl *docImpl = (DOMDocumentImpl *)ownerDoc;
    fName = docImpl->getPooledString(aName);
    fNode.isSpecified(true);
}

DOMAttrImpl::DOMAttrImpl(const DOMAttrImpl &other, bool /*deep*/)
    : DOMAttr(other)
    , fNode(other.fNode)
    , fParent (other.fParent)
    , fName(other.fName)
    , fSchemaType(other.fSchemaType)
{
    if (other.fNode.isSpecified())
        fNode.isSpecified(true);
    else
        fNode.isSpecified(false);

    if (other.fNode.isIdAttr())
    {
        fNode.isIdAttr(true);
        DOMDocumentImpl *doc = (DOMDocumentImpl *)fParent.fOwnerDocument;
        doc->getNodeIDMap()->add(this);
    }

    fParent.cloneChildren(&other);
}


DOMAttrImpl::~DOMAttrImpl() {
}


DOMNode * DOMAttrImpl::cloneNode(bool deep) const
{
    DOMNode* newNode = new (fParent.fOwnerDocument, DOMDocumentImpl::ATTR_OBJECT) DOMAttrImpl(*this, deep);
    fNode.callUserDataHandlers(DOMUserDataHandler::NODE_CLONED, this, newNode);
    return newNode;
}


const XMLCh * DOMAttrImpl::getNodeName()  const{
    return fName;
}

DOMNode::NodeType DOMAttrImpl::getNodeType() const {
    return DOMNode::ATTRIBUTE_NODE;
}


const XMLCh * DOMAttrImpl::getName() const {
    return fName;
}


const XMLCh * DOMAttrImpl::getNodeValue() const
{
    return getValue();
}


bool DOMAttrImpl::getSpecified() const
{
    return fNode.isSpecified();
}




const XMLCh * DOMAttrImpl::getValue() const
{
    if (fParent.fFirstChild == 0) {
        return XMLUni::fgZeroLenString; // return "";
    }

    // Simple case where attribute value is just a single text node
    DOMNode *node = castToChildImpl(fParent.fFirstChild)->nextSibling;
    if (node == 0 && fParent.fFirstChild->getNodeType() == DOMNode::TEXT_NODE) {
        return fParent.fFirstChild->getNodeValue();
    }

    //
    // Complicated case where attribute value is a DOM tree
    //
    // According to the spec, the child nodes of the Attr node may be either
    // Text or EntityReference nodes.
    //
    // The parser will not create such thing, this is for those created by users.
    //
    // In such case, we have to visit each child to retrieve the text
    //

    DOMDocumentImpl* doc = (DOMDocumentImpl*)fParent.fOwnerDocument;

    XMLBuffer buf(1023, doc->getMemoryManager());
    for (node = fParent.fFirstChild; node != 0; node = castToChildImpl(node)->nextSibling)
        getTextValue(node, buf);

    return doc->getPooledString(buf.getRawBuffer());
}

void DOMAttrImpl::getTextValue(DOMNode* node, XMLBuffer& buf) const
{
    if (node->getNodeType() == DOMNode::TEXT_NODE)
        buf.append(node->getNodeValue());
    else if (node->getNodeType() == DOMNode::ENTITY_REFERENCE_NODE)
    {
        for (node = node->getFirstChild(); node != 0; node = castToChildImpl(node)->nextSibling)
        {
            getTextValue(node, buf);
        }
    }

    return;
}


void DOMAttrImpl::setNodeValue(const XMLCh *val)
{
    setValue(val);
}



void DOMAttrImpl::setSpecified(bool arg)
{
    fNode.isSpecified(arg);
}



void DOMAttrImpl::setValue(const XMLCh *val)
{
    if (fNode.isReadOnly())
    {
        throw DOMException(DOMException::NO_MODIFICATION_ALLOWED_ERR, 0, GetDOMNodeMemoryManager);
    }

    //  If this attribute was of type ID and in the map, take it out,
    //    then put it back in with the new name.  For now, we don't worry
    //    about what happens if the new name conflicts
    //
    DOMDocumentImpl *doc = (DOMDocumentImpl *)fParent.fOwnerDocument;
    if (fNode.isIdAttr())
        doc->getNodeIDMap()->remove(this);

    DOMNode *kid;
    while ((kid = fParent.fFirstChild) != 0)         // Remove existing kids
    {
        DOMNode* node = removeChild(kid);
        if (node)
            node->release();
    }

    if (val != 0)              // Create and add the new one
        fParent.appendChildFast(doc->createTextNode(val));
    fNode.isSpecified(true);
    fParent.changed();

    if (fNode.isIdAttr())
        doc->getNodeIDMap()->add(this);

}

void DOMAttrImpl::setValueFast(const XMLCh *val)
{
    if (val != 0)
      fParent.appendChildFast(fParent.fOwnerDocument->createTextNode(val));

    fNode.isSpecified (true);
}



//Introduced in DOM Level 2

DOMElement *DOMAttrImpl::getOwnerElement() const
{
    // if we have an owner, ownerNode is our ownerElement, otherwise it's
    // our ownerDocument and we don't have an ownerElement
    return (DOMElement *) (fNode.isOwned() ? fNode.fOwnerNode : 0);
}


//internal use by parser only
void DOMAttrImpl::setOwnerElement(DOMElement *ownerElem)
{
    fNode.fOwnerNode = ownerElem;
    // revisit.  Is this backwards?  isOwned(true)?
    fNode.isOwned(false);
}


//For DOM Level 3

void DOMAttrImpl::release()
{
    if (fNode.isOwned() && !fNode.isToBeReleased())
        throw DOMException(DOMException::INVALID_ACCESS_ERR,0, GetDOMNodeMemoryManager);

    DOMDocumentImpl* doc = (DOMDocumentImpl*)fParent.fOwnerDocument;
    if (doc) {
        fNode.callUserDataHandlers(DOMUserDataHandler::NODE_DELETED, 0, 0);
        fParent.release();
        doc->release(this, DOMMemoryManager::ATTR_OBJECT);
    }
    else {
        // shouldn't reach here
        throw DOMException(DOMException::INVALID_ACCESS_ERR,0, GetDOMNodeMemoryManager);
    }
}


bool DOMAttrImpl::isId() const {
    return fNode.isIdAttr();
}


DOMNode* DOMAttrImpl::rename(const XMLCh* namespaceURI, const XMLCh* name)
{
    DOMElement* el = getOwnerElement();
    DOMDocumentImpl* doc = (DOMDocumentImpl*)fParent.fOwnerDocument;

    if (el)
        el->removeAttributeNode(this);

    if (!namespaceURI || !*namespaceURI) {
        fName = doc->getPooledString(name);

        if (el)
            el->setAttributeNode(this);

        // and fire user data NODE_RENAMED event
        castToNodeImpl(this)->callUserDataHandlers(DOMUserDataHandler::NODE_RENAMED, this, this);

        return this;
    }
    else {

        // create a new AttrNS
        DOMAttr* newAttr = doc->createAttributeNS(namespaceURI, name);

        // transfer the userData
        doc->transferUserData(castToNodeImpl(this), castToNodeImpl(newAttr));

        // move children to new node
        DOMNode* child = getFirstChild();
        while (child) {
            removeChild(child);
            newAttr->appendChild(child);
            child = getFirstChild();
        }

        // reattach attr to element
        if (el)
            el->setAttributeNodeNS(newAttr);

        // and fire user data NODE_RENAMED event
        castToNodeImpl(newAttr)->callUserDataHandlers(DOMUserDataHandler::NODE_RENAMED, this, newAttr);

        return newAttr;
    }
}

const DOMTypeInfo *DOMAttrImpl::getSchemaTypeInfo() const
{
    if(!fSchemaType)
        return &DOMTypeInfoImpl::g_DtdNotValidatedAttribute;

    return fSchemaType;
}


void DOMAttrImpl::setSchemaTypeInfo(const DOMTypeInfoImpl* typeInfo)
{
    fSchemaType = typeInfo;
}

bool DOMAttrImpl::isSupported(const XMLCh *feature, const XMLCh *version) const
{
    // check for '+DOMPSVITypeInfo'
    if(feature && *feature=='+' && XMLString::equals(feature+1, XMLUni::fgXercescInterfacePSVITypeInfo))
        return true;
    return fNode.isSupported (feature, version);
}

void* DOMAttrImpl::getFeature(const XMLCh* feature, const XMLCh* version) const
{
    if(XMLString::equals(feature, XMLUni::fgXercescInterfacePSVITypeInfo))
        return (DOMPSVITypeInfo*)fSchemaType;
    return fNode.getFeature(feature, version);
}

           DOMNode*         DOMAttrImpl::appendChild(DOMNode *newChild)          {return fParent.appendChild (newChild); }
           DOMNamedNodeMap* DOMAttrImpl::getAttributes() const                   {return fNode.getAttributes (); }
           DOMNodeList*     DOMAttrImpl::getChildNodes() const                   {return fParent.getChildNodes (); }
           DOMNode*         DOMAttrImpl::getFirstChild() const                   {return fParent.getFirstChild (); }
           DOMNode*         DOMAttrImpl::getLastChild() const                    {return fParent.getLastChild (); }
     const XMLCh*           DOMAttrImpl::getLocalName() const                    {return fNode.getLocalName (); }
     const XMLCh*           DOMAttrImpl::getNamespaceURI() const                 {return fNode.getNamespaceURI (); }
           DOMNode*         DOMAttrImpl::getNextSibling() const                  {return fNode.getNextSibling (); }
           DOMDocument*     DOMAttrImpl::getOwnerDocument() const                {return fParent.fOwnerDocument; }
     const XMLCh*           DOMAttrImpl::getPrefix() const                       {return fNode.getPrefix (); }
           DOMNode*         DOMAttrImpl::getParentNode() const                   {return fNode.getParentNode (); }
           DOMNode*         DOMAttrImpl::getPreviousSibling() const              {return fNode.getPreviousSibling (); }
           bool             DOMAttrImpl::hasChildNodes() const                   {return fParent.hasChildNodes (); }
           DOMNode*         DOMAttrImpl::insertBefore(DOMNode *newChild, DOMNode *refChild)
                                                                                 {return fParent.insertBefore (newChild, refChild); }
           void             DOMAttrImpl::normalize()                             {fParent.normalize (); }
           DOMNode*         DOMAttrImpl::removeChild(DOMNode *oldChild)          {return fParent.removeChild (oldChild); }
           DOMNode*         DOMAttrImpl::replaceChild(DOMNode *newChild, DOMNode *oldChild)
                                                                                 {return fParent.replaceChild (newChild, oldChild); }
           void             DOMAttrImpl::setPrefix(const XMLCh  *prefix)         {fNode.setPrefix(prefix); }
           bool             DOMAttrImpl::hasAttributes() const                   {return fNode.hasAttributes(); }
           bool             DOMAttrImpl::isSameNode(const DOMNode* other) const  {return fNode.isSameNode(other); }
           bool             DOMAttrImpl::isEqualNode(const DOMNode* arg) const   {return fParent.isEqualNode(arg); }
           void*            DOMAttrImpl::setUserData(const XMLCh* key, void* data, DOMUserDataHandler* handler)
                                                                                 {return fNode.setUserData(key, data, handler); }
           void*            DOMAttrImpl::getUserData(const XMLCh* key) const     {return fNode.getUserData(key); }
           const XMLCh*     DOMAttrImpl::getBaseURI() const                      {return fNode.getBaseURI(); }
           short            DOMAttrImpl::compareDocumentPosition(const DOMNode* other) const {return fNode.compareDocumentPosition(other); }
           const XMLCh*     DOMAttrImpl::getTextContent() const                  {return fNode.getTextContent(); }
           void             DOMAttrImpl::setTextContent(const XMLCh* textContent){fNode.setTextContent(textContent); }
           const XMLCh*     DOMAttrImpl::lookupPrefix(const XMLCh* namespaceURI) const  {return fNode.lookupPrefix(namespaceURI); }
           bool             DOMAttrImpl::isDefaultNamespace(const XMLCh* namespaceURI) const {return fNode.isDefaultNamespace(namespaceURI); }
           const XMLCh*     DOMAttrImpl::lookupNamespaceURI(const XMLCh* prefix) const  {return fNode.lookupNamespaceURI(prefix); }

XERCES_CPP_NAMESPACE_END
