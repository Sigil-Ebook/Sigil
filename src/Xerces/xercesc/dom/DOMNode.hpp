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
 * $Id: DOMNode.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMNODE_HPP)
#define XERCESC_INCLUDE_GUARD_DOMNODE_HPP

#include <xercesc/util/XercesDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN


class DOMDocument;
class DOMNamedNodeMap;
class DOMNodeList;
class DOMUserDataHandler;

/**
 * The <code>DOMNode</code> interface is the primary datatype for the entire
 * Document Object Model. It represents a single node in the document tree.
 * While all objects implementing the <code>DOMNode</code> interface expose
 * methods for dealing with children, not all objects implementing the
 * <code>DOMNode</code> interface may have children. For example,
 * <code>DOMText</code> nodes may not have children, and adding children to
 * such nodes results in a <code>DOMException</code> being raised.
 * <p>The attributes <code>nodeName</code>, <code>nodeValue</code> and
 * <code>attributes</code> are included as a mechanism to get at node
 * information without casting down to the specific derived interface. In
 * cases where there is no obvious mapping of these attributes for a
 * specific <code>nodeType</code> (e.g., <code>nodeValue</code> for an
 * <code>DOMElement</code> or <code>attributes</code> for a <code>DOMComment</code>
 * ), this returns <code>null</code>. Note that the specialized interfaces
 * may contain additional and more convenient mechanisms to get and set the
 * relevant information.
 * <p>The values of <code>nodeName</code>,
 * <code>nodeValue</code>, and <code>attributes</code> vary according to the
 * node type as follows:
 * <table border='1'>
 * <tr>
 * <td>Interface</td>
 * <td>nodeName</td>
 * <td>nodeValue</td>
 * <td>attributes</td>
 * </tr>
 * <tr>
 * <td valign='top' rowspan='1' colspan='1'>DOMAttr</td>
 * <td valign='top' rowspan='1' colspan='1'>name of attribute</td>
 * <td valign='top' rowspan='1' colspan='1'>value of attribute</td>
 * <td valign='top' rowspan='1' colspan='1'>null</td>
 * </tr>
 * <tr>
 * <td valign='top' rowspan='1' colspan='1'>DOMCDATASection</td>
 * <td valign='top' rowspan='1' colspan='1'>&quot;\#cdata-section&quot;</td>
 * <td valign='top' rowspan='1' colspan='1'>content of the CDATA Section</td>
 * <td valign='top' rowspan='1' colspan='1'>null</td>
 * </tr>
 * <tr>
 * <td valign='top' rowspan='1' colspan='1'>DOMComment</td>
 * <td valign='top' rowspan='1' colspan='1'>&quot;\#comment&quot;</td>
 * <td valign='top' rowspan='1' colspan='1'>content of the comment</td>
 * <td valign='top' rowspan='1' colspan='1'>null</td>
 * </tr>
 * <tr>
 * <td valign='top' rowspan='1' colspan='1'>DOMDocument</td>
 * <td valign='top' rowspan='1' colspan='1'>&quot;\#document&quot;</td>
 * <td valign='top' rowspan='1' colspan='1'>null</td>
 * <td valign='top' rowspan='1' colspan='1'>null</td>
 * </tr>
 * <tr>
 * <td valign='top' rowspan='1' colspan='1'>DOMDocumentFragment</td>
 * <td valign='top' rowspan='1' colspan='1'>&quot;\#document-fragment&quot;</td>
 * <td valign='top' rowspan='1' colspan='1'>null</td>
 * <td valign='top' rowspan='1' colspan='1'>null</td>
 * </tr>
 * <tr>
 * <td valign='top' rowspan='1' colspan='1'>DOMDocumentType</td>
 * <td valign='top' rowspan='1' colspan='1'>document type name</td>
 * <td valign='top' rowspan='1' colspan='1'>null</td>
 * <td valign='top' rowspan='1' colspan='1'>null</td>
 * </tr>
 * <tr>
 * <td valign='top' rowspan='1' colspan='1'>DOMElement</td>
 * <td valign='top' rowspan='1' colspan='1'>tag name</td>
 * <td valign='top' rowspan='1' colspan='1'>null</td>
 * <td valign='top' rowspan='1' colspan='1'>NamedNodeMap</td>
 * </tr>
 * <tr>
 * <td valign='top' rowspan='1' colspan='1'>DOMEntity</td>
 * <td valign='top' rowspan='1' colspan='1'>entity name</td>
 * <td valign='top' rowspan='1' colspan='1'>null</td>
 * <td valign='top' rowspan='1' colspan='1'>null</td>
 * </tr>
 * <tr>
 * <td valign='top' rowspan='1' colspan='1'>DOMEntityReference</td>
 * <td valign='top' rowspan='1' colspan='1'>name of entity referenced</td>
 * <td valign='top' rowspan='1' colspan='1'>null</td>
 * <td valign='top' rowspan='1' colspan='1'>null</td>
 * </tr>
 * <tr>
 * <td valign='top' rowspan='1' colspan='1'>DOMNotation</td>
 * <td valign='top' rowspan='1' colspan='1'>notation name</td>
 * <td valign='top' rowspan='1' colspan='1'>null</td>
 * <td valign='top' rowspan='1' colspan='1'>null</td>
 * </tr>
 * <tr>
 * <td valign='top' rowspan='1' colspan='1'>DOMProcessingInstruction</td>
 * <td valign='top' rowspan='1' colspan='1'>target</td>
 * <td valign='top' rowspan='1' colspan='1'>entire content excluding the target</td>
 * <td valign='top' rowspan='1' colspan='1'>null</td>
 * </tr>
 * <tr>
 * <td valign='top' rowspan='1' colspan='1'>DOMText</td>
 * <td valign='top' rowspan='1' colspan='1'>&quot;\#text&quot;</td>
 * <td valign='top' rowspan='1' colspan='1'>content of the text node</td>
 * <td valign='top' rowspan='1' colspan='1'>null</td>
 * </tr>
 * </table>
 * <p>See also the <a href='http://www.w3.org/TR/2000/REC-DOM-Level-2-Core-20001113'>Document Object Model (DOM) Level 2 Core Specification</a>.
 *
 * @since DOM Level 1
 */
