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
 * $Id: SAXParser.cpp 882548 2009-11-20 13:44:14Z borisk $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/internal/XMLScannerResolver.hpp>
#include <xercesc/framework/XMLValidator.hpp>
#include <xercesc/util/IOException.hpp>
#include <xercesc/sax/DocumentHandler.hpp>
#include <xercesc/sax/DTDHandler.hpp>
#include <xercesc/sax/ErrorHandler.hpp>
#include <xercesc/sax/EntityResolver.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/validators/common/GrammarResolver.hpp>
#include <xercesc/framework/XMLGrammarPool.hpp>
#include <xercesc/framework/XMLSchemaDescription.hpp>
#include <xercesc/util/Janitor.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/util/XMLEntityResolver.hpp>
#include <string.h>

XERCES_CPP_NAMESPACE_BEGIN


// ---------------------------------------------------------------------------
//  SAXParser: Constructors and Destructor
// ---------------------------------------------------------------------------


typedef JanitorMemFunCall<SAXParser>    CleanupType;
typedef JanitorMemFunCall<SAXParser>    ResetInProgressType;


SAXParser::SAXParser( XMLValidator* const   valToAdopt
                    , MemoryManager* const  manager
                    , XMLGrammarPool* const gramPool):

    fParseInProgress(false)
    , fElemDepth(0)
    , fAdvDHCount(0)
    , fAdvDHListSize(32)
    , fDocHandler(0)
    , fDTDHandler(0)
    , fEntityResolver(0)
    , fXMLEntityResolver(0)
    , fErrorHandler(0)
    , fPSVIHandler(0)
    , fAdvDHList(0)
    , fScanner(0)
    , fGrammarResolver(0)
    , fURIStringPool(0)
    , fValidator(valToAdopt)
    , fMemoryManager(manager)
    , fGrammarPool(gramPool)
    , fElemQNameBuf(1023, manager)
{
    CleanupType cleanup(this, &SAXParser::cleanUp);

    try
    {
        initialize();
    }
    catch(const OutOfMemoryException&)
    {
        // Don't cleanup when out of memory, since executing the
        // code can cause problems.
        cleanup.release();

        throw;
    }

    cleanup.release();
}


SAXParser::~SAXParser()
{
    cleanUp();
}

// ---------------------------------------------------------------------------
//  SAXParser: Initialize/CleanUp methods
// ---------------------------------------------------------------------------
void SAXParser::initialize()
{
    // Create grammar resolver and string pool to pass to scanner
    fGrammarResolver = new (fMemoryManager) GrammarResolver(fGrammarPool, fMemoryManager);
    fURIStringPool = fGrammarResolver->getStringPool();

    // Create our scanner and tell it what validator to use
    fScanner = XMLScannerResolver::getDefaultScanner(fValidator, fGrammarResolver, fMemoryManager);
    fScanner->setURIStringPool(fURIStringPool);

    // Create the initial advanced handler list array and zero it out
    fAdvDHList = (XMLDocumentHandler**) fMemoryManager->allocate
    (
        fAdvDHListSize * sizeof(XMLDocumentHandler*)
    );//new XMLDocumentHandler*[fAdvDHListSize];
    memset(fAdvDHList, 0, sizeof(void*) * fAdvDHListSize);
}

void SAXParser::cleanUp()
{
    fMemoryManager->deallocate(fAdvDHList);//delete [] fAdvDHList;
    delete fScanner;
    delete fGrammarResolver;
    // grammar pool must do this
    //delete fURIStringPool;

    if (fValidator)
        delete fValidator;
}


// ---------------------------------------------------------------------------
//  SAXParser: Advanced document handler list maintenance methods
// ---------------------------------------------------------------------------
void SAXParser::installAdvDocHandler(XMLDocumentHandler* const toInstall)
{
    // See if we need to expand and do so now if needed
    if (fAdvDHCount == fAdvDHListSize)
    {
        // Calc a new size and allocate the new temp buffer
        const XMLSize_t newSize = (XMLSize_t)(fAdvDHListSize * 1.5);
        XMLDocumentHandler** newList = (XMLDocumentHandler**) fMemoryManager->allocate
        (
            newSize * sizeof(XMLDocumentHandler*)
        );//new XMLDocumentHandler*[newSize];

        // Copy over the old data to the new list and zero out the rest
        memcpy(newList, fAdvDHList, sizeof(void*) * fAdvDHListSize);
        memset
        (
            &newList[fAdvDHListSize]
            , 0
            , sizeof(void*) * (newSize - fAdvDHListSize)
        );

        // And now clean up the old array and store the new stuff
        fMemoryManager->deallocate(fAdvDHList);//delete [] fAdvDHList;
        fAdvDHList = newList;
        fAdvDHListSize = newSize;
    }

    // Add this new guy into the empty slot
    fAdvDHList[fAdvDHCount++] = toInstall;

    //
    //  Install ourself as the document handler with the scanner. We might
    //  already be, but its not worth checking, just do it.
    //
    fScanner->setDocHandler(this);
}


