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
 * $Id: XSDElementNSImpl.hpp 672232 2008-06-27 10:16:38Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XSDELEMENTNSIMPL_HPP)
#define XERCESC_INCLUDE_GUARD_XSDELEMENTNSIMPL_HPP

//
//  This file is part of the internal implementation of the C++ XML DOM.
//  It is used by TraverseSchema to store line/column information.
//  It should NOT be included or used directly by application programs.
//
//  Applications should include the file <xercesc/dom/DOM.hpp> for the entire
//  DOM API, or xercesc/dom/DOM*.hpp for individual DOM classes, where the class
//  name is substituded for the *.
//


#include "DOMElementNSImpl.hpp"

XERCES_CPP_NAMESPACE_BEGIN



class CDOM_EXPORT XSDElementNSImpl: public DOMElementNSImpl {
protected:
    XMLFileLoc fLineNo;     //Line number
    XMLFileLoc fColumnNo;   //Column number


public:
    XSDElementNSImpl(DOMDocument *ownerDoc, const XMLCh *name);
    XSDElementNSImpl(DOMDocument *ownerDoc, //DOM Level 2
	                 const XMLCh *namespaceURI,
                     const XMLCh *qualifiedName,
                     const XMLFileLoc lineNo,
                     const XMLFileLoc columnNo);
    XSDElementNSImpl(const XSDElementNSImpl &other, bool deep=false);

    virtual DOMNode * cloneNode(bool deep) const;

    XMLFileLoc getLineNo() const   { return fLineNo;   }
    XMLFileLoc getColumnNo() const { return fColumnNo; }

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XSDElementNSImpl& operator=(const XSDElementNSImpl&);
};

XERCES_CPP_NAMESPACE_END

#endif