class  CDOM_EXPORT DOMNode {
protected:
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    /** @name Hidden constructors */
    //@{
    DOMNode() {}
    DOMNode(const DOMNode &) {}
    //@}

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    /** @name Unimplemented operators */
    //@{
    DOMNode & operator = (const DOMNode &);
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
    virtual ~DOMNode() {};
    //@}

    // -----------------------------------------------------------------------
    //  Class Types
    // -----------------------------------------------------------------------
    /** @name Public Constants */
    //@{
    /**
     * NodeType
     *
     * @since DOM Level 1
     */
    enum NodeType {
        ELEMENT_NODE                = 1,
        ATTRIBUTE_NODE              = 2,
        TEXT_NODE                   = 3,
        CDATA_SECTION_NODE          = 4,
        ENTITY_REFERENCE_NODE       = 5,
        ENTITY_NODE                 = 6,
        PROCESSING_INSTRUCTION_NODE = 7,
        COMMENT_NODE                = 8,
        DOCUMENT_NODE               = 9,
        DOCUMENT_TYPE_NODE          = 10,
        DOCUMENT_FRAGMENT_NODE      = 11,
        NOTATION_NODE               = 12
    };

    /**
     * DocumentPosition:
     *
     * <p><code>DOCUMENT_POSITION_CONTAINED_BY:</code>
     * The node is contained by the reference node. A node which is contained is always following, too.</p>
     * <p><code>DOCUMENT_POSITION_CONTAINS:</code>
     * The node contains the reference node. A node which contains is always preceding, too.</p>
     * <p><code>DOCUMENT_POSITION_DISCONNECTED:</code>
     * The two nodes are disconnected. Order between disconnected nodes is always implementation-specific.</p>
     * <p><code>DOCUMENT_POSITION_FOLLOWING:</code>
     * The node follows the reference node.</p>
     * <p><code>DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC:</code>
     * The determination of preceding versus following is implementation-specific.</p>
     * <p><code>DOCUMENT_POSITION_PRECEDING:</code>
     * The second node precedes the reference node.</p>
     *
     * @since DOM Level 3
     */
    enum DocumentPosition {
        DOCUMENT_POSITION_DISCONNECTED            = 0x01,
        DOCUMENT_POSITION_PRECEDING               = 0x02,
        DOCUMENT_POSITION_FOLLOWING               = 0x04,
        DOCUMENT_POSITION_CONTAINS                = 0x08,
        DOCUMENT_POSITION_CONTAINED_BY            = 0x10,
        DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC = 0x20
    };
    //@}

    // -----------------------------------------------------------------------
    //  Virtual DOMNode interface
    // -----------------------------------------------------------------------
    /** @name Functions introduced in DOM Level 1 */
    //@{
    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    /**
     * The name of this node, depending on its type; see the table above.
     * @since DOM Level 1
     */
    virtual const XMLCh *   getNodeName() const = 0;

