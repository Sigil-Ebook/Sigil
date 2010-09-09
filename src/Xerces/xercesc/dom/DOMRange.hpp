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
 * $Id: DOMRange.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMRANGE_HPP)
#define XERCESC_INCLUDE_GUARD_DOMRANGE_HPP

#include <xercesc/util/XercesDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class DOMNode;
class DOMDocumentFragment;

/**
 * <p>See also the <a href='http://www.w3.org/TR/2000/REC-DOM-Level-2-Traversal-Range-20001113'>Document Object Model (DOM) Level 2 Traversal and Range Specification</a>.
 * @since DOM Level 2
 */
class CDOM_EXPORT DOMRange {
protected:
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    /** @name Hidden constructors */
    //@{    
    DOMRange() {}
    DOMRange(const DOMRange &) {}
    //@}

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    /** @name Unimplemented operators */
    //@{
    DOMRange & operator = (const DOMRange &);
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
    virtual ~DOMRange() {};
    //@}

    // -----------------------------------------------------------------------
    //  Class Types
    // -----------------------------------------------------------------------
    /** @name Public Constants */
    //@{
    /**
     * Constants CompareHow.
     *
     * <p><code>START_TO_START:</code>
     * Compare start boundary-point of <code>sourceRange</code> to start
     * boundary-point of Range on which <code>compareBoundaryPoints</code>
     * is invoked.</p>
     *
     * <p><code>START_TO_END:</code>
     * Compare start boundary-point of <code>sourceRange</code> to end
     * boundary-point of Range on which <code>compareBoundaryPoints</code>
     * is invoked.</p>
     *
     * <p><code>END_TO_END:</code>
     * Compare end boundary-point of <code>sourceRange</code> to end
     * boundary-point of Range on which <code>compareBoundaryPoints</code>
     * is invoked.</p>
     *
     * <p><code>END_TO_START:</code>
     * Compare end boundary-point of <code>sourceRange</code> to start
     * boundary-point of Range on which <code>compareBoundaryPoints</code>
     * is invoked.</p>
     *
     * @since DOM Level 2
     */
    enum CompareHow {
        START_TO_START  = 0,
        START_TO_END    = 1,
        END_TO_END      = 2,
        END_TO_START    = 3
    };

    //@}

    // -----------------------------------------------------------------------
    //  Virtual DOMRange interface
    // -----------------------------------------------------------------------
    /** @name Functions introduced in DOM Level 2 */
    //@{
    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    /**
     * DOMNode within which the Range begins
     * @exception DOMException
     *   INVALID_STATE_ERR: Raised if <code>detach()</code> has already been
     *   invoked on this object.
     *
     * @since DOM Level 2
     */
    virtual DOMNode* getStartContainer() const = 0;

    /**
     * Offset within the starting node of the Range.
     * @exception DOMException
     *   INVALID_STATE_ERR: Raised if <code>detach()</code> has already been
     *   invoked on this object.
     *
     * @since DOM Level 2
     */
    virtual XMLSize_t getStartOffset() const = 0;

    /**
     * DOMNode within which the Range ends
     * @exception DOMException
     *   INVALID_STATE_ERR: Raised if <code>detach()</code> has already been
     *   invoked on this object.
     *
     * @since DOM Level 2
     */
    virtual DOMNode* getEndContainer() const = 0;

    /**
     * Offset within the ending node of the Range.
     * @exception DOMException
     *   INVALID_STATE_ERR: Raised if <code>detach()</code> has already been
     *   invoked on this object.
     *
     * @since DOM Level 2
     */
    virtual XMLSize_t getEndOffset() const = 0;

    /**
     * TRUE if the Range is collapsed
     * @exception DOMException
     *   INVALID_STATE_ERR: Raised if <code>detach()</code> has already been
     *   invoked on this object.
     *
     * @since DOM Level 2
     */
    virtual bool getCollapsed() const = 0;

