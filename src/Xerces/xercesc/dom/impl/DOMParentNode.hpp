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
 * $Id: DOMParentNode.hpp 678709 2008-07-22 10:56:56Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMPARENTNODE_HPP)
#define XERCESC_INCLUDE_GUARD_DOMPARENTNODE_HPP

//
//  This file is part of the internal implementation of the C++ XML DOM.
//  It should NOT be included or used directly by application programs.
//
//  Applications should include the file <xercesc/dom/DOM.hpp> for the entire
//  DOM API, or xercesc/dom/DOM*.hpp for individual DOM classes, where the class
//  name is substituded for the *.
//

/**
 * ParentNode provides the capability of having child
 * nodes. Not every node in the DOM can have children, so only nodes that can
 * should include this class and pay the price for it.
 * <P>
 * While we have a direct reference to the first child, the last child is
 * stored as the previous sibling of the first child. First child nodes are
 * marked as being so, and getNextSibling hides this fact.
 *
 **/

#include <xercesc/util/XercesDefs.hpp>
#include "DOMNodeListImpl.hpp"

XERCES_CPP_NAMESPACE_BEGIN


class DOMChildNode;
class DOMDocument;
class DOMNode;
class DOMNodeList;

class CDOM_EXPORT DOMParentNode  {
public:
    DOMDocument            *fOwnerDocument; // Document this node belongs to
    DOMNode                *fFirstChild;
    DOMNodeListImpl            fChildNodeList;      // for GetChildNodes()

public:
    DOMParentNode(DOMDocument *ownerDocument);
    DOMParentNode(const DOMParentNode &other);

    DOMDocument * getOwnerDocument() const;
    void setOwnerDocument(DOMDocument* doc);

    // Track changes to the node tree structure under this node.  An optimization
    //   for NodeLists.
    int changes() const;
    void changed();

    DOMNode*     appendChild(DOMNode *newChild);
    DOMNodeList* getChildNodes() const;
    DOMNode*     getFirstChild() const;
    DOMNode*     getLastChild() const;
    bool         hasChildNodes() const;
    DOMNode*     insertBefore(DOMNode *newChild, DOMNode *refChild);
    DOMNode*     item(unsigned int index) const;
    DOMNode*     removeChild(DOMNode *oldChild);
    DOMNode*     replaceChild(DOMNode *newChild, DOMNode *oldChild);

    // Append certain types of nodes fast. Used to speed up XML to DOM
    // parsing. See the function implementation for detail.
    virtual DOMNode*     appendChildFast(DOMNode *newChild);

    //Introduced in DOM Level 2
    void	normalize();

    //Introduced in DOM Level 3
    bool isEqualNode(const DOMNode* arg) const;

    // NON-DOM
    // unlike getOwnerDocument this never returns null, even for Document nodes
    DOMDocument * getDocument() const;
    void          release();


public:
    void cloneChildren(const DOMNode *other);
    DOMNode * lastChild() const;
    void lastChild(DOMNode *);

private:
    // unimplemented
    DOMParentNode& operator= (const DOMParentNode& other);
};

#define GetDOMParentNodeMemoryManager GET_DIRECT_MM(fOwnerDocument)

XERCES_CPP_NAMESPACE_END

#endif
