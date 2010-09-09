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
 * $Id: DOMXPathNamespace.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMXPATHNAMESPACE_HPP)
#define XERCESC_INCLUDE_GUARD_DOMXPATHNAMESPACE_HPP

#include <xercesc/dom/DOMNode.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class DOMElement;

/**
 * The <code>DOMXPathNamespace</code> interface is returned by <code>DOMXPathResult</code>
 * interfaces to represent the XPath namespace node type that DOM lacks. There is no
 * public constructor for this node type. Attempts to place it into a hierarchy or a 
 * NamedNodeMap result in a DOMException with the code HIERARCHY_REQUEST_ERR. This node
 * is read only, so methods or setting of attributes that would mutate the node result 
 * in a <code>DOMException</code> with the code NO_MODIFICATION_ALLOWED_ERR.
 * The core specification describes attributes of the <code>DOMNode</code> interface that 
 * are different for different node types but does not describe XPATH_NAMESPACE_NODE, 
 * so here is a description of those attributes for this node type. All attributes of
 * <code>DOMNode</code> not described in this section have a null or false value.
 * ownerDocument matches the ownerDocument of the ownerElement even if the element is later adopted.
 * nodeName is always the string "#namespace".
 * prefix is the prefix of the namespace represented by the node.
 * localName is the same as prefix.
 * nodeType is equal to XPATH_NAMESPACE_NODE.
 * namespaceURI is the namespace URI of the namespace represented by the node.
 * nodeValue is the same as namespaceURI.
 * adoptNode, cloneNode, and importNode fail on this node type by raising a DOMException with the code NOT_SUPPORTED_ERR.
 * Note: In future versions of the XPath specification, the definition of a namespace node may
 * be changed incompatibly, in which case incompatible changes to field values may be required to
 * implement versions beyond XPath 1.0.
 * @since DOM Level 3
 */
class CDOM_EXPORT DOMXPathNamespace : public DOMNode
{

protected:
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    /** @name Hidden constructors */
    //@{    
    DOMXPathNamespace() {};
    //@}

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    /** @name Unimplemented constructors and operators */
    //@{
    DOMXPathNamespace(const DOMXPathNamespace &);
    DOMXPathNamespace& operator = (const  DOMXPathNamespace&);
    //@}

public:

    
    enum XPathNodeType {
        XPATH_NAMESPACE_NODE = 13
    };

    // -----------------------------------------------------------------------
    //  All constructors are hidden, just the destructor is available
    // -----------------------------------------------------------------------
    /** @name Destructor */
    //@{
    /**
     * Destructor
     *
     */
    virtual ~DOMXPathNamespace() {};
    //@}

    // -----------------------------------------------------------------------
    // Virtual DOMXPathNamespace interface
    // -----------------------------------------------------------------------
    /** @name Functions introduced in DOM Level 3 */
    //@{
    /**
     * The <code>DOMElement</code> on which the namespace was in scope when 
     * it was requested. This does not change on a returned namespace node
     * even if the document changes such that the namespace goes out of
     * scope on that element and this node is no longer found there by XPath.
     * @since DOM Level 3
     */
    virtual DOMElement     *getOwnerElement() const = 0;

    //@}
};

XERCES_CPP_NAMESPACE_END

#endif