    /**
     * The deepest common ancestor container of the Range's two
     * boundary-points.
     * @exception DOMException
     *   INVALID_STATE_ERR: Raised if <code>detach()</code> has already been
     *   invoked on this object.
     *
     * @since DOM Level 2
     */
    virtual const DOMNode* getCommonAncestorContainer() const = 0;

    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------
    /**
     * Sets the attributes describing the start of the Range.
     * @param refNode The <code>refNode</code> value. This parameter must be
     *   different from <code>null</code>.
     * @param offset The <code>startOffset</code> value.
     * @exception DOMRangeException
     *   INVALID_NODE_TYPE_ERR: Raised if <code>refNode</code> or an ancestor
     *   of <code>refNode</code> is an DOMEntity, DOMNotation, or DOMDocumentType
     *   node.
     * @exception DOMException
     *   INDEX_SIZE_ERR: Raised if <code>offset</code> is negative or greater
     *   than the number of child units in <code>refNode</code>. Child units
     *   are 16-bit units if <code>refNode</code> is a type of DOMCharacterData
     *   node (e.g., a DOMText or DOMComment node) or a DOMProcessingInstruction
     *   node. Child units are Nodes in all other cases.
     *   <br>INVALID_STATE_ERR: Raised if <code>detach()</code> has already
     *   been invoked on this object.
     *   <br>WRONG_DOCUMENT_ERR: Raised if <code>refNode</code> was created
     *   from a different document than the one that created this range.
     *
     * @since DOM Level 2
     */
    virtual void setStart(const DOMNode *refNode, XMLSize_t offset) = 0;

    /**
     * Sets the attributes describing the end of a Range.
     * @param refNode The <code>refNode</code> value. This parameter must be
     *   different from <code>null</code>.
     * @param offset The <code>endOffset</code> value.
     * @exception DOMRangeException
     *   INVALID_NODE_TYPE_ERR: Raised if <code>refNode</code> or an ancestor
     *   of <code>refNode</code> is an DOMEntity, DOMNotation, or DOMDocumentType
     *   node.
     * @exception DOMException
     *   INDEX_SIZE_ERR: Raised if <code>offset</code> is negative or greater
     *   than the number of child units in <code>refNode</code>. Child units
     *   are 16-bit units if <code>refNode</code> is a type of DOMCharacterData
     *   node (e.g., a DOMText or DOMComment node) or a DOMProcessingInstruction
     *   node. Child units are Nodes in all other cases.
     *   <br>INVALID_STATE_ERR: Raised if <code>detach()</code> has already
     *   been invoked on this object.
     *   <br>WRONG_DOCUMENT_ERR: Raised if <code>refNode</code> was created
     *   from a different document than the one that created this range.
     *
     * @since DOM Level 2
     */
    virtual void setEnd(const DOMNode *refNode, XMLSize_t offset) = 0;

    /**
     * Sets the start position to be before a node
     * @param refNode Range starts before <code>refNode</code>
     * @exception DOMRangeException
     *   INVALID_NODE_TYPE_ERR: Raised if the root container of
     *   <code>refNode</code> is not an DOMAttr, DOMDocument, or DOMDocumentFragment
     *   node or if <code>refNode</code> is a DOMDocument, DOMDocumentFragment,
     *   DOMAttr, DOMEntity, or DOMNotation node.
     * @exception DOMException
     *   INVALID_STATE_ERR: Raised if <code>detach()</code> has already been
     *   invoked on this object.
     *   <br>WRONG_DOCUMENT_ERR: Raised if <code>refNode</code> was created
     *   from a different document than the one that created this range.
     *
     * @since DOM Level 2
     */
    virtual void setStartBefore(const DOMNode *refNode) = 0;

