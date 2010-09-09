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
 * $Id: DOMDocumentFragmentImpl.hpp 641193 2008-03-26 08:06:57Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMDOCUMENTFRAGMENTIMPL_HPP)
#define XERCESC_INCLUDE_GUARD_DOMDOCUMENTFRAGMENTIMPL_HPP

//
//  This file is part of the internal implementation of the C++ XML DOM.
//  It should NOT be included or used directly by application programs.
//
//  Applications should include the file <xercesc/dom/DOM.hpp> for the entire
//  DOM API, or xercesc/dom/DOM*.hpp for individual DOM classes, where the class
//  name is substituded for the *.
//

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/dom/DOMDocumentFragment.hpp>
#include "DOMParentNode.hpp"
#include "DOMNodeImpl.hpp"

XERCES_CPP_NAMESPACE_BEGIN


class CDOM_EXPORT DOMDocumentFragmentImpl: public DOMDocumentFragment {
protected:
    DOMNodeImpl     fNode;
    DOMParentNode   fParent;

protected:
    DOMDocumentFragmentImpl(DOMDocument *);
    DOMDocumentFragmentImpl(const DOMDocumentFragmentImpl &other, bool deep);
    friend class DOMDocumentImpl;

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    DOMDocumentFragmentImpl & operator = (const DOMDocumentFragmentImpl &);

public:
    virtual ~DOMDocumentFragmentImpl();

public:
    // Declare all of the functions from DOMNode.
    DOMNODE_FUNCTIONS;
};

XERCES_CPP_NAMESPACE_END

#endif

