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

/**
*  This file contains code to build the DOM tree. It registers a document
*  handler with the scanner. In these handler methods, appropriate DOM nodes
*  are created and added to the DOM tree.
*
* $Id: XercesDOMParser.cpp 882548 2009-11-20 13:44:14Z borisk $
*
*/



// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/sax/EntityResolver.hpp>
#include <xercesc/sax/ErrorHandler.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/util/IOException.hpp>
#include <xercesc/internal/XMLScanner.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/validators/common/GrammarResolver.hpp>
#include <xercesc/framework/XMLGrammarPool.hpp>
#include <xercesc/framework/XMLSchemaDescription.hpp>
#include <xercesc/util/Janitor.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/util/XMLEntityResolver.hpp>

XERCES_CPP_NAMESPACE_BEGIN



// ---------------------------------------------------------------------------
//  XercesDOMParser: Constructors and Destructor
// ---------------------------------------------------------------------------
XercesDOMParser::XercesDOMParser( XMLValidator* const   valToAdopt
                                , MemoryManager* const  manager
                                , XMLGrammarPool* const gramPool):

AbstractDOMParser(valToAdopt, manager, gramPool)
, fEntityResolver(0)
, fXMLEntityResolver(0)
, fErrorHandler(0)
{
}


XercesDOMParser::~XercesDOMParser()
{
}


// ---------------------------------------------------------------------------
//  XercesDOMParser: Getter methods
// ---------------------------------------------------------------------------
bool XercesDOMParser::isCachingGrammarFromParse() const
{
    return getScanner()->isCachingGrammarFromParse();
}

bool XercesDOMParser::isUsingCachedGrammarInParse() const
{
    return getScanner()->isUsingCachedGrammarInParse();
}

Grammar* XercesDOMParser::getGrammar(const XMLCh* const nameSpaceKey)
{
    return getGrammarResolver()->getGrammar(nameSpaceKey);
}

Grammar* XercesDOMParser::getRootGrammar()
{
    return getScanner()->getRootGrammar();
}

const XMLCh* XercesDOMParser::getURIText(unsigned int uriId) const
{
    return getScanner()->getURIText(uriId);
}

XMLFilePos XercesDOMParser::getSrcOffset() const
{
    return getScanner()->getSrcOffset();
}

bool XercesDOMParser::getIgnoreCachedDTD() const
{
    return getScanner()->getIgnoreCachedDTD();
}

// ---------------------------------------------------------------------------
//  XercesDOMParser: Setter methods
// ---------------------------------------------------------------------------
void XercesDOMParser::setErrorHandler(ErrorHandler* const handler)
{
    fErrorHandler = handler;
    XMLScanner* scanner = getScanner();
    if (fErrorHandler) {
        scanner->setErrorReporter(this);
        scanner->setErrorHandler(fErrorHandler);
    }
    else {
        scanner->setErrorReporter(0);
        scanner->setErrorHandler(0);
    }
}

void XercesDOMParser::setEntityResolver(EntityResolver* const handler)
{
    fEntityResolver = handler;
    if (fEntityResolver) {
        getScanner()->setEntityHandler(this);
        fXMLEntityResolver = 0;
    }
    else {
        getScanner()->setEntityHandler(0);
    }
}

void XercesDOMParser::setXMLEntityResolver(XMLEntityResolver* const handler)
{
    fXMLEntityResolver = handler;
    if (fXMLEntityResolver) {
        getScanner()->setEntityHandler(this);
        fEntityResolver = 0;
    }
    else {
        getScanner()->setEntityHandler(0);
    }
}

void XercesDOMParser::cacheGrammarFromParse(const bool newState)
{
    getScanner()->cacheGrammarFromParse(newState);

    if (newState)
        getScanner()->useCachedGrammarInParse(newState);
}

void XercesDOMParser::useCachedGrammarInParse(const bool newState)
{
    if (newState || !getScanner()->isCachingGrammarFromParse())
        getScanner()->useCachedGrammarInParse(newState);
}

void XercesDOMParser::setIgnoreCachedDTD(const bool newValue)
{
    getScanner()->setIgnoredCachedDTD(newValue);
}

// ---------------------------------------------------------------------------
//  XercesDOMParser: Utilities
// ---------------------------------------------------------------------------
void XercesDOMParser::resetDocumentPool()
{
    resetPool();
}


