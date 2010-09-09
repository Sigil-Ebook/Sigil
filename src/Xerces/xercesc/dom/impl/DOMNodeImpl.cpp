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
 * $Id: DOMNodeImpl.cpp 673428 2008-07-02 16:00:35Z amassari $
 */

// This class doesn't support having any children, and implements the behavior
// of an empty NodeList as far getChildNodes is concerned.
// The ParentNode subclass overrides this behavior.


#include "DOMCasts.hpp"

#include "DOMDocumentTypeImpl.hpp"
#include "DOMElementImpl.hpp"
#include "DOMAttrImpl.hpp"

#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMException.hpp>

#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLInitializer.hpp>
#include <stdio.h>
#include <assert.h>

XERCES_CPP_NAMESPACE_BEGIN

//Though DOMNodeImpl does not derivate from DOMNode, it shares
//the same GetDOMNodeMemoryManager

const unsigned short DOMNodeImpl::READONLY     = 0x1<<0;
const unsigned short DOMNodeImpl::SYNCDATA     = 0x1<<1;
const unsigned short DOMNodeImpl::SYNCCHILDREN = 0x1<<2;
const unsigned short DOMNodeImpl::OWNED        = 0x1<<3;
const unsigned short DOMNodeImpl::FIRSTCHILD   = 0x1<<4;
const unsigned short DOMNodeImpl::SPECIFIED    = 0x1<<5;
const unsigned short DOMNodeImpl::IGNORABLEWS  = 0x1<<6;
const unsigned short DOMNodeImpl::SETVALUE     = 0x1<<7;
const unsigned short DOMNodeImpl::ID_ATTR      = 0x1<<8;
const unsigned short DOMNodeImpl::USERDATA     = 0x1<<9;
const unsigned short DOMNodeImpl::LEAFNODETYPE = 0x1<<10;
const unsigned short DOMNodeImpl::CHILDNODE    = 0x1<<11;
const unsigned short DOMNodeImpl::TOBERELEASED = 0x1<<12;

//
//
static DOMNodeListImpl *gEmptyNodeList = 0; // Singleton empty node list.

void XMLInitializer::initializeDOMNodeListImpl()
{
    gEmptyNodeList = new DOMNodeListImpl(0);
}

void XMLInitializer::terminateDOMNodeListImpl()
{
    delete gEmptyNodeList;
    gEmptyNodeList = 0;
}

// -----------------------------------------------------------------------
//  DOMNodeImpl Functions
// -----------------------------------------------------------------------
DOMNodeImpl::DOMNodeImpl(DOMNode *ownerNode)
:  fOwnerNode(ownerNode)
{
    this->flags = 0;
    // as long as we do not have any owner, fOwnerNode is our ownerDocument
}

// This only makes a shallow copy, cloneChildren must also be called for a
// deep clone
DOMNodeImpl::DOMNodeImpl(const DOMNodeImpl &other)
{
    this->flags = other.flags;
    this->isReadOnly(false);

    // Need to break the association w/ original parent
    this->fOwnerNode = other.getOwnerDocument();
    this->isOwned(false);
}



DOMNodeImpl::~DOMNodeImpl() {
}


DOMNode * DOMNodeImpl::appendChild(DOMNode *)
{
    // Only node types that don't allow children will use this default function.
    //   Others will go to DOMParentNode::appendChild.
    throw DOMException(DOMException::HIERARCHY_REQUEST_ERR,0, GetDOMNodeMemoryManager);
    return 0;
    //  return insertBefore(newChild, 0);
}


DOMNamedNodeMap * DOMNodeImpl::getAttributes() const {
    return 0;                   // overridden in ElementImpl
}


DOMNodeList *DOMNodeImpl::getChildNodes() const {
    return gEmptyNodeList;
}



DOMNode * DOMNodeImpl::getFirstChild() const {
    return 0;                   // overridden in ParentNode
}


DOMNode * DOMNodeImpl::getLastChild() const
{
    return 0;                   // overridden in ParentNode
}


DOMNode * DOMNodeImpl::getNextSibling() const {
    return 0;                // overridden in ChildNode
}


const XMLCh * DOMNodeImpl::getNodeValue() const {
    return 0;                    // Overridden by anything that has a value
}