    /**
     * Sets the start position to be after a node
     * @param refNode Range starts after <code>refNode</code>
     * @exception DOMRangeException
     *   INVALID_NODE_TYPE_ERR: Raised if the root container of
     *   <code>refNode</code> is not an DOMAttr, DOMDocument, or DOMDocumentFragment
     *   node or if <code>refNode</code> is a DOMDocument, DOMDocumentFragment,
     *   DOMAttr, DOMEntity, or DOMNotation node.
     * @exception DOMException
     *   INVALID_STATE_ERR: Raised if <code>detach()</code> has already been
     *   invoked on this object.
     *   <br>WRONG_DOCUMENT_ERR: Raised if <code>refNode</code> was created
     *   from a different document than the one that created this range.
     *
     * @since DOM Level 2
     */
    virtual void setStartAfter(const DOMNode *refNode) = 0;

    /**
     * Sets the end position to be before a node.
     * @param refNode Range ends before <code>refNode</code>
     * @exception DOMRangeException
     *   INVALID_NODE_TYPE_ERR: Raised if the root container of
     *   <code>refNode</code> is not an DOMAttr, DOMDocument, or DOMDocumentFragment
     *   node or if <code>refNode</code> is a DOMDocument, DOMDocumentFragment,
     *   DOMAttr, DOMEntity, or DOMNotation node.
     * @exception DOMException
     *   INVALID_STATE_ERR: Raised if <code>detach()</code> has already been
     *   invoked on this object.
     *   <br>WRONG_DOCUMENT_ERR: Raised if <code>refNode</code> was created
     *   from a different document than the one that created this range.
     *
     * @since DOM Level 2
     */
    virtual void setEndBefore(const DOMNode *refNode) = 0;

    /**
     * Sets the end of a Range to be after a node
     * @param refNode Range ends after <code>refNode</code>.
     * @exception DOMRangeException
     *   INVALID_NODE_TYPE_ERR: Raised if the root container of
     *   <code>refNode</code> is not a DOMAttr, DOMDocument or DOMDocumentFragment
     *   node or if <code>refNode</code> is a DOMDocument, DOMDocumentFragment,
     *   DOMAttr, DOMEntity, or DOMNotation node.
     * @exception DOMException
     *   INVALID_STATE_ERR: Raised if <code>detach()</code> has already been
     *   invoked on this object.
     *   <br>WRONG_DOCUMENT_ERR: Raised if <code>refNode</code> was created
     *   from a different document than the one that created this range.
     *
     * @since DOM Level 2
     */
    virtual void setEndAfter(const DOMNode *refNode) = 0;

    // -----------------------------------------------------------------------
    //  Misc methods
    // -----------------------------------------------------------------------
    /**
     * Collapse a Range onto one of its boundary-points
     * @param toStart If TRUE, collapses the Range onto its start; if FALSE,
     *   collapses it onto its end.
     * @exception DOMException
     *   INVALID_STATE_ERR: Raised if <code>detach()</code> has already been
     *   invoked on this object.
     *
     * @since DOM Level 2
     */
    virtual void collapse(bool toStart) = 0;

    /**
     * Select a node and its contents
     * @param refNode The node to select.
     * @exception DOMRangeException
     *   INVALID_NODE_TYPE_ERR: Raised if an ancestor of <code>refNode</code>
     *   is an DOMEntity, DOMNotation or DOMDocumentType node or if
     *   <code>refNode</code> is a DOMDocument, DOMDocumentFragment, DOMAttr, DOMEntity,
     *   or DOMNotation node.
     * @exception DOMException
     *   INVALID_STATE_ERR: Raised if <code>detach()</code> has already been
     *   invoked on this object.
     *   <br>WRONG_DOCUMENT_ERR: Raised if <code>refNode</code> was created
     *   from a different document than the one that created this range.
     *
     * @since DOM Level 2
     */
    virtual void selectNode(const DOMNode *refNode) = 0;