bool SAXParser::removeAdvDocHandler(XMLDocumentHandler* const toRemove)
{
    // If our count is zero, can't be any installed
    if (!fAdvDHCount)
        return false;

    //
    //  Search the array until we find this handler. If we find a null entry
    //  first, we can stop there before the list is kept contiguous.
    //
    XMLSize_t index;
    for (index = 0; index < fAdvDHCount; index++)
    {
        //
        //  We found it. We have to keep the list contiguous, so we have to
        //  copy down any used elements after this one.
        //
        if (fAdvDHList[index] == toRemove)
        {
            //
            //  Optimize if only one entry (pretty common). Otherwise, we
            //  have to copy them down to compact them.
            //
            if (fAdvDHCount > 1)
            {
                index++;
                while (index < fAdvDHCount)
                    fAdvDHList[index - 1] = fAdvDHList[index];
            }

            // Bump down the count and zero out the last one
            fAdvDHCount--;
            fAdvDHList[fAdvDHCount] = 0;

            //
            //  If this leaves us with no advanced handlers and there is
            //  no SAX doc handler installed on us, then remove us from the
            //  scanner as the document handler.
            //
            if (!fAdvDHCount && !fDocHandler)
                fScanner->setDocHandler(0);

            return true;
        }
    }

    // Never found it
    return false;
}


// ---------------------------------------------------------------------------
//  SAXParser: Getter methods
// ---------------------------------------------------------------------------
const XMLValidator& SAXParser::getValidator() const
{
    return *fScanner->getValidator();
}


bool SAXParser::getDoNamespaces() const
{
    return fScanner->getDoNamespaces();
}

bool SAXParser::getGenerateSyntheticAnnotations() const
{
    return fScanner->getGenerateSyntheticAnnotations();
}

bool SAXParser::getValidateAnnotations() const
{
    return fScanner->getValidateAnnotations();
}

bool SAXParser::getExitOnFirstFatalError() const
{
    return fScanner->getExitOnFirstFatal();
}

bool SAXParser::getValidationConstraintFatal() const
{
    return fScanner->getValidationConstraintFatal();
}


SAXParser::ValSchemes SAXParser::getValidationScheme() const
{
    const XMLScanner::ValSchemes scheme = fScanner->getValidationScheme();

    if (scheme == XMLScanner::Val_Always)
        return Val_Always;
    else if (scheme == XMLScanner::Val_Never)
        return Val_Never;

    return Val_Auto;
}

bool SAXParser::getDoSchema() const
{
    return fScanner->getDoSchema();
}

bool SAXParser::getValidationSchemaFullChecking() const
{
    return fScanner->getValidationSchemaFullChecking();
}

bool SAXParser::getIdentityConstraintChecking() const
{
    return fScanner->getIdentityConstraintChecking();
}

int SAXParser::getErrorCount() const
{
    return fScanner->getErrorCount();
}

XMLCh* SAXParser::getExternalSchemaLocation() const
{
    return fScanner->getExternalSchemaLocation();
}

XMLCh* SAXParser::getExternalNoNamespaceSchemaLocation() const
{
    return fScanner->getExternalNoNamespaceSchemaLocation();
}

SecurityManager* SAXParser::getSecurityManager() const
{
    return fScanner->getSecurityManager();
}

XMLSize_t SAXParser::getLowWaterMark() const
{
    return fScanner->getLowWaterMark();
}

bool SAXParser::getLoadExternalDTD() const
{
    return fScanner->getLoadExternalDTD();
}

bool SAXParser::getLoadSchema() const
{
    return fScanner->getLoadSchema();
}

bool SAXParser::isCachingGrammarFromParse() const
{
    return fScanner->isCachingGrammarFromParse();
}

bool SAXParser::isUsingCachedGrammarInParse() const
{
    return fScanner->isUsingCachedGrammarInParse();
}

bool SAXParser::getCalculateSrcOfs() const
{
    return fScanner->getCalculateSrcOfs();
}