//
//  Unlike the external getOwnerDocument, this one returns the owner document
//     for document nodes as well as all of the other node types.
//
DOMDocument *DOMNodeImpl::getOwnerDocument() const
{
    if (!this->isLeafNode())
    {
        DOMElementImpl *ep = (DOMElementImpl *)castToNode(this);
        return ep->fParent.fOwnerDocument;
    }

    //  Leaf node types - those that cannot have children, like Text.
    if (isOwned()) {

        DOMDocument* ownerDoc = fOwnerNode->getOwnerDocument();

        if (!ownerDoc) {

            assert (fOwnerNode->getNodeType() == DOMNode::DOCUMENT_NODE);
            return  (DOMDocument *)fOwnerNode;
        }
        else {
            return ownerDoc;
        }
    } else {
        assert (fOwnerNode->getNodeType() == DOMNode::DOCUMENT_NODE);
        return  (DOMDocument *)fOwnerNode;
    }
}


void DOMNodeImpl::setOwnerDocument(DOMDocument *doc) {
    // if we have an owner we rely on it to have it right
    // otherwise fOwnerNode is our ownerDocument
    if (!isOwned()) {
        // revisit.  Problem with storage for doctype nodes that were created
        //                on the system heap in advance of having a document.
        fOwnerNode = doc;
    }
}

DOMNode * DOMNodeImpl::getParentNode() const
{
    return 0;                // overridden in ChildNode
}


DOMNode*  DOMNodeImpl::getPreviousSibling() const
{
    return 0;                // overridden in ChildNode
}

bool DOMNodeImpl::hasChildNodes() const
{
    return false;
}



DOMNode *DOMNodeImpl::insertBefore(DOMNode *, DOMNode *) {
    throw DOMException(DOMException::HIERARCHY_REQUEST_ERR, 0, GetDOMNodeMemoryManager);
    return 0;
}


DOMNode *DOMNodeImpl::removeChild(DOMNode *)
{
    throw DOMException(DOMException::NOT_FOUND_ERR, 0, GetDOMNodeMemoryManager);
    return 0;
}


DOMNode *DOMNodeImpl::replaceChild(DOMNode *, DOMNode *)
{
    throw DOMException(DOMException::HIERARCHY_REQUEST_ERR,0, GetDOMNodeMemoryManager);
    return 0;
}



void DOMNodeImpl::setNodeValue(const XMLCh *)
{
    // Default behavior is to do nothing, overridden in some subclasses
}



void DOMNodeImpl::setReadOnly(bool readOnl, bool deep)
{
    this->isReadOnly(readOnl);

    if (deep) {
        for (DOMNode *mykid = castToNode(this)->getFirstChild();
            mykid != 0;
            mykid = mykid->getNextSibling()) {

            short kidNodeType = mykid->getNodeType();

            switch (kidNodeType) {
            case DOMNode::ENTITY_REFERENCE_NODE:
                break;
            case DOMNode::ELEMENT_NODE:
                ((DOMElementImpl*) mykid)->setReadOnly(readOnl, true);
                break;
            case DOMNode::DOCUMENT_TYPE_NODE:
               ((DOMDocumentTypeImpl*) mykid)->setReadOnly(readOnl, true);
               break;
            default:
                castToNodeImpl(mykid)->setReadOnly(readOnl, true);
                break;
            }
        }
    }
}


//Introduced in DOM Level 2

void DOMNodeImpl::normalize()
{
    // does nothing by default, overridden by subclasses
}


bool DOMNodeImpl::isSupported(const XMLCh *feature, const XMLCh *version) const
{
    return DOMImplementation::getImplementation()->hasFeature(feature, version);
}

const XMLCh *DOMNodeImpl::getNamespaceURI() const
{
    return 0;
}

const XMLCh *DOMNodeImpl::getPrefix() const
{
    return 0;
}

const XMLCh *DOMNodeImpl::getLocalName() const
{
    return 0;
}


void DOMNodeImpl::setPrefix(const XMLCh *)
{
    throw DOMException(DOMException::NAMESPACE_ERR, 0, GetDOMNodeMemoryManager);
}


bool DOMNodeImpl::hasAttributes() const {
    return 0;                   // overridden in ElementImpl
}





const XMLCh *DOMNodeImpl::getXmlString()      {return XMLUni::fgXMLString;}
const XMLCh *DOMNodeImpl::getXmlURIString()   {return XMLUni::fgXMLURIName;}
const XMLCh *DOMNodeImpl::getXmlnsString()    {return XMLUni::fgXMLNSString;}
const XMLCh *DOMNodeImpl::getXmlnsURIString() {return XMLUni::fgXMLNSURIName;}

