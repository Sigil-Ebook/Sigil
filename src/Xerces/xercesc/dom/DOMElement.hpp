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
 * $Id: DOMElement.hpp 792236 2009-07-08 17:22:35Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMELEMENT_HPP)
#define XERCESC_INCLUDE_GUARD_DOMELEMENT_HPP

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/dom/DOMNode.hpp>

XERCES_CPP_NAMESPACE_BEGIN


class DOMAttr;
class DOMNodeList;
class DOMTypeInfo;


/**
 * By far the vast majority of objects (apart from text) that authors
 * encounter when traversing a document are <code>DOMElement</code> nodes.
 *
 * Assume the following XML document:&lt;elementExample id="demo"&gt;
 * &lt;subelement1/&gt;
 * &lt;subelement2&gt;&lt;subsubelement/&gt;&lt;/subelement2&gt;
 * &lt;/elementExample&gt;
 * <p>When represented using DOM, the top node is an <code>DOMElement</code> node
 * for "elementExample", which contains two child <code>DOMElement</code> nodes,
 * one for "subelement1" and one for "subelement2". "subelement1" contains no
 * child nodes.
 * <p>Elements may have attributes associated with them; since the
 * <code>DOMElement</code> interface inherits from <code>DOMNode</code>, the generic
 *  <code>DOMNode</code> interface method <code>getAttributes</code> may be used
 * to retrieve the set of all attributes for an element.  There are methods on
 *  the <code>DOMElement</code> interface to retrieve either an <code>DOMAttr</code>
 *  object by name or an attribute value by name. In XML, where an attribute
 * value may contain entity references, an <code>DOMAttr</code> object should be
 * retrieved to examine the possibly fairly complex sub-tree representing the
 * attribute value. On the other hand, in HTML, where all attributes have
 * simple string values, methods to directly access an attribute value can
 * safely be used as a convenience.
 *
 * @since DOM Level 1
 *
 * It also defines the ElementTraversal helper interface defined by http://www.w3.org/TR/2008/REC-ElementTraversal-20081222/
 *
 */

class CDOM_EXPORT DOMElement: public DOMNode {
protected:
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    /** @name Hidden constructors */
    //@{    
    DOMElement() {}
    DOMElement(const DOMElement &other) : DOMNode(other) {}
    //@}
    
private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    /** @name Unimplemented operators */
    //@{
    DOMElement & operator = (const DOMElement &);
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
    virtual ~DOMElement() {};
    //@}

    // -----------------------------------------------------------------------
    //  Virtual DOMElement interface
    // -----------------------------------------------------------------------
    /** @name Functions introduced in DOM Level 1 */
    //@{
    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    /**
     * The name of the element.
     *
     * For example, in: &lt;elementExample
     * id="demo"&gt;  ... &lt;/elementExample&gt; , <code>tagName</code> has
     * the value <code>"elementExample"</code>. Note that this is
     * case-preserving in XML, as are all of the operations of the DOM.
     * @since DOM Level 1
     */
    virtual const XMLCh *         getTagName() const = 0;

    /**
     * Retrieves an attribute value by name.
     *
     * @param name The name of the attribute to retrieve.
     * @return The <code>DOMAttr</code> value as a string, or the empty  string if
     *   that attribute does not have a specified or default value.
     * @since DOM Level 1
     */
    virtual const XMLCh *         getAttribute(const XMLCh *name) const = 0;

    /**
     * Retrieves an <code>DOMAttr</code> node by name.
     *
     * @param name The name (<CODE>nodeName</CODE>) of the attribute to retrieve.
     * @return The <code>DOMAttr</code> node with the specified name (<CODE>nodeName</CODE>) or
     *   <code>null</code> if there is no such attribute.
     * @since DOM Level 1
     */
    virtual DOMAttr       * getAttributeNode(const XMLCh *name) const = 0;

