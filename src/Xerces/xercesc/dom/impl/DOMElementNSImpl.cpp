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
 * $Id: DOMElementNSImpl.cpp 678709 2008-07-22 10:56:56Z borisk $
 */

#include <xercesc/util/XMLUniDefs.hpp>
#include "DOMElementNSImpl.hpp"
#include "DOMDocumentImpl.hpp"
#include "DOMTypeInfoImpl.hpp"
#include "DOMCasts.hpp"
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/util/XMLUri.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

DOMElementNSImpl::DOMElementNSImpl(DOMDocument *ownerDoc, const XMLCh *nam) :
    DOMElementImpl(ownerDoc, nam)
{
    this->fNamespaceURI=0;	  //DOM Level 2
    this->fLocalName=0;       //DOM Level 2
    this->fPrefix=0;
    this->fSchemaType = 0;
}

//Introduced in DOM Level 2
DOMElementNSImpl::DOMElementNSImpl(DOMDocument *ownerDoc,
                                   const XMLCh *namespaceURI,
                                   const XMLCh *qualifiedName) :
    DOMElementImpl(ownerDoc, qualifiedName)
{
  setName(namespaceURI, qualifiedName);
  this->fSchemaType = 0;
}

DOMElementNSImpl::DOMElementNSImpl(DOMDocument *ownerDoc,
                                   const XMLCh *namespaceURI,
                                   const XMLCh *prefix,
                                   const XMLCh *localName,
                                   const XMLCh *qualifiedName)
    : DOMElementImpl(ownerDoc, qualifiedName)
{
  this->fSchemaType = 0;

  DOMDocumentImpl* docImpl = (DOMDocumentImpl*)fParent.fOwnerDocument;

  if (prefix == 0 || *prefix == 0)
  {
    fPrefix = 0;
    fLocalName = fName;
  }
  else
  {
    fPrefix = docImpl->getPooledString(prefix);
    fLocalName = docImpl->getPooledString(localName);
  }

  // DOM Level 3: namespace URI is never empty string.
  //
  const XMLCh * URI = DOMNodeImpl::mapPrefix (
    fPrefix,
    (!namespaceURI || !*namespaceURI) ? 0 : namespaceURI,
    DOMNode::ELEMENT_NODE);

  fNamespaceURI = (URI == 0) ? 0 : docImpl->getPooledString(URI);
}

DOMElementNSImpl::DOMElementNSImpl(const DOMElementNSImpl &other, bool deep) :
    DOMElementImpl(other, deep)
{
    this->fNamespaceURI = other.fNamespaceURI;	        //DOM Level 2
    this->fLocalName = other.fLocalName;                //DOM Level 2
    this->fPrefix = other.fPrefix;
    this->fSchemaType = other.fSchemaType;
}

DOMNode * DOMElementNSImpl::cloneNode(bool deep) const {
    DOMNode* newNode = new (fParent.fOwnerDocument, DOMMemoryManager::ELEMENT_NS_OBJECT) DOMElementNSImpl(*this, deep);
    fNode.callUserDataHandlers(DOMUserDataHandler::NODE_CLONED, this, newNode);
    return newNode;
}

const XMLCh * DOMElementNSImpl::getNamespaceURI() const
{
    return fNamespaceURI;
}

const XMLCh * DOMElementNSImpl::getPrefix() const
{
    return fPrefix;
}


const XMLCh * DOMElementNSImpl::getLocalName() const
{
    return fLocalName;
}

void DOMElementNSImpl::setPrefix(const XMLCh *prefix)
{
    if (fNode.isReadOnly())
        throw DOMException(DOMException::NO_MODIFICATION_ALLOWED_ERR, 0, GetDOMNodeMemoryManager);
    if (fNamespaceURI == 0 || fNamespaceURI[0] == chNull)
        throw DOMException(DOMException::NAMESPACE_ERR, 0, GetDOMNodeMemoryManager);

    if (prefix == 0 || *prefix == 0) {
        fPrefix = 0;
        fName = fLocalName;
        return;
    }

    DOMDocumentImpl* doc = (DOMDocumentImpl*) fParent.fOwnerDocument;

    if(!doc->isXMLName(prefix))
        throw DOMException(DOMException::INVALID_CHARACTER_ERR,0, GetDOMNodeMemoryManager);

    const XMLCh * xml      = DOMNodeImpl::getXmlString();
    const XMLCh * xmlURI   = DOMNodeImpl::getXmlURIString();

    if (XMLString::equals(prefix, xml) &&
        !XMLString::equals(fNamespaceURI, xmlURI))
        throw DOMException(DOMException::NAMESPACE_ERR, 0, GetDOMNodeMemoryManager);


    if (XMLString::indexOf(prefix, chColon) != -1) {
        throw DOMException(DOMException::NAMESPACE_ERR, 0, GetDOMNodeMemoryManager);
    }

    this-> fPrefix = doc->getPooledString(prefix);

    XMLSize_t prefixLen = XMLString::stringLen(prefix);
    XMLSize_t newQualifiedNameLen = prefixLen+1+XMLString::stringLen(fLocalName);

    XMLCh *newName;
    XMLCh temp[256];
    if (newQualifiedNameLen >= 255)
      newName = (XMLCh*) doc->getMemoryManager()->allocate
        (
            newQualifiedNameLen * sizeof(XMLCh)
        );//new XMLCh[newQualifiedNameLen];
    else
        newName = temp;

    // newName = prefix + chColon + fLocalName;
    XMLString::copyString(newName, prefix);
    newName[prefixLen] = chColon;
    XMLString::copyString(&newName[prefixLen+1], fLocalName);

    fName = doc->getPooledString(newName);

    if (newQualifiedNameLen >= 255)
        doc->getMemoryManager()->deallocate(newName);//delete[] newName;

}

