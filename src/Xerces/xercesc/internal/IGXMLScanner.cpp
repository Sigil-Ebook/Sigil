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
 * $Id: IGXMLScanner.cpp 882548 2009-11-20 13:44:14Z borisk $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/internal/IGXMLScanner.hpp>
#include <xercesc/util/RuntimeException.hpp>
#include <xercesc/util/UnexpectedEOFException.hpp>
#include <xercesc/sax/InputSource.hpp>
#include <xercesc/framework/XMLDocumentHandler.hpp>
#include <xercesc/framework/XMLEntityHandler.hpp>
#include <xercesc/framework/XMLPScanToken.hpp>
#include <xercesc/internal/EndOfEntityException.hpp>
#include <xercesc/framework/MemoryManager.hpp>
#include <xercesc/framework/XMLGrammarPool.hpp>
#include <xercesc/framework/XMLDTDDescription.hpp>
#include <xercesc/framework/psvi/PSVIElement.hpp>
#include <xercesc/framework/psvi/PSVIHandler.hpp>
#include <xercesc/framework/psvi/PSVIAttributeList.hpp>
#include <xercesc/validators/common/GrammarResolver.hpp>
#include <xercesc/validators/DTD/DocTypeHandler.hpp>
#include <xercesc/validators/DTD/DTDScanner.hpp>
#include <xercesc/validators/DTD/DTDValidator.hpp>
#include <xercesc/validators/schema/SchemaValidator.hpp>
#include <xercesc/validators/schema/identity/IdentityConstraintHandler.hpp>
#include <xercesc/validators/schema/identity/IC_Selector.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

XERCES_CPP_NAMESPACE_BEGIN


typedef JanitorMemFunCall<IGXMLScanner> CleanupType;
typedef JanitorMemFunCall<ReaderMgr>    ReaderMgrResetType;


