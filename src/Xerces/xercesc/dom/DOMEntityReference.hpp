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
 * $Id: DOMEntityReference.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMENTITYREFERENCE_HPP)
#define XERCESC_INCLUDE_GUARD_DOMENTITYREFERENCE_HPP

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/dom/DOMNode.hpp>

XERCES_CPP_NAMESPACE_BEGIN


/**
 * <code>DOMEntityReference</code> objects may be inserted into the structure
 * model when an entity reference is in the source document, or when the
 * user wishes to insert an entity reference. Note that character references
 * and references to predefined entities are considered to be expanded by
 * the HTML or XML processor so that characters are represented by their
 * Unicode equivalent rather than by an entity reference. Moreover, the XML
 * processor may completely expand references to entities while building the
 * structure model, instead of providing <code>DOMEntityReference</code>
 * objects. If it does provide such objects, then for a given
 * <code>DOMEntityReference</code> node, it may be that there is no
 * <code>DOMEntity</code> node representing the referenced entity. If such an
 * <code>DOMEntity</code> exists, then the subtree of the
 * <code>DOMEntityReference</code> node is in general a copy of the
 * <code>DOMEntity</code> node subtree. However, this may not be true when an
 * entity contains an unbound namespace prefix. In such a case, because the
 * namespace prefix resolution depends on where the entity reference is, the
 * descendants of the <code>DOMEntityReference</code> node may be bound to
 * different namespace URIs.
 * <p>As for <code>DOMEntity</code> nodes, <code>DOMEntityReference</code> nodes and
 * all their descendants are readonly.
 * <p>See also the <a href='http://www.w3.org/TR/2000/REC-DOM-Level-2-Core-20001113'>Document Object Model (DOM) Level 2 Core Specification</a>.
 *
 * @since DOM Level 1
 */

class CDOM_EXPORT DOMEntityReference: public DOMNode {
protected:
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    /** @name Hidden constructors */
    //@{    
    DOMEntityReference() {}
    DOMEntityReference(const DOMEntityReference &other) : DOMNode(other) {}
    //@}

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    /** @name Unimplemented operators */
    //@{
    DOMEntityReference & operator = (const DOMEntityReference &);
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
    virtual ~DOMEntityReference() {};
    //@}

};

XERCES_CPP_NAMESPACE_END

#endif