    /**
     * Gets the value of this node, depending on its type.
     *
     * @exception DOMException
     *   NO_MODIFICATION_ALLOWED_ERR: Raised when the node is readonly.
     * @since DOM Level 1
     */
    virtual const XMLCh *       getNodeValue() const = 0;

    /**
     * An enum value representing the type of the underlying object.
     * @since DOM Level 1
     */
    virtual NodeType            getNodeType() const = 0;

    /**
     * Gets the parent of this node.
     *
     * All nodes, except <code>DOMDocument</code>,
     * <code>DOMDocumentFragment</code>, and <code>DOMAttr</code> may have a parent.
     * However, if a node has just been created and not yet added to the tree,
     * or if it has been removed from the tree, a <code>null</code> DOMNode
     * is returned.
     * @since DOM Level 1
     */
    virtual DOMNode        *getParentNode() const = 0;

    /**
     * Gets a <code>DOMNodeList</code> that contains all children of this node.
     *
     * If there
     * are no children, this is a <code>DOMNodeList</code> containing no nodes.
     * The content of the returned <code>DOMNodeList</code> is "live" in the sense
     * that, for instance, changes to the children of the node object that
     * it was created from are immediately reflected in the nodes returned by
     * the <code>DOMNodeList</code> accessors; it is not a static snapshot of the
     * content of the node. This is true for every <code>DOMNodeList</code>,
     * including the ones returned by the <code>getElementsByTagName</code>
     * method.
     * @since DOM Level 1
     */
    virtual DOMNodeList    *getChildNodes() const = 0;
    /**
     * Gets the first child of this node.
     *
     * If there is no such node, this returns <code>null</code>.
     * @since DOM Level 1
     */
    virtual DOMNode        *getFirstChild() const = 0;

    /**
     * Gets the last child of this node.
     *
     * If there is no such node, this returns <code>null</code>.
     * @since DOM Level 1
     */
    virtual DOMNode        *getLastChild() const = 0;

    /**
     * Gets the node immediately preceding this node.
     *
     * If there is no such node, this returns <code>null</code>.
     * @since DOM Level 1
     */
    virtual DOMNode        *getPreviousSibling() const = 0;

    /**
     * Gets the node immediately following this node.
     *
     * If there is no such node, this returns <code>null</code>.
     * @since DOM Level 1
     */
    virtual DOMNode        *getNextSibling() const = 0;

    /**
     * Gets a <code>DOMNamedNodeMap</code> containing the attributes of this node (if it
     * is an <code>DOMElement</code>) or <code>null</code> otherwise.
     * @since DOM Level 1
     */
    virtual DOMNamedNodeMap  *getAttributes() const = 0;

    /**
     * Gets the <code>DOMDocument</code> object associated with this node.
     *
     * This is also
     * the <code>DOMDocument</code> object used to create new nodes. When this
     * node is a <code>DOMDocument</code> or a <code>DOMDocumentType</code>
     * which is not used with any <code>DOMDocument</code> yet, this is
     * <code>null</code>.
     *
     * @since DOM Level 1
     */
    virtual DOMDocument      *getOwnerDocument() const = 0;

    // -----------------------------------------------------------------------
    //  Node methods
    // -----------------------------------------------------------------------
    /**
     * Returns a duplicate of this node.
     *
     * This function serves as a generic copy constructor for nodes.
     *
     * The duplicate node has no parent (
     * <code>parentNode</code> returns <code>null</code>.).
     * <br>Cloning an <code>DOMElement</code> copies all attributes and their
     * values, including those generated by the  XML processor to represent
     * defaulted attributes, but this method does not copy any text it contains
     * unless it is a deep clone, since the text is contained in a child
     * <code>DOMText</code> node. Cloning any other type of node simply returns a
     * copy of this node.
     * @param deep If <code>true</code>, recursively clone the subtree under the
     *   specified node; if <code>false</code>, clone only the node itself (and
     *   its attributes, if it is an <code>DOMElement</code>).
     * @return The duplicate node.
     * @since DOM Level 1
     */
    virtual DOMNode        * cloneNode(bool deep) const = 0;