// ---------------------------------------------------------------------------
//  IGXMLScanner: Constructors and Destructor
// ---------------------------------------------------------------------------
IGXMLScanner::IGXMLScanner( XMLValidator* const  valToAdopt
                          , GrammarResolver* const grammarResolver
                          , MemoryManager* const manager) :

    XMLScanner(valToAdopt, grammarResolver, manager)
    , fSeeXsi(false)
    , fGrammarType(Grammar::UnKnown)
    , fElemStateSize(16)
    , fElemState(0)
    , fElemLoopState(0)
    , fContent(1023, manager)
    , fRawAttrList(0)
    , fRawAttrColonListSize(32)
    , fRawAttrColonList(0)
    , fDTDValidator(0)
    , fSchemaValidator(0)
    , fDTDGrammar(0)
    , fICHandler(0)
    , fLocationPairs(0)
    , fDTDElemNonDeclPool(0)
    , fSchemaElemNonDeclPool(0)
    , fElemCount(0)
    , fAttDefRegistry(0)
    , fUndeclaredAttrRegistry(0)
    , fPSVIAttrList(0)
    , fModel(0)
    , fPSVIElement(0)
    , fErrorStack(0)
    , fSchemaInfoList(0)
    , fCachedSchemaInfoList (0)
{
    CleanupType cleanup(this, &IGXMLScanner::cleanUp);

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

IGXMLScanner::IGXMLScanner( XMLDocumentHandler* const docHandler
                          , DocTypeHandler* const     docTypeHandler
                          , XMLEntityHandler* const   entityHandler
                          , XMLErrorReporter* const   errHandler
                          , XMLValidator* const       valToAdopt
                          , GrammarResolver* const    grammarResolver
                          , MemoryManager* const      manager) :

    XMLScanner(docHandler, docTypeHandler, entityHandler, errHandler, valToAdopt, grammarResolver, manager)
    , fSeeXsi(false)
    , fGrammarType(Grammar::UnKnown)
    , fElemStateSize(16)
    , fElemState(0)
    , fElemLoopState(0)
    , fContent(1023, manager)
    , fRawAttrList(0)
    , fRawAttrColonListSize(32)
    , fRawAttrColonList(0)
    , fDTDValidator(0)
    , fSchemaValidator(0)
    , fDTDGrammar(0)
    , fICHandler(0)
    , fLocationPairs(0)
    , fDTDElemNonDeclPool(0)
    , fSchemaElemNonDeclPool(0)
    , fElemCount(0)
    , fAttDefRegistry(0)
    , fUndeclaredAttrRegistry(0)
    , fPSVIAttrList(0)
    , fModel(0)
    , fPSVIElement(0)
    , fErrorStack(0)
    , fSchemaInfoList(0)
    , fCachedSchemaInfoList (0)
{
    CleanupType cleanup(this, &IGXMLScanner::cleanUp);

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

IGXMLScanner::~IGXMLScanner()
{
    cleanUp();
}

// ---------------------------------------------------------------------------
//  XMLScanner: Getter methods
// ---------------------------------------------------------------------------
NameIdPool<DTDEntityDecl>* IGXMLScanner::getEntityDeclPool()
{
    if(!fDTDGrammar)
        return 0;
    return fDTDGrammar->getEntityDeclPool();
}

const NameIdPool<DTDEntityDecl>* IGXMLScanner::getEntityDeclPool() const
{
    if(!fDTDGrammar)
        return 0;
    return fDTDGrammar->getEntityDeclPool();
}

// ---------------------------------------------------------------------------
//  IGXMLScanner: Main entry point to scan a document
// ---------------------------------------------------------------------------
void IGXMLScanner::scanDocument(const InputSource& src)
{
    //  Bump up the sequence id for this parser instance. This will invalidate
    //  any previous progressive scan tokens.
    fSequenceId++;

    ReaderMgrResetType  resetReaderMgr(&fReaderMgr, &ReaderMgr::reset);

    try
    {
        //  Reset the scanner and its plugged in stuff for a new run. This
        //  resets all the data structures, creates the initial reader and
        //  pushes it on the stack, and sets up the base document path.
        scanReset(src);

        // If we have a document handler, then call the start document
        if (fDocHandler)
            fDocHandler->startDocument();

        //  Scan the prolog part, which is everything before the root element
        //  including the DTD subsets.
        scanProlog();

        //  If we got to the end of input, then its not a valid XML file.
        //  Else, go on to scan the content.
        if (fReaderMgr.atEOF())
        {
            emitError(XMLErrs::EmptyMainEntity);
        }
        else
        {
            // Scan content, and tell it its not an external entity
            if (scanContent())
            {
                // Do post-parse validation if required
                if (fValidate)
                {
                    //  We handle ID reference semantics at this level since
                    //  its required by XML 1.0.
                    checkIDRefs();

                    // Then allow the validator to do any extra stuff it wants
//                    fValidator->postParseValidation();
                }

                // That went ok, so scan for any miscellaneous stuff
                if (!fReaderMgr.atEOF())
                    scanMiscellaneous();
            }
        }

        // If we have a document handler, then call the end document
        if (fDocHandler)
            fDocHandler->endDocument();

        //cargill debug:
        //fGrammarResolver->getXSModel();
    }
    //  NOTE:
    //
    //  In all of the error processing below, the emitError() call MUST come
    //  before the flush of the reader mgr, or it will fail because it tries
    //  to find out the position in the XML source of the error.
    catch(const XMLErrs::Codes)
    {
        // This is a 'first failure' exception, so fall through
    }
    catch(const XMLValid::Codes)
    {
        // This is a 'first fatal error' type exit, so fall through
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
    }
    catch(const OutOfMemoryException&)
    {
        // This is a special case for out-of-memory
        // conditions, because resetting the ReaderMgr
        // can be problematic.
        resetReaderMgr.release();

        throw;
    }
}


bool IGXMLScanner::scanNext(XMLPScanToken& token)
{
    // Make sure this token is still legal
    if (!isLegalToken(token))
        ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Scan_BadPScanToken, fMemoryManager);

    // Find the next token and remember the reader id
    XMLSize_t orgReader;
    XMLTokens curToken;

    ReaderMgrResetType  resetReaderMgr(&fReaderMgr, &ReaderMgr::reset);

    bool retVal = true;

    try
    {
        while (true)
        {
            //  We have to handle any end of entity exceptions that happen here.
            //  We could be at the end of X nested entities, each of which will
            //  generate an end of entity exception as we try to move forward.
            try
            {
                curToken = senseNextToken(orgReader);
                break;
            }
            catch(const EndOfEntityException& toCatch)
            {
                // Send an end of entity reference event
                if (fDocHandler)
                    fDocHandler->endEntityReference(toCatch.getEntity());
            }
        }

        if (curToken == Token_CharData)
        {
            scanCharData(fCDataBuf);
        }
        else if (curToken == Token_EOF)
        {
            if (!fElemStack.isEmpty())
            {
                const ElemStack::StackElem* topElem = fElemStack.popTop();
                emitError
                (
                    XMLErrs::EndedWithTagsOnStack
                    , topElem->fThisElement->getFullName()
                );
            }

            retVal = false;
        }
        else
        {
            // Its some sort of markup
            bool gotData = true;
            switch(curToken)
            {
                case Token_CData :
                    // Make sure we are within content
                    if (fElemStack.isEmpty())
                        emitError(XMLErrs::CDATAOutsideOfContent);
                    scanCDSection();
                    break;

                case Token_Comment :
                    scanComment();
                    break;

                case Token_EndTag :
                    scanEndTag(gotData);
                    break;

                case Token_PI :
                    scanPI();
                    break;

                case Token_StartTag :
                    if (fDoNamespaces)
                        scanStartTagNS(gotData);
                    else
                        scanStartTag(gotData);
                    break;

                default :
                    fReaderMgr.skipToChar(chOpenAngle);
                    break;
            }

            if (orgReader != fReaderMgr.getCurrentReaderNum())
                emitError(XMLErrs::PartialMarkupInEntity);

            // If we hit the end, then do the miscellaneous part
            if (!gotData)
            {
                // Do post-parse validation if required
                if (fValidate)
                {
                    //  We handle ID reference semantics at this level since
                    //  its required by XML 1.0.
                    checkIDRefs();

                    // Then allow the validator to do any extra stuff it wants
//                    fValidator->postParseValidation();
                }

                // That went ok, so scan for any miscellaneous stuff
                scanMiscellaneous();

                if (toCheckIdentityConstraint())
                    fICHandler->endDocument();

                if (fDocHandler)
                    fDocHandler->endDocument();
            }
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
        retVal = false;
    }
    catch(const XMLValid::Codes)
    {
        // This is a 'first fatal error' type exit, so return failure
        retVal = false;
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

        retVal = false;
    }
    catch(const OutOfMemoryException&)
    {
        // This is a special case for out-of-memory
        // conditions, because resetting the ReaderMgr
        // can be problematic.
        resetReaderMgr.release();

        throw;
    }

    // If we are not at the end, release the object that will
    // reset the ReaderMgr.
    if (retVal)
        resetReaderMgr.release();

    return retVal;
}



// ---------------------------------------------------------------------------
//  IGXMLScanner: Private helper methods. Most of these are implemented in
//  IGXMLScanner2.Cpp.
// ---------------------------------------------------------------------------

//  This method handles the common initialization, to avoid having to do
//  it redundantly in multiple constructors.
void IGXMLScanner::commonInit()
{

    //  Create the element state array
    fElemState = (unsigned int*) fMemoryManager->allocate
    (
        fElemStateSize * sizeof(unsigned int)
    ); //new unsigned int[fElemStateSize];
    fElemLoopState = (unsigned int*) fMemoryManager->allocate
    (
        fElemStateSize * sizeof(unsigned int)
    ); //new unsigned int[fElemStateSize];

    //  And we need one for the raw attribute scan. This just stores key/
    //  value string pairs (prior to any processing.)
    fRawAttrList = new (fMemoryManager) RefVectorOf<KVStringPair>(32, true, fMemoryManager);
    fRawAttrColonList = (int*) fMemoryManager->allocate
    (
        fRawAttrColonListSize * sizeof(int)
    );

    //  Create the Validator and init them
    fDTDValidator = new (fMemoryManager) DTDValidator();
    initValidator(fDTDValidator);
    fSchemaValidator = new (fMemoryManager) SchemaValidator(0, fMemoryManager);
    initValidator(fSchemaValidator);

    // Create IdentityConstraint info
    fICHandler = new (fMemoryManager) IdentityConstraintHandler(this, fMemoryManager);

    // Create schemaLocation pair info
    fLocationPairs = new (fMemoryManager) ValueVectorOf<XMLCh*>(8, fMemoryManager);
    // create pools for undeclared elements
    fDTDElemNonDeclPool = new (fMemoryManager) NameIdPool<DTDElementDecl>(29, 128, fMemoryManager);
    fSchemaElemNonDeclPool = new (fMemoryManager) RefHash3KeysIdPool<SchemaElementDecl>(29, true, 128, fMemoryManager);
    fAttDefRegistry = new (fMemoryManager) RefHashTableOf<unsigned int, PtrHasher>
    (
        131, false, fMemoryManager
    );
    fUndeclaredAttrRegistry = new (fMemoryManager) Hash2KeysSetOf<StringHasher>(7, fMemoryManager);
    fPSVIAttrList = new (fMemoryManager) PSVIAttributeList(fMemoryManager);

    fSchemaInfoList = new (fMemoryManager) RefHash2KeysTableOf<SchemaInfo>(29, fMemoryManager);
    fCachedSchemaInfoList = new (fMemoryManager) RefHash2KeysTableOf<SchemaInfo>(29, fMemoryManager);

    // use fDTDValidator as the default validator
    if (!fValidator)
        fValidator = fDTDValidator;
}

void IGXMLScanner::cleanUp()
{
    fMemoryManager->deallocate(fElemState); //delete [] fElemState;
    fMemoryManager->deallocate(fElemLoopState); //delete [] fElemLoopState;
    delete fRawAttrList;
    fMemoryManager->deallocate(fRawAttrColonList);
    delete fDTDValidator;
    delete fSchemaValidator;
    delete fICHandler;
    delete fLocationPairs;
    delete fDTDElemNonDeclPool;
    delete fSchemaElemNonDeclPool;
    delete fAttDefRegistry;
    delete fUndeclaredAttrRegistry;
    delete fPSVIAttrList;
    delete fPSVIElement;
    delete fErrorStack;
    delete fSchemaInfoList;
    delete fCachedSchemaInfoList;
}

// ---------------------------------------------------------------------------
//  IGXMLScanner: Private scanning methods
// ---------------------------------------------------------------------------

//  This method is called from scanStartTag() to handle the very raw initial
//  scan of the attributes. It just fills in the passed collection with
//  key/value pairs for each attribute. No processing is done on them at all.
XMLSize_t
IGXMLScanner::rawAttrScan(const   XMLCh* const                elemName
                          ,       RefVectorOf<KVStringPair>&  toFill
                          ,       bool&                       isEmpty)
{
    //  Keep up with how many attributes we've seen so far, and how many
    //  elements are available in the vector. This way we can reuse old
    //  elements until we run out and then expand it.
    XMLSize_t attCount = 0;
    XMLSize_t curVecSize = toFill.size();

    // Assume it is not empty
    isEmpty = false;

    //  We loop until we either see a /> or >, handling key/value pairs util
    //  we get there. We place them in the passed vector, which we will expand
    //  as required to hold them.
    while (true)
    {
        // Get the next character, which should be non-space
        XMLCh nextCh = fReaderMgr.peekNextChar();

        //  If the next character is not a slash or closed angle bracket,
        //  then it must be whitespace, since whitespace is required
        //  between the end of the last attribute and the name of the next
        //  one.
        //
        if (attCount)
        {
            if ((nextCh != chForwardSlash) && (nextCh != chCloseAngle))
            {
                bool bFoundSpace;
                fReaderMgr.skipPastSpaces(bFoundSpace);
                if (!bFoundSpace)
                {
                    // Emit the error but keep on going
                    emitError(XMLErrs::ExpectedWhitespace);
                }
                // Ok, peek another char
                nextCh = fReaderMgr.peekNextChar();
            }
        }

        //  Ok, here we first check for any of the special case characters.
        //  If its not one, then we do the normal case processing, which
        //  assumes that we've hit an attribute value, Otherwise, we do all
        //  the special case checks.
        if (!fReaderMgr.getCurrentReader()->isSpecialStartTagChar(nextCh))
        {
            //  Assume it's going to be an attribute, so get a name from
            //  the input.
            int colonPosition;
            if (!fReaderMgr.getQName(fAttNameBuf, &colonPosition))
            {
                if (fAttNameBuf.isEmpty())
                    emitError(XMLErrs::ExpectedAttrName);
                else
                    emitError(XMLErrs::InvalidAttrName, fAttNameBuf.getRawBuffer());
                fReaderMgr.skipPastChar(chCloseAngle);
                return attCount;
            }

            const XMLCh* curAttNameBuf = fAttNameBuf.getRawBuffer();

            // And next must be an equal sign
            if (!scanEq())
            {
                static const XMLCh tmpList[] =
                {
                    chSingleQuote, chDoubleQuote, chCloseAngle
                    , chOpenAngle, chForwardSlash, chNull
                };

                emitError(XMLErrs::ExpectedEqSign);

                //  Try to sync back up by skipping forward until we either
                //  hit something meaningful.
                const XMLCh chFound = fReaderMgr.skipUntilInOrWS(tmpList);

                if ((chFound == chCloseAngle) || (chFound == chForwardSlash))
                {
                    // Jump back to top for normal processing of these
                    continue;
                }
                else if ((chFound == chSingleQuote)
                      ||  (chFound == chDoubleQuote)
                      ||  fReaderMgr.getCurrentReader()->isWhitespace(chFound))
                {
                    // Just fall through assuming that the value is to follow
                }
                else if (chFound == chOpenAngle)
                {
                    // Assume a malformed tag and that new one is starting
                    emitError(XMLErrs::UnterminatedStartTag, elemName);
                    return attCount;
                }
                else
                {
                    // Something went really wrong
                    return attCount;
                }
            }

            //  Next should be the quoted attribute value. We just do a simple
            //  and stupid scan of this value. The only thing we do here
            //  is to expand entity references.
            if (!basicAttrValueScan(curAttNameBuf, fAttValueBuf))
            {
                static const XMLCh tmpList[] =
                {
                    chCloseAngle, chOpenAngle, chForwardSlash, chNull
                };

                emitError(XMLErrs::ExpectedAttrValue);

                //  It failed, so lets try to get synced back up. We skip
                //  forward until we find some whitespace or one of the
                //  chars in our list.
                const XMLCh chFound = fReaderMgr.skipUntilInOrWS(tmpList);

                if ((chFound == chCloseAngle)
                ||  (chFound == chForwardSlash)
                ||  fReaderMgr.getCurrentReader()->isWhitespace(chFound))
                {
                    //  Just fall through and process this attribute, though
                    //  the value will be "".
                }
                else if (chFound == chOpenAngle)
                {
                    // Assume a malformed tag and that new one is starting
                    emitError(XMLErrs::UnterminatedStartTag, elemName);
                    return attCount;
                }
                else
                {
                    // Something went really wrong
                    return attCount;
                }
            }

            //  And now lets add it to the passed collection. If we have not
            //  filled it up yet, then we use the next element. Else we add
            //  a new one.
            KVStringPair* curPair = 0;
            if (attCount >= curVecSize)
            {
                curPair = new (fMemoryManager) KVStringPair
                (
                    curAttNameBuf
                    , fAttNameBuf.getLen()
                    , fAttValueBuf.getRawBuffer()
                    , fAttValueBuf.getLen()
                    , fMemoryManager
                );
                toFill.addElement(curPair);
            }
             else
            {
                curPair = toFill.elementAt(attCount);
                curPair->set
                (
                    curAttNameBuf,
                    fAttNameBuf.getLen(),
                    fAttValueBuf.getRawBuffer(),
                    fAttValueBuf.getLen()
                );
            }

            if (attCount >= fRawAttrColonListSize) {
                resizeRawAttrColonList();
            }
            // Set the position of the colon and bump the count of attributes we've gotten
            fRawAttrColonList[attCount++] = colonPosition;

            // And go to the top again for another attribute
            continue;
        }

        //  It was some special case character so do all of the checks and
        //  deal with it.
        if (!nextCh)
            ThrowXMLwithMemMgr(UnexpectedEOFException, XMLExcepts::Gen_UnexpectedEOF, fMemoryManager);

        if (nextCh == chForwardSlash)
        {
            fReaderMgr.getNextChar();
            isEmpty = true;
            if (!fReaderMgr.skippedChar(chCloseAngle))
                emitError(XMLErrs::UnterminatedStartTag, elemName);
            break;
        }
        else if (nextCh == chCloseAngle)
        {
            fReaderMgr.getNextChar();
            break;
        }
        else if (nextCh == chOpenAngle)
        {
            //  Check for this one specially, since its going to be common
            //  and it is kind of auto-recovering since we've already hit the
            //  next open bracket, which is what we would have seeked to (and
            //  skipped this whole tag.)
            emitError(XMLErrs::UnterminatedStartTag, elemName);
            break;
        }
        else if ((nextCh == chSingleQuote) || (nextCh == chDoubleQuote))
        {
            //  Check for this one specially, which is probably a missing
            //  attribute name, e.g. ="value". Just issue expected name
            //  error and eat the quoted string, then jump back to the
            //  top again.
            emitError(XMLErrs::ExpectedAttrName);
            fReaderMgr.getNextChar();
            fReaderMgr.skipQuotedString(nextCh);
            fReaderMgr.skipPastSpaces();
            continue;
        }
    }

    return attCount;
}


//  This method will kick off the scanning of the primary content of the
//  document, i.e. the elements.
bool IGXMLScanner::scanContent()
{
    //  Go into a loop until we hit the end of the root element, or we fall
    //  out because there is no root element.
    //
    //  We have to do kind of a deeply nested double loop here in order to
    //  avoid doing the setup/teardown of the exception handler on each
    //  round. Doing it this way we only do it when an exception actually
    //  occurs.
    bool gotData = true;
    bool inMarkup = false;
    while (gotData)
    {
        try
        {
            while (gotData)
            {
                //  Sense what the next top level token is. According to what
                //  this tells us, we will call something to handle that kind
                //  of thing.
                XMLSize_t orgReader;
                const XMLTokens curToken = senseNextToken(orgReader);

                //  Handle character data and end of file specially. Char data
                //  is not markup so we don't want to handle it in the loop
                //  below.
                if (curToken == Token_CharData)
                {
                    //  Scan the character data and call appropriate events. Let
                    //  him use our local character data buffer for efficiency.
                    scanCharData(fCDataBuf);
                    continue;
                }
                else if (curToken == Token_EOF)
                {
                    //  The element stack better be empty at this point or we
                    //  ended prematurely before all elements were closed.
                    if (!fElemStack.isEmpty())
                    {
                        const ElemStack::StackElem* topElem = fElemStack.popTop();
                        emitError
                        (
                            XMLErrs::EndedWithTagsOnStack
                            , topElem->fThisElement->getFullName()
                        );
                    }

                    // Its the end of file, so clear the got data flag
                    gotData = false;
                    continue;
                }

                // We are in some sort of markup now
                inMarkup = true;

                //  According to the token we got, call the appropriate
                //  scanning method.
                switch(curToken)
                {
                    case Token_CData :
                        // Make sure we are within content
                        if (fElemStack.isEmpty())
                            emitError(XMLErrs::CDATAOutsideOfContent);
                        scanCDSection();
                        break;

                    case Token_Comment :
                        scanComment();
                        break;

                    case Token_EndTag :
                        scanEndTag(gotData);
                        break;

                    case Token_PI :
                        scanPI();
                        break;

                    case Token_StartTag :
                        if (fDoNamespaces)
                            scanStartTagNS(gotData);
                        else
                            scanStartTag(gotData);
                        break;

                    default :
                        fReaderMgr.skipToChar(chOpenAngle);
                        break;
                }

                if (orgReader != fReaderMgr.getCurrentReaderNum())
                    emitError(XMLErrs::PartialMarkupInEntity);

                // And we are back out of markup again
                inMarkup = false;
            }
        }
        catch(const EndOfEntityException& toCatch)
        {
            //  If we were in some markup when this happened, then its a
            //  partial markup error.
            if (inMarkup)
                emitError(XMLErrs::PartialMarkupInEntity);

            // Send an end of entity reference event
            if (fDocHandler)
                fDocHandler->endEntityReference(toCatch.getEntity());

            inMarkup = false;
        }
    }

    // It went ok, so return success
    return true;
}


void IGXMLScanner::scanEndTag(bool& gotData)
{
    //  Assume we will still have data until proven otherwise. It will only
    //  ever be false if this is the end of the root element.
    gotData = true;

    //  Check if the element stack is empty. If so, then this is an unbalanced
    //  element (i.e. more ends than starts, perhaps because of bad text
    //  causing one to be skipped.)
    if (fElemStack.isEmpty())
    {
        emitError(XMLErrs::MoreEndThanStartTags);
        fReaderMgr.skipPastChar(chCloseAngle);
        ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Scan_UnbalancedStartEnd, fMemoryManager);
    }

    //  Pop the stack of the element we are supposed to be ending. Remember
    //  that we don't own this. The stack just keeps them and reuses them.
    unsigned int uriId = (fDoNamespaces)
        ? fElemStack.getCurrentURI() : fEmptyNamespaceId;

    // these get initialized below
    const ElemStack::StackElem* topElem = 0;
    const XMLCh *elemName = 0;

    // Make sure that its the end of the element that we expect
    // special case for schema validation, whose element decls,
    // obviously don't contain prefix information
    if(fGrammarType == Grammar::SchemaGrammarType)
    {
        elemName = fElemStack.getCurrentSchemaElemName();
        topElem = fElemStack.topElement();
    }
    else
    {
        topElem = fElemStack.topElement();
        elemName = topElem->fThisElement->getFullName();
    }
    if (!fReaderMgr.skippedStringLong(elemName))
    {
        emitError
        (
            XMLErrs::ExpectedEndOfTagX
            , elemName
        );
        fReaderMgr.skipPastChar(chCloseAngle);
        fElemStack.popTop();
        return;
    }

    // Make sure we are back on the same reader as where we started
    if (topElem->fReaderNum != fReaderMgr.getCurrentReaderNum())
        emitError(XMLErrs::PartialTagMarkupError);

    // Skip optional whitespace
    fReaderMgr.skipPastSpaces();

    // Make sure we find the closing bracket
    if (!fReaderMgr.skippedChar(chCloseAngle))
    {
        emitError
        (
            XMLErrs::UnterminatedEndTag
            , topElem->fThisElement->getFullName()
        );
    }

    if (fGrammarType == Grammar::SchemaGrammarType)
    {
        // reset error occurred
        fPSVIElemContext.fErrorOccurred = fErrorStack->pop();
        if (fValidate && topElem->fThisElement->isDeclared())
        {
            fPSVIElemContext.fCurrentTypeInfo = ((SchemaValidator*) fValidator)->getCurrentTypeInfo();
            if(!fPSVIElemContext.fCurrentTypeInfo)
                fPSVIElemContext.fCurrentDV = ((SchemaValidator*) fValidator)->getCurrentDatatypeValidator();
            else
                fPSVIElemContext.fCurrentDV = 0;
            if(fPSVIHandler)
            {
                fPSVIElemContext.fNormalizedValue = ((SchemaValidator*) fValidator)->getNormalizedValue();

                if (XMLString::equals(fPSVIElemContext.fNormalizedValue, XMLUni::fgZeroLenString))
                    fPSVIElemContext.fNormalizedValue = 0;
            }
        }
        else
        {
            fPSVIElemContext.fCurrentDV = 0;
            fPSVIElemContext.fCurrentTypeInfo = 0;
            fPSVIElemContext.fNormalizedValue = 0;
        }
    }

    //  If validation is enabled, then lets pass him the list of children and
    //  this element and let him validate it.
    DatatypeValidator* psviMemberType = 0;
    if (fValidate)
    {

       //
       // XML1.0-3rd
       // Validity Constraint:
       // The declaration matches EMPTY and the element has no content (not even
       // entity references, comments, PIs or white space).
       //
       if ( (fGrammarType == Grammar::DTDGrammarType) &&
            (topElem->fCommentOrPISeen)               &&
            (((DTDElementDecl*) topElem->fThisElement)->getModelType() == DTDElementDecl::Empty))
       {
           fValidator->emitError
               (
               XMLValid::EmptyElemHasContent
               , topElem->fThisElement->getFullName()
               );
       }

       //
       // XML1.0-3rd
       // Validity Constraint:
       //
       // The declaration matches children and the sequence of child elements
       // belongs to the language generated by the regular expression in the
       // content model, with optional white space, comments and PIs
       // (i.e. markup matching production [27] Misc) between the start-tag and
       // the first child element, between child elements, or between the last
       // child element and the end-tag.
       //
       // Note that
       //    a CDATA section containing only white space or
       //    a reference to an entity whose replacement text is character references
       //       expanding to white space do not match the nonterminal S, and hence
       //       cannot appear in these positions; however,
       //    a reference to an internal entity with a literal value consisting
       //       of character references expanding to white space does match S,
       //       since its replacement text is the white space resulting from expansion
       //       of the character references.
       //
       if ( (fGrammarType == Grammar::DTDGrammarType)  &&
            (topElem->fReferenceEscaped)               &&
            (((DTDElementDecl*) topElem->fThisElement)->getModelType() == DTDElementDecl::Children))
       {
           fValidator->emitError
               (
               XMLValid::ElemChildrenHasInvalidWS
               , topElem->fThisElement->getFullName()
               );
       }
        XMLSize_t failure;
        bool res = fValidator->checkContent
        (
            topElem->fThisElement
            , topElem->fChildren
            , topElem->fChildCount
            , &failure
        );

        if (!res)
        {
            //  One of the elements is not valid for the content. NOTE that
            //  if no children were provided but the content model requires
            //  them, it comes back with a zero value. But we cannot use that
            //  to index the child array in this case, and have to put out a
            //  special message.
            if (!topElem->fChildCount)
            {
                fValidator->emitError
                (
                    XMLValid::EmptyNotValidForContent
                    , topElem->fThisElement->getFormattedContentModel()
                );
            }
            else if (failure >= topElem->fChildCount)
            {
                fValidator->emitError
                (
                    XMLValid::NotEnoughElemsForCM
                    , topElem->fThisElement->getFormattedContentModel()
                );
            }
            else
            {
                fValidator->emitError
                (
                    XMLValid::ElementNotValidForContent
                    , topElem->fChildren[failure]->getRawName()
                    , topElem->fThisElement->getFormattedContentModel()
                );
            }
        }


        if (fGrammarType == Grammar::SchemaGrammarType) {
            if (((SchemaValidator*) fValidator)->getErrorOccurred())
                fPSVIElemContext.fErrorOccurred = true;
            else if (fPSVIElemContext.fCurrentDV && fPSVIElemContext.fCurrentDV->getType() == DatatypeValidator::Union)
                psviMemberType = fValidationContext->getValidatingMemberType();

            if (fPSVIHandler)
            {
                fPSVIElemContext.fIsSpecified = ((SchemaValidator*) fValidator)->getIsElemSpecified();
                if(fPSVIElemContext.fIsSpecified)
                    fPSVIElemContext.fNormalizedValue = ((SchemaElementDecl *)topElem->fThisElement)->getDefaultValue();
            }

            // call matchers and de-activate context
            if (toCheckIdentityConstraint())
            {
                fICHandler->deactivateContext
                             (
                              (SchemaElementDecl *) topElem->fThisElement
                            , fContent.getRawBuffer()
                            , fValidationContext
                            , fPSVIElemContext.fCurrentDV
                             );
            }

        }
    }

    // QName dv needed topElem to resolve URIs on the checkContent
    fElemStack.popTop();

    // See if it was the root element, to avoid multiple calls below
    const bool isRoot = fElemStack.isEmpty();

    if (fGrammarType == Grammar::SchemaGrammarType)
    {
        if (fPSVIHandler)
        {
            endElementPSVI(
                (SchemaElementDecl*)topElem->fThisElement, psviMemberType);
        }
        // now we can reset the datatype buffer, since the
        // application has had a chance to copy the characters somewhere else
        ((SchemaValidator *)fValidator)->clearDatatypeBuffer();
    }

    // If we have a doc handler, tell it about the end tag
    if (fDocHandler)
    {
        if (fGrammarType == Grammar::SchemaGrammarType) {
            if (topElem->fPrefixColonPos != -1)
                fPrefixBuf.set(elemName, topElem->fPrefixColonPos);
            else
                fPrefixBuf.reset();
        }
        else {
            fPrefixBuf.set(topElem->fThisElement->getElementName()->getPrefix());
        }
        fDocHandler->endElement
        (
            *topElem->fThisElement
            , uriId
            , isRoot
            , fPrefixBuf.getRawBuffer()
        );
    }

    if (fGrammarType == Grammar::SchemaGrammarType) {
        if (!isRoot)
        {
            // update error information
            fErrorStack->push((fErrorStack->size() && fErrorStack->pop()) || fPSVIElemContext.fErrorOccurred);


        }
    }

    // If this was the root, then done with content
    gotData = !isRoot;

    if (gotData) {
        if (fDoNamespaces) {
            // Restore the grammar
            fGrammar = fElemStack.getCurrentGrammar();
            fGrammarType = fGrammar->getGrammarType();
            if (fGrammarType == Grammar::SchemaGrammarType && !fValidator->handlesSchema()) {
                if (fValidatorFromUser)
                    ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Gen_NoSchemaValidator, fMemoryManager);
                else {
                    fValidator = fSchemaValidator;
                }
            }
            else if (fGrammarType == Grammar::DTDGrammarType && !fValidator->handlesDTD()) {
                if (fValidatorFromUser)
                    ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Gen_NoDTDValidator, fMemoryManager);
                else {
                    fValidator = fDTDValidator;
                }
            }

            fValidator->setGrammar(fGrammar);
        }

        // Restore the validation flag
        fValidate = fElemStack.getValidationFlag();
    }
}