//Return a URI mapped from the given prefix and namespaceURI as below
//    prefix   namespaceURI    output
//---------------------------------------------------
//    "xml"     xmlURI          xmlURI
//    "xml"     otherwise       NAMESPACE_ERR
//    "xmlns"   xmlnsURI        xmlnsURI (nType = ATTRIBUTE_NODE only)
//    "xmlns"   otherwise       NAMESPACE_ERR (nType = ATTRIBUTE_NODE only)
//    != null   null or ""      NAMESPACE_ERR
//    else      any             namesapceURI
const XMLCh* DOMNodeImpl::mapPrefix(const XMLCh *prefix,
                                     const XMLCh *namespaceURI, short nType)
{
    if (prefix == 0)
        return namespaceURI;

    if (XMLString::equals(prefix, XMLUni::fgXMLString))  {
        if (XMLString::equals(namespaceURI, XMLUni::fgXMLURIName))
            return XMLUni::fgXMLURIName;
        throw DOMException(DOMException::NAMESPACE_ERR, 0);
    } else if (nType == DOMNode::ATTRIBUTE_NODE && XMLString::equals(prefix, XMLUni::fgXMLNSString)) {
        if (XMLString::equals(namespaceURI, XMLUni::fgXMLNSURIName))
            return XMLUni::fgXMLNSURIName;
        throw DOMException(DOMException::NAMESPACE_ERR, 0);
    } else if (namespaceURI == 0 || *namespaceURI == 0)
        throw DOMException(DOMException::NAMESPACE_ERR, 0);
    return namespaceURI;
}

//Introduced in DOM Level 3
void* DOMNodeImpl::setUserData(const XMLCh* key, void* data, DOMUserDataHandler* handler)
{
   if (!data && !hasUserData())
       return 0;

    hasUserData(true);
    return ((DOMDocumentImpl*)getOwnerDocument())->setUserData(this, key, data, handler);
}

void* DOMNodeImpl::getUserData(const XMLCh* key) const
{
   if (hasUserData())
       return ((DOMDocumentImpl*)getOwnerDocument())->getUserData(this, key);
    return 0;
}

void DOMNodeImpl::callUserDataHandlers(DOMUserDataHandler::DOMOperationType operation,
                                       const DOMNode* src,
                                       DOMNode* dst) const
{
    DOMDocumentImpl* doc=(DOMDocumentImpl*)getOwnerDocument();
    if (doc)
        doc->callUserDataHandlers(this, operation, src, dst);
}

bool DOMNodeImpl::isSameNode(const DOMNode* other) const
{
    return (castToNode(this) == other);
}

bool DOMNodeImpl::isEqualNode(const DOMNode* arg) const
{
    if (!arg)
        return false;

    if (isSameNode(arg)) {
        return true;
    }

    DOMNode* thisNode = castToNode(this);

    if (arg->getNodeType() != thisNode->getNodeType()) {
        return false;
    }

    // the compareString will check null string as well
    if (!XMLString::equals(thisNode->getNodeName(), arg->getNodeName())) {
        return false;
    }

    if (!XMLString::equals(thisNode->getLocalName(),arg->getLocalName())) {
        return false;
    }

    if (!XMLString::equals(thisNode->getNamespaceURI(), arg->getNamespaceURI())) {
        return false;
    }

    if (!XMLString::equals(thisNode->getPrefix(), arg->getPrefix())) {
        return false;
    }

    if (!XMLString::equals(thisNode->getNodeValue(), arg->getNodeValue())) {
        return false;
    }

    return true;
}

const XMLCh* DOMNodeImpl::lookupPrefix(const XMLCh* namespaceURI) const {
    // REVISIT: When Namespaces 1.1 comes out this may not be true
    // Prefix can't be bound to null namespace
    if (namespaceURI == 0) {
        return 0;
    }

    DOMNode *thisNode = castToNode(this);

    short type = thisNode->getNodeType();

    switch (type) {
    case DOMNode::ELEMENT_NODE: {
        return lookupPrefix(namespaceURI, (DOMElement*)thisNode);
    }
    case DOMNode::DOCUMENT_NODE:{
        return ((DOMDocument*)thisNode)->getDocumentElement()->lookupPrefix(namespaceURI);
    }

    case DOMNode::ENTITY_NODE :
    case DOMNode::NOTATION_NODE:
    case DOMNode::DOCUMENT_FRAGMENT_NODE:
    case DOMNode::DOCUMENT_TYPE_NODE:
        // type is unknown
        return 0;
    case DOMNode::ATTRIBUTE_NODE:{
        if (fOwnerNode->getNodeType() == DOMNode::ELEMENT_NODE) {
            return fOwnerNode->lookupPrefix(namespaceURI);
        }
        return 0;
    }
    default:{
        DOMNode *ancestor = getElementAncestor(thisNode);
        if (ancestor != 0) {
            return ancestor->lookupPrefix(namespaceURI);
        }
        return 0;
    }
    }
}


