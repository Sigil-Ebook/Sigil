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
 * $Id: DOMXPathResult.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMXPATHRESULT_HPP)
#define XERCESC_INCLUDE_GUARD_DOMXPATHRESULT_HPP

#include <xercesc/util/XercesDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class DOMXPathNSResolver;
class DOMXPathExpression;
class DOMTypeInfo;
class DOMNode;

/**
 * The <code>DOMXPathResult</code> interface represents the result of the
 * evaluation of an XPath 1.0 or XPath 2.0 expression within the context
 * of a particular node. Since evaluation of an XPath expression can result
 * in various result types, this object makes it possible to discover and
 * manipulate the type and value of the result.
 *
 * Note that some function signatures were changed compared to the
 * DOM Level 3 in order to accommodate XPath 2.0.
 *
 * @since DOM Level 3
 */
class CDOM_EXPORT DOMXPathResult
{

protected:
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    /** @name Hidden constructors */
    //@{
    DOMXPathResult() {};
    //@}

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    /** @name Unimplemented constructors and operators */
    //@{
    DOMXPathResult(const DOMXPathResult &);
    DOMXPathResult& operator = (const  DOMXPathResult&);
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
    virtual ~DOMXPathResult() {};
    //@}

    // -----------------------------------------------------------------------
    //  Class Types
    // -----------------------------------------------------------------------
    /** @name Public Constants */
    //@{
    /**
     * <p>ANY_TYPE
     * <br>[XPath 1.0] This code does not represent a specific type. An evaluation of an XPath
     * expression will never produce this type. If this type is requested, then
     * the evaluation returns whatever type naturally results from evaluation
     * of the expression.
     * If the natural result is a node set when ANY_TYPE was requested, then
     * UNORDERED_NODE_ITERATOR_TYPE is always the resulting type. Any other
     * representation of a node set must be explicitly requested.
     * <p>ANY_UNORDERED_NODE_TYPE
     * <br>[XPath 1.0] The result is a node set as defined by XPath 1.0 and will be accessed
     * as a single node, which may be null if the node set is empty. Document
     * modification does not invalidate the node, but may mean that the result
     * node no longer corresponds to the current document. This is a convenience
     * that permits optimization since the implementation can stop once any node
     * in the resulting set has been found.
     * If there is more than one node in the actual result, the single node
     * returned might not be the first in document order.
     * <p>BOOLEAN_TYPE
     * <br>[XPath 1.0] The result is a boolean as defined by XPath 1.0. Document modification
     * does not invalidate the boolean, but may mean that reevaluation would not
     * yield the same boolean.
     * <p>FIRST_ORDERED_NODE_TYPE
     * <br>[XPath 1.0] The result is a node set as defined by XPath 1.0 and will be accessed
     * as a single node, which may be null if the node set is empty. Document
     * modification does not invalidate the node, but may mean that the result
     * node no longer corresponds to the current document. This is a convenience
     * that permits optimization since the implementation can stop once the first
     * node in document order of the resulting set has been found.
     * If there are more than one node in the actual result, the single node
     * returned will be the first in document order.
     * <p>NUMBER_TYPE
     * <br>[XPath 1.0] The result is a number as defined by XPath 1.0. Document modification does
     * not invalidate the number, but may mean that reevaluation would not yield the
     * same number.
     * <p>ORDERED_NODE_ITERATOR_TYPE
     * <br>[XPath 1.0] The result is a node set as defined by XPath 1.0 that will be accessed
     * iteratively, which will produce document-ordered nodes. Document modification
     * invalidates the iteration.
     * <p>ORDERED_NODE_SNAPSHOT_TYPE
     * <br>[XPath 1.0] The result is a node set as defined by XPath 1.0 that will be accessed as a
     * snapshot list of nodes that will be in original document order. Document
     * modification does not invalidate the snapshot but may mean that reevaluation would
     * not yield the same snapshot and nodes in the snapshot may have been altered, moved,
     * or removed from the document.
     * <p>STRING_TYPE
     * <br>[XPath 1.0] The result is a string as defined by XPath 1.0. Document modification does not
     * invalidate the string, but may mean that the string no longer corresponds to the
     * current document.
     * <p>UNORDERED_NODE_ITERATOR_TYPE
     * <br>[XPath 1.0] The result is a node set as defined by XPath 1.0 that will be accessed iteratively,
     * which may not produce nodes in a particular order. Document modification invalidates the iteration.
     * This is the default type returned if the result is a node set and ANY_TYPE is requested.
     * <p>UNORDERED_NODE_SNAPSHOT_TYPE
     * <br>[XPath 1.0] The result is a node set as defined by XPath 1.0 that will be accessed as a
     * snapshot list of nodes that may not be in a particular order. Document modification
     * does not invalidate the snapshot but may mean that reevaluation would not yield the same
     * snapshot and nodes in the snapshot may have been altered, moved, or removed from the document.
     * <p>FIRST_RESULT_TYPE
     * <br>[XPath 2.0] The result is a sequence as defined by XPath 2.0 and will be accessed
     * as a single current value or there will be no current value if the sequence
     * is empty. Document modification does not invalidate the value, but may mean
     * that the result no longer corresponds to the current document. This is a
     * convenience that permits optimization since the implementation can stop once
     * the first item in the resulting sequence has been found. If there is more
     * than one item in the actual result, the single item returned might not be
     * the first in document order.
     * <p>ITERATOR_RESULT_TYPE
     * <br>[XPath 2.0] The result is a sequence as defined by XPath 2.0 that will be accessed
     * iteratively. Document modification invalidates the iteration.
     * <p>SNAPSHOT_RESULT_TYPE
     * <br>[XPath 2.0] The result is a sequence as defined by XPath 2.0 that will be accessed
     * as a snapshot list of values. Document modification does not invalidate the
     * snapshot but may mean that reevaluation would not yield the same snapshot
     * and any items in the snapshot may have been altered, moved, or removed from
     * the document.
     */
    enum ResultType {
                /* XPath 1.0 */
                ANY_TYPE = 0,
                NUMBER_TYPE = 1,
                STRING_TYPE = 2,
                BOOLEAN_TYPE = 3,
                UNORDERED_NODE_ITERATOR_TYPE = 4,
                ORDERED_NODE_ITERATOR_TYPE = 5,
                UNORDERED_NODE_SNAPSHOT_TYPE = 6,
                ORDERED_NODE_SNAPSHOT_TYPE = 7,
                ANY_UNORDERED_NODE_TYPE = 8,
                FIRST_ORDERED_NODE_TYPE = 9,
                /* XPath 2.0 */
                FIRST_RESULT_TYPE    = 100,
                ITERATOR_RESULT_TYPE = 101,
                SNAPSHOT_RESULT_TYPE = 102
    };
    //@}


