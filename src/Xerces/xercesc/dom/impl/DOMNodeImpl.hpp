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
 * $Id: DOMNodeImpl.hpp 671894 2008-06-26 13:29:21Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMNODEIMPL_HPP)
#define XERCESC_INCLUDE_GUARD_DOMNODEIMPL_HPP

//
//  This file is part of the internal implementation of the C++ XML DOM.
//  It should NOT be included or used directly by application programs.
//
//  Applications should include the file <xercesc/dom/DOM.hpp> for the entire
//  DOM API, or xercesc/dom/DOM*.hpp for individual DOM classes, where the class
//  name is substituded for the *.
//

/**
 * A DOMNodeImpl doesn't have any children, and can therefore only be directly
 * inherited by classes of nodes that never have any, such as Text nodes. For
 * other types, such as Element, classes must inherit from ParentNode.
 * <P>
 * All nodes in a single document must originate
 * in that document. (Note that this is much tighter than "must be
 * same implementation") Nodes are all aware of their ownerDocument,
 * and attempts to mismatch will throw WRONG_DOCUMENT_ERR.
 * <P>
 * However, to save memory not all nodes always have a direct reference
 * to their ownerDocument. When a node is owned by another node it relies
 * on its owner to store its ownerDocument. Parent nodes always store it
 * though, so there is never more than one level of indirection.
 * And when a node doesn't have an owner, ownerNode refers to its
 * ownerDocument.
 **/

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/dom/DOMUserDataHandler.hpp>

XERCES_CPP_NAMESPACE_BEGIN


class DOMNamedNodeMap;
class DOMNodeList;
class DOMNode;
class DOMDocument;
class DOMElement;

class CDOM_EXPORT DOMNodeImpl {
public:

    // data
    DOMNode                *fOwnerNode; // typically the parent but not always!

    unsigned short flags;

    static const unsigned short READONLY;
    static const unsigned short SYNCDATA;
    static const unsigned short SYNCCHILDREN;
    static const unsigned short OWNED;
    static const unsigned short FIRSTCHILD;
    static const unsigned short SPECIFIED;
    static const unsigned short IGNORABLEWS;
    static const unsigned short SETVALUE;
    static const unsigned short ID_ATTR;
    static const unsigned short USERDATA;
    static const unsigned short LEAFNODETYPE;
    static const unsigned short CHILDNODE;
    static const unsigned short TOBERELEASED;


public:
    DOMNodeImpl(DOMNode *ownerDocument);
    DOMNodeImpl(const DOMNodeImpl &other);
    ~DOMNodeImpl();

    DOMNode         * appendChild(DOMNode *newChild);
    DOMNamedNodeMap * getAttributes() const;
    DOMNodeList     * getChildNodes() const;
    DOMNode         * getFirstChild() const;
    DOMNode         * getLastChild() const;
    const XMLCh     * getLocalName() const;
    const XMLCh     * getNamespaceURI() const;
    DOMNode         * getNextSibling() const;
    const XMLCh     * getNodeValue() const;
    DOMDocument     * getOwnerDocument() const;
    DOMNode         * getParentNode() const;
    const XMLCh     * getPrefix() const;
    DOMNode         * getPreviousSibling() const;
    bool              hasChildNodes() const;
    DOMNode         * insertBefore(DOMNode *newChild, DOMNode *refChild);
    void              normalize();
    DOMNode         * removeChild(DOMNode *oldChild);
    DOMNode         * replaceChild(DOMNode *newChild, DOMNode *oldChild);
    void              setNodeValue(const XMLCh *value);
    void              setPrefix(const XMLCh *fPrefix);
    void              setReadOnly(bool readOnly, bool deep);
    bool              isSupported(const XMLCh *feature, const XMLCh *version) const;
    bool              hasAttributes() const;