DOMNode* DOMNodeImpl::getElementAncestor (const DOMNode* currentNode) const {
    DOMNode* parent = currentNode->getParentNode();
    while(parent != 0) {
        short type = parent->getNodeType();
        if (type == DOMNode::ELEMENT_NODE) {
            return parent;
        }
        parent=parent->getParentNode();
    }
    return 0;
}


const XMLCh* DOMNodeImpl::lookupPrefix(const XMLCh* const namespaceURI, DOMElement *originalElement) const {
    DOMNode *thisNode = castToNode(this);

    const XMLCh* ns = thisNode->getNamespaceURI();
    // REVISIT: if no prefix is available is it null or empty string, or
    //          could be both?
    const XMLCh* prefix = thisNode->getPrefix();

    if (ns != 0 && XMLString::equals(ns,namespaceURI) && prefix != 0) {
        const XMLCh* foundNamespace =  originalElement->lookupNamespaceURI(prefix);
        if (foundNamespace != 0 && XMLString::equals(foundNamespace, namespaceURI)) {
            return prefix;
        }
    }
    if (thisNode->hasAttributes()) {
        DOMNamedNodeMap *nodeMap = thisNode->getAttributes();

        if(nodeMap != 0) {
            XMLSize_t length = nodeMap->getLength();

            for (XMLSize_t i = 0;i < length;i++) {
                DOMNode *attr = nodeMap->item(i);
                const XMLCh* attrPrefix = attr->getPrefix();
                const XMLCh* value = attr->getNodeValue();

                ns = attr->getNamespaceURI();

                if (ns != 0 && XMLString::equals(ns, XMLUni::fgXMLNSURIName)) {
                    // DOM Level 2 nodes
                    if ((attrPrefix != 0 && XMLString::equals(attrPrefix, XMLUni::fgXMLNSString)) &&
                        XMLString::equals(value, namespaceURI)) {
                        const XMLCh* localname= attr->getLocalName();
                        const XMLCh* foundNamespace = originalElement->lookupNamespaceURI(localname);
                        if (foundNamespace != 0 && XMLString::equals(foundNamespace, namespaceURI)) {
                            return localname;
                        }
                    }
                }
            }
        }
    }
    DOMNode *ancestor = getElementAncestor(thisNode);
    if (ancestor != 0) {
        return castToNodeImpl(ancestor)->lookupPrefix(namespaceURI, originalElement);
    }
    return 0;
}

const XMLCh* DOMNodeImpl::lookupNamespaceURI(const XMLCh* specifiedPrefix) const  {
    DOMNode *thisNode = castToNode(this);

    short type = thisNode->getNodeType();
    switch (type) {
    case DOMNode::ELEMENT_NODE : {
        const XMLCh* ns = thisNode->getNamespaceURI();
        const XMLCh* prefix = thisNode->getPrefix();
        if (ns != 0) {
            // REVISIT: is it possible that prefix is empty string?
            if (specifiedPrefix == 0 && prefix == specifiedPrefix) {
                // looking for default namespace
                return ns;
            } else if (prefix != 0 && XMLString::equals(prefix, specifiedPrefix)) {
                // non default namespace
                return ns;
            }
        }
        if (thisNode->hasAttributes()) {
            DOMNamedNodeMap *nodeMap = thisNode->getAttributes();
            if(nodeMap != 0) {
                XMLSize_t length = nodeMap->getLength();
                for (XMLSize_t i = 0;i < length;i++) {
                    DOMNode *attr = nodeMap->item(i);
                    const XMLCh *attrPrefix = attr->getPrefix();
                    const XMLCh *value = attr->getNodeValue();
                    ns = attr->getNamespaceURI();

                    if (ns != 0 && XMLString::equals(ns, XMLUni::fgXMLNSURIName)) {
                        // at this point we are dealing with DOM Level 2 nodes only
                        if (specifiedPrefix == 0 &&
                            XMLString::equals(attr->getNodeName(), XMLUni::fgXMLNSString)) {
                            // default namespace
                            return value;
                        } else if (attrPrefix != 0 &&
                                   XMLString::equals(attrPrefix, XMLUni::fgXMLNSString) &&
                                   XMLString::equals(attr->getLocalName(), specifiedPrefix)) {
                            // non default namespace
                            return value;
                        }
                    }
                }
            }
        }
        DOMNode *ancestor = getElementAncestor(thisNode);
        if (ancestor != 0) {
            return ancestor->lookupNamespaceURI(specifiedPrefix);
        }
        return 0;
    }
    case DOMNode::DOCUMENT_NODE : {
        return((DOMDocument*)thisNode)->getDocumentElement()->lookupNamespaceURI(specifiedPrefix);
    }
    case DOMNode::ENTITY_NODE :
    case DOMNode::NOTATION_NODE:
    case DOMNode::DOCUMENT_FRAGMENT_NODE:
    case DOMNode::DOCUMENT_TYPE_NODE:
        // type is unknown
        return 0;
    case DOMNode::ATTRIBUTE_NODE:{
        if (fOwnerNode->getNodeType() == DOMNode::ELEMENT_NODE) {
            return fOwnerNode->lookupNamespaceURI(specifiedPrefix);
        }
        return 0;
    }
    default:{
        DOMNode *ancestor = getElementAncestor(castToNode(this));
        if (ancestor != 0) {
            return ancestor->lookupNamespaceURI(specifiedPrefix);
        }
        return 0;
    }
    }
}


