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
 * $Id: DOMNormalizer.hpp 676911 2008-07-15 13:27:32Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMNORMALIZER_HPP)
#define XERCESC_INCLUDE_GUARD_DOMNORMALIZER_HPP

//
//  This file is part of the internal implementation of the C++ XML DOM.
//  It should NOT be included or used directly by application programs.
//
//  Applications should include the file <xercesc/dom/DOM.hpp> for the entire
//  DOM API, or xercesc/dom/DOM*.hpp for individual DOM classes, where the class
//  name is substituded for the *.
//

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/util/RefHashTableOf.hpp>
#include <xercesc/util/RefVectorOf.hpp>
#include <xercesc/framework/XMLErrorCodes.hpp>


XERCES_CPP_NAMESPACE_BEGIN

class DOMConfigurationImpl;
class DOMErrorHandler;
class DOMDocumentImpl;
class DOMNode;
class DOMElementImpl;
class DOMAttr;
class DOMNamedNodeMap;

class DOMNormalizer : public XMemory {

    //the following are the data structures maintain the stack of namespace information
    class InScopeNamespaces : public XMemory {
        class Scope : public XMemory {
        public:
            Scope(Scope *baseScopeWithBindings);
            ~Scope();
            void addOrChangeBinding(const XMLCh *prefix, const XMLCh *uri,
                                    MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
            const XMLCh* getUri(const XMLCh *prefix) const;
            const XMLCh* getPrefix(const XMLCh* uri) const;
            Scope *fBaseScopeWithBindings;

        private:
            RefHashTableOf<XMLCh> *fPrefixHash;
            RefHashTableOf<XMLCh> *fUriHash;
            // unimplemented
            Scope ( const Scope& toCopy);
            Scope& operator= (const Scope& other);
        };

    public:
        InScopeNamespaces(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
        ~InScopeNamespaces();
        void addOrChangeBinding(const XMLCh *prefix, const XMLCh *uri,
                                MemoryManager* const manager  = XMLPlatformUtils::fgMemoryManager);
        void addScope(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
        void removeScope();
        bool isValidBinding(const XMLCh* prefix, const XMLCh* uri) const;
        const XMLCh* getOrDeclarePrefix(const XMLCh* uri);
        const XMLCh* getPrefix(const XMLCh* uri) const;
        const XMLCh* getUri(const XMLCh* prefix) const;
        XMLSize_t size();

    private:
        RefVectorOf<Scope> *fScopes;
        Scope *lastScopeWithBindings;
        // unimplemented
        InScopeNamespaces ( const InScopeNamespaces& toCopy);
        InScopeNamespaces& operator= (const InScopeNamespaces& other);
    };

public:
    DOMNormalizer(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    ~DOMNormalizer();

    /**
     * Main entry method to normalize a document
     */
    void normalizeDocument(DOMDocumentImpl *doc);

private:
    // unimplemented
    DOMNormalizer ( const DOMNormalizer& toCopy);
    DOMNormalizer& operator= (const DOMNormalizer& other);

protected:
    /**
     * Recursively normalizes a node
     */
    DOMNode * normalizeNode(DOMNode *node) const;

    /**
     * Helper method that fixes up the namespace declarations according to the
     * DOM Level 3 psydocode
     */
    void namespaceFixUp(DOMElementImpl *ele) const;

    /**
     * Converts an integer to an XMLCh - max 15 digits long.
     */
    const XMLCh * integerToXMLCh(unsigned int i) const;

    /**
     * Adds a namespace attribute or replaces the value of existing namespace
     * attribute with the given prefix and value for URI.
     * In case prefix is empty will add/update default namespace declaration.
     */
    void addOrChangeNamespaceDecl(const XMLCh* prefix, const XMLCh* uri, DOMElementImpl *element) const;

    /**
     * Adds a custom namespace in the form "NSx" where x is an integer that
     * has not yet used in the document
     */
    const XMLCh* addCustomNamespaceDecl(const XMLCh* uri, DOMElementImpl *element) const;


    /**
     * Report an error
     */
    void error(const XMLErrs::Codes code, const DOMNode *node) const;

    //
    // fDocument - the document we are operating on
    //
    // fDOMConfiguration - the configuration from the document
    //
    // fErrorHandler - the errorhandler to be used when reporting errors during normalization
    //
    // fNSScope - the data stucture that holds the prefix-uri information
    //
    // fNewNamespaceCount - the number of custom namespace declarations we have created
    //
    DOMDocumentImpl *fDocument;
    DOMConfigurationImpl *fConfiguration;
    DOMErrorHandler *fErrorHandler;
    InScopeNamespaces *fNSScope;
    unsigned int fNewNamespaceCount;
    MemoryManager* fMemoryManager;
};



XERCES_CPP_NAMESPACE_END

#endif
