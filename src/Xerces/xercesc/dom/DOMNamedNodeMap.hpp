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
 * $Id: DOMNamedNodeMap.hpp 671894 2008-06-26 13:29:21Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMNAMEDNODEMAP_HPP)
#define XERCESC_INCLUDE_GUARD_DOMNAMEDNODEMAP_HPP

#include <xercesc/util/XercesDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN


class DOMNode;

/**
 *  <code>DOMNamedNodeMap</code>s  are used to
 * represent collections of nodes that can be accessed by name.
 *
 * Note that <code>DOMNamedNodeMap</code> does not inherit from <code>DOMNodeList</code>;
 * <code>DOMNamedNodeMap</code>s are not maintained in any particular order.
 * Nodes contained in a <code>DOMNamedNodeMap</code> may
 * also be accessed by an ordinal index, but this is simply to allow
 * convenient enumeration of the contents, and
 * does not imply that the DOM specifies an order to these Nodes.
 *
 * @since DOM Level 1
 */
class CDOM_EXPORT DOMNamedNodeMap {
protected:
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    /** @name Hidden constructors */
    //@{
    DOMNamedNodeMap() {};
    //@}

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    /** @name Unimplemented constructors and operators */
    //@{
    DOMNamedNodeMap(const DOMNamedNodeMap &);
    DOMNamedNodeMap & operator = (const DOMNamedNodeMap &);
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
    virtual ~DOMNamedNodeMap() {};
    //@}

    // -----------------------------------------------------------------------
    //  Virtual DOMNamedNodeMap interface
    // -----------------------------------------------------------------------
    /** @name Functions introduced in DOM Level 1 */
    //@{
    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------
    /**
     * Adds a node using its <code>nodeName</code> attribute.
     *
     * <br>As the <code>nodeName</code> attribute is used to derive the name
     * which the node must be stored under, multiple nodes of certain types
     * (those that have a "special" string value) cannot be stored as the names
     * would clash. This is seen as preferable to allowing nodes to be aliased.
     * @param arg A node to store in a named node map. The node will later be
     *   accessible using the value of the <code>nodeName</code> attribute of
     *   the node. If a node with that name is already present in the map, it
     *   is replaced by the new one.
     * @return If the new <code>DOMNode</code> replaces an existing node the
     *   replaced <code>DOMNode</code> is returned,
     *   otherwise <code>null</code> is returned.
     * @exception DOMException
     *   WRONG_DOCUMENT_ERR: Raised if <code>arg</code> was created from a
     *   different document than the one that created the
     *   <code>DOMNamedNodeMap</code>.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this
     *   <code>DOMNamedNodeMap</code> is readonly.
     *   <br>INUSE_ATTRIBUTE_ERR: Raised if <code>arg</code> is an
     *   <code>DOMAttr</code> that is already an attribute of another
     *   <code>DOMElement</code> object. The DOM user must explicitly clone
     *   <code>DOMAttr</code> nodes to re-use them in other elements.
     * @since DOM Level 1
     */
    virtual DOMNode   *setNamedItem(DOMNode *arg) = 0;

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    /**
     * Returns the <code>index</code>th item in the map.
     *
     * If <code>index</code>
     * is greater than or equal to the number of nodes in the map, this returns
     * <code>null</code>.
     * @param index Index into the map.
     * @return The node at the <code>index</code>th position in the
     *   <code>DOMNamedNodeMap</code>, or <code>null</code> if that is not a valid
     *   index.
     * @since DOM Level 1
     */
    virtual DOMNode     *item(XMLSize_t index) const = 0;

    /**
     * Retrieves a node specified by name.
     *
     * @param name The <code>nodeName</code> of a node to retrieve.
     * @return A <code>DOMNode</code> (of any type) with the specified <code>nodeName</code>, or
     *   <code>null</code> if it does not identify any node in
     *   the map.
     * @since DOM Level 1
     */
    virtual DOMNode   *getNamedItem(const XMLCh *name) const = 0;

    /**
     * The number of nodes in the map.
     *
     * The range of valid child node indices is
     * 0 to <code>length-1</code> inclusive.
     * @since DOM Level 1
     */
    virtual XMLSize_t getLength() const = 0;

