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
* $Id: DOMLSParserImpl.cpp 882548 2009-11-20 13:44:14Z borisk $
*
*/



// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/parsers/DOMLSParserImpl.hpp>
#include <xercesc/dom/DOMLSResourceResolver.hpp>
#include <xercesc/dom/DOMErrorHandler.hpp>
#include <xercesc/dom/DOMLSParserFilter.hpp>
#include <xercesc/dom/DOMNodeFilter.hpp>
#include <xercesc/dom/impl/DOMErrorImpl.hpp>
#include <xercesc/dom/impl/DOMLocatorImpl.hpp>
#include <xercesc/dom/impl/DOMConfigurationImpl.hpp>
#include <xercesc/dom/impl/DOMStringListImpl.hpp>
#include <xercesc/dom/impl/DOMDocumentImpl.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOMLSException.hpp>
#include <xercesc/dom/DOMDocumentFragment.hpp>
#include <xercesc/dom/DOMNamedNodeMap.hpp>
#include <xercesc/internal/XMLScanner.hpp>
#include <xercesc/framework/Wrapper4DOMLSInput.hpp>
#include <xercesc/framework/XMLGrammarPool.hpp>
#include <xercesc/framework/XMLSchemaDescription.hpp>
#include <xercesc/util/Janitor.hpp>
#include <xercesc/validators/common/GrammarResolver.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/util/XMLEntityResolver.hpp>
#include <xercesc/util/RuntimeException.hpp>
#include <xercesc/util/XMLDOMMsg.hpp>

XERCES_CPP_NAMESPACE_BEGIN


// ---------------------------------------------------------------------------
//  A filter used to abort processing
// ---------------------------------------------------------------------------
class __AbortFilter : public DOMLSParserFilter
{
public:
    __AbortFilter() {}
    virtual FilterAction acceptNode(DOMNode*)             { return FILTER_INTERRUPT; }
    virtual FilterAction startElement(DOMElement* )       { return FILTER_INTERRUPT; }
    virtual DOMNodeFilter::ShowType getWhatToShow() const { return DOMNodeFilter::SHOW_ALL; }
};

static __AbortFilter g_AbortFilter;

// ---------------------------------------------------------------------------
//  DOMLSParserImpl: Constructors and Destructor
// ---------------------------------------------------------------------------
DOMLSParserImpl::DOMLSParserImpl( XMLValidator* const   valToAdopt
                              , MemoryManager* const  manager
                              , XMLGrammarPool* const gramPool) :

AbstractDOMParser(valToAdopt, manager, gramPool)
, fEntityResolver(0)
, fXMLEntityResolver(0)
, fErrorHandler(0)
, fFilter(0)
, fCharsetOverridesXMLEncoding(true)
, fUserAdoptsDocument(false)
, fSupportedParameters(0)
, fFilterAction(0)
, fFilterDelayedTextNodes(0)
, fWrapNodesInDocumentFragment(0)
, fWrapNodesContext(0)
{
    // dom spec has different default from scanner's default, so set explicitly
    getScanner()->setNormalizeData(false);

    fSupportedParameters=new (fMemoryManager) DOMStringListImpl(48, manager);
    fSupportedParameters->add(XMLUni::fgDOMResourceResolver);
    fSupportedParameters->add(XMLUni::fgDOMErrorHandler);
    fSupportedParameters->add(XMLUni::fgXercesEntityResolver);
    fSupportedParameters->add(XMLUni::fgXercesSchemaExternalSchemaLocation);
	fSupportedParameters->add(XMLUni::fgXercesSchemaExternalNoNameSpaceSchemaLocation);
	fSupportedParameters->add(XMLUni::fgXercesSecurityManager);
	fSupportedParameters->add(XMLUni::fgXercesScannerName);
    fSupportedParameters->add(XMLUni::fgXercesParserUseDocumentFromImplementation);
    fSupportedParameters->add(XMLUni::fgDOMCharsetOverridesXMLEncoding);
    fSupportedParameters->add(XMLUni::fgDOMDisallowDoctype);
    fSupportedParameters->add(XMLUni::fgDOMIgnoreUnknownCharacterDenormalization);
    fSupportedParameters->add(XMLUni::fgDOMNamespaces);
    fSupportedParameters->add(XMLUni::fgDOMSupportedMediatypesOnly);
    fSupportedParameters->add(XMLUni::fgDOMValidate);
    fSupportedParameters->add(XMLUni::fgDOMValidateIfSchema);
    fSupportedParameters->add(XMLUni::fgDOMWellFormed);
    fSupportedParameters->add(XMLUni::fgDOMCanonicalForm);
    fSupportedParameters->add(XMLUni::fgDOMCDATASections);
    fSupportedParameters->add(XMLUni::fgDOMCheckCharacterNormalization);
    fSupportedParameters->add(XMLUni::fgDOMComments);
    fSupportedParameters->add(XMLUni::fgDOMDatatypeNormalization);
    fSupportedParameters->add(XMLUni::fgDOMElementContentWhitespace);
    fSupportedParameters->add(XMLUni::fgDOMEntities);
    fSupportedParameters->add(XMLUni::fgDOMNamespaceDeclarations);
    fSupportedParameters->add(XMLUni::fgDOMNormalizeCharacters);
    fSupportedParameters->add(XMLUni::fgDOMSchemaLocation);
    fSupportedParameters->add(XMLUni::fgDOMSchemaType);
    fSupportedParameters->add(XMLUni::fgDOMSplitCDATASections);
    fSupportedParameters->add(XMLUni::fgDOMInfoset);
    fSupportedParameters->add(XMLUni::fgXercesSchema);
    fSupportedParameters->add(XMLUni::fgXercesSchemaFullChecking);
    fSupportedParameters->add(XMLUni::fgXercesUserAdoptsDOMDocument);
    fSupportedParameters->add(XMLUni::fgXercesLoadExternalDTD);
    fSupportedParameters->add(XMLUni::fgXercesLoadSchema);
    fSupportedParameters->add(XMLUni::fgXercesContinueAfterFatalError);
    fSupportedParameters->add(XMLUni::fgXercesValidationErrorAsFatal);
    fSupportedParameters->add(XMLUni::fgXercesCacheGrammarFromParse);
    fSupportedParameters->add(XMLUni::fgXercesUseCachedGrammarInParse);
    fSupportedParameters->add(XMLUni::fgXercesCalculateSrcOfs);
    fSupportedParameters->add(XMLUni::fgXercesStandardUriConformant);
    fSupportedParameters->add(XMLUni::fgXercesDOMHasPSVIInfo);
    fSupportedParameters->add(XMLUni::fgXercesGenerateSyntheticAnnotations);
    fSupportedParameters->add(XMLUni::fgXercesValidateAnnotations);
    fSupportedParameters->add(XMLUni::fgXercesIdentityConstraintChecking);
    fSupportedParameters->add(XMLUni::fgXercesIgnoreCachedDTD);
    fSupportedParameters->add(XMLUni::fgXercesIgnoreAnnotations);
    fSupportedParameters->add(XMLUni::fgXercesDisableDefaultEntityResolution);
    fSupportedParameters->add(XMLUni::fgXercesSkipDTDValidation);
    fSupportedParameters->add(XMLUni::fgXercesDoXInclude);
    fSupportedParameters->add(XMLUni::fgXercesHandleMultipleImports);

    // LSParser by default does namespace processing
    setDoNamespaces(true);
}


