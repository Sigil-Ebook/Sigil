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
 * $Id: DOMNodeIterator.hpp 671894 2008-06-26 13:29:21Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMNODEITERATOR_HPP)
#define XERCESC_INCLUDE_GUARD_DOMNODEITERATOR_HPP

#include <xercesc/dom/DOMNodeFilter.hpp>
#include <xercesc/dom/DOMNode.hpp>

XERCES_CPP_NAMESPACE_BEGIN


/**
 * <code>DOMNodeIterators</code> are used to step through a set of nodes, e.g.
 * the set of nodes in a <code>DOMNodeList</code>, the document subtree
 * governed by a particular <code>DOMNode</code>, the results of a query, or
 * any other set of nodes. The set of nodes to be iterated is determined by
 * the implementation of the <code>DOMNodeIterator</code>. DOM Level 2
 * specifies a single <code>DOMNodeIterator</code> implementation for
 * document-order traversal of a document subtree. Instances of these
 * <code>DOMNodeIterators</code> are created by calling
 * <code>DOMDocumentTraversal</code><code>.createNodeIterator()</code>.
 * <p>See also the <a href='http://www.w3.org/TR/2000/REC-DOM-Level-2-Traversal-Range-20001113'>Document Object Model (DOM) Level 2 Traversal and Range Specification</a>.
 * @since DOM Level 2
 */
class CDOM_EXPORT DOMNodeIterator
{
protected:
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    /** @name Hidden constructors */
    //@{
    DOMNodeIterator() {}
    DOMNodeIterator(const DOMNodeIterator &) {}
    //@}

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    /** @name Unimplemented operators */
    //@{
    DOMNodeIterator & operator = (const DOMNodeIterator &);
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
    virtual ~DOMNodeIterator() {};
    //@}

    // -----------------------------------------------------------------------
    //  Virtual DOMNodeFilter interface
    // -----------------------------------------------------------------------
    /** @name Functions introduced in DOM Level 2 */
    //@{
    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    /**
     * The <code>root</code> node of the <code>DOMNodeIterator</code>, as specified
     * when it was created.
     * @since DOM Level 2
     */
    virtual DOMNode*           getRoot() = 0;
    /**
     * Return which node types are presented via the iterator.
     * This attribute determines which node types are presented via the
     * <code>DOMNodeIterator</code>. The available set of constants is defined
     * in the <code>DOMNodeFilter</code> interface.  Nodes not accepted by
     * <code>whatToShow</code> will be skipped, but their children may still
     * be considered. Note that this skip takes precedence over the filter,
     * if any.
     * @since DOM Level 2
     *
     */
    virtual DOMNodeFilter::ShowType getWhatToShow() = 0;

    /**
     * The <code>DOMNodeFilter</code> used to screen nodes.
     *
     * @since DOM Level 2
     */
    virtual DOMNodeFilter*     getFilter() = 0;

    /**
     * Return the expandEntityReferences flag.
     * The value of this flag determines whether the children of entity
     * reference nodes are visible to the <code>DOMNodeIterator</code>. If
     * false, these children  and their descendants will be rejected. Note
     * that this rejection takes precedence over <code>whatToShow</code> and
     * the filter. Also note that this is currently the only situation where
     * <code>DOMNodeIterators</code> may reject a complete subtree rather than
     * skipping individual nodes.
     * <br>
     * <br> To produce a view of the document that has entity references
     * expanded and does not expose the entity reference node itself, use
     * the <code>whatToShow</code> flags to hide the entity reference node
     * and set <code>expandEntityReferences</code> to true when creating the
     * <code>DOMNodeIterator</code>. To produce a view of the document that has
     * entity reference nodes but no entity expansion, use the
     * <code>whatToShow</code> flags to show the entity reference node and
     * set <code>expandEntityReferences</code> to false.
     *
     * @since DOM Level 2
     */
    virtual bool               getExpandEntityReferences() = 0;

    // -----------------------------------------------------------------------
    //  Query methods
    // -----------------------------------------------------------------------
    /**
     * Returns the next node in the set and advances the position of the
     * <code>DOMNodeIterator</code> in the set. After a
     * <code>DOMNodeIterator</code> is created, the first call to
     * <code>nextNode()</code> returns the first node in the set.
     * @return The next <code>DOMNode</code> in the set being iterated over, or
     *   <code>null</code> if there are no more members in that set.
     * @exception DOMException
     *   INVALID_STATE_ERR: Raised if this method is called after the
     *   <code>detach</code> method was invoked.
     * @since DOM Level 2
     */
    virtual DOMNode*           nextNode() = 0;

    /**
     * Returns the previous node in the set and moves the position of the
     * <code>DOMNodeIterator</code> backwards in the set.
     * @return The previous <code>DOMNode</code> in the set being iterated over,
     *   or <code>null</code> if there are no more members in that set.
     * @exception DOMException
     *   INVALID_STATE_ERR: Raised if this method is called after the
     *   <code>detach</code> method was invoked.
     * @since DOM Level 2
     */
    virtual DOMNode*           previousNode() = 0;

    /**
     * Detaches the <code>DOMNodeIterator</code> from the set which it iterated
     * over, releasing any computational resources and placing the
     * <code>DOMNodeIterator</code> in the INVALID state. After
     * <code>detach</code> has been invoked, calls to <code>nextNode</code>
     * or <code>previousNode</code> will raise the exception
     * INVALID_STATE_ERR.
     * @since DOM Level 2
     */
    virtual void               detach() = 0;
    //@}

    // -----------------------------------------------------------------------
    //  Non-standard Extension
    // -----------------------------------------------------------------------
    /** @name Non-standard Extension */
    //@{
    /**
     * Called to indicate that this NodeIterator is no longer in use
     * and that the implementation may relinquish any resources associated with it.
     * (release() will call detach() where appropriate)
     *
     * Access to a released object will lead to unexpected result.
     */
    virtual void               release() = 0;
    //@}
};

#define GetDOMNodeIteratorMemoryManager GET_DIRECT_MM(fDocument)

XERCES_CPP_NAMESPACE_END

#endif
