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
 * $Id: DOMStringList.hpp 671894 2008-06-26 13:29:21Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMSTRINGLIST_HPP)
#define XERCESC_INCLUDE_GUARD_DOMSTRINGLIST_HPP

#include <xercesc/util/XercesDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN


/**
 * The <code>DOMStringList</code> interface provides the abstraction of an ordered
 * collection of strings, without defining or constraining how this collection
 * is implemented. The items in the <code>DOMStringList</code> are accessible via
 * an integral index, starting from 0.
 */

class  CDOM_EXPORT DOMStringList {
protected:
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    /** @name Hidden constructors */
    //@{
    DOMStringList() {};
    //@}

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    /** @name Unimplemented constructors and operators */
    //@{
    DOMStringList(const DOMStringList &);
    DOMStringList & operator = (const DOMStringList &);
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
    virtual ~DOMStringList()  {};
    //@}

    // -----------------------------------------------------------------------
    //  Virtual DOMStringList interface
    // -----------------------------------------------------------------------
    /** @name Functions introduced in DOM Level 3 */
    //@{
    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    /**
     * Returns the <code>index</code> item in the collection.
     *
     * If <code>index</code> is greater than or equal to the number of strings in
     * the list, this returns <code>null</code>.
     *
     * @param index Index into the collection.
     * @return The string at the <code>index</code>th position in the
     *   <code>DOMStringList</code>, or <code>null</code> if that is not a valid
     *   index.
     * @since DOM Level 3
     */
    virtual const XMLCh *item(XMLSize_t index) const = 0;

    /**
     * Returns the number of strings in the list.
     *
     * The range of valid child node indices is 0 to <code>length-1</code> inclusive.
     *
     * @since DOM Level 3
     */
    virtual XMLSize_t getLength() const = 0;

    /**
     * Test if a string is part of this DOMStringList
     *
     * @return <code>true</code> if the string has been found, <code>false</code> otherwise.
     *
     * @since DOM Level 3
     */
    virtual bool contains(const XMLCh*) const = 0;
    //@}

    // -----------------------------------------------------------------------
    //  Non-standard Extension
    // -----------------------------------------------------------------------
    /** @name Non-standard Extension */
    //@{
    /**
     * Called to indicate that this list is no longer in use
     * and that the implementation may relinquish any resources associated with it and
     * its associated children.
     *
     * Access to a released object will lead to unexpected result.
     *
     */
    virtual void release() = 0;
    //@}

};

XERCES_CPP_NAMESPACE_END

#endif