    /**
     * Inserts the node <code>newChild</code> before the existing child node
     * <code>refChild</code>.
     *
     * If <code>refChild</code> is <code>null</code>,
     * insert <code>newChild</code> at the end of the list of children.
     * <br>If <code>newChild</code> is a <code>DOMDocumentFragment</code> object,
     * all of its children are inserted, in the same order, before
     * <code>refChild</code>. If the <code>newChild</code> is already in the
     * tree, it is first removed.  Note that a <code>DOMNode</code> that
     * has never been assigned to refer to an actual node is == null.
     * @param newChild The node to insert.
     * @param refChild The reference node, i.e., the node before which the new
     *   node must be inserted.
     * @return The node being inserted.
     * @exception DOMException
     *   HIERARCHY_REQUEST_ERR: Raised if this node is of a type that does not
     *   allow children of the type of the <code>newChild</code> node, or if
     *   the node to insert is one of this node's ancestors.
     *   <br>WRONG_DOCUMENT_ERR: Raised if <code>newChild</code> was created
     *   from a different document than the one that created this node.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this node or the node being
     *   inserted is readonly.
     *   <br>NOT_FOUND_ERR: Raised if <code>refChild</code> is not a child of
     *   this node.
     * @since DOM Level 1
     */
    virtual DOMNode       *insertBefore(DOMNode *newChild,
                                          DOMNode *refChild) = 0;


    /**
     * Replaces the child node <code>oldChild</code> with <code>newChild</code>
     * in the list of children, and returns the <code>oldChild</code> node.
     *
     * If <CODE>newChild</CODE> is a <CODE>DOMDocumentFragment</CODE> object,
     * <CODE>oldChild</CODE> is replaced by all of the <CODE>DOMDocumentFragment</CODE>
     * children, which are inserted in the same order.
     *
     * If the <code>newChild</code> is already in the tree, it is first removed.
     * @param newChild The new node to put in the child list.
     * @param oldChild The node being replaced in the list.
     * @return The node replaced.
     * @exception DOMException
     *   HIERARCHY_REQUEST_ERR: Raised if this node is of a type that does not
     *   allow children of the type of the <code>newChild</code> node, or it
     *   the node to put in is one of this node's ancestors.
     *   <br>WRONG_DOCUMENT_ERR: Raised if <code>newChild</code> was created
     *   from a different document than the one that created this node.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this node or the new node is readonly.
     *   <br>NOT_FOUND_ERR: Raised if <code>oldChild</code> is not a child of
     *   this node.
     * @since DOM Level 1
     */
    virtual DOMNode  *replaceChild(DOMNode *newChild,
                                     DOMNode *oldChild) = 0;
    /**
     * Removes the child node indicated by <code>oldChild</code> from the list
     * of children, and returns it.
     *
     * @param oldChild The node being removed.
     * @return The node removed.
     * @exception DOMException
     *   NO_MODIFICATION_ALLOWED_ERR: Raised if this node is readonly.
     *   <br>NOT_FOUND_ERR: Raised if <code>oldChild</code> is not a child of
     *   this node.
     * @since DOM Level 1
     */
    virtual DOMNode        *removeChild(DOMNode *oldChild) = 0;

    /**
     * Adds the node <code>newChild</code> to the end of the list of children of
     * this node.
     *
     * If the <code>newChild</code> is already in the tree, it is
     * first removed.
     * @param newChild The node to add.If it is a  <code>DOMDocumentFragment</code>
     *   object, the entire contents of the document fragment are moved into
     *   the child list of this node
     * @return The node added.
     * @exception DOMException
     *   HIERARCHY_REQUEST_ERR: Raised if this node is of a type that does not
     *   allow children of the type of the <code>newChild</code> node, or if
     *   the node to append is one of this node's ancestors.
     *   <br>WRONG_DOCUMENT_ERR: Raised if <code>newChild</code> was created
     *   from a different document than the one that created this node.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this node or the node being
     *   appended is readonly.
     * @since DOM Level 1
     */
    virtual DOMNode        *appendChild(DOMNode *newChild) = 0;

    // -----------------------------------------------------------------------
    //  Query methods
    // -----------------------------------------------------------------------
    /**
     *  This is a convenience method to allow easy determination of whether a
     * node has any children.
     *
     * @return  <code>true</code> if the node has any children,
     *   <code>false</code> if the node has no children.
     * @since DOM Level 1
     */
    virtual bool             hasChildNodes() const = 0;

    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------
    /**
     * Sets the value of the node.
     *
     * Any node which can have a nodeValue will
     * also accept requests to set it to a string. The exact response to
     * this varies from node to node -- Attribute, for example, stores
     * its values in its children and has to replace them with a new Text
     * holding the replacement value.
     *
     * For most types of Node, value is null and attempting to set it
     * will throw DOMException(NO_MODIFICATION_ALLOWED_ERR). This will
     * also be thrown if the node is read-only.
     * @see #getNodeValue
     * @since DOM Level 1
     */
    virtual void              setNodeValue(const XMLCh  *nodeValue) = 0;
    //@}

