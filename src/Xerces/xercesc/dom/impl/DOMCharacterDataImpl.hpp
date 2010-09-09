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
 * $Id: DOMCharacterDataImpl.hpp 678709 2008-07-22 10:56:56Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMCHARACTERDATAIMPL_HPP)
#define XERCESC_INCLUDE_GUARD_DOMCHARACTERDATAIMPL_HPP

//
//  This file is part of the internal implementation of the C++ XML DOM.
//  It should NOT be included or used directly by application programs.
//
//  Applications should include the file <xercesc/dom/DOM.hpp> for the entire
//  DOM API, or xercesc/dom/DOM*.hpp for individual DOM classes, where the class
//  name is substituded for the *.
//

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/util/XMLString.hpp>

XERCES_CPP_NAMESPACE_BEGIN


class DOMNode;
class DOMDocument;
class DOMDocumentImpl;
class DOMBuffer;

// Instances of DOMCharacterDataImpl appear as members of node types
//   that implement the DOMCharacterData interfaces.
//   Operations in those classes are delegated to this class.
//
class CDOM_EXPORT DOMCharacterDataImpl
{
public:
    DOMBuffer* fDataBuf;
    // for the buffer bid
    DOMDocumentImpl* fDoc;

public:
                   DOMCharacterDataImpl(DOMDocument *doc, const XMLCh *dat);
                   DOMCharacterDataImpl(DOMDocument *doc, const XMLCh* data, XMLSize_t n);
                   DOMCharacterDataImpl(const DOMCharacterDataImpl &other);
                   ~DOMCharacterDataImpl();
    const          XMLCh * getNodeValue() const;
    void           setNodeValue(const XMLCh * value);
    void           appendData(const DOMNode *node, const  XMLCh *data);
    void           appendData(const DOMNode *node, const  XMLCh *data, XMLSize_t n);
    void           deleteData(const DOMNode *node, XMLSize_t offset, XMLSize_t count);
    const XMLCh*   getData() const;
    XMLSize_t      getLength() const;
    void           insertData(const DOMNode *node, XMLSize_t offset, const XMLCh * data);
    void           replaceData(const DOMNode *node, XMLSize_t offset, XMLSize_t count, const XMLCh * data);
    void           setData(const DOMNode *node, const XMLCh * arg);
    void           setNodeValue(const DOMNode *node, const XMLCh *value);


    const XMLCh*   substringData(const DOMNode *node, XMLSize_t offset, XMLSize_t count) const;
    void           releaseBuffer();

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    DOMCharacterDataImpl & operator = (const DOMCharacterDataImpl &);
};

#define GetDOMCharacterDataImplMemoryManager GET_DIRECT_MM(fDoc)

XERCES_CPP_NAMESPACE_END


#endif
