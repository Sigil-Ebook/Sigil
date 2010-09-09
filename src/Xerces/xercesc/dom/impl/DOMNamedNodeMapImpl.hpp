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
 * $Id: DOMNamedNodeMapImpl.hpp 671894 2008-06-26 13:29:21Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMNAMEDNODEMAPIMPL_HPP)
#define XERCESC_INCLUDE_GUARD_DOMNAMEDNODEMAPIMPL_HPP

//
//  This file is part of the internal implementation of the C++ XML DOM.
//  It should NOT be included or used directly by application programs.
//
//  Applications should include the file <xercesc/dom/DOM.hpp> for the entire
//  DOM API, or xercesc/dom/DOM*.hpp for individual DOM classes, where the class
//  name is substituded for the *.
//

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/dom/DOMNamedNodeMap.hpp>

XERCES_CPP_NAMESPACE_BEGIN


class       DOMNodeVector;
class       DOMNode;

#define MAP_SIZE    193

class CDOM_EXPORT DOMNamedNodeMapImpl: public DOMNamedNodeMap {
protected:
    DOMNodeVector*    fBuckets[MAP_SIZE];
    DOMNode*          fOwnerNode;       // the node this map belongs to
    //bool             fReadOnly;     // revisit - flag on owner node instead?

    bool            readOnly();  // revisit.  Look at owner node read-only.

public:
    DOMNamedNodeMapImpl(DOMNode *ownerNode);

    virtual                 ~DOMNamedNodeMapImpl();
    virtual DOMNamedNodeMapImpl *cloneMap(DOMNode *ownerNode);
    virtual void            setReadOnly(bool readOnly, bool deep);

    virtual XMLSize_t       getLength() const;
    virtual DOMNode*        item(XMLSize_t index) const;
    virtual DOMNode*        getNamedItem(const XMLCh *name) const;
    virtual DOMNode*        setNamedItem(DOMNode *arg);
    virtual DOMNode*        removeNamedItem(const XMLCh *name);

    //Introduced in DOM Level 2
    virtual DOMNode*        getNamedItemNS(const XMLCh *namespaceURI,
	                                        const XMLCh *localName) const;
    virtual DOMNode*        setNamedItemNS(DOMNode *arg);
    virtual DOMNode*        removeNamedItemNS(const XMLCh *namespaceURI,
	                                           const XMLCh *localName);
private:
    // unimplemented
    DOMNamedNodeMapImpl(const DOMNamedNodeMapImpl &);
    DOMNamedNodeMapImpl & operator = (const DOMNamedNodeMapImpl &);
};

XERCES_CPP_NAMESPACE_END

#endif
