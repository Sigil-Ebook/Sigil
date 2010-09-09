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
 * $Id: DOMEntityReferenceImpl.hpp 641193 2008-03-26 08:06:57Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMENTITYREFERENCEIMPL_HPP)
#define XERCESC_INCLUDE_GUARD_DOMENTITYREFERENCEIMPL_HPP

//
//  This file is part of the internal implementation of the C++ XML DOM.
//  It should NOT be included or used directly by application programs.
//
//  Applications should include the file <xercesc/dom/DOM.hpp> for the entire
//  DOM API, or xercesc/dom/DOM*.hpp for individual DOM classes, where the class
//  name is substituded for the *.
//

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/dom/DOMEntityReference.hpp>

#include "DOMParentNode.hpp"
#include "DOMChildNode.hpp"
#include "DOMNodeImpl.hpp"

XERCES_CPP_NAMESPACE_BEGIN


class CDOM_EXPORT DOMEntityReferenceImpl: public DOMEntityReference
{
protected:
    DOMNodeImpl      fNode;
    DOMParentNode    fParent;
    DOMChildNode     fChild;

    const XMLCh    *fName;
    const XMLCh    *fBaseURI;

    friend class XercesDOMParser;

public:
    DOMEntityReferenceImpl(DOMDocument *ownerDoc, const XMLCh *entityName);
    DOMEntityReferenceImpl(DOMDocument *ownerDoc, const XMLCh *entityName, bool cloneChild);
    DOMEntityReferenceImpl(const DOMEntityReferenceImpl &other, bool deep=false);
    virtual ~DOMEntityReferenceImpl();

public:
    // Declare all of the functions from DOMNode.
    DOMNODE_FUNCTIONS;
    
public:
    virtual void setReadOnly(bool readOnly,bool deep);

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------    
    DOMEntityReferenceImpl & operator = (const DOMEntityReferenceImpl &);
};

XERCES_CPP_NAMESPACE_END

#endif

