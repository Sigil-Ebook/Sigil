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
 * $Id: XIncludeUtils.cpp 933212 2010-04-12 12:17:58Z amassari $
 */

#include <xercesc/xinclude/XIncludeUtils.hpp>
#include <xercesc/xinclude/XIncludeLocation.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/util/XMLUri.hpp>
#include <xercesc/util/XMLMsgLoader.hpp>
#include <xercesc/util/XMLResourceIdentifier.hpp>
#include <xercesc/util/BinInputStream.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/internal/XMLInternalErrorHandler.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/sax/InputSource.hpp>
#include <xercesc/framework/URLInputSource.hpp>

XERCES_CPP_NAMESPACE_BEGIN

XIncludeUtils::XIncludeUtils(XMLErrorReporter *errorReporter){
    fErrorReporter = errorReporter;
    fIncludeHistoryHead = NULL;
}

XIncludeUtils::~XIncludeUtils(){
    freeInclusionHistory();
}

// ---------------------------------------------------------------------------
//  Generic function to parse a dom node performing any Xinclude's it ecounters,
//   storing its results in parsedDocument, which is expected to be a real
//   document. sourceNode is the current location in parsedDocument, and
//   all xinclude manipulation is done in place (i.e. source is manipulated).
// ---------------------------------------------------------------------------
bool
XIncludeUtils::parseDOMNodeDoingXInclude(DOMNode *sourceNode, DOMDocument *parsedDocument, XMLEntityHandler* entityResolver){
    if (sourceNode) {
        /* create the list of child elements here, since it gets changed during the parse */
        RefVectorOf<DOMNode> children(10, false);
        for (DOMNode *child = sourceNode->getFirstChild(); child != NULL; child = child->getNextSibling()){
            children.addElement(child);
        }

        if (sourceNode->getNodeType() == DOMNode::ELEMENT_NODE){
            if (isXIIncludeDOMNode(sourceNode)){
                /* once we do an include on the source element, it is unsafe to do the include
                   on the children, since they will have been changed by the top level include */
                bool success = doDOMNodeXInclude(sourceNode, parsedDocument, entityResolver);

                //popFromCurrentInclusionHistoryStack(NULL);
                /* return here as we do not want to fall through to the parsing of the children below
                   - they should have been replaced by the XInclude */
                return success;
            } else if (isXIFallbackDOMNode(sourceNode)){
                /* This must be a fallback element that is not a child of an include element.
                   This is defined as a fatal error */
                XIncludeUtils::reportError(sourceNode, XMLErrs::XIncludeOrphanFallback,
                    NULL, parsedDocument->getDocumentURI());
                return false;
            }
        }

        /* to have got here, we must not have found an xinclude element in the current element, so
           need to walk the entire child list parsing for each. An xinclude in  a
           node does not affect a peer, so we can simply parse each child in turn */
        for (XMLSize_t i = 0; i < children.size(); i++){
            parseDOMNodeDoingXInclude(children.elementAt(i), parsedDocument, entityResolver);
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// utility func to extract a DOMNodes Base attr value if present
// ---------------------------------------------------------------------------
static const XMLCh *
getBaseAttrValue(DOMNode *node){
    if (node->getNodeType() == DOMNode::ELEMENT_NODE){
        DOMElement *elem = (DOMElement *)node;
        if(elem->hasAttributes()) {
            /* get all the attributes of the node */
            DOMNamedNodeMap *pAttributes = elem->getAttributes();
            XMLSize_t nSize = pAttributes->getLength();
            for(XMLSize_t i=0;i<nSize;++i) {
                DOMAttr *pAttributeNode = (DOMAttr*) pAttributes->item(i);
                /* get attribute name */
                if (XMLString::equals(pAttributeNode->getName(), XIncludeUtils::fgXIBaseAttrName)){
                    /*if (namespace == XMLUni::fgXMLString){

                    }*/
                    return pAttributeNode->getValue();
                }
            }
        }
    }
    return NULL;
}

// ---------------------------------------------------------------------------
//  This method assumes that currentNode is an xinclude element and parses
//   it accordingly, acting on what it finds.
// ---------------------------------------------------------------------------
bool
XIncludeUtils::doDOMNodeXInclude(DOMNode *xincludeNode, DOMDocument *parsedDocument, XMLEntityHandler* entityResolver){
    bool modifiedNode = false;
    /* the relevant attributes to look for */
    const XMLCh *href = NULL;
    const XMLCh *parse = NULL;
    const XMLCh *xpointer = NULL;
    const XMLCh *encoding = NULL;
    const XMLCh *accept = NULL;
    const XMLCh *acceptlanguage = NULL;
    DOMNode *includeParent = xincludeNode->getParentNode();


    if(xincludeNode->hasAttributes()) {
        /* get all the attributes of the node */
        DOMNamedNodeMap *pAttributes = xincludeNode->getAttributes();
        XMLSize_t nSize = pAttributes->getLength();
        for(XMLSize_t i=0;i<nSize;++i) {
            DOMAttr *pAttributeNode = (DOMAttr*) pAttributes->item(i);
            const XMLCh *attrName = pAttributeNode->getName();
            /* check each attribute against the potential useful names */
            if (XMLString::equals(attrName, XIncludeUtils::fgXIIncludeHREFAttrName)){
                href = pAttributeNode->getValue();
            } else if (XMLString::equals(attrName, XIncludeUtils::fgXIIncludeParseAttrName)){
                parse = pAttributeNode->getValue();
            } else if (XMLString::equals(attrName, XIncludeUtils::fgXIIncludeXPointerAttrName)){
                xpointer = pAttributeNode->getValue();
            } else if (XMLString::equals(attrName, XIncludeUtils::fgXIIncludeEncodingAttrName)){
                encoding = pAttributeNode->getValue();
            } else if (XMLString::equals(attrName, XIncludeUtils::fgXIIncludeAcceptAttrName)){
                accept = pAttributeNode->getValue();
            } else if (XMLString::equals(attrName, XIncludeUtils::fgXIIncludeAcceptLanguageAttrName)){
                acceptlanguage = pAttributeNode->getValue();
            } else {
                /* if any other attribute is in the xi namespace, it's an error */
                const XMLCh *attrNamespaceURI = pAttributeNode->getNamespaceURI();
                if (attrNamespaceURI && XMLString::equals(attrNamespaceURI, XIncludeUtils::fgXIIIncludeNamespaceURI)){
                } else {
                    /* ignore - any other attribute is allowed according to spec,
                       and must be ignored */
                }
            }
        }
    }
    // 3.1 xi:include Element
    // The children property of the xi:include element may include a single xi:fallback element;
    // the appearance of more than one xi:fallback element, an xi:include element,
    // or any other element from the XInclude namespace is a fatal error.
    DOMNode *child;
    DOMElement *fallback = NULL;
    for (child = xincludeNode->getFirstChild(); child != 0; child=child->getNextSibling()){
        if(child->getNodeType()!=DOMNode::ELEMENT_NODE)
            continue;
        if ( isXIFallbackDOMNode(child) ){
            if (fallback != NULL){
                /* fatal error - there are more than one fallback children */
                XIncludeUtils::reportError(xincludeNode, XMLErrs::XIncludeMultipleFallbackElems,
                    parsedDocument->getDocumentURI(), parsedDocument->getDocumentURI());
                return false;
            }
            fallback = (DOMElement*)child;
        }
        else if(isXIIncludeDOMNode(child) || XMLString::equals(child->getNamespaceURI(), XIncludeUtils::fgXIIIncludeNamespaceURI)) {
            /* fatal error - an xi element different from xi:fallback is a child of xi:include */
            XIncludeUtils::reportError(xincludeNode, XMLErrs::XIncludeDisallowedChild,
                child->getNodeName(), parsedDocument->getDocumentURI());
            return false;
        }
    }

    if (href == NULL){
        /* this is an unrecoverable error until we have xpointer support -
           if there is an xpointer, the current document is assumed
           however, there is no xpointer support yet */
        XIncludeUtils::reportError(xincludeNode, XMLErrs::XIncludeNoHref,
            NULL, parsedDocument->getDocumentURI());
        return false;
    }

    /* set up the accept and accept-language values */
    if (accept != NULL){

    }

    if (parse == NULL){
        /* use the default, as specified */
        parse = XIncludeUtils::fgXIIncludeParseAttrXMLValue;
    }

    if (xpointer != NULL){
        /* not supported yet */
        /* Note that finding an xpointer attr along with parse="text" is a Fatal Error
         *  - http://www.w3.org/TR/xinclude/#include-location */
        XIncludeUtils::reportError(xincludeNode, XMLErrs::XIncludeXPointerNotSupported,
            NULL, href);
        return false;
    }

    /* set up the href according to what has gone before */
    XIncludeLocation hrefLoc(href);
    XIncludeLocation relativeLocation(href);
    const XMLCh *includeBase = xincludeNode->getBaseURI();
    if (includeBase != NULL){
        hrefLoc.prependPath(includeBase);
    }

    if (getBaseAttrValue(xincludeNode) != NULL){
        relativeLocation.prependPath(getBaseAttrValue(xincludeNode));
    }

    /*  Take the relevant action - we need to retrieve the target as a whole before
        we can know if it was successful or not, therefore the do* methods do
        not modify the parsedDocument. Swapping the results in is left to the
        caller (i.e. here) */
    DOMText *includedText = NULL;
    DOMDocument *includedDoc = NULL;
    if (XMLString::equals(parse, XIncludeUtils::fgXIIncludeParseAttrXMLValue)){
        /* including a XML element */
        includedDoc = doXIncludeXMLFileDOM(hrefLoc.getLocation(), relativeLocation.getLocation(), xincludeNode, parsedDocument, entityResolver);
    } else if (XMLString::equals(parse, XIncludeUtils::fgXIIncludeParseAttrTextValue)){
        /* including a text value */
        includedText = doXIncludeTEXTFileDOM(hrefLoc.getLocation(), relativeLocation.getLocation(), encoding, xincludeNode, parsedDocument, entityResolver);
    } else {
        /* invalid parse attribute value - fatal error according to the specification */
        XIncludeUtils::reportError(xincludeNode, XMLErrs::XIncludeInvalidParseVal,
            parse, parsedDocument->getDocumentURI());
        return false;
    }

    RefVectorOf<DOMNode> delayedProcessing(12,false);
    if (includedDoc == NULL && includedText == NULL){
        /* there was an error - this is now a resource error
           let's see if there is a fallback */
        XIncludeUtils::reportError(xincludeNode, XMLErrs::XIncludeIncludeFailedResourceError,
            hrefLoc.getLocation(), parsedDocument->getDocumentURI());

        if (includeParent == NULL){
            includeParent = parsedDocument;
        }

        // we could be getting errors trying to insert elements at the root of the document, so we should use replaceChild;
        // in order to handle multiple nodes, add them to a document fragment and use that to replace the original node
        if (fallback){
            /* baseURI fixups - see http://www.w3.org/TR/xinclude/#base for details. */
            XMLUri parentURI(includeParent->getBaseURI());
            XMLUri includedURI(fallback->getBaseURI());

            if (fallback->hasChildNodes()){
                DOMDocumentFragment* frag = parsedDocument->createDocumentFragment();
                DOMNode *child = fallback->getFirstChild();
                /* add the content of the fallback element, and remove the fallback elem itself */
                for ( ; child != NULL ; child=child->getNextSibling()){
                    if (child->getNodeType() == DOMNode::DOCUMENT_TYPE_NODE){
                        continue;
                    }
                    DOMNode *newNode = parsedDocument->importNode(child, true);
                    /* if the paths differ we need to add a base attribute */
                    if (newNode->getNodeType()==DOMNode::ELEMENT_NODE && !XMLString::equals(parentURI.getPath(), includedURI.getPath())){
                        if (getBaseAttrValue(newNode) == NULL){
                            /* need to calculate the proper path difference to get the relativePath */
                            ((DOMElement*)newNode)->setAttribute(fgXIBaseAttrName, getBaseAttrValue(fallback->getParentNode()));
                        } else {
                            /* the included node has base of its own which takes precedence */
                            XIncludeLocation xil(getBaseAttrValue(newNode));
                            if (getBaseAttrValue(fallback->getParentNode()) != NULL){
                                /* prepend any specific base modification of the xinclude node */
                                xil.prependPath(getBaseAttrValue(fallback->getParentNode()));
                            }
                            ((DOMElement*)newNode)->setAttribute(fgXIBaseAttrName, xil.getLocation());
                        }
                    }
                    DOMNode *newChild = frag->appendChild(newNode);
                    // don't process the node now, wait until it is placed in the final position
                    delayedProcessing.addElement(newChild);
                    //parseDOMNodeDoingXInclude(newChild, parsedDocument, entityResolver);
                }
                includeParent->replaceChild(frag, xincludeNode);
                frag->release();

                for(XMLSize_t i=0;i<delayedProcessing.size();i++)
                {
                    DOMNode* childNode=delayedProcessing.elementAt(i);
                    parseDOMNodeDoingXInclude(childNode, parsedDocument, entityResolver);
                }
                modifiedNode = true;
            } else {
                /* empty fallback element - simply remove it! */
                includeParent->removeChild(xincludeNode);
                modifiedNode = true;
            }
        } else {
            XIncludeUtils::reportError(xincludeNode, XMLErrs::XIncludeIncludeFailedNoFallback,
                parsedDocument->getDocumentURI(), parsedDocument->getDocumentURI());
            return false;
        }
    } else {
        if (includedDoc){
            /* record the successful include while we process the children */
            addDocumentURIToCurrentInclusionHistoryStack(hrefLoc.getLocation());

            DOMDocumentFragment* frag = parsedDocument->createDocumentFragment();
            /* need to import the document prolog here */
            DOMNode *child = includedDoc->getFirstChild();
            for (; child != NULL; child = child->getNextSibling()) {
                if (child->getNodeType() == DOMNode::DOCUMENT_TYPE_NODE)
                    continue;
                // check for NOTATION or ENTITY clash
                if(child->getNodeType()==DOMNode::ELEMENT_NODE && includedDoc->getDoctype()!=NULL) {
                    DOMNamedNodeMap *pAttributes = child->getAttributes();
                    XMLSize_t nSize = pAttributes->getLength();
                    for(XMLSize_t i=0;i<nSize;++i) {
                        DOMAttr *pAttributeNode = (DOMAttr*) pAttributes->item(i);
                        const DOMTypeInfo * typeInfo=pAttributeNode->getSchemaTypeInfo();
                        if(typeInfo && XMLString::equals(typeInfo->getTypeNamespace(), XMLUni::fgInfosetURIName)) {
                            if(XMLString::equals(typeInfo->getTypeName(), XMLUni::fgNotationString)) {
                                const XMLCh* notationName=pAttributeNode->getNodeValue();
                                DOMNotation* notat=(DOMNotation*)includedDoc->getDoctype()->getNotations()->getNamedItem(notationName);
                                // ensure we have a DTD
                                if(parsedDocument->getDoctype()==NULL)
                                    parsedDocument->insertBefore(parsedDocument->createDocumentType(parsedDocument->getDocumentElement()->getNodeName(), NULL,NULL), parsedDocument->getFirstChild());
                                DOMNotation* myNotation=(DOMNotation*)parsedDocument->getDoctype()->getNotations()->getNamedItem(notationName);
                                if(myNotation==NULL)
                                {
                                    // it's missing, add it
                                    parsedDocument->getDoctype()->getNotations()->setNamedItem(parsedDocument->importNode(notat, true));
                                }
                                else if(XMLString::equals(myNotation->getPublicId(), notat->getPublicId()) &&
                                        XMLString::equals(myNotation->getSystemId(), notat->getSystemId()) &&
                                        XMLString::equals(myNotation->getBaseURI(), notat->getBaseURI()))
                                {
                                    // it's duplicate, ignore it
                                }
                                else
                                {
                                    // it's a conflict, report it
                                    XIncludeUtils::reportError(xincludeNode, XMLErrs::XIncludeConflictingNotation,
                                        notationName, parsedDocument->getDocumentURI());
                                }
                            }
                            else if(XMLString::equals(typeInfo->getTypeName(), XMLUni::fgEntityString)) {
                                const XMLCh* entityName=pAttributeNode->getNodeValue();
                                DOMEntity* ent=(DOMEntity*)includedDoc->getDoctype()->getEntities()->getNamedItem(entityName);
                                // ensure we have a DTD
                                if(parsedDocument->getDoctype()==NULL)
                                    parsedDocument->insertBefore(parsedDocument->createDocumentType(parsedDocument->getDocumentElement()->getNodeName(), NULL,NULL), parsedDocument->getFirstChild());
                                DOMEntity* myEnt=(DOMEntity*)parsedDocument->getDoctype()->getEntities()->getNamedItem(entityName);
                                if(myEnt==NULL)
                                {
                                    // it's missing, add it
                                    parsedDocument->getDoctype()->getEntities()->setNamedItem(parsedDocument->importNode(ent, true));
                                }
                                else if(XMLString::equals(myEnt->getPublicId(), ent->getPublicId()) &&
                                        XMLString::equals(myEnt->getSystemId(), ent->getSystemId()) &&
                                        XMLString::equals(myEnt->getBaseURI(), ent->getBaseURI()))
                                {
                                    // it's duplicate, ignore it
                                }
                                else
                                {
                                    // it's a conflict, report it
                                    XIncludeUtils::reportError(xincludeNode, XMLErrs::XIncludeConflictingEntity,
                                        entityName, parsedDocument->getDocumentURI());
                                }
                            }
                        }
                    }
                }
                DOMNode *newNode = parsedDocument->importNode(child, true);
                DOMNode *newChild = frag->appendChild(newNode);
                // don't process the node now, wait until it is placed in the final position
                delayedProcessing.addElement(newChild);
                //parseDOMNodeDoingXInclude(newChild, parsedDocument, entityResolver);
            }
            includeParent->replaceChild(frag, xincludeNode);
            frag->release();

            for(XMLSize_t i=0;i<delayedProcessing.size();i++)
            {
                DOMNode* childNode=delayedProcessing.elementAt(i);
                parseDOMNodeDoingXInclude(childNode, parsedDocument, entityResolver);
            }
            popFromCurrentInclusionHistoryStack(NULL);
            modifiedNode = true;
        } else if (includedText){
            includeParent->replaceChild(includedText, xincludeNode);
            modifiedNode = true;
        }
    }

    if (includedDoc)
        includedDoc->release();

    return modifiedNode;
}

DOMDocument *
XIncludeUtils::doXIncludeXMLFileDOM(const XMLCh *href,
                                    const XMLCh *relativeHref,
                                    DOMNode *includeNode,
                                    DOMDocument *parsedDocument,
                                    XMLEntityHandler* entityResolver){
    if (XIncludeUtils::isInCurrentInclusionHistoryStack(href)){
         /* including something back up the current history */
         XIncludeUtils::reportError(parsedDocument, XMLErrs::XIncludeCircularInclusionLoop,
              href, href);
         return NULL;
    }

    if (XMLString::equals(href, parsedDocument->getBaseURI())){
        /* trying to include itself */
        XIncludeUtils::reportError(parsedDocument, XMLErrs::XIncludeCircularInclusionDocIncludesSelf,
              href, href);
        return NULL;
    }

    /* Instantiate the DOM parser. */
    XercesDOMParser parser;
    parser.setDoNamespaces(true);
    /* don't want to recurse the xi processing here */
    parser.setDoXInclude(false);
    /* create the schema info nodes, so that we can detect conflicting notations */
    parser.setCreateSchemaInfo(true);
    XMLInternalErrorHandler xierrhandler;
    parser.setErrorHandler(&xierrhandler);

    DOMDocument *includedNode = NULL;
    try {
        InputSource* is=NULL;
        Janitor<InputSource> janIS(is);
        if(entityResolver) {
            XMLResourceIdentifier resIdentifier(XMLResourceIdentifier::ExternalEntity,
                                                relativeHref,
                                                NULL,
                                                NULL,
                                                includeNode->getBaseURI());
            is=entityResolver->resolveEntity(&resIdentifier);
            janIS.reset(is);
        }
        if(janIS.get()!=NULL)
            parser.parse(*janIS.get());
        else
            parser.parse(href);
        /* need to be able to release the parser but keep the document */
        if (!xierrhandler.getSawError() && !xierrhandler.getSawFatal())
            includedNode = parser.adoptDocument();
    }
    catch (const XMLException& /*toCatch*/)
    {
        XIncludeUtils::reportError(parsedDocument, XMLErrs::XIncludeResourceErrorWarning,
              href, href);
    }
    catch (const DOMException& /*toCatch*/)
    {
        XIncludeUtils::reportError(parsedDocument, XMLErrs::XIncludeResourceErrorWarning,
              href, href);
    }
    catch (...)
    {
        XIncludeUtils::reportError(parsedDocument, XMLErrs::XIncludeResourceErrorWarning,
             href, href);
    }

    //addDocumentURIToCurrentInclusionHistoryStack(href);

    if(includedNode != NULL){
        /* baseURI fixups - see http://www.w3.org/TR/xinclude/#base for details. */
        DOMElement *topLevelElement = includedNode->getDocumentElement();
        if (topLevelElement && topLevelElement->getNodeType() == DOMNode::ELEMENT_NODE ){
            XMLUri parentURI(includeNode->getBaseURI());
            XMLUri includedURI(includedNode->getBaseURI());

            /* if the paths differ we need to add a base attribute */
            if (!XMLString::equals(parentURI.getPath(), includedURI.getPath())){
                if (getBaseAttrValue(topLevelElement) == NULL){
                    /* need to calculate the proper path difference to get the relativePath */
                    topLevelElement->setAttribute(fgXIBaseAttrName, relativeHref);
                } else {
                    /* the included node has base of its own which takes precedence */
                    XIncludeLocation xil(getBaseAttrValue(topLevelElement));
                    if (getBaseAttrValue(includeNode) != NULL){
                        /* prepend any specific base modification of the xinclude node */
                        xil.prependPath(getBaseAttrValue(includeNode));
                    }
                    topLevelElement->setAttribute(fgXIBaseAttrName, xil.getLocation());
                }
            }
        }
    }
    return includedNode;
}

DOMText *
XIncludeUtils::doXIncludeTEXTFileDOM(const XMLCh *href,
                                     const XMLCh *relativeHref,
                                     const XMLCh *encoding,
                                     DOMNode *includeNode,
                                     DOMDocument *parsedDocument,
                                     XMLEntityHandler* entityResolver){
    if (encoding == NULL)
        /* "UTF-8" is stipulated default by spec */
        encoding = XMLUni::fgUTF8EncodingString;

    XMLTransService::Codes failReason;
    XMLTranscoder* transcoder = XMLPlatformUtils::fgTransService->makeNewTranscoderFor(encoding, failReason, 16*1024);
    Janitor<XMLTranscoder> janTranscoder(transcoder);
    if (failReason){
        XIncludeUtils::reportError(parsedDocument, XMLErrs::XIncludeCannotOpenFile, href, href);
        return NULL;
    }

    //addDocumentURIToCurrentInclusionHistoryStack(href);

    InputSource* is=NULL;
    Janitor<InputSource> janIS(is);
    if(entityResolver) {
        XMLResourceIdentifier resIdentifier(XMLResourceIdentifier::ExternalEntity,
                                            relativeHref,
                                            NULL,
                                            NULL,
                                            includeNode->getBaseURI());
        is=entityResolver->resolveEntity(&resIdentifier);
        janIS.reset(is);
    }
    if(janIS.get()==NULL)
        janIS.reset(new URLInputSource(href));
    if(janIS.get()==NULL) {
        XIncludeUtils::reportError(parsedDocument, XMLErrs::XIncludeCannotOpenFile,
            href, href);
        return NULL;
    }
    BinInputStream* stream=janIS.get()->makeStream();
    if(stream==NULL) {
        XIncludeUtils::reportError(parsedDocument, XMLErrs::XIncludeCannotOpenFile,
            href, href);
        return NULL;
    }
    Janitor<BinInputStream> janStream(stream);
    const XMLSize_t maxToRead=16*1024;
    XMLByte* buffer=(XMLByte*)XMLPlatformUtils::fgMemoryManager->allocate(maxToRead * sizeof(XMLByte));
    if(buffer==NULL)
        throw OutOfMemoryException();
    ArrayJanitor<XMLByte> janBuffer(buffer, XMLPlatformUtils::fgMemoryManager);
    XMLCh* xmlChars=(XMLCh*)XMLPlatformUtils::fgMemoryManager->allocate(maxToRead*2*sizeof(XMLCh));
    if(xmlChars==NULL)
        throw OutOfMemoryException();
    ArrayJanitor<XMLCh> janUniBuffer(xmlChars, XMLPlatformUtils::fgMemoryManager);
    unsigned char *charSizes = (unsigned char *)XMLPlatformUtils::fgMemoryManager->allocate(maxToRead * sizeof(unsigned char));
    if(charSizes==NULL)
        throw OutOfMemoryException();
    ArrayJanitor<unsigned char> janCharSizes(charSizes, XMLPlatformUtils::fgMemoryManager);

    XMLSize_t nRead, nOffset=0;
    XMLBuffer repository;
    while((nRead=stream->readBytes(buffer+nOffset, maxToRead-nOffset))>0){
        XMLSize_t bytesEaten=0;
        XMLSize_t nCount = transcoder->transcodeFrom(buffer, nRead, xmlChars, maxToRead*2, bytesEaten, charSizes);
        repository.append(xmlChars, nCount);
        if(bytesEaten<nRead) {
            nOffset=nRead-bytesEaten;
            memmove(buffer, buffer+bytesEaten, nRead-bytesEaten);
        }
    }
    return parsedDocument->createTextNode(repository.getRawBuffer());
}

/*static*/ bool
XIncludeUtils::isXIIncludeDOMNode(DOMNode *node){
    const XMLCh *nodeName = node->getLocalName();
    const XMLCh *namespaceURI = node->getNamespaceURI();

    return isXIIncludeElement(nodeName, namespaceURI);
}

/*static*/ bool
XIncludeUtils::isXIFallbackDOMNode(DOMNode *node){
    const XMLCh *nodeName = node->getLocalName();
    const XMLCh *namespaceURI = node->getNamespaceURI();

    return isXIFallbackElement(nodeName, namespaceURI);
}

/*static*/ bool
XIncludeUtils::isXIIncludeElement(const XMLCh *name, const XMLCh *namespaceURI){
    if (namespaceURI == NULL || name == NULL){
        /* no namespaces not supported */
        return false;
    }
    if (XMLString::equals(name, fgXIIncludeQName)
        && XMLString::equals(namespaceURI, fgXIIIncludeNamespaceURI)){
        return true;
    }
    return false;
}

/*static*/ bool
XIncludeUtils::isXIFallbackElement(const XMLCh *name, const XMLCh *namespaceURI){
    if (namespaceURI == NULL || name == NULL){
        /* no namespaces not supported */
        return false;
    }
    if (XMLString::equals(name, fgXIFallbackQName)
        && XMLString::equals(namespaceURI, fgXIIIncludeNamespaceURI)){
        return true;
    }
    return false;
}

/* 4.1.1 */
const XMLCh *
XIncludeUtils::getEscapedHRefAttrValue(const XMLCh * /*hrefAttrValue*/, bool & /*needsDeallocating*/){
    XMLCh *escapedAttr = NULL;
    return escapedAttr;
}

/* 4.1.2 */
bool
XIncludeUtils::setContentNegotiation(const XMLCh * /*acceptAttrValue*/, const XMLCh * /*acceptLangAttrValue*/){
    return false;
}

bool
XIncludeUtils::checkTextIsValidForInclude(XMLCh * /*includeChars*/){
    return false;
}

// ========================================================
// the stack utilities are slightly convoluted debug versions, they
// will be pared down for the release code
// ========================================================
static XIncludeHistoryNode *
getTopOfCurrentInclusionHistoryStack(XIncludeHistoryNode *head){
    XIncludeHistoryNode *historyCursor = head;
    if (historyCursor == NULL){
        return NULL;
    }
    while (historyCursor->next != NULL){
        historyCursor = historyCursor->next;
    }
    return historyCursor;
}

bool
XIncludeUtils::addDocumentURIToCurrentInclusionHistoryStack(const XMLCh *URItoAdd){
    XIncludeHistoryNode *newNode = (XIncludeHistoryNode *)XMLPlatformUtils::fgMemoryManager->allocate(sizeof(XIncludeHistoryNode));
    if (newNode == NULL){
        return false;
    }
    newNode->URI = XMLString::replicate(URItoAdd);
    newNode->next = NULL;

    if (fIncludeHistoryHead == NULL){
        fIncludeHistoryHead = newNode;
        return true;
    }
    XIncludeHistoryNode *topNode = getTopOfCurrentInclusionHistoryStack(fIncludeHistoryHead);
    topNode->next = newNode;
    return true;
}

bool
XIncludeUtils::isInCurrentInclusionHistoryStack(const XMLCh *toFind){
    XIncludeHistoryNode *historyCursor = fIncludeHistoryHead;
    /* walk the list */
    while (historyCursor != NULL){
        if (XMLString::equals(toFind, historyCursor->URI)){
            return true;
        }
        historyCursor = historyCursor->next;
    }
    return false;
}

XIncludeHistoryNode *
XIncludeUtils::popFromCurrentInclusionHistoryStack(const XMLCh * /*toPop*/){
    XIncludeHistoryNode *historyCursor = fIncludeHistoryHead;
    XIncludeHistoryNode *penultimateCursor = historyCursor;

    if (fIncludeHistoryHead == NULL){
        return NULL;
    }

    while (historyCursor->next != NULL){
        penultimateCursor = historyCursor;
        historyCursor = historyCursor->next;
    }

    if (historyCursor == fIncludeHistoryHead){
        fIncludeHistoryHead = NULL;
    } else {
        penultimateCursor->next = NULL;
    }

    XMLString::release(&(historyCursor->URI));
    XMLPlatformUtils::fgMemoryManager->deallocate((void *)historyCursor);
    return NULL;
}

void
XIncludeUtils::freeInclusionHistory(){
    XIncludeHistoryNode *historyCursor = XIncludeUtils::fIncludeHistoryHead;
    while (historyCursor != NULL){
        XIncludeHistoryNode *next = historyCursor->next;
        XMLString::release(&(historyCursor->URI));
        XMLPlatformUtils::fgMemoryManager->deallocate((void *)historyCursor);
        historyCursor = next;
    }
    XIncludeUtils::fIncludeHistoryHead = NULL;
}

bool
XIncludeUtils::reportError(const DOMNode* const    /*errorNode*/
                              , XMLErrs::Codes errorType
                              , const XMLCh*   const    errorMsg
                              , const XMLCh * const href)
{
    bool toContinueProcess = true;   /* default value for no error handler */

    const XMLCh* const                    systemId = href;
    const XMLCh* const                    publicId = href;
    /* TODO - look these up somehow? */
    const XMLFileLoc                 lineNum = 0;
    const XMLFileLoc                 colNum = 0;

    if (fErrorReporter)
    {
        // Load the message into a local for display
        const XMLSize_t msgSize = 1023;
        XMLCh errText[msgSize + 1];

        /* TODO - investigate whether this is complete */
        XMLMsgLoader  *errMsgLoader = XMLPlatformUtils::loadMsgSet(XMLUni::fgXMLErrDomain);
        if (errorMsg == NULL){
            if (errMsgLoader->loadMsg(errorType, errText, msgSize))
            {
                    // <TBD> Probably should load a default msg here
            }
        } else {
            if (errMsgLoader->loadMsg(errorType, errText, msgSize, errorMsg))
            {
                    // <TBD> Probably should load a default msg here
            }
        }

        fErrorReporter->error(errorType
                              , XMLUni::fgXMLErrDomain    //fgXMLErrDomain
                              , XMLErrs::errorType(errorType)
                              , errText
                              , systemId
                              , publicId
                              , lineNum
                              , colNum);
    }

    if (XMLErrs::isFatal(errorType))
        fErrorCount++;

    return toContinueProcess;
}

/* TODO - declared in this file for convenience, prob ought to be moved out to
   util/XMLUni.cpp before releasing */
const XMLCh XIncludeUtils::fgXIIncludeQName[] =
{
    chLatin_i, chLatin_n, chLatin_c, chLatin_l, chLatin_u, chLatin_d, chLatin_e, chNull
};
const XMLCh XIncludeUtils::fgXIFallbackQName[] =
{
    chLatin_f, chLatin_a, chLatin_l, chLatin_l, chLatin_b, chLatin_a, chLatin_c, chLatin_k, chNull
};
const XMLCh XIncludeUtils::fgXIIncludeHREFAttrName[] =
{
    chLatin_h, chLatin_r, chLatin_e, chLatin_f, chNull
};
const XMLCh XIncludeUtils::fgXIIncludeParseAttrName[] =
{
    chLatin_p, chLatin_a, chLatin_r, chLatin_s, chLatin_e, chNull
};
const XMLCh XIncludeUtils::fgXIIncludeXPointerAttrName[] =
{
    chLatin_x, chLatin_p, chLatin_o, chLatin_i, chLatin_n, chLatin_t, chLatin_e, chLatin_r, chNull
};
const XMLCh XIncludeUtils::fgXIIncludeEncodingAttrName[] =
{
     chLatin_e, chLatin_n, chLatin_c, chLatin_o, chLatin_d, chLatin_i, chLatin_n, chLatin_g, chNull
};
const XMLCh XIncludeUtils::fgXIIncludeAcceptAttrName[] =
{
     chLatin_a, chLatin_c, chLatin_c, chLatin_e, chLatin_p, chLatin_t, chNull
};
const XMLCh XIncludeUtils::fgXIIncludeAcceptLanguageAttrName[] =
{
     chLatin_a, chLatin_c, chLatin_c, chLatin_e, chLatin_p, chLatin_t, chDash, chLatin_l, chLatin_a,
         chLatin_n, chLatin_g, chLatin_u, chLatin_a, chLatin_g, chLatin_e, chNull
};
const XMLCh XIncludeUtils::fgXIIncludeParseAttrXMLValue[] =
{
    chLatin_x, chLatin_m, chLatin_l, chNull
};
const XMLCh XIncludeUtils::fgXIIncludeParseAttrTextValue[] =
{
    chLatin_t, chLatin_e, chLatin_x, chLatin_t, chNull
};
const XMLCh XIncludeUtils::fgXIIIncludeNamespaceURI[] =
{
    /* http://www.w3.org/2001/XInclude */
    chLatin_h, chLatin_t, chLatin_t, chLatin_p, chColon, chForwardSlash
    ,   chForwardSlash, chLatin_w, chLatin_w, chLatin_w, chPeriod
    ,   chLatin_w, chDigit_3, chPeriod, chLatin_o, chLatin_r, chLatin_g
    ,   chForwardSlash, chDigit_2, chDigit_0, chDigit_0, chDigit_1
    ,    chForwardSlash, chLatin_X, chLatin_I, chLatin_n, chLatin_c, chLatin_l
    ,    chLatin_u, chLatin_d, chLatin_e, chNull
};
const XMLCh XIncludeUtils::fgXIBaseAttrName[] =
{
    chLatin_x, chLatin_m, chLatin_l, chColon, chLatin_b, chLatin_a, chLatin_s, chLatin_e, chNull
};

XERCES_CPP_NAMESPACE_END