    /**
     * Returns a <code>DOMNodeList</code> of all descendant elements with a given
     * tag name, in the order in which they would be encountered in a preorder
     * traversal of the <code>DOMElement</code> tree.
     *
     * @param name The name of the tag to match on. The special value "*"
     *   matches all tags.
     * @return A list of matching <code>DOMElement</code> nodes.
     * @since DOM Level 1
     */
    virtual DOMNodeList   * getElementsByTagName(const XMLCh *name) const = 0;

    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------
    /**
     * Adds a new attribute.
     *
     * If an attribute with that name is already present
     * in the element, its value is changed to be that of the value parameter.
     * This value is a simple string, it is not parsed as it is being set. So
     * any markup (such as syntax to be recognized as an entity reference) is
     * treated as literal text, and needs to be appropriately escaped by the
     * implementation when it is written out. In order to assign an attribute
     * value that contains entity references, the user must create an
     * <code>DOMAttr</code> node plus any <code>DOMText</code> and
     * <code>DOMEntityReference</code> nodes, build the appropriate subtree, and
     * use <code>setAttributeNode</code> to assign it as the value of an
     * attribute.
     * @param name The name of the attribute to create or alter.
     * @param value Value to set in string form.
     * @exception DOMException
     *   INVALID_CHARACTER_ERR: Raised if the specified name contains an
     *   illegal character.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this node is readonly.
     * @since DOM Level 1
     */
    virtual void             setAttribute(const XMLCh *name,
                                  const XMLCh *value) = 0;
    /**
     * Adds a new attribute.
     *
     * If an attribute with that name (<CODE>nodeName</CODE>) is already present
     * in the element, it is replaced by the new one.
     * @param newAttr The <code>DOMAttr</code> node to add to the attribute list.
     * @return If the <code>newAttr</code> attribute replaces an existing
     *   attribute, the replaced
     *   <code>DOMAttr</code> node is returned, otherwise <code>null</code> is
     *   returned.
     * @exception DOMException
     *   WRONG_DOCUMENT_ERR: Raised if <code>newAttr</code> was created from a
     *   different document than the one that created the element.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this node is readonly.
     *   <br>INUSE_ATTRIBUTE_ERR: Raised if <code>newAttr</code> is already an
     *   attribute of another <code>DOMElement</code> object. The DOM user must
     *   explicitly clone <code>DOMAttr</code> nodes to re-use them in other
     *   elements.
     * @since DOM Level 1
     */
    virtual DOMAttr       * setAttributeNode(DOMAttr *newAttr) = 0;

    /**
     * Removes the specified attribute node.
     * If the removed <CODE>DOMAttr</CODE>
     *   has a default value it is immediately replaced. The replacing attribute
     *   has the same namespace URI and local name, as well as the original prefix,
     *   when applicable.
     *
     * @param oldAttr The <code>DOMAttr</code> node to remove from the attribute
     *   list.
     * @return The <code>DOMAttr</code> node that was removed.
     * @exception DOMException
     *   NO_MODIFICATION_ALLOWED_ERR: Raised if this node is readonly.
     *   <br>NOT_FOUND_ERR: Raised if <code>oldAttr</code> is not an attribute
     *   of the element.
     * @since DOM Level 1
     */
    virtual DOMAttr       * removeAttributeNode(DOMAttr *oldAttr) = 0;

    /**
     * Removes an attribute by name.
     *
     * If the removed attribute
     *   is known to have a default value, an attribute immediately appears
     *   containing the default value as well as the corresponding namespace URI,
     *   local name, and prefix when applicable.<BR>To remove an attribute by local
     *   name and namespace URI, use the <CODE>removeAttributeNS</CODE> method.
     * @param name The name of the attribute to remove.
     * @exception DOMException
     *   NO_MODIFICATION_ALLOWED_ERR: Raised if this node is readonly.
     * @since DOM Level 1
     */
    virtual void              removeAttribute(const XMLCh *name) = 0;
    //@}

    /** @name Functions introduced in DOM Level 2. */
    //@{
    /**
     * Retrieves an attribute value by local name and namespace URI.
     *
     * @param namespaceURI The <em>namespace URI</em> of
     *    the attribute to retrieve.
     * @param localName The <em>local name</em> of the
     *    attribute to retrieve.
     * @return The <code>DOMAttr</code> value as a string, or an <CODE>null</CODE> if
     *    that attribute does not have a specified or default value.
     * @since DOM Level 2
     */
    virtual const XMLCh *         getAttributeNS(const XMLCh *namespaceURI,
                                                 const XMLCh *localName) const = 0;