bool SAXParser::getStandardUriConformant() const
{
    return fScanner->getStandardUriConformant();
}

Grammar* SAXParser::getGrammar(const XMLCh* const nameSpaceKey)
{
    return fGrammarResolver->getGrammar(nameSpaceKey);
}

Grammar* SAXParser::getRootGrammar()
{
    return fScanner->getRootGrammar();
}

const XMLCh* SAXParser::getURIText(unsigned int uriId) const
{
    return fScanner->getURIText(uriId);
}

XMLFilePos SAXParser::getSrcOffset() const
{
    return fScanner->getSrcOffset();
}

bool SAXParser::getIgnoreCachedDTD() const
{
    return fScanner->getIgnoreCachedDTD();
}

bool SAXParser::getIgnoreAnnotations() const
{
    return fScanner->getIgnoreAnnotations();
}

bool SAXParser::getDisableDefaultEntityResolution() const
{
    return fScanner->getDisableDefaultEntityResolution();
}

bool SAXParser::getSkipDTDValidation() const
{
    return fScanner->getSkipDTDValidation();
}

bool SAXParser::getHandleMultipleImports() const
{
    return fScanner->getHandleMultipleImports();
}

// ---------------------------------------------------------------------------
//  SAXParser: Setter methods
// ---------------------------------------------------------------------------
void SAXParser::setDoNamespaces(const bool newState)
{
    fScanner->setDoNamespaces(newState);
}

void SAXParser::setGenerateSyntheticAnnotations(const bool newState)
{
    fScanner->setGenerateSyntheticAnnotations(newState);
}

void SAXParser::setValidateAnnotations(const bool newState)
{
    fScanner->setValidateAnnotations(newState);
}

void SAXParser::setExitOnFirstFatalError(const bool newState)
{
    fScanner->setExitOnFirstFatal(newState);
}


void SAXParser::setValidationConstraintFatal(const bool newState)
{
    fScanner->setValidationConstraintFatal(newState);
}


void SAXParser::setValidationScheme(const ValSchemes newScheme)
{
    if (newScheme == Val_Never)
        fScanner->setValidationScheme(XMLScanner::Val_Never);
    else if (newScheme == Val_Always)
        fScanner->setValidationScheme(XMLScanner::Val_Always);
    else
        fScanner->setValidationScheme(XMLScanner::Val_Auto);
}

void SAXParser::setDoSchema(const bool newState)
{
    fScanner->setDoSchema(newState);
}

void SAXParser::setValidationSchemaFullChecking(const bool schemaFullChecking)
{
    fScanner->setValidationSchemaFullChecking(schemaFullChecking);
}

void SAXParser::setIdentityConstraintChecking(const bool identityConstraintChecking)
{
    fScanner->setIdentityConstraintChecking(identityConstraintChecking);
}

void SAXParser::setExternalSchemaLocation(const XMLCh* const schemaLocation)
{
    fScanner->setExternalSchemaLocation(schemaLocation);
}
void SAXParser::setExternalNoNamespaceSchemaLocation(const XMLCh* const noNamespaceSchemaLocation)
{
    fScanner->setExternalNoNamespaceSchemaLocation(noNamespaceSchemaLocation);
}

void SAXParser::setExternalSchemaLocation(const char* const schemaLocation)
{
    fScanner->setExternalSchemaLocation(schemaLocation);
}
void SAXParser::setExternalNoNamespaceSchemaLocation(const char* const noNamespaceSchemaLocation)
{
    fScanner->setExternalNoNamespaceSchemaLocation(noNamespaceSchemaLocation);
}

void SAXParser::setSecurityManager(SecurityManager* const securityManager)
{
    // since this could impact various components, don't permit it to change
    // during a parse
    if (fParseInProgress)
        ThrowXMLwithMemMgr(IOException, XMLExcepts::Gen_ParseInProgress, fMemoryManager);

    fScanner->setSecurityManager(securityManager);
}

void SAXParser::setLowWaterMark(XMLSize_t lwm)
{
    fScanner->setLowWaterMark(lwm);
}

void SAXParser::setLoadExternalDTD(const bool newState)
{
    fScanner->setLoadExternalDTD(newState);
}

void SAXParser::setLoadSchema(const bool newState)
{
    fScanner->setLoadSchema(newState);
}

void SAXParser::cacheGrammarFromParse(const bool newState)
{
    fScanner->cacheGrammarFromParse(newState);

    if (newState)
        fScanner->useCachedGrammarInParse(newState);
}