DOMLSParserImpl::~DOMLSParserImpl()
{
    delete fSupportedParameters;
    delete fFilterAction;
    delete fFilterDelayedTextNodes;
}


// ---------------------------------------------------------------------------
//  DOMLSParserImpl: Setter methods
// ---------------------------------------------------------------------------
bool DOMLSParserImpl::getBusy() const
{
    return getParseInProgress();
}

// ---------------------------------------------------------------------------
//  DOMLSParserImpl: Setter methods
// ---------------------------------------------------------------------------
void DOMLSParserImpl::setFilter(DOMLSParserFilter* const filter)
{
    fFilter = filter;
}

// ---------------------------------------------------------------------------
//  DOMLSParserImpl: DOMConfiguration methods
// ---------------------------------------------------------------------------
void DOMLSParserImpl::setParameter(const XMLCh* name, const void* value)
{
    if (XMLString::compareIStringASCII(name, XMLUni::fgDOMResourceResolver) == 0)
    {
        fEntityResolver = (DOMLSResourceResolver*)value;
        if (fEntityResolver) {
            getScanner()->setEntityHandler(this);
            fXMLEntityResolver = 0;
        }
        else {
            getScanner()->setEntityHandler(0);
        }
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMErrorHandler) == 0)
    {
        fErrorHandler = (DOMErrorHandler*)value;
        if (fErrorHandler) {
            getScanner()->setErrorReporter(this);
        }
        else {
            getScanner()->setErrorReporter(0);
        }
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMSchemaLocation) == 0)
    {
        // TODO
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMSchemaType) == 0)
    {
        // TODO
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesEntityResolver) == 0)
    {
        fXMLEntityResolver = (XMLEntityResolver*)value;
        if (fXMLEntityResolver) {
            getScanner()->setEntityHandler(this);
            fEntityResolver = 0;
        }
        else {
          getScanner()->setEntityHandler(0);
        }
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesSchemaExternalSchemaLocation) == 0)
    {
      setExternalSchemaLocation((XMLCh*)value);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesSchemaExternalNoNameSpaceSchemaLocation) == 0)
    {
      setExternalNoNamespaceSchemaLocation((XMLCh*)value);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesSecurityManager) == 0)
    {
      setSecurityManager((SecurityManager*)value);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesScannerName) == 0)
    {
        AbstractDOMParser::useScanner((const XMLCh*) value);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesParserUseDocumentFromImplementation) == 0)
    {
        useImplementation((const XMLCh*) value);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesLowWaterMark) == 0)
    {
        setLowWaterMark(*(const XMLSize_t*)value);
    }
    else
        throw DOMException(DOMException::NOT_FOUND_ERR, 0, getMemoryManager());
}

void DOMLSParserImpl::setParameter(const XMLCh* name, bool state)
{
    if (XMLString::compareIStringASCII(name, XMLUni::fgDOMCharsetOverridesXMLEncoding) == 0)
    {
        // in fact, setting this has no effect to the parser
        fCharsetOverridesXMLEncoding = state;
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMDisallowDoctype) == 0)
    {
        // TODO
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMIgnoreUnknownCharacterDenormalization) == 0)
    {
        // TODO
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMNamespaces) == 0)
    {
        setDoNamespaces(state);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMSupportedMediatypesOnly) == 0)
    {
        if (state)
            throw DOMException(DOMException::NOT_SUPPORTED_ERR, 0, getMemoryManager());
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMValidate) == 0)
    {
        if (state) {
            if (getValidationScheme() == AbstractDOMParser::Val_Never)
                setValidationScheme(AbstractDOMParser::Val_Always);
        }
        else {
            setValidationScheme(AbstractDOMParser::Val_Never);
        }
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMValidateIfSchema) == 0)
    {
        if (state) {
            setValidationScheme(AbstractDOMParser::Val_Auto);
        }
        else {
            setValidationScheme(AbstractDOMParser::Val_Never);
        }
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMWellFormed) == 0)
    {
        if(state==false)
            throw DOMException(DOMException::NOT_SUPPORTED_ERR, 0, getMemoryManager());
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMCanonicalForm) == 0 )
    {
        // TODO
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMCDATASections) == 0 )
    {
        // TODO
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMCheckCharacterNormalization) == 0 )
    {
        // TODO
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMComments) == 0)
    {
        setCreateCommentNodes(state);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMDatatypeNormalization) == 0)
    {
        getScanner()->setNormalizeData(state);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMElementContentWhitespace) == 0)
    {
        setIncludeIgnorableWhitespace(state);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMEntities) == 0)
    {
        setCreateEntityReferenceNodes(state);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMNamespaceDeclarations) == 0)
    {
        if (state==false)
            throw DOMException(DOMException::NOT_SUPPORTED_ERR, 0, getMemoryManager());
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMNormalizeCharacters) == 0)
    {
        // TODO
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMSplitCDATASections) == 0)
    {
        // TODO
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMInfoset) == 0)
    {
        if (!state)
            throw DOMException(DOMException::NOT_SUPPORTED_ERR, 0, getMemoryManager());
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesSchema) == 0)
    {
        setDoSchema(state);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesSchemaFullChecking) == 0)
    {
        setValidationSchemaFullChecking(state);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesUserAdoptsDOMDocument) == 0)
    {
        if(state)
            fUserAdoptsDocument = true;
        else
            fUserAdoptsDocument = false;
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesLoadExternalDTD) == 0)
    {
        setLoadExternalDTD(state);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesLoadSchema) == 0)
    {
        setLoadSchema(state);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesContinueAfterFatalError) == 0)
    {
        setExitOnFirstFatalError(!state);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesValidationErrorAsFatal) == 0)
    {
        setValidationConstraintFatal(state);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesCacheGrammarFromParse) == 0)
    {
        getScanner()->cacheGrammarFromParse(state);

        if (state)
            getScanner()->useCachedGrammarInParse(state);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesUseCachedGrammarInParse) == 0)
    {
        if (state || !getScanner()->isCachingGrammarFromParse())
            getScanner()->useCachedGrammarInParse(state);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesCalculateSrcOfs) == 0)
    {
        getScanner()->setCalculateSrcOfs(state);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesStandardUriConformant) == 0)
    {
        getScanner()->setStandardUriConformant(state);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesDOMHasPSVIInfo) == 0)
    {
        setCreateSchemaInfo(state);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesGenerateSyntheticAnnotations) == 0)
    {
        getScanner()->setGenerateSyntheticAnnotations(state);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesValidateAnnotations) == 0)
    {
        getScanner()->setValidateAnnotations(state);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesIdentityConstraintChecking) == 0)
    {
        getScanner()->setIdentityConstraintChecking(state);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesIgnoreCachedDTD) == 0)
    {
        getScanner()->setIgnoredCachedDTD(state);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesIgnoreAnnotations) == 0)
    {
        getScanner()->setIgnoreAnnotations(state);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesDisableDefaultEntityResolution) == 0)
    {
        getScanner()->setDisableDefaultEntityResolution(state);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesSkipDTDValidation) == 0)
    {
        getScanner()->setSkipDTDValidation(state);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesDoXInclude) == 0)
    {
        setDoXInclude(state);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesHandleMultipleImports) == 0)
    {
        getScanner()->setHandleMultipleImports(state);
    }
    else
        throw DOMException(DOMException::NOT_FOUND_ERR, 0, getMemoryManager());
}