    /** @name Functions introduced in DOM Level 2. */
    //@{
    /**
     * Puts all <CODE>DOMText</CODE>
     * nodes in the full depth of the sub-tree underneath this <CODE>DOMNode</CODE>,
     * including attribute nodes, into a "normal" form where only markup (e.g.,
     * tags, comments, processing instructions, CDATA sections, and entity
     * references) separates <CODE>DOMText</CODE>
     * nodes, i.e., there are neither adjacent <CODE>DOMText</CODE>
     * nodes nor empty <CODE>DOMText</CODE>
     * nodes. This can be used to ensure that the DOM view of a document is the
     * same as if it were saved and re-loaded, and is useful when operations
     * (such as XPointer lookups) that depend on a particular document tree
     * structure are to be used.
     * <P><B>Note:</B> In cases where the document contains <CODE>DOMCDATASections</CODE>,
     * the normalize operation alone may not be sufficient, since XPointers do
     * not differentiate between <CODE>DOMText</CODE>
     * nodes and <CODE>DOMCDATASection</CODE>
     * nodes.</P>
     *
     * @since DOM Level 2
     */
    virtual void              normalize() = 0;

    /**
     * Tests whether the DOM implementation implements a specific
     * feature and that feature is supported by this node.
     *
     * @param feature The string of the feature to test. This is the same
     * name as what can be passed to the method <code>hasFeature</code> on
     * <code>DOMImplementation</code>.
     * @param version This is the version number of the feature to test. In
     * Level 2, version 1, this is the string "2.0". If the version is not
     * specified, supporting any version of the feature will cause the
     * method to return <code>true</code>.
     * @return Returns <code>true</code> if the specified feature is supported
     * on this node, <code>false</code> otherwise.
     * @since DOM Level 2
     */
    virtual bool              isSupported(const XMLCh *feature,
	                                       const XMLCh *version) const = 0;

    /**
     * Get the <em>namespace URI</em> of
     * this node, or <code>null</code> if it is unspecified.
     * <p>
     * This is not a computed value that is the result of a namespace lookup
     * based on an examination of the namespace declarations in scope. It is
     * merely the namespace URI given at creation time.
     * <p>
     * For nodes of any type other than <CODE>ELEMENT_NODE</CODE> and
     * <CODE>ATTRIBUTE_NODE</CODE> and nodes created with a DOM Level 1 method,
     * such as <CODE>createElement</CODE> from the <CODE>DOMDocument</CODE>
     * interface, this is always <CODE>null</CODE>.
     *
     * @since DOM Level 2
     */
    virtual const XMLCh *         getNamespaceURI() const = 0;

    /**
     * Get the <em>namespace prefix</em>
     * of this node, or <code>null</code> if it is unspecified.
     *
     * @since DOM Level 2
     */
    virtual const XMLCh *          getPrefix() const = 0;

    /**
     * Returns the local part of the <em>qualified name</em> of this node.
     * <p>
     * For nodes created with a DOM Level 1 method, such as
     * <code>createElement</code> from the <code>DOMDocument</code> interface,
     * it is null.
     *
     * @since DOM Level 2
     */
    virtual const XMLCh *          getLocalName() const = 0;

    /**
     * Set the <em>namespace prefix</em> of this node.
     * <p>
     * Note that setting this attribute, when permitted, changes
     * the <CODE>nodeName</CODE> attribute, which holds the <EM>qualified
     * name</EM>, as well as the <CODE>tagName</CODE> and <CODE>name</CODE>
     * attributes of the <CODE>DOMElement</CODE> and <CODE>DOMAttr</CODE>
     * interfaces, when applicable.
     * <p>
     * Note also that changing the prefix of an
     * attribute, that is known to have a default value, does not make a new
     * attribute with the default value and the original prefix appear, since the
     * <CODE>namespaceURI</CODE> and <CODE>localName</CODE> do not change.
     *
     *
     * @param prefix The prefix of this node.
     * @exception DOMException
     *   INVALID_CHARACTER_ERR: Raised if the specified prefix contains
     *                          an illegal character.
     * <br>
     *   NO_MODIFICATION_ALLOWED_ERR: Raised if this node is readonly.
     * <br>
     *   NAMESPACE_ERR: Raised if the specified <CODE>prefix</CODE> is
     *      malformed, if the <CODE>namespaceURI</CODE> of this node is
     *      <CODE>null</CODE>, if the specified prefix is "xml" and the
     *      <CODE>namespaceURI</CODE> of this node is different from
     *      "http://www.w3.org/XML/1998/namespace", if this node is an attribute
     *      and the specified prefix is "xmlns" and the
     *      <CODE>namespaceURI</CODE> of this node is different from
     *      "http://www.w3.org/2000/xmlns/", or if this node is an attribute and
     *      the <CODE>qualifiedName</CODE> of this node is "xmlns".
     * @since DOM Level 2
     */
    virtual void              setPrefix(const XMLCh * prefix) = 0;