void SAXParser::useCachedGrammarInParse(const bool newState)
{
    if (newState || !fScanner->isCachingGrammarFromParse())
        fScanner->useCachedGrammarInParse(newState);
}

void SAXParser::setCalculateSrcOfs(const bool newState)
{
    fScanner->setCalculateSrcOfs(newState);
}

void SAXParser::setStandardUriConformant(const bool newState)
{
    fScanner->setStandardUriConformant(newState);
}

void SAXParser::useScanner(const XMLCh* const scannerName)
{
    XMLScanner* tempScanner = XMLScannerResolver::resolveScanner
    (
        scannerName
        , fValidator
        , fGrammarResolver
        , fMemoryManager
    );

    if (tempScanner) {

        tempScanner->setParseSettings(fScanner);
        tempScanner->setURIStringPool(fURIStringPool);
        delete fScanner;
        fScanner = tempScanner;
    }
}

void SAXParser::setInputBufferSize(const XMLSize_t bufferSize)
{
    fScanner->setInputBufferSize(bufferSize);
}

void SAXParser::setIgnoreCachedDTD(const bool newValue)
{
    fScanner->setIgnoredCachedDTD(newValue);
}

void SAXParser::setIgnoreAnnotations(const bool newValue)
{
    fScanner->setIgnoreAnnotations(newValue);
}

void SAXParser::setDisableDefaultEntityResolution(const bool newValue)
{
    fScanner->setDisableDefaultEntityResolution(newValue);
}

void SAXParser::setSkipDTDValidation(const bool newValue)
{
    fScanner->setSkipDTDValidation(newValue);
}

void SAXParser::setHandleMultipleImports(const bool newValue)
{
    fScanner->setHandleMultipleImports(newValue);
}

// ---------------------------------------------------------------------------
//  SAXParser: Overrides of the SAX Parser interface
// ---------------------------------------------------------------------------
void SAXParser::parse(const InputSource& source)
{
    // Avoid multiple entrance
    if (fParseInProgress)
        ThrowXMLwithMemMgr(IOException, XMLExcepts::Gen_ParseInProgress, fMemoryManager);

    ResetInProgressType     resetInProgress(this, &SAXParser::resetInProgress);

    try
    {
        fParseInProgress = true;
        fScanner->scanDocument(source);
    }
    catch(const OutOfMemoryException&)
    {
        resetInProgress.release();

        throw;
    }
}

void SAXParser::parse(const XMLCh* const systemId)
{
    // Avoid multiple entrance
    if (fParseInProgress)
        ThrowXMLwithMemMgr(IOException, XMLExcepts::Gen_ParseInProgress, fMemoryManager);

    ResetInProgressType     resetInProgress(this, &SAXParser::resetInProgress);

    try
    {
        fParseInProgress = true;
        fScanner->scanDocument(systemId);
    }
    catch(const OutOfMemoryException&)
    {
        resetInProgress.release();

        throw;
    }
}

void SAXParser::parse(const char* const systemId)
{
    // Avoid multiple entrance
    if (fParseInProgress)
        ThrowXMLwithMemMgr(IOException, XMLExcepts::Gen_ParseInProgress, fMemoryManager);

    ResetInProgressType     resetInProgress(this, &SAXParser::resetInProgress);

    try
    {
        fParseInProgress = true;
        fScanner->scanDocument(systemId);
    }
    catch(const OutOfMemoryException&)
    {
        resetInProgress.release();

        throw;
    }
}

void SAXParser::setDocumentHandler(DocumentHandler* const handler)
{
    fDocHandler = handler;
    if (fDocHandler)
    {
        //
        //  Make sure we are set as the document handler with the scanner.
        //  We may already be (if advanced handlers are installed), but its
        //  not worthing checking, just do it.
        //
        fScanner->setDocHandler(this);
    }
     else
    {
        //
        //  If we don't have any advanced handlers either, then deinstall us
        //  from the scanner because we don't need document events anymore.
        //
        if (!fAdvDHCount)
            fScanner->setDocHandler(0);
    }
}


void SAXParser::setDTDHandler(DTDHandler* const handler)
{
    fDTDHandler = handler;
    if (fDTDHandler)
        fScanner->setDocTypeHandler(this);
    else
        fScanner->setDocTypeHandler(0);
}