//  This method handles the high level logic of scanning the DOCType
//  declaration. This calls the DTDScanner and kicks off both the scanning of
//  the internal subset and the scanning of the external subset, if any.
//
//  When we get here the '<!DOCTYPE' part has already been scanned, which is
//  what told us that we had a doc type decl to parse.
void IGXMLScanner::scanDocTypeDecl()
{
    //  We have a doc type. So, switch the Grammar.
    switchGrammar(XMLUni::fgDTDEntityString);

    if (fDocTypeHandler)
        fDocTypeHandler->resetDocType();

    // There must be some space after DOCTYPE
    bool skippedSomething;
    fReaderMgr.skipPastSpaces(skippedSomething);
    if (!skippedSomething)
    {
        emitError(XMLErrs::ExpectedWhitespace);

        // Just skip the Doctype declaration and return
        fReaderMgr.skipPastChar(chCloseAngle);
        return;
    }

    // Get a buffer for the root element
    XMLBufBid bbRootName(&fBufMgr);

    //  Get a name from the input, which should be the name of the root
    //  element of the upcoming content.
    int  colonPosition;
    bool validName = fDoNamespaces ? fReaderMgr.getQName(bbRootName.getBuffer(), &colonPosition) :
                                     fReaderMgr.getName(bbRootName.getBuffer());
    if (!validName)
    {
        if (bbRootName.isEmpty())
            emitError(XMLErrs::NoRootElemInDOCTYPE);
        else
            emitError(XMLErrs::InvalidRootElemInDOCTYPE, bbRootName.getRawBuffer());
        fReaderMgr.skipPastChar(chCloseAngle);
        return;
    }

    //  Store the root element name for later check
    setRootElemName(bbRootName.getRawBuffer());

    //  This element obviously is not going to exist in the element decl
    //  pool yet, but we need to call docTypeDecl. So force it into
    //  the element decl pool, marked as being there because it was in
    //  the DOCTYPE. Later, when its declared, the status will be updated.
    //
    //  Only do this if we are not reusing the validator! If we are reusing,
    //  then look it up instead. It has to exist!
    MemoryManager* const  rootDeclMgr =
        fUseCachedGrammar ? fMemoryManager : fGrammarPoolMemoryManager;

    DTDElementDecl* rootDecl = new (rootDeclMgr) DTDElementDecl
    (
        bbRootName.getRawBuffer()
        , fEmptyNamespaceId
        , DTDElementDecl::Any
        , rootDeclMgr
    );

    Janitor<DTDElementDecl> rootDeclJanitor(rootDecl);
    rootDecl->setCreateReason(DTDElementDecl::AsRootElem);
    rootDecl->setExternalElemDeclaration(true);
    if(!fUseCachedGrammar)
    {
        fGrammar->putElemDecl(rootDecl);
        rootDeclJanitor.release();
    } else
    {
        // attach this to the undeclared element pool so that it gets deleted
        XMLElementDecl* elemDecl = fDTDElemNonDeclPool->getByKey(bbRootName.getRawBuffer());
        if (elemDecl)
        {
            rootDecl->setId(elemDecl->getId());
        }
        else
        {
            rootDecl->setId(fDTDElemNonDeclPool->put((DTDElementDecl*)rootDecl));
            rootDeclJanitor.release();
        }
    }

    // Skip any spaces after the name
    fReaderMgr.skipPastSpaces();

    //  And now if we are looking at a >, then we are done. It is not
    //  required to have an internal or external subset, though why you
    //  would not escapes me.
    if (fReaderMgr.skippedChar(chCloseAngle)) {

        //  If we have a doc type handler and advanced callbacks are enabled,
        //  call the doctype event.
        if (fDocTypeHandler)
            fDocTypeHandler->doctypeDecl(*rootDecl, 0, 0, false);
        return;
    }

    // either internal/external subset
    if (fValScheme == Val_Auto && !fValidate)
        fValidate = true;

    bool    hasIntSubset = false;
    bool    hasExtSubset = false;
    XMLCh*  sysId = 0;
    XMLCh*  pubId = 0;

    DTDScanner dtdScanner
    (
        (DTDGrammar*) fGrammar
        , fDocTypeHandler
        , fGrammarPoolMemoryManager
        , fMemoryManager
    );
    dtdScanner.setScannerInfo(this, &fReaderMgr, &fBufMgr);

    //  If the next character is '[' then we have no external subset cause
    //  there is no system id, just the opening character of the internal
    //  subset. Else, has to be an id.
    //
    // Just look at the next char, don't eat it.
    if (fReaderMgr.peekNextChar() == chOpenSquare)
    {
        hasIntSubset = true;
    }
    else
    {
        // Indicate we have an external subset
        hasExtSubset = true;
        fHasNoDTD = false;

        // Get buffers for the ids
        XMLBufBid bbPubId(&fBufMgr);
        XMLBufBid bbSysId(&fBufMgr);

        // Get the external subset id
        if (!dtdScanner.scanId(bbPubId.getBuffer(), bbSysId.getBuffer(), DTDScanner::IDType_External))
        {
            fReaderMgr.skipPastChar(chCloseAngle);
            return;
        }

        // Get copies of the ids we got
        pubId = XMLString::replicate(bbPubId.getRawBuffer(), fMemoryManager);
        sysId = XMLString::replicate(bbSysId.getRawBuffer(), fMemoryManager);

        // Skip spaces and check again for the opening of an internal subset
        fReaderMgr.skipPastSpaces();

        // Just look at the next char, don't eat it.
        if (fReaderMgr.peekNextChar() == chOpenSquare) {
            hasIntSubset = true;
        }
    }

    // Insure that the ids get cleaned up, if they got allocated
    ArrayJanitor<XMLCh> janSysId(sysId, fMemoryManager);
    ArrayJanitor<XMLCh> janPubId(pubId, fMemoryManager);

    //  If we have a doc type handler and advanced callbacks are enabled,
    //  call the doctype event.
    if (fDocTypeHandler)
        fDocTypeHandler->doctypeDecl(*rootDecl, pubId, sysId, hasIntSubset, hasExtSubset);

    //  Ok, if we had an internal subset, we are just past the [ character
    //  and need to parse that first.
    if (hasIntSubset)
    {
        // Eat the opening square bracket
        fReaderMgr.getNextChar();

        checkInternalDTD(hasExtSubset, sysId, pubId);

        //  And try to scan the internal subset. If we fail, try to recover
        //  by skipping forward tot he close angle and returning.
        if (!dtdScanner.scanInternalSubset())
        {
            fReaderMgr.skipPastChar(chCloseAngle);
            return;
        }

        //  Do a sanity check that some expanded PE did not propogate out of
        //  the doctype. This could happen if it was terminated early by bad
        //  syntax.
        if (fReaderMgr.getReaderDepth() > 1)
        {
            emitError(XMLErrs::PEPropogated);

            // Ask the reader manager to pop back down to the main level
            fReaderMgr.cleanStackBackTo(1);
        }

        fReaderMgr.skipPastSpaces();
    }

    // And that should leave us at the closing > of the DOCTYPE line
    if (!fReaderMgr.skippedChar(chCloseAngle))
    {
        //  Do a special check for the common scenario of an extra ] char at
        //  the end. This is easy to recover from.
        if (fReaderMgr.skippedChar(chCloseSquare)
        &&  fReaderMgr.skippedChar(chCloseAngle))
        {
            emitError(XMLErrs::ExtraCloseSquare);
        }
         else
        {
            emitError(XMLErrs::UnterminatedDOCTYPE);
            fReaderMgr.skipPastChar(chCloseAngle);
        }
    }

    //  If we had an external subset, then we need to deal with that one
    //  next. If we are reusing the validator, then don't scan it.
    if (hasExtSubset) {

        InputSource* srcUsed=0;
        Janitor<InputSource> janSrc(srcUsed);
        // If we had an internal subset and we're using the cached grammar, it
        // means that the ignoreCachedDTD is set, so we ignore the cached
        // grammar
        if (fUseCachedGrammar && !hasIntSubset)
        {
            srcUsed = resolveSystemId(sysId, pubId);
            if (srcUsed) {
                janSrc.reset(srcUsed);
                Grammar* grammar = fGrammarResolver->getGrammar(srcUsed->getSystemId());

                if (grammar && grammar->getGrammarType() == Grammar::DTDGrammarType) {

                    fDTDGrammar = (DTDGrammar*) grammar;
                    fGrammar = fDTDGrammar;
                    fValidator->setGrammar(fGrammar);
                    // If we don't report at least the external subset boundaries,
                    // an advanced document handler cannot know when the DTD end,
                    // since we've already sent a doctype decl that indicates there's
                    // there's an external subset.
                    if (fDocTypeHandler)
                    {
                        fDocTypeHandler->startExtSubset();
                        fDocTypeHandler->endExtSubset();
                    }

                    return;
                }
            }
        }

        if (fLoadExternalDTD || fValidate)
        {
            // And now create a reader to read this entity
            XMLReader* reader;
            if (srcUsed) {
                reader = fReaderMgr.createReader
                        (
                            *srcUsed
                            , false
                            , XMLReader::RefFrom_NonLiteral
                            , XMLReader::Type_General
                            , XMLReader::Source_External
                            , fCalculateSrcOfs
                            , fLowWaterMark
                        );
            }
            else {
                reader = fReaderMgr.createReader
                        (
                            sysId
                            , pubId
                            , false
                            , XMLReader::RefFrom_NonLiteral
                            , XMLReader::Type_General
                            , XMLReader::Source_External
                            , srcUsed
                            , fCalculateSrcOfs
                            , fLowWaterMark
                            , fDisableDefaultEntityResolution
                        );
                janSrc.reset(srcUsed);
            }
            //  If it failed then throw an exception
            if (!reader)
                ThrowXMLwithMemMgr1(RuntimeException, XMLExcepts::Gen_CouldNotOpenDTD, srcUsed ? srcUsed->getSystemId() : sysId, fMemoryManager);

            if (fToCacheGrammar) {

                unsigned int stringId = fGrammarResolver->getStringPool()->addOrFind(srcUsed->getSystemId());
                const XMLCh* sysIdStr = fGrammarResolver->getStringPool()->getValueForId(stringId);

                fGrammarResolver->orphanGrammar(XMLUni::fgDTDEntityString);
                ((XMLDTDDescription*) (fGrammar->getGrammarDescription()))->setSystemId(sysIdStr);
                fGrammarResolver->putGrammar(fGrammar);
            }

            //  In order to make the processing work consistently, we have to
            //  make this look like an external entity. So create an entity
            //  decl and fill it in and push it with the reader, as happens
            //  with an external entity. Put a janitor on it to insure it gets
            //  cleaned up. The reader manager does not adopt them.
            const XMLCh gDTDStr[] = { chLatin_D, chLatin_T, chLatin_D , chNull };
            DTDEntityDecl* declDTD = new (fMemoryManager) DTDEntityDecl(gDTDStr, false, fMemoryManager);
            declDTD->setSystemId(sysId);
            declDTD->setIsExternal(true);
            Janitor<DTDEntityDecl> janDecl(declDTD);

            // Mark this one as a throw at end
            reader->setThrowAtEnd(true);

            // And push it onto the stack, with its pseudo name
            fReaderMgr.pushReader(reader, declDTD);

            // Tell it its not in an include section
            dtdScanner.scanExtSubsetDecl(false, true);
        }
    }
}

