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
 * $Id: DOMNodeList.hpp 671894 2008-06-26 13:29:21Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMNODELIST_HPP)
#define XERCESC_INCLUDE_GUARD_DOMNODELIST_HPP

#include <xercesc/util/XercesDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN


class DOMNode;


/**
 * The <code>DOMNodeList</code> interface provides the abstraction of an ordered
 * collection of nodes.  DOMNodeLists are created by DOMDocument::getElementsByTagName(),
 * DOMNode::getChildNodes(),
 *
 * <p>The items in the <code>DOMNodeList</code> are accessible via an integral
 * index, starting from 0.
 *
 * DOMNodeLists are "live", in that any changes to the document tree are immediately
 * reflected in any DOMNodeLists that may have been created for that tree.
 */

class  CDOM_EXPORT DOMNodeList {
protected:
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    /** @name Hidden constructors */
    //@{
    DOMNodeList() {};
    //@}

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    /** @name Unimplemented constructors and operators */
    //@{
    DOMNodeList(const DOMNodeList &);
    DOMNodeList & operator = (const DOMNodeList &);
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
    virtual ~DOMNodeList()  {};
    //@}

    // -----------------------------------------------------------------------
    //  Virtual DOMNodeList interface
    // -----------------------------------------------------------------------
    /** @name Functions introduced in DOM Level 1 */
    //@{
    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    /**
     * Returns the <code>index</code> item in the collection.
     *
     * If <code>index</code> is greater than or equal to the number of nodes in
     * the list, this returns <code>null</code>.
     *
     * @param index Index into the collection.
     * @return The node at the <code>index</code>th position in the
     *   <code>DOMNodeList</code>, or <code>null</code> if that is not a valid
     *   index.
     * @since DOM Level 1
     */
    virtual DOMNode  *item(XMLSize_t index) const = 0;

    /**
     * Returns the number of nodes in the list.
     *
     * The range of valid child node indices is 0 to <code>length-1</code> inclusive.
     * @since DOM Level 1
     */
    virtual XMLSize_t getLength() const = 0;
    //@}
};

XERCES_CPP_NAMESPACE_END

#endif