void DOMElementNSImpl::release()
{
    if (fNode.isOwned() && !fNode.isToBeReleased())
        throw DOMException(DOMException::INVALID_ACCESS_ERR,0, GetDOMNodeMemoryManager);

    DOMDocumentImpl* doc = (DOMDocumentImpl*) fParent.fOwnerDocument;
    if (doc) {
        fNode.callUserDataHandlers(DOMUserDataHandler::NODE_DELETED, 0, 0);
        fParent.release();
        doc->release(this, DOMMemoryManager::ELEMENT_NS_OBJECT);
    }
    else {
        // shouldn't reach here
        throw DOMException(DOMException::INVALID_ACCESS_ERR,0, GetDOMNodeMemoryManager);
    }
}

DOMNode* DOMElementNSImpl::rename(const XMLCh* namespaceURI, const XMLCh* name)
{
    setName(namespaceURI, name);
    fAttributes->reconcileDefaultAttributes(getDefaultAttributes());
    // and fire user data NODE_RENAMED event
    castToNodeImpl(this)->callUserDataHandlers(DOMUserDataHandler::NODE_RENAMED, this, this);

    return this;
}

void DOMElementNSImpl::setName(const XMLCh *namespaceURI,
                               const XMLCh *qualifiedName)
{
    DOMDocumentImpl* ownerDoc = (DOMDocumentImpl *) fParent.fOwnerDocument;
    this->fName = ownerDoc->getPooledString(qualifiedName);

    int index = DOMDocumentImpl::indexofQualifiedName(qualifiedName);
    if (index < 0)
        throw DOMException(DOMException::NAMESPACE_ERR, 0, GetDOMNodeMemoryManager);

    if (index == 0)
    {
        //qualifiedName contains no ':'
        //
        fPrefix = 0;
        fLocalName = fName;
    }
    else
    {	//0 < index < this->name.length()-1
        //
        fPrefix = ownerDoc->getPooledNString(qualifiedName, index);
        fLocalName = ownerDoc->getPooledString(fName+index+1);

        // Before we carry on, we should check if the prefix or localName are valid XMLName
        if (!ownerDoc->isXMLName(fPrefix) || !ownerDoc->isXMLName(fLocalName))
          throw DOMException(DOMException::NAMESPACE_ERR, 0, GetDOMNodeMemoryManager);
    }

    // DOM Level 3: namespace URI is never empty string.
    //
    const XMLCh * URI = DOMNodeImpl::mapPrefix (
      fPrefix,
      (!namespaceURI || !*namespaceURI) ? 0 : namespaceURI,
      DOMNode::ELEMENT_NODE);

    fNamespaceURI = (URI == 0) ? 0 : ownerDoc->getPooledString(URI);
}

const DOMTypeInfo *DOMElementNSImpl::getSchemaTypeInfo() const
{
    if(!fSchemaType)
        return &DOMTypeInfoImpl::g_DtdValidatedElement;
    return fSchemaType;
}

void DOMElementNSImpl::setSchemaTypeInfo(const DOMTypeInfoImpl* typeInfo)
{
    fSchemaType = typeInfo;
}

bool DOMElementNSImpl::isSupported(const XMLCh *feature, const XMLCh *version) const
{
    // check for '+DOMPSVITypeInfo'
    if(feature && *feature=='+' && XMLString::equals(feature+1, XMLUni::fgXercescInterfacePSVITypeInfo))
        return true;
    return fNode.isSupported (feature, version);
}

void* DOMElementNSImpl::getFeature(const XMLCh* feature, const XMLCh* version) const
{
    if(XMLString::equals(feature, XMLUni::fgXercescInterfacePSVITypeInfo))
        return (DOMPSVITypeInfo*)fSchemaType;
    return DOMElementImpl::getFeature(feature, version);
}

XERCES_CPP_NAMESPACE_END
