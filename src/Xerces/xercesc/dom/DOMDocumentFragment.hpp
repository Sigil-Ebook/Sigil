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
 * $Id: DOMDocumentFragment.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMDOCUMENTFRAGMENT_HPP)
#define XERCESC_INCLUDE_GUARD_DOMDOCUMENTFRAGMENT_HPP

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/dom/DOMNode.hpp>

XERCES_CPP_NAMESPACE_BEGIN


/**
 * DOMDocumentFragment is a "lightweight" or "minimal"
 * DOMDocument object.
 *
 * It is very common to want to be able to
 * extract a portion of a document's tree or to create a new fragment of a
 * document. Imagine implementing a user command like cut or rearranging a
 * document by moving fragments around. It is desirable to have an object
 * which can hold such fragments and it is quite natural to use a DOMNode for
 * this purpose. While it is true that a <code>DOMDocument</code> object could
 * fulfill this role,  a <code>DOMDocument</code> object can potentially be a
 * heavyweight  object, depending on the underlying implementation. What is
 * really needed for this is a very lightweight object.
 * <code>DOMDocumentFragment</code> is such an object.
 * <p>Furthermore, various operations -- such as inserting nodes as children
 * of another <code>DOMNode</code> -- may take <code>DOMDocumentFragment</code>
 * objects as arguments;  this results in all the child nodes of the
 * <code>DOMDocumentFragment</code>  being moved to the child list of this node.
 * <p>The children of a <code>DOMDocumentFragment</code> node are zero or more
 * nodes representing the tops of any sub-trees defining the structure of the
 * document. <code>DOMDocumentFragment</code> nodes do not need to be
 * well-formed XML documents (although they do need to follow the rules
 * imposed upon well-formed XML parsed entities, which can have multiple top
 * nodes).  For example, a <code>DOMDocumentFragment</code> might have only one
 * child and that child node could be a <code>DOMText</code> node. Such a
 * structure model  represents neither an HTML document nor a well-formed XML
 * document.
 * <p>When a <code>DOMDocumentFragment</code> is inserted into a
 * <code>DOMDocument</code> (or indeed any other <code>DOMNode</code> that may take
 * children) the children of the <code>DOMDocumentFragment</code> and not the
 * <code>DOMDocumentFragment</code>  itself are inserted into the
 * <code>DOMNode</code>. This makes the <code>DOMDocumentFragment</code> very
 * useful when the user wishes to create nodes that are siblings; the
 * <code>DOMDocumentFragment</code> acts as the parent of these nodes so that the
 *  user can use the standard methods from the <code>DOMNode</code>  interface,
 * such as <code>insertBefore()</code> and  <code>appendChild()</code>.
 *
 * @since DOM Level 1
 */

class CDOM_EXPORT DOMDocumentFragment: public DOMNode {
protected:
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    /** @name Hidden constructors */
    //@{    
    DOMDocumentFragment() {};
    //@}

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    /** @name Unimplemented constructors and operators */
    //@{
    DOMDocumentFragment(const DOMDocumentFragment &);
    DOMDocumentFragment & operator = (const DOMDocumentFragment &);
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
    virtual ~DOMDocumentFragment() {};
	//@}

};

XERCES_CPP_NAMESPACE_END

#endif
