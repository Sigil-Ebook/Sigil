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
 * $Id: DOMAttr.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMATTR_HPP)
#define XERCESC_INCLUDE_GUARD_DOMATTR_HPP

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/dom/DOMNode.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class DOMElement;
class DOMTypeInfo;

/**
 * The <code>DOMAttr</code> class refers to an attribute of an XML element.
 *
 * Typically the allowable values for the
 * attribute are defined in a documenttype definition.
 * <p><code>DOMAttr</code> objects inherit the <code>DOMNode</code>  interface, but
 * since attributes are not actually child nodes of the elements they are associated with, the
 * DOM does not consider them part of the document  tree.  Thus, the
 * <code>DOMNode</code> attributes <code>parentNode</code>,
 * <code>previousSibling</code>, and <code>nextSibling</code> have a  null
 * value for <code>DOMAttr</code> objects. The DOM takes the  view that
 * attributes are properties of elements rather than having a  separate
 * identity from the elements they are associated with;  this should make it
 * more efficient to implement such features as default attributes associated
 * with all elements of a  given type.  Furthermore, attribute nodes
 * may not be immediate children of a <code>DOMDocumentFragment</code>. However,
 * they can be associated with <code>DOMElement</code> nodes contained within a
 * <code>DOMDocumentFragment</code>. In short, users of the DOM
 * need to be aware that  <code>DOMAttr</code> nodes have some things in  common
 * with other objects inheriting the <code>DOMNode</code> interface, but they
 * also are quite distinct.
 *
 * @since DOM Level 1
 */
class CDOM_EXPORT DOMAttr: public DOMNode {
protected:
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    /** @name Hidden constructors */
    //@{    
    DOMAttr() {}
    DOMAttr(const DOMAttr &other) : DOMNode(other) {}
    //@}

private:    
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    /** @name Unimplemented operators */
    //@{
    DOMAttr & operator = (const DOMAttr &);
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
    virtual ~DOMAttr() {};
    //@}

    // -----------------------------------------------------------------------
    //  Virtual DOMAttr interface
    // -----------------------------------------------------------------------
    /** @name Functions introduced in DOM Level 1 */
    //@{
    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    /**
     * Returns the name of this attribute.
     * @since DOM Level 1
     */
    virtual const XMLCh *       getName() const = 0;

    /**
     *
     * Returns true if the attribute received its value explicitly in the
     * XML document, or if a value was assigned programatically with
     * the setValue function.  Returns false if the attribute value
     * came from the default value declared in the document's DTD.
     * @since DOM Level 1
     */
    virtual bool            getSpecified() const = 0;

    /**
     * Returns the value of the attribute.
     *
     * The value of the attribute is returned as a string.
     * Character and general entity references are replaced with their values.
     * @since DOM Level 1
     */
    virtual const XMLCh *       getValue() const = 0;

    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------
    /**
     * Sets the value of the attribute.  A text node with the unparsed contents
     * of the string will be created.
     *
     * @param value The value of the DOM attribute to be set
     * @since DOM Level 1
     */
    virtual void            setValue(const XMLCh *value) = 0;
    //@}

    /** @name Functions introduced in DOM Level 2. */
    //@{
    /**
     * The <code>DOMElement</code> node this attribute is attached to or
     * <code>null</code> if this attribute is not in use.
     *
     * @since DOM Level 2
     */
    virtual DOMElement     *getOwnerElement() const = 0;
    //@}

    /** @name Functions introduced in DOM Level 3. */
    //@{
    /**
     * Returns whether this attribute is known to be of type ID or not. 
     * When it is and its value is unique, the ownerElement of this attribute 
     * can be retrieved using getElementById on DOMDocument.
     *
     * @return <code>bool</code> stating if this <code>DOMAttr</code> is an ID
     * @since DOM level 3
     */
    virtual bool            isId() const = 0;


    /**
     * Returns the type information associated with this attribute.
     *
     * @return the <code>DOMTypeInfo</code> associated with this attribute
     * @since DOM level 3
     */
    virtual const DOMTypeInfo * getSchemaTypeInfo() const = 0;

    //@}

};

XERCES_CPP_NAMESPACE_END

#endif