const void* DOMLSParserImpl::getParameter(const XMLCh* name) const
{
    if (XMLString::compareIStringASCII(name, XMLUni::fgDOMCharsetOverridesXMLEncoding) == 0)
    {
        return (void*)fCharsetOverridesXMLEncoding;
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMDisallowDoctype) == 0)
    {
        // TODO
        return 0;
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMIgnoreUnknownCharacterDenormalization) == 0)
    {
        // TODO
        return 0;
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMNamespaces) == 0)
    {
        return (void*)getDoNamespaces();
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMResourceResolver) == 0)
    {
        return fEntityResolver;
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMSupportedMediatypesOnly) == 0)
    {
        return (void*)false;
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMValidate) == 0)
    {
        return (void*)(getValidationScheme() != AbstractDOMParser::Val_Never);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMValidateIfSchema) == 0)
    {
        return (void*)(getValidationScheme() == AbstractDOMParser::Val_Auto);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMWellFormed) == 0)
    {
        return (void*)true;
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMCanonicalForm) == 0 )
    {
        // TODO
        return 0;
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMCDATASections) == 0 )
    {
        // TODO
        return 0;
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMCheckCharacterNormalization) == 0 )
    {
        // TODO
        return 0;
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMComments) == 0)
    {
        return (void*)getCreateCommentNodes();
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMDatatypeNormalization) == 0)
    {
        return (void*)getScanner()->getNormalizeData();
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMElementContentWhitespace) == 0)
    {
        return (void*)getIncludeIgnorableWhitespace();
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMEntities) == 0)
    {
        return (void*)getCreateEntityReferenceNodes();
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMErrorHandler) == 0)
    {
        return fErrorHandler;
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMNamespaceDeclarations) == 0)
    {
        return (void*)true;
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMNormalizeCharacters) == 0)
    {
        return 0;
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMSchemaLocation) == 0)
    {
        return 0;
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMSchemaType) == 0)
    {
        return 0;
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMSplitCDATASections) == 0)
    {
        return 0;
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMInfoset) == 0)
    {
        return (void*)true;
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesSchema) == 0)
    {
        return (void*)getDoSchema();
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesSchemaFullChecking) == 0)
    {
        return (void*)getValidationSchemaFullChecking();
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesIdentityConstraintChecking) == 0)
    {
        return (void*)getIdentityConstraintChecking();
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesLoadExternalDTD) == 0)
    {
        return (void*)getLoadExternalDTD();
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesLoadSchema) == 0)
    {
        return (void*)getLoadSchema();
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesContinueAfterFatalError) == 0)
    {
        return (void*)!getExitOnFirstFatalError();
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesValidationErrorAsFatal) == 0)
    {
        return (void*)getValidationConstraintFatal();
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesCacheGrammarFromParse) == 0)
    {
        return (void*)getScanner()->isCachingGrammarFromParse();
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesUseCachedGrammarInParse) == 0)
    {
        return (void*)getScanner()->isUsingCachedGrammarInParse();
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesCalculateSrcOfs) == 0)
    {
        return (void*)getScanner()->getCalculateSrcOfs();
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesStandardUriConformant) == 0)
    {
        return (void*)getScanner()->getStandardUriConformant();
    }
    else if(XMLString::compareIStringASCII(name, XMLUni::fgXercesUserAdoptsDOMDocument) == 0)
    {
        return (void*)fUserAdoptsDocument;
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesDOMHasPSVIInfo) == 0)
    {
        return (void*)getCreateSchemaInfo();
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesGenerateSyntheticAnnotations) == 0)
    {
        return (void*)getScanner()->getGenerateSyntheticAnnotations();
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesValidateAnnotations) == 0)
    {
        return (void*)getScanner()->getValidateAnnotations();
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesIgnoreCachedDTD) == 0)
    {
        return (void*)getScanner()->getIgnoreCachedDTD();
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesIgnoreAnnotations) == 0)
    {
        return (void*)getScanner()->getIgnoreAnnotations();
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesDisableDefaultEntityResolution) == 0)
    {
        return (void*)getScanner()->getDisableDefaultEntityResolution();
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesSkipDTDValidation) == 0)
    {
        return (void*)getScanner()->getSkipDTDValidation();
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesHandleMultipleImports) == 0)
    {
        return (void*)getScanner()->getHandleMultipleImports();
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesEntityResolver) == 0)
    {
        return fXMLEntityResolver;
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesSchemaExternalSchemaLocation) == 0)
    {
        return getExternalSchemaLocation();
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesSchemaExternalNoNameSpaceSchemaLocation) == 0)
    {
        return getExternalNoNamespaceSchemaLocation();
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesSecurityManager) == 0)
    {
        return getSecurityManager();
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesDoXInclude) == 0)
    {
        return (void *)getDoXInclude();
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesLowWaterMark) == 0)
    {
      return (void*)&getLowWaterMark();
    }
    else
        throw DOMException(DOMException::NOT_FOUND_ERR, 0, getMemoryManager());
}