const XMLCh*     DOMNodeImpl::getBaseURI() const{
    DOMNode *thisNode = castToNode(this);
    DOMNode* parent = thisNode->getParentNode();
    if (parent)
        return parent->getBaseURI();
    else
        return 0;
}

const DOMNode*   DOMNodeImpl::getTreeParentNode(const DOMNode* node) const {
    const DOMNode* parent=node->getParentNode();
    if(parent)
        return parent;
    short nodeType=node->getNodeType();
    switch(nodeType)
    {
    case DOMNode::ATTRIBUTE_NODE: return ((const DOMAttr*)node)->getOwnerElement();
    case DOMNode::NOTATION_NODE:
    case DOMNode::ENTITY_NODE:    return node->getOwnerDocument()->getDoctype();
    }
    return 0;
}

short            DOMNodeImpl::compareDocumentPosition(const DOMNode* other) const {
    DOMNode* thisNode = castToNode(this);

    // If the two nodes being compared are the same node, then no flags are set on the return.
    if (thisNode == other)
        return 0;

    //if this is a custom node, we don't really know what to do, just return
    //user should provide its own compareDocumentPosition logic, and shouldn't reach here
    if(thisNode->getNodeType() > 12) {
        return 0;
    }

    //if it is a custom node we must ask it for the order
    if(other->getNodeType() > 12) {
        return reverseTreeOrderBitPattern(other->compareDocumentPosition(thisNode));
    }

    // Otherwise, the order of two nodes is determined by looking for common containers --
    // containers which contain both. A node directly contains any child nodes.
    // A node also directly contains any other nodes attached to it such as attributes
    // contained in an element or entities and notations contained in a document type.
    // Nodes contained in contained nodes are also contained, but less-directly as
    // the number of intervening containers increases.

    // If one of the nodes being compared contains the other node, then the container precedes
    // the contained node, and reversely the contained node follows the container. For example,
    // when comparing an element against its own attribute or child, the element node precedes
    // its attribute node and its child node, which both follow it.

    const DOMNode* tmpNode;
    const DOMNode* myRoot = castToNode(this);
    int myDepth=0;
    while((tmpNode=getTreeParentNode(myRoot))!=0)
    {
        myRoot=tmpNode;
        if(myRoot==other)
            return DOMNode::DOCUMENT_POSITION_CONTAINS | DOMNode::DOCUMENT_POSITION_PRECEDING;
        myDepth++;
    }

    const DOMNode* hisRoot = other;
    int hisDepth=0;
    while((tmpNode=getTreeParentNode(hisRoot))!=0)
    {
        hisRoot=tmpNode;
        if(hisRoot==thisNode)
            return DOMNode::DOCUMENT_POSITION_CONTAINED_BY | DOMNode::DOCUMENT_POSITION_FOLLOWING;
        hisDepth++;
    }

    // If there is no common container node, then the order is based upon order between the
    // root container of each node that is in no container. In this case, the result is
    // disconnected and implementation-specific. This result is stable as long as these
    // outer-most containing nodes remain in memory and are not inserted into some other
    // containing node. This would be the case when the nodes belong to different documents
    // or fragments, and cloning the document or inserting a fragment might change the order.

    if(myRoot!=hisRoot)
        return DOMNode::DOCUMENT_POSITION_DISCONNECTED | DOMNode::DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC |
              (myRoot<hisRoot?DOMNode::DOCUMENT_POSITION_PRECEDING:DOMNode::DOCUMENT_POSITION_FOLLOWING);

    // If neither of the previous cases apply, then there exists a most-direct container common
    // to both nodes being compared. In this case, the order is determined based upon the two
    // determining nodes directly contained in this most-direct common container that either
    // are or contain the corresponding nodes being compared.

    // if the two depths are different, go to the same one
    myRoot = castToNode(this);
    hisRoot = other;
    if (myDepth > hisDepth) {
        for (int i= 0 ; i < myDepth - hisDepth; i++)
            myRoot = getTreeParentNode(myRoot);
    }
    else {
        for (int i = 0; i < hisDepth - myDepth; i++)
            hisRoot = getTreeParentNode(hisRoot);
    }

    // We now have nodes at the same depth in the tree.  Find a common ancestor.
    const DOMNode *myNodeP=myRoot;
	const DOMNode *hisNodeP=hisRoot;
    while(myRoot!=hisRoot)
    {
        myNodeP = myRoot;
        hisNodeP = hisRoot;
        myRoot = getTreeParentNode(myRoot);
        hisRoot = getTreeParentNode(hisRoot);
    }

    short myNodeType=myNodeP->getNodeType();
    short hisNodeType=hisNodeP->getNodeType();
    bool bMyNodeIsChild=(myNodeType!=DOMNode::ATTRIBUTE_NODE && myNodeType!=DOMNode::ENTITY_NODE && myNodeType!=DOMNode::NOTATION_NODE);
    bool bHisNodeIsChild=(hisNodeType!=DOMNode::ATTRIBUTE_NODE && hisNodeType!=DOMNode::ENTITY_NODE && hisNodeType!=DOMNode::NOTATION_NODE);

    // If these two determining nodes are both child nodes, then the natural DOM order of these
    // determining nodes within the containing node is returned as the order of the corresponding nodes.
    // This would be the case, for example, when comparing two child elements of the same element.
    if(bMyNodeIsChild && bHisNodeIsChild)
    {
        while(myNodeP != 0)
        {
            myNodeP = myNodeP->getNextSibling();
            if(myNodeP == hisNodeP)
                return DOMNode::DOCUMENT_POSITION_FOLLOWING;
        }
        return DOMNode::DOCUMENT_POSITION_PRECEDING;
    }

    // If one of the two determining nodes is a child node and the other is not, then the corresponding
    // node of the child node follows the corresponding node of the non-child node. This would be the case,
    // for example, when comparing an attribute of an element with a child element of the same element.
    else if(!bMyNodeIsChild && bHisNodeIsChild)
        return DOMNode::DOCUMENT_POSITION_FOLLOWING;
    else if(bMyNodeIsChild && !bHisNodeIsChild)
        return DOMNode::DOCUMENT_POSITION_PRECEDING;

    else
    {
        // If neither of the two determining node is a child node and one determining node has a greater value
        // of nodeType than the other, then the corresponding node precedes the other. This would be the case,
        // for example, when comparing an entity of a document type against a notation of the same document type.
        if(myNodeType!=hisNodeType)
            return (myNodeType<hisNodeType)?DOMNode::DOCUMENT_POSITION_FOLLOWING:DOMNode::DOCUMENT_POSITION_PRECEDING;

        // If neither of the two determining node is a child node and nodeType is the same for both determining
        // nodes, then an implementation-dependent order between the determining nodes is returned. This order
        // is stable as long as no nodes of the same nodeType are inserted into or removed from the direct container.
        // This would be the case, for example, when comparing two attributes of the same element, and inserting
        // or removing additional attributes might change the order between existing attributes.
        return DOMNode::DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC | ((myNodeP<hisNodeP)?DOMNode::DOCUMENT_POSITION_FOLLOWING:DOMNode::DOCUMENT_POSITION_PRECEDING);
    }
    // REVISIT:  shouldn't get here.   Should probably throw an
    // exception
    return 0;
}

