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
 * $Id: DOMXPathEvaluator.hpp 698579 2008-09-24 14:13:08Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMXPATHEVALUATOR_HPP)
#define XERCESC_INCLUDE_GUARD_DOMXPATHEVALUATOR_HPP

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/dom/DOMXPathResult.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class DOMXPathNSResolver;
class DOMXPathExpression;
class DOMNode;

/**
 * The evaluation of XPath expressions is provided by <code>DOMXPathEvaluator</code>.
 * In a DOM implementation which supports the XPath feature, the <code>DOMXPathEvaluator</code>
 * interface will be implemented on the same object which implements the Document interface permitting
 * it to be obtained by casting or by using the DOM Level 3 getFeature method. In this case the
 * implementation obtained from the Document supports the XPath DOM module and is compatible
 * with the XPath 1.0 specification.
 * Evaluation of expressions with specialized extension functions or variables may not
 * work in all implementations and is, therefore, not portable. XPathEvaluator implementations
 * may be available from other sources that could provide specific support for specialized extension
 * functions or variables as would be defined by other specifications.
 * @since DOM Level 3
 */
class CDOM_EXPORT DOMXPathEvaluator
{

protected:
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    /** @name Hidden constructors */
    //@{
    DOMXPathEvaluator() {};
    //@}

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    /** @name Unimplemented constructors and operators */
    //@{
    DOMXPathEvaluator(const DOMXPathEvaluator &);
    DOMXPathEvaluator& operator = (const  DOMXPathEvaluator&);
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
    virtual ~DOMXPathEvaluator() {};
    //@}

    // -----------------------------------------------------------------------
    // Virtual DOMXPathEvaluator interface
    // -----------------------------------------------------------------------
    /** @name Functions introduced in DOM Level 3 */
    //@{

    /**
     * Creates a parsed XPath expression with resolved namespaces. This is useful
     * when an expression will be reused in an application since it makes it
     * possible to compile the expression string into a more efficient internal
     * form and preresolve all namespace prefixes which occur within the expression.
     * @param expression of type XMLCh - The XPath expression string to be parsed.
     * @param resolver of type <code>XPathNSResolver</code> - The resolver permits
     * translation of all prefixes, including the xml namespace prefix, within the XPath expression
     * into appropriate namespace URIs. If this is specified as null, any namespace
     * prefix within the expression will result in <code>DOMException</code> being thrown with the
     * code NAMESPACE_ERR.
     * @return <code>DOMXPathExpression</code> The compiled form of the XPath expression.
     * @exception <code>DOMXPathException</code>
     * INVALID_EXPRESSION_ERR: Raised if the expression is not legal according to the
     * rules of the <code>DOMXPathEvaluator</code>.
     * @exception DOMException
     * NAMESPACE_ERR: Raised if the expression contains namespace prefixes which cannot
     * be resolved by the specified <code>XPathNSResolver</code>.
     * @since DOM Level 3
     */
    virtual DOMXPathExpression* createExpression(const XMLCh *expression,
                                                 const DOMXPathNSResolver *resolver) = 0;


    /** Adapts any DOM node to resolve namespaces so that an XPath expression can be
     * easily evaluated relative to the context of the node where it appeared within
     * the document. This adapter works like the DOM Level 3 method lookupNamespaceURI
     * on nodes in resolving the namespaceURI from a given prefix using the current
     * information available in the node's hierarchy at the time lookupNamespaceURI
     * is called. also correctly resolving the implicit xml prefix.
     * @param nodeResolver of type <code>DOMNode</code> The node to be used as a context
     * for namespace resolution. If this parameter is null, an unpopulated
     * <code>DOMXPathNSResolver</code> is returned, which can be populated using the
     * Xerces-C extension <code>DOMXPathNSResolver::addNamespaceBinding()</code>.
     * @return <code>DOMXPathNSResolver</code> The object which resolves namespaces
     * with respect to the definitions in scope for the specified node.
     */
    virtual DOMXPathNSResolver* createNSResolver(const DOMNode *nodeResolver) = 0;


    /**
     * Evaluates an XPath expression string and returns a result of the specified
     * type if possible.
     * @param expression of type XMLCh The XPath expression string to be parsed
     * and evaluated.
     * @param contextNode of type <code>DOMNode</code> The context is context node
     * for the evaluation
     * of this XPath expression. If the <code>DOMXPathEvaluator</code> was obtained by
     * casting the <code>DOMDocument</code> then this must be owned by the same
     * document and must be a <code>DOMDocument</code>, <code>DOMElement</code>,
     * <code>DOMAttribute</code>, <code>DOMText</code>, <code>DOMCDATASection</code>,
     * <code>DOMComment</code>, <code>DOMProcessingInstruction</code>, or
     * <code>XPathNamespace</code> node. If the context node is a <code>DOMText</code> or
     * a <code>DOMCDATASection</code>, then the context is interpreted as the whole
     * logical text node as seen by XPath, unless the node is empty in which case it
     * may not serve as the XPath context.
     * @param resolver of type <code>XPathNSResolver</code> The resolver permits
     * translation of all prefixes, including the xml namespace prefix, within
     * the XPath expression into appropriate namespace URIs. If this is specified
     * as null, any namespace prefix within the expression will result in
     * <code>DOMException</code> being thrown with the code NAMESPACE_ERR.
     * @param type - If a specific type is specified, then
     * the result will be returned as the corresponding type. This must be one
     * of the codes of the <code>DOMXPathResult</code> interface.
     * @param result of type DOMXPathResult* - The result specifies a specific result object
     * which may be reused and returned by this method. If this is specified as
     * null or the implementation does not reuse the specified result, a new result
     * object will be constructed and returned.
     * @return DOMXPathResult* The result of the evaluation of the XPath expression.
     * @exception <code>DOMXPathException</code>
     * INVALID_EXPRESSION_ERR: Raised if the expression is not legal
     * according to the rules of the <code>DOMXPathEvaluator</code>
     * TYPE_ERR: Raised if the result cannot be converted to return the specified type.
     * @exception <code>DOMException</code>
     * NAMESPACE_ERR: Raised if the expression contains namespace prefixes
     * which cannot be resolved by the specified <code>XPathNSResolver</code>.
     * WRONG_DOCUMENT_ERR: The DOMNode is from a document that is not supported
     * by this <code>DOMXPathEvaluator</code>.
     * NOT_SUPPORTED_ERR: The DOMNode is not a type permitted as an XPath context
     * node or the request type is not permitted by this <code>DOMXPathEvaluator</code>.
     */
    virtual DOMXPathResult* evaluate(const XMLCh *expression,
                                     const DOMNode *contextNode,
                                     const DOMXPathNSResolver *resolver,
                                     DOMXPathResult::ResultType type,
                                     DOMXPathResult* result) = 0;

    //@}
};

XERCES_CPP_NAMESPACE_END

#endif
