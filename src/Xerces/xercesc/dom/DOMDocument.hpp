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
 * $Id: DOMDocument.hpp 932887 2010-04-11 13:04:59Z borisk $
*/

#if !defined(XERCESC_INCLUDE_GUARD_DOMDOCUMENT_HPP)
#define XERCESC_INCLUDE_GUARD_DOMDOCUMENT_HPP

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/dom/DOMNode.hpp>
#include <xercesc/dom/DOMDocumentRange.hpp>
#include <xercesc/dom/DOMDocumentTraversal.hpp>
#include <xercesc/dom/DOMXPathEvaluator.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class DOMConfiguration;
class DOMDocumentType;
class DOMElement;
class DOMDocumentFragment;
class DOMComment;
class DOMCDATASection;
class DOMProcessingInstruction;
class DOMAttr;
class DOMEntity;
class DOMEntityReference;
class DOMImplementation;
class DOMNodeFilter;
class DOMNodeList;
class DOMNotation;
class DOMText;
class DOMNode;


/**
 * The <code>DOMDocument</code> interface represents the entire XML
 * document. Conceptually, it is the root of the document tree, and provides
 * the primary access to the document's data.
 * <p>Since elements, text nodes, comments, processing instructions, etc.
 * cannot exist outside the context of a <code>DOMDocument</code>, the
 * <code>DOMDocument</code> interface also contains the factory methods needed
 * to create these objects. The <code>DOMNode</code> objects created have a
 * <code>ownerDocument</code> attribute which associates them with the
 * <code>DOMDocument</code> within whose context they were created.
 * <p>See also the <a href='http://www.w3.org/TR/2000/REC-DOM-Level-2-Core-20001113'>Document Object Model (DOM) Level 2 Core Specification</a>.
 */

