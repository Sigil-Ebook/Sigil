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
 * $Id: DOMLSParserFilter.hpp 671894 2008-06-26 13:29:21Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMLSPARSERFILTER_HPP)
#define XERCESC_INCLUDE_GUARD_DOMLSPARSERFILTER_HPP

 /**
 *
 * DOMLSParserFilter.hpp: interface for the DOMLSParserFilter class.
 *
 * DOMLSParserFilter provide applications the ability to examine nodes
 * as they are being created during the parse process.
 *
 * DOMLSParserFilter lets the application decide what nodes should be
 * in the output DOM tree or not.
 *
 * @since DOM Level 3
 */

#include <xercesc/dom/DOMNodeFilter.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class DOMElement;
class DOMNode;

class CDOM_EXPORT DOMLSParserFilter {
protected:
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    /** @name Hidden constructors */
    //@{
    DOMLSParserFilter() {};
    //@}

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    /** @name Unimplemented constructors and operators */
    //@{
    DOMLSParserFilter(const DOMLSParserFilter &);
    DOMLSParserFilter & operator = (const DOMLSParserFilter &);
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
    virtual ~DOMLSParserFilter() {};
    //@}

    // -----------------------------------------------------------------------
    //  Class Types
    // -----------------------------------------------------------------------
    /** @name Public Contants */
    //@{
    /**
     * Constants returned by acceptNode.
     *
     * <p><code>FILTER_ACCEPT:</code>
     * Accept the node.</p>
     *
     * <p><code>FILTER_REJECT:</code>
     * Reject the node and its children.</p>
     *
     * <p><code>FILTER_SKIP:</code>
     * Skip this single node. The children of this node will still be considered.</p>
     *
     * <p><code>FILTER_INTERRUPT:</code>
     * Interrupt the normal processing of the document.</p>
     *
     * @since DOM Level 3
     */
    enum FilterAction {FILTER_ACCEPT = 1,
                       FILTER_REJECT = 2,
                       FILTER_SKIP   = 3,
                       FILTER_INTERRUPT = 4};

    // -----------------------------------------------------------------------
    //  Virtual DOMLSParserFilter interface
    // -----------------------------------------------------------------------
    /** @name Functions introduced in DOM Level 3 */
    //@{
	 /**
       * This method will be called by the parser at the completion of the parsing of each node.
       * The node and all of its descendants will exist and be complete. The parent node will also exist,
       * although it may be incomplete, i.e. it may have additional children that have not yet been parsed.
       * Attribute nodes are never passed to this function.
       * From within this method, the new node may be freely modified - children may be added or removed,
       * text nodes modified, etc. The state of the rest of the document outside this node is not defined,
       * and the affect of any attempt to navigate to, or to modify any other part of the document is undefined.
       * For validating parsers, the checks are made on the original document, before any modification by the
       * filter. No validity checks are made on any document modifications made by the filter.
       * If this new node is rejected, the parser might reuse the new node and any of its descendants.
       *
       * @param node The newly constructed element. At the time this method is called, the element is complete -
       *             it has all of its children (and their children, recursively) and attributes, and is attached
       *             as a child to its parent.
       * @return One of the FilterAction enum
       */
    virtual FilterAction acceptNode(DOMNode* node) = 0;

	 /**
       * The parser will call this method after each <code>DOMElement</code> start tag has been scanned,
       * but before the remainder of the <code>DOMElement</code> is processed. The intent is to allow the element,
       * including any children, to be efficiently skipped. Note that only element nodes are passed to the
       * startElement function.
       * The element node passed to startElement for filtering will include all of the attributes, but none
       * of the children nodes. The <code>DOMElement</code> may not yet be in place in the document being
       * constructed (it may not have a parent node.)
       * A startElement filter function may access or change the attributes for the <code>DOMElement</code>.
       * Changing namespace declarations will have no effect on namespace resolution by the parser.
       *
       * @param node The newly encountered element. At the time this method is called, the element is incomplete -
       *             it will have its attributes, but no children.
       * @return One of the FilterAction enum
       */
    virtual FilterAction startElement(DOMElement* node) = 0;

    /**
     * Tells the <code>DOMLSParser</code> what types of nodes to show to the method <code>DOMLSParserFilter::acceptNode</code>.
     * If a node is not shown to the filter using this attribute, it is automatically included in the DOM document being built.
     * See <code>DOMNodeFilter</code> for definition of the constants. The constants SHOW_ATTRIBUTE, SHOW_DOCUMENT,
     * SHOW_DOCUMENT_TYPE, SHOW_NOTATION, SHOW_ENTITY, and SHOW_DOCUMENT_FRAGMENT are meaningless here.
     * Those nodes will never be passed to DOMLSParserFilter::acceptNode.
     *
     * @return The constants of what types of nodes to show.
     * @since DOM Level 3
     */
    virtual DOMNodeFilter::ShowType getWhatToShow() const = 0;

    //@}
};

XERCES_CPP_NAMESPACE_END

#endif
