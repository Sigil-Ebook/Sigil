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
 * $Id: DOMTreeWalker.hpp 671894 2008-06-26 13:29:21Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMTREEWALKER_HPP)
#define XERCESC_INCLUDE_GUARD_DOMTREEWALKER_HPP

#include <xercesc/dom/DOMNode.hpp>
#include <xercesc/dom/DOMNodeFilter.hpp>

XERCES_CPP_NAMESPACE_BEGIN


/**
 * <code>DOMTreeWalker</code> objects are used to navigate a document tree or
 * subtree using the view of the document defined by their
 * <code>whatToShow</code> flags and filter (if any). Any function which
 * performs navigation using a <code>DOMTreeWalker</code> will automatically
 * support any view defined by a <code>DOMTreeWalker</code>.
 * <p>Omitting nodes from the logical view of a subtree can result in a
 * structure that is substantially different from the same subtree in the
 * complete, unfiltered document. Nodes that are siblings in the
 * <code>DOMTreeWalker</code> view may be children of different, widely
 * separated nodes in the original view. For instance, consider a
 * <code>DOMNodeFilter</code> that skips all nodes except for DOMText nodes and
 * the root node of a document. In the logical view that results, all text
 * nodes will be siblings and appear as direct children of the root node, no
 * matter how deeply nested the structure of the original document.
 * <p>See also the <a href='http://www.w3.org/TR/2000/REC-DOM-Level-2-Traversal-Range-20001113'>Document Object Model (DOM) Level 2 Traversal and Range Specification</a>.
 *
 * @since DOM Level 2
 */
class CDOM_EXPORT DOMTreeWalker {
protected:
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    /** @name Hidden constructors */
    //@{
    DOMTreeWalker() {}
    DOMTreeWalker(const DOMTreeWalker &) {}
    //@}

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    /** @name Unimplemented constructors and operators */
    //@{
    DOMTreeWalker & operator = (const DOMTreeWalker &);
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
    virtual ~DOMTreeWalker() {};
    //@}

    // -----------------------------------------------------------------------
    //  Virtual DOMTreeWalker interface
    // -----------------------------------------------------------------------
    /** @name Functions introduced in DOM Level 2 */
    //@{
    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------

    /**
     * The <code>root</code> node of the <code>DOMTreeWalker</code>, as specified
     * when it was created.
     *
     * @since DOM Level 2
     */
    virtual DOMNode*          getRoot() = 0;
    /**
     * This attribute determines which node types are presented via the
     * <code>DOMTreeWalker</code>. The available set of constants is defined in
     * the <code>DOMNodeFilter</code> interface.  Nodes not accepted by
     * <code>whatToShow</code> will be skipped, but their children may still
     * be considered. Note that this skip takes precedence over the filter,
     * if any.
     *
     * @since DOM Level 2
     */
    virtual DOMNodeFilter::ShowType getWhatToShow()= 0;

    /**
     * Return The filter used to screen nodes.
     *
     * @since DOM Level 2
     */
    virtual DOMNodeFilter*	   getFilter()= 0;

    /**
     * The value of this flag determines whether the children of entity
     * reference nodes are visible to the <code>DOMTreeWalker</code>. If false,
     * these children  and their descendants will be rejected. Note that
     * this rejection takes precedence over <code>whatToShow</code> and the
     * filter, if any.
     * <br> To produce a view of the document that has entity references
     * expanded and does not expose the entity reference node itself, use
     * the <code>whatToShow</code> flags to hide the entity reference node
     * and set <code>expandEntityReferences</code> to true when creating the
     * <code>DOMTreeWalker</code>. To produce a view of the document that has
     * entity reference nodes but no entity expansion, use the
     * <code>whatToShow</code> flags to show the entity reference node and
     * set <code>expandEntityReferences</code> to false.
     *
     * @since DOM Level 2
     */
    virtual bool              getExpandEntityReferences()= 0;

    /**
     * Return the node at which the DOMTreeWalker is currently positioned.
     *
     * @since DOM Level 2
     */
    virtual DOMNode*          getCurrentNode()= 0;

    // -----------------------------------------------------------------------
    //  Query methods
    // -----------------------------------------------------------------------
    /**
     * Moves to and returns the closest visible ancestor node of the current
     * node. If the search for <code>parentNode</code> attempts to step
     * upward from the <code>DOMTreeWalker</code>'s <code>root</code> node, or
     * if it fails to find a visible ancestor node, this method retains the
     * current position and returns <code>null</code>.
     * @return The new parent node, or <code>null</code> if the current node
     *   has no parent  in the <code>DOMTreeWalker</code>'s logical view.
     *
     * @since DOM Level 2
     */
    virtual DOMNode*          parentNode()= 0;

