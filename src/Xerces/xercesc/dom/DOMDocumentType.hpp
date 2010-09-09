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
 * $Id: DOMDocumentType.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMDOCUMENTTYPE_HPP)
#define XERCESC_INCLUDE_GUARD_DOMDOCUMENTTYPE_HPP

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/dom/DOMNode.hpp>

XERCES_CPP_NAMESPACE_BEGIN


class DOMNamedNodeMap;

/**
 * Each <code>DOMDocument</code> has a <code>doctype</code> attribute whose value
 * is either <code>null</code> or a <code>DOMDocumentType</code> object. The
 * <code>DOMDocumentType</code> interface in the DOM Core provides an interface
 * to the list of entities that are defined for the document, and little
 * else because the effect of namespaces and the various XML schema efforts
 * on DTD representation are not clearly understood as of this writing.
 * <p>The DOM Level 2 doesn't support editing <code>DOMDocumentType</code> nodes.
 * <p>See also the <a href='http://www.w3.org/TR/2000/REC-DOM-Level-2-Core-20001113'>Document Object Model (DOM) Level 2 Core Specification</a>.
 *
 * @since DOM Level 1
 */
class CDOM_EXPORT DOMDocumentType: public DOMNode {
protected:
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    /** @name Hidden constructors */
    //@{    
    DOMDocumentType() {};
    //@}

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    /** @name Unimplemented constructors and operators */
    //@{
    DOMDocumentType(const DOMDocumentType &);
    DOMDocumentType & operator = (const DOMDocumentType &);
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
    virtual ~DOMDocumentType() {};
    //@}

    // -----------------------------------------------------------------------
    //  Virtual DOMDocumentType interface
    // -----------------------------------------------------------------------
    /** @name Functions introduced in DOM Level 1 */
    //@{
    /**
     * The name of DTD; i.e., the name immediately following the
     * <code>DOCTYPE</code> keyword.
     *
     * @since DOM Level 1
     */
    virtual const XMLCh *       getName() const = 0;

    /**
     * A <code>DOMNamedNodeMap</code> containing the general entities, both
     * external and internal, declared in the DTD. Parameter entities are
     * not contained. Duplicates are discarded. For example in:
     * <code>&lt;!DOCTYPE<br>
     * ex SYSTEM "ex.dtd" [ &lt;!ENTITY foo "foo"&gt; &lt;!ENTITY bar<br>
     * "bar"&gt; &lt;!ENTITY bar "bar2"&gt; &lt;!ENTITY % baz "baz"&gt;<br>
     * ]&gt; &lt;ex/&gt;<br></code>
     *  the interface provides access to <code>foo</code>
     * and the first declaration of <code>bar</code> but not the second
     * declaration of <code>bar</code> or <code>baz</code>. Every node in
     * this map also implements the <code>DOMEntity</code> interface.
     * <br>The DOM Level 2 does not support editing entities, therefore
     * <code>entities</code> cannot be altered in any way.
     *
     * @since DOM Level 1
     */
    virtual DOMNamedNodeMap *getEntities() const = 0;


    /**
     * A <code>DOMNamedNodeMap</code> containing the notations declared in the
     * DTD. Duplicates are discarded. Every node in this map also implements
     * the <code>DOMNotation</code> interface.
     * <br>The DOM Level 2 does not support editing notations, therefore
     * <code>notations</code> cannot be altered in any way.
     *
     * @since DOM Level 1
     */
    virtual DOMNamedNodeMap *getNotations() const = 0;
    //@}

    /** @name Functions introduced in DOM Level 2. */
    //@{
    /**
     * Get the public identifier of the external subset.
     *
     * @return The public identifier of the external subset.
     * @since DOM Level 2
     */
    virtual const XMLCh *     getPublicId() const = 0;

    /**
     * Get the system identifier of the external subset.
     *
     * @return The system identifier of the external subset.
     * @since DOM Level 2
     */
    virtual const XMLCh *     getSystemId() const = 0;

    /**
     * The internal subset as a string, or <code>null</code> if there is none.
     * This is does not contain the delimiting square brackets.The actual
     * content returned depends on how much information is available to the
     * implementation. This may vary depending on various parameters,
     * including the XML processor used to build the document.
     *
     * @return The internal subset as a string.
     * @since DOM Level 2
     */
    virtual const XMLCh *     getInternalSubset() const = 0;
    //@}

};

XERCES_CPP_NAMESPACE_END

#endif