bool DOMLSParserImpl::canSetParameter(const XMLCh* name, const void* /*value*/) const
{
    if (XMLString::compareIStringASCII(name, XMLUni::fgDOMResourceResolver) == 0 ||
        XMLString::compareIStringASCII(name, XMLUni::fgDOMErrorHandler) == 0 ||
        XMLString::compareIStringASCII(name, XMLUni::fgXercesEntityResolver) == 0 ||
        XMLString::compareIStringASCII(name, XMLUni::fgXercesSchemaExternalSchemaLocation) == 0 ||
        XMLString::compareIStringASCII(name, XMLUni::fgXercesSchemaExternalNoNameSpaceSchemaLocation) == 0 ||
        XMLString::compareIStringASCII(name, XMLUni::fgXercesSecurityManager) == 0 ||
        XMLString::compareIStringASCII(name, XMLUni::fgXercesScannerName) == 0 ||
        XMLString::compareIStringASCII(name, XMLUni::fgXercesParserUseDocumentFromImplementation) == 0 ||
        XMLString::compareIStringASCII(name, XMLUni::fgXercesLowWaterMark) == 0)
      return true;
    else if(XMLString::compareIStringASCII(name, XMLUni::fgDOMSchemaLocation) == 0 ||
            XMLString::compareIStringASCII(name, XMLUni::fgDOMSchemaType) == 0)
      return false;

    return false;
}

bool DOMLSParserImpl::canSetParameter(const XMLCh* name, bool value) const
{
    if (XMLString::compareIStringASCII(name, XMLUni::fgDOMCharsetOverridesXMLEncoding) == 0 ||
        XMLString::compareIStringASCII(name, XMLUni::fgDOMNamespaces) == 0 ||
        XMLString::compareIStringASCII(name, XMLUni::fgDOMValidate) == 0 ||
        XMLString::compareIStringASCII(name, XMLUni::fgDOMValidateIfSchema) == 0 ||
        XMLString::compareIStringASCII(name, XMLUni::fgDOMComments) == 0 ||
        XMLString::compareIStringASCII(name, XMLUni::fgDOMDatatypeNormalization) == 0 ||
        XMLString::compareIStringASCII(name, XMLUni::fgDOMElementContentWhitespace) == 0 ||
        XMLString::compareIStringASCII(name, XMLUni::fgDOMEntities) == 0 ||
        XMLString::compareIStringASCII(name, XMLUni::fgXercesSchema) == 0 ||
        XMLString::compareIStringASCII(name, XMLUni::fgXercesSchemaFullChecking) == 0 ||
        XMLString::compareIStringASCII(name, XMLUni::fgXercesIdentityConstraintChecking) == 0 ||
        XMLString::compareIStringASCII(name, XMLUni::fgXercesLoadExternalDTD) == 0 ||
        XMLString::compareIStringASCII(name, XMLUni::fgXercesLoadSchema) == 0 ||
        XMLString::compareIStringASCII(name, XMLUni::fgXercesContinueAfterFatalError) == 0 ||
        XMLString::compareIStringASCII(name, XMLUni::fgXercesValidationErrorAsFatal) == 0 ||
        XMLString::compareIStringASCII(name, XMLUni::fgXercesCacheGrammarFromParse) == 0 ||
        XMLString::compareIStringASCII(name, XMLUni::fgXercesUseCachedGrammarInParse) == 0 ||
        XMLString::compareIStringASCII(name, XMLUni::fgXercesCalculateSrcOfs) == 0 ||
        XMLString::compareIStringASCII(name, XMLUni::fgXercesStandardUriConformant) == 0 ||
        XMLString::compareIStringASCII(name, XMLUni::fgXercesUserAdoptsDOMDocument) == 0 ||
        XMLString::compareIStringASCII(name, XMLUni::fgXercesDOMHasPSVIInfo) == 0 ||
        XMLString::compareIStringASCII(name, XMLUni::fgXercesGenerateSyntheticAnnotations) == 0 ||
        XMLString::compareIStringASCII(name, XMLUni::fgXercesValidateAnnotations) == 0 ||
        XMLString::compareIStringASCII(name, XMLUni::fgXercesIgnoreCachedDTD) == 0 ||
        XMLString::compareIStringASCII(name, XMLUni::fgXercesIgnoreAnnotations) == 0 ||
        XMLString::compareIStringASCII(name, XMLUni::fgXercesDisableDefaultEntityResolution) == 0 ||
        XMLString::compareIStringASCII(name, XMLUni::fgXercesSkipDTDValidation) == 0 ||
		XMLString::compareIStringASCII(name, XMLUni::fgXercesDoXInclude) == 0 ||
        XMLString::compareIStringASCII(name, XMLUni::fgXercesHandleMultipleImports) == 0)
      return true;
    else if(XMLString::compareIStringASCII(name, XMLUni::fgDOMDisallowDoctype) == 0 ||
            XMLString::compareIStringASCII(name, XMLUni::fgDOMIgnoreUnknownCharacterDenormalization) == 0 ||
            XMLString::compareIStringASCII(name, XMLUni::fgDOMCanonicalForm) == 0 ||
            XMLString::compareIStringASCII(name, XMLUni::fgDOMCDATASections) == 0 ||
            XMLString::compareIStringASCII(name, XMLUni::fgDOMCheckCharacterNormalization) == 0 ||
            XMLString::compareIStringASCII(name, XMLUni::fgDOMNormalizeCharacters) == 0 ||
            XMLString::compareIStringASCII(name, XMLUni::fgDOMSplitCDATASections) == 0)
      return false;
    else if(XMLString::compareIStringASCII(name, XMLUni::fgDOMSupportedMediatypesOnly) == 0)
      return value?false:true;
    else if(XMLString::compareIStringASCII(name, XMLUni::fgDOMWellFormed) == 0 ||
            XMLString::compareIStringASCII(name, XMLUni::fgDOMNamespaceDeclarations) == 0 ||
            XMLString::compareIStringASCII(name, XMLUni::fgDOMInfoset) == 0)
      return value?true:false;

    return false;
}