    // Introduced in DOM Level 3
    void*             setUserData(const XMLCh* key, void* data, DOMUserDataHandler* handler);
    void*             getUserData(const XMLCh* key) const;
    bool              isSameNode(const DOMNode* other) const;
    bool              isEqualNode(const DOMNode* arg) const;
    const XMLCh*      getBaseURI() const ;
    short             compareDocumentPosition(const DOMNode* other) const;
    const XMLCh*      getTextContent() const ;
    const XMLCh*      getTextContent(XMLCh* pzBuffer, XMLSize_t& rnBufferLength) const;
    void              setTextContent(const XMLCh* textContent) ;
    const XMLCh*      lookupPrefix(const XMLCh* namespaceURI) const ;
    bool              isDefaultNamespace(const XMLCh* namespaceURI) const ;
    const XMLCh*      lookupNamespaceURI(const XMLCh* prefix) const  ;
    void*             getFeature(const XMLCh* feature, const XMLCh* version) const;


    // Helper functions for DOM Level 3
    void              release();
    void              callUserDataHandlers(DOMUserDataHandler::DOMOperationType operation,
                                           const DOMNode* src,
                                           DOMNode* dst) const;
    //reverses the bit pattern given by compareDocumentPosition
    short             reverseTreeOrderBitPattern(short pattern) const;
    const DOMNode*    getTreeParentNode(const DOMNode* node) const;


    //Utility, not part of DOM Level 2 API
    static  bool      isKidOK(DOMNode *parent, DOMNode *child);
    static const XMLCh *mapPrefix(const XMLCh *prefix,
                               const XMLCh *namespaceURI, short nType);

    static const XMLCh *getXmlnsString();
    static const XMLCh *getXmlnsURIString();
    static const XMLCh *getXmlString();
    static const XMLCh *getXmlURIString();

public: // should really be protected - ALH

    DOMNode* getElementAncestor (const DOMNode* currentNode) const;
    const XMLCh* lookupPrefix(const XMLCh* const namespaceURI, DOMElement *el) const ;
    void setOwnerDocument(DOMDocument *doc);

    /*
     * Flags setters and getters
     */

    inline bool isReadOnly() const {
        return (flags & READONLY) != 0;
    }

    inline void isReadOnly(bool value) {
        flags = (value ? flags | READONLY : flags & ~READONLY);
    }

    inline bool needsSyncData() const {
        return (flags & SYNCDATA) != 0;
    }

    inline void needsSyncData(bool value) {
        flags = (value ? flags | SYNCDATA : flags & ~SYNCDATA);
    }

    inline bool needsSyncChildren() const {
        return (flags & SYNCCHILDREN) != 0;
    }

    inline void needsSyncChildren(bool value) {
        flags = (value ? flags | SYNCCHILDREN : flags & ~SYNCCHILDREN);
    }

    // For Attributes, true if the attr node is attached to an element.
    // For all other node types, true if the node has a parent node.
    inline bool isOwned() const {
        return (flags & OWNED) != 0;
    }

    inline void isOwned(bool value) {
        flags = (value ? flags | OWNED : flags & ~OWNED);
    }

    inline bool isFirstChild() const {
        return (flags & FIRSTCHILD) != 0;
    }

    inline void isFirstChild(bool value) {
        flags = (value ? flags | FIRSTCHILD : flags & ~FIRSTCHILD);
    }

    inline bool isSpecified() const {
        return (flags & SPECIFIED) != 0;
    }

    inline void isSpecified(bool value) {
        flags = (value ? flags | SPECIFIED : flags & ~SPECIFIED);
    }

    inline bool ignorableWhitespace() const {
        return (flags & IGNORABLEWS) != 0;
    }

    inline void ignorableWhitespace(bool value) {
        flags = (value ? flags | IGNORABLEWS : flags & ~IGNORABLEWS);
    }

    inline bool setValue() const {
        return (flags & SETVALUE) != 0;
    }

    inline void setValue(bool value) {
        flags = (value ? flags | SETVALUE : flags & ~SETVALUE);
    }

    inline bool isIdAttr() const {
        return (flags & ID_ATTR) != 0;
    }

    inline void isIdAttr(bool value) {
        flags = (value ? flags | ID_ATTR : flags & ~ID_ATTR);
    }

    inline bool hasUserData() const {
        return (flags & USERDATA) != 0;
    }

    inline void hasUserData(bool value) {
        flags = (value ? flags | USERDATA : flags & ~USERDATA);
    }

