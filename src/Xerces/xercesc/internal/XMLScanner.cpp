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
 * $Id: XMLScanner.cpp 882548 2009-11-20 13:44:14Z borisk $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/internal/XMLScanner.hpp>
#include <xercesc/internal/ValidationContextImpl.hpp>
#include <xercesc/util/Janitor.hpp>
#include <xercesc/util/Mutexes.hpp>
#include <xercesc/util/RuntimeException.hpp>
#include <xercesc/util/UnexpectedEOFException.hpp>
#include <xercesc/util/XMLMsgLoader.hpp>
#include <xercesc/util/XMLInitializer.hpp>
#include <xercesc/framework/LocalFileInputSource.hpp>
#include <xercesc/framework/URLInputSource.hpp>
#include <xercesc/framework/XMLDocumentHandler.hpp>
#include <xercesc/framework/XMLEntityHandler.hpp>
#include <xercesc/framework/XMLPScanToken.hpp>
#include <xercesc/framework/XMLValidator.hpp>
#include <xercesc/internal/EndOfEntityException.hpp>
#include <xercesc/validators/DTD/DocTypeHandler.hpp>
#include <xercesc/validators/common/GrammarResolver.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/util/XMLResourceIdentifier.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Local static data
// ---------------------------------------------------------------------------
static XMLUInt32       gScannerId = 0;
static XMLMutex*       sScannerMutex = 0;
static XMLMsgLoader*   gMsgLoader = 0;

void XMLInitializer::initializeXMLScanner()
{
    gMsgLoader = XMLPlatformUtils::loadMsgSet(XMLUni::fgXMLErrDomain);

    if (!gMsgLoader)
      XMLPlatformUtils::panic(PanicHandler::Panic_CantLoadMsgDomain);

    sScannerMutex = new XMLMutex(XMLPlatformUtils::fgMemoryManager);
}

void XMLInitializer::terminateXMLScanner()
{
    delete gMsgLoader;
    gMsgLoader = 0;

    delete sScannerMutex;
    sScannerMutex = 0;
}

//
//
typedef JanitorMemFunCall<XMLScanner>   CleanupType;
typedef JanitorMemFunCall<ReaderMgr>    ReaderMgrResetType;


