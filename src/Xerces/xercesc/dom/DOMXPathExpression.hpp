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
 * $Id: DOMXPathExpression.hpp 698579 2008-09-24 14:13:08Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMXPATHEXPRESSION_HPP)
#define XERCESC_INCLUDE_GUARD_DOMXPATHEXPRESSION_HPP

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/dom/DOMXPathResult.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class DOMNode;

/**
 * The <code>DOMXPathExpression</code> interface represents a parsed and resolved XPath expression.
 * @since DOM Level 3
 */
class CDOM_EXPORT DOMXPathExpression
{

protected:
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    /** @name Hidden constructors */
    //@{
    DOMXPathExpression() {};
    //@}

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    /** @name Unimplemented constructors and operators */
    //@{
    DOMXPathExpression(const DOMXPathExpression &);
    DOMXPathExpression& operator = (const  DOMXPathExpression&);
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
    virtual ~DOMXPathExpression() {};
    //@}

    // -----------------------------------------------------------------------
    // Virtual DOMXPathExpression interface
    // -----------------------------------------------------------------------
    /** @name Functions introduced in DOM Level 3 */
    //@{

    /**
     * Evaluates this XPath expression and returns a result.
     * @param contextNode of type <code>DOMNode</code> The context is context
     * node for the evaluation of this XPath expression.
     * If the XPathEvaluator was obtained by casting the Document then this must
     * be owned by the same document and must be a <code>DOMDocument</code>, <code>DOMElement</code>,
     * <code>DOMAttribute</code>, <code>DOMText</code>, <code>DOMCDATASection</code>,
     * <code>DOMComment</code>, <code>DOMProcessingInstruction</code>, or
     * <code>XPathNamespace</code>. If the context node is a <code>DOMText</code> or a
     * <code>DOMCDATASection</code>, then the context is interpreted as the whole logical
     * text node as seen by XPath, unless the node is empty in which case it may not
     * serve as the XPath context.
     * @param type If a specific type is specified, then the result
     * will be coerced to return the specified type relying on XPath conversions and fail
     * if the desired coercion is not possible. This must be one of the type codes of <code>DOMXPathResult</code>.
     * @param result of type DOMXPathResult* The result specifies a specific result object which
     * may be reused and returned by this method. If this is specified as nullor the
     * implementation does not reuse the specified result, a new result object will be constructed
     * and returned.
     * @return DOMXPathResult* The result of the evaluation of the XPath expression.
     * @exception <code>DOMXPathException</code>
     * TYPE_ERR: Raised if the result cannot be converted to return the specified type.
     * @exception <code>DOMException</code>
     * WRONG_DOCUMENT_ERR: The <code>DOMNode</code> is from a document that is not supported by
     * the <code>XPathEvaluator</code> that created this <code>DOMXPathExpression</code>.
     * NOT_SUPPORTED_ERR: The DOMNode is not a type permitted as an XPath context node or the
     * request type is not permitted by this <code>DOMXPathExpression</code>.
     */

    virtual DOMXPathResult* evaluate(const DOMNode *contextNode,
                                     DOMXPathResult::ResultType type,
                                     DOMXPathResult* result) const = 0;
    //@}

    // -----------------------------------------------------------------------
    //  Non-standard Extension
    // -----------------------------------------------------------------------
    /** @name Non-standard Extension */
    //@{
    /**
     * Called to indicate that this DOMXPathExpression is no longer in use
     * and that the implementation may relinquish any resources associated with it.
     *
     * Access to a released object will lead to unexpected result.
     */
    virtual void release() = 0;
    //@}

};

XERCES_CPP_NAMESPACE_END

#endif