    // -----------------------------------------------------------------------
    //  Node methods
    // -----------------------------------------------------------------------
    /**
     * Removes a node specified by name.
     *
     * If the removed node is an
     * <code>DOMAttr</code> with a default value it is immediately replaced.
     * @param name The <code>nodeName</code> of a node to remove.
     * @return The node removed from the map if a node with such a name exists.
     * @exception DOMException
     *   NOT_FOUND_ERR: Raised if there is no node named <code>name</code> in
     *   the map.
     * <br>
     *   NO_MODIFICATION_ALLOWED_ERR: Raised if this <code>DOMNamedNodeMap</code>
     *   is readonly.
     * @since DOM Level 1
     */
    virtual DOMNode    *removeNamedItem(const XMLCh *name) = 0;
    //@}

    /** @name Functions introduced in DOM Level 2 */
    //@{
    /**
     * Retrieves a node specified by local name and namespace URI.
     *
     * @param namespaceURI The <em>namespace URI</em> of
     *    the node to retrieve.
     * @param localName The <em>local name</em> of the node to retrieve.
     * @return A <code>DOMNode</code> (of any type) with the specified
     *    local name and namespace URI, or <code>null</code> if they do not
     *    identify any node in the map.
     * @since DOM Level 2
     */
    virtual DOMNode   *getNamedItemNS(const XMLCh *namespaceURI,
                                      const XMLCh *localName) const = 0;

    /**
     * Adds a node using its <CODE>namespaceURI</CODE> and <CODE>localName</CODE>.
     *
     * @param arg A node to store in a named node map. The node will later be
     *       accessible using the value of the <CODE>namespaceURI</CODE> and
     *       <CODE>localName</CODE> attribute of the node. If a node with those
     *       namespace URI and local name is already present in the map, it is
     *       replaced by the new one.
     * @return If the new <code>DOMNode</code> replaces an existing node the
     *   replaced <code>DOMNode</code> is returned,
     *   otherwise <code>null</code> is returned.
     * @exception DOMException
     *   WRONG_DOCUMENT_ERR: Raised if <code>arg</code> was created from a
     *   different document than the one that created the
     *   <code>DOMNamedNodeMap</code>.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this
     *   <code>DOMNamedNodeMap</code> is readonly.
     *   <br>INUSE_ATTRIBUTE_ERR: Raised if <code>arg</code> is an
     *   <code>DOMAttr</code> that is already an attribute of another
     *   <code>DOMElement</code> object. The DOM user must explicitly clone
     *   <code>DOMAttr</code> nodes to re-use them in other elements.
     * @since DOM Level 2
     */
    virtual DOMNode   *setNamedItemNS(DOMNode *arg) = 0;

    /**
     * Removes a node specified by local name and namespace URI.
     *
     * @param namespaceURI The <em>namespace URI</em> of
     *    the node to remove.
     * @param localName The <em>local name</em> of the
     *    node to remove. When this <code>DOMNamedNodeMap</code> contains the
     *    attributes attached to an element, as returned by the attributes
     *    attribute of the <code>DOMNode</code> interface, if the removed
     *    attribute is known to have a default value, an attribute
     *    immediately appears containing the default value
     *    as well as the corresponding namespace URI, local name, and prefix.
     * @return The node removed from the map if a node with such a local name
     *    and namespace URI exists.
     * @exception DOMException
     *   NOT_FOUND_ERR: Raised if there is no node named <code>name</code> in
     *   the map.
     * <br>
     *   NO_MODIFICATION_ALLOWED_ERR: Raised if this <code>DOMNamedNodeMap</code>
     *   is readonly.
     * @since DOM Level 2
     */
    virtual DOMNode     *removeNamedItemNS(const XMLCh *namespaceURI,
                                           const XMLCh *localName) = 0;
    //@}

};

#define GetDOMNamedNodeMapMemoryManager   GET_INDIRECT_MM(fOwnerNode)

XERCES_CPP_NAMESPACE_END

#endif
