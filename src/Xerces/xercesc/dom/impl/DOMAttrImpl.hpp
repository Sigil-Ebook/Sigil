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
 * $Id: DOMAttrImpl.hpp 678709 2008-07-22 10:56:56Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMATTRIMPL_HPP)
#define XERCESC_INCLUDE_GUARD_DOMATTRIMPL_HPP

//
//  This file is part of the internal implementation of the C++ XML DOM.
//  It should NOT be included or used directly by application programs.
//
//  Applications should include the file <xercesc/dom/DOM.hpp> for the entire
//  DOM API, or xercesc/dom/DOM*.hpp for individual DOM classes, where the class
//  name is substituded for the *.
//


#include <xercesc/util/XercesDefs.hpp>
#include "DOMParentNode.hpp"
#include "DOMNodeImpl.hpp"
#include "DOMDocumentImpl.hpp"
#include <xercesc/dom/DOMAttr.hpp>
#include <xercesc/framework/XMLBuffer.hpp>
#include "DOMNodeIDMap.hpp"

XERCES_CPP_NAMESPACE_BEGIN

class DOMElementImpl;
class DOMTypeInfoImpl;

class CDOM_EXPORT DOMAttrImpl: public DOMAttr {

public:
    DOMNodeImpl        fNode;
    DOMParentNode      fParent;
    const XMLCh       *fName;

protected:
    const DOMTypeInfoImpl *fSchemaType;

public:
    DOMAttrImpl(DOMDocument *ownerDocument, const XMLCh *aName);
    DOMAttrImpl(const DOMAttrImpl &other, bool deep=false);
    virtual ~DOMAttrImpl();

public:
     // Add all functions that are pure virtual in DOMNODE
    DOMNODE_FUNCTIONS;

public:
    virtual const XMLCh *       getName() const;
    virtual bool                getSpecified() const;
    virtual const XMLCh *       getValue() const;
    virtual void                setSpecified(bool arg);
    virtual void                setValue(const XMLCh * value);
    virtual DOMElement *        getOwnerElement() const;
    virtual bool                isId() const;
    virtual const DOMTypeInfo*  getSchemaTypeInfo() const;

    void setOwnerElement(DOMElement *ownerElem);    //internal use only

    // helper function for DOM Level 3 renameNode
    virtual DOMNode* rename(const XMLCh* namespaceURI, const XMLCh* name);

    //helper function for DOM Level 3 TypeInfo
    virtual void setSchemaTypeInfo(const DOMTypeInfoImpl* typeInfo);

   // helper method that sets this attr to an idnode and places it into the document map
   virtual void addAttrToIDNodeMap();

   // helper to remove this attr from from the id map if it is in there
   virtual void removeAttrFromIDNodeMap();

public:
    // Set attribute value fast. Assumptions:
    //
    // - node is not read-only
    // - no ID management is performed
    // - this attribute does not have a value
    //
    virtual void setValueFast (const XMLCh * value);

protected:
    void getTextValue(DOMNode* node, XMLBuffer& buf) const;

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    DOMAttrImpl& operator=(const DOMAttrImpl&);
};

inline void DOMAttrImpl::removeAttrFromIDNodeMap()
{
    if (fNode.isIdAttr()) {
        ((DOMDocumentImpl *)fParent.fOwnerDocument)->getNodeIDMap()->remove(this);
        fNode.isIdAttr(false);
    }
}

inline void DOMAttrImpl::addAttrToIDNodeMap()
{
    if (fNode.isIdAttr())
        return;

    fNode.isIdAttr(true);

    // REVIST For now, we don't worry about what happens if the new
    // name conflicts as per setValue
    DOMDocumentImpl *doc = (DOMDocumentImpl *)(fParent.fOwnerDocument);

    if (doc->fNodeIDMap == 0)
        doc->fNodeIDMap = new (doc) DOMNodeIDMap(500, doc);

    doc->getNodeIDMap()->add(this);
}

XERCES_CPP_NAMESPACE_END

#endif