    // -----------------------------------------------------------------------
    // Virtual DOMXPathResult interface
    // -----------------------------------------------------------------------
    /** @name Functions introduced in DOM Level 3 */
    //@{

    /**
     * Returns the result type of this result
     * @return ResultType
     * A code representing the type of this result, as defined by the type constants.
     */
    virtual ResultType getResultType() const = 0;

    /**
     * Returns the DOM type info of the current result node or value
     * (XPath 2 only).
     * @return typeInfo of type TypeInfo, readonly
     */
    virtual const DOMTypeInfo *getTypeInfo() const = 0;

    /**
     * Returns true if the result has a current result and the value is a
     * node (XPath 2 only). This function is necessary to distinguish
     * between a string value and a node of type string as returned by
     * the getTypeInfo() function.
     * @return isNode of type boolean, readonly
     */
    virtual bool isNode() const = 0;

    /**
     * Returns the boolean value of this result
     * @return booleanValue of type boolean
     * The value of this boolean result.
     * @exception DOMXPathException
     * TYPE_ERR: raised if ResultType is not BOOLEAN_TYPE (XPath 1.0) or
     * if current result cannot be properly converted to boolean (XPath 2.0).
     * <br>
     * NO_RESULT_ERROR: raised if there is no current result in the result object (XPath 2.0).
     */
    virtual bool getBooleanValue() const = 0;

    /**
     * Returns the integer value of this result (XPath 2 only).
     * @return integerValue of type int
     * The value of this integer result.
     * @exception DOMXPathException
     * TYPE_ERR: raised if current result cannot be properly converted to
     * int (XPath 2.0).
     * <br>
     * NO_RESULT_ERROR: raised if there is no current result in the result object (XPath 2.0).
     */
    virtual int getIntegerValue() const = 0;

    /**
     * Returns the number value of this result
     * @return numberValue
     * The value of this number result. If the native double type of the DOM
     * binding does not directly support the exact IEEE 754 result of the XPath
     * expression, then it is up to the definition of the binding to specify how
     * the XPath number is converted to the native binding number.
     * @exception DOMXPathException
     * TYPE_ERR: raised if ResultType is not NUMBER_TYPE (XPath 1.0) or
     * if current result cannot be properly converted to double (XPath 2.0).
     * <br>
     * NO_RESULT_ERROR: raised if there is no current result in the result object (XPath 2.0).
     */
    virtual double getNumberValue() const = 0;

