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
 * $Id: DOMText.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMTEXT_HPP)
#define XERCESC_INCLUDE_GUARD_DOMTEXT_HPP

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/dom/DOMCharacterData.hpp>

XERCES_CPP_NAMESPACE_BEGIN


/**
 * The <code>DOMText</code> interface inherits from <code>DOMCharacterData</code>
 * and represents the textual content (termed character data in XML) of an
 * <code>DOMElement</code> or <code>DOMAttr</code>. If there is no markup inside
 * an element's content, the text is contained in a single object
 * implementing the <code>DOMText</code> interface that is the only child of
 * the element. If there is markup, it is parsed into the information items
 * (elements, comments, etc.) and <code>DOMText</code> nodes that form the list
 * of children of the element.
 * <p>When a document is first made available via the DOM, there is only one
 * <code>DOMText</code> node for each block of text. Users may create adjacent
 * <code>DOMText</code> nodes that represent the contents of a given element
 * without any intervening markup, but should be aware that there is no way
 * to represent the separations between these nodes in XML or HTML, so they
 * will not (in general) persist between DOM editing sessions. The
 * <code>normalize()</code> method on <code>DOMNode</code> merges any such
 * adjacent <code>DOMText</code> objects into a single node for each block of
 * text.
 * <p>See also the <a href='http://www.w3.org/TR/2000/REC-DOM-Level-2-Core-20001113'>Document Object Model (DOM) Level 2 Core Specification</a>.
 */
class CDOM_EXPORT DOMText: public DOMCharacterData {
protected:
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    /** @name Hidden constructors */
    //@{    
    DOMText() {}
    DOMText(const DOMText &other) : DOMCharacterData(other) {}
    //@}

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    /** @name Unimplemented operators */
    //@{
    DOMText & operator = (const DOMText &);
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
    virtual ~DOMText() {};
    //@}

    // -----------------------------------------------------------------------
    //  Virtual DOMText interface
    // -----------------------------------------------------------------------
    /** @name Functions introduced in DOM Level 1 */
    //@{
    /**
     * Breaks this node into two nodes at the specified <code>offset</code>,
     * keeping both in the tree as siblings. After being split, this node
     * will contain all the content up to the <code>offset</code> point. A
     * new node of the same type, which contains all the content at and
     * after the <code>offset</code> point, is returned. If the original
     * node had a parent node, the new node is inserted as the next sibling
     * of the original node. When the <code>offset</code> is equal to the
     * length of this node, the new node has no data.
     * @param offset The 16-bit unit offset at which to split, starting from
     *   <code>0</code>.
     * @return The new node, of the same type as this node.
     * @exception DOMException
     *   INDEX_SIZE_ERR: Raised if the specified offset is negative or greater
     *   than the number of 16-bit units in <code>data</code>.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this node is readonly.
     * @since DOM Level 1
     */
    virtual DOMText *splitText(XMLSize_t offset) = 0;
    //@}

    /** @name Functions introduced in DOM Level 3 */
    //@{
    /**
     * Returns whether this text node contains element content whitespace,
     * often abusively called "ignorable whitespace". The text node is determined 
     * to contain whitespace in element content during the load of the document 
     * or if validation occurs while using <code>DOMDocument::normalizeDocument()</code>.
     *
     * @since DOM Level 3
     */
    virtual bool     getIsElementContentWhitespace() const = 0;

    /**
     * Returns all text of <code>DOMText</code> nodes logically-adjacent text
     * nodes to this node, concatenated in document order.
     *
     * @since DOM Level 3
     */
    virtual const XMLCh* getWholeText() const = 0;

    /**
     * Substitutes the a specified text for the text of the current node and
     * all logically-adjacent text nodes.
     *
     * <br>This method returns the node in the hierarchy which received the
     * replacement text, which is null if the text was empty or is the
     * current node if the current node is not read-only or otherwise is a
     * new node of the same type as the current node inserted at the site of
     * the replacement. All logically-adjacent text nodes are removed
     * including the current node unless it was the recipient of the
     * replacement text.
     * <br>Where the nodes to be removed are read-only descendants of an
     * <code>DOMEntityReference</code>, the <code>DOMEntityReference</code> must
     * be removed instead of the read-only nodes. If any
     * <code>DOMEntityReference</code> to be removed has descendants that are
     * not <code>DOMEntityReference</code>, <code>DOMText</code>, or
     * <code>DOMCDATASection</code> nodes, the <code>replaceWholeText</code>
     * method must fail before performing any modification of the document,
     * raising a <code>DOMException</code> with the code
     * <code>NO_MODIFICATION_ALLOWED_ERR</code>.
     *
     * @param content The content of the replacing <code>DOMText</code> node.
     * @return The <code>DOMText</code> node created with the specified content.
     * @exception DOMException
     *   NO_MODIFICATION_ALLOWED_ERR: Raised if one of the <code>DOMText</code>
     *   nodes being replaced is readonly.
     * @since DOM Level 3
     */
    virtual DOMText* replaceWholeText(const XMLCh* content) = 0;
    //@}

    // -----------------------------------------------------------------------
    // Non-standard extension
    // -----------------------------------------------------------------------
    /** @name Non-standard extension */
    //@{
    /**
     * Non-standard extension
     *
     * Return true if this node contains ignorable whitespaces only.
     * @return True if this node contains ignorable whitespaces only.
     */
    virtual bool isIgnorableWhitespace() const = 0;
    //@}

};


XERCES_CPP_NAMESPACE_END

#endif