bool IGXMLScanner::scanStartTag(bool& gotData)
{
    //  Assume we will still have data until proven otherwise. It will only
    //  ever be false if this is the root and its empty.
    gotData = true;

    //  Get the QName. In this case, we are not doing namespaces, so we just
    //  use it as is and don't have to break it into parts.
    if (!fReaderMgr.getName(fQNameBuf))
    {
        emitError(XMLErrs::ExpectedElementName);
        fReaderMgr.skipToChar(chOpenAngle);
        return false;
    }

    // Assume it won't be an empty tag
    bool isEmpty = false;

    //  Lets try to look up the element in the validator's element decl pool
    //  We can pass bogus values for the URI id and the base name. We know that
    //  this can only be called if we are doing a DTD style validator and that
    //  he will only look at the QName.
    //
    //  We tell him to fault in a decl if he does not find one.
    //  Actually, we *don't* tell him to fault in a decl if he does not find one- NG
    bool wasAdded = false;
    const XMLCh *rawQName = fQNameBuf.getRawBuffer();
    XMLElementDecl* elemDecl = fGrammar->getElemDecl
    (
        fEmptyNamespaceId
        , 0
        , rawQName
        , Grammar::TOP_LEVEL_SCOPE
    );
    // look for it in the undeclared pool:
    if(!elemDecl)
    {
        elemDecl = fDTDElemNonDeclPool->getByKey(rawQName);
    }
    if(!elemDecl)
    {
        // we're assuming this must be a DTD element.  DTD's can be
        // used with or without namespaces, but schemas cannot be used without
        // namespaces.
        wasAdded = true;
        elemDecl = new (fMemoryManager) DTDElementDecl
        (
            rawQName
            , fEmptyNamespaceId
            , DTDElementDecl::Any
            , fMemoryManager
        );
        elemDecl->setId(fDTDElemNonDeclPool->put((DTDElementDecl*)elemDecl));
    }

    //  We do something different here according to whether we found the
    //  element or not.
    if (wasAdded)
    {
        // If validating then emit an error
        if (fValidate)
        {
            // This is to tell the reuse Validator that this element was
            // faulted-in, was not an element in the validator pool originally
            elemDecl->setCreateReason(XMLElementDecl::JustFaultIn);

            fValidator->emitError
            (
                XMLValid::ElementNotDefined
                , elemDecl->getFullName()
            );
        }
    }
    else
    {
        // If its not marked declared and validating, then emit an error
        if (fValidate && !elemDecl->isDeclared())
        {
            fValidator->emitError
            (
                XMLValid::ElementNotDefined
                , elemDecl->getFullName()
            );
        }
    }

    // See if its the root element
    const bool isRoot = fElemStack.isEmpty();

    // Expand the element stack and add the new element
    fElemStack.addLevel(elemDecl, fReaderMgr.getCurrentReaderNum());
    fElemStack.setValidationFlag(fValidate);

    //  Validate the element
    if (fValidate)
        fValidator->validateElement(elemDecl);

    //  If this is the first element and we are validating, check the root
    //  element.
    if (isRoot)
    {
        fRootGrammar = fGrammar;

        if (fValidate)
        {
            //  If a DocType exists, then check if it matches the root name there.
            if (fRootElemName && !XMLString::equals(fQNameBuf.getRawBuffer(), fRootElemName))
                fValidator->emitError(XMLValid::RootElemNotLikeDocType);
        }
    }
    else
    {
        //  If the element stack is not empty, then add this element as a
        //  child of the previous top element. If its empty, this is the root
        //  elem and is not the child of anything.
        fElemStack.addChild(elemDecl->getElementName(), true);
    }

    // Skip any whitespace after the name
    fReaderMgr.skipPastSpaces();

    //  We loop until we either see a /> or >, handling attribute/value
    //  pairs until we get there.
    XMLSize_t    attCount = 0;
    XMLSize_t    curAttListSize = fAttrList->size();
    wasAdded = false;

    fElemCount++;

    while (true)
    {
        // And get the next non-space character
        XMLCh nextCh = fReaderMgr.peekNextChar();

        //  If the next character is not a slash or closed angle bracket,
        //  then it must be whitespace, since whitespace is required
        //  between the end of the last attribute and the name of the next
        //  one.
        if (attCount)
        {
            if ((nextCh != chForwardSlash) && (nextCh != chCloseAngle))
            {
                bool bFoundSpace;
                fReaderMgr.skipPastSpaces(bFoundSpace);
                if (!bFoundSpace)
                {
                    // Emit the error but keep on going
                    emitError(XMLErrs::ExpectedWhitespace);
                }
                // Ok, peek another char
                nextCh = fReaderMgr.peekNextChar();
            }
        }

        //  Ok, here we first check for any of the special case characters.
        //  If its not one, then we do the normal case processing, which
        //  assumes that we've hit an attribute value, Otherwise, we do all
        //  the special case checks.
        if (!fReaderMgr.getCurrentReader()->isSpecialStartTagChar(nextCh))
        {
            //  Assume its going to be an attribute, so get a name from
            //  the input.
            if (!fReaderMgr.getName(fAttNameBuf))
            {
                emitError(XMLErrs::ExpectedAttrName);
                fReaderMgr.skipPastChar(chCloseAngle);
                return false;
            }

            // And next must be an equal sign
            if (!scanEq())
            {
                static const XMLCh tmpList[] =
                {
                    chSingleQuote, chDoubleQuote, chCloseAngle
                    , chOpenAngle, chForwardSlash, chNull
                };

                emitError(XMLErrs::ExpectedEqSign);

                //  Try to sync back up by skipping forward until we either
                //  hit something meaningful.
                const XMLCh chFound = fReaderMgr.skipUntilInOrWS(tmpList);

                if ((chFound == chCloseAngle) || (chFound == chForwardSlash))
                {
                    // Jump back to top for normal processing of these
                    continue;
                }
                else if ((chFound == chSingleQuote)
                      ||  (chFound == chDoubleQuote)
                      ||  fReaderMgr.getCurrentReader()->isWhitespace(chFound))
                {
                    // Just fall through assuming that the value is to follow
                }
                else if (chFound == chOpenAngle)
                {
                    // Assume a malformed tag and that new one is starting
                    emitError(XMLErrs::UnterminatedStartTag, elemDecl->getFullName());
                    return false;
                }
                else
                {
                    // Something went really wrong
                    return false;
                }
            }
            //  See if this attribute is declared for this element. If we are
            //  not validating of course it will not be at first, but we will
            //  fault it into the pool (to avoid lots of redundant errors.)
            XMLCh * namePtr = fAttNameBuf.getRawBuffer();
            XMLAttDef* attDef = ((DTDElementDecl *)elemDecl)->getAttDef(namePtr);

            //  Add this attribute to the attribute list that we use to
            //  pass them to the handler. We reuse its existing elements
            //  but expand it as required.
            // Note that we want to this first since this will
            // make a copy of the namePtr; we can then make use of
            // that copy in the hashtable lookup that checks
            // for duplicates.  This will mean we may have to update
            // the type of the XMLAttr later.
            XMLAttr* curAtt;
            if (attCount >= curAttListSize)
            {
                curAtt = new (fMemoryManager) XMLAttr
                (
                    0
                    , namePtr
                    , XMLUni::fgZeroLenString
                    , XMLUni::fgZeroLenString
                    , (attDef)?attDef->getType():XMLAttDef::CData
                    , true
                    , fMemoryManager
                );
                fAttrList->addElement(curAtt);
            }
            else
            {
                curAtt = fAttrList->elementAt(attCount);
                curAtt->set
                (
                    0
                    , namePtr
                    , XMLUni::fgZeroLenString
                    , XMLUni::fgZeroLenString
                    , (attDef)?attDef->getType():XMLAttDef::CData
                );
                curAtt->setSpecified(true);
            }
            // reset namePtr so it refers to newly-allocated memory
            namePtr = (XMLCh *)curAtt->getName();

            if (!attDef)
            {
                //  If there is a validation handler, then we are validating
                //  so emit an error.
                if (fValidate)
                {
                    fValidator->emitError
                    (
                        XMLValid::AttNotDefinedForElement
                        , fAttNameBuf.getRawBuffer()
                        , elemDecl->getFullName()
                    );
                }
                if(!fUndeclaredAttrRegistry->putIfNotPresent(namePtr, 0))
                {
                    emitError
                    (
                        XMLErrs::AttrAlreadyUsedInSTag
                        , namePtr
                        , elemDecl->getFullName()
                     );
                }
            }
            else
            {
                // prepare for duplicate detection
                unsigned int *curCountPtr = fAttDefRegistry->get(attDef);
                if(!curCountPtr)
                {
                    curCountPtr = getNewUIntPtr();
                    *curCountPtr = fElemCount;
                    fAttDefRegistry->put(attDef, curCountPtr);
                }
                else if(*curCountPtr < fElemCount)
                    *curCountPtr = fElemCount;
                else
                {
                    emitError
                    (
                        XMLErrs::AttrAlreadyUsedInSTag
                        , attDef->getFullName()
                        , elemDecl->getFullName()
                    );
                }
            }

            //  Skip any whitespace before the value and then scan the att
            //  value. This will come back normalized with entity refs and
            //  char refs expanded.
            fReaderMgr.skipPastSpaces();
            if (!scanAttValue(attDef, namePtr, fAttValueBuf))
            {
                static const XMLCh tmpList[] =
                {
                    chCloseAngle, chOpenAngle, chForwardSlash, chNull
                };

                emitError(XMLErrs::ExpectedAttrValue);

                //  It failed, so lets try to get synced back up. We skip
                //  forward until we find some whitespace or one of the
                //  chars in our list.
                const XMLCh chFound = fReaderMgr.skipUntilInOrWS(tmpList);

                if ((chFound == chCloseAngle)
                ||  (chFound == chForwardSlash)
                ||  fReaderMgr.getCurrentReader()->isWhitespace(chFound))
                {
                    //  Just fall through and process this attribute, though
                    //  the value will be "".
                }
                else if (chFound == chOpenAngle)
                {
                    // Assume a malformed tag and that new one is starting
                    emitError(XMLErrs::UnterminatedStartTag, elemDecl->getFullName());
                    return false;
                }
                else
                {
                    // Something went really wrong
                    return false;
                }
            }
            // must set the newly-minted value on the XMLAttr:
            curAtt->setValue(fAttValueBuf.getRawBuffer());

            //  Now that its all stretched out, lets look at its type and
            //  determine if it has a valid value. It will output any needed
            //  errors, but we just keep going. We only need to do this if
            //  we are validating.
            if (attDef)
            {
                // Let the validator pass judgement on the attribute value
                if (fValidate)
                {
                    fValidator->validateAttrValue
                    (
                        attDef
                        , fAttValueBuf.getRawBuffer()
                        , false
                        , elemDecl
                    );
                }
            }

            attCount++;
            // And jump back to the top of the loop
            continue;
        }

        //  It was some special case character so do all of the checks and
        //  deal with it.
        if (!nextCh)
            ThrowXMLwithMemMgr(UnexpectedEOFException, XMLExcepts::Gen_UnexpectedEOF, fMemoryManager);

        if (nextCh == chForwardSlash)
        {
            fReaderMgr.getNextChar();
            isEmpty = true;
            if (!fReaderMgr.skippedChar(chCloseAngle))
                emitError(XMLErrs::UnterminatedStartTag, elemDecl->getFullName());
            break;
        }
        else if (nextCh == chCloseAngle)
        {
            fReaderMgr.getNextChar();
            break;
        }
        else if (nextCh == chOpenAngle)
        {
            //  Check for this one specially, since its going to be common
            //  and it is kind of auto-recovering since we've already hit the
            //  next open bracket, which is what we would have seeked to (and
            //  skipped this whole tag.)
            emitError(XMLErrs::UnterminatedStartTag, elemDecl->getFullName());
            break;
        }
        else if ((nextCh == chSingleQuote) || (nextCh == chDoubleQuote))
        {
            //  Check for this one specially, which is probably a missing
            //  attribute name, e.g. ="value". Just issue expected name
            //  error and eat the quoted string, then jump back to the
            //  top again.
            emitError(XMLErrs::ExpectedAttrName);
            fReaderMgr.getNextChar();
            fReaderMgr.skipQuotedString(nextCh);
            fReaderMgr.skipPastSpaces();
            continue;
        }
    }

    if(attCount)
    {
        // clean up after ourselves:
        // clear the map used to detect duplicate attributes
        fUndeclaredAttrRegistry->removeAll();
    }

    //  Ok, so lets get an enumerator for the attributes of this element
    //  and run through them for well formedness and validity checks. But
    //  make sure that we had any attributes before we do it, since the list
    //  would have have gotten faulted in anyway.
    if (elemDecl->hasAttDefs())
    {
        // N.B.:  this assumes DTD validation.
        XMLAttDefList& attDefList = elemDecl->getAttDefList();
        for(XMLSize_t i=0; i<attDefList.getAttDefCount(); i++)
        {
            // Get the current att def, for convenience and its def type
            const XMLAttDef& curDef = attDefList.getAttDef(i);
            const XMLAttDef::DefAttTypes defType = curDef.getDefaultType();

            unsigned int *attCountPtr = fAttDefRegistry->get(&curDef);
            if (!attCountPtr || *attCountPtr < fElemCount)
            { // did not occur
                if (fValidate)
                {
                    // If we are validating and its required, then an error
                    if (defType == XMLAttDef::Required)
                    {
                        fValidator->emitError
                        (
                            XMLValid::RequiredAttrNotProvided
                            , curDef.getFullName()
                        );
                    }
                    else if ((defType == XMLAttDef::Default) ||
		                       (defType == XMLAttDef::Fixed)  )
                    {
                        if (fStandalone && curDef.isExternal())
                        {
                            // XML 1.0 Section 2.9
                            // Document is standalone, so attributes must not be defaulted.
                            fValidator->emitError(XMLValid::NoDefAttForStandalone, curDef.getFullName(), elemDecl->getFullName());

                        }
                    }
                }

                // Fault in the value if needed, and bump the att count
                if ((defType == XMLAttDef::Default)
                ||  (defType == XMLAttDef::Fixed))
                {
                    // Let the validator pass judgement on the attribute value
                    if (fValidate)
                    {
                        fValidator->validateAttrValue
                        (
                            &curDef
                            , curDef.getValue()
                            , false
                            , elemDecl
                        );
                    }

                    XMLAttr* curAtt;
                    if (attCount >= curAttListSize)
                    {
                        curAtt = new (fMemoryManager) XMLAttr
                        (
                            0
                            , curDef.getFullName()
                            , XMLUni::fgZeroLenString
                            , curDef.getValue()
                            , curDef.getType()
                            , false
                            , fMemoryManager
                        );
                        fAttrList->addElement(curAtt);
                        curAttListSize++;
                    }
                    else
                    {
                        curAtt = fAttrList->elementAt(attCount);
                        curAtt->set
                        (
                            0
                            , curDef.getFullName()
                            , XMLUni::fgZeroLenString
                            , curDef.getValue()
                            , curDef.getType()
                        );
                        curAtt->setSpecified(false);
                    }
                    attCount++;
                }
            }
        }
    }

    //  If empty, validate content right now if we are validating and then
    //  pop the element stack top. Else, we have to update the current stack
    //  top's namespace mapping elements.
    if (isEmpty)
    {
        // If validating, then insure that its legal to have no content
        if (fValidate)
        {
            XMLSize_t failure;
            bool res = fValidator->checkContent(elemDecl, 0, 0, &failure);
            if (!res)
            {
                fValidator->emitError
                (
                    XMLValid::ElementNotValidForContent
                    , elemDecl->getFullName()
                    , elemDecl->getFormattedContentModel()
                );
            }
        }

        // Pop the element stack back off since it'll never be used now
        fElemStack.popTop();

        // If the elem stack is empty, then it was an empty root
        if (isRoot)
            gotData = false;
        else {
            // Restore the validation flag
            fValidate = fElemStack.getValidationFlag();
        }
    }

    //  If we have a document handler, then tell it about this start tag. We
    //  don't have any URI id to send along, so send fEmptyNamespaceId. We also do not send
    //  any prefix since its just one big name if we are not doing namespaces.
    if (fDocHandler)
    {
        fDocHandler->startElement
        (
            *elemDecl
            , fEmptyNamespaceId
            , 0
            , *fAttrList
            , attCount
            , isEmpty
            , isRoot
        );
    }

    return true;
}


