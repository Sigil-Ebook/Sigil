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
 * $Id: SAX2XMLFilterImpl.cpp 673975 2008-07-04 09:23:56Z borisk $
 */

#include <xercesc/parsers/SAX2XMLFilterImpl.hpp>

XERCES_CPP_NAMESPACE_BEGIN

SAX2XMLFilterImpl::SAX2XMLFilterImpl(SAX2XMLReader* parent) :
    fParentReader(0)
    , fDocHandler(0)
    , fDTDHandler(0)
    , fEntityResolver(0)
    , fErrorHandler(0)
{
    setParent(parent);
}

SAX2XMLFilterImpl::~SAX2XMLFilterImpl()
{
}

// ---------------------------------------------------------------------------
//  SAX2XMLFilterImpl: XMLFilter impl
// ---------------------------------------------------------------------------
void SAX2XMLFilterImpl::setParent(SAX2XMLReader* parent)
{
    if(fParentReader)
    {
        fParentReader->setEntityResolver(0);
        fParentReader->setDTDHandler(0);
        fParentReader->setContentHandler(0);
        fParentReader->setErrorHandler(0);
    }
    fParentReader=parent;
    if(fParentReader)
    {
        fParentReader->setEntityResolver(this);
        fParentReader->setDTDHandler(this);
        fParentReader->setContentHandler(this);
        fParentReader->setErrorHandler(this);
    }
}

bool SAX2XMLFilterImpl::getExitOnFirstFatalError() const
{
    if(fParentReader)
        fParentReader->getExitOnFirstFatalError();
    return false;
}

bool SAX2XMLFilterImpl::getValidationConstraintFatal() const
{
    if(fParentReader)
        fParentReader->getValidationConstraintFatal();
    return false;
}

Grammar* SAX2XMLFilterImpl::getRootGrammar()
{
    if(fParentReader)
        fParentReader->getRootGrammar();
    return NULL;
}

const XMLCh* SAX2XMLFilterImpl::getURIText(unsigned int uriId) const
{
    if(fParentReader)
        fParentReader->getURIText(uriId);
    return NULL;
}

XMLFilePos SAX2XMLFilterImpl::getSrcOffset() const
{
    if(fParentReader)
        fParentReader->getSrcOffset();
    return 0;
}

// ---------------------------------------------------------------------------
//  SAX2XMLFilterImpl Validator functions
// ---------------------------------------------------------------------------
void SAX2XMLFilterImpl::setValidator(XMLValidator* valueToAdopt)
{
    if(fParentReader)
        fParentReader->setValidator(valueToAdopt);
}

XMLValidator* SAX2XMLFilterImpl::getValidator() const
{
    if(fParentReader)
        return fParentReader->getValidator();
	return 0;
}

// ---------------------------------------------------------------------------
//  SAX2XMLReader Interface
// ---------------------------------------------------------------------------
XMLSize_t SAX2XMLFilterImpl::getErrorCount() const
{
    if(fParentReader)
        return fParentReader->getErrorCount();
    return 0;
}

void SAX2XMLFilterImpl::setExitOnFirstFatalError(const bool newState)
{
    if(fParentReader)
        fParentReader->setExitOnFirstFatalError(newState);
}

void SAX2XMLFilterImpl::setValidationConstraintFatal(const bool newState)
{
    if(fParentReader)
        fParentReader->setValidationConstraintFatal(newState);
}

void SAX2XMLFilterImpl::parse (const   InputSource&    source)
{
    if(fParentReader)
        fParentReader->parse(source);
}

void SAX2XMLFilterImpl::parse (const   XMLCh* const    systemId)
{
    if(fParentReader)
        fParentReader->parse(systemId);
}

void SAX2XMLFilterImpl::parse (const   char* const     systemId)
{
    if(fParentReader)
        fParentReader->parse(systemId);
}