   /**
     * Returns the string value of this result
     * @return stringValue
     * The value of this string result.
     * @exception DOMXPathException
     * TYPE_ERR: raised if ResultType is not STRING_TYPE (XPath 1.0) or
     * if current result cannot be properly converted to string (XPath 2.0).
     * <br>
     * NO_RESULT_ERROR: raised if there is no current result in the result object (XPath 2.0).
     */
    virtual const XMLCh* getStringValue() const = 0;

    /**
     * Returns the node value of this result
     * @return nodeValue
     * The value of this node result, which may be null.
     * @exception DOMXPathException
     * TYPE_ERR: raised if ResultType is not ANY_UNORDERED_NODE_TYPE,
     * FIRST_ORDERED_NODE_TYPE, UNORDERED_NODE_ITERATOR_TYPE,
     * ORDERED_NODE_ITERATOR_TYPE, UNORDERED_NODE_SNAPSHOT_TYPE, or
     * ORDERED_NODE_SNAPSHOT_TYPE (XPath 1.0) or if current result is
     * not a node (XPath 2.0).
     * <br>
     * NO_RESULT_ERROR: raised if there is no current result in the result
     * object.
     */
    virtual DOMNode* getNodeValue() const = 0;

    /**
     * Iterates and returns true if the current result is the next item from the
     * sequence or false if there are no more items.
     * @return boolean True if the current result is the next item from the sequence
     * or false if there are no more items.
     * @exception XPathException
     * TYPE_ERR: raised if ResultType is not UNORDERED_NODE_ITERATOR_TYPE or
     * ORDERED_NODE_ITERATOR_TYPE (XPath 1.0) or if ResultType is not
     * ITERATOR_RESULT_TYPE (XPath 2.0).
     * @exception DOMException
     * INVALID_STATE_ERR: The document has been mutated since the result was returned.
     */
    virtual bool iterateNext() = 0;

    /**
     * Signifies that the iterator has become invalid.
     * @return invalidIteratorState
     * True if ResultType is UNORDERED_NODE_ITERATOR_TYPE or
     * ORDERED_NODE_ITERATOR_TYPE (XPath 1.0) or ITERATOR_RESULT_TYPE (XPath 2.0)
     * and the document has been modified since this result was returned.
     * @exception XPathException
     * TYPE_ERR: raised if ResultType is not UNORDERED_NODE_ITERATOR_TYPE or
     * ORDERED_NODE_ITERATOR_TYPE (XPath 1.0) or if ResultType is not
     * ITERATOR_RESULT_TYPE (XPath 2.0).
     */
    virtual bool getInvalidIteratorState() const = 0;

    /**
     * Sets the current result to the indexth item in the snapshot collection. If
     * index is greater than or equal to the number of items in the list, this method
     * returns false. Unlike the iterator result, the snapshot does not become
     * invalid, but may not correspond to the current document if it is mutated.
     * @param index of type XMLSize_t - Index into the snapshot collection.
     * @return boolean True if the current result is the next item from the sequence
     * or false if there are no more items.
     * @exception XPathException
     * TYPE_ERR: raised if ResultType is not UNORDERED_NODE_SNAPSHOT_TYPE or
     * ORDERED_NODE_SNAPSHOT_TYPE (XPath 1.0) or if ResultType is not
     * SNAPSHOT_RESULT_TYPE (XPath 2.0).
     */
    virtual bool snapshotItem(XMLSize_t index) = 0;

    /**
     * The number of items in the result snapshot. Valid values for snapshotItem
     * indices are 0 to snapshotLength-1 inclusive.
     * @return snapshotLength of type XMLSize_t
     * @exception XPathException
     * TYPE_ERR: raised if ResultType is not UNORDERED_NODE_SNAPSHOT_TYPE or
     * ORDERED_NODE_SNAPSHOT_TYPE (XPath 1.0) or if ResultType is not
     * SNAPSHOT_RESULT_TYPE (XPath 2.0).
     */
    virtual XMLSize_t getSnapshotLength() const = 0;

    //@}

    // -----------------------------------------------------------------------
    //  Non-standard Extension
    // -----------------------------------------------------------------------
    /** @name Non-standard Extension */
    //@{
    /**
     * Called to indicate that this DOMXPathResult is no longer in use
     * and that the implementation may relinquish any resources associated with it.
     *
     * Access to a released object will lead to unexpected result.
     */
    virtual void release() = 0;
    //@}
};

XERCES_CPP_NAMESPACE_END

#endif