//  This method is called to scan a start tag when we are processing
//  namespaces. There are two different versions of this method, one for
//  namespace aware processing and one for non-namespace aware processing.
//
//  This method is called after we've scanned the < of a start tag. So we
//  have to get the element name, then scan the attributes, after which
//  we are either going to see >, />, or attributes followed by one of those
//  sequences.
bool IGXMLScanner::scanStartTagNS(bool& gotData)
{
    //  Assume we will still have data until proven otherwise. It will only
    //  ever be false if this is the root and its empty.
    gotData = true;

    // Reset element content buffer
    fContent.reset();

    //  The current position is after the open bracket, so we need to read in
    //  in the element name.
    int prefixColonPos;
    if (!fReaderMgr.getQName(fQNameBuf, &prefixColonPos))
    {
        if (fQNameBuf.isEmpty())
            emitError(XMLErrs::ExpectedElementName);
        else
            emitError(XMLErrs::InvalidElementName, fQNameBuf.getRawBuffer());
        fReaderMgr.skipToChar(chOpenAngle);
        return false;
    }

    // See if its the root element
    const bool isRoot = fElemStack.isEmpty();

    // Skip any whitespace after the name
    fReaderMgr.skipPastSpaces();

    //  First we have to do the rawest attribute scan. We don't do any
    //  normalization of them at all, since we don't know yet what type they
    //  might be (since we need the element decl in order to do that.)
    bool isEmpty;
    XMLSize_t attCount = rawAttrScan
    (
        fQNameBuf.getRawBuffer()
        , *fRawAttrList
        , isEmpty
    );

    // save the contentleafname and currentscope before addlevel, for later use
    ContentLeafNameTypeVector* cv = 0;
    XMLContentModel* cm = 0;
    unsigned int currentScope = Grammar::TOP_LEVEL_SCOPE;
    bool laxThisOne = false;

    if (!isRoot && fGrammarType == Grammar::SchemaGrammarType)
    {
        // schema validator will have correct type if validating
        SchemaElementDecl* tempElement = (SchemaElementDecl*)
            fElemStack.topElement()->fThisElement;
        SchemaElementDecl::ModelTypes modelType = tempElement->getModelType();
        ComplexTypeInfo *currType = 0;

        if (fValidate)
        {
            currType = ((SchemaValidator*)fValidator)->getCurrentTypeInfo();
            if (currType)
                modelType = (SchemaElementDecl::ModelTypes)currType->getContentType();
            else // something must have gone wrong
                modelType = SchemaElementDecl::Any;
        }
        else
        {
            currType = tempElement->getComplexTypeInfo();
        }

        if ((modelType == SchemaElementDecl::Mixed_Simple)
          ||  (modelType == SchemaElementDecl::Mixed_Complex)
          ||  (modelType == SchemaElementDecl::Children))
        {
            cm = currType->getContentModel();
            cv = cm->getContentLeafNameTypeVector();
            currentScope = fElemStack.getCurrentScope();
        }
        else if (modelType == SchemaElementDecl::Any) {
            laxThisOne = true;
        }
    }

    //  Now, since we might have to update the namespace map for this element,
    //  but we don't have the element decl yet, we just tell the element stack
    //  to expand up to get ready.
    XMLSize_t elemDepth = fElemStack.addLevel();
    fElemStack.setValidationFlag(fValidate);
    fElemStack.setPrefixColonPos(prefixColonPos);

    //  Check if there is any external schema location specified, and if we are at root,
    //  go through them first before scanning those specified in the instance document
    if (isRoot && fDoSchema
        && (fExternalSchemaLocation || fExternalNoNamespaceSchemaLocation)) {

        if (fExternalSchemaLocation)
            parseSchemaLocation(fExternalSchemaLocation, true);
        if (fExternalNoNamespaceSchemaLocation)
            resolveSchemaGrammar(fExternalNoNamespaceSchemaLocation, XMLUni::fgZeroLenString, true);
    }

    //  Make an initial pass through the list and find any xmlns attributes or
    //  schema attributes.
    if (attCount) {
        scanRawAttrListforNameSpaces(attCount);
    }

    //  Also find any default or fixed xmlns attributes in DTD defined for
    //  this element.
    XMLElementDecl* elemDecl = 0;
    const XMLCh* qnameRawBuf = fQNameBuf.getRawBuffer();

    if (fGrammarType == Grammar::DTDGrammarType) {

        if (!fSkipDTDValidation) {
            elemDecl = fGrammar->getElemDecl(
                fEmptyNamespaceId, 0, qnameRawBuf, Grammar::TOP_LEVEL_SCOPE
            );

            if (elemDecl) {
                if (elemDecl->hasAttDefs()) {
                    XMLAttDefList& attDefList = elemDecl->getAttDefList();
                    for(XMLSize_t i=0; i<attDefList.getAttDefCount(); i++)
                    {
                        // Get the current att def, for convenience and its def type
                        const XMLAttDef& curDef = attDefList.getAttDef(i);
                        const XMLAttDef::DefAttTypes defType = curDef.getDefaultType();

                        // update the NSMap if there are any default/fixed xmlns attributes
                        if ((defType == XMLAttDef::Default)
                        ||  (defType == XMLAttDef::Fixed))
                        {
                            const XMLCh* rawPtr = curDef.getFullName();
                            if (!XMLString::compareNString(rawPtr, XMLUni::fgXMLNSColonString, 6)
                            ||  XMLString::equals(rawPtr, XMLUni::fgXMLNSString))
                                updateNSMap(rawPtr, curDef.getValue());
                        }
                    }
                }
            }
        }

        if (!elemDecl) {
            elemDecl = fDTDElemNonDeclPool->getByKey(qnameRawBuf);
        }
    }

    //  Resolve the qualified name to a URI and name so that we can look up
    //  the element decl for this element. We have now update the prefix to
    //  namespace map so we should get the correct element now.
    unsigned int uriId = resolveQNameWithColon(
        qnameRawBuf, fPrefixBuf, ElemStack::Mode_Element, prefixColonPos
    );

    //if schema, check if we should lax or skip the validation of this element
    bool parentValidation = fValidate;
    if (cv) {
        QName element(fPrefixBuf.getRawBuffer(), &qnameRawBuf[prefixColonPos + 1], uriId, fMemoryManager);
        // elementDepth will be > 0, as cv is only constructed if element is not
        // root.
        laxThisOne = laxElementValidation(&element, cv, cm, elemDepth - 1);
    }

    //  Look up the element now in the grammar. This will get us back a
    //  generic element decl object. We tell him to fault one in if he does
    //  not find it.
    bool wasAdded = false;
    const XMLCh* nameRawBuf = &qnameRawBuf[prefixColonPos + 1];

    if (fDoSchema) {

        if (fGrammarType == Grammar::DTDGrammarType) {
            if (!switchGrammar(getURIText(uriId))) {
                fValidator->emitError(
                    XMLValid::GrammarNotFound, getURIText(uriId)
                );
            }
        }

        if (fGrammarType == Grammar::SchemaGrammarType) {
            elemDecl = fGrammar->getElemDecl(
                uriId, nameRawBuf, qnameRawBuf, currentScope
            );

            // if not found, then it may be a reference, try TOP_LEVEL_SCOPE
            if (!elemDecl) {
                bool checkTopLevel = (currentScope != Grammar::TOP_LEVEL_SCOPE);
                const XMLCh* original_uriStr = fGrammar->getTargetNamespace();
                unsigned int orgGrammarUri = fURIStringPool->getId(original_uriStr);

                if (orgGrammarUri != uriId) {
                    if (switchGrammar(getURIText(uriId))) {
                        checkTopLevel = true;
                    }
                    else {
                        // the laxElementValidation routine (called above) will
                        // set fValidate to false for a "skipped" element
                        if (!laxThisOne && fValidate) {
                            fValidator->emitError(
                                XMLValid::GrammarNotFound, getURIText(uriId)
                            );
                        }
                        checkTopLevel = false;
                    }
                }

                if (checkTopLevel) {
                    elemDecl = fGrammar->getElemDecl(
                        uriId, nameRawBuf, qnameRawBuf, Grammar::TOP_LEVEL_SCOPE
                    );
                }

                if (!elemDecl && currentScope != Grammar::TOP_LEVEL_SCOPE) {

                    if (orgGrammarUri == uriId) {
                        // still not found in specified uri
                        // try emptyNamespace see if element should be
                        // un-qualified.
                        // Use a temp variable until we decide this is the case
                        if (uriId != fEmptyNamespaceId) {
                            XMLElementDecl* tempElemDecl = fGrammar->getElemDecl(
                                fEmptyNamespaceId, nameRawBuf, qnameRawBuf, currentScope
                            );

                            if (tempElemDecl && tempElemDecl->getCreateReason() != XMLElementDecl::JustFaultIn && fValidate) {
                                fValidator->emitError(
                                    XMLValid::ElementNotUnQualified, qnameRawBuf
                                );
                                elemDecl = tempElemDecl;
                            }
                        }
                    }
                    // still Not found in specified uri
                    // go to original Grammar again to see if element needs
                    // to be fully qualified.
                    // Use a temp variable until we decide this is the case
                    else if (uriId == fEmptyNamespaceId) {

                        if (switchGrammar(original_uriStr)) {
                            XMLElementDecl* tempElemDecl = fGrammar->getElemDecl(
                                orgGrammarUri, nameRawBuf, qnameRawBuf, currentScope
                            );
                            if (tempElemDecl && tempElemDecl->getCreateReason() != XMLElementDecl::JustFaultIn && fValidate) {
                                fValidator->emitError(
                                    XMLValid::ElementNotQualified, qnameRawBuf
                                );
                                elemDecl = tempElemDecl;
                            }
                        }
                        else if (!laxThisOne && fValidate) {
                            fValidator->emitError(
                                XMLValid::GrammarNotFound,original_uriStr
                            );
                        }
                    }
                }

                if (!elemDecl) {
                    // still not found
                    // switch back to original grammar first if necessary
                    if (orgGrammarUri != uriId) {
                        switchGrammar(original_uriStr);
                    }

                    // look in the list of undeclared elements, as would have been
                    // done before we made grammars stateless:
                    elemDecl = fSchemaElemNonDeclPool->getByKey(
                        nameRawBuf, uriId, (int)Grammar::TOP_LEVEL_SCOPE
                    );
                }
            }
        }
    }

    if (!elemDecl) {

        if (fGrammarType == Grammar::DTDGrammarType) {
            elemDecl = new (fMemoryManager) DTDElementDecl(
                qnameRawBuf, uriId, DTDElementDecl::Any, fMemoryManager
            );
            elemDecl->setId(fDTDElemNonDeclPool->put((DTDElementDecl*)elemDecl));
        }
        else if (fGrammarType == Grammar::SchemaGrammarType)  {
            elemDecl = new (fMemoryManager) SchemaElementDecl(
                fPrefixBuf.getRawBuffer(), nameRawBuf, uriId
                , SchemaElementDecl::Any, Grammar::TOP_LEVEL_SCOPE
                , fMemoryManager
            );
            elemDecl->setId(
                fSchemaElemNonDeclPool->put((void*)elemDecl->getBaseName()
                , uriId, (int)Grammar::TOP_LEVEL_SCOPE, (SchemaElementDecl*)elemDecl)
            );
        }
        wasAdded = true;
    }

    // this info needed for DOMTypeInfo
    fPSVIElemContext.fErrorOccurred = false;

    //  We do something different here according to whether we found the
    //  element or not.
    bool bXsiTypeSet= (fValidator && fGrammarType == Grammar::SchemaGrammarType)?((SchemaValidator*)fValidator)->getIsXsiTypeSet():false;
    if (wasAdded)
    {
        if (laxThisOne && !bXsiTypeSet) {
            fValidate = false;
            fElemStack.setValidationFlag(fValidate);
        }
        else if (fValidate)
        {
            // If validating then emit an error

            // This is to tell the reuse Validator that this element was
            // faulted-in, was not an element in the grammar pool originally
            elemDecl->setCreateReason(XMLElementDecl::JustFaultIn);

            // xsi:type was specified, don't complain about missing definition
            if(!bXsiTypeSet)
            {
                fValidator->emitError
                (
                    XMLValid::ElementNotDefined
                    , elemDecl->getFullName()
                );

                if(fGrammarType == Grammar::SchemaGrammarType)
                {
                    fPSVIElemContext.fErrorOccurred = true;
                }
            }
        }
    }
    else
    {
        // If its not marked declared and validating, then emit an error
        if (!elemDecl->isDeclared()) {
            if(elemDecl->getCreateReason() == XMLElementDecl::NoReason) {
                if(!bXsiTypeSet && fGrammarType == Grammar::SchemaGrammarType) {
                    fPSVIElemContext.fErrorOccurred = true;
                }
            }

            if (laxThisOne) {
                fValidate = false;
                fElemStack.setValidationFlag(fValidate);
            }
            else if (fValidate && !bXsiTypeSet)
            {
                fValidator->emitError
                (
                    XMLValid::ElementNotDefined
                    , elemDecl->getFullName()
                );
            }
        }
    }

    //  Now we can update the element stack to set the current element
    //  decl. We expanded the stack above, but couldn't store the element
    //  decl because we didn't know it yet.
    fElemStack.setElement(elemDecl, fReaderMgr.getCurrentReaderNum());
    fElemStack.setCurrentURI(uriId);

    if (isRoot)
    {
        fRootGrammar = fGrammar;
        if (fGrammarType == Grammar::SchemaGrammarType && !fRootElemName)
            fRootElemName = XMLString::replicate(qnameRawBuf, fMemoryManager);
    }

    if (fGrammarType == Grammar::SchemaGrammarType && fPSVIHandler)
    {

        fPSVIElemContext.fElemDepth++;
        if (elemDecl->isDeclared())
        {
            fPSVIElemContext.fNoneValidationDepth = fPSVIElemContext.fElemDepth;
        }
        else
        {
            fPSVIElemContext.fFullValidationDepth = fPSVIElemContext.fElemDepth;

            /******
             * While we report an error for historical reasons, this should
             * actually result in lax assessment - NG.
            if (isRoot && fValidate)
                fPSVIElemContext.fErrorOccurred = true;
            *****/
        }
    }

    //  Validate the element
    if (fValidate)
    {
        fValidator->validateElement(elemDecl);
        if (fValidator->handlesSchema())
        {
            if (((SchemaValidator*) fValidator)->getErrorOccurred())
                fPSVIElemContext.fErrorOccurred = true;
        }
    }

    if (fGrammarType == Grammar::SchemaGrammarType) {

        // squirrel away the element's QName, so that we can do an efficient
        // end-tag match
        fElemStack.setCurrentSchemaElemName(fQNameBuf.getRawBuffer());

        ComplexTypeInfo* typeinfo = (fValidate)
            ? ((SchemaValidator*)fValidator)->getCurrentTypeInfo()
            : ((SchemaElementDecl*) elemDecl)->getComplexTypeInfo();

        if (typeinfo) {
            currentScope = typeinfo->getScopeDefined();

            // switch grammar if the typeinfo has a different grammar (happens when there is xsi:type)
            XMLCh* typeName = typeinfo->getTypeName();
            const int comma = XMLString::indexOf(typeName, chComma);
            if (comma > 0) {
                XMLBuffer prefixBuf(comma+1, fMemoryManager);
                prefixBuf.append(typeName, comma);
                const XMLCh* uriStr = prefixBuf.getRawBuffer();

                bool errorCondition = !switchGrammar(uriStr) && fValidate;
                if (errorCondition && !laxThisOne)
                {
                    fValidator->emitError
                    (
                        XMLValid::GrammarNotFound
                        , prefixBuf.getRawBuffer()
                    );
                }
            }
            else if (comma == 0) {
                bool errorCondition = !switchGrammar(XMLUni::fgZeroLenString) && fValidate;
                if (errorCondition && !laxThisOne)
                {
                    fValidator->emitError
                    (
                        XMLValid::GrammarNotFound
                        , XMLUni::fgZeroLenString
                    );
                }
            }
        }
        fElemStack.setCurrentScope(currentScope);

        // Set element next state
        if (elemDepth >= fElemStateSize) {
            resizeElemState();
        }

        fElemState[elemDepth] = 0;
        fElemLoopState[elemDepth] = 0;
    }

    fElemStack.setCurrentGrammar(fGrammar);

    //  If this is the first element and we are validating, check the root
    //  element.
    if (isRoot)
    {
        if (fValidate)
        {
            //  If a DocType exists, then check if it matches the root name there.
            if (fRootElemName && !XMLString::equals(qnameRawBuf, fRootElemName))
                fValidator->emitError(XMLValid::RootElemNotLikeDocType);
        }
    }
    else if (parentValidation)
    {
        //  If the element stack is not empty, then add this element as a
        //  child of the previous top element. If its empty, this is the root
        //  elem and is not the child of anything.
        fElemStack.addChild(elemDecl->getElementName(), true);
    }

    // PSVI handling:  even if it turns out there are
    // no attributes, we need to reset this list...
    if(getPSVIHandler() && fGrammarType == Grammar::SchemaGrammarType )
        fPSVIAttrList->reset();

    //  Now lets get the fAttrList filled in. This involves faulting in any
    //  defaulted and fixed attributes and normalizing the values of any that
    //  we got explicitly.
    //
    //  We update the attCount value with the total number of attributes, but
    //  it goes in with the number of values we got during the raw scan of
    //  explictly provided attrs above.
    attCount = buildAttList(*fRawAttrList, attCount, elemDecl, *fAttrList);
    if(attCount)
    {
        // clean up after ourselves:
        // clear the map used to detect duplicate attributes
        fUndeclaredAttrRegistry->removeAll();
    }

    // activate identity constraints
    if (fGrammar  &&
        fGrammarType == Grammar::SchemaGrammarType &&
        toCheckIdentityConstraint())
    {
        fICHandler->activateIdentityConstraint
                        (
                          (SchemaElementDecl*) elemDecl
                        , (int) elemDepth
                        , uriId
                        , fPrefixBuf.getRawBuffer()
                        , *fAttrList
                        , attCount
                        , fValidationContext
                        );
    }

    // Since the element may have default values, call start tag now regardless if it is empty or not
    // If we have a document handler, then tell it about this start tag
    if (fDocHandler)
    {
        fDocHandler->startElement
        (
            *elemDecl
            , uriId
            , fPrefixBuf.getRawBuffer()
            , *fAttrList
            , attCount
            , false
            , isRoot
        );
    }

    // if we have a PSVIHandler, now's the time to call
    // its handleAttributesPSVI method:
    if(fPSVIHandler && fGrammarType == Grammar::SchemaGrammarType)
    {
        QName *eName = elemDecl->getElementName();
        fPSVIHandler->handleAttributesPSVI
        (
            eName->getLocalPart()
            , fURIStringPool->getValueForId(eName->getURI())
            , fPSVIAttrList
        );
    }

    //  If empty, validate content right now if we are validating and then
    //  pop the element stack top. Else, we have to update the current stack
    //  top's namespace mapping elements.
    if (isEmpty)
    {
        // Pop the element stack back off since it'll never be used now
        fElemStack.popTop();

        // reset current type info
        DatatypeValidator* psviMemberType = 0;
        if (fGrammarType == Grammar::SchemaGrammarType)
        {
            if (fValidate && elemDecl->isDeclared())
            {
                fPSVIElemContext.fCurrentTypeInfo = ((SchemaValidator*) fValidator)->getCurrentTypeInfo();
                if(!fPSVIElemContext.fCurrentTypeInfo)
                    fPSVIElemContext.fCurrentDV = ((SchemaValidator*) fValidator)->getCurrentDatatypeValidator();
                else
                    fPSVIElemContext.fCurrentDV = 0;
                if(fPSVIHandler)
                {
                    fPSVIElemContext.fNormalizedValue = ((SchemaValidator*) fValidator)->getNormalizedValue();

                    if (XMLString::equals(fPSVIElemContext.fNormalizedValue, XMLUni::fgZeroLenString))
                        fPSVIElemContext.fNormalizedValue = 0;
                }
            }
            else
            {
                fPSVIElemContext.fCurrentDV = 0;
                fPSVIElemContext.fCurrentTypeInfo = 0;
                fPSVIElemContext.fNormalizedValue = 0;
            }
        }

        // If validating, then insure that its legal to have no content
        if (fValidate)
        {
            XMLSize_t failure;
            bool res = fValidator->checkContent(elemDecl, 0, 0, &failure);
            if (!res)
            {
                fValidator->emitError
                (
                    XMLValid::ElementNotValidForContent
                    , elemDecl->getFullName()
                    , elemDecl->getFormattedContentModel()
                );
            }

            if (fGrammarType == Grammar::SchemaGrammarType) {

                if (((SchemaValidator*) fValidator)->getErrorOccurred())
                {
                    fPSVIElemContext.fErrorOccurred = true;
                }
                else
                {
                    if (fPSVIHandler)
                    {
                        fPSVIElemContext.fIsSpecified = ((SchemaValidator*) fValidator)->getIsElemSpecified();
                        if(fPSVIElemContext.fIsSpecified)
                            fPSVIElemContext.fNormalizedValue = ((SchemaElementDecl *)elemDecl)->getDefaultValue();
                    }
                    // note that if we're empty, won't be a current DV
                    if (fPSVIElemContext.fCurrentDV && fPSVIElemContext.fCurrentDV->getType() == DatatypeValidator::Union)
                        psviMemberType = fValidationContext->getValidatingMemberType();
                }

                // call matchers and de-activate context
                if (toCheckIdentityConstraint())
                {
                    fICHandler->deactivateContext
                                   (
                                    (SchemaElementDecl *) elemDecl
                                  , fContent.getRawBuffer()
                                  , fValidationContext
                                  , fPSVIElemContext.fCurrentDV
                                   );
                }

            }
        }
        else if (fGrammarType == Grammar::SchemaGrammarType) {
            ((SchemaValidator*)fValidator)->resetNillable();
        }

        if (fGrammarType == Grammar::SchemaGrammarType)
        {
            if (fPSVIHandler)
            {
                endElementPSVI((SchemaElementDecl*)elemDecl, psviMemberType);
            }
        }

        // If we have a doc handler, tell it about the end tag
        if (fDocHandler)
        {
            fDocHandler->endElement
            (
                *elemDecl
                , uriId
                , isRoot
                , fPrefixBuf.getRawBuffer()
            );
        }

        // If the elem stack is empty, then it was an empty root
        if (isRoot)
            gotData = false;
        else
        {
            // Restore the grammar
            fGrammar = fElemStack.getCurrentGrammar();
            fGrammarType = fGrammar->getGrammarType();
            if (fGrammarType == Grammar::SchemaGrammarType && !fValidator->handlesSchema()) {
                if (fValidatorFromUser)
                    ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Gen_NoSchemaValidator, fMemoryManager);
                else {
                    fValidator = fSchemaValidator;
                }
            }
            else if (fGrammarType == Grammar::DTDGrammarType && !fValidator->handlesDTD()) {
                if (fValidatorFromUser)
                    ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Gen_NoDTDValidator, fMemoryManager);
                else {
                    fValidator = fDTDValidator;
                }
            }

            fValidator->setGrammar(fGrammar);

            // Restore the validation flag
            fValidate = fElemStack.getValidationFlag();
        }
    }
    else if (fGrammarType == Grammar::SchemaGrammarType)
    {
        // send a partial element psvi
        if (fPSVIHandler)
        {

            ComplexTypeInfo*   curTypeInfo = 0;
            DatatypeValidator* curDV = 0;
            XSTypeDefinition*  typeDef = 0;

            if (fValidate && elemDecl->isDeclared())
            {
                curTypeInfo = ((SchemaValidator*) fValidator)->getCurrentTypeInfo();

                if (curTypeInfo)
                {
                    typeDef = (XSTypeDefinition*) fModel->getXSObject(curTypeInfo);
                }
                else
                {
                    curDV = ((SchemaValidator*) fValidator)->getCurrentDatatypeValidator();

                    if (curDV)
                    {
                        typeDef = (XSTypeDefinition*) fModel->getXSObject(curDV);
                    }
                }
            }

            fPSVIElement->reset
                (
                  PSVIElement::VALIDITY_NOTKNOWN
                , PSVIElement::VALIDATION_NONE
                , fRootElemName
                , ((SchemaValidator*) fValidator)->getIsElemSpecified()
                , (elemDecl->isDeclared()) ? (XSElementDeclaration*) fModel->getXSObject(elemDecl) : 0
                , typeDef
                , 0 //memberType
                , fModel
                , ((SchemaElementDecl*)elemDecl)->getDefaultValue()
                , 0
                , 0
                , 0
                );


            fPSVIHandler->handlePartialElementPSVI
                (
                  elemDecl->getBaseName()
                , fURIStringPool->getValueForId(elemDecl->getURI())
                , fPSVIElement
                );

        }

        // not empty
        fErrorStack->push(fPSVIElemContext.fErrorOccurred);
    }

    return true;
}