    /**
     * Select the contents within a node
     * @param refNode DOMNode to select from
     * @exception DOMRangeException
     *   INVALID_NODE_TYPE_ERR: Raised if <code>refNode</code> or an ancestor
     *   of <code>refNode</code> is an DOMEntity, DOMNotation or DOMDocumentType node.
     * @exception DOMException
     *   INVALID_STATE_ERR: Raised if <code>detach()</code> has already been
     *   invoked on this object.
     *   <br>WRONG_DOCUMENT_ERR: Raised if <code>refNode</code> was created
     *   from a different document than the one that created this range.
     *
     * @since DOM Level 2
     */
    virtual void selectNodeContents(const DOMNode *refNode) = 0;

    /**
     * Compare the boundary-points of two Ranges in a document.
     * @param how A code representing the type of comparison, as defined
     *   above.
     * @param sourceRange The <code>Range</code> on which this current
     *   <code>Range</code> is compared to.
     * @return  -1, 0 or 1 depending on whether the corresponding
     *   boundary-point of the Range is respectively before, equal to, or
     *   after the corresponding boundary-point of <code>sourceRange</code>.
     * @exception DOMException
     *   WRONG_DOCUMENT_ERR: Raised if the two Ranges are not in the same
     *   DOMDocument or DOMDocumentFragment.
     *   <br>INVALID_STATE_ERR: Raised if <code>detach()</code> has already
     *   been invoked on this object.
     *
     * @since DOM Level 2
     */
    virtual short compareBoundaryPoints(CompareHow how, const DOMRange* sourceRange) const = 0;

    /**
     * Removes the contents of a Range from the containing document or
     * document fragment without returning a reference to the removed
     * content.
     * @exception DOMException
     *   NO_MODIFICATION_ALLOWED_ERR: Raised if any portion of the content of
     *   the Range is read-only or any of the nodes that contain any of the
     *   content of the Range are read-only.
     *   <br>INVALID_STATE_ERR: Raised if <code>detach()</code> has already
     *   been invoked on this object.
     *
     * @since DOM Level 2
     */
    virtual void deleteContents() = 0;

    /**
     * Moves the contents of a Range from the containing document or document
     * fragment to a new DOMDocumentFragment.
     * @return A DOMDocumentFragment containing the extracted contents.
     * @exception DOMException
     *   NO_MODIFICATION_ALLOWED_ERR: Raised if any portion of the content of
     *   the Range is read-only or any of the nodes which contain any of the
     *   content of the Range are read-only.
     *   <br>HIERARCHY_REQUEST_ERR: Raised if a DOMDocumentType node would be
     *   extracted into the new DOMDocumentFragment.
     *   <br>INVALID_STATE_ERR: Raised if <code>detach()</code> has already
     *   been invoked on this object.
     *
     * @since DOM Level 2
     */
    virtual DOMDocumentFragment* extractContents() = 0;

    /**
     * Duplicates the contents of a Range
     * @return A DOMDocumentFragment that contains content equivalent to this
     *   Range.
     * @exception DOMException
     *   HIERARCHY_REQUEST_ERR: Raised if a DOMDocumentType node would be
     *   extracted into the new DOMDocumentFragment.
     *   <br>INVALID_STATE_ERR: Raised if <code>detach()</code> has already
     *   been invoked on this object.
     *
     * @since DOM Level 2
     */
    virtual DOMDocumentFragment* cloneContents() const = 0;

    /**
     * Inserts a node into the DOMDocument or DOMDocumentFragment at the start of
     * the Range. If the container is a DOMText node, this will be split at the
     * start of the Range (as if the DOMText node's splitText method was
     * performed at the insertion point) and the insertion will occur
     * between the two resulting DOMText nodes. Adjacent DOMText nodes will not be
     * automatically merged. If the node to be inserted is a
     * DOMDocumentFragment node, the children will be inserted rather than the
     * DOMDocumentFragment node itself.
     * @param newNode The node to insert at the start of the Range
     * @exception DOMException
     *   NO_MODIFICATION_ALLOWED_ERR: Raised if an ancestor container of the
     *   start of the Range is read-only.
     *   <br>WRONG_DOCUMENT_ERR: Raised if <code>newNode</code> and the
     *   container of the start of the Range were not created from the same
     *   document.
     *   <br>HIERARCHY_REQUEST_ERR: Raised if the container of the start of
     *   the Range is of a type that does not allow children of the type of
     *   <code>newNode</code> or if <code>newNode</code> is an ancestor of
     *   the container.
     *   <br>INVALID_STATE_ERR: Raised if <code>detach()</code> has already
     *   been invoked on this object.
     * @exception DOMRangeException
     *   INVALID_NODE_TYPE_ERR: Raised if <code>newNode</code> is an DOMAttr,
     *   DOMEntity, DOMNotation, or DOMDocument node.
     *
     * @since DOM Level 2
     */
    virtual void insertNode(DOMNode *newNode) = 0;