void SAXParser::setErrorHandler(ErrorHandler* const handler)
{
    //
    //  Store the handler. Then either install or deinstall us as the
    //  error reporter on the scanner.
    //
    fErrorHandler = handler;
    if (fErrorHandler) {
        fScanner->setErrorReporter(this);
        fScanner->setErrorHandler(fErrorHandler);
    }
    else {
        fScanner->setErrorReporter(0);
        fScanner->setErrorHandler(0);
    }
}


void SAXParser::setPSVIHandler(PSVIHandler* const handler)
{
    fPSVIHandler = handler;
    if (fPSVIHandler) {
        fScanner->setPSVIHandler(fPSVIHandler);
    }
    else {
        fScanner->setPSVIHandler(0);
    }
}

void SAXParser::setEntityResolver(EntityResolver* const resolver)
{
    fEntityResolver = resolver;
    if (fEntityResolver) {
        fScanner->setEntityHandler(this);
        fXMLEntityResolver = 0;
    }
    else {
        fScanner->setEntityHandler(0);
    }
}

void SAXParser::setXMLEntityResolver(XMLEntityResolver* const resolver)
{
    fXMLEntityResolver = resolver;
    if (fXMLEntityResolver) {
        fScanner->setEntityHandler(this);
        fEntityResolver = 0;
    }
    else {
        fScanner->setEntityHandler(0);
    }
}

// ---------------------------------------------------------------------------
//  SAXParser: Progressive parse methods
// ---------------------------------------------------------------------------
bool SAXParser::parseFirst( const   XMLCh* const    systemId
                            ,       XMLPScanToken&  toFill)
{
    //
    //  Avoid multiple entrance. We cannot enter here while a regular parse
    //  is in progress.
    //
    if (fParseInProgress)
        ThrowXMLwithMemMgr(IOException, XMLExcepts::Gen_ParseInProgress, fMemoryManager);

    return fScanner->scanFirst(systemId, toFill);
}

bool SAXParser::parseFirst( const   char* const     systemId
                            ,       XMLPScanToken&  toFill)
{
    //
    //  Avoid multiple entrance. We cannot enter here while a regular parse
    //  is in progress.
    //
    if (fParseInProgress)
        ThrowXMLwithMemMgr(IOException, XMLExcepts::Gen_ParseInProgress, fMemoryManager);

    return fScanner->scanFirst(systemId, toFill);
}

bool SAXParser::parseFirst( const   InputSource&    source
                            ,       XMLPScanToken&  toFill)
{
    //
    //  Avoid multiple entrance. We cannot enter here while a regular parse
    //  is in progress.
    //
    if (fParseInProgress)
        ThrowXMLwithMemMgr(IOException, XMLExcepts::Gen_ParseInProgress, fMemoryManager);

    return fScanner->scanFirst(source, toFill);
}

bool SAXParser::parseNext(XMLPScanToken& token)
{
    return fScanner->scanNext(token);
}

void SAXParser::parseReset(XMLPScanToken& token)
{
    // Reset the scanner
    fScanner->scanReset(token);
}


// ---------------------------------------------------------------------------
//  SAXParser: Overrides of the XMLDocumentHandler interface
// ---------------------------------------------------------------------------
void SAXParser::docCharacters(  const   XMLCh* const    chars
                                , const XMLSize_t       length
                                , const bool            cdataSection)
{
    // Suppress the chars before the root element.
    if (fElemDepth)
    {
        // Just map to the SAX document handler
        if (fDocHandler)
            fDocHandler->characters(chars, length);
    }

    //
    //  If there are any installed advanced handlers, then lets call them
    //  with this info.
    //
    for (XMLSize_t index = 0; index < fAdvDHCount; index++)
        fAdvDHList[index]->docCharacters(chars, length, cdataSection);
}


void SAXParser::docComment(const XMLCh* const commentText)
{
    //
    //  SAX has no way to report this. But, if there are any installed
    //  advanced handlers, then lets call them with this info.
    //
    for (XMLSize_t index = 0; index < fAdvDHCount; index++)
        fAdvDHList[index]->docComment(commentText);
}


void SAXParser::XMLDecl( const  XMLCh* const    versionStr
                        , const XMLCh* const    encodingStr
                        , const XMLCh* const    standaloneStr
                        , const XMLCh* const    actualEncodingStr
                        )
{
    //
    //  SAX has no way to report this. But, if there are any installed
    //  advanced handlers, then lets call them with this info.
    //
    for (XMLSize_t index = 0; index < fAdvDHCount; index++)
        fAdvDHList[index]->XMLDecl( versionStr,
                                    encodingStr,
                                    standaloneStr,
                                    actualEncodingStr );
}