    /**
     * Moves the <code>DOMTreeWalker</code> to the first visible child of the
     * current node, and returns the new node. If the current node has no
     * visible children, returns <code>null</code>, and retains the current
     * node.
     * @return The new node, or <code>null</code> if the current node has no
     *   visible children  in the <code>DOMTreeWalker</code>'s logical view.
     *
     * @since DOM Level 2
     */
    virtual DOMNode*          firstChild()= 0;

    /**
     * Moves the <code>DOMTreeWalker</code> to the last visible child of the
     * current node, and returns the new node. If the current node has no
     * visible children, returns <code>null</code>, and retains the current
     * node.
     * @return The new node, or <code>null</code> if the current node has no
     *   children  in the <code>DOMTreeWalker</code>'s logical view.
     *
     * @since DOM Level 2
     */
    virtual DOMNode*          lastChild()= 0;

    /**
     * Moves the <code>DOMTreeWalker</code> to the previous sibling of the
     * current node, and returns the new node. If the current node has no
     * visible previous sibling, returns <code>null</code>, and retains the
     * current node.
     * @return The new node, or <code>null</code> if the current node has no
     *   previous sibling.  in the <code>DOMTreeWalker</code>'s logical view.
     *
     * @since DOM Level 2
     */
    virtual DOMNode*          previousSibling()= 0;

    /**
     * Moves the <code>DOMTreeWalker</code> to the next sibling of the current
     * node, and returns the new node. If the current node has no visible
     * next sibling, returns <code>null</code>, and retains the current node.
     * @return The new node, or <code>null</code> if the current node has no
     *   next sibling.  in the <code>DOMTreeWalker</code>'s logical view.
     *
     * @since DOM Level 2
     */
    virtual DOMNode*          nextSibling()= 0;

    /**
     * Moves the <code>DOMTreeWalker</code> to the previous visible node in
     * document order relative to the current node, and returns the new
     * node. If the current node has no previous node,  or if the search for
     * <code>previousNode</code> attempts to step upward from the
     * <code>DOMTreeWalker</code>'s <code>root</code> node,  returns
     * <code>null</code>, and retains the current node.
     * @return The new node, or <code>null</code> if the current node has no
     *   previous node  in the <code>DOMTreeWalker</code>'s logical view.
     *
     * @since DOM Level 2
     */
    virtual DOMNode*          previousNode()= 0;

    /**
     * Moves the <code>DOMTreeWalker</code> to the next visible node in document
     * order relative to the current node, and returns the new node. If the
     * current node has no next node, or if the search for nextNode attempts
     * to step upward from the <code>DOMTreeWalker</code>'s <code>root</code>
     * node, returns <code>null</code>, and retains the current node.
     * @return The new node, or <code>null</code> if the current node has no
     *   next node  in the <code>DOMTreeWalker</code>'s logical view.
     *
     * @since DOM Level 2
     */
    virtual DOMNode*          nextNode()= 0;

    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------
    /**
     * The node at which the <code>DOMTreeWalker</code> is currently positioned.
     * <br>Alterations to the DOM tree may cause the current node to no longer
     * be accepted by the <code>DOMTreeWalker</code>'s associated filter.
     * <code>currentNode</code> may also be explicitly set to any node,
     * whether or not it is within the subtree specified by the
     * <code>root</code> node or would be accepted by the filter and
     * <code>whatToShow</code> flags. Further traversal occurs relative to
     * <code>currentNode</code> even if it is not part of the current view,
     * by applying the filters in the requested direction; if no traversal
     * is possible, <code>currentNode</code> is not changed.
     * @exception DOMException
     *   NOT_SUPPORTED_ERR: Raised if an attempt is made to set
     *   <code>currentNode</code> to <code>null</code>.
     *
     * @since DOM Level 2
     */
    virtual void              setCurrentNode(DOMNode* currentNode)= 0;
    //@}

    // -----------------------------------------------------------------------
    //  Non-standard Extension
    // -----------------------------------------------------------------------
    /** @name Non-standard Extension */
    //@{
    /**
     * Called to indicate that this TreeWalker is no longer in use
     * and that the implementation may relinquish any resources associated with it.
     *
     * Access to a released object will lead to unexpected result.
     */
    virtual void              release() = 0;
    //@}
};

#define GetDOMTreeWalkerMemoryManager GET_INDIRECT_MM(fCurrentNode)

XERCES_CPP_NAMESPACE_END

#endif