    //
    //  LeafNode is set true for node types that can not be ParentNodes (can't have children)
    //    This knowledge is used to allow casting from any unknown node type to the
    //    IDParentImpl or IDChildImpl parts of the node.
    //
    inline bool isLeafNode() const {
        return (flags & LEAFNODETYPE) != 0;
    }

    inline void setIsLeafNode(bool value) {
        flags = (value ? flags | LEAFNODETYPE : flags & ~LEAFNODETYPE);
    }


    //
    // ChildNode is set true for node types that can be children of other nodes, and
    //   therefore include a DOMChildNode data member.  Note that all of the leaf
    //   node types (above flag) are also ChildNodes, but not all ChildNodes are
    //   leaf nodes.
    inline bool isChildNode() const {
        return (flags & CHILDNODE) != 0;
    }

    inline void setIsChildNode(bool value) {
        flags = (value ? flags | CHILDNODE : flags & ~CHILDNODE);
    }

    // True if this node has to be released regardless if it has a owner or not
    // This is true if called from fParent->release()
    inline bool isToBeReleased() const {
        return (flags & TOBERELEASED) != 0;
    }

    inline void isToBeReleased(bool value) {
        flags = (value ? flags | TOBERELEASED : flags & ~TOBERELEASED);
    }

};


// This macro lists all of the pure virtual functions declared in DOMNode that must
//   be implemented by all node types.  Since there is no inheritance of implementation,
//   using this macro in the class declaration of the node types make it easier to
//   accurately get all of the functions declared.
//
#define DOMNODE_FUNCTIONS \
    virtual       DOMNode*         appendChild(DOMNode *newChild) ;\
    virtual       DOMNode*         cloneNode(bool deep) const ;\
    virtual       DOMNamedNodeMap* getAttributes() const ;\
    virtual       DOMNodeList*     getChildNodes() const ;\
    virtual       DOMNode*         getFirstChild() const ;\
    virtual       DOMNode*         getLastChild() const ;\
    virtual const XMLCh*           getLocalName() const ;\
    virtual const XMLCh*           getNamespaceURI() const ;\
    virtual       DOMNode*         getNextSibling() const ;\
    virtual const XMLCh*           getNodeName() const ;\
    virtual       NodeType         getNodeType() const ;\
    virtual const XMLCh*           getNodeValue() const ;\
    virtual       DOMDocument*     getOwnerDocument() const ;\
    virtual const XMLCh*           getPrefix() const ;\
    virtual       DOMNode*         getParentNode() const ;\
    virtual       DOMNode*         getPreviousSibling() const ;\
    virtual       bool             hasChildNodes() const ;\
    virtual       DOMNode*         insertBefore(DOMNode *newChild, DOMNode *refChild) ;\
    virtual       void             normalize() ;\
    virtual       DOMNode*         removeChild(DOMNode *oldChild) ;\
    virtual       DOMNode*         replaceChild(DOMNode *newChild, DOMNode *oldChild) ;\
    virtual       void             setNodeValue(const XMLCh  *nodeValue) ;\
    virtual       bool             isSupported(const XMLCh *feature, const XMLCh *version) const ;\
    virtual       bool             hasAttributes() const ;\
    virtual       void             setPrefix(const XMLCh * prefix) ;\
    virtual       void*            setUserData(const XMLCh* key, void* data, DOMUserDataHandler* handler) ;\
    virtual       void*            getUserData(const XMLCh* key) const ;\
    virtual       bool             isSameNode(const DOMNode* other) const;\
    virtual       bool             isEqualNode(const DOMNode* arg) const;\
    virtual const XMLCh*           getBaseURI() const ;\
    virtual short                  compareDocumentPosition(const DOMNode* other) const ;\
    virtual const XMLCh*           getTextContent() const ;\
            const XMLCh*           getTextContent(XMLCh* pzBuffer, unsigned int& rnBufferLength) const;\
    virtual void                   setTextContent(const XMLCh* textContent) ;\
    virtual const XMLCh*           lookupPrefix(const XMLCh* namespaceURI) const  ;\
    virtual bool                   isDefaultNamespace(const XMLCh* namespaceURI) const;\
    virtual const XMLCh*           lookupNamespaceURI(const XMLCh* prefix) const  ;\
    virtual       void*            getFeature(const XMLCh* feature, const XMLCh* version) const ;\
    virtual       void             release()