    /**
     * Adds a new attribute. If an attribute with the same
     * local name and namespace URI is already present on the element, its prefix
     * is changed to be the prefix part of the <CODE>qualifiedName</CODE>, and
     * its value is changed to be the <CODE>value</CODE> parameter. This value is
     * a simple string, it is not parsed as it is being set. So any markup (such
     * as syntax to be recognized as an entity reference) is treated as literal
     * text, and needs to be appropriately escaped by the implementation when it
     * is written out. In order to assign an attribute value that contains entity
     * references, the user must create an <CODE>DOMAttr</CODE>
     * node plus any <CODE>DOMText</CODE> and <CODE>DOMEntityReference</CODE>
     * nodes, build the appropriate subtree, and use
     * <CODE>setAttributeNodeNS</CODE> or <CODE>setAttributeNode</CODE> to assign
     * it as the value of an attribute.
     *
     * @param namespaceURI The <em>namespace URI</em> of
     *    the attribute to create or alter.
     * @param qualifiedName The <em>qualified name</em> of the
     *    attribute to create or alter.
     * @param value The value to set in string form.
     * @exception DOMException
     *   INVALID_CHARACTER_ERR: Raised if the specified qualified name contains an
     *   illegal character.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this node is readonly.
     * <br>
     *   NAMESPACE_ERR: Raised if the <CODE>qualifiedName</CODE> is
     *        malformed, if the <CODE>qualifiedName</CODE> has a prefix and the
     *        <CODE>namespaceURI</CODE> is <CODE>null</CODE> or an empty string,
     *        if the <CODE>qualifiedName</CODE> has a prefix that is "xml" and the
     *        <CODE>namespaceURI</CODE> is different from
     *        "http://www.w3.org/XML/1998/namespace", if the
     *        <CODE>qualifiedName</CODE> has a prefix that is "xmlns" and the
     *        <CODE>namespaceURI</CODE> is different from
     *        "http://www.w3.org/2000/xmlns/", or if the
     *        <CODE>qualifiedName</CODE> is "xmlns" and the
     *        <CODE>namespaceURI</CODE> is different from
     *        "http://www.w3.org/2000/xmlns/".
     * @since DOM Level 2
     */
    virtual void             setAttributeNS(const XMLCh *namespaceURI,
                                            const XMLCh *qualifiedName, const XMLCh *value) = 0;

    /**
     * Removes an attribute by local name and namespace URI. If the
     * removed attribute has a default value it is immediately replaced.
     * The replacing attribute has the same namespace URI and local name, as well as
     * the original prefix.
     *
     * @param namespaceURI The <em>namespace URI</em> of
     *    the attribute to remove.
     * @param localName The <em>local name</em> of the
     *    attribute to remove.
     * @exception DOMException
     *   NO_MODIFICATION_ALLOWED_ERR: Raised if this node is readonly.
     * @since DOM Level 2
     */
    virtual void              removeAttributeNS(const XMLCh *namespaceURI,
                                                const XMLCh *localName) = 0;

    /**
     * Retrieves an <code>DOMAttr</code> node by local name and namespace URI.
     *
     * @param namespaceURI The <em>namespace URI</em> of
     *    the attribute to retrieve.
     * @param localName The <em>local name</em> of the
     *    attribute to retrieve.
     * @return The <code>DOMAttr</code> node with the specified attribute local
     *    name and namespace URI or <code>null</code> if there is no such attribute.
     * @since DOM Level 2
     */
    virtual DOMAttr      *  getAttributeNodeNS(const XMLCh *namespaceURI,
                                               const XMLCh *localName) const = 0;