class CDOM_EXPORT DOMDocument: public DOMDocumentRange,
 public DOMXPathEvaluator,
 public DOMDocumentTraversal,
 public DOMNode {


protected:
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    /** @name Hidden constructors */
    //@{
    DOMDocument() {};
    //@}

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    /** @name Unimplemented constructors and operators */
    //@{
    DOMDocument(const DOMDocument &);
    DOMDocument & operator = (const DOMDocument &);
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
    virtual ~DOMDocument() {};
    //@}

    // -----------------------------------------------------------------------
    // Virtual DOMDocument interface
    // -----------------------------------------------------------------------
    /** @name Functions introduced in DOM Level 1 */
    //@{
    /**
     * Creates an element of the type specified. Note that the instance
     * returned implements the <code>DOMElement</code> interface, so attributes
     * can be specified directly on the returned object.
     * <br>In addition, if there are known attributes with default values,
     * <code>DOMAttr</code> nodes representing them are automatically created
     * and attached to the element.
     * <br>To create an element with a qualified name and namespace URI, use
     * the <code>createElementNS</code> method.
     * @param tagName The name of the element type to instantiate. For XML,
     *   this is case-sensitive.
     * @return A new <code>DOMElement</code> object with the
     *   <code>nodeName</code> attribute set to <code>tagName</code>, and
     *   <code>localName</code>, <code>prefix</code>, and
     *   <code>namespaceURI</code> set to <code>null</code>.
     * @exception DOMException
     *   INVALID_CHARACTER_ERR: Raised if the specified name contains an
     *   illegal character.
     * @since DOM Level 1
     */
    virtual DOMElement     *createElement(const XMLCh *tagName) = 0;

    /**
     * Creates an empty <code>DOMDocumentFragment</code> object.
     * @return A new <code>DOMDocumentFragment</code>.
     * @since DOM Level 1
     */
    virtual DOMDocumentFragment   *createDocumentFragment() = 0;

    /**
     * Creates a <code>DOMText</code> node given the specified string.
     * @param data The data for the node.
     * @return The new <code>DOMText</code> object.
     * @since DOM Level 1
     */
    virtual DOMText         *createTextNode(const XMLCh *data) = 0;

    /**
     * Creates a <code>DOMComment</code> node given the specified string.
     * @param data The data for the node.
     * @return The new <code>DOMComment</code> object.
     * @since DOM Level 1
     */
    virtual DOMComment      *createComment(const XMLCh *data) = 0;

    /**
     * Creates a <code>DOMCDATASection</code> node whose value is the specified
     * string.
     * @param data The data for the <code>DOMCDATASection</code> contents.
     * @return The new <code>DOMCDATASection</code> object.
     * @since DOM Level 1
     */
    virtual DOMCDATASection   *createCDATASection(const XMLCh *data) = 0;

    /**
     * Creates a <code>DOMProcessingInstruction</code> node given the specified
     * name and data strings.
     * @param target The target part of the processing instruction.
     * @param data The data for the node.
     * @return The new <code>DOMProcessingInstruction</code> object.
     * @exception DOMException
     *   INVALID_CHARACTER_ERR: Raised if the specified target contains an
     *   illegal character.
     * @since DOM Level 1
     */
    virtual DOMProcessingInstruction *createProcessingInstruction(const XMLCh *target,
        const XMLCh *data) = 0;


    /**
     * Creates an <code>DOMAttr</code> of the given name. Note that the
     * <code>DOMAttr</code> instance can then be set on an <code>DOMElement</code>
     * using the <code>setAttributeNode</code> method.
     * <br>To create an attribute with a qualified name and namespace URI, use
     * the <code>createAttributeNS</code> method.
     * @param name The name of the attribute.
     * @return A new <code>DOMAttr</code> object with the <code>nodeName</code>
     *   attribute set to <code>name</code>, and <code>localName</code>,
     *   <code>prefix</code>, and <code>namespaceURI</code> set to
     *   <code>null</code>. The value of the attribute is the empty string.
     * @exception DOMException
     *   INVALID_CHARACTER_ERR: Raised if the specified name contains an
     *   illegal character.
     * @since DOM Level 1
     */
    virtual DOMAttr     *createAttribute(const XMLCh *name) = 0;


    /**
     * Creates an <code>DOMEntityReference</code> object. In addition, if the
     * referenced entity is known, the child list of the
     * <code>DOMEntityReference</code> node is made the same as that of the
     * corresponding <code>DOMEntity</code> node.If any descendant of the
     * <code>DOMEntity</code> node has an unbound namespace prefix, the
     * corresponding descendant of the created <code>DOMEntityReference</code>
     * node is also unbound; (its <code>namespaceURI</code> is
     * <code>null</code>). The DOM Level 2 does not support any mechanism to
     * resolve namespace prefixes.
     * @param name The name of the entity to reference.
     * @return The new <code>DOMEntityReference</code> object.
     * @exception DOMException
     *   INVALID_CHARACTER_ERR: Raised if the specified name contains an
     *   illegal character.
     * @since DOM Level 1
     */
    virtual DOMEntityReference    *createEntityReference(const XMLCh *name) = 0;

    /**
     * The Document Type Declaration (see <code>DOMDocumentType</code>)
     * associated with this document. For XML
     * documents without a document type declaration this returns
     * <code>null</code>. The DOM Level 2 does not support editing the
     * Document Type Declaration. <code>docType</code> cannot be altered in
     * any way, including through the use of methods inherited from the
     * <code>DOMNode</code> interface, such as <code>insertNode</code> or
     * <code>removeNode</code>.
     * @since DOM Level 1
     */
    virtual DOMDocumentType       *getDoctype() const = 0;

    /**
     * The <code>DOMImplementation</code> object that handles this document. A
     * DOM application may use objects from multiple implementations.
     * @since DOM Level 1
     */
    virtual DOMImplementation  *getImplementation() const = 0;

    /**
     * This is a convenience attribute that allows direct access to the child
     * node that is the root element of the document.
     * @since DOM Level 1
     */
    virtual DOMElement     *getDocumentElement() const = 0;

    /**
     * Returns a <code>DOMNodeList</code> of all the <code>DOMElement(s)</code> with a
     * given tag name in the order in which they are encountered in a
     * preorder traversal of the <code>DOMDocument</code> tree.
     *
     * The returned node list is "live", in that changes
     * to the document tree made after a nodelist was initially
     * returned will be immediately reflected in the node list.
     * @param tagname The name of the tag to match on. The special value "*"
     *   matches all tags.
     * @return A new <code>DOMNodeList</code> object containing all the matched
     *   <code>DOMElement(s)</code>.
     * @since DOM Level 1
     */
    virtual DOMNodeList      *getElementsByTagName(const XMLCh *tagname) const = 0;

    //@}

    /** @name Functions introduced in DOM Level 2. */
    //@{

    /**
     * Imports a node from another document to this document. The returned
     * node has no parent; (<code>parentNode</code> is <code>null</code>).
     * The source node is not altered or removed from the original document;
     * this method creates a new copy of the source node.
     * <br>For all nodes, importing a node creates a node object owned by the
     * importing document, with attribute values identical to the source
     * node's <code>nodeName</code> and <code>nodeType</code>, plus the
     * attributes related to namespaces (<code>prefix</code>,
     * <code>localName</code>, and <code>namespaceURI</code>). As in the
     * <code>cloneNode</code> operation on a <code>DOMNode</code>, the source
     * node is not altered.
     * <br>Additional information is copied as appropriate to the
     * <code>nodeType</code>, attempting to mirror the behavior expected if
     * a fragment of XML source was copied from one document to
     * another, recognizing that the two documents may have different DTDs
     * in the XML case. The following list describes the specifics for each
     * type of node.
     * <dl>
     * <dt>ATTRIBUTE_NODE</dt>
     * <dd>The <code>ownerElement</code> attribute
     * is set to <code>null</code> and the <code>specified</code> flag is
     * set to <code>true</code> on the generated <code>DOMAttr</code>. The
     * descendants of the source <code>DOMAttr</code> are recursively imported
     * and the resulting nodes reassembled to form the corresponding subtree.
     * Note that the <code>deep</code> parameter has no effect on
     * <code>DOMAttr</code> nodes; they always carry their children with them
     * when imported.</dd>
     * <dt>DOCUMENT_FRAGMENT_NODE</dt>
     * <dd>If the <code>deep</code> option
     * was set to <code>true</code>, the descendants of the source element
     * are recursively imported and the resulting nodes reassembled to form
     * the corresponding subtree. Otherwise, this simply generates an empty
     * <code>DOMDocumentFragment</code>.</dd>
     * <dt>DOCUMENT_NODE</dt>
     * <dd><code>DOMDocument</code>
     * nodes cannot be imported.</dd>
     * <dt>DOCUMENT_TYPE_NODE</dt>
     * <dd><code>DOMDocumentType</code>
     * nodes cannot be imported.</dd>
     * <dt>ELEMENT_NODE</dt>
     * <dd>Specified attribute nodes of the
     * source element are imported, and the generated <code>DOMAttr</code>
     * nodes are attached to the generated <code>DOMElement</code>. Default
     * attributes are not copied, though if the document being imported into
     * defines default attributes for this element name, those are assigned.
     * If the <code>importNode</code> <code>deep</code> parameter was set to
     * <code>true</code>, the descendants of the source element are
     * recursively imported and the resulting nodes reassembled to form the
     * corresponding subtree.</dd>
     * <dt>ENTITY_NODE</dt>
     * <dd><code>DOMEntity</code> nodes can be
     * imported, however in the current release of the DOM the
     * <code>DOMDocumentType</code> is readonly. Ability to add these imported
     * nodes to a <code>DOMDocumentType</code> will be considered for addition
     * to a future release of the DOM.On import, the <code>publicId</code>,
     * <code>systemId</code>, and <code>notationName</code> attributes are
     * copied. If a <code>deep</code> import is requested, the descendants
     * of the the source <code>DOMEntity</code> are recursively imported and
     * the resulting nodes reassembled to form the corresponding subtree.</dd>
     * <dt>
     * ENTITY_REFERENCE_NODE</dt>
     * <dd>Only the <code>DOMEntityReference</code> itself is
     * copied, even if a <code>deep</code> import is requested, since the
     * source and destination documents might have defined the entity
     * differently. If the document being imported into provides a
     * definition for this entity name, its value is assigned.</dd>
     * <dt>NOTATION_NODE</dt>
     * <dd>
     * <code>DOMNotation</code> nodes can be imported, however in the current
     * release of the DOM the <code>DOMDocumentType</code> is readonly. Ability
     * to add these imported nodes to a <code>DOMDocumentType</code> will be
     * considered for addition to a future release of the DOM.On import, the
     * <code>publicId</code> and <code>systemId</code> attributes are copied.
     * Note that the <code>deep</code> parameter has no effect on
     * <code>DOMNotation</code> nodes since they never have any children.</dd>
     * <dt>
     * PROCESSING_INSTRUCTION_NODE</dt>
     * <dd>The imported node copies its
     * <code>target</code> and <code>data</code> values from those of the
     * source node.</dd>
     * <dt>TEXT_NODE, CDATA_SECTION_NODE, COMMENT_NODE</dt>
     * <dd>These three
     * types of nodes inheriting from <code>DOMCharacterData</code> copy their
     * <code>data</code> and <code>length</code> attributes from those of
     * the source node.</dd>
     * </dl>
     * @param importedNode The node to import.
     * @param deep If <code>true</code>, recursively import the subtree under
     *   the specified node; if <code>false</code>, import only the node
     *   itself, as explained above. This has no effect on <code>DOMAttr</code>
     *   , <code>DOMEntityReference</code>, and <code>DOMNotation</code> nodes.
     * @return The imported node that belongs to this <code>DOMDocument</code>.
     * @exception DOMException
     *   NOT_SUPPORTED_ERR: Raised if the type of node being imported is not
     *   supported.
     * @since DOM Level 2
     */
    virtual DOMNode        *importNode(const DOMNode *importedNode, bool deep) = 0;

    /**
     * Creates an element of the given qualified name and namespace URI.
     * @param namespaceURI The namespace URI of the element to create.
     * @param qualifiedName The qualified name of the element type to
     *   instantiate.
     * @return A new <code>DOMElement</code> object with the following
     *   attributes:
     * <table border='1'>
     * <tr>
     * <td valign='top' rowspan='1' colspan='1'><code>Attribute</code></td>
     * <td valign='top' rowspan='1' colspan='1'>
     *   <code>Value</code></td>
     * </tr>
     * <tr>
     * <td valign='top' rowspan='1' colspan='1'><code>DOMNode.nodeName</code></td>
     * <td valign='top' rowspan='1' colspan='1'>
     *   <code>qualifiedName</code></td>
     * </tr>
     * <tr>
     * <td valign='top' rowspan='1' colspan='1'><code>DOMNode.namespaceURI</code></td>
     * <td valign='top' rowspan='1' colspan='1'>
     *   <code>namespaceURI</code></td>
     * </tr>
     * <tr>
     * <td valign='top' rowspan='1' colspan='1'><code>DOMNode.prefix</code></td>
     * <td valign='top' rowspan='1' colspan='1'>prefix, extracted
     *   from <code>qualifiedName</code>, or <code>null</code> if there is
     *   no prefix</td>
     * </tr>
     * <tr>
     * <td valign='top' rowspan='1' colspan='1'><code>DOMNode.localName</code></td>
     * <td valign='top' rowspan='1' colspan='1'>local name, extracted from
     *   <code>qualifiedName</code></td>
     * </tr>
     * <tr>
     * <td valign='top' rowspan='1' colspan='1'><code>DOMElement.tagName</code></td>
     * <td valign='top' rowspan='1' colspan='1'>
     *   <code>qualifiedName</code></td>
     * </tr>
     * </table>
     * @exception DOMException
     *   INVALID_CHARACTER_ERR: Raised if the specified qualified name
     *   contains an illegal character, per the XML 1.0 specification .
     *   <br>NAMESPACE_ERR: Raised if the <code>qualifiedName</code> is
     *   malformed per the Namespaces in XML specification, if the
     *   <code>qualifiedName</code> has a prefix and the
     *   <code>namespaceURI</code> is <code>null</code>, or if the
     *   <code>qualifiedName</code> has a prefix that is "xml" and the
     *   <code>namespaceURI</code> is different from "
     *   http://www.w3.org/XML/1998/namespace" .
     *   <br>NOT_SUPPORTED_ERR: Always thrown if the current document does not
     *   support the <code>"XML"</code> feature, since namespaces were
     *   defined by XML.
     * @since DOM Level 2
     */
    virtual DOMElement         *createElementNS(const XMLCh *namespaceURI,
	                                              const XMLCh *qualifiedName) = 0;

    /**
     * Creates an attribute of the given qualified name and namespace URI.
     * @param namespaceURI The namespace URI of the attribute to create.
     * @param qualifiedName The qualified name of the attribute to
     *   instantiate.
     * @return A new <code>DOMAttr</code> object with the following attributes:
     * <table border='1'>
     * <tr>
     * <td valign='top' rowspan='1' colspan='1'><code>Attribute</code></td>
     * <td valign='top' rowspan='1' colspan='1'>
     *   <code>Value</code></td>
     * </tr>
     * <tr>
     * <td valign='top' rowspan='1' colspan='1'><code>DOMNode.nodeName</code></td>
     * <td valign='top' rowspan='1' colspan='1'>qualifiedName</td>
     * </tr>
     * <tr>
     * <td valign='top' rowspan='1' colspan='1'>
     *   <code>DOMNode.namespaceURI</code></td>
     * <td valign='top' rowspan='1' colspan='1'><code>namespaceURI</code></td>
     * </tr>
     * <tr>
     * <td valign='top' rowspan='1' colspan='1'>
     *   <code>DOMNode.prefix</code></td>
     * <td valign='top' rowspan='1' colspan='1'>prefix, extracted from
     *   <code>qualifiedName</code>, or <code>null</code> if there is no
     *   prefix</td>
     * </tr>
     * <tr>
     * <td valign='top' rowspan='1' colspan='1'><code>DOMNode.localName</code></td>
     * <td valign='top' rowspan='1' colspan='1'>local name, extracted from
     *   <code>qualifiedName</code></td>
     * </tr>
     * <tr>
     * <td valign='top' rowspan='1' colspan='1'><code>DOMAttr.name</code></td>
     * <td valign='top' rowspan='1' colspan='1'>
     *   <code>qualifiedName</code></td>
     * </tr>
     * <tr>
     * <td valign='top' rowspan='1' colspan='1'><code>DOMNode.nodeValue</code></td>
     * <td valign='top' rowspan='1' colspan='1'>the empty
     *   string</td>
     * </tr>
     * </table>
     * @exception DOMException
     *   INVALID_CHARACTER_ERR: Raised if the specified qualified name
     *   contains an illegal character, per the XML 1.0 specification .
     *   <br>NAMESPACE_ERR: Raised if the <code>qualifiedName</code> is
     *   malformed per the Namespaces in XML specification, if the
     *   <code>qualifiedName</code> has a prefix and the
     *   <code>namespaceURI</code> is <code>null</code>, if the
     *   <code>qualifiedName</code> has a prefix that is "xml" and the
     *   <code>namespaceURI</code> is different from "
     *   http://www.w3.org/XML/1998/namespace", or if the
     *   <code>qualifiedName</code>, or its prefix, is "xmlns" and the
     *   <code>namespaceURI</code> is different from "
     *   http://www.w3.org/2000/xmlns/".
     *   <br>NOT_SUPPORTED_ERR: Always thrown if the current document does not
     *   support the <code>"XML"</code> feature, since namespaces were
     *   defined by XML.
     * @since DOM Level 2
     */
    virtual DOMAttr        *createAttributeNS(const XMLCh *namespaceURI,
	                                            const XMLCh *qualifiedName) = 0;

    /**
     * Returns a <code>DOMNodeList</code> of all the <code>DOMElement(s)</code> with a
     * given local name and namespace URI in the order in which they are
     * encountered in a preorder traversal of the <code>DOMDocument</code> tree.
     * @param namespaceURI The namespace URI of the elements to match on. The
     *   special value "*" matches all namespaces.
     * @param localName The local name of the elements to match on. The
     *   special value "*" matches all local names.
     * @return A new <code>DOMNodeList</code> object containing all the matched
     *   <code>DOMElement(s)</code>.
     * @since DOM Level 2
     */
    virtual DOMNodeList        *getElementsByTagNameNS(const XMLCh *namespaceURI,
	                                                     const XMLCh *localName) const = 0;

    /**
     * Returns the <code>DOMElement</code> whose <code>ID</code> is given by
     * <code>elementId</code>. If no such element exists, returns
     * <code>null</code>. Behavior is not defined if more than one element
     * has this <code>ID</code>. The DOM implementation must have
     * information that says which attributes are of type ID. Attributes
     * with the name "ID" are not of type ID unless so defined.
     * Implementations that do not know whether attributes are of type ID or
     * not are expected to return <code>null</code>.
     * @param elementId The unique <code>id</code> value for an element.
     * @return The matching element.
     * @since DOM Level 2
     */
    virtual  DOMElement        * getElementById(const XMLCh *elementId) const = 0;
    //@}

    /** @name Functions introduced in DOM Level 3. */
    //@{

    /**
     * An attribute specifying the encoding used for this document at the time of the parsing.
     * This is <code>null</code> when it is not known, such as when the DOMDocument was created in memory.
     *
     * @since DOM Level 3
     */
    virtual const XMLCh*           getInputEncoding() const = 0;

    /**
     * An attribute specifying, as part of the XML declaration, the encoding of this document.
     * This is <code>null</code> when unspecified or when it is not known, such as when the
     * DOMDocument was created in memory.
     *
     * @since DOM Level 3
     */
    virtual const XMLCh*           getXmlEncoding() const = 0;

    /**
     * An attribute specifying, as part of the XML declaration, whether this document is standalone.
     * This is <code>false</code> when unspecified.
     *
     * @since DOM Level 3
     */
    virtual bool                   getXmlStandalone() const = 0;

    /**
     * An attribute specifying, as part of the XML declaration, whether this
     * document is standalone.
     * <br> This attribute represents the property [standalone] defined in .
     *
     * @since DOM Level 3
     */
    virtual void                   setXmlStandalone(bool standalone) = 0;

    /**
     * An attribute specifying, as part of the XML declaration, the version
     * number of this document. This is <code>null</code> when unspecified.
     * <br> This attribute represents the property [version] defined in .
     *
     * @since DOM Level 3
     */
    virtual const XMLCh*           getXmlVersion() const = 0;

    /**
     * An attribute specifying, as part of the XML declaration, the version
     * number of this document. This is <code>null</code> when unspecified.
     * <br> This attribute represents the property [version] defined in .
     *
     * @since DOM Level 3
     */
    virtual void                   setXmlVersion(const XMLCh* version) = 0;

    /**
     * The location of the document or <code>null</code> if undefined.
     * <br>Beware that when the <code>DOMDocument</code> supports the feature
     * "HTML" , the href attribute of the HTML BASE element takes precedence
     * over this attribute.
     *
     * @since DOM Level 3
     */
    virtual const XMLCh*           getDocumentURI() const = 0;
    /**
     * The location of the document or <code>null</code> if undefined.
     * <br>Beware that when the <code>DOMDocument</code> supports the feature
     * "HTML" , the href attribute of the HTML BASE element takes precedence
     * over this attribute.
     *
     * @since DOM Level 3
     */
    virtual void                   setDocumentURI(const XMLCh* documentURI) = 0;

    /**
     * An attribute specifying whether errors checking is enforced or not.
     * When set to <code>false</code>, the implementation is free to not
     * test every possible error case normally defined on DOM operations,
     * and not raise any <code>DOMException</code>. In case of error, the
     * behavior is undefined. This attribute is <code>true</code> by
     * defaults.
     *
     * @since DOM Level 3
     */
    virtual bool                   getStrictErrorChecking() const = 0;
    /**
     * An attribute specifying whether errors checking is enforced or not.
     * When set to <code>false</code>, the implementation is free to not
     * test every possible error case normally defined on DOM operations,
     * and not raise any <code>DOMException</code>. In case of error, the
     * behavior is undefined. This attribute is <code>true</code> by
     * defaults.
     *
     * @since DOM Level 3
     */
    virtual void                   setStrictErrorChecking(bool strictErrorChecking) = 0;

    /**
     * Rename an existing node. When possible this simply changes the name of
     * the given node, otherwise this creates a new node with the specified
     * name and replaces the existing node with the new node as described
     * below. This only applies to nodes of type <code>ELEMENT_NODE</code>
     * and <code>ATTRIBUTE_NODE</code>.
     * <br>When a new node is created, the following operations are performed:
     * the new node is created, any registered event listener is registered
     * on the new node, any user data attached to the old node is removed
     * from that node, the old node is removed from its parent if it has
     * one, the children are moved to the new node, if the renamed node is
     * an <code>DOMElement</code> its attributes are moved to the new node, the
     * new node is inserted at the position the old node used to have in its
     * parent's child nodes list if it has one, the user data that was
     * attached to the old node is attach to the new node, the user data
     * event <code>NODE_RENAMED</code> is fired.
     * <br>When the node being renamed is an <code>DOMAttr</code> that is
     * attached to an <code>DOMElement</code>, the node is first removed from
     * the <code>DOMElement</code> attributes map. Then, once renamed, either
     * by modifying the existing node or creating a new one as described
     * above, it is put back.
     *
     * @param n The node to rename.
     * @param namespaceURI The new namespaceURI.
     * @param qualifiedName The new qualified name.
     * @return The renamed node. This is either the specified node or the new
     *   node that was created to replace the specified node.
     * @exception DOMException
     *   NOT_SUPPORTED_ERR: Raised when the type of the specified node is
     *   neither <code>ELEMENT_NODE</code> nor <code>ATTRIBUTE_NODE</code>.
     *   <br>WRONG_DOCUMENT_ERR: Raised when the specified node was created
     *   from a different document than this document.
     *   <br>NAMESPACE_ERR: Raised if the <code>qualifiedName</code> is
     *   malformed per the Namespaces in XML specification, if the
     *   <code>qualifiedName</code> has a prefix and the
     *   <code>namespaceURI</code> is <code>null</code>, or if the
     *   <code>qualifiedName</code> has a prefix that is "xml" and the
     *   <code>namespaceURI</code> is different from "
     *   http://www.w3.org/XML/1998/namespace" . Also raised, when the node
     *   being renamed is an attribute, if the <code>qualifiedName</code>,
     *   or its prefix, is "xmlns" and the <code>namespaceURI</code> is
     *   different from "http://www.w3.org/2000/xmlns/".
     * @since DOM Level 3
     */
    virtual DOMNode* renameNode(DOMNode* n, const XMLCh* namespaceURI, const XMLCh* qualifiedName) = 0;


    /**
     * Changes the <code>ownerDocument</code> of a node, its children, as well
     * as the attached attribute nodes if there are any. If the node has a
     * parent it is first removed from its parent child list. This
     * effectively allows moving a subtree from one document to another. The
     * following list describes the specifics for each type of node.
     *
     * <dl>
     * <dt>
     * ATTRIBUTE_NODE</dt>
     * <dd>The <code>ownerElement</code> attribute is set to
     * <code>null</code> and the <code>specified</code> flag is set to
     * <code>true</code> on the adopted <code>DOMAttr</code>. The descendants
     * of the source <code>DOMAttr</code> are recursively adopted.</dd>
     * <dt>
     * DOCUMENT_FRAGMENT_NODE</dt>
     * <dd>The descendants of the source node are
     * recursively adopted.</dd>
     * <dt>DOCUMENT_NODE</dt>
     * <dd><code>DOMDocument</code> nodes cannot
     * be adopted.</dd>
     * <dt>DOCUMENT_TYPE_NODE</dt>
     * <dd><code>DOMDocumentType</code> nodes cannot
     * be adopted.</dd>
     * <dt>ELEMENT_NODE</dt>
     * <dd>Specified attribute nodes of the source
     * element are adopted, and the generated <code>DOMAttr</code> nodes.
     * Default attributes are discarded, though if the document being
     * adopted into defines default attributes for this element name, those
     * are assigned. The descendants of the source element are recursively
     * adopted.</dd>
     * <dt>ENTITY_NODE</dt>
     * <dd><code>DOMEntity</code> nodes cannot be adopted.</dd>
     * <dt>
     * ENTITY_REFERENCE_NODE</dt>
     * <dd>Only the <code>DOMEntityReference</code> node
     * itself is adopted, the descendants are discarded, since the source
     * and destination documents might have defined the entity differently.
     * If the document being imported into provides a definition for this
     * entity name, its value is assigned.</dd>
     * <dt>NOTATION_NODE</dt>
     * <dd><code>DOMNotation</code>
     * nodes cannot be adopted.</dd>
     * <dt>PROCESSING_INSTRUCTION_NODE, TEXT_NODE,
     * CDATA_SECTION_NODE, COMMENT_NODE</dt>
     * <dd>These nodes can all be adopted. No
     * specifics.</dd>
     * </dl>
     * @param source The node to move into this document.
     * @return The adopted node, or <code>null</code> if this operation
     *   fails, such as when the source node comes from a different
     *   implementation.
     * @exception DOMException
     *   NOT_SUPPORTED_ERR: Raised if the source node is of type
     *   <code>DOCUMENT</code>, <code>DOCUMENT_TYPE</code>.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised when the source node is
     *   readonly.
     * @since DOM Level 3
     */
    virtual DOMNode*               adoptNode(DOMNode* source) = 0;

    /**
     * This method acts as if the document was going through a save and load
     * cycle, putting the document in a "normal" form. The actual result
     * depends on the features being set. See <code>DOMConfiguration</code> for
     * details.
     *
     * <br>Noticeably this method normalizes <code>DOMText</code> nodes, makes
     * the document "namespace wellformed", according to the algorithm
     * described below in pseudo code, by adding missing namespace
     * declaration attributes and adding or changing namespace prefixes,
     * updates the replacement tree of <code>DOMEntityReference</code> nodes,
     * normalizes attribute values, etc.
     * <br>Mutation events, when supported, are generated to reflect the
     * changes occurring on the document.
     * Note that this is a partial implementation. Not all the required features are implemented.
     * Currently <code>DOMAttr</code> and <code>DOMText</code> nodes are normalized.
     * Features to remove <code>DOMComment</code> and <code>DOMCDATASection</code> work.
     * @since DOM Level 3
     *
     */
    virtual void                   normalizeDocument() = 0;


    /**
     * The configuration used when DOMDocument::normalizeDocument is invoked.
     *
     * @return The <code>DOMConfiguration</code> from this <code>DOMDocument</code>
     *
     * @since DOM Level 3
     */
    virtual DOMConfiguration*      getDOMConfig() const = 0;

    //@}

    // -----------------------------------------------------------------------
    // Non-standard extension
    // -----------------------------------------------------------------------
    /** @name Non-standard extension */
    //@{
    /**
     * Non-standard extension
     *
     * Create a new entity.
     * @param name The name of the entity to instantiate
     *
     */
    virtual DOMEntity     *createEntity(const XMLCh *name) = 0;

    /**
     * Non-standard extension
     *
     * Create a DOMDocumentType node.
     * @return A <code>DOMDocumentType</code> that references the newly
     *  created DOMDocumentType node.
     *
     */
    virtual DOMDocumentType *createDocumentType(const XMLCh *name) = 0;

    /***
     * Provide default implementation to maintain source code compatibility
     ***/
    virtual DOMDocumentType* createDocumentType(const XMLCh *qName,
                                                const XMLCh*,  //publicId,
                                                const XMLCh*   //systemId
                                               )
    {
        return createDocumentType(qName);
    }

    /**
     * Non-standard extension.
     *
     * Create a Notation.
     * @param name The name of the notation to instantiate
     * @return A <code>DOMNotation</code> that references the newly
     *  created DOMNotation node.
     */
    virtual DOMNotation *createNotation(const XMLCh *name) = 0;

    /**
     * Non-standard extension.
     *
     * Creates an element of the given qualified name and
     * namespace URI, and also stores line/column number info.
     * Used by internally XSDXercesDOMParser during schema traversal.
     *
     * @see createElementNS(const XMLCh *namespaceURI, const XMLCh *qualifiedName)
     */
    virtual DOMElement *createElementNS(const XMLCh *namespaceURI,
                                        const XMLCh *qualifiedName,
                                        const XMLFileLoc lineNum,
                                        const XMLFileLoc columnNum) = 0;
    //@}

};

XERCES_CPP_NAMESPACE_END

#endif