const DOMStringList* DOMLSParserImpl::getParameterNames() const
{
    return fSupportedParameters;
}

// ---------------------------------------------------------------------------
//  DOMLSParserImpl: Feature methods
// ---------------------------------------------------------------------------
void DOMLSParserImpl::release()
{
    DOMLSParserImpl* builder = (DOMLSParserImpl*) this;
    delete builder;
}

void DOMLSParserImpl::resetDocumentPool()
{
    resetPool();
}


// ---------------------------------------------------------------------------
//  DOMLSParserImpl: Parsing methods
// ---------------------------------------------------------------------------
DOMDocument* DOMLSParserImpl::parse(const DOMLSInput* source)
{
    if (getParseInProgress())
        throw DOMException(DOMException::INVALID_STATE_ERR, XMLDOMMsg::LSParser_ParseInProgress, fMemoryManager);

    // remove the abort filter, if present
    if(fFilter==&g_AbortFilter)
        fFilter=0;
    if(fFilterAction)
        fFilterAction->removeAll();
    if(fFilterDelayedTextNodes)
        fFilterDelayedTextNodes->removeAll();

    Wrapper4DOMLSInput isWrapper((DOMLSInput*)source, fEntityResolver, false, getMemoryManager());

    AbstractDOMParser::parse(isWrapper);

    // Disabled until 4.0.0. See XERCESC-1894 for details.
    //if(getErrorCount()!=0)
    //    throw DOMLSException(DOMLSException::PARSE_ERR, XMLDOMMsg::LSParser_ParsingFailed, fMemoryManager);

    if (fUserAdoptsDocument)
        return adoptDocument();
    else
        return getDocument();
}

DOMDocument* DOMLSParserImpl::parseURI(const XMLCh* const systemId)
{
    if (getParseInProgress())
        throw DOMException(DOMException::INVALID_STATE_ERR, XMLDOMMsg::LSParser_ParseInProgress, fMemoryManager);

    // remove the abort filter, if present
    if(fFilter==&g_AbortFilter)
        fFilter=0;
    if(fFilterAction)
        fFilterAction->removeAll();
    if(fFilterDelayedTextNodes)
        fFilterDelayedTextNodes->removeAll();

    AbstractDOMParser::parse(systemId);

    // Disabled until 4.0.0. See XERCESC-1894 for details.
    //if(getErrorCount()!=0)
    //    throw DOMLSException(DOMLSException::PARSE_ERR, XMLDOMMsg::LSParser_ParsingFailed, fMemoryManager);

    if (fUserAdoptsDocument)
        return adoptDocument();
    else
        return getDocument();
}

DOMDocument* DOMLSParserImpl::parseURI(const char* const systemId)
{
    if (getParseInProgress())
        throw DOMException(DOMException::INVALID_STATE_ERR, XMLDOMMsg::LSParser_ParseInProgress, fMemoryManager);

    // remove the abort filter, if present
    if(fFilter==&g_AbortFilter)
        fFilter=0;
    if(fFilterAction)
        fFilterAction->removeAll();
    if(fFilterDelayedTextNodes)
        fFilterDelayedTextNodes->removeAll();

    AbstractDOMParser::parse(systemId);

    // Disabled until 4.0.0. See XERCESC-1894 for details.
    //if(getErrorCount()!=0)
    //    throw DOMLSException(DOMLSException::PARSE_ERR, XMLDOMMsg::LSParser_ParsingFailed, fMemoryManager);

    if (fUserAdoptsDocument)
        return adoptDocument();
    else
        return getDocument();
}

void DOMLSParserImpl::startDocument()
{
    if(fWrapNodesInDocumentFragment)
    {
        fDocument = (DOMDocumentImpl*)fWrapNodesInDocumentFragment->getOwnerDocument();
        fCurrentParent = fCurrentNode = fWrapNodesInDocumentFragment;
        // set DOM error checking off
        fDocument->setErrorChecking(false);

        // if we have namespaces in scope, push them down to the reader
        ValueHashTableOf<unsigned int> inScopeNS(7, fMemoryManager);
        DOMNode* cursor = fWrapNodesContext;
        while(cursor)
        {
            if(cursor->getNodeType()==DOMNode::ELEMENT_NODE)
            {
                DOMNamedNodeMap* attrs = cursor->getAttributes();
                for(XMLSize_t i=0; i<attrs->getLength(); i++)
                {
                    DOMNode* attr = attrs->item(i);
                    if(XMLString::equals(attr->getNamespaceURI(), XMLUni::fgXMLNSURIName) && !inScopeNS.containsKey(attr->getLocalName()))
                        inScopeNS.put((void*)attr->getLocalName(), fScanner->getURIStringPool()->addOrFind(attr->getNodeValue()));
                    else if(XMLString::equals(attr->getNodeName(), XMLUni::fgXMLNSString) && !inScopeNS.containsKey(XMLUni::fgZeroLenString))
                        inScopeNS.put((void*)XMLUni::fgZeroLenString, fScanner->getURIStringPool()->addOrFind(attr->getNodeValue()));
                }
            }
            cursor = cursor->getParentNode();
        }
        ValueHashTableOfEnumerator<unsigned int> iter(&inScopeNS, false, fMemoryManager);
        while(iter.hasMoreElements())
        {
            XMLCh* prefix = (XMLCh*)iter.nextElementKey();
            fScanner->addGlobalPrefix(prefix, inScopeNS.get(prefix));
        }

        // in this case the document URI and the input encoding must be propagated to the context document
        if(fWrapNodesAction==ACTION_REPLACE_CHILDREN && fWrapNodesContext->getNodeType()==DOMNode::DOCUMENT_NODE)
        {
            fDocument->setDocumentURI(fScanner->getLocator()->getSystemId());
            fDocument->setInputEncoding(fScanner->getReaderMgr()->getCurrentEncodingStr());
        }
    }
    else
        AbstractDOMParser::startDocument();
}

