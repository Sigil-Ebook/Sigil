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
 * $Id: DOMTreeWalkerImpl.hpp 671894 2008-06-26 13:29:21Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMTREEWALKERIMPL_HPP)
#define XERCESC_INCLUDE_GUARD_DOMTREEWALKERIMPL_HPP

//
//  This file is part of the internal implementation of the C++ XML DOM.
//  It should NOT be included or used directly by application programs.
//
//  Applications should include the file <xercesc/dom/DOM.hpp> for the entire
//  DOM API, or xercesc/dom/DOM*.hpp for individual DOM classes, where the class
//  name is substituded for the *.
//

#include <xercesc/dom/DOMTreeWalker.hpp>

XERCES_CPP_NAMESPACE_BEGIN


class CDOM_EXPORT DOMTreeWalkerImpl : public DOMTreeWalker {
protected:
        // The whatToShow mask.
        DOMNodeFilter::ShowType fWhatToShow;

        // The NodeFilter reference.
        DOMNodeFilter* fNodeFilter;

        // The current Node.
        DOMNode* fCurrentNode;

        // The root Node.
        DOMNode* fRoot;

        // The expandEntity reference flag.
        bool fExpandEntityReferences;

public:
    // Implementation Note: No state is kept except the data above
    // (fWhatToShow, fNodeFilter, fCurrentNode, fRoot) such that
    // setters could be created for these data values and the
    // implementation will still work.

    /** Public constructor */
    DOMTreeWalkerImpl (
        DOMNode* root,
        DOMNodeFilter::ShowType whatToShow,
        DOMNodeFilter* nodeFilter,
        bool expandEntityRef);
    DOMTreeWalkerImpl (const DOMTreeWalkerImpl& twi);
    DOMTreeWalkerImpl& operator= (const DOMTreeWalkerImpl& twi);

    // Return the root node.
    virtual DOMNode* getRoot ();

    // Return the whatToShow value.
    virtual DOMNodeFilter::ShowType  getWhatToShow ();

    // Return the NodeFilter.
    virtual DOMNodeFilter* getFilter ();


    // Return the current DOMNode.
    virtual DOMNode* getCurrentNode ();

    // Return the current Node.
    virtual void setCurrentNode (DOMNode* node);

    // Return the parent Node from the current node,
    //  after applying filter, whatToshow.
    //  If result is not null, set the current Node.
    virtual DOMNode* parentNode ();

    // Return the first child Node from the current node,
    //  after applying filter, whatToshow.
    //  If result is not null, set the current Node.
    virtual DOMNode* firstChild ();

    // Return the last child Node from the current node,
    //  after applying filter, whatToshow.
    //  If result is not null, set the current Node.
    virtual DOMNode* lastChild ();

    // Return the previous sibling Node from the current node,
    //  after applying filter, whatToshow.
    //  If result is not null, set the current Node.
    virtual DOMNode* previousSibling ();

    // Return the next sibling Node from the current node,
    //  after applying filter, whatToshow.
    //  If result is not null, set the current Node.

    virtual DOMNode* nextSibling ();
    // Return the previous Node from the current node,
    //  after applying filter, whatToshow.
    //  If result is not null, set the current Node.
    virtual DOMNode* previousNode ();

    // Return the next Node from the current node,
    //  after applying filter, whatToshow.
    //  If result is not null, set the current Node.
    virtual DOMNode* nextNode ();

    // Get the expandEntity reference flag.
    virtual bool getExpandEntityReferences();

    // release the resource
    virtual void release();

protected:

    // Internal function.
    //  Return the parent Node, from the input node
    //  after applying filter, whatToshow.
    //  The current node is not consulted or set.
    DOMNode* getParentNode (DOMNode* node);

    // Internal function.
    //  Return the nextSibling Node, from the input node
    //  after applying filter, whatToshow.
    //  The current node is not consulted or set.
    DOMNode* getNextSibling (DOMNode* node);

    // Internal function.
    //  Return the previous sibling Node, from the input node
    //  after applying filter, whatToshow.
    //  The current node is not consulted or set.
    DOMNode* getPreviousSibling (DOMNode* node);

    // Internal function.
    //  Return the first child Node, from the input node
    //  after applying filter, whatToshow.
    //  The current node is not consulted or set.
    DOMNode* getFirstChild (DOMNode* node);

    // Internal function.
    //  Return the last child Node, from the input node
    //  after applying filter, whatToshow.
    //  The current node is not consulted or set.
    DOMNode* getLastChild (DOMNode* node);

    // The node is accepted if it passes the whatToShow and the filter.
    short acceptNode (DOMNode* node);


};

XERCES_CPP_NAMESPACE_END

#endif