// ---------------------------------------------------------------------------
//  SAX2XMLFilterImpl: Progressive parse methods
// ---------------------------------------------------------------------------
bool SAX2XMLFilterImpl::parseFirst( const   XMLCh* const    systemId
                            ,       XMLPScanToken&  toFill)
{
    if(fParentReader)
        return fParentReader->parseFirst(systemId, toFill);
    return false;
}

bool SAX2XMLFilterImpl::parseFirst( const   char* const     systemId
                            ,       XMLPScanToken&  toFill)
{
    if(fParentReader)
        fParentReader->parseFirst(systemId, toFill);
    return false;
}

bool SAX2XMLFilterImpl::parseFirst( const   InputSource&    source
                            ,       XMLPScanToken&  toFill)
{
    if(fParentReader)
        fParentReader->parseFirst(source, toFill);
    return false;
}

bool SAX2XMLFilterImpl::parseNext(XMLPScanToken& token)
{
    if(fParentReader)
        fParentReader->parseNext(token);
    return false;
}

void SAX2XMLFilterImpl::parseReset(XMLPScanToken& token)
{
    if(fParentReader)
        fParentReader->parseReset(token);
}

// ---------------------------------------------------------------------------
//  SAX2XMLFilterImpl: Features and Properties
// ---------------------------------------------------------------------------

void SAX2XMLFilterImpl::setFeature(const XMLCh* const name, const bool value)
{
    if(fParentReader)
        fParentReader->setFeature(name,value);
}

bool SAX2XMLFilterImpl::getFeature(const XMLCh* const name) const
{
    if(fParentReader)
        return fParentReader->getFeature(name);
    return false;
}

void SAX2XMLFilterImpl::setProperty(const XMLCh* const name, void* value)
{
    if(fParentReader)
        fParentReader->setProperty(name,value);
}

void* SAX2XMLFilterImpl::getProperty(const XMLCh* const name) const
{
    if(fParentReader)
        return fParentReader->getProperty(name);
    return NULL;
}

// ---------------------------------------------------------------------------
//  SAX2XMLFilterImpl: Grammar preparsing
// ---------------------------------------------------------------------------
Grammar* SAX2XMLFilterImpl::loadGrammar(const char* const systemId,
                                        const Grammar::GrammarType grammarType,
                                        const bool toCache)
{
    if(fParentReader)
        return fParentReader->loadGrammar(systemId, grammarType, toCache);
    return NULL;
}

Grammar* SAX2XMLFilterImpl::loadGrammar(const XMLCh* const systemId,
                                        const Grammar::GrammarType grammarType,
                                        const bool toCache)
{
    if(fParentReader)
        return fParentReader->loadGrammar(systemId, grammarType, toCache);
    return NULL;
}

Grammar* SAX2XMLFilterImpl::loadGrammar(const InputSource& source,
                                        const Grammar::GrammarType grammarType,
                                        const bool toCache)
{
    if(fParentReader)
        return fParentReader->loadGrammar(source, grammarType, toCache);
    return NULL;
}

void SAX2XMLFilterImpl::resetCachedGrammarPool()
{
    if(fParentReader)
        fParentReader->resetCachedGrammarPool();
}

void SAX2XMLFilterImpl::setInputBufferSize(const XMLSize_t bufferSize)
{
    if(fParentReader)
        fParentReader->setInputBufferSize(bufferSize);
}

Grammar* SAX2XMLFilterImpl::getGrammar(const XMLCh* const nameSpaceKey)
{
    if(fParentReader)
        return fParentReader->getGrammar(nameSpaceKey);
    return NULL;
}

// -----------------------------------------------------------------------
//  Implementation of the EntityResolver interface
// -----------------------------------------------------------------------
InputSource* SAX2XMLFilterImpl::resolveEntity(const XMLCh* const    publicId
                                            , const XMLCh* const    systemId)
{
    if(fEntityResolver)
        return fEntityResolver->resolveEntity(publicId, systemId);
    return 0;
}