// ---------------------------------------------------------------------------
//  IGXMLScanner: Helper methos
// ---------------------------------------------------------------------------
void IGXMLScanner::resizeElemState() {

    unsigned int newSize = fElemStateSize * 2;
    unsigned int* newElemState = (unsigned int*) fMemoryManager->allocate
    (
        newSize * sizeof(unsigned int)
    ); //new unsigned int[newSize];
    unsigned int* newElemLoopState = (unsigned int*) fMemoryManager->allocate
    (
        newSize * sizeof(unsigned int)
    ); //new unsigned int[newSize];

    // Copy the existing values
    unsigned int index = 0;
    for (; index < fElemStateSize; index++)
    {
        newElemState[index] = fElemState[index];
        newElemLoopState[index] = fElemLoopState[index];
    }

    for (; index < newSize; index++)
        newElemLoopState[index] = newElemState[index] = 0;

    // Delete the old array and udpate our members
    fMemoryManager->deallocate(fElemState); //delete [] fElemState;
    fMemoryManager->deallocate(fElemLoopState); //delete [] fElemState;
    fElemState = newElemState;
    fElemLoopState = newElemLoopState;
    fElemStateSize = newSize;
}

void IGXMLScanner::resizeRawAttrColonList() {

    unsigned int newSize = fRawAttrColonListSize * 2;
    int* newRawAttrColonList = (int*) fMemoryManager->allocate
    (
        newSize * sizeof(int)
    ); //new int[newSize];

    // Copy the existing values
    unsigned int index = 0;
    for (; index < fRawAttrColonListSize; index++)
        newRawAttrColonList[index] = fRawAttrColonList[index];

    // Delete the old array and udpate our members
    fMemoryManager->deallocate(fRawAttrColonList); //delete [] fRawAttrColonList;
    fRawAttrColonList = newRawAttrColonList;
    fRawAttrColonListSize = newSize;
}