void DOMLSParserImpl::XMLDecl(  const XMLCh* const    versionStr
                              , const XMLCh* const    encodingStr
                              , const XMLCh* const    standaloneStr
                              , const XMLCh* const    actualEncStr
                             )
{
    if(fWrapNodesInDocumentFragment && !(fWrapNodesAction==ACTION_REPLACE_CHILDREN && fWrapNodesContext->getNodeType()==DOMNode::DOCUMENT_NODE))
    {
        // don't change the properties for the context document, unless the context node is a
        // DOMDocument node and the action is ACTION_REPLACE_CHILDREN
    }
    else
        AbstractDOMParser::XMLDecl(versionStr, encodingStr, standaloneStr, actualEncStr);
}

DOMNode* DOMLSParserImpl::parseWithContext(const DOMLSInput* source,
                                           DOMNode* contextNode,
                                           const ActionType action)
{
    if (getParseInProgress())
        throw DOMException(DOMException::INVALID_STATE_ERR, XMLDOMMsg::LSParser_ParseInProgress, fMemoryManager);

    // remove the abort filter, if present
    if(fFilter==&g_AbortFilter)
        fFilter=0;
    if(fFilterAction)
        fFilterAction->removeAll();
    if(fFilterDelayedTextNodes)
        fFilterDelayedTextNodes->removeAll();

    DOMDocumentFragment* holder = contextNode->getOwnerDocument()->createDocumentFragment();
    // When parsing the input stream, the context node (or its parent, depending on where
    // the result will be inserted) is used for resolving unbound namespace prefixes
    if(action==ACTION_INSERT_BEFORE || action==ACTION_INSERT_AFTER || action==ACTION_REPLACE)
        fWrapNodesContext = contextNode->getParentNode();
    else
        fWrapNodesContext = contextNode;
    fWrapNodesInDocumentFragment = holder;
    fWrapNodesAction = action;
    // When calling parseWithContext, the values of the following configuration parameters
    // will be ignored and their default values will always be used instead: "validate",
    // "validate-if-schema", and "element-content-whitespace".
    ValSchemes oldValidate = getValidationScheme();
    setValidationScheme(Val_Never);
    bool oldElementContentWhitespace = getIncludeIgnorableWhitespace();
    setIncludeIgnorableWhitespace(true);

    Wrapper4DOMLSInput isWrapper((DOMLSInput*)source, fEntityResolver, false, getMemoryManager());
    AbstractDOMParser::parse(isWrapper);

    setValidationScheme(oldValidate);
    setIncludeIgnorableWhitespace(oldElementContentWhitespace);
    fWrapNodesContext = NULL;
    fWrapNodesInDocumentFragment = NULL;
    fDocument = NULL;

    if(getErrorCount()!=0)
    {
        holder->release();
        throw DOMLSException(DOMLSException::PARSE_ERR, XMLDOMMsg::LSParser_ParsingFailed, fMemoryManager);
    }

    DOMNode* result = holder->getFirstChild();
    DOMNode* node, *parent = contextNode->getParentNode();
    switch(action)
    {
    case ACTION_REPLACE_CHILDREN:
        // remove existing children
        while((node = contextNode->getFirstChild())!=NULL)
            contextNode->removeChild(node)->release();
        // then fall back to behave like an append
    case ACTION_APPEND_AS_CHILDREN:
        while((node = holder->getFirstChild())!=NULL)
            contextNode->appendChild(holder->removeChild(node));
        break;
    case ACTION_INSERT_BEFORE:
        while((node = holder->getFirstChild())!=NULL)
            parent->insertBefore(holder->removeChild(node), contextNode);
        break;
    case ACTION_INSERT_AFTER:
        while((node = holder->getLastChild())!=NULL)
            parent->insertBefore(holder->removeChild(node), contextNode->getNextSibling());
        break;
    case ACTION_REPLACE:
        while((node = holder->getFirstChild())!=NULL)
            parent->insertBefore(holder->removeChild(node), contextNode);
        parent->removeChild(contextNode)->release();
        break;
    }
    holder->release();

    // TODO whenever we add support for DOM Mutation Events:
    //   As the new data is inserted into the document, at least one mutation event is fired
    //   per new immediate child or sibling of the context node.
    return result;
}

void DOMLSParserImpl::abort()
{
    fFilter=&g_AbortFilter;
}

// ---------------------------------------------------------------------------
//  DOMLSParserImpl: Implementation of the XMLErrorReporter interface
// ---------------------------------------------------------------------------
void DOMLSParserImpl::error( const   unsigned int                code
                            , const XMLCh* const
                            , const XMLErrorReporter::ErrTypes  errType
                            , const XMLCh* const                errorText
                            , const XMLCh* const                systemId
                            , const XMLCh* const
                            , const XMLFileLoc                  lineNum
                            , const XMLFileLoc                  colNum)
{
    if (fErrorHandler) {

        DOMError::ErrorSeverity severity = DOMError::DOM_SEVERITY_ERROR;

        if (errType == XMLErrorReporter::ErrType_Warning)
            severity = DOMError::DOM_SEVERITY_WARNING;
        else if (errType == XMLErrorReporter::ErrType_Fatal)
            severity = DOMError::DOM_SEVERITY_FATAL_ERROR;

        DOMLocatorImpl location(lineNum, colNum, getCurrentNode(), systemId);
        if(getScanner()->getCalculateSrcOfs())
            location.setByteOffset(getScanner()->getSrcOffset());
        DOMErrorImpl domError(severity, errorText, &location);

        // if user return false, we should stop the process, so throw an error
        bool toContinueProcess = true;
        try
        {
            toContinueProcess = fErrorHandler->handleError(domError);
        }
        catch(...)
        {
        }
        if (!toContinueProcess && !getScanner()->getInException())
            throw (XMLErrs::Codes) code;
    }
}

void DOMLSParserImpl::resetErrors()
{
}


// ---------------------------------------------------------------------------
//  DOMLSParserImpl: Implementation of XMLEntityHandler interface
// ---------------------------------------------------------------------------
InputSource*
DOMLSParserImpl::resolveEntity( XMLResourceIdentifier* resourceIdentifier )
{
    //
    //  Just map it to the SAX entity resolver. If there is not one installed,
    //  return a null pointer to cause the default resolution.
    //
    if (fEntityResolver) {
        DOMLSInput* is = fEntityResolver->resolveResource(resourceIdentifier->getResourceIdentifierType()==XMLResourceIdentifier::ExternalEntity?XMLUni::fgDOMDTDType:XMLUni::fgDOMXMLSchemaType,
                                                          resourceIdentifier->getNameSpace(),
                                                          resourceIdentifier->getPublicId(),
                                                          resourceIdentifier->getSystemId(),
                                                          resourceIdentifier->getBaseURI());
        if (is)
            return new (getMemoryManager()) Wrapper4DOMLSInput(is, fEntityResolver, true, getMemoryManager());
    }
    if (fXMLEntityResolver) {
        return(fXMLEntityResolver->resolveEntity(resourceIdentifier));
    }

    return 0;
}