short DOMNodeImpl::reverseTreeOrderBitPattern(short pattern) const {

    if(pattern & DOMNode::DOCUMENT_POSITION_PRECEDING) {
        pattern &= !DOMNode::DOCUMENT_POSITION_PRECEDING;
        pattern |= DOMNode::DOCUMENT_POSITION_FOLLOWING;
    }
    else if(pattern & DOMNode::DOCUMENT_POSITION_FOLLOWING) {
        pattern &= !DOMNode::DOCUMENT_POSITION_FOLLOWING;
        pattern |= DOMNode::DOCUMENT_POSITION_PRECEDING;
    }

    if(pattern & DOMNode::DOCUMENT_POSITION_CONTAINED_BY) {
        pattern &= !DOMNode::DOCUMENT_POSITION_CONTAINED_BY;
        pattern |= DOMNode::DOCUMENT_POSITION_CONTAINS;
    }
    else if(pattern & DOMNode::DOCUMENT_POSITION_CONTAINS) {
        pattern &= !DOMNode::DOCUMENT_POSITION_CONTAINS;
        pattern |= DOMNode::DOCUMENT_POSITION_CONTAINED_BY;
    }

    return pattern;
}

/***
 *
 *   Excerpt from http://www.w3.org/TR/2003/WD-DOM-Level-3-Core-20030226/core.html#Node3-textContent
 *
 *   textContent of type DOMString, introduced in DOM Level 3
 *
 *   This attribute returns the text content of this node and its descendants. When it is defined
 *   to be null, setting it has no effect.
 *
 *   When set, any possible children this node may have are removed and replaced by a single Text node
 *   containing the string this attribute is set to.
 *
 *   On getting, no serialization is performed, the returned string does not contain any markup.
 *   No whitespace normalization is performed, the returned string does not contain the element content
 *   whitespaces Fundamental Interfaces.
 *
 *   Similarly, on setting, no parsing is performed either, the input string is taken as pure textual content.
 *
 *   The string returned is made of the text content of this node depending on its type,
 *   as defined below:
 *
 *       Node type                                           Content
 *   ====================       ========================================================================
 *     ELEMENT_NODE               concatenation of the textContent attribute value of every child node,
 *     ENTITY_NODE			      excluding COMMENT_NODE and PROCESSING_INSTRUCTION_NODE nodes.
 *     ENTITY_REFERENCE_NODE	  This is the empty string if the node has no children.
 *     DOCUMENT_FRAGMENT_NODE
 *    --------------------------------------------------------------------------------------------------
 *     ATTRIBUTE_NODE
 *     TEXT_NODE
 *     CDATA_SECTION_NODE
 *     COMMENT_NODE,
 *     PROCESSING_INSTRUCTION_NODE   nodeValue
 *    --------------------------------------------------------------------------------------------------
 *     DOCUMENT_NODE,
 *     DOCUMENT_TYPE_NODE,
 *     NOTATION_NODE                 null
 *
 ***/

