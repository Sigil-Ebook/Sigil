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
 * $Id: DOMLSSerializerFilter.hpp 671894 2008-06-26 13:29:21Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMLSSERIALIZERFILTER_HPP)
#define XERCESC_INCLUDE_GUARD_DOMLSSERIALIZERFILTER_HPP

/**
 *
 * DOMLSSerializerFilter.hpp: interface for the DOMLSSerializerFilter class.
 *
 * DOMLSSerializerFilter provide applications the ability to examine nodes
 * as they are being serialized.
 *
 * DOMLSSerializerFilter lets the application decide what nodes should be
 * serialized or not.
 *
 * The DOMDocument, DOMDocumentType, DOMNotation, and DOMEntity nodes are not passed
 * to the filter.
 *
 * @since DOM Level 3
 */


#include <xercesc/dom/DOMNodeFilter.hpp>

XERCES_CPP_NAMESPACE_BEGIN


class CDOM_EXPORT DOMLSSerializerFilter : public DOMNodeFilter {
protected:
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    /** @name Hidden constructors */
    //@{
    DOMLSSerializerFilter() {};
    //@}

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    /** @name Unimplemented constructors and operators */
    //@{
    DOMLSSerializerFilter(const DOMLSSerializerFilter &);
    DOMLSSerializerFilter & operator = (const DOMLSSerializerFilter &);
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
    virtual ~DOMLSSerializerFilter() {};
    //@}

    // -----------------------------------------------------------------------
    //  Virtual DOMLSSerializerFilter interface
    // -----------------------------------------------------------------------
    /** @name Functions introduced in DOM Level 3 */
    //@{
	 /**
     * Interface from <code>DOMNodeFilter</code>,
     * to be implemented by implementation (derived class)
     */
    virtual FilterAction acceptNode(const DOMNode* node) const = 0;

    /**
     * Tells the DOMLSSerializer what types of nodes to show to the filter.
     * See <code>DOMNodeFilter</code> for definition of the constants.
     * The constant SHOW_ATTRIBUTE is meaningless here, attribute nodes will
     * never be passed to a DOMLSSerializerFilter.
     *
     * @return The constants of what types of nodes to show.
     * @since DOM Level 3
     */
    virtual ShowType getWhatToShow() const =0;
    //@}
};

XERCES_CPP_NAMESPACE_END

#endif