// -----------------------------------------------------------------------
//  Implementation of the DTDHandler interface
// -----------------------------------------------------------------------
void SAX2XMLFilterImpl::notationDecl( const XMLCh* const    name
                                    , const XMLCh* const    publicId
                                    , const XMLCh* const    systemId)
{
    if(fDTDHandler)
        fDTDHandler->notationDecl(name, publicId, systemId);
}

void SAX2XMLFilterImpl::unparsedEntityDecl(const XMLCh* const    name
                                         , const XMLCh* const    publicId
                                         , const XMLCh* const    systemId
                                         , const XMLCh* const    notationName)
{
    if(fDTDHandler)
        fDTDHandler->unparsedEntityDecl(name, publicId, systemId, notationName);
}

void SAX2XMLFilterImpl::resetDocType()
{
    if(fDTDHandler)
        fDTDHandler->resetDocType();
}

// -----------------------------------------------------------------------
//  Implementation of the ContentHandler interface
// -----------------------------------------------------------------------

void SAX2XMLFilterImpl::characters(const XMLCh* const    chars
                                 , const XMLSize_t       length)
{
    if(fDocHandler)
        fDocHandler->characters(chars, length);
}

void SAX2XMLFilterImpl::endDocument()
{
    if(fDocHandler)
        fDocHandler->endDocument();
}

void SAX2XMLFilterImpl::endElement(const XMLCh* const uri
		                         , const XMLCh* const localname
		                         , const XMLCh* const qname)
{
    if(fDocHandler)
        fDocHandler->endElement(uri, localname, qname);
}

void SAX2XMLFilterImpl::ignorableWhitespace(const XMLCh* const    chars
                                          , const XMLSize_t       length)
{
    if(fDocHandler)
        fDocHandler->ignorableWhitespace(chars, length);
}

void SAX2XMLFilterImpl::processingInstruction(const XMLCh* const    target
                                            , const XMLCh* const    data)
{
    if(fDocHandler)
        fDocHandler->processingInstruction(target, data);
}

void SAX2XMLFilterImpl::setDocumentLocator(const Locator* const locator)
{
    if(fDocHandler)
        fDocHandler->setDocumentLocator(locator);
}

void SAX2XMLFilterImpl::startDocument()
{
    if(fDocHandler)
        fDocHandler->startDocument();
}

void SAX2XMLFilterImpl::startElement(const   XMLCh* const    uri
                                   , const   XMLCh* const    localname
                                   , const   XMLCh* const    qname
                                   , const   Attributes&     attrs)
{
    if(fDocHandler)
        fDocHandler->startElement(uri, localname, qname, attrs);
}

void SAX2XMLFilterImpl::startPrefixMapping(const	XMLCh* const	prefix
                                    	,  const	XMLCh* const	uri)
{
    if(fDocHandler)
        fDocHandler->startPrefixMapping(prefix, uri);
}

void SAX2XMLFilterImpl::endPrefixMapping(const XMLCh* const	prefix)
{
    if(fDocHandler)
        fDocHandler->endPrefixMapping(prefix);
}

void SAX2XMLFilterImpl::skippedEntity(const	XMLCh* const	name)
{
    if(fDocHandler)
        fDocHandler->skippedEntity(name);
}

// -----------------------------------------------------------------------
//  Implementation of the ErrorHandler interface
// -----------------------------------------------------------------------

void SAX2XMLFilterImpl::warning(const SAXParseException& exc)
{
    if(fErrorHandler)
        fErrorHandler->warning(exc);
}

void SAX2XMLFilterImpl::error(const SAXParseException& exc)
{
    if(fErrorHandler)
        fErrorHandler->error(exc);
}

void SAX2XMLFilterImpl::fatalError(const SAXParseException& exc)
{
    if(fErrorHandler)
        fErrorHandler->fatalError(exc);
}

void SAX2XMLFilterImpl::resetErrors()
{
    if(fErrorHandler)
        fErrorHandler->resetErrors();
}

XERCES_CPP_NAMESPACE_END