    /**
     *  Returns whether this node (if it is an element) has any attributes.
     * @return <code>true</code> if this node has any attributes,
     *   <code>false</code> otherwise.
     * @since DOM Level 2
     */
    virtual bool              hasAttributes() const = 0;
    //@}

    /** @name Functions introduced in DOM Level 3. */
    //@{
    /**
     * Returns whether this node is the same node as the given one.
     * <br>This method provides a way to determine whether two
     * <code>DOMNode</code> references returned by the implementation reference
     * the same object. When two <code>DOMNode</code> references are references
     * to the same object, even if through a proxy, the references may be
     * used completely interchangeably, such that all attributes have the
     * same values and calling the same DOM method on either reference
     * always has exactly the same effect.
     *
     * @param other The node to test against.
     * @return Returns <code>true</code> if the nodes are the same,
     *   <code>false</code> otherwise.
     * @since DOM Level 3
     */
    virtual bool              isSameNode(const DOMNode* other) const = 0;

    /**
     * Tests whether two nodes are equal.
     * <br>This method tests for equality of nodes, not sameness (i.e.,
     * whether the two nodes are pointers to the same object) which can be
     * tested with <code>DOMNode::isSameNode</code>. All nodes that are the same
     * will also be equal, though the reverse may not be true.
     * <br>Two nodes are equal if and only if the following conditions are
     * satisfied: The two nodes are of the same type.The following string
     * attributes are equal: <code>nodeName</code>, <code>localName</code>,
     * <code>namespaceURI</code>, <code>prefix</code>, <code>nodeValue</code>
     * , <code>baseURI</code>. This is: they are both <code>null</code>, or
     * they have the same length and are character for character identical.
     * The <code>attributes</code> <code>DOMNamedNodeMaps</code> are equal.
     * This is: they are both <code>null</code>, or they have the same
     * length and for each node that exists in one map there is a node that
     * exists in the other map and is equal, although not necessarily at the
     * same index.The <code>childNodes</code> <code>DOMNodeLists</code> are
     * equal. This is: they are both <code>null</code>, or they have the
     * same length and contain equal nodes at the same index. This is true
     * for <code>DOMAttr</code> nodes as for any other type of node. Note that
     * normalization can affect equality; to avoid this, nodes should be
     * normalized before being compared.
     * <br>For two <code>DOMDocumentType</code> nodes to be equal, the following
     * conditions must also be satisfied: The following string attributes
     * are equal: <code>publicId</code>, <code>systemId</code>,
     * <code>internalSubset</code>.The <code>entities</code>
     * <code>DOMNamedNodeMaps</code> are equal.The <code>notations</code>
     * <code>DOMNamedNodeMaps</code> are equal.
     * <br>On the other hand, the following do not affect equality: the
     * <code>ownerDocument</code> attribute, the <code>specified</code>
     * attribute for <code>DOMAttr</code> nodes, the
     * <code>isWhitespaceInElementContent</code> attribute for
     * <code>DOMText</code> nodes, as well as any user data or event listeners
     * registered on the nodes.
     *
     * @param arg The node to compare equality with.
     * @return If the nodes, and possibly subtrees are equal,
     *   <code>true</code> otherwise <code>false</code>.
     * @since DOM Level 3
     */
    virtual bool              isEqualNode(const DOMNode* arg) const = 0;


    /**
     * Associate an object to a key on this node. The object can later be
     * retrieved from this node by calling <code>getUserData</code> with the
     * same key.
     *
     * Deletion of the user data remains the responsibility of the
     * application program; it will not be automatically deleted when
     * the nodes themselves are reclaimed.
     *
     * Both the parameter <code>data</code> and the returned object are
     * void pointer, it is applications' responsibility to keep track of
     * their original type.  Casting them to the wrong type may result
     * unexpected behavior.
     *
     * @param key The key to associate the object to.
     * @param data The object to associate to the given key, or
     *   <code>null</code> to remove any existing association to that key.
     * @param handler The handler to associate to that key, or
     *   <code>null</code>.
     * @return Returns the void* object previously associated to
     *   the given key on this node, or <code>null</code> if there was none.
     * @see #getUserData
     *
     * @since DOM Level 3
     */
    virtual void*             setUserData(const XMLCh* key,
                                          void* data,
                                          DOMUserDataHandler* handler) = 0;