// ---------------------------------------------------------------------------
//  IGXMLScanner: Grammar preparsing
// ---------------------------------------------------------------------------
Grammar* IGXMLScanner::loadGrammar(const   InputSource& src
                                   , const short        grammarType
                                   , const bool         toCache)
{
    Grammar* loadedGrammar = 0;

    ReaderMgrResetType  resetReaderMgr(&fReaderMgr, &ReaderMgr::reset);

    try
    {
        fGrammarResolver->cacheGrammarFromParse(false);
		// if the new grammar has to be cached, better use the already cached
		// grammars, or the an exception will be thrown when caching an already
		// cached grammar
        fGrammarResolver->useCachedGrammarInParse(toCache);
        fRootGrammar = 0;

        if (fValScheme == Val_Auto) {
            fValidate = true;
        }

        // Reset some status flags
        fInException = false;
        fStandalone = false;
        fErrorCount = 0;
        fHasNoDTD = true;
        fSeeXsi = false;

        if (grammarType == Grammar::SchemaGrammarType) {
            loadedGrammar = loadXMLSchemaGrammar(src, toCache);
        }
        else if (grammarType == Grammar::DTDGrammarType) {
            loadedGrammar = loadDTDGrammar(src, toCache);
        }
    }
    //  NOTE:
    //
    //  In all of the error processing below, the emitError() call MUST come
    //  before the flush of the reader mgr, or it will fail because it tries
    //  to find out the position in the XML source of the error.
    catch(const XMLErrs::Codes)
    {
        // This is a 'first fatal error' type exit, so fall through
    }
    catch(const XMLValid::Codes)
    {
        // This is a 'first fatal error' type exit, so fall through
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
    }
    catch(const OutOfMemoryException&)
    {
        // This is a special case for out-of-memory
        // conditions, because resetting the ReaderMgr
        // can be problematic.
        resetReaderMgr.release();

        throw;
    }

    return loadedGrammar;
}