// ---------------------------------------------------------------------------
//  XercesDOMParser: Implementation of the XMLErrorReporter interface
// ---------------------------------------------------------------------------
void XercesDOMParser::error( const   unsigned int
                             , const XMLCh* const
                             , const XMLErrorReporter::ErrTypes  errType
                             , const XMLCh* const                errorText
                             , const XMLCh* const                systemId
                             , const XMLCh* const                publicId
                             , const XMLFileLoc                  lineNum
                             , const XMLFileLoc                  colNum)
{
    SAXParseException toThrow = SAXParseException
        (
        errorText
        , publicId
        , systemId
        , lineNum
        , colNum
        , getMemoryManager()
        );

    //
    //  If there is an error handler registered, call it, otherwise ignore
    //  all but the fatal errors.
    //
    if (!fErrorHandler)
    {
        if (errType == XMLErrorReporter::ErrType_Fatal)
            throw toThrow;
        return;
    }

    if (errType == XMLErrorReporter::ErrType_Warning)
        fErrorHandler->warning(toThrow);
    else if (errType >= XMLErrorReporter::ErrType_Fatal)
        fErrorHandler->fatalError(toThrow);
    else
        fErrorHandler->error(toThrow);
}

void XercesDOMParser::resetErrors()
{
}


// ---------------------------------------------------------------------------
//  XercesDOMParser: Implementation of XMLEntityHandler interface
// ---------------------------------------------------------------------------
InputSource*
XercesDOMParser::resolveEntity(XMLResourceIdentifier* resourceIdentifier)
{
    //
    //  Just map it to the SAX entity resolver. If there is not one installed,
    //  return a null pointer to cause the default resolution.
    //
    if (fEntityResolver)
        return fEntityResolver->resolveEntity(resourceIdentifier->getPublicId(),
                                                resourceIdentifier->getSystemId());
    if (fXMLEntityResolver)
        return fXMLEntityResolver->resolveEntity(resourceIdentifier);

    return 0;
}

typedef JanitorMemFunCall<XercesDOMParser>  ResetParseType;

// ---------------------------------------------------------------------------
//  XercesDOMParser: Grammar preparsing methods
// ---------------------------------------------------------------------------
Grammar* XercesDOMParser::loadGrammar(const char* const systemId,
                                      const Grammar::GrammarType grammarType,
                                      const bool toCache)
{
    // Avoid multiple entrance
    if (getParseInProgress())
        ThrowXMLwithMemMgr(IOException, XMLExcepts::Gen_ParseInProgress, fMemoryManager);

    ResetParseType  resetParse(this, &XercesDOMParser::resetParse);

    Grammar* grammar = 0;

    try
    {
        setParseInProgress(true);
        if (grammarType == Grammar::DTDGrammarType)
            getScanner()->setDocTypeHandler(0);
        grammar = getScanner()->loadGrammar(systemId, grammarType, toCache);
    }
    catch(const OutOfMemoryException&)
    {
        resetParse.release();

        throw;
    }

    return grammar;
}

Grammar* XercesDOMParser::loadGrammar(const XMLCh* const systemId,
                                      const Grammar::GrammarType grammarType,
                                      const bool toCache)
{
    // Avoid multiple entrance
    if (getParseInProgress())
        ThrowXMLwithMemMgr(IOException, XMLExcepts::Gen_ParseInProgress, fMemoryManager);

    ResetParseType  resetParse(this, &XercesDOMParser::resetParse);

    Grammar* grammar = 0;

    try
    {
        setParseInProgress(true);
        if (grammarType == Grammar::DTDGrammarType)
            getScanner()->setDocTypeHandler(0);
        grammar = getScanner()->loadGrammar(systemId, grammarType, toCache);
    }
    catch(const OutOfMemoryException&)
    {
        resetParse.release();

        throw;
    }

    return grammar;
}

Grammar* XercesDOMParser::loadGrammar(const InputSource& source,
                                      const Grammar::GrammarType grammarType,
                                      const bool toCache)
{
    // Avoid multiple entrance
    if (getParseInProgress())
        ThrowXMLwithMemMgr(IOException, XMLExcepts::Gen_ParseInProgress, fMemoryManager);

    ResetParseType  resetParse(this, &XercesDOMParser::resetParse);

    Grammar* grammar = 0;

    try
    {
        setParseInProgress(true);
        if (grammarType == Grammar::DTDGrammarType)
            getScanner()->setDocTypeHandler(0);
        grammar = getScanner()->loadGrammar(source, grammarType, toCache);
    }
    catch(const OutOfMemoryException&)
    {
        resetParse.release();

        throw;
    }

    return grammar;
}

void XercesDOMParser::resetParse()
{
    if (getScanner()->getDocTypeHandler() == 0)
    {
        getScanner()->setDocTypeHandler(this);
    }

    setParseInProgress(false);
}

void XercesDOMParser::resetCachedGrammarPool()
{
    getGrammarResolver()->resetCachedGrammar();
    getScanner()->resetCachedGrammar();
}

XERCES_CPP_NAMESPACE_END
