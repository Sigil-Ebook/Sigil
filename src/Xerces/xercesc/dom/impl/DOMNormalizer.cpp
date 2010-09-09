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

#include <xercesc/dom/DOMAttr.hpp>
#include <xercesc/dom/DOMNode.hpp>
#include <xercesc/dom/DOMErrorHandler.hpp>
#include <xercesc/dom/DOMError.hpp>
#include <xercesc/dom/DOMText.hpp>
#include <xercesc/framework/XMLBuffer.hpp>

#include <xercesc/util/Mutexes.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLInitializer.hpp>
#include <xercesc/util/XMLMsgLoader.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/util/XMLUniDefs.hpp>

#include "DOMConfigurationImpl.hpp"
#include "DOMDocumentImpl.hpp"
#include "DOMElementImpl.hpp"
#include "DOMErrorImpl.hpp"
#include "DOMEntityReferenceImpl.hpp"
#include "DOMNormalizer.hpp"
#include "DOMTextImpl.hpp"

XERCES_CPP_NAMESPACE_BEGIN

static XMLMsgLoader*   gMsgLoader = 0;

void XMLInitializer::initializeDOMNormalizer()
{
    gMsgLoader = XMLPlatformUtils::loadMsgSet(XMLUni::fgXMLErrDomain);

    if (!gMsgLoader)
      XMLPlatformUtils::panic(PanicHandler::Panic_CantLoadMsgDomain);
}

void XMLInitializer::terminateDOMNormalizer()
{
    delete gMsgLoader;
    gMsgLoader = 0;
}

//
//
DOMNormalizer::DOMNormalizer(MemoryManager* const manager)
    : fDocument(0)
    , fConfiguration(0)
    , fErrorHandler(0)
    , fNSScope(0)
    , fNewNamespaceCount(1)
    , fMemoryManager(manager)
{
    fNSScope = new (fMemoryManager) InScopeNamespaces(fMemoryManager);
}

DOMNormalizer::~DOMNormalizer() {
    delete fNSScope;
}

void DOMNormalizer::normalizeDocument(DOMDocumentImpl *doc) {

    fDocument = doc;
    fConfiguration = (DOMConfigurationImpl*)doc->getDOMConfig();
    DOMConfigurationImpl *dci = (DOMConfigurationImpl*)fDocument->getDOMConfig();
    if(dci)
        fErrorHandler = dci->getErrorHandler();
    else
        fErrorHandler = 0;

    DOMNode *child = 0;
    DOMNode *next = 0;
    ((DOMNormalizer *)this)->fNewNamespaceCount = 1;

    for(child = doc->getFirstChild();child != 0; child = next) {
        next = child->getNextSibling();
        child = normalizeNode(child);
        if(child != 0) {
            next = child;
        }
    }
}

DOMNode * DOMNormalizer::normalizeNode(DOMNode *node) const {
    switch(node->getNodeType()) {
    case DOMNode::ELEMENT_NODE: {
        fNSScope->addScope(fMemoryManager);
        DOMNamedNodeMap *attrMap = node->getAttributes();

        if(fConfiguration->featureValues & DOMConfigurationImpl::FEATURE_NAMESPACES) {
            namespaceFixUp((DOMElementImpl*)node);
        }
        else {
            //this is done in namespace fixup so no need to do it if namespace is on
            if(attrMap) {
                for(XMLSize_t i = 0; i < attrMap->getLength(); i++) {
                    attrMap->item(i)->normalize();
                }
            }
        }

        DOMNode *child = node->getFirstChild();
        DOMNode *next = 0;
        for (; child != 0; child = next) {
            next = child->getNextSibling();
            child = normalizeNode(child);
            if(child != 0) {
                next = child;
            }
        }
        fNSScope->removeScope();
        break;
    }
    case DOMNode::COMMENT_NODE: {
        if (!(fConfiguration->featureValues & DOMConfigurationImpl::FEATURE_COMMENTS)) {
            DOMNode *prevSibling = node->getPreviousSibling();
            DOMNode *parent = node->getParentNode();
            // remove the comment node
            parent->removeChild(node);
            if (prevSibling != 0 && prevSibling->getNodeType() == DOMNode::TEXT_NODE) {
                DOMNode *nextSibling = prevSibling->getNextSibling();
                if (nextSibling != 0 && nextSibling->getNodeType() == DOMNode::TEXT_NODE) {
                    ((DOMTextImpl*)nextSibling)->insertData(0, prevSibling->getNodeValue());
                    parent->removeChild(prevSibling);
                    return nextSibling;
                }
            }
        }
        break;
    }
    case DOMNode::CDATA_SECTION_NODE: {
        if (!(fConfiguration->featureValues & DOMConfigurationImpl::FEATURE_CDATA_SECTIONS)) {
            // convert CDATA to TEXT nodes
            DOMText *text = fDocument->createTextNode(node->getNodeValue());
            DOMNode *parent = node->getParentNode();
            DOMNode *prevSibling = node->getPreviousSibling();
            node = parent->replaceChild(text, node);
            if (prevSibling != 0 && prevSibling->getNodeType() == DOMNode::TEXT_NODE) {
                text->insertData(0, prevSibling->getNodeValue());
                parent->removeChild(prevSibling);
            }
            return text; // Don't advance;
        }
        break;
    }
    case DOMNode::TEXT_NODE: {
        DOMNode *next = node->getNextSibling();

        if(next != 0 && next->getNodeType() == DOMNode::TEXT_NODE) {
            ((DOMText*)node)->appendData(next->getNodeValue());
            node->getParentNode()->removeChild(next);
            return node;
        } else {
            const XMLCh* nv = node->getNodeValue();
            if (nv == 0 || *nv == 0) {
                node->getParentNode()->removeChild(node);
            }
        }
    }
    default:
        break;
    }

    return 0;
}