    /**
     * Retrieves the object associated to a key on a this node. The object
     * must first have been set to this node by calling
     * <code>setUserData</code> with the same key.
     *
     * @param key The key the object is associated to.
     * @return Returns the <code>void*</code> associated to the given key
     *   on this node, or <code>null</code> if there was none.
     * @see #setUserData
     * @since DOM Level 3
     */
    virtual void*             getUserData(const XMLCh* key) const = 0;


    /**
     * The absolute base URI of this node or <code>null</code> if undefined.
     * This value is computed according to . However, when the
     * <code>DOMDocument</code> supports the feature "HTML" , the base URI is
     * computed using first the value of the href attribute of the HTML BASE
     * element if any, and the value of the <code>documentURI</code>
     * attribute from the <code>DOMDocument</code> interface otherwise.
     *
     * <br> When the node is an <code>DOMElement</code>, a <code>DOMDocument</code>
     * or a a <code>DOMProcessingInstruction</code>, this attribute represents
     * the properties [base URI] defined in . When the node is a
     * <code>DOMNotation</code>, an <code>DOMEntity</code>, or an
     * <code>DOMEntityReference</code>, this attribute represents the
     * properties [declaration base URI].
     * @since DOM Level 3
     */
    virtual const XMLCh*      getBaseURI() const = 0;

    /**
     * Compares the reference node, i.e. the node on which this method is being called,
     * with a node, i.e. the one passed as a parameter, with regard to their position
     * in the document and according to the document order.
     *
     * @param other The node to compare against this node.
     * @return Returns how the given node is positioned relatively to this
     *   node.
     * @since DOM Level 3
     */
    virtual short             compareDocumentPosition(const DOMNode* other) const = 0;

    /**
     * This attribute returns the text content of this node and its
     * descendants. No serialization is performed, the returned string
     * does not contain any markup. No whitespace normalization is
     * performed and the returned string does not contain the white
     * spaces in element content.
     *
     * <br>The string returned is made of the text content of this node
     * depending on its type, as defined below:
     * <table border='1'>
     * <tr>
     * <td>Node type</td>
     * <td>Content</td>
     * </tr>
     * <tr>
     * <td valign='top' rowspan='1' colspan='1'>
     * ELEMENT_NODE, ENTITY_NODE, ENTITY_REFERENCE_NODE,
     * DOCUMENT_FRAGMENT_NODE</td>
     * <td valign='top' rowspan='1' colspan='1'>concatenation of the <code>textContent</code>
     * attribute value of every child node, excluding COMMENT_NODE and
     * PROCESSING_INSTRUCTION_NODE nodes</td>
     * </tr>
     * <tr>
     * <td valign='top' rowspan='1' colspan='1'>ATTRIBUTE_NODE, TEXT_NODE,
     * CDATA_SECTION_NODE, COMMENT_NODE, PROCESSING_INSTRUCTION_NODE</td>
     * <td valign='top' rowspan='1' colspan='1'>
     * <code>nodeValue</code></td>
     * </tr>
     * <tr>
     * <td valign='top' rowspan='1' colspan='1'>DOCUMENT_NODE, DOCUMENT_TYPE_NODE, NOTATION_NODE</td>
     * <td valign='top' rowspan='1' colspan='1'>
     * null</td>
     * </tr>
     * </table>
     * @exception DOMException
     *   DOMSTRING_SIZE_ERR: Raised when it would return more characters than
     *   fit in a <code>DOMString</code> variable on the implementation
     *   platform.
     * @see #setTextContent
     * @since DOM Level 3
     */
    virtual const XMLCh*      getTextContent() const = 0;

    /**
     * This attribute removes any possible children this node may have and, if the
     * new string is not empty or null, replaced by a single <code>DOMText</code>
     * node containing the string this attribute is set to. No parsing is
     * performed, the input string is taken as pure textual content.
     *
     * @exception DOMException
     *   NO_MODIFICATION_ALLOWED_ERR: Raised when the node is readonly.
     * @see #getTextContent
     * @since DOM Level 3
     */
    virtual void              setTextContent(const XMLCh* textContent) = 0;