void SAXParser::docPI(  const   XMLCh* const    target
                        , const XMLCh* const    data)
{
    // Just map to the SAX document handler
    if (fDocHandler)
        fDocHandler->processingInstruction(target, data);

    //
    //  If there are any installed advanced handlers, then lets call them
    //  with this info.
    //
    for (XMLSize_t index = 0; index < fAdvDHCount; index++)
        fAdvDHList[index]->docPI(target, data);
}


void SAXParser::endDocument()
{
    if (fDocHandler)
        fDocHandler->endDocument();

    //
    //  If there are any installed advanced handlers, then lets call them
    //  with this info.
    //
    for (XMLSize_t index = 0; index < fAdvDHCount; index++)
        fAdvDHList[index]->endDocument();
}


void SAXParser::endElement( const   XMLElementDecl& elemDecl
                            , const unsigned int    uriId
                            , const bool            isRoot
                            , const XMLCh* const    elemPrefix)
{
    // Just map to the SAX document handler
    if (fDocHandler) {
        if (fScanner->getDoNamespaces()) {

            if (elemPrefix && *elemPrefix) {

                fElemQNameBuf.set(elemPrefix);
                fElemQNameBuf.append(chColon);
                fElemQNameBuf.append(elemDecl.getBaseName());
                fDocHandler->endElement(fElemQNameBuf.getRawBuffer());
            }
            else {
                fDocHandler->endElement(elemDecl.getBaseName());
            }
        }
        else
            fDocHandler->endElement(elemDecl.getFullName());

    }

    //
    //  If there are any installed advanced handlers, then lets call them
    //  with this info.
    //
    for (XMLSize_t index = 0; index < fAdvDHCount; index++)
        fAdvDHList[index]->endElement(elemDecl, uriId, isRoot, elemPrefix);

    //
    //  Dump the element depth down again. Don't let it underflow in case
    //  of malformed XML.
    //
    if (fElemDepth)
        fElemDepth--;
}


void SAXParser::endEntityReference(const XMLEntityDecl& entityDecl)
{
    //
    //  SAX has no way to report this event. But, if there are any installed
    //  advanced handlers, then lets call them with this info.
    //
    for (XMLSize_t index = 0; index < fAdvDHCount; index++)
        fAdvDHList[index]->endEntityReference(entityDecl);
}


void SAXParser::ignorableWhitespace(const   XMLCh* const    chars
                                    , const XMLSize_t       length
                                    , const bool            cdataSection)
{
    // Do not report the whitespace before the root element.
    if (!fElemDepth)
        return;

    // Just map to the SAX document handler
    if (fDocHandler)
        fDocHandler->ignorableWhitespace(chars, length);

    //
    //  If there are any installed advanced handlers, then lets call them
    //  with this info.
    //
    for (XMLSize_t index = 0; index < fAdvDHCount; index++)
        fAdvDHList[index]->ignorableWhitespace(chars, length, cdataSection);
}


void SAXParser::resetDocument()
{
    // Just map to the SAX document handler
    if (fDocHandler)
        fDocHandler->resetDocument();

    //
    //  If there are any installed advanced handlers, then lets call them
    //  with this info.
    //
    for (XMLSize_t index = 0; index < fAdvDHCount; index++)
        fAdvDHList[index]->resetDocument();

    // Make sure our element depth flag gets set back to zero
    fElemDepth = 0;
}


void SAXParser::startDocument()
{
    // Just map to the SAX document handler
    if (fDocHandler)
        fDocHandler->setDocumentLocator(fScanner->getLocator());
    if(fDocHandler)
        fDocHandler->startDocument();

    //
    //  If there are any installed advanced handlers, then lets call them
    //  with this info.
    //
    for (XMLSize_t index = 0; index < fAdvDHCount; index++)
        fAdvDHList[index]->startDocument();
}