    /**
     * Adds a new attribute.
     *
     * If an attribute with that local name and namespace URI is already present
     * in the element, it is replaced by the new one.
     *
     * @param newAttr The <code>DOMAttr</code> node to add to the attribute list.
     * @return If the <code>newAttr</code> attribute replaces an existing
     *    attribute with the same <em>local name</em> and <em>namespace URI</em>,
     *    the replaced <code>DOMAttr</code> node is
     *    returned, otherwise <code>null</code> is returned.
     * @exception DOMException
     *   WRONG_DOCUMENT_ERR: Raised if <code>newAttr</code> was created from a
     *   different document than the one that created the element.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this node is readonly.
     *   <br>INUSE_ATTRIBUTE_ERR: Raised if <code>newAttr</code> is already an
     *   attribute of another <code>DOMElement</code> object. The DOM user must
     *   explicitly clone <code>DOMAttr</code> nodes to re-use them in other
     *   elements.
     * @since DOM Level 2
     */
    virtual DOMAttr      *  setAttributeNodeNS(DOMAttr *newAttr) = 0;

    /**
     * Returns a <code>DOMNodeList</code> of all the <code>DOMElement</code>s
     * with a given local name and namespace URI in the order in which they
     * would be encountered in a preorder traversal of the
     * <code>DOMDocument</code> tree, starting from this node.
     *
     * @param namespaceURI The <em>namespace URI</em> of
     *    the elements to match on. The special value "*" matches all
     *    namespaces.
     * @param localName The <em>local name</em> of the
     *    elements to match on. The special value "*" matches all local names.
     * @return A new <code>DOMNodeList</code> object containing all the matched
     *    <code>DOMElement</code>s.
     * @since DOM Level 2
     */
    virtual DOMNodeList   * getElementsByTagNameNS(const XMLCh *namespaceURI,
                                                   const XMLCh *localName) const = 0;

    /**
     * Returns <code>true</code> when an attribute with a given name is
     * specified on this element or has a default value, <code>false</code>
     * otherwise.
     * @param name The name of the attribute to look for.
     * @return <code>true</code> if an attribute with the given name is
     *   specified on this element or has a default value, <code>false</code>
     *    otherwise.
     * @since DOM Level 2
     */
    virtual bool         hasAttribute(const XMLCh *name) const = 0;

    /**
     * Returns <code>true</code> when an attribute with a given local name and
     * namespace URI is specified on this element or has a default value,
     * <code>false</code> otherwise. HTML-only DOM implementations do not
     * need to implement this method.
     * @param namespaceURI The namespace URI of the attribute to look for.
     * @param localName The local name of the attribute to look for.
     * @return <code>true</code> if an attribute with the given local name
     *   and namespace URI is specified or has a default value on this
     *   element, <code>false</code> otherwise.
     * @since DOM Level 2
     */
    virtual bool         hasAttributeNS(const XMLCh *namespaceURI,
                                        const XMLCh *localName) const = 0;
    //@}

    /** @name Functions introduced in DOM Level 3 */
    //@{

    /**
     * If the parameter isId is <code>true</code>, this method declares the specified 
     * attribute to be a user-determined ID attribute. 
     * This affects the value of <code>DOMAttr::isId</code> and the behavior of 
     * <code>DOMDocument::getElementById</code>, but does not change any schema that 
     * may be in use, in particular this does not affect the <code>DOMAttr::getSchemaTypeInfo</code>
     * of the specified DOMAttr node. Use the value <code>false</code> for the parameter isId 
     * to undeclare an attribute for being a user-determined ID attribute.
     * To specify an <code>DOMAttr</code> by local name and namespace URI, use the
     * setIdAttributeNS method.
     *
     * @param name The name of the <code>DOMAttr</code>.
     * @param isId Whether the attribute is of type ID.
     * @exception DOMException
     *    NO_MODIFICATION_ALLOWED_ERR: Raised if this node is readonly.<br>
     *    NOT_FOUND_ERR: Raised if the specified node is not an <code>DOMAttr</code>
     * of this element.
     *
     * @since DOM Level 3
     */
    virtual void setIdAttribute(const XMLCh* name, bool isId) = 0;