// ---------------------------------------------------------------------------
//  XMLScanner: Constructors and Destructor
// ---------------------------------------------------------------------------
XMLScanner::XMLScanner(XMLValidator* const valToAdopt,
                       GrammarResolver* const grammarResolver,
                       MemoryManager* const manager)
    : fBufferSize(1024 * 1024)
    , fLowWaterMark (100)
    , fStandardUriConformant(false)
    , fCalculateSrcOfs(false)
    , fDoNamespaces(false)
    , fExitOnFirstFatal(true)
    , fValidationConstraintFatal(false)
    , fInException(false)
    , fStandalone(false)
    , fHasNoDTD(true)
    , fValidate(false)
    , fValidatorFromUser(false)
    , fDoSchema(false)
    , fSchemaFullChecking(false)
    , fIdentityConstraintChecking(true)
    , fToCacheGrammar(false)
    , fUseCachedGrammar(false)
    , fLoadExternalDTD(true)
    , fLoadSchema(true)
    , fNormalizeData(true)
    , fGenerateSyntheticAnnotations(false)
    , fValidateAnnotations(false)
    , fIgnoreCachedDTD(false)
    , fIgnoreAnnotations(false)
    , fDisableDefaultEntityResolution(false)
    , fSkipDTDValidation(false)
    , fHandleMultipleImports(false)
    , fErrorCount(0)
    , fEntityExpansionLimit(0)
    , fEntityExpansionCount(0)
    , fEmptyNamespaceId(0)
    , fUnknownNamespaceId(0)
    , fXMLNamespaceId(0)
    , fXMLNSNamespaceId(0)
    , fSchemaNamespaceId(0)
    , fUIntPool(0)
    , fUIntPoolRow(0)
    , fUIntPoolCol(0)
    , fUIntPoolRowTotal(2)
    , fScannerId(0)
    , fSequenceId(0)
    , fAttrList(0)
    , fAttrDupChkRegistry(0)
    , fDocHandler(0)
    , fDocTypeHandler(0)
    , fEntityHandler(0)
    , fErrorReporter(0)
    , fErrorHandler(0)
    , fPSVIHandler(0)
    , fValidationContext(0)
    , fEntityDeclPoolRetrieved(false)
    , fReaderMgr(manager)
    , fValidator(valToAdopt)
    , fValScheme(Val_Never)
    , fGrammarResolver(grammarResolver)
    , fGrammarPoolMemoryManager(grammarResolver->getGrammarPoolMemoryManager())
    , fGrammar(0)
    , fRootGrammar(0)
    , fURIStringPool(0)
    , fRootElemName(0)
    , fExternalSchemaLocation(0)
    , fExternalNoNamespaceSchemaLocation(0)
    , fSecurityManager(0)
    , fXMLVersion(XMLReader::XMLV1_0)
    , fMemoryManager(manager)
    , fBufMgr(manager)
    , fAttNameBuf(1023, manager)
    , fAttValueBuf(1023, manager)
    , fCDataBuf(1023, manager)
    , fQNameBuf(1023, manager)
    , fPrefixBuf(1023, manager)
    , fURIBuf(1023, manager)
    , fWSNormalizeBuf(1023, manager)
    , fElemStack(manager)
{
    CleanupType cleanup(this, &XMLScanner::cleanUp);

    try
    {
        commonInit();
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

XMLScanner::XMLScanner( XMLDocumentHandler* const  docHandler
                          , DocTypeHandler* const    docTypeHandler
                          , XMLEntityHandler* const  entityHandler
                          , XMLErrorReporter* const  errHandler
                          , XMLValidator* const      valToAdopt
                          , GrammarResolver* const   grammarResolver
                          , MemoryManager* const     manager)

    : fBufferSize(1024 * 1024)
    , fLowWaterMark (100)
    , fStandardUriConformant(false)
    , fCalculateSrcOfs(false)
    , fDoNamespaces(false)
    , fExitOnFirstFatal(true)
    , fValidationConstraintFatal(false)
    , fInException(false)
    , fStandalone(false)
    , fHasNoDTD(true)
    , fValidate(false)
    , fValidatorFromUser(false)
    , fDoSchema(false)
    , fSchemaFullChecking(false)
    , fIdentityConstraintChecking(true)
    , fToCacheGrammar(false)
    , fUseCachedGrammar(false)
	, fLoadExternalDTD(true)
    , fLoadSchema(true)
    , fNormalizeData(true)
    , fGenerateSyntheticAnnotations(false)
    , fValidateAnnotations(false)
    , fIgnoreCachedDTD(false)
    , fIgnoreAnnotations(false)
    , fDisableDefaultEntityResolution(false)
    , fSkipDTDValidation(false)
    , fHandleMultipleImports(false)
    , fErrorCount(0)
    , fEntityExpansionLimit(0)
    , fEntityExpansionCount(0)
    , fEmptyNamespaceId(0)
    , fUnknownNamespaceId(0)
    , fXMLNamespaceId(0)
    , fXMLNSNamespaceId(0)
    , fSchemaNamespaceId(0)
    , fUIntPool(0)
    , fUIntPoolRow(0)
    , fUIntPoolCol(0)
    , fUIntPoolRowTotal(2)
    , fScannerId(0)
    , fSequenceId(0)
    , fAttrList(0)
    , fAttrDupChkRegistry(0)
    , fDocHandler(docHandler)
    , fDocTypeHandler(docTypeHandler)
    , fEntityHandler(entityHandler)
    , fErrorReporter(errHandler)
    , fErrorHandler(0)
    , fPSVIHandler(0)
    , fValidationContext(0)
    , fEntityDeclPoolRetrieved(false)
    , fReaderMgr(manager)
    , fValidator(valToAdopt)
    , fValScheme(Val_Never)
    , fGrammarResolver(grammarResolver)
    , fGrammarPoolMemoryManager(grammarResolver->getGrammarPoolMemoryManager())
    , fGrammar(0)
    , fRootGrammar(0)
    , fURIStringPool(0)
    , fRootElemName(0)
    , fExternalSchemaLocation(0)
    , fExternalNoNamespaceSchemaLocation(0)
    , fSecurityManager(0)
    , fXMLVersion(XMLReader::XMLV1_0)
    , fMemoryManager(manager)
    , fBufMgr(manager)
    , fAttNameBuf(1023, manager)
    , fAttValueBuf(1023, manager)
    , fCDataBuf(1023, manager)
    , fQNameBuf(1023, manager)
    , fPrefixBuf(1023, manager)
    , fURIBuf(1023, manager)
    , fWSNormalizeBuf(1023, manager)
    , fElemStack(manager)
{
    CleanupType cleanup(this, &XMLScanner::cleanUp);

    try
    {
        commonInit();
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

XMLScanner::~XMLScanner()
{
    cleanUp();
}

void XMLScanner::resetCachedGrammar ()
{
}

void XMLScanner::setValidator(XMLValidator* const valToAdopt)
{
    if (fValidatorFromUser)
        delete fValidator;
    fValidator = valToAdopt;
    fValidatorFromUser = true;
    initValidator(fValidator);
}



// ---------------------------------------------------------------------------
//  XMLScanner: Main entry point to scan a document
// ---------------------------------------------------------------------------
void XMLScanner::scanDocument(  const   XMLCh* const    systemId)
{
    //  First we try to parse it as a URL. If that fails, we assume its
    //  a file and try it that way.
    InputSource* srcToUse = 0;
    try
    {
        //  Create a temporary URL. Since this is the primary document,
        //  it has to be fully qualified. If not, then assume we are just
        //  mistaking a file for a URL.
        XMLURL tmpURL(fMemoryManager);

        if (XMLURL::parse(systemId, tmpURL)) {

            if (tmpURL.isRelative()) {
                if (!fStandardUriConformant)
                    srcToUse = new (fMemoryManager) LocalFileInputSource(systemId, fMemoryManager);
                else {
                    // since this is the top of the try/catch, cannot call ThrowXMLwithMemMgr
                    // emit the error directly
                    MalformedURLException e(__FILE__, __LINE__, XMLExcepts::URL_NoProtocolPresent, fMemoryManager);
                    fInException = true;
                    emitError
                    (
                        XMLErrs::XMLException_Fatal
                        , e.getCode()
                        , e.getMessage()
                    );
                    return;
                }
            }
            else
            {
                if (fStandardUriConformant && tmpURL.hasInvalidChar()) {
                    MalformedURLException e(__FILE__, __LINE__, XMLExcepts::URL_MalformedURL, fMemoryManager);
                    fInException = true;
                    emitError
                    (
                        XMLErrs::XMLException_Fatal
                        , e.getCode()
                        , e.getMessage()
                    );
                    return;
                }
                srcToUse = new (fMemoryManager) URLInputSource(tmpURL, fMemoryManager);
            }
        }
        else {

            if (!fStandardUriConformant)
                srcToUse = new (fMemoryManager) LocalFileInputSource(systemId, fMemoryManager);
            else {
                // since this is the top of the try/catch, cannot call ThrowXMLwithMemMgr
                // emit the error directly
                // lazy bypass ... since all MalformedURLException are fatal, no need to check the type
                MalformedURLException e(__FILE__, __LINE__, XMLExcepts::URL_MalformedURL, fMemoryManager);
                fInException = true;
                emitError
                (
                    XMLErrs::XMLException_Fatal
                    , e.getCode()
                    , e.getMessage()
                );
                return;
            }
        }
    }
    catch(const XMLException& excToCatch)
    {
        //  For any other XMLException,
        //  emit the error and catch any user exception thrown from here.
        fInException = true;
        if (excToCatch.getErrorType() == XMLErrorReporter::ErrType_Warning)
            emitError
            (
                XMLErrs::XMLException_Warning
                , excToCatch.getCode()
                , excToCatch.getMessage()
            );
        else if (excToCatch.getErrorType() >= XMLErrorReporter::ErrType_Fatal)
            emitError
            (
                XMLErrs::XMLException_Fatal
                , excToCatch.getCode()
                , excToCatch.getMessage()
            );
        else
            emitError
            (
                XMLErrs::XMLException_Error
                , excToCatch.getCode()
                , excToCatch.getMessage()
            );
        return;
    }

    Janitor<InputSource> janSrc(srcToUse);
    scanDocument(*srcToUse);
}

void XMLScanner::scanDocument(  const   char* const systemId)
{
    // We just delegate this to the XMLCh version after transcoding
    XMLCh* tmpBuf = XMLString::transcode(systemId, fMemoryManager);
    ArrayJanitor<XMLCh> janBuf(tmpBuf, fMemoryManager);
    scanDocument(tmpBuf);
}


//  This method begins a progressive parse. It scans through the prolog and
//  returns a token to be used on subsequent scanNext() calls. If the return
//  value is true, then the token is legal and ready for further use. If it
//  returns false, then the scan of the prolog failed and the token is not
//  going to work on subsequent scanNext() calls.
bool XMLScanner::scanFirst( const   XMLCh* const    systemId
                            ,       XMLPScanToken&  toFill)
{
    //  First we try to parse it as a URL. If that fails, we assume its
    //  a file and try it that way.
    InputSource* srcToUse = 0;
    try
    {
        //  Create a temporary URL. Since this is the primary document,
        //  it has to be fully qualified. If not, then assume we are just
        //  mistaking a file for a URL.
        XMLURL tmpURL(fMemoryManager);
        if (XMLURL::parse(systemId, tmpURL)) {
            if (tmpURL.isRelative()) {
                if (!fStandardUriConformant)
                    srcToUse = new (fMemoryManager) LocalFileInputSource(systemId, fMemoryManager);
                else {
                    // since this is the top of the try/catch, cannot call ThrowXMLwithMemMgr
                    // emit the error directly
                    MalformedURLException e(__FILE__, __LINE__, XMLExcepts::URL_NoProtocolPresent, fMemoryManager);
                    fInException = true;
                    emitError
                    (
                        XMLErrs::XMLException_Fatal
                        , e.getCode()
                        , e.getMessage()
                    );
                    return false;
                }
            }
            else
            {
                if (fStandardUriConformant && tmpURL.hasInvalidChar()) {
                    MalformedURLException e(__FILE__, __LINE__, XMLExcepts::URL_MalformedURL, fMemoryManager);
                    fInException = true;
                    emitError
                    (
                        XMLErrs::XMLException_Fatal
                        , e.getCode()
                        , e.getMessage()
                    );
                    return false;
                }
                srcToUse = new (fMemoryManager) URLInputSource(tmpURL, fMemoryManager);
            }
        }
        else {
            if (!fStandardUriConformant)
                srcToUse = new (fMemoryManager) LocalFileInputSource(systemId,  fMemoryManager);
            else {
                // since this is the top of the try/catch, cannot call ThrowXMLwithMemMgr
                // emit the error directly
                // lazy bypass ... since all MalformedURLException are fatal, no need to check the type
                MalformedURLException e(__FILE__, __LINE__, XMLExcepts::URL_MalformedURL);
                fInException = true;
                emitError
                (
                    XMLErrs::XMLException_Fatal
                    , e.getCode()
                    , e.getMessage()
                );
                return false;
            }
        }
    }
    catch(const XMLException& excToCatch)
    {
        //  For any other XMLException,
        //  emit the error and catch any user exception thrown from here.
        fInException = true;
        if (excToCatch.getErrorType() == XMLErrorReporter::ErrType_Warning)
            emitError
            (
                XMLErrs::XMLException_Warning
                , excToCatch.getCode()
                , excToCatch.getMessage()
            );
        else if (excToCatch.getErrorType() >= XMLErrorReporter::ErrType_Fatal)
            emitError
            (
                XMLErrs::XMLException_Fatal
                , excToCatch.getCode()
                , excToCatch.getMessage()
            );
        else
            emitError
            (
                XMLErrs::XMLException_Error
                , excToCatch.getCode()
                , excToCatch.getMessage()
            );
        return false;
    }

    Janitor<InputSource> janSrc(srcToUse);
    return scanFirst(*srcToUse, toFill);
}

bool XMLScanner::scanFirst( const   char* const     systemId
                            ,       XMLPScanToken&  toFill)
{
    // We just delegate this to the XMLCh version after transcoding
    XMLCh* tmpBuf = XMLString::transcode(systemId, fMemoryManager);
    ArrayJanitor<XMLCh> janBuf(tmpBuf, fMemoryManager);
    return scanFirst(tmpBuf, toFill);
}

bool XMLScanner::scanFirst( const   InputSource&    src
                           ,       XMLPScanToken&  toFill)
{
    //  Bump up the sequence id for this new scan cycle. This will invalidate
    //  any previous tokens we've returned.
    fSequenceId++;

    ReaderMgrResetType  resetReaderMgr(&fReaderMgr, &ReaderMgr::reset);

   // Reset the scanner and its plugged in stuff for a new run.  This
    // resets all the data structures, creates the initial reader and
    // pushes it on the stack, and sets up the base document path
    scanReset(src);

    // If we have a document handler, then call the start document
    if (fDocHandler)
        fDocHandler->startDocument();

    try
    {
        //  Scan the prolog part, which is everything before the root element
        //  including the DTD subsets. This is all that is done on the scan
        //  first.
        scanProlog();

        //  If we got to the end of input, then its not a valid XML file.
        //  Else, go on to scan the content.
        if (fReaderMgr.atEOF())
        {
            emitError(XMLErrs::EmptyMainEntity);
        }
    }
    //  NOTE:
    //
    //  In all of the error processing below, the emitError() call MUST come
    //  before the flush of the reader mgr, or it will fail because it tries
    //  to find out the position in the XML source of the error.
    catch(const XMLErrs::Codes)
    {
        // This is a 'first failure' exception so return failure
        return false;
    }
    catch(const XMLValid::Codes)
    {
        // This is a 'first fatal error' type exit, return failure
        return false;
    }
    catch(const XMLException& excToCatch)
    {
        //  Emit the error and catch any user exception thrown from here. Make
        //  sure in all cases we flush the reader manager.
        fInException = true;
        try
        {
            if (excToCatch.getErrorType() == XMLErrorReporter::ErrType_Warning)
                emitError
                (
                    XMLErrs::XMLException_Warning
                    , excToCatch.getCode()
                    , excToCatch.getMessage()
                );
            else if (excToCatch.getErrorType() >= XMLErrorReporter::ErrType_Fatal)
                emitError
                (
                    XMLErrs::XMLException_Fatal
                    , excToCatch.getCode()
                    , excToCatch.getMessage()
                );
            else
                emitError
                (
                    XMLErrs::XMLException_Error
                    , excToCatch.getCode()
                    , excToCatch.getMessage()
                );
        }
        catch(const OutOfMemoryException&)
        {
            // This is a special case for out-of-memory
            // conditions, because resetting the ReaderMgr
            // can be problematic.
            resetReaderMgr.release();

            throw;
        }

        return false;
    }
    catch(const OutOfMemoryException&)
    {
        // This is a special case for out-of-memory
        // conditions, because resetting the ReaderMgr
        // can be problematic.
        resetReaderMgr.release();

        throw;
    }

    // Fill in the caller's token to make it legal and return success
    toFill.set(fScannerId, fSequenceId);

    // Release the object that will reset the ReaderMgr, since there's
    // more to scan.
    resetReaderMgr.release();

    return true;
}


void XMLScanner::scanReset(XMLPScanToken& token)
{
    // Make sure this token is still legal
    if (!isLegalToken(token))
        ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Scan_BadPScanToken, fMemoryManager);

    // Reset the reader manager
    fReaderMgr.reset();

    // And invalidate any tokens by bumping our sequence number
    fSequenceId++;

    // Reset our error count
    fErrorCount = 0;
}

void XMLScanner::setParseSettings(XMLScanner* const refScanner)
{
    setDocHandler(refScanner->getDocHandler());
    setDocTypeHandler(refScanner->getDocTypeHandler());
    setErrorHandler(refScanner->getErrorHandler());
    setErrorReporter(refScanner->getErrorReporter());
    setEntityHandler(refScanner->getEntityHandler());
    setDoNamespaces(refScanner->getDoNamespaces());
    setDoSchema(refScanner->getDoSchema());
    setCalculateSrcOfs(refScanner->getCalculateSrcOfs());
    setStandardUriConformant(refScanner->getStandardUriConformant());
    setExitOnFirstFatal(refScanner->getExitOnFirstFatal());
    setValidationConstraintFatal(refScanner->getValidationConstraintFatal());
    setIdentityConstraintChecking(refScanner->getIdentityConstraintChecking());
    setValidationSchemaFullChecking(refScanner->getValidationSchemaFullChecking());
    cacheGrammarFromParse(refScanner->isCachingGrammarFromParse());
    useCachedGrammarInParse(refScanner->isUsingCachedGrammarInParse());
    setLoadExternalDTD(refScanner->getLoadExternalDTD());
    setLoadSchema(refScanner->getLoadSchema());
    setNormalizeData(refScanner->getNormalizeData());
    setExternalSchemaLocation(refScanner->getExternalSchemaLocation());
    setExternalNoNamespaceSchemaLocation(refScanner->getExternalNoNamespaceSchemaLocation());
    setValidationScheme(refScanner->getValidationScheme());
    setSecurityManager(refScanner->getSecurityManager());
    setPSVIHandler(refScanner->getPSVIHandler());
}

// ---------------------------------------------------------------------------
//  XMLScanner: Private helper methods.
// ---------------------------------------------------------------------------

//  This method handles the common initialization, to avoid having to do
//  it redundantly in multiple constructors.
void XMLScanner::commonInit()
{
    //  We have to do a little init that involves statics, so we have to
    //  use the mutex to protect it.
    {
        XMLMutexLock lockInit(sScannerMutex);

        // And assign ourselves the next available scanner id
        fScannerId = ++gScannerId;
    }

    //  Create the attribute list, which is used to store attribute values
    //  during start tag processing. Give it a reasonable initial size that
    //  will serve for most folks, though it will grow as required.
    fAttrList = new (fMemoryManager) RefVectorOf<XMLAttr>(32, true, fMemoryManager);

    //  Create the id ref list. This is used to enforce XML 1.0 ID ref
    //  semantics, i.e. all id refs must refer to elements that exist
    fValidationContext = new (fMemoryManager) ValidationContextImpl(fMemoryManager);
    fValidationContext->setElemStack(&fElemStack);
    fValidationContext->setScanner(this);

    //  Create the GrammarResolver
    //fGrammarResolver = new GrammarResolver();

    // create initial, 64-element, fUIntPool
    fUIntPool = (unsigned int **)fMemoryManager->allocate(sizeof(unsigned int *) *fUIntPoolRowTotal);
    memset(fUIntPool, 0, sizeof(unsigned int *) * fUIntPoolRowTotal);
    fUIntPool[0] = (unsigned int *)fMemoryManager->allocate(sizeof(unsigned int) << 6);
    memset(fUIntPool[0], 0, sizeof(unsigned int) << 6);

    // Register self as handler for XMLBufferFull events on the CDATA buffer
    fCDataBuf.setFullHandler(this, fBufferSize);

   if (fValidator) {
       fValidatorFromUser = true;
       initValidator(fValidator);
   }
}

void XMLScanner::cleanUp()
{
    delete fAttrList;
    delete fAttrDupChkRegistry;
    delete fValidationContext;
    fMemoryManager->deallocate(fRootElemName);//delete [] fRootElemName;
    fMemoryManager->deallocate(fExternalSchemaLocation);//delete [] fExternalSchemaLocation;
    fMemoryManager->deallocate(fExternalNoNamespaceSchemaLocation);//delete [] fExternalNoNamespaceSchemaLocation;
    // delete fUIntPool
    if (fUIntPool)
    {
        for (unsigned int i=0; i<=fUIntPoolRow; i++)
        {
            fMemoryManager->deallocate(fUIntPool[i]);
        }
        fMemoryManager->deallocate(fUIntPool);
    }
}

void XMLScanner::initValidator(XMLValidator* theValidator) {

    //  Tell the validator about the stuff it needs to know in order to
    //  do its work.
    theValidator->setScannerInfo(this, &fReaderMgr, &fBufMgr);
    theValidator->setErrorReporter(fErrorReporter);
}

// ---------------------------------------------------------------------------
//  XMLScanner: Error emitting methods
// ---------------------------------------------------------------------------

//  These methods are called whenever the scanner wants to emit an error.
//  It handles getting the message loaded, doing token replacement, etc...
//  and then calling the error handler, if its installed.
bool XMLScanner::emitErrorWillThrowException(const XMLErrs::Codes toEmit)
{
    if (XMLErrs::isFatal(toEmit) && fExitOnFirstFatal && !fInException)
        return true;
    return false;
}

void XMLScanner::emitError(const XMLErrs::Codes toEmit)
{
    // Bump the error count if it is not a warning
    if (XMLErrs::errorType(toEmit) != XMLErrorReporter::ErrType_Warning)
        incrementErrorCount();

    if (fErrorReporter)
    {
        // Load the message into a local for display
        const XMLSize_t msgSize = 1023;
        XMLCh errText[msgSize + 1];

        if (!gMsgLoader->loadMsg(toEmit, errText, msgSize))
        {
                // <TBD> Probably should load a default msg here
        }

        //  Create a LastExtEntityInfo structure and get the reader manager
        //  to fill it in for us. This will give us the information about
        //  the last reader on the stack that was an external entity of some
        //  sort (i.e. it will ignore internal entities.
        ReaderMgr::LastExtEntityInfo lastInfo;
        fReaderMgr.getLastExtEntityInfo(lastInfo);

        fErrorReporter->error
        (
            toEmit
            , XMLUni::fgXMLErrDomain
            , XMLErrs::errorType(toEmit)
            , errText
            , lastInfo.systemId
            , lastInfo.publicId
            , lastInfo.lineNumber
            , lastInfo.colNumber
        );
    }

    // Bail out if its fatal an we are to give up on the first fatal error
    if (emitErrorWillThrowException(toEmit))
        throw toEmit;
}

void XMLScanner::emitError( const   XMLErrs::Codes    toEmit
                            , const XMLCh* const        text1
                            , const XMLCh* const        text2
                            , const XMLCh* const        text3
                            , const XMLCh* const        text4)
{
    // Bump the error count if it is not a warning
    if (XMLErrs::errorType(toEmit) != XMLErrorReporter::ErrType_Warning)
        incrementErrorCount();

    if (fErrorReporter)
    {
        //  Load the message into alocal and replace any tokens found in
        //  the text.
        const XMLSize_t maxChars = 2047;
        XMLCh errText[maxChars + 1];

        if (!gMsgLoader->loadMsg(toEmit, errText, maxChars, text1, text2, text3, text4, fMemoryManager))
        {
                // <TBD> Should probably load a default message here
        }

        //  Create a LastExtEntityInfo structure and get the reader manager
        //  to fill it in for us. This will give us the information about
        //  the last reader on the stack that was an external entity of some
        //  sort (i.e. it will ignore internal entities.
        ReaderMgr::LastExtEntityInfo lastInfo;
        fReaderMgr.getLastExtEntityInfo(lastInfo);

        fErrorReporter->error
        (
            toEmit
            , XMLUni::fgXMLErrDomain
            , XMLErrs::errorType(toEmit)
            , errText
            , lastInfo.systemId
            , lastInfo.publicId
            , lastInfo.lineNumber
            , lastInfo.colNumber
        );
    }

    // Bail out if its fatal an we are to give up on the first fatal error
    if (emitErrorWillThrowException(toEmit))
        throw toEmit;
}

void XMLScanner::emitError( const   XMLErrs::Codes    toEmit
                            , const char* const         text1
                            , const char* const         text2
                            , const char* const         text3
                            , const char* const         text4)
{
    // Bump the error count if it is not a warning
    if (XMLErrs::errorType(toEmit) != XMLErrorReporter::ErrType_Warning)
        incrementErrorCount();

    if (fErrorReporter)
    {
        //  Load the message into alocal and replace any tokens found in
        //  the text.
        const XMLSize_t maxChars = 2047;
        XMLCh errText[maxChars + 1];

        if (!gMsgLoader->loadMsg(toEmit, errText, maxChars, text1, text2, text3, text4, fMemoryManager))
        {
                // <TBD> Should probably load a default message here
        }

        //  Create a LastExtEntityInfo structure and get the reader manager
        //  to fill it in for us. This will give us the information about
        //  the last reader on the stack that was an external entity of some
        //  sort (i.e. it will ignore internal entities.
        ReaderMgr::LastExtEntityInfo lastInfo;
        fReaderMgr.getLastExtEntityInfo(lastInfo);

        fErrorReporter->error
        (
            toEmit
            , XMLUni::fgXMLErrDomain
            , XMLErrs::errorType(toEmit)
            , errText
            , lastInfo.systemId
            , lastInfo.publicId
            , lastInfo.lineNumber
            , lastInfo.colNumber
        );
    }

    // Bail out if its fatal an we are to give up on the first fatal error
    if (emitErrorWillThrowException(toEmit))
        throw toEmit;
}

void XMLScanner::emitError( const   XMLErrs::Codes      toEmit
                            , const XMLExcepts::Codes   originalExceptCode
                            , const XMLCh* const        text1
                            , const XMLCh* const        text2
                            , const XMLCh* const        text3
                            , const XMLCh* const        text4)
{
    // Bump the error count if it is not a warning
    if (XMLErrs::errorType(toEmit) != XMLErrorReporter::ErrType_Warning)
        incrementErrorCount();

    if (fErrorReporter)
    {
        //  Load the message into alocal and replace any tokens found in
        //  the text.
        const XMLSize_t maxChars = 2047;
        XMLCh errText[maxChars + 1];

        if (!gMsgLoader->loadMsg(toEmit, errText, maxChars, text1, text2, text3, text4, fMemoryManager))
        {
                // <TBD> Should probably load a default message here
        }

        //  Create a LastExtEntityInfo structure and get the reader manager
        //  to fill it in for us. This will give us the information about
        //  the last reader on the stack that was an external entity of some
        //  sort (i.e. it will ignore internal entities.
        ReaderMgr::LastExtEntityInfo lastInfo;
        fReaderMgr.getLastExtEntityInfo(lastInfo);

        fErrorReporter->error
        (
            originalExceptCode
            , XMLUni::fgExceptDomain    //fgXMLErrDomain
            , XMLErrs::errorType(toEmit)
            , errText
            , lastInfo.systemId
            , lastInfo.publicId
            , lastInfo.lineNumber
            , lastInfo.colNumber
        );
    }

    // Bail out if its fatal an we are to give up on the first fatal error
    if (emitErrorWillThrowException(toEmit))
        throw toEmit;
}

// ---------------------------------------------------------------------------
//  XMLScanner: Getter methods
// ---------------------------------------------------------------------------

//  This method allows the caller to query the current location of the scanner.
//  It will return the sys/public ids of the current entity, and the line/col
//  position within it.
//
//  NOTE: This API returns the location with the last external file. So if its
//  currently scanning an entity, the position returned will be the end of
//  the entity reference in the file that had the reference.
//
/*bool
XMLScanner::getLastExtLocation(         XMLCh* const    sysIdToFill
                                , const unsigned int    maxSysIdChars
                                ,       XMLCh* const    pubIdToFill
                                , const unsigned int    maxPubIdChars
                                ,       XMLSSize_t&     lineToFill
                                ,       XMLSSize_t&     colToFill) const
{
    // Create a local info object and get it filled in by the reader manager
    ReaderMgr::LastExtEntityInfo lastInfo;
    fReaderMgr.getLastExtEntityInfo(lastInfo);

    // Fill in the line and column number
    lineToFill = lastInfo.lineNumber;
    colToFill = lastInfo.colNumber;

    // And copy over as much of the ids as will fit
    sysIdToFill[0] = 0;
    if (lastInfo.systemId)
    {
        if (XMLString::stringLen(lastInfo.systemId) > maxSysIdChars)
            return false;
        XMLString::copyString(sysIdToFill, lastInfo.systemId);
    }

    pubIdToFill[0] = 0;
    if (lastInfo.publicId)
    {
        if (XMLString::stringLen(lastInfo.publicId) > maxPubIdChars)
            return false;
        XMLString::copyString(pubIdToFill, lastInfo.publicId);
    }
    return true;
}*/


// ---------------------------------------------------------------------------
//  XMLScanner: Private scanning methods
// ---------------------------------------------------------------------------

//  This method is called after the end of the root element, to handle
//  any miscellaneous stuff hanging around.
void XMLScanner::scanMiscellaneous()
{
    // Get a buffer for this work
    XMLBufBid bbCData(&fBufMgr);

    while (true)
    {
        try
        {
            const XMLCh nextCh = fReaderMgr.peekNextChar();

            // Watch for end of file and break out
            if (!nextCh)
                break;

            if (nextCh == chOpenAngle)
            {
                if (checkXMLDecl(true))
                {
                    // Can't have an XML decl here
                    emitError(XMLErrs::NotValidAfterContent);
                    fReaderMgr.skipPastChar(chCloseAngle);
                }
                else if (fReaderMgr.skippedString(XMLUni::fgPIString))
                {
                    scanPI();
                }
                 else if (fReaderMgr.skippedString(XMLUni::fgCommentString))
                {
                    scanComment();
                }
                else
                {
                    // This can't be possible, so just give up
                    emitError(XMLErrs::ExpectedCommentOrPI);
                    fReaderMgr.skipPastChar(chCloseAngle);
                }
            }
            else if (fReaderMgr.getCurrentReader()->isWhitespace(nextCh))
            {
                //  If we have a doc handler, then gather up the spaces and
                //  call back. Otherwise, just skip over whitespace.
                if (fDocHandler)
                {
                    fReaderMgr.getSpaces(bbCData.getBuffer());
                    fDocHandler->ignorableWhitespace
                    (
                        bbCData.getRawBuffer()
                        , bbCData.getLen()
                        , false
                    );
                }
                else
                {
                    fReaderMgr.skipPastSpaces();
                }
            }
            else
            {
                emitError(XMLErrs::ExpectedCommentOrPI);
                fReaderMgr.skipPastChar(chCloseAngle);
            }
        }
        catch(const EndOfEntityException&)
        {
            //  Some entity leaked out of the content part of the document. Issue
            //  a warning and keep going.
            emitError(XMLErrs::EntityPropogated);
        }
    }
}


//  Scans a PI and calls the appropriate callbacks. At entry we have just
//  scanned the <? part, and need to now start on the PI target name.
void XMLScanner::scanPI()
{
    const XMLCh* namePtr = 0;
    const XMLCh* targetPtr = 0;

    //  If there are any spaces here, then warn about it. If we aren't in
    //  'first error' mode, then we'll come back and can easily pick up
    //  again by just skipping them.
    if (fReaderMgr.lookingAtSpace())
    {
        emitError(XMLErrs::PINameExpected);
        fReaderMgr.skipPastSpaces();
    }

    // Get a buffer for the PI name and scan it in
    XMLBufBid bbName(&fBufMgr);
    if (!fReaderMgr.getName(bbName.getBuffer()))
    {
        emitError(XMLErrs::PINameExpected);
        fReaderMgr.skipPastChar(chCloseAngle);
        return;
    }

    // Point the name pointer at the raw data
    namePtr = bbName.getRawBuffer();

    // See if it is some form of 'xml' and emit a warning
    //if (!XMLString::compareIString(namePtr, XMLUni::fgXMLString))
    if (bbName.getLen() == 3 &&
        (((namePtr[0] == chLatin_x) || (namePtr[0] == chLatin_X)) &&
         ((namePtr[1] == chLatin_m) || (namePtr[1] == chLatin_M)) &&
         ((namePtr[2] == chLatin_l) || (namePtr[2] == chLatin_L))))
        emitError(XMLErrs::NoPIStartsWithXML);

    // If namespaces are enabled, then no colons allowed
    if (fDoNamespaces)
    {
        if (XMLString::indexOf(namePtr, chColon) != -1)
            emitError(XMLErrs::ColonNotLegalWithNS);
    }

    //  If we don't hit a space next, then the PI has no target. If we do
    //  then get out the target. Get a buffer for it as well
    XMLBufBid bbTarget(&fBufMgr);
    if (fReaderMgr.skippedSpace())
    {
        // Skip any leading spaces
        fReaderMgr.skipPastSpaces();

        bool gotLeadingSurrogate = false;

        // It does have a target, so lets move on to deal with that.
        while (1)
        {
            const XMLCh nextCh = fReaderMgr.getNextChar();

            // Watch for an end of file, which is always bad here
            if (!nextCh)
            {
                emitError(XMLErrs::UnterminatedPI);
                ThrowXMLwithMemMgr(UnexpectedEOFException, XMLExcepts::Gen_UnexpectedEOF, fMemoryManager);
            }

            // Watch for potential terminating character
            if (nextCh == chQuestion)
            {
                // It must be followed by '>' to be a termination of the target
                if (fReaderMgr.skippedChar(chCloseAngle))
                    break;
            }

            // Check for correct surrogate pairs
            if ((nextCh >= 0xD800) && (nextCh <= 0xDBFF))
            {
                if (gotLeadingSurrogate)
                    emitError(XMLErrs::Expected2ndSurrogateChar);
                else
                    gotLeadingSurrogate = true;
            }
             else
            {
                if (gotLeadingSurrogate)
                {
                    if ((nextCh < 0xDC00) || (nextCh > 0xDFFF))
                        emitError(XMLErrs::Expected2ndSurrogateChar);
                }
                // Its got to at least be a valid XML character
                else if (!fReaderMgr.getCurrentReader()->isXMLChar(nextCh)) {

                    XMLCh tmpBuf[9];
                    XMLString::binToText
                    (
                        nextCh
                        , tmpBuf
                        , 8
                        , 16
                        , fMemoryManager
                    );
                    emitError(XMLErrs::InvalidCharacter, tmpBuf);
                }

                gotLeadingSurrogate = false;
            }

            bbTarget.append(nextCh);
        }
    }
    else
    {
        // No target, but make sure its terminated ok
        if (!fReaderMgr.skippedChar(chQuestion))
        {
            emitError(XMLErrs::UnterminatedPI);
            fReaderMgr.skipPastChar(chCloseAngle);
            return;
        }

        if (!fReaderMgr.skippedChar(chCloseAngle))
        {
            emitError(XMLErrs::UnterminatedPI);
            fReaderMgr.skipPastChar(chCloseAngle);
            return;
        }
    }

    // Point the target pointer at the raw data
    targetPtr = bbTarget.getRawBuffer();

    // If we have a handler, then call it
    if (fDocHandler)
    {
        fDocHandler->docPI
        (
            namePtr
            , targetPtr
       );
    }

    //mark PI is seen within the current element
    if (! fElemStack.isEmpty())
        fElemStack.setCommentOrPISeen();

}

//  Scans all the input from the start of the file to the root element.
//  There does not have to be anything in the prolog necessarily, but usually
//  there is at least an XMLDecl.
//
//  On exit from here we are either at the end of the file or about to read
//  the opening < of the root element.
void XMLScanner::scanProlog()
{
    bool sawDocTypeDecl = false;
    // Get a buffer for whitespace processing
    XMLBufBid bbCData(&fBufMgr);

    //  Loop through the prolog. If there is no content, this could go all
    //  the way to the end of the file.
    try
    {
        while (true)
        {
            const XMLCh nextCh = fReaderMgr.peekNextChar();

            if (nextCh == chOpenAngle)
            {
                //  Ok, it could be the xml decl, a comment, the doc type line,
                //  or the start of the root element.
                if (checkXMLDecl(true))
                {
                    // There shall be at lease --ONE-- space in between
                    // the tag '<?xml' and the VersionInfo.
                    //
                    //  If we are not at line 1, col 6, then the decl was not
                    //  the first text, so its invalid.
                    const XMLReader* curReader = fReaderMgr.getCurrentReader();
                    if ((curReader->getLineNumber() != 1)
                    ||  (curReader->getColumnNumber() != 7))
                    {
                        emitError(XMLErrs::XMLDeclMustBeFirst);
                    }

                    scanXMLDecl(Decl_XML);
                }
                else if (fReaderMgr.skippedString(XMLUni::fgPIString))
                {
                    scanPI();
                }
                 else if (fReaderMgr.skippedString(XMLUni::fgCommentString))
                {
                    scanComment();
                }
                 else if (fReaderMgr.skippedString(XMLUni::fgDocTypeString))
                {
                    if (sawDocTypeDecl) {
                        emitError(XMLErrs::DuplicateDocTypeDecl);
                    }
                    scanDocTypeDecl();
                    sawDocTypeDecl = true;

                    // if reusing grammar, this has been validated already in first scan
                    // skip for performance
                    if (fValidate && fGrammar && !fGrammar->getValidated()) {
                        //  validate the DTD scan so far
                        fValidator->preContentValidation(fUseCachedGrammar, true);
                    }
                }
                else
                {
                    // Assume its the start of the root element
                    return;
                }
            }
            else if (fReaderMgr.getCurrentReader()->isWhitespace(nextCh))
            {
                //  If we have a document handler then gather up the
                //  whitespace and call back. Otherwise just skip over spaces.
                if (fDocHandler)
                {
                    fReaderMgr.getSpaces(bbCData.getBuffer());
                    fDocHandler->ignorableWhitespace
                    (
                        bbCData.getRawBuffer()
                        , bbCData.getLen()
                        , false
                    );
                }
                 else
                {
                    fReaderMgr.skipPastSpaces();
                }
            }
             else
            {
                emitError(XMLErrs::InvalidDocumentStructure);

                // Watch for end of file and break out
                if (!nextCh)
                    break;
                else
                    fReaderMgr.skipPastChar(chCloseAngle);
            }

        }
    }
    catch(const EndOfEntityException&)
    {
        //  We should never get an end of entity here. They should only
        //  occur within the doc type scanning method, and not leak out to
        //  here.
        emitError
        (
            XMLErrs::UnexpectedEOE
            , "in prolog"
        );
    }
}


//  Scans the <?xml .... ?> line. This stuff is all sequential so we don't
//  do any state machine loop here. We just bull straight through it. It ends
//  past the closing bracket. If there is a document handler, then its called
//  on the XMLDecl callback.
//
//  On entry, the <?xml has been scanned, and we pick it up from there.
//
//  NOTE: In order to provide good recovery from bad XML here, we try to be
//  very flexible. No matter what order the stuff is in, we'll keep going
//  though we'll issue errors.
//
//  The parameter tells us which type of decl we should expect, Text or XML.
//    [23] XMLDecl ::= '<?xml' VersionInfo EncodingDecl? SDDecl? S? '?>'
//    [77] TextDecl::= '<?xml' VersionInfo? EncodingDecl S? '?>'
void XMLScanner::scanXMLDecl(const DeclTypes type)
{
    // Get us some buffers to use
    XMLBufBid bbVersion(&fBufMgr);
    XMLBufBid bbEncoding(&fBufMgr);
    XMLBufBid bbStand(&fBufMgr);
    XMLBufBid bbDummy(&fBufMgr);
    XMLBufBid bbName(&fBufMgr);

    //  We use this little enum and array to keep up with what we found
    //  and what order we found them in. This lets us get them free form
    //  without too much overhead, but still know that they were in the
    //  wrong order.
    enum Strings
    {
        VersionString
        , EncodingString
        , StandaloneString
        , UnknownString

        , StringCount
    };
    int flags[StringCount] = { -1, -1, -1, -1 };

    //  Also set up a list of buffers in the right order so that we know
    //  where to put stuff.
    XMLBuffer* buffers[StringCount] ;
    buffers[0] = &bbVersion.getBuffer();
    buffers[1] = &bbEncoding.getBuffer();
    buffers[2] = &bbStand.getBuffer();
    buffers[3] = &bbDummy.getBuffer();

    int curCount = 0;
    Strings curString;
    XMLBuffer& nameBuf = bbName.getBuffer();
    while (true)
    {
        // Skip any spaces
        bool skippedSomething;
        fReaderMgr.skipPastSpaces(skippedSomething, true);

        // If we are looking at a question mark, then break out
        if (fReaderMgr.lookingAtChar(chQuestion))
            break;

        // If this is not the first string, then we require the spaces
        if (!skippedSomething && curCount)
            emitError(XMLErrs::ExpectedWhitespace);

        //  Get characters up to the next whitespace or equal's sign.
        if (!scanUpToWSOr(nameBuf, chEqual))
            emitError(XMLErrs::ExpectedDeclString);

        // See if it matches any of our expected strings
        if (XMLString::equals(nameBuf.getRawBuffer(), XMLUni::fgVersionString))
            curString = VersionString;
        else if (XMLString::equals(nameBuf.getRawBuffer(), XMLUni::fgEncodingString))
            curString = EncodingString;
        else if (XMLString::equals(nameBuf.getRawBuffer(), XMLUni::fgStandaloneString))
            curString = StandaloneString;
        else
            curString = UnknownString;

        //  If its an unknown string, then give that error. Else check to
        //  see if this one has been done already and give that error.
        if (curString == UnknownString)
            emitError(XMLErrs::ExpectedDeclString, nameBuf.getRawBuffer());
        else if (flags[curString] != -1)
            emitError(XMLErrs::DeclStringRep, nameBuf.getRawBuffer());
        else if (flags[curString] == -1)
            flags[curString] = ++curCount;

        //  Scan for an equal's sign. If we don't find it, issue an error
        //  but keep trying to go on.
        if (!scanEq(true))
            emitError(XMLErrs::ExpectedEqSign);

        //  Get a quote string into the buffer for the string that we are
        //  currently working on.
        if (!getQuotedString(*buffers[curString]))
        {
            emitError(XMLErrs::ExpectedQuotedString);
            fReaderMgr.skipPastChar(chCloseAngle);
            return;
        }

        // And validate the value according which one it was
        const XMLCh* rawValue = buffers[curString]->getRawBuffer();
        if (curString == VersionString)
        {
            if (XMLString::equals(rawValue, XMLUni::fgVersion1_1)) {
                if (type == Decl_XML) {
                	fXMLVersion = XMLReader::XMLV1_1;
                    fReaderMgr.setXMLVersion(XMLReader::XMLV1_1);
                }
                else {
            	    if (fXMLVersion != XMLReader::XMLV1_1)
            	        emitError(XMLErrs::UnsupportedXMLVersion, rawValue);
            	}
            }
            else if (XMLString::equals(rawValue, XMLUni::fgVersion1_0)) {
                if (type == Decl_XML) {
                	fXMLVersion = XMLReader::XMLV1_0;
                    fReaderMgr.setXMLVersion(XMLReader::XMLV1_0);
                }
            }
            else
                emitError(XMLErrs::UnsupportedXMLVersion, rawValue);
        }
         else if (curString == EncodingString)
        {
            if (!XMLString::isValidEncName(rawValue))
                emitError(XMLErrs::BadXMLEncoding, rawValue);
        }
         else if (curString == StandaloneString)
        {
            if (XMLString::equals(rawValue, XMLUni::fgYesString))
                fStandalone = true;
            else if (XMLString::equals(rawValue, XMLUni::fgNoString))
                fStandalone = false;
            else
            {
                emitError(XMLErrs::BadStandalone);
                //if (!XMLString::compareIString(rawValue, XMLUni::fgYesString))
                //else if (!XMLString::compareIString(rawValue, XMLUni::fgNoString))
                if (buffers[curString]->getLen() == 3 &&
                    (((rawValue[0] == chLatin_y) || (rawValue[0] == chLatin_Y)) &&
                     ((rawValue[1] == chLatin_e) || (rawValue[1] == chLatin_E)) &&
                     ((rawValue[2] == chLatin_s) || (rawValue[2] == chLatin_S))))
                    fStandalone = true;
                else if (buffers[curString]->getLen() == 2 &&
                    (((rawValue[0] == chLatin_n) || (rawValue[0] == chLatin_N)) &&
                     ((rawValue[1] == chLatin_o) || (rawValue[1] == chLatin_O))))
                    fStandalone = false;
            }
        }
    }

    //  Make sure that the strings present are in order. We don't care about
    //  which ones are present at this point, just that any there are in the
    //  right order.
    int curTop = 0;
    for (int index = VersionString; index < StandaloneString; index++)
    {
        if (flags[index] != -1)
        {
            if (flags[index] !=  curTop + 1)
            {
                emitError(XMLErrs::DeclStringsInWrongOrder);
                break;
            }
            curTop = flags[index];
        }
    }

    //  If its an XML decl, the version must be present.
    //  If its a Text decl, then encoding must be present AND standalone must not be present.
    if ((type == Decl_XML) && (flags[VersionString] == -1))
        emitError(XMLErrs::XMLVersionRequired);
    else if (type == Decl_Text) {
        if (flags[StandaloneString] != -1)
            emitError(XMLErrs::StandaloneNotLegal);
        if (flags[EncodingString] == -1)
            emitError(XMLErrs::EncodingRequired);
    }

    if (!fReaderMgr.skippedChar(chQuestion))
    {
        emitError(XMLErrs::UnterminatedXMLDecl);
        fReaderMgr.skipPastChar(chCloseAngle);
    }
     else if (!fReaderMgr.skippedChar(chCloseAngle))
    {
        emitError(XMLErrs::UnterminatedXMLDecl);
        fReaderMgr.skipPastChar(chCloseAngle);
    }

    //  Do this before we possibly update the reader with the
    //  actual encoding string. Otherwise, we will pass the wrong thing
    //  for the last parameter!
    const XMLCh* actualEnc = fReaderMgr.getCurrentEncodingStr();

    //  Ok, we've now seen the real encoding string, if there was one, so
    //  lets call back on the current reader and tell it what the real
    //  encoding string was. If it fails, that's because it represents some
    //  sort of contradiction with the autosensed format, and it keeps the
    //  original encoding.
    //
    //  NOTE: This can fail for a number of reasons, such as a bogus encoding
    //  name or because its in flagrant contradiction of the auto-sensed
    //  format.
    if (flags[EncodingString] != -1)
    {
        if (!fReaderMgr.getCurrentReader()->setEncoding(bbEncoding.getRawBuffer()))
            emitError(XMLErrs::ContradictoryEncoding, bbEncoding.getRawBuffer());
        else
            actualEnc = bbEncoding.getRawBuffer();
    }

    //  If we have a document handler then call the XML Decl callback.
    if (type == Decl_XML)
    {
        if (fDocHandler)
            fDocHandler->XMLDecl
            (
                bbVersion.getRawBuffer()
                , bbEncoding.getRawBuffer()
                , bbStand.getRawBuffer()
                , actualEnc
            );
    }
    else if (type == Decl_Text)
    {
        if (fDocTypeHandler)
            fDocTypeHandler->TextDecl
            (
                bbVersion.getRawBuffer()
                , bbEncoding.getRawBuffer()
            );
    }
}

const XMLCh* XMLScanner::getURIText(const   unsigned int    uriId) const
{
    if (fURIStringPool->exists(uriId)) {
        // Look up the URI in the string pool and return its id
        const XMLCh* value = fURIStringPool->getValueForId(uriId);
        if (!value)
            return XMLUni::fgZeroLenString;

        return value;
    }
    else
        return XMLUni::fgZeroLenString;
}

bool XMLScanner::getURIText(  const   unsigned int    uriId
                      ,       XMLBuffer&      uriBufToFill) const
{
    if (fURIStringPool->exists(uriId)) {
        // Look up the URI in the string pool and return its id
        const XMLCh* value = fURIStringPool->getValueForId(uriId);
        if (!value)
            return false;

        uriBufToFill.set(value);
        return true;
    }
    else
        return false;
}

bool XMLScanner::checkXMLDecl(bool startWithAngle) {

    // [23] XMLDecl     ::= '<?xml' VersionInfo EncodingDecl? SDDecl? S? '?>'
    // [24] VersionInfo ::= S 'version' Eq ("'" VersionNum "'" | '"' VersionNum '"')
    //
    // [3]  S           ::= (#x20 | #x9 | #xD | #xA)+
    if (startWithAngle) {
        if (fReaderMgr.peekString(XMLUni::fgXMLDeclString)) {
            if (fReaderMgr.skippedString(XMLUni::fgXMLDeclStringSpace)
               || fReaderMgr.skippedString(XMLUni::fgXMLDeclStringHTab)
               || fReaderMgr.skippedString(XMLUni::fgXMLDeclStringLF)
               || fReaderMgr.skippedString(XMLUni::fgXMLDeclStringCR))
            {
                return true;
            }
        }
        else if (fReaderMgr.skippedString(XMLUni::fgXMLDeclStringSpaceU)
           || fReaderMgr.skippedString(XMLUni::fgXMLDeclStringHTabU)
           || fReaderMgr.skippedString(XMLUni::fgXMLDeclStringLFU)
           || fReaderMgr.skippedString(XMLUni::fgXMLDeclStringCRU))
        {
            //  Just in case, check for upper case. If found, issue
            //  an error, but keep going.
            emitError(XMLErrs::XMLDeclMustBeLowerCase);
            return true;
        }
    }
    else {
        if (fReaderMgr.peekString(XMLUni::fgXMLString)) {
            if (fReaderMgr.skippedString(XMLUni::fgXMLStringSpace)
               || fReaderMgr.skippedString(XMLUni::fgXMLStringHTab)
               || fReaderMgr.skippedString(XMLUni::fgXMLStringLF)
               || fReaderMgr.skippedString(XMLUni::fgXMLStringCR))
            {
                return true;
            }
        }
        else if (fReaderMgr.skippedString(XMLUni::fgXMLStringSpaceU)
           || fReaderMgr.skippedString(XMLUni::fgXMLStringHTabU)
           || fReaderMgr.skippedString(XMLUni::fgXMLStringLFU)
           || fReaderMgr.skippedString(XMLUni::fgXMLStringCRU))
        {
            //  Just in case, check for upper case. If found, issue
            //  an error, but keep going.
            emitError(XMLErrs::XMLDeclMustBeLowerCase);
            return true;
        }
    }

    return false;
}


// ---------------------------------------------------------------------------
//  XMLScanner: Grammar preparsing
// ---------------------------------------------------------------------------
Grammar* XMLScanner::loadGrammar(const   XMLCh* const systemId
                                 , const short        grammarType
                                 , const bool         toCache)
{
    InputSource* srcToUse = 0;

    if (fEntityHandler){
        ReaderMgr::LastExtEntityInfo lastInfo;
        fReaderMgr.getLastExtEntityInfo(lastInfo);
        XMLResourceIdentifier resourceIdentifier(XMLResourceIdentifier::ExternalEntity,
                            systemId, 0, XMLUni::fgZeroLenString, lastInfo.systemId,
                            &fReaderMgr);
        srcToUse = fEntityHandler->resolveEntity(&resourceIdentifier);
    }

    //  First we try to parse it as a URL. If that fails, we assume its
    //  a file and try it that way.
    if (!srcToUse) {
        if (fDisableDefaultEntityResolution)
            return 0;

        try
        {
            //  Create a temporary URL. Since this is the primary document,
            //  it has to be fully qualified. If not, then assume we are just
            //  mistaking a file for a URL.
            XMLURL tmpURL(fMemoryManager);

            if (XMLURL::parse(systemId, tmpURL)) {

                if (tmpURL.isRelative())
                {
                    if (!fStandardUriConformant)
                        srcToUse = new (fMemoryManager) LocalFileInputSource(systemId, fMemoryManager);
                    else {
                        // since this is the top of the try/catch, cannot call ThrowXMLwithMemMgr
                        // emit the error directly
                        MalformedURLException e(__FILE__, __LINE__, XMLExcepts::URL_NoProtocolPresent, fMemoryManager);
                        fInException = true;
                        emitError
                        (
                            XMLErrs::XMLException_Fatal
                            , e.getCode()
                            , e.getMessage()
                        );
                        return 0;
                    }
                }
                else
                {
                    if (fStandardUriConformant && tmpURL.hasInvalidChar()) {
                        MalformedURLException e(__FILE__, __LINE__, XMLExcepts::URL_MalformedURL, fMemoryManager);
                        fInException = true;
                        emitError
                        (
                            XMLErrs::XMLException_Fatal
                            , e.getCode()
                            , e.getMessage()
                        );
                        return 0;
                    }
                    srcToUse = new (fMemoryManager) URLInputSource(tmpURL, fMemoryManager);
                }
            }
            else
            {
                if (!fStandardUriConformant)
                    srcToUse = new (fMemoryManager) LocalFileInputSource(systemId, fMemoryManager);
                else {
                    // since this is the top of the try/catch, cannot call ThrowXMLwithMemMgr
                    // emit the error directly
                    // lazy bypass ... since all MalformedURLException are fatal, no need to check the type
                    MalformedURLException e(__FILE__, __LINE__, XMLExcepts::URL_MalformedURL);
                    fInException = true;
                    emitError
                    (
                        XMLErrs::XMLException_Fatal
                        , e.getCode()
                        , e.getMessage()
                    );
                    return 0;
                }
            }
        }
        catch(const XMLException& excToCatch)
        {
            //  For any other XMLException,
            //  emit the error and catch any user exception thrown from here.
            fInException = true;
            if (excToCatch.getErrorType() == XMLErrorReporter::ErrType_Warning)
                emitError
                (
                    XMLErrs::XMLException_Warning
                    , excToCatch.getCode()
                    , excToCatch.getMessage()
                );
            else if (excToCatch.getErrorType() >= XMLErrorReporter::ErrType_Fatal)
                emitError
                (
                    XMLErrs::XMLException_Fatal
                    , excToCatch.getCode()
                    , excToCatch.getMessage()
                );
            else
                emitError
                (
                    XMLErrs::XMLException_Error
                    , excToCatch.getCode()
                    , excToCatch.getMessage()
                );
                return 0;
        }
    }

    Janitor<InputSource> janSrc(srcToUse);
    return loadGrammar(*srcToUse, grammarType, toCache);
}

Grammar* XMLScanner::loadGrammar(const   char* const systemId
                                 , const short       grammarType
                                 , const bool        toCache)
{
    // We just delegate this to the XMLCh version after transcoding
    XMLCh* tmpBuf = XMLString::transcode(systemId, fMemoryManager);
    ArrayJanitor<XMLCh> janBuf(tmpBuf, fMemoryManager);
    return loadGrammar(tmpBuf, grammarType, toCache);
}


// ---------------------------------------------------------------------------
//  XMLScanner: Setter methods
// ---------------------------------------------------------------------------
void XMLScanner::setURIStringPool(XMLStringPool* const stringPool)
{
    fURIStringPool = stringPool;
    fEmptyNamespaceId   = fURIStringPool->addOrFind(XMLUni::fgZeroLenString);
    fUnknownNamespaceId = fURIStringPool->addOrFind(XMLUni::fgUnknownURIName);
    fXMLNamespaceId     = fURIStringPool->addOrFind(XMLUni::fgXMLURIName);
    fXMLNSNamespaceId   = fURIStringPool->addOrFind(XMLUni::fgXMLNSURIName);
}

// ---------------------------------------------------------------------------
//  XMLScanner: Private helper methods
// ---------------------------------------------------------------------------

/***
 * In reusing grammars (cacheing grammar from parse, or use cached grammar), internal
 * dtd is allowed conditionally.
 *
 * In the case of cacheing grammar from parse, it is NOT allowed.
 *
 * In the case of use cached grammar,
 *   if external dtd is present and it is parsed before, then it is not allowed,
 *   otherwise it is allowed.
 *
 ***/
void XMLScanner::checkInternalDTD(bool hasExtSubset
                                 ,const XMLCh* const sysId
                                 ,const XMLCh* const pubId)
{
    if (fToCacheGrammar)
        ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Val_CantHaveIntSS, fMemoryManager);

    if (fUseCachedGrammar && hasExtSubset && !fIgnoreCachedDTD)
    {
        InputSource* sysIdSrc = resolveSystemId(sysId, pubId);
        if (sysIdSrc) {
            Janitor<InputSource> janSysIdSrc(sysIdSrc);
            Grammar* grammar = fGrammarResolver->getGrammar(sysIdSrc->getSystemId());

            if (grammar && grammar->getGrammarType() == Grammar::DTDGrammarType)
            {
                ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Val_CantHaveIntSS, fMemoryManager);
            }
        }
    }

}

//  This method is called after the content scan to insure that all the
//  ID/IDREF attributes match up (i.e. that all IDREFs refer to IDs.) This is
//  an XML 1.0 rule, so we can do here in the core.

void XMLScanner::checkIDRefs()
{
    //  Iterate the id ref list. If we find any entries here which are used
    //  but not declared, then that's an error.
    RefHashTableOfEnumerator<XMLRefInfo> refEnum(fValidationContext->getIdRefList(), false, fMemoryManager);
    while (refEnum.hasMoreElements())
    {
        // Get a ref to the current element
        const XMLRefInfo& curRef = refEnum.nextElement();

        // If its used but not declared, then its an error
        if (!curRef.getDeclared() && curRef.getUsed() && fValidate)
            fValidator->emitError(XMLValid::IDNotDeclared, curRef.getRefName());
    }
}


//  This just does a simple check that the passed progressive scan token is
//  legal for this scanner.
bool XMLScanner::isLegalToken(const XMLPScanToken& toCheck)
{
    return ((fScannerId == toCheck.fScannerId)
    &&      (fSequenceId == toCheck.fSequenceId));
}


//  This method will handle figuring out what the next top level token is
//  in the input stream. It will return an enumerated value that indicates
//  what it believes the next XML level token must be. It will eat as many
//  chars are required to figure out what is next.
XMLScanner::XMLTokens XMLScanner::senseNextToken(XMLSize_t& orgReader)
{
    //  Get the next character and use it to guesstimate what the next token
    //  is going to be. We turn on end of entity exceptions when we do this
    //  in order to catch the scenario where the current entity ended at
    //  the > of some markup.
    XMLCh nextCh=0;

    XMLReader* curReader=fReaderMgr.getCurrentReader();
    // avoid setting up the ThrowEOEJanitor if we know that we have data in the current reader
    if(curReader && curReader->charsLeftInBuffer()>0)
        nextCh = fReaderMgr.peekNextChar();
    else
    {
        ThrowEOEJanitor janMgr(&fReaderMgr, true);
        nextCh = fReaderMgr.peekNextChar();
    }

    //  If it's not a '<' we must be in content (unless it's a EOF)
    //
    //  This includes entity references '&' of some sort. These must
    //  be character data because that's the only place a reference can
    //  occur in content.
    if (nextCh != chOpenAngle)
        return nextCh?Token_CharData:Token_EOF;

    //  Ok it had to have been a '<' character. So get it out of the reader
    //  and store the reader number where we saw it, passing it back to the
    //  caller.
    fReaderMgr.getNextChar();
    orgReader = fReaderMgr.getCurrentReaderNum();

    //  Ok, so lets go through the things that it could be at this point which
    //  are all some form of markup.
    switch(fReaderMgr.peekNextChar())
    {
    case chForwardSlash:
        {
            fReaderMgr.getNextChar();
            return Token_EndTag;
        }
    case chBang:
        {
            static const XMLCh gCDATAStr[] =
            {
                    chBang, chOpenSquare, chLatin_C, chLatin_D, chLatin_A
                ,   chLatin_T, chLatin_A, chNull
            };

            static const XMLCh gCommentString[] =
            {
                chBang, chDash, chDash, chNull
            };

            if (fReaderMgr.skippedString(gCDATAStr))
                return Token_CData;

            if (fReaderMgr.skippedString(gCommentString))
                return Token_Comment;

            emitError(XMLErrs::ExpectedCommentOrCDATA);
            return Token_Unknown;
        }
    case chQuestion:
        {
            // It must be a PI
            fReaderMgr.getNextChar();
            return Token_PI;
        }
    }
    //  Assume its an element name, so return with a start tag token. If it
    //  turns out not to be, then it will fail when it cannot get a valid tag.
    return Token_StartTag;
}

// ---------------------------------------------------------------------------
//  XMLScanner: Private parsing methods
// ---------------------------------------------------------------------------

//  This guy just scans out a single or double quoted string of characters.
//  It does not pass any judgement on the contents and assumes that it is
//  illegal to have another quote of the same kind inside the string's
//  contents.
//
//  NOTE: This is for simple stuff like the strings in the XMLDecl which
//  cannot have any entities inside them. So this guy does not handle any
//  end of entity stuff.
bool XMLScanner::getQuotedString(XMLBuffer& toFill)
{
    // Reset the target buffer
    toFill.reset();

    // Get the next char which must be a single or double quote
    XMLCh quoteCh;
    if (!fReaderMgr.skipIfQuote(quoteCh))
        return false;

	XMLCh nextCh;
    // Get another char and see if it matches the starting quote char
    while ((nextCh=fReaderMgr.getNextChar())!=quoteCh)
    {
        //  We should never get either an end of file null char here. If we
        //  do, just fail. It will be handled more gracefully in the higher
        //  level code that called us.
        if (!nextCh)
            return false;

        // Else add it to the buffer
        toFill.append(nextCh);
    }
    return true;
}


//  This method scans a character reference and returns the character that
//  was refered to. It assumes that we've already scanned the &# characters
//  that prefix the numeric code.
bool XMLScanner::scanCharRef(XMLCh& toFill, XMLCh& second)
{
    bool gotOne = false;
    unsigned int value = 0;

    //  Set the radix. Its supposed to be a lower case x if hex. But, in
    //  order to recover well, we check for an upper and put out an error
    //  for that.
    unsigned int radix = 10;
    if (fReaderMgr.skippedChar(chLatin_x))
    {
        radix = 16;
    }
    else if (fReaderMgr.skippedChar(chLatin_X))
    {
        emitError(XMLErrs::HexRadixMustBeLowerCase);
        radix = 16;
    }

    while (true)
    {
        const XMLCh nextCh = fReaderMgr.peekNextChar();

        // Watch for EOF
        if (!nextCh)
            ThrowXMLwithMemMgr(UnexpectedEOFException, XMLExcepts::Gen_UnexpectedEOF, fMemoryManager);

        // Break out on the terminating semicolon
        if (nextCh == chSemiColon)
        {
            fReaderMgr.getNextChar();
            break;
        }

        //  Convert this char to a binary value, or bail out if its not
        //  one.
        unsigned int nextVal;
        if ((nextCh >= chDigit_0) && (nextCh <= chDigit_9))
            nextVal = (unsigned int)(nextCh - chDigit_0);
        else if ((nextCh >= chLatin_A) && (nextCh <= chLatin_F))
            nextVal= (unsigned int)(10 + (nextCh - chLatin_A));
        else if ((nextCh >= chLatin_a) && (nextCh <= chLatin_f))
            nextVal = (unsigned int)(10 + (nextCh - chLatin_a));
        else
        {
            // Return a zero
            toFill = 0;

            //  If we got at least a sigit, then do an unterminated ref error.
            //  Else, do an expected a numerical ref thing.
            if (gotOne)
                emitError(XMLErrs::UnterminatedCharRef);
            else
                emitError(XMLErrs::ExpectedNumericalCharRef);

            // Return failure
            return false;
        }

        //  Make sure its valid for the radix. If not, then just eat the
        //  digit and go on after issueing an error. Else, update the
        //  running value with this new digit.
        if (nextVal >= radix)
        {
            XMLCh tmpStr[2];
            tmpStr[0] = nextCh;
            tmpStr[1] = chNull;
            emitError(XMLErrs::BadDigitForRadix, tmpStr);
        }
        else
        {
            value = (value * radix) + nextVal;
            // Guard against overflow.
            if (value > 0x10FFFF) {
                // Character reference was not in the valid range
                emitError(XMLErrs::InvalidCharacterRef);
                return false;
            }
        }

        // Indicate that we got at least one good digit
        gotOne = true;

        // And eat the last char
        fReaderMgr.getNextChar();
    }

    // Return the char (or chars)
    // And check if the character expanded is valid or not
    if (value >= 0x10000 && value <= 0x10FFFF)
    {
        value -= 0x10000;
        toFill = XMLCh((value >> 10) + 0xD800);
        second = XMLCh((value & 0x3FF) + 0xDC00);
    }
    else if (value <= 0xFFFD)
    {
        toFill = XMLCh(value);
        second = 0;
        if (!fReaderMgr.getCurrentReader()->isXMLChar(toFill) && !fReaderMgr.getCurrentReader()->isControlChar(toFill)) {
            // Character reference was not in the valid range
            emitError(XMLErrs::InvalidCharacterRef);
            return false;
        }
    }
    else {
        // Character reference was not in the valid range
        emitError(XMLErrs::InvalidCharacterRef);
        return false;
    }

    return true;
}


//  We get here after the '<!--' part of the comment. We scan past the
//  terminating '-->' It will calls the appropriate handler with the comment
//  text, if one is provided. A comment can be in either the document or
//  the DTD, so the fInDocument flag is used to know which handler to send
//  it to.
void XMLScanner::scanComment()
{

    enum States
    {
        InText
        , OneDash
        , TwoDashes
    };

    // Get a buffer for this
    XMLBufBid bbComment(&fBufMgr);

    //  Get the comment text into a temp buffer. Be sure to use temp buffer
    //  two here, since its to be used for stuff that is potentially longer
    //  than just a name.
    States curState = InText;
    bool gotLeadingSurrogate = false;
    while (true)
    {
        // Get the next character
        const XMLCh nextCh = fReaderMgr.getNextChar();

        //  Watch for an end of file
        if (!nextCh)
        {
            emitError(XMLErrs::UnterminatedComment);
            ThrowXMLwithMemMgr(UnexpectedEOFException, XMLExcepts::Gen_UnexpectedEOF, fMemoryManager);
        }

        // Check for correct surrogate pairs
        if ((nextCh >= 0xD800) && (nextCh <= 0xDBFF))
        {
            if (gotLeadingSurrogate)
                emitError(XMLErrs::Expected2ndSurrogateChar);
            else
                gotLeadingSurrogate = true;
        }
        else
        {
            if (gotLeadingSurrogate)
            {
                if ((nextCh < 0xDC00) || (nextCh > 0xDFFF))
                    emitError(XMLErrs::Expected2ndSurrogateChar);
            }
            // Its got to at least be a valid XML character
            else if (!fReaderMgr.getCurrentReader()->isXMLChar(nextCh)) {

                XMLCh tmpBuf[9];
                XMLString::binToText
                (
                    nextCh
                    , tmpBuf
                    , 8
                    , 16
                    , fMemoryManager
                );
                emitError(XMLErrs::InvalidCharacter, tmpBuf);
            }

            gotLeadingSurrogate = false;
        }

        if (curState == InText)
        {
            // If its a dash, go to OneDash state. Otherwise take as text
            if (nextCh == chDash)
                curState = OneDash;
            else
                bbComment.append(nextCh);
        }
        else if (curState == OneDash)
        {
            //  If its another dash, then we change to the two dashes states.
            //  Otherwise, we have to put in the deficit dash and the new
            //  character and go back to InText.
            if (nextCh == chDash)
            {
                curState = TwoDashes;
            }
            else
            {
                bbComment.append(chDash);
                bbComment.append(nextCh);
                curState = InText;
            }
        }
        else if (curState == TwoDashes)
        {
            // The next character must be the closing bracket
            if (nextCh != chCloseAngle)
            {
                emitError(XMLErrs::IllegalSequenceInComment);
                fReaderMgr.skipPastChar(chCloseAngle);
                return;
            }
            break;
        }
    }

    // If we have an available handler, call back with the comment.
    if (fDocHandler)
    {
        fDocHandler->docComment
        (
            bbComment.getRawBuffer()
        );
    }

    //mark comment is seen within the current element
    if (! fElemStack.isEmpty())
        fElemStack.setCommentOrPISeen();

}


//  Most equal signs can have white space around them, so this little guy
//  just makes the calling code cleaner by eating whitespace.
bool XMLScanner::scanEq(bool inDecl)
{
    if(inDecl)
    {
        bool skippedSomething;
        fReaderMgr.skipPastSpaces(skippedSomething, inDecl);
        if (fReaderMgr.skippedChar(chEqual))
        {
            fReaderMgr.skipPastSpaces(skippedSomething, inDecl);
            return true;
        }
    }
    else
    {
        fReaderMgr.skipPastSpaces();
        if (fReaderMgr.skippedChar(chEqual))
        {
            fReaderMgr.skipPastSpaces();
            return true;
        }
    }
    return false;
}


XMLSize_t
XMLScanner::scanUpToWSOr(XMLBuffer& toFill, const XMLCh chEndChar)
{
    fReaderMgr.getUpToCharOrWS(toFill, chEndChar);
    return toFill.getLen();
}

unsigned int *XMLScanner::getNewUIntPtr()
{
    // this method hands back a new pointer initialized to 0
    unsigned int *retVal;
    if(fUIntPoolCol < 64)
    {
        retVal = fUIntPool[fUIntPoolRow]+fUIntPoolCol;
        fUIntPoolCol++;
        return retVal;
    }
    // time to grow the pool...
    if(fUIntPoolRow+1 == fUIntPoolRowTotal)
    {
        // and time to add some space for new rows:
        fUIntPoolRowTotal <<= 1;
        unsigned int **newArray = (unsigned int **)fMemoryManager->allocate(sizeof(unsigned int *) * fUIntPoolRowTotal );
        memcpy(newArray, fUIntPool, (fUIntPoolRow+1) * sizeof(unsigned int *));
        fMemoryManager->deallocate(fUIntPool);
        fUIntPool = newArray;
        // need to 0 out new elements we won't need:
        for (unsigned int i=fUIntPoolRow+2; i<fUIntPoolRowTotal; i++)
            fUIntPool[i] = 0;
    }
    // now to add a new row; we just made sure we have space
    fUIntPoolRow++;
    fUIntPool[fUIntPoolRow] = (unsigned int *)fMemoryManager->allocate(sizeof(unsigned int) << 6);
    memset(fUIntPool[fUIntPoolRow], 0, sizeof(unsigned int) << 6);
    // point to next element
    fUIntPoolCol = 1;
    return fUIntPool[fUIntPoolRow];
}

void XMLScanner::resetUIntPool()
{
    // to reuse the unsigned int pool--and the hashtables that use it--
    // simply reinitialize everything to 0's
    for(unsigned int i = 0; i<= fUIntPoolRow; i++)
        memset(fUIntPool[i], 0, sizeof(unsigned int) << 6);
}

void XMLScanner::recreateUIntPool()
{
    // this allows a bloated unsigned int pool to be dispensed with

    // first, delete old fUIntPool
    for (unsigned int i=0; i<=fUIntPoolRow; i++)
    {
        fMemoryManager->deallocate(fUIntPool[i]);
    }
    fMemoryManager->deallocate(fUIntPool);

    fUIntPoolRow = fUIntPoolCol = 0;
    fUIntPoolRowTotal = 2;
    fUIntPool = (unsigned int **)fMemoryManager->allocate(sizeof(unsigned int *) * fUIntPoolRowTotal);
    fUIntPool[0] = (unsigned int *)fMemoryManager->allocate(sizeof(unsigned int) << 6);
    memset(fUIntPool[fUIntPoolRow], 0, sizeof(unsigned int) << 6);
    fUIntPool[1] = 0;
}

unsigned int XMLScanner::resolvePrefix(  const XMLCh* const        prefix
                                       , const ElemStack::MapModes mode)
{
    //
    //  If the prefix is empty, and we are in attribute mode, then we assign
    //  it to the empty namespace because the default namespace does not
    //  apply to attributes.
    //
    if (!*prefix)
    {
        if(mode == ElemStack::Mode_Attribute)
            return fEmptyNamespaceId;
    }
    //  Watch for the special namespace prefixes. We always map these to
    //  special URIs. 'xml' gets mapped to the official URI that its defined
    //  to map to by the NS spec. xmlns gets mapped to a special place holder
    //  URI that we define (so that it maps to something checkable.)
    else
    {
        if (XMLString::equals(prefix, XMLUni::fgXMLNSString))
            return fXMLNSNamespaceId;
        else if (XMLString::equals(prefix, XMLUni::fgXMLString))
            return fXMLNamespaceId;
    }

    //  Ask the element stack to search up itself for a mapping for the
    //  passed prefix.
    bool unknown;
    unsigned int uriId = fElemStack.mapPrefixToURI(prefix, unknown);

    // If it was unknown, then the URI was faked in but we have to issue an error
    if (unknown)
        emitError(XMLErrs::UnknownPrefix, prefix);

    // check to see if uriId is empty; in XML 1.1 an emptynamespace is okay unless
    // we are trying to use it.
    if (*prefix &&
        mode == ElemStack::Mode_Element &&
        fXMLVersion != XMLReader::XMLV1_0 &&
        uriId == fElemStack.getEmptyNamespaceId())
        emitError(XMLErrs::UnknownPrefix, prefix);

    return uriId;
}

unsigned int
XMLScanner::resolveQName(  const XMLCh* const           qName
                         ,       XMLBuffer&             prefixBuf
                         , const ElemStack::MapModes    mode
                         ,       int&                   prefixColonPos)
{
    prefixColonPos = XMLString::indexOf(qName, chColon);
    return resolveQNameWithColon(qName, prefixBuf, mode, prefixColonPos);
}

unsigned int
XMLScanner::resolveQNameWithColon(  const XMLCh* const          qName
                                  ,       XMLBuffer&            prefixBuf
                                  , const ElemStack::MapModes   mode
                                  , const int                   prefixColonPos)
{
    //  Lets split out the qName into a URI and name buffer first. The URI
    //  can be empty.
    if (prefixColonPos == -1)
    {
        //  Its all name with no prefix, so put the whole thing into the name
        //  buffer. Then map the empty string to a URI, since the empty string
        //  represents the default namespace. This will either return some
        //  explicit URI which the default namespace is mapped to, or the
        //  the default global namespace.
        prefixBuf.reset();
        return resolvePrefix(XMLUni::fgZeroLenString, mode);
    }
    else
    {
        //  Copy the chars up to but not including the colon into the prefix
        //  buffer.
        prefixBuf.set(qName, prefixColonPos);
        return resolvePrefix(prefixBuf.getRawBuffer(), mode);
    }
}

XERCES_CPP_NAMESPACE_END