/*
 *  Here are dummy stubs for most of the functions introduced by DOMNode.
 *    Each subclass of DOMNode will have something like this that delegates each
 *    function to the appropriate implementation.
 *    Functions that must be supplied by every node class are omitted.
 *
           DOMNode*         xxx::appendChild(DOMNode *newChild)          {return fParent.appendChild (newChild); };
           DOMNamedNodeMap* xxx::getAttributes() const                   {return fNode.getAttributes (); };
           DOMNodeList*     xxx::getChildNodes() const                   {return fParent.getChildNodes (); };
           DOMNode*         xxx::getFirstChild() const                   {return fParent.getFirstChild (); };
           DOMNode*         xxx::getLastChild() const                    {return fParent.getLastChild (); };
     const XMLCh*           xxx::getLocalName() const                    {return fNode.getLocalName (); };
     const XMLCh*           xxx::getNamespaceURI() const                 {return fNode.getNamespaceURI (); };
           DOMNode*         xxx::getNextSibling() const                  {return fChild.getNextSibling (); };
     const XMLCh*           xxx::getNodeValue() const                    {return fNode.getNodeValue (); };
           DOMDocument*     xxx::getOwnerDocument() const                {return fNode.getOwnerDocument (); };
     const XMLCh*           xxx::getPrefix() const                       {return fNode.getPrefix (); };
           DOMNode*         xxx::getParentNode() const                   {return fChild.getParentNode (this); };
           DOMNode*         xxx::getPreviousSibling() const              {return fChild.getPreviousSibling (this); };
           bool             xxx::hasChildNodes() const                   {return fParent.hasChildNodes (); };
           DOMNode*         xxx::insertBefore(DOMNode *newChild, DOMNode *refChild)
                                                                         {return fParent.insertBefore (newChild, refChild); };
           void             xxx::normalize()                             {fParent.normalize(); };
           DOMNode*         xxx::removeChild(DOMNode *oldChild)          {return fParent.removeChild (oldChild); };
           DOMNode*         xxx::replaceChild(DOMNode *newChild, DOMNode *oldChild)
                                                                         {return fParent.replaceChild (newChild, oldChild); };
           bool             xxx::isSupported(const XMLCh *feature, const XMLCh *version) const
                                                                         {return fNode.isSupported (feature, version); };
           void             xxx::setPrefix(const XMLCh  *prefix)         {fNode.setPrefix(prefix); };
           bool             xxx::hasAttributes() const                   {return fNode.hasAttributes(); };
           bool             xxx::isSameNode(const DOMNode* other) const  {return fNode.isSameNode(other); };
           bool             xxx::isEqualNode(const DOMNode* arg) const   {return fNode.isEqualNode(arg); };
           void*            xxx::setUserData(const XMLCh* key, void* data, DOMUserDataHandler* handler)
                                                                         {return fNode.setUserData(key, data, handler); };
           void*            xxx::getUserData(const XMLCh* key) const     {return fNode.getUserData(key); };
           const XMLCh*     xxx::getBaseURI() const                      {return fNode.getBaseURI(); };
           short            xxx::compareDocumentPosition(const DOMNode* other) const {return fNode.compareDocumentPosition(other); };
           const XMLCh*     xxx::getTextContent() const                  {return fNode.getTextContent(); };
           void             xxx::setTextContent(const XMLCh* textContent){fNode.setTextContent(textContent); };
           const XMLCh*     xxx::lookupPrefix(const XMLCh* namespaceURI) const {return fNode.lookupPrefix(namespaceURI); };
           bool             xxx::isDefaultNamespace(const XMLCh* namespaceURI) const {return fNode.isDefaultNamespace(namespaceURI); };
           const XMLCh*     xxx::lookupNamespaceURI(const XMLCh* prefix) const {return fNode.lookupNamespaceURI(prefix); };
           void*            xxx::getFeature(const XMLCh* feature, const XMLCh* version) const {return fNode.getFeature(feature, version); };


*/



XERCES_CPP_NAMESPACE_END

#endif