const XMLCh*     DOMNodeImpl::getTextContent() const
{
	XMLSize_t nBufferLength = 0;

	getTextContent(NULL, nBufferLength);
	XMLCh* pzBuffer = (XMLCh*)((DOMDocumentImpl*)getOwnerDocument())->allocate((nBufferLength+1) * sizeof(XMLCh));
	getTextContent(pzBuffer, nBufferLength);
	pzBuffer[nBufferLength] = 0;

	return pzBuffer;

}

const XMLCh*    DOMNodeImpl::getTextContent(XMLCh* pzBuffer, XMLSize_t& rnBufferLength) const
{
	XMLSize_t nRemainingBuffer = rnBufferLength;
	rnBufferLength = 0;

	if (pzBuffer)
		*pzBuffer = 0;

	DOMNode *thisNode = castToNode(this);

	switch (thisNode->getNodeType())
	{
	case DOMNode::ELEMENT_NODE:
    case DOMNode::ENTITY_NODE:
    case DOMNode::ENTITY_REFERENCE_NODE:
    case DOMNode::DOCUMENT_FRAGMENT_NODE:
    {
		DOMNode* current = thisNode->getFirstChild();

		while (current != NULL)
		{
			if (current->getNodeType() != DOMNode::COMMENT_NODE &&
				current->getNodeType() != DOMNode::PROCESSING_INSTRUCTION_NODE)
			{

				if (pzBuffer)
				{
					XMLSize_t nContentLength = nRemainingBuffer;
					castToNodeImpl(current)->getTextContent(pzBuffer + rnBufferLength, nContentLength);
					rnBufferLength += nContentLength;
					nRemainingBuffer -= nContentLength;
				}
				else
				{
					XMLSize_t nContentLength = 0;
					castToNodeImpl(current)->getTextContent(NULL, nContentLength);
					rnBufferLength += nContentLength;
				}
			}

			current = current->getNextSibling();

		}
    }

    break;

    case DOMNode::ATTRIBUTE_NODE:
    case DOMNode::TEXT_NODE:
    case DOMNode::CDATA_SECTION_NODE:
    case DOMNode::COMMENT_NODE:
    case DOMNode::PROCESSING_INSTRUCTION_NODE:
    {
		const XMLCh* pzValue = thisNode->getNodeValue();
		XMLSize_t nStrLen = XMLString::stringLen(pzValue);

		if (pzBuffer)
		{
			XMLSize_t nContentLength = (nRemainingBuffer >= nStrLen) ? nStrLen : nRemainingBuffer;
			XMLString::copyNString(pzBuffer + rnBufferLength, pzValue, nContentLength);
			rnBufferLength += nContentLength;
			nRemainingBuffer -= nContentLength;
		}
		else
		{
			rnBufferLength += nStrLen;
		}

    }

    break;

	/***
         DOCUMENT_NODE
		 DOCUMENT_TYPE_NODE
		 NOTATION_NODE
	***/
	default:

		break;
	}

	return pzBuffer;

}