void SAXParser::
startElement(   const   XMLElementDecl&         elemDecl
                , const unsigned int            elemURLId
                , const XMLCh* const            elemPrefix
                , const RefVectorOf<XMLAttr>&   attrList
                , const XMLSize_t               attrCount
                , const bool                    isEmpty
                , const bool                    isRoot)
{
    // Bump the element depth counter if not empty
    if (!isEmpty)
        fElemDepth++;

    if (fDocHandler)
    {
        fAttrList.setVector(&attrList, attrCount);
        if (fScanner->getDoNamespaces()) {

            if (elemPrefix && *elemPrefix) {

                fElemQNameBuf.set(elemPrefix);
                fElemQNameBuf.append(chColon);
                fElemQNameBuf.append(elemDecl.getBaseName());
                fDocHandler->startElement(fElemQNameBuf.getRawBuffer(), fAttrList);

                // If its empty, send the end tag event now
                if (isEmpty && fDocHandler)
                    fDocHandler->endElement(fElemQNameBuf.getRawBuffer());
            }
            else {

                fDocHandler->startElement(elemDecl.getBaseName(), fAttrList);

                // If its empty, send the end tag event now
                if (isEmpty && fDocHandler)
                    fDocHandler->endElement(elemDecl.getBaseName());
            }
        }
        else {
            fDocHandler->startElement(elemDecl.getFullName(), fAttrList);

            // If its empty, send the end tag event now
            if (isEmpty && fDocHandler)
                fDocHandler->endElement(elemDecl.getFullName());
        }
    }

    //
    //  If there are any installed advanced handlers, then lets call them
    //  with this info.
    //
    for (XMLSize_t index = 0; index < fAdvDHCount; index++)
    {
        fAdvDHList[index]->startElement
        (
            elemDecl
            , elemURLId
            , elemPrefix
            , attrList
            , attrCount
            , isEmpty
            , isRoot
        );
    }
}


void SAXParser::startEntityReference(const XMLEntityDecl& entityDecl)
{
    //
    //  SAX has no way to report this. But, If there are any installed
    //  advanced handlers, then lets call them with this info.
    //
    for (XMLSize_t index = 0; index < fAdvDHCount; index++)
        fAdvDHList[index]->startEntityReference(entityDecl);
}



// ---------------------------------------------------------------------------
//  SAXParser: Overrides of the DocTypeHandler interface
// ---------------------------------------------------------------------------
void SAXParser::attDef( const   DTDElementDecl&
                        , const DTDAttDef&
                        , const bool)
{
    // Unused by SAX DTDHandler interface at this time
}


void SAXParser::doctypeComment(const XMLCh* const)
{
    // Unused by SAX DTDHandler interface at this time
}


void SAXParser::doctypeDecl(const   DTDElementDecl&
                            , const XMLCh* const
                            , const XMLCh* const
                            , const bool
                            , const bool)
{
    // Unused by SAX DTDHandler interface at this time
}


void SAXParser::doctypePI(  const   XMLCh* const
                            , const XMLCh* const)
{
    // Unused by SAX DTDHandler interface at this time
}


void SAXParser::doctypeWhitespace(  const   XMLCh* const
                                    , const XMLSize_t)
{
    // Unused by SAX DTDHandler interface at this time
}


void SAXParser::elementDecl(const DTDElementDecl&, const bool)
{
    // Unused by SAX DTDHandler interface at this time
}


void SAXParser::endAttList(const DTDElementDecl&)
{
    // Unused by SAX DTDHandler interface at this time
}


void SAXParser::endIntSubset()
{
    // Unused by SAX DTDHandler interface at this time
}


void SAXParser::endExtSubset()
{
    // Unused by SAX DTDHandler interface at this time
}


void SAXParser::entityDecl( const   DTDEntityDecl&  entityDecl
                            , const bool
                            , const bool            isIgnored)
{
    //
    //  If we have a DTD handler, and this entity is not ignored, and
    //  its an unparsed entity, then send this one.
    //
    if (fDTDHandler && !isIgnored)
    {
        if (entityDecl.isUnparsed())
        {
            fDTDHandler->unparsedEntityDecl
            (
                entityDecl.getName()
                , entityDecl.getPublicId()
                , entityDecl.getSystemId()
                , entityDecl.getNotationName()
            );
        }
    }
}


void SAXParser::resetDocType()
{
    // Just map to the DTD handler
    if (fDTDHandler)
        fDTDHandler->resetDocType();
}


void SAXParser::notationDecl(   const   XMLNotationDecl&    notDecl
                                , const bool                isIgnored)
{
    if (fDTDHandler && !isIgnored)
    {
        fDTDHandler->notationDecl
        (
            notDecl.getName()
            , notDecl.getPublicId()
            , notDecl.getSystemId()
        );
    }
}


void SAXParser::startAttList(const DTDElementDecl&)
{
    // Unused by SAX DTDHandler interface at this time
}


void SAXParser::startIntSubset()
{
    // Unused by SAX DTDHandler interface at this time
}


