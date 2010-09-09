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
 * $Id: DOMChildNode.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMCHILDNODE_HPP)
#define XERCESC_INCLUDE_GUARD_DOMCHILDNODE_HPP

//
//  This file is part of the internal implementation of the C++ XML DOM.
//  It should NOT be included or used directly by application programs.
//
//  Applications should include the file <xercesc/dom/DOM.hpp> for the entire
//  DOM API, or xercesc/dom/DOM*.hpp for individual DOM classes, where the class
//  name is substituded for the *.
//

/**
 * ChildNode adds to NodeImpl the capability of being a child, this is having
 * siblings.
 **/

#include <xercesc/util/XercesDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN


class DOMDocument;
class DOMNode;


class CDOM_EXPORT DOMChildNode {

public:
    DOMNode                *previousSibling;
    DOMNode                *nextSibling;

    DOMChildNode();
    DOMChildNode(const DOMChildNode &other);
    ~DOMChildNode();

    DOMNode * getNextSibling() const;
    DOMNode * getParentNode(const DOMNode *thisNode) const;
    DOMNode * getPreviousSibling(const DOMNode *thisNode) const;

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    DOMChildNode & operator = (const DOMChildNode &);   
};


XERCES_CPP_NAMESPACE_END

#endif