void DOMNormalizer::namespaceFixUp(DOMElementImpl *ele) const {
    DOMAttrMapImpl *attrMap = ele->fAttributes;

    XMLSize_t len = attrMap->getLength();
    //get the ns info from the attrs
    for(XMLSize_t i = 0; i < len; i++) {
        DOMAttr *at = (DOMAttr*)attrMap->item(i);

        //normalize the attr whatever happens
        at->normalize();

        const XMLCh *uri = at->getNamespaceURI();
        const XMLCh *value = at->getNodeValue();

        if(XMLString::equals(XMLUni::fgXMLNSURIName, uri)) {
            if(XMLString::equals(XMLUni::fgXMLNSURIName, value)) {
                error(XMLErrs::NSDeclInvalid, ele);
            }
            else {
                const XMLCh *prefix = at->getPrefix();

                if(XMLString::equals(prefix, XMLUni::fgXMLNSString)) {
                    fNSScope->addOrChangeBinding(at->getLocalName(), value, fMemoryManager);
                }
                else {
                    fNSScope->addOrChangeBinding(XMLUni::fgZeroLenString, value, fMemoryManager);
                }
            }
        }
    }

    const XMLCh* prefix = ele->getPrefix();
    prefix ? prefix : prefix = XMLUni::fgZeroLenString;
    const XMLCh* uri = ele->getNamespaceURI();
    uri ? uri : uri = XMLUni::fgZeroLenString;

    if(!XMLString::equals(uri, XMLUni::fgZeroLenString)) {
        if(!fNSScope->isValidBinding(prefix, uri)) {
            addOrChangeNamespaceDecl(prefix, uri, ele);
            fNSScope->addOrChangeBinding(prefix, uri, fMemoryManager);
        }
    }
    else {
        if(ele->getLocalName() == 0) {
            error(XMLErrs::DOMLevel1Node, ele);
        }
        else if(!fNSScope->isValidBinding(XMLUni::fgZeroLenString, XMLUni::fgZeroLenString)) {
            addOrChangeNamespaceDecl(XMLUni::fgZeroLenString, XMLUni::fgZeroLenString, ele);
            fNSScope->addOrChangeBinding(XMLUni::fgZeroLenString, XMLUni::fgZeroLenString, fMemoryManager);
        }
    }

    //fix up non ns attrs
    len = attrMap->getLength();

    // hp aCC complains this i is a redefinition of the i on line 283
    for(XMLSize_t j = 0; j < len; j++) {
        DOMAttr *at = (DOMAttr*)attrMap->item(j);
        const XMLCh *uri = at->getNamespaceURI();
        const XMLCh* prefix = at->getPrefix();

        if(!XMLString::equals(XMLUni::fgXMLNSURIName, uri)) {
            if(uri != 0) {
                if(prefix == 0 || !fNSScope->isValidBinding(prefix, uri)) {

                    const XMLCh* newPrefix =  fNSScope->getPrefix(uri);

                    if(newPrefix != 0) {
                        at->setPrefix(newPrefix);
                    }
                    else {
                        if(prefix != 0 && !fNSScope->getUri(prefix)) {
                            fNSScope->addOrChangeBinding(prefix, uri, fMemoryManager);
                            addOrChangeNamespaceDecl(prefix, uri, ele);
                        }
                        else {
                            newPrefix = addCustomNamespaceDecl(uri, ele);
                            fNSScope->addOrChangeBinding(newPrefix, uri, fMemoryManager);
                            at->setPrefix(newPrefix);
                        }
                    }
                }
            }
            else if(at->getLocalName() == 0) {
                error(XMLErrs::DOMLevel1Node, at);
            }
        }
    }
}