    /**
     * Look up the prefix associated to the given namespace URI, starting from this node.
     * The default namespace declarations are ignored by this method.
     *
     * @param namespaceURI The namespace URI to look for.
     * @return Returns an associated namespace prefix if found,
     *   <code>null</code> if none is found. If more
     *   than one prefix are associated to the namespace prefix, the
     *   returned namespace prefix is implementation dependent.
     * @since DOM Level 3
     */
    virtual const XMLCh*      lookupPrefix(const XMLCh* namespaceURI) const = 0;

    /**
     * This method checks if the specified <code>namespaceURI</code> is the
     * default namespace or not.
     *
     * @param namespaceURI The namespace URI to look for.
     * @return  <code>true</code> if the specified <code>namespaceURI</code>
     *   is the default namespace, <code>false</code> otherwise.
     * @since DOM Level 3
     */
    virtual bool              isDefaultNamespace(const XMLCh* namespaceURI) const = 0;

    /**
     * Look up the namespace URI associated to the given prefix, starting from
     * this node.
     *
     * @param prefix The prefix to look for. If this parameter is
     *   <code>null</code>, the method will return the default namespace URI
     *   if any.
     * @return Returns the associated namespace URI or <code>null</code> if
     *   none is found.
     * @since DOM Level 3
     */
    virtual const XMLCh*      lookupNamespaceURI(const XMLCh* prefix) const  = 0;

    /**
     * This method makes available a <code>DOMNode</code>'s specialized interface
     *
     * @param feature The name of the feature requested (case-insensitive).
     * @param version The version of the feature requested.
     * @return Returns an alternate <code>DOMNode</code> which implements the
     *   specialized APIs of the specified feature, if any, or
     *   <code>null</code> if there is no alternate <code>DOMNode</code> which
     *   implements interfaces associated with that feature. Any alternate
     *   <code>DOMNode</code> returned by this method must delegate to the
     *   primary core <code>DOMNode</code> and not return results inconsistent
     *   with the primary core <code>DOMNode</code> such as <code>key</code>,
     *   <code>attributes</code>, <code>childNodes</code>, etc.
     * @since DOM Level 3
     */
    virtual void*             getFeature(const XMLCh* feature, const XMLCh* version) const = 0;
    //@}

    // -----------------------------------------------------------------------
    //  Non-standard Extension
    // -----------------------------------------------------------------------
    /** @name Non-standard Extension */
    //@{
    /**
     * Called to indicate that this Node (and its associated children) is no longer in use
     * and that the implementation may relinquish any resources associated with it and
     * its associated children.
     *
     * If this is a document, any nodes it owns (created by DOMDocument::createXXXX())
     * are also released.
     *
     * Access to a released object will lead to unexpected result.
     *
     * @exception DOMException
     *   INVALID_ACCESS_ERR: Raised if this Node has a parent and thus should not be released yet.
     */
    virtual void              release() = 0;
    //@}
#if defined(XML_DOMREFCOUNT_EXPERIMENTAL)
    // -----------------------------------------------------------------------
    //  Non-standard Extension
    // -----------------------------------------------------------------------
    /** @name Non-standard Extension */
    //@{
    /**
	 * This is custom function which can be implemented by classes deriving
	 * from DOMNode for implementing reference counting on DOMNodes. Any
	 * implementation which has memory management model which involves
	 * disposing of nodes immediately after being used can override this
	 * function to do that job.
     */
    virtual void decRefCount() {}
    //@}

    // -----------------------------------------------------------------------
    //  Non-standard Extension
    // -----------------------------------------------------------------------
    /** @name Non-standard Extension */
    //@{
    /**
	 * This is custom function which can be implemented by classes deriving
	 * from DOMNode for implementing reference counting on DOMNodes.
     */
    virtual void incRefCount() {}
    //@}
#endif
};

/***
 * Utilities macros for getting memory manager within DOM
***/
#define GET_OWNER_DOCUMENT(ptr)      \
        ((DOMDocumentImpl*)(ptr->getOwnerDocument()))

#define GET_DIRECT_MM(ptr)           \
        (ptr ? ((DOMDocumentImpl*)ptr)->getMemoryManager() : XMLPlatformUtils::fgMemoryManager)

#define GET_INDIRECT_MM(ptr)                                                    \
        (!ptr ? XMLPlatformUtils::fgMemoryManager :                              \
        GET_OWNER_DOCUMENT(ptr) ? GET_OWNER_DOCUMENT(ptr)->getMemoryManager() : \
        XMLPlatformUtils::fgMemoryManager)

/***
 * For DOMNode and its derivatives
***/
#define GetDOMNodeMemoryManager GET_INDIRECT_MM(this)

XERCES_CPP_NAMESPACE_END

#endif
