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
 * $Id: DOMNodeListImpl.hpp 676911 2008-07-15 13:27:32Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMNODELISTIMPL_HPP)
#define XERCESC_INCLUDE_GUARD_DOMNODELISTIMPL_HPP

//
//  This file is part of the internal implementation of the C++ XML DOM.
//  It should NOT be included or used directly by application programs.
//
//  Applications should include the file <xercesc/dom/DOM.hpp> for the entire
//  DOM API, or xercesc/dom/DOM*.hpp for individual DOM classes, where the class
//  name is substituded for the *.
//


//  NodeList implementation class -
//     This is for NodeLists returned by GetChildNodes only, not for
//     node lists returned by GetElementsByTagName
//
//     Every node type capable of having children has (as an embedded member)
//     an instance of this class.  To hold down the size overhead on each node, a
//     cache of extended data for active node lists is maintained
//     separately.
//

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/dom/DOMNodeList.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class DOMParentNode;
class DOMNode;

class CDOM_EXPORT DOMNodeListImpl: public DOMNodeList
{
protected:
    DOMParentNode   *fNode;

private:
    // Unused, and unimplemented constructors, operators, etc.
    DOMNodeListImpl();
    DOMNodeListImpl(const DOMNodeListImpl & other);
    DOMNodeListImpl & operator = (const DOMNodeListImpl & other);

public:
    DOMNodeListImpl(DOMParentNode *node);
    virtual             ~DOMNodeListImpl();
    virtual DOMNode *    item(XMLSize_t index) const;
    virtual XMLSize_t    getLength() const;
};

XERCES_CPP_NAMESPACE_END

#endif