const XMLCh * DOMNormalizer::integerToXMLCh(unsigned int i) const {
    XMLCh *buf = (XMLCh*) fMemoryManager->allocate(15 * sizeof(XMLCh));//new XMLCh[15];
	XMLCh *pos = buf + sizeof(buf) - sizeof(XMLCh);
	*pos = chNull;

	do {
        switch(i % 10) {
        case 0 : *--pos = chDigit_0;break;
        case 1 : *--pos = chDigit_1;break;
        case 2 : *--pos = chDigit_2;break;
        case 3 : *--pos = chDigit_3;break;
        case 4 : *--pos = chDigit_4;break;
        case 5 : *--pos = chDigit_5;break;
        case 6 : *--pos = chDigit_6;break;
        case 7 : *--pos = chDigit_7;break;
        case 8 : *--pos = chDigit_8;break;
        case 9 : *--pos = chDigit_9;break;
        default:;
        }
		i /= 10;
	} while (i);

    const XMLCh *copy = fDocument->getPooledString(pos);
    fMemoryManager->deallocate(buf);//delete[] buf;
	return copy;
}





void DOMNormalizer::addOrChangeNamespaceDecl(const XMLCh* prefix, const XMLCh* uri, DOMElementImpl* element) const {

    if (XMLString::equals(prefix, XMLUni::fgZeroLenString)) {
        element->setAttributeNS(XMLUni::fgXMLNSURIName, XMLUni::fgXMLNSString, uri);
    } else {
        XMLBuffer buf(1023, fMemoryManager);
        buf.set(XMLUni::fgXMLNSString);
        buf.append(chColon);
        buf.append(prefix);
        element->setAttributeNS(XMLUni::fgXMLNSURIName, buf.getRawBuffer(), uri);
    }
}

const XMLCh* DOMNormalizer::addCustomNamespaceDecl(const XMLCh* uri, DOMElementImpl *element) const {
    XMLBuffer preBuf(1023, fMemoryManager);
    preBuf.append(chLatin_N);
    preBuf.append(chLatin_S);
    preBuf.append(integerToXMLCh(fNewNamespaceCount));
    ((DOMNormalizer *)this)->fNewNamespaceCount++;

    while(fNSScope->getUri(preBuf.getRawBuffer())) {
        preBuf.reset();
        preBuf.append(chLatin_N);
        preBuf.append(chLatin_S);
        preBuf.append(integerToXMLCh(fNewNamespaceCount));
        ((DOMNormalizer *)this)->fNewNamespaceCount++;
    }

    XMLBuffer buf(1023, fMemoryManager);
    buf.set(XMLUni::fgXMLNSString);
    buf.append(chColon);
    buf.append(preBuf.getRawBuffer());
    element->setAttributeNS(XMLUni::fgXMLNSURIName, buf.getRawBuffer(), uri);

    return element->getAttributeNodeNS(XMLUni::fgXMLNSURIName, preBuf.getRawBuffer())->getLocalName();
}

XMLSize_t DOMNormalizer::InScopeNamespaces::size() {
    return fScopes->size();
}

DOMNormalizer::InScopeNamespaces::InScopeNamespaces(MemoryManager* const manager)
: lastScopeWithBindings(0)
{
    fScopes = new (manager) RefVectorOf<Scope>(10, true, manager);
}

DOMNormalizer::InScopeNamespaces::~InScopeNamespaces() {
    delete fScopes;
}

void DOMNormalizer::InScopeNamespaces::addOrChangeBinding(const XMLCh *prefix, const XMLCh *uri,
                                                          MemoryManager* const manager) {
    XMLSize_t s = fScopes->size();

    if(!s)
        addScope(manager);

    Scope *curScope = fScopes->elementAt(s - 1);
    curScope->addOrChangeBinding(prefix, uri, manager);

    lastScopeWithBindings = curScope;
}

void DOMNormalizer::InScopeNamespaces::addScope(MemoryManager* const manager) {
    Scope *s = new (manager) Scope(lastScopeWithBindings);
    fScopes->addElement(s);
}

void DOMNormalizer::InScopeNamespaces::removeScope() {
    lastScopeWithBindings = fScopes->elementAt(fScopes->size() - 1)->fBaseScopeWithBindings;
    Scope *s = fScopes->orphanElementAt(fScopes->size() - 1);
    delete s;
}