    /**
     * If the parameter isId is <code>true</code>, this method declares the specified 
     * attribute to be a user-determined ID attribute. 
     * This affects the value of <code>DOMAttr::isId</code> and the behavior of 
     * <code>DOMDocument::getElementById</code>, but does not change any schema that 
     * may be in use, in particular this does not affect the <code>DOMAttr::getSchemaTypeInfo</code>
     * of the specified DOMAttr node. Use the value <code>false</code> for the parameter isId 
     * to undeclare an attribute for being a user-determined ID attribute.
     *
     * @param namespaceURI The namespace URI of the <code>DOMAttr</code>.
     * @param localName The local name of the <code>DOMAttr</code>.
     * @param isId Whether the attribute is of type ID.
     * @exception  DOMException
     *   NO_MODIFICATION_ALLOWED_ERR: Raised if this node is readonly.<br>
     *   NOT_FOUND_ERR: Raised if the specified node is not an <code>DOMAttr</code> of this element.
     *
     * @since DOM Level 3
     */
    virtual void setIdAttributeNS(const XMLCh* namespaceURI, const XMLCh* localName, bool isId) = 0;



    /**
     * If the parameter isId is <code>true</code>, this method declares the specified 
     * attribute to be a user-determined ID attribute. 
     * This affects the value of <code>DOMAttr::isId</code> and the behavior of 
     * <code>DOMDocument::getElementById</code>, but does not change any schema that 
     * may be in use, in particular this does not affect the <code>DOMAttr::getSchemaTypeInfo</code>
     * of the specified DOMAttr node. Use the value <code>false</code> for the parameter isId 
     * to undeclare an attribute for being a user-determined ID attribute.
     *
     * @param idAttr The <code>DOMAttr</code> node.
     * @param isId Whether the attribute is of type ID.
     * @exception  DOMException
     *   NO_MODIFICATION_ALLOWED_ERR: Raised if this node is readonly.<br>
     *   NOT_FOUND_ERR: Raised if the specified node is not an <code>DOMAttr</code> of this element.
     *
     * @since DOM Level 3
     */
    virtual void setIdAttributeNode(const DOMAttr *idAttr, bool isId) = 0;



    /**
     * Returns the type information associated with this element.
     *
     * @return the <code>DOMTypeInfo</code> associated with this element
     * @since DOM level 3
     */
    virtual const DOMTypeInfo* getSchemaTypeInfo() const = 0;

    //@}

    // -----------------------------------------------------------------------
    //  DOMElementTraversal interface
    // -----------------------------------------------------------------------
    /** @name Functions introduced in the ElementTraversal specification (http://www.w3.org/TR/2008/REC-ElementTraversal-20081222/)*/
    //@{
    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    /**
     * The first child of type DOMElement.
     *
     * @return The <code>DOMElement</code> object that is the first element node
     *   among the child nodes of this node, or <code>null</code> if there is none.
     */
    virtual DOMElement *         getFirstElementChild() const = 0;

    /**
     * The last child of type DOMElement.
     *
     * @return The <code>DOMElement</code> object that is the last element node
     *   among the child nodes of this node, or <code>null</code> if there is none.
     */
    virtual DOMElement *         getLastElementChild() const = 0;

    /**
     * The previous sibling node of type DOMElement.
     *
     * @return The <code>DOMElement</code> object that is the previous sibling element node
     *   in document order, or <code>null</code> if there is none.
     */
    virtual DOMElement *         getPreviousElementSibling() const = 0;

    /**
     * The next sibling node of type DOMElement.
     *
     * @return The <code>DOMElement</code> object that is the next sibling element node
     *   in document order, or <code>null</code> if there is none.
     */
    virtual DOMElement *         getNextElementSibling() const = 0;

    /**
     * The number of child nodes that are of type DOMElement.
     *
     * Note: the count is computed every time this function is invoked
     *
     * @return The number of <code>DOMElement</code> objects that are direct children
     *   of this object (nested elements are not counted), or <code>0</code> if there is none.
     * 
     */
    virtual XMLSize_t            getChildElementCount() const = 0;
    //@}
};

XERCES_CPP_NAMESPACE_END

#endif



