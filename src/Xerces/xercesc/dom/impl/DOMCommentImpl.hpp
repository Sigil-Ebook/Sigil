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
 * $Id: DOMCommentImpl.hpp 676911 2008-07-15 13:27:32Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMCOMMENTIMPL_HPP)
#define XERCESC_INCLUDE_GUARD_DOMCOMMENTIMPL_HPP

//
//  This file is part of the internal implementation of the C++ XML DOM.
//  It should NOT be included or used directly by application programs.
//
//  Applications should include the file <xercesc/dom/DOM.hpp> for the entire
//  DOM API, or xercesc/dom/DOM*.hpp for individual DOM classes, where the class
//  name is substituded for the *.
//


#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/dom/DOMComment.hpp>

#include "DOMNodeImpl.hpp"
#include "DOMChildNode.hpp"
#include "DOMCharacterDataImpl.hpp"

XERCES_CPP_NAMESPACE_BEGIN


class CDOM_EXPORT DOMCommentImpl: public DOMComment {
public:
    DOMNodeImpl            fNode;
    DOMChildNode           fChild;
    DOMCharacterDataImpl   fCharacterData;

public:
    DOMCommentImpl(DOMDocument *, const XMLCh *);
    DOMCommentImpl(const DOMCommentImpl &other, bool deep);
    virtual ~DOMCommentImpl();

public:
    // Declare all of the functions from DOMNode.
    DOMNODE_FUNCTIONS;

public:
    // Functions from DOMCharacterData
    virtual void          appendData(const  XMLCh *data);
    virtual void          deleteData(XMLSize_t offset, XMLSize_t count);
    virtual const XMLCh * getData() const;
    virtual XMLSize_t     getLength() const;
    virtual void          insertData(XMLSize_t offset, const XMLCh * data);
    virtual void          replaceData(XMLSize_t offset, XMLSize_t count, const XMLCh * data);
    virtual void          setData(const XMLCh * arg);
    virtual const XMLCh * substringData(XMLSize_t offset, XMLSize_t count) const;

    // Non standard extension for the range to work
    DOMComment* splitText(XMLSize_t offset);

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    DOMCommentImpl & operator = (const DOMCommentImpl &);
};

XERCES_CPP_NAMESPACE_END

#endif