void IGXMLScanner::resetCachedGrammar ()
{
  fCachedSchemaInfoList->removeAll ();
}

Grammar* IGXMLScanner::loadDTDGrammar(const InputSource& src,
                                      const bool toCache)
{
    // Reset the validators
    fDTDValidator->reset();
    if (fValidatorFromUser)
        fValidator->reset();

    if (!fValidator->handlesDTD()) {
        if (fValidatorFromUser && fValidate)
            ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Gen_NoDTDValidator, fMemoryManager);
        else {
            fValidator = fDTDValidator;
        }
    }

    fDTDGrammar = (DTDGrammar*) fGrammarResolver->getGrammar(XMLUni::fgDTDEntityString);

    if (fDTDGrammar) {
        fDTDGrammar->reset();
    }
    else {
        fDTDGrammar = new (fGrammarPoolMemoryManager) DTDGrammar(fGrammarPoolMemoryManager);
        fGrammarResolver->putGrammar(fDTDGrammar);
    }

    fGrammar = fDTDGrammar;
    fGrammarType = fGrammar->getGrammarType();
    fValidator->setGrammar(fGrammar);

    //  And for all installed handlers, send reset events. This gives them
    //  a chance to flush any cached data.
    if (fDocHandler)
        fDocHandler->resetDocument();
    if (fEntityHandler)
        fEntityHandler->resetEntities();
    if (fErrorReporter)
        fErrorReporter->resetErrors();

    // Clear out the id reference list
    resetValidationContext();
    // and clear out the darned undeclared DTD element pool...
    fDTDElemNonDeclPool->removeAll();

    if (toCache) {

        unsigned int sysId = fGrammarResolver->getStringPool()->addOrFind(src.getSystemId());
        const XMLCh* sysIdStr = fGrammarResolver->getStringPool()->getValueForId(sysId);

        fGrammarResolver->orphanGrammar(XMLUni::fgDTDEntityString);
        ((XMLDTDDescription*) (fGrammar->getGrammarDescription()))->setSystemId(sysIdStr);
        fGrammarResolver->putGrammar(fGrammar);
    }

    //  Handle the creation of the XML reader object for this input source.
    //  This will provide us with transcoding and basic lexing services.
    XMLReader* newReader = fReaderMgr.createReader
    (
        src
        , false
        , XMLReader::RefFrom_NonLiteral
        , XMLReader::Type_General
        , XMLReader::Source_External
        , fCalculateSrcOfs
        , fLowWaterMark
    );
    if (!newReader) {
        if (src.getIssueFatalErrorIfNotFound())
            ThrowXMLwithMemMgr1(RuntimeException, XMLExcepts::Scan_CouldNotOpenSource, src.getSystemId(), fMemoryManager);
        else
            ThrowXMLwithMemMgr1(RuntimeException, XMLExcepts::Scan_CouldNotOpenSource_Warning, src.getSystemId(), fMemoryManager);
    }

    //  In order to make the processing work consistently, we have to
    //  make this look like an external entity. So create an entity
    //  decl and fill it in and push it with the reader, as happens
    //  with an external entity. Put a janitor on it to insure it gets
    //  cleaned up. The reader manager does not adopt them.
    const XMLCh gDTDStr[] = { chLatin_D, chLatin_T, chLatin_D , chNull };
    DTDEntityDecl* declDTD = new (fMemoryManager) DTDEntityDecl(gDTDStr, false, fMemoryManager);
    declDTD->setSystemId(src.getSystemId());
    declDTD->setIsExternal(true);
    Janitor<DTDEntityDecl> janDecl(declDTD);

    // Mark this one as a throw at end
    newReader->setThrowAtEnd(true);

    // And push it onto the stack, with its pseudo name
    fReaderMgr.pushReader(newReader, declDTD);

    //  If we have a doc type handler and advanced callbacks are enabled,
    //  call the doctype event.
    if (fDocTypeHandler) {

        // Create a dummy root
        DTDElementDecl* rootDecl = new (fGrammarPoolMemoryManager) DTDElementDecl
        (
            gDTDStr
            , fEmptyNamespaceId
            , DTDElementDecl::Any
            , fGrammarPoolMemoryManager
        );
        rootDecl->setCreateReason(DTDElementDecl::AsRootElem);
        rootDecl->setExternalElemDeclaration(true);
        Janitor<DTDElementDecl> janSrc(rootDecl);

        fDocTypeHandler->doctypeDecl(*rootDecl, src.getPublicId(), src.getSystemId(), false, true);
    }

    // Create DTDScanner
    DTDScanner dtdScanner
    (
        (DTDGrammar*) fGrammar
        , fDocTypeHandler
        , fGrammarPoolMemoryManager
        , fMemoryManager
    );
    dtdScanner.setScannerInfo(this, &fReaderMgr, &fBufMgr);

    // Tell it its not in an include section
    dtdScanner.scanExtSubsetDecl(false, true);

    if (fValidate) {
        //  validate the DTD scan so far
        fValidator->preContentValidation(false, true);
    }

    if (toCache)
        fGrammarResolver->cacheGrammars();

    return fDTDGrammar;
}

// ---------------------------------------------------------------------------
//  IGXMLScanner: Helper methods
// ---------------------------------------------------------------------------
void IGXMLScanner::processSchemaLocation(XMLCh* const schemaLoc)
{
    XMLCh* locStr = schemaLoc;
    XMLReader* curReader = fReaderMgr.getCurrentReader();

    fLocationPairs->removeAllElements();
    while (*locStr)
    {
        do {
            // Do we have an escaped character ?
            if (*locStr == 0xFFFF)
                continue;

            if (!curReader->isWhitespace(*locStr))
               break;

            *locStr = chNull;
        } while (*++locStr);

        if (*locStr) {

            fLocationPairs->addElement(locStr);

            while (*++locStr) {
                // Do we have an escaped character ?
                if (*locStr == 0xFFFF)
                    continue;
                if (curReader->isWhitespace(*locStr))
                    break;
            }
        }
    }
}

void IGXMLScanner::endElementPSVI(SchemaElementDecl* const elemDecl,
                                  DatatypeValidator* const memberDV)
{
    PSVIElement::ASSESSMENT_TYPE validationAttempted;
    PSVIElement::VALIDITY_STATE validity = PSVIElement::VALIDITY_NOTKNOWN;

    if (fPSVIElemContext.fElemDepth > fPSVIElemContext.fFullValidationDepth)
        validationAttempted = PSVIElement::VALIDATION_FULL;
    else if (fPSVIElemContext.fElemDepth > fPSVIElemContext.fNoneValidationDepth)
        validationAttempted = PSVIElement::VALIDATION_NONE;
    else
    {
        validationAttempted  = PSVIElement::VALIDATION_PARTIAL;
		fPSVIElemContext.fFullValidationDepth =
            fPSVIElemContext.fNoneValidationDepth = fPSVIElemContext.fElemDepth - 1;
    }

    if (fValidate && elemDecl->isDeclared())
    {
        validity = (fPSVIElemContext.fErrorOccurred)
            ? PSVIElement::VALIDITY_INVALID : PSVIElement::VALIDITY_VALID;
    }

    XSTypeDefinition* typeDef = 0;
    bool isMixed = false;
    if (fPSVIElemContext.fCurrentTypeInfo)
    {
        typeDef = (XSTypeDefinition*) fModel->getXSObject(fPSVIElemContext.fCurrentTypeInfo);
        SchemaElementDecl::ModelTypes modelType = (SchemaElementDecl::ModelTypes)fPSVIElemContext.fCurrentTypeInfo->getContentType();
        isMixed = (modelType == SchemaElementDecl::Mixed_Simple
                || modelType == SchemaElementDecl::Mixed_Complex);
    }
    else if (fPSVIElemContext.fCurrentDV)
        typeDef = (XSTypeDefinition*) fModel->getXSObject(fPSVIElemContext.fCurrentDV);

    XMLCh* canonicalValue = 0;
    if (fPSVIElemContext.fNormalizedValue && !isMixed &&
            validity == PSVIElement::VALIDITY_VALID)
    {
        if (memberDV)
            canonicalValue = (XMLCh*) memberDV->getCanonicalRepresentation(fPSVIElemContext.fNormalizedValue, fMemoryManager);
        else if (fPSVIElemContext.fCurrentDV)
            canonicalValue = (XMLCh*) fPSVIElemContext.fCurrentDV->getCanonicalRepresentation(fPSVIElemContext.fNormalizedValue, fMemoryManager);
    }

    fPSVIElement->reset
    (
        validity
        , validationAttempted
        , fRootElemName
        , fPSVIElemContext.fIsSpecified
        , (elemDecl->isDeclared())
            ? (XSElementDeclaration*) fModel->getXSObject(elemDecl) : 0
        , typeDef
        , (memberDV) ? (XSSimpleTypeDefinition*) fModel->getXSObject(memberDV) : 0
        , fModel
        , elemDecl->getDefaultValue()
        , fPSVIElemContext.fNormalizedValue
        , canonicalValue
    );

    fPSVIHandler->handleElementPSVI
    (
        elemDecl->getBaseName()
        , fURIStringPool->getValueForId(elemDecl->getURI())
        , fPSVIElement
    );

    // decrease element depth
    fPSVIElemContext.fElemDepth--;

}

void IGXMLScanner::resetPSVIElemContext()
{
    fPSVIElemContext.fIsSpecified = false;
    fPSVIElemContext.fErrorOccurred = false;
    fPSVIElemContext.fElemDepth = -1;
    fPSVIElemContext.fFullValidationDepth = -1;
    fPSVIElemContext.fNoneValidationDepth = -1;
    fPSVIElemContext.fCurrentDV = 0;
    fPSVIElemContext.fCurrentTypeInfo = 0;
    fPSVIElemContext.fNormalizedValue = 0;
}

XERCES_CPP_NAMESPACE_END
