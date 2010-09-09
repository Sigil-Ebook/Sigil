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
 * $Id: DOMImplementationImpl.hpp 671894 2008-06-26 13:29:21Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMIMPLEMENTATIONIMPL_HPP)
#define XERCESC_INCLUDE_GUARD_DOMIMPLEMENTATIONIMPL_HPP

//
//  This file is part of the internal implementation of the C++ XML DOM.
//  It should NOT be included or used directly by application programs.
//
//  Applications should include the file <xercesc/dom/DOM.hpp> for the entire
//  DOM API, or xercesc/dom/DOM*.hpp for individual DOM classes, where the class
//  name is substituded for the *.
//

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationSource.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XMLMsgLoader;

class DOMImplementationImpl: public XMemory,
                             public DOMImplementation,
                             public DOMImplementationSource
{
private:
    DOMImplementationImpl(const DOMImplementationImpl &);
    DOMImplementationImpl & operator = (const DOMImplementationImpl &);
    friend class XMLInitializer;

protected:
    DOMImplementationImpl() {};

public:
    virtual ~DOMImplementationImpl() {};
    static DOMImplementationImpl*   getDOMImplementationImpl();
    static XMLMsgLoader* getMsgLoader4DOM();

    // ------------------------------------------------------------
    // DOMImplementation Virtual interface
    // ------------------------------------------------------------
    virtual bool                hasFeature(const  XMLCh * feature,  const  XMLCh * version) const;

    // Introduced in DOM Level 2
    virtual DOMDocumentType*    createDocumentType(const XMLCh *qualifiedName,
                                                   const XMLCh * publicId,
                                                   const XMLCh *systemId);
    virtual DOMDocument*        createDocument(const XMLCh *namespaceURI,
                                               const XMLCh *qualifiedName,
                                               DOMDocumentType *doctype,
                                               MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    // DOM Level 3
    virtual void* getFeature(const XMLCh* feature, const XMLCh* version) const;

    // Non-standard extension
    virtual DOMDocument*        createDocument(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    // ------------------------------------------------------------
    // DOMImplementationLS Virtual interface
    // ------------------------------------------------------------
    // Introduced in DOM Level 3
    virtual DOMLSParser* createLSParser(const DOMImplementationLSMode mode,
                                        const XMLCh* const     schemaType,
                                        MemoryManager* const   manager = XMLPlatformUtils::fgMemoryManager,
                                        XMLGrammarPool*  const gramPool = 0);
    virtual DOMLSSerializer* createLSSerializer(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    virtual DOMLSInput* createLSInput(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    virtual DOMLSOutput* createLSOutput(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    // ------------------------------------------------------------
    // DOMImplementationSource Virtual interface
    // ------------------------------------------------------------
    virtual DOMImplementation* getDOMImplementation(const XMLCh* features) const;
    virtual DOMImplementationList* getDOMImplementationList(const XMLCh* features) const;

};


XERCES_CPP_NAMESPACE_END

#endif