    /**
     * Reparents the contents of the Range to the given node and inserts the
     * node at the position of the start of the Range.
     * @param newParent The node to surround the contents with.
     * @exception DOMException
     *   NO_MODIFICATION_ALLOWED_ERR: Raised if an ancestor container of
     *   either boundary-point of the Range is read-only.
     *   <br>WRONG_DOCUMENT_ERR: Raised if <code> newParent</code> and the
     *   container of the start of the Range were not created from the same
     *   document.
     *   <br>HIERARCHY_REQUEST_ERR: Raised if the container of the start of
     *   the Range is of a type that does not allow children of the type of
     *   <code>newParent</code> or if <code>newParent</code> is an ancestor
     *   of the container or if <code>node</code> would end up with a child
     *   node of a type not allowed by the type of <code>node</code>.
     *   <br>INVALID_STATE_ERR: Raised if <code>detach()</code> has already
     *   been invoked on this object.
     * @exception DOMRangeException
     *   BAD_BOUNDARYPOINTS_ERR: Raised if the Range partially selects a
     *   non-text node.
     *   <br>INVALID_NODE_TYPE_ERR: Raised if <code> node</code> is an DOMAttr,
     *   DOMEntity, DOMDocumentType, DOMNotation, DOMDocument, or DOMDocumentFragment node.
     *
     * @since DOM Level 2
     */
    virtual void surroundContents(DOMNode *newParent) = 0;

    /**
     * Produces a new Range whose boundary-points are equal to the
     * boundary-points of the Range.
     * @return The duplicated Range.
     * @exception DOMException
     *   INVALID_STATE_ERR: Raised if <code>detach()</code> has already been
     *   invoked on this object.
     *
     * @since DOM Level 2
     */
    virtual DOMRange* cloneRange() const = 0;

    /**
     * Returns the contents of a Range as a string. This string contains only
     * the data characters, not any markup.
     * @return The contents of the Range.
     * @exception DOMException
     *   INVALID_STATE_ERR: Raised if <code>detach()</code> has already been
     *   invoked on this object.
     *
     * @since DOM Level 2
     */
    virtual const XMLCh* toString() const = 0;

    /**
     * Called to indicate that the Range is no longer in use and that the
     * implementation may relinquish any resources associated with this
     * Range. Subsequent calls to any methods or attribute getters on this
     * Range will result in a <code>DOMException</code> being thrown with an
     * error code of <code>INVALID_STATE_ERR</code>.
     * @exception DOMException
     *   INVALID_STATE_ERR: Raised if <code>detach()</code> has already been
     *   invoked on this object.
     *
     * @since DOM Level 2
     */
    virtual void detach() = 0;

    //@}

    // -----------------------------------------------------------------------
    //  Non-standard Extension
    // -----------------------------------------------------------------------
    /** @name Non-standard Extension */
    //@{
    /**
     * Called to indicate that this Range is no longer in use
     * and that the implementation may relinquish any resources associated with it.
     * (release() will call detach() where appropriate)
     *
     * Access to a released object will lead to unexpected result.
     */
    virtual void release() = 0;
    //@}
};


XERCES_CPP_NAMESPACE_END

#endif