void SAXParser::startExtSubset()
{
    // Unused by SAX DTDHandler interface at this time
}


void SAXParser::TextDecl(   const  XMLCh* const
                            , const XMLCh* const)
{
    // Unused by SAX DTDHandler interface at this time
}


// ---------------------------------------------------------------------------
//  SAXParser: Overrides of the XMLErrorReporter interface
// ---------------------------------------------------------------------------
void SAXParser::resetErrors()
{
    if (fErrorHandler)
        fErrorHandler->resetErrors();
}


void SAXParser::error(  const   unsigned int
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
        , fMemoryManager
    );

    if (!fErrorHandler)
    {
        if (errType == XMLErrorReporter::ErrType_Fatal)
            throw toThrow;
        else
            return;
    }

    if (errType == XMLErrorReporter::ErrType_Warning)
        fErrorHandler->warning(toThrow);
    else if (errType == XMLErrorReporter::ErrType_Fatal)
        fErrorHandler->fatalError(toThrow);
    else
        fErrorHandler->error(toThrow);
}



// ---------------------------------------------------------------------------
//  SAXParser: Handlers for the XMLEntityHandler interface
// ---------------------------------------------------------------------------
void SAXParser::endInputSource(const InputSource&)
{
}

bool SAXParser::expandSystemId(const XMLCh* const, XMLBuffer&)
{
    return false;
}


void SAXParser::resetEntities()
{
    // Nothing to do for this one
}

InputSource*
SAXParser::resolveEntity(  XMLResourceIdentifier* resourceIdentifier )
{
    // Just map to the SAX entity resolver handler
    if (fEntityResolver)
        return fEntityResolver->resolveEntity(resourceIdentifier->getPublicId(),
                                                resourceIdentifier->getSystemId());
    if (fXMLEntityResolver)
        return fXMLEntityResolver->resolveEntity(resourceIdentifier);
    return 0;
}


void SAXParser::startInputSource(const InputSource&)
{
    // Nothing to do for this one
}


// ---------------------------------------------------------------------------
//  SAXParser: Grammar preparsing methods
// ---------------------------------------------------------------------------
Grammar* SAXParser::loadGrammar(const char* const systemId,
                                const Grammar::GrammarType grammarType,
                                const bool toCache)
{
    // Avoid multiple entrance
    if (fParseInProgress)
        ThrowXMLwithMemMgr(IOException, XMLExcepts::Gen_ParseInProgress, fMemoryManager);

    ResetInProgressType     resetInProgress(this, &SAXParser::resetInProgress);

    Grammar* grammar = 0;
    try
    {
        fParseInProgress = true;
        grammar = fScanner->loadGrammar(systemId, grammarType, toCache);
    }
    catch(const OutOfMemoryException&)
    {
        resetInProgress.release();

        throw;
    }

    return grammar;
}

Grammar* SAXParser::loadGrammar(const XMLCh* const systemId,
                                const Grammar::GrammarType grammarType,
                                const bool toCache)
{
    // Avoid multiple entrance
    if (fParseInProgress)
        ThrowXMLwithMemMgr(IOException, XMLExcepts::Gen_ParseInProgress, fMemoryManager);

    ResetInProgressType     resetInProgress(this, &SAXParser::resetInProgress);

    Grammar* grammar = 0;
    try
    {
        fParseInProgress = true;
        grammar = fScanner->loadGrammar(systemId, grammarType, toCache);
    }
    catch(const OutOfMemoryException&)
    {
        resetInProgress.release();

        throw;
    }

    return grammar;
}

Grammar* SAXParser::loadGrammar(const InputSource& source,
                                const Grammar::GrammarType grammarType,
                                const bool toCache)
{
    // Avoid multiple entrance
    if (fParseInProgress)
        ThrowXMLwithMemMgr(IOException, XMLExcepts::Gen_ParseInProgress, fMemoryManager);

    ResetInProgressType     resetInProgress(this, &SAXParser::resetInProgress);

    Grammar* grammar = 0;
    try
    {
        fParseInProgress = true;
        grammar = fScanner->loadGrammar(source, grammarType, toCache);
    }
    catch(const OutOfMemoryException&)
    {
        resetInProgress.release();

        throw;
    }

    return grammar;
}

void SAXParser::resetInProgress()
{
    fParseInProgress = false;
}

void SAXParser::resetCachedGrammarPool()
{
    fGrammarResolver->resetCachedGrammar();
    fScanner->resetCachedGrammar();
}

XERCES_CPP_NAMESPACE_END