void DOMNodeImpl::setTextContent(const XMLCh* textContent){
    DOMNode *thisNode = castToNode(this);
    switch (thisNode->getNodeType())
    {
        case DOMNode::ELEMENT_NODE:
        case DOMNode::ENTITY_NODE:
        case DOMNode::ENTITY_REFERENCE_NODE:
        case DOMNode::DOCUMENT_FRAGMENT_NODE:
            {
                if (isReadOnly())
                  throw DOMException(DOMException::NO_MODIFICATION_ALLOWED_ERR, 0, GetDOMNodeMemoryManager);

                // Remove all childs
                DOMNode* current = thisNode->getFirstChild();
                while (current != NULL)
                {
                    thisNode->removeChild(current);
                    current = thisNode->getFirstChild();
                }
                if (textContent != NULL)
                {
                    // Add textnode containing data
                    current = ((DOMDocumentImpl*)thisNode->getOwnerDocument())->createTextNode(textContent);
                    thisNode->appendChild(current);
                }
            }
            break;

        case DOMNode::ATTRIBUTE_NODE:
        case DOMNode::TEXT_NODE:
        case DOMNode::CDATA_SECTION_NODE:
        case DOMNode::COMMENT_NODE:
        case DOMNode::PROCESSING_INSTRUCTION_NODE:
            if (isReadOnly())
                throw DOMException(DOMException::NO_MODIFICATION_ALLOWED_ERR, 0, GetDOMNodeMemoryManager);

            thisNode->setNodeValue(textContent);
            break;

        case DOMNode::DOCUMENT_NODE:
        case DOMNode::DOCUMENT_TYPE_NODE:
        case DOMNode::NOTATION_NODE:
            break;

        default:
            throw DOMException(DOMException::NOT_SUPPORTED_ERR, 0, GetDOMNodeMemoryManager);
    }
}


bool DOMNodeImpl::isDefaultNamespace(const XMLCh* namespaceURI) const{
	DOMNode *thisNode = castToNode(this);
    short type = thisNode->getNodeType();
    switch (type) {
    case DOMNode::ELEMENT_NODE: {
        const XMLCh *prefix = thisNode->getPrefix();

        // REVISIT: is it possible that prefix is empty string?
        if (prefix == 0 || !*prefix) {
            return XMLString::equals(namespaceURI, thisNode->getNamespaceURI());
        }

        if (thisNode->hasAttributes()) {
            DOMElement *elem = (DOMElement *)thisNode;
            DOMNode *attr = elem->getAttributeNodeNS(XMLUni::fgXMLNSURIName, XMLUni::fgXMLNSString);
            if (attr != 0) {
                const XMLCh *value = attr->getNodeValue();
                return XMLString::equals(namespaceURI, value);
            }
        }
        DOMNode *ancestor = getElementAncestor(thisNode);
        if (ancestor != 0) {
            return ancestor->isDefaultNamespace(namespaceURI);
        }

        return false;
    }
    case DOMNode::DOCUMENT_NODE:{
        return ((DOMDocument*)thisNode)->getDocumentElement()->isDefaultNamespace(namespaceURI);
    }

    case DOMNode::ENTITY_NODE :
    case DOMNode::NOTATION_NODE:
    case DOMNode::DOCUMENT_FRAGMENT_NODE:
    case DOMNode::DOCUMENT_TYPE_NODE:
        // type is unknown
        return false;
    case DOMNode::ATTRIBUTE_NODE:{
        if (fOwnerNode->getNodeType() == DOMNode::ELEMENT_NODE) {
            return fOwnerNode->isDefaultNamespace(namespaceURI);

        }
        return false;
    }
    default:{
        DOMNode *ancestor = getElementAncestor(thisNode);
        if (ancestor != 0) {
            return ancestor->isDefaultNamespace(namespaceURI);
        }
        return false;
    }

    }
}

void* DOMNodeImpl::getFeature(const XMLCh*, const XMLCh*) const {
    return 0;
}


// non-standard extension
void DOMNodeImpl::release()
{
    // shouldn't reach here
    throw DOMException(DOMException::INVALID_ACCESS_ERR,0, GetDOMNodeMemoryManager);
}

XERCES_CPP_NAMESPACE_END