bool DOMNormalizer::InScopeNamespaces::isValidBinding(const XMLCh* prefix, const XMLCh* uri) const {
    const XMLCh* actual = fScopes->elementAt(fScopes->size() - 1)->getUri(prefix);
    if(actual == 0 || !XMLString::equals(actual, uri))
        return false;
    return true;
}

const XMLCh* DOMNormalizer::InScopeNamespaces::getPrefix(const XMLCh* uri) const {
    return fScopes->elementAt(fScopes->size() - 1)->getPrefix(uri);
}

const XMLCh* DOMNormalizer::InScopeNamespaces::getUri(const XMLCh* prefix) const {
    return fScopes->elementAt(fScopes->size() - 1)->getUri(prefix);
}



DOMNormalizer::InScopeNamespaces::Scope::Scope(Scope *baseScopeWithBindings) : fBaseScopeWithBindings(baseScopeWithBindings), fPrefixHash(0), fUriHash(0)
{
}

DOMNormalizer::InScopeNamespaces::Scope::~Scope() {
    delete fPrefixHash;
    delete fUriHash;
}

void DOMNormalizer::InScopeNamespaces::Scope::addOrChangeBinding(const XMLCh *prefix, const XMLCh *uri,
                                                                 MemoryManager* const manager) {
    //initialize and copy forward now we need to
    if(!fUriHash) {
        fPrefixHash = new (manager) RefHashTableOf<XMLCh>(10, (bool) false, manager);
        fUriHash = new (manager) RefHashTableOf<XMLCh>(10, (bool) false, manager);

        if(fBaseScopeWithBindings) {
            RefHashTableOfEnumerator<XMLCh> preEnumer(fBaseScopeWithBindings->fPrefixHash, false, manager);
            while(preEnumer.hasMoreElements()) {
                const XMLCh* prefix = (XMLCh*) preEnumer.nextElementKey();
                const XMLCh* uri  = fBaseScopeWithBindings->fPrefixHash->get((void*)prefix);

                //have to cast here because otherwise we have delete problems under windows :(
                fPrefixHash->put((void *)prefix, (XMLCh*)uri);
            }

            RefHashTableOfEnumerator<XMLCh> uriEnumer(fBaseScopeWithBindings->fUriHash, false, manager);
            while(uriEnumer.hasMoreElements()) {
                const XMLCh* uri = (XMLCh*) uriEnumer.nextElementKey();
                const XMLCh* prefix  = fBaseScopeWithBindings->fUriHash->get((void*)uri);

                //have to cast here because otherwise we have delete problems under windows :(
                fUriHash->put((void *)uri, (XMLCh*)prefix);
            }
        }
    }

    const XMLCh *oldUri = fPrefixHash->get(prefix);
    if(oldUri) {
        fUriHash->removeKey(oldUri);
    }

    fPrefixHash->put((void *)prefix, (XMLCh*)uri);
    fUriHash->put((void *)uri, (XMLCh*)prefix);
}

const XMLCh* DOMNormalizer::InScopeNamespaces::Scope::getUri(const XMLCh *prefix) const {
    const XMLCh* uri = 0;

    if(fPrefixHash) {
        uri = fPrefixHash->get(prefix);
    }
    else if(fBaseScopeWithBindings) {
        uri = fBaseScopeWithBindings->getUri(prefix);
    }

    return uri ? uri : 0;
}

const XMLCh* DOMNormalizer::InScopeNamespaces::Scope::getPrefix(const XMLCh* uri) const {
    const XMLCh* prefix = 0;

    if(fUriHash) {
        prefix = fUriHash->get(uri);
    }
    else if(fBaseScopeWithBindings) {
        prefix = fBaseScopeWithBindings->getPrefix(uri);
    }
    return prefix ? prefix : 0;
}

void DOMNormalizer::error(const XMLErrs::Codes code, const DOMNode *node) const
{
    if (fErrorHandler) {

        //  Load the message into alocal and replace any tokens found in
        //  the text.
        const XMLSize_t maxChars = 2047;
        XMLCh errText[maxChars + 1];

        if (!gMsgLoader->loadMsg(code, errText, maxChars))
        {
                // <TBD> Should probably load a default message here
        }

        DOMErrorImpl domError(
          XMLErrs::DOMErrorType (code), 0, errText, (void*)node);
        bool toContinueProcess = true;
        try
        {
            toContinueProcess = fErrorHandler->handleError(domError);
        }
        catch(...)
        {
        }
        if (!toContinueProcess)
            throw (XMLErrs::Codes) code;
    }
}



XERCES_CPP_NAMESPACE_END
