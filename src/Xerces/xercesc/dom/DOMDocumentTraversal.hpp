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
 * $Id: DOMDocumentTraversal.hpp 671894 2008-06-26 13:29:21Z borisk $
*/

#if !defined(XERCESC_INCLUDE_GUARD_DOMDOCUMENTTRAVERSAL_HPP)
#define XERCESC_INCLUDE_GUARD_DOMDOCUMENTTRAVERSAL_HPP

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/dom/DOMNodeFilter.hpp>

XERCES_CPP_NAMESPACE_BEGIN


class DOMNode;
class DOMNodeIterator;
class DOMTreeWalker;


/**
 * <code>DOMDocumentTraversal</code> contains methods that create
 * <code>DOMNodeIterators</code> and <code>DOMTreeWalkers</code> to traverse a
 * node and its children in document order (depth first, pre-order
 * traversal, which is equivalent to the order in which the start tags occur
 * in the text representation of the document). In DOMs which support the
 * Traversal feature, <code>DOMDocumentTraversal</code> will be implemented by
 * the same objects that implement the DOMDocument interface.
 * <p>See also the <a href='http://www.w3.org/TR/2000/REC-DOM-Level-2-Traversal-Range-20001113'>Document Object Model (DOM) Level 2 Traversal and Range Specification</a>.
 * @since DOM Level 2
 */
class CDOM_EXPORT DOMDocumentTraversal {

protected:
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    /** @name Hidden constructors */
    //@{
    DOMDocumentTraversal() {};
    //@}

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    /** @name Unimplemented constructors and operators */
    //@{
    DOMDocumentTraversal(const DOMDocumentTraversal &);
    DOMDocumentTraversal & operator = (const DOMDocumentTraversal &);
    //@}

public:
    // -----------------------------------------------------------------------
    //  All constructors are hidden, just the destructor is available
    // -----------------------------------------------------------------------
    /** @name Destructor */
    //@{
    /**
     * Destructor
     *
     */
    virtual ~DOMDocumentTraversal() {};
    //@}

    // -----------------------------------------------------------------------
    //  Virtual DOMDocumentRange interface
    // -----------------------------------------------------------------------
    /** @name Functions introduced in DOM Level 2 */
    //@{
    /**
     * Creates a NodeIterator object.   (DOM2)
     *
     * NodeIterators are used to step through a set of nodes, e.g. the set of nodes in a NodeList, the
     * document subtree governed by a particular node, the results of a query, or any other set of nodes.
     * The set of nodes to be iterated is determined by the implementation of the NodeIterator. DOM Level 2
     * specifies a single NodeIterator implementation for document-order traversal of a document subtree.
     * Instances of these iterators are created by calling <code>DOMDocumentTraversal.createNodeIterator()</code>.
     *
     * To produce a view of the document that has entity references expanded and does not
     * expose the entity reference node itself, use the <code>whatToShow</code> flags to hide the entity
     * reference node and set expandEntityReferences to true when creating the iterator. To
     * produce a view of the document that has entity reference nodes but no entity expansion,
     * use the <code>whatToShow</code> flags to show the entity reference node and set
     * expandEntityReferences to false.
     *
     * @param root The root node of the DOM tree
     * @param whatToShow This attribute determines which node types are presented via the iterator.
     * @param filter The filter used to screen nodes
     * @param entityReferenceExpansion The value of this flag determines whether the children of entity reference nodes are
     *                   visible to the iterator. If false, they will be skipped over.
     * @since DOM Level 2
     */

    virtual DOMNodeIterator *createNodeIterator(DOMNode* root,
                                                DOMNodeFilter::ShowType whatToShow,
                                                DOMNodeFilter* filter,
                                                bool entityReferenceExpansion) = 0;
    /**
     * Creates a TreeWalker object.   (DOM2)
     *
     * TreeWalker objects are used to navigate a document tree or subtree using the view of the document defined
     * by its whatToShow flags and any filters that are defined for the TreeWalker. Any function which performs
     * navigation using a TreeWalker will automatically support any view defined by a TreeWalker.
     *
     * Omitting nodes from the logical view of a subtree can result in a structure that is substantially different from
     * the same subtree in the complete, unfiltered document. Nodes that are siblings in the TreeWalker view may
     * be children of different, widely separated nodes in the original view. For instance, consider a Filter that skips
     * all nodes except for DOMText nodes and the root node of a document. In the logical view that results, all text
     * nodes will be siblings and appear as direct children of the root node, no matter how deeply nested the
     * structure of the original document.
     *
     * To produce a view of the document that has entity references expanded
     * and does not expose the entity reference node itself, use the whatToShow
     * flags to hide the entity reference node and set <code>expandEntityReferences</code> to
     * true when creating the TreeWalker. To produce a view of the document
     * that has entity reference nodes but no entity expansion, use the
     * <code>whatToShow</code> flags to show the entity reference node and set
     * <code>expandEntityReferences</code> to false
     *
     * @param root The root node of the DOM tree
     * @param whatToShow This attribute determines which node types are presented via the tree-walker.
     * @param filter The filter used to screen nodes
     * @param entityReferenceExpansion The value of this flag determines whether the children of entity reference nodes are
     *                   visible to the tree-walker. If false, they will be skipped over.
     * @since DOM Level 2
     */

    virtual DOMTreeWalker  *createTreeWalker(DOMNode* root,
                                             DOMNodeFilter::ShowType whatToShow,
                                             DOMNodeFilter* filter,
                                             bool entityReferenceExpansion) = 0;

    //@}
};


XERCES_CPP_NAMESPACE_END

#endif