typedef JanitorMemFunCall<DOMLSParserImpl>    ResetParseType;

// ---------------------------------------------------------------------------
//  DOMLSParserImpl: Grammar preparsing methods
// ---------------------------------------------------------------------------
Grammar* DOMLSParserImpl::loadGrammar(const char* const systemId,
                                     const Grammar::GrammarType grammarType,
                                     const bool toCache)
{
    // Avoid multiple entrance
    if (getParseInProgress())
        throw DOMException(DOMException::INVALID_STATE_ERR, XMLDOMMsg::LSParser_ParseInProgress, fMemoryManager);

    ResetParseType  resetParse(this, &DOMLSParserImpl::resetParse);

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

Grammar* DOMLSParserImpl::loadGrammar(const XMLCh* const systemId,
                                     const Grammar::GrammarType grammarType,
                                     const bool toCache)
{
    // Avoid multiple entrance
    if (getParseInProgress())
        throw DOMException(DOMException::INVALID_STATE_ERR, XMLDOMMsg::LSParser_ParseInProgress, fMemoryManager);

    ResetParseType  resetParse(this, &DOMLSParserImpl::resetParse);

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

Grammar* DOMLSParserImpl::loadGrammar(const DOMLSInput* source,
                                     const Grammar::GrammarType grammarType,
                                     const bool toCache)
{
    // Avoid multiple entrance
    if (getParseInProgress())
        throw DOMException(DOMException::INVALID_STATE_ERR, XMLDOMMsg::LSParser_ParseInProgress, fMemoryManager);

    ResetParseType  resetParse(this, &DOMLSParserImpl::resetParse);

    Grammar* grammar = 0;

    try
    {
        setParseInProgress(true);
        if (grammarType == Grammar::DTDGrammarType)
            getScanner()->setDocTypeHandler(0);
        Wrapper4DOMLSInput isWrapper((DOMLSInput*)source, fEntityResolver, false, getMemoryManager());
        grammar = getScanner()->loadGrammar(isWrapper, grammarType, toCache);
    }
    catch(const OutOfMemoryException&)
    {
        resetParse.release();

        throw;
    }

    return grammar;
}

void DOMLSParserImpl::resetCachedGrammarPool()
{
    getGrammarResolver()->resetCachedGrammar();
    getScanner()->resetCachedGrammar();
}

void DOMLSParserImpl::resetParse()
{
    if (getScanner()->getDocTypeHandler() == 0)
    {
        getScanner()->setDocTypeHandler(this);
    }

    setParseInProgress(false);
}

Grammar* DOMLSParserImpl::getGrammar(const XMLCh* const nameSpaceKey) const
{
    return getGrammarResolver()->getGrammar(nameSpaceKey);
}

Grammar* DOMLSParserImpl::getRootGrammar() const
{
    return getScanner()->getRootGrammar();
}

const XMLCh* DOMLSParserImpl::getURIText(unsigned int uriId) const
{
    return getScanner()->getURIText(uriId);
}

XMLFilePos DOMLSParserImpl::getSrcOffset() const
{
    return getScanner()->getSrcOffset();
}

void DOMLSParserImpl::applyFilter(DOMNode* node)
{
    DOMLSParserFilter::FilterAction action;
    // if the parent was already rejected, reject this too
    if(fFilterAction && fFilterAction->containsKey(fCurrentParent) && fFilterAction->get(fCurrentParent)==DOMLSParserFilter::FILTER_REJECT)
        action = DOMLSParserFilter::FILTER_REJECT;
    else
        action = fFilter->acceptNode(node);

    switch(action)
    {
    case DOMLSParserFilter::FILTER_ACCEPT:      break;
    case DOMLSParserFilter::FILTER_REJECT:
    case DOMLSParserFilter::FILTER_SKIP:        if(node==fCurrentNode)
                                                    fCurrentNode = (node->getPreviousSibling()?node->getPreviousSibling():fCurrentParent);
                                                fCurrentParent->removeChild(node);
                                                node->release();
                                                break;
    case DOMLSParserFilter::FILTER_INTERRUPT:   throw DOMLSException(DOMLSException::PARSE_ERR, XMLDOMMsg::LSParser_ParsingAborted, fMemoryManager);
    }
}

void DOMLSParserImpl::docCharacters(const XMLCh* const    chars
                                  , const XMLSize_t       length
                                  , const bool            cdataSection)
{
    AbstractDOMParser::docCharacters(chars, length, cdataSection);
    if(fFilter)
    {
        // send the notification for the previous text node
        if(fFilterDelayedTextNodes && fCurrentNode->getPreviousSibling() && fFilterDelayedTextNodes->containsKey(fCurrentNode->getPreviousSibling()))
        {
            DOMNode* textNode = fCurrentNode->getPreviousSibling();
            fFilterDelayedTextNodes->removeKey(textNode);
            applyFilter(textNode);
        }
        DOMNodeFilter::ShowType whatToShow=fFilter->getWhatToShow();
        if(cdataSection && (whatToShow & DOMNodeFilter::SHOW_CDATA_SECTION))
        {
            applyFilter(fCurrentNode);
        }
        else if(!cdataSection && (whatToShow & DOMNodeFilter::SHOW_TEXT))
        {
            if(fFilterDelayedTextNodes==0)
                fFilterDelayedTextNodes=new (fMemoryManager) ValueHashTableOf<bool, PtrHasher>(7, fMemoryManager);
            fFilterDelayedTextNodes->put(fCurrentNode, true);
        }
    }
}

void DOMLSParserImpl::docComment(const XMLCh* const  comment)
{
    if(fFilter)
    {
        // send the notification for the previous text node
        if(fFilterDelayedTextNodes && fFilterDelayedTextNodes->containsKey(fCurrentNode))
        {
            fFilterDelayedTextNodes->removeKey(fCurrentNode);
            applyFilter(fCurrentNode);
        }
    }

    AbstractDOMParser::docComment(comment);
    if(fFilter)
    {
        DOMNodeFilter::ShowType whatToShow=fFilter->getWhatToShow();
        if(whatToShow & DOMNodeFilter::SHOW_COMMENT)
            applyFilter(fCurrentNode);
    }
}

void DOMLSParserImpl::docPI(const XMLCh* const    target
                          , const XMLCh* const    data)
{
    if(fFilter)
    {
        // send the notification for the previous text node
        if(fFilterDelayedTextNodes && fFilterDelayedTextNodes->containsKey(fCurrentNode))
        {
            fFilterDelayedTextNodes->removeKey(fCurrentNode);
            applyFilter(fCurrentNode);
        }
    }

    AbstractDOMParser::docPI(target, data);
    if(fFilter)
    {
        DOMNodeFilter::ShowType whatToShow=fFilter->getWhatToShow();
        if(whatToShow & DOMNodeFilter::SHOW_PROCESSING_INSTRUCTION)
            applyFilter(fCurrentNode);
    }
}

void DOMLSParserImpl::startEntityReference(const XMLEntityDecl& entDecl)
{
    if(fCreateEntityReferenceNodes && fFilter)
    {
        // send the notification for the previous text node
        if(fFilterDelayedTextNodes && fFilterDelayedTextNodes->containsKey(fCurrentNode))
        {
            fFilterDelayedTextNodes->removeKey(fCurrentNode);
            applyFilter(fCurrentNode);
        }
    }

    DOMNode* origParent = fCurrentParent;
    AbstractDOMParser::startEntityReference(entDecl);
    if (fCreateEntityReferenceNodes && fFilter)
    {
        if(fFilterAction && fFilterAction->containsKey(origParent) && fFilterAction->get(origParent)==DOMLSParserFilter::FILTER_REJECT)
            fFilterAction->put(fCurrentNode, DOMLSParserFilter::FILTER_REJECT);
    }
}

void DOMLSParserImpl::endElement(const XMLElementDecl& elemDecl
                               , const unsigned int    urlId
                               , const bool            isRoot
                               , const XMLCh* const    elemPrefix)
{
    if(fFilter)
    {
        // send the notification for the previous text node
        if(fFilterDelayedTextNodes && fFilterDelayedTextNodes->containsKey(fCurrentNode))
        {
            fFilterDelayedTextNodes->removeKey(fCurrentNode);
            applyFilter(fCurrentNode);
        }
    }

    AbstractDOMParser::endElement(elemDecl, urlId, isRoot, elemPrefix);
    if(fFilter)
    {
        DOMNodeFilter::ShowType whatToShow=fFilter->getWhatToShow();
        if(whatToShow & DOMNodeFilter::SHOW_ELEMENT)
        {
            DOMNode* thisNode = fCurrentNode;
            DOMLSParserFilter::FilterAction action;
            if(fFilterAction && fFilterAction->containsKey(thisNode))
            {
                action = fFilterAction->get(thisNode);
                fFilterAction->removeKey(thisNode);
            }
            else
                action = fFilter->acceptNode(thisNode);
            switch(action)
            {
            case DOMLSParserFilter::FILTER_ACCEPT:      break;
            case DOMLSParserFilter::FILTER_REJECT:      fCurrentNode = (thisNode->getPreviousSibling()?thisNode->getPreviousSibling():fCurrentParent);
                                                        fCurrentParent->removeChild(thisNode);
                                                        thisNode->release();
                                                        break;
            case DOMLSParserFilter::FILTER_SKIP:        {
                                                            DOMNode* child=thisNode->getFirstChild();
                                                            while(child)
                                                            {
                                                                DOMNode* next=child->getNextSibling();
                                                                fCurrentParent->appendChild(child);
                                                                child=next;
                                                            }
                                                            fCurrentNode = (thisNode->getPreviousSibling()?thisNode->getPreviousSibling():fCurrentParent);
                                                            fCurrentParent->removeChild(thisNode);
                                                            thisNode->release();
                                                        }
                                                        break;
            case DOMLSParserFilter::FILTER_INTERRUPT:   throw DOMLSException(DOMLSException::PARSE_ERR, XMLDOMMsg::LSParser_ParsingAborted, fMemoryManager);
            }
        }
    }
}

void DOMLSParserImpl::startElement(const XMLElementDecl&         elemDecl
                                 , const unsigned int            urlId
                                 , const XMLCh* const            elemPrefix
                                 , const RefVectorOf<XMLAttr>&   attrList
                                 , const XMLSize_t               attrCount
                                 , const bool                    isEmpty
                                 , const bool                    isRoot)
{
    if(fFilter)
    {
        // send the notification for the previous text node
        if(fFilterDelayedTextNodes && fFilterDelayedTextNodes->containsKey(fCurrentNode))
        {
            fFilterDelayedTextNodes->removeKey(fCurrentNode);
            applyFilter(fCurrentNode);
        }
    }

    DOMNode* origParent = fCurrentParent;
    AbstractDOMParser::startElement(elemDecl, urlId, elemPrefix, attrList, attrCount, false, isRoot);
    if(fFilter)
    {
        // if the parent was already rejected, reject this too
        if(fFilterAction && fFilterAction->containsKey(origParent) && fFilterAction->get(origParent)==DOMLSParserFilter::FILTER_REJECT)
            fFilterAction->put(fCurrentNode, DOMLSParserFilter::FILTER_REJECT);
        else
        {
            DOMLSParserFilter::FilterAction action = fFilter->startElement((DOMElement*)fCurrentNode);

            switch(action)
            {
            case DOMLSParserFilter::FILTER_ACCEPT:      break;
            case DOMLSParserFilter::FILTER_REJECT:
            case DOMLSParserFilter::FILTER_SKIP:        if(fFilterAction==0)
                                                            fFilterAction=new (fMemoryManager) ValueHashTableOf<DOMLSParserFilter::FilterAction, PtrHasher>(7, fMemoryManager);
                                                        fFilterAction->put(fCurrentNode, action);
                                                        break;
            case DOMLSParserFilter::FILTER_INTERRUPT:   throw DOMLSException(DOMLSException::PARSE_ERR, XMLDOMMsg::LSParser_ParsingAborted, fMemoryManager);
            }
        }
    }
    if(isEmpty)
        endElement(elemDecl, urlId, isRoot, elemPrefix);
}

XERCES_CPP_NAMESPACE_END
