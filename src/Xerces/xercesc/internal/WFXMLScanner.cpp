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
  * $Id: WFXMLScanner.cpp 833045 2009-11-05 13:21:27Z borisk $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/internal/WFXMLScanner.hpp>
#include <xercesc/util/Janitor.hpp>
#include <xercesc/util/RuntimeException.hpp>
#include <xercesc/util/UnexpectedEOFException.hpp>
#include <xercesc/sax/InputSource.hpp>
#include <xercesc/framework/XMLDocumentHandler.hpp>
#include <xercesc/framework/XMLEntityHandler.hpp>
#include <xercesc/framework/XMLPScanToken.hpp>
#include <xercesc/framework/XMLValidityCodes.hpp>
#include <xercesc/internal/EndOfEntityException.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  WFXMLScanner: Constructors and Destructor
// ---------------------------------------------------------------------------


typedef JanitorMemFunCall<WFXMLScanner> CleanupType;
typedef JanitorMemFunCall<ReaderMgr>    ReaderMgrResetType;


WFXMLScanner::WFXMLScanner( XMLValidator* const  valToAdopt
                          , GrammarResolver* const grammarResolver
                          , MemoryManager* const manager) :

    XMLScanner(valToAdopt, grammarResolver, manager)
    , fElementIndex(0)
    , fElements(0)
    , fEntityTable(0)
    , fAttrNameHashList(0)
    , fAttrNSList(0)
    , fElementLookup(0)
{
    CleanupType cleanup(this, &WFXMLScanner::cleanUp);

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

WFXMLScanner::WFXMLScanner( XMLDocumentHandler* const docHandler
                          , DocTypeHandler* const     docTypeHandler
                          , XMLEntityHandler* const   entityHandler
                          , XMLErrorReporter* const   errHandler
                          , XMLValidator* const       valToAdopt
                          , GrammarResolver* const    grammarResolver
                          , MemoryManager* const      manager) :

    XMLScanner(docHandler, docTypeHandler, entityHandler, errHandler, valToAdopt, grammarResolver, manager)
    , fElementIndex(0)
    , fElements(0)
    , fEntityTable(0)
    , fAttrNameHashList(0)
    , fAttrNSList(0)
    , fElementLookup(0)
{
    CleanupType cleanup(this, &WFXMLScanner::cleanUp);

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

WFXMLScanner::~WFXMLScanner()
{
    cleanUp();
}

// ---------------------------------------------------------------------------
//  XMLScanner: Getter methods
// ---------------------------------------------------------------------------
NameIdPool<DTDEntityDecl>* WFXMLScanner::getEntityDeclPool()
{
    return 0;
}

const NameIdPool<DTDEntityDecl>* WFXMLScanner::getEntityDeclPool() const
{
    return 0;
}

// ---------------------------------------------------------------------------
//  WFXMLScanner: Main entry point to scan a document
// ---------------------------------------------------------------------------
void WFXMLScanner::scanDocument(const InputSource& src)
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
                // That went ok, so scan for any miscellaneous stuff
                if (!fReaderMgr.atEOF())
                    scanMiscellaneous();
            }
        }

        // If we have a document handler, then call the end document
        if (fDocHandler)
            fDocHandler->endDocument();
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


bool WFXMLScanner::scanNext(XMLPScanToken& token)
{
    // Make sure this token is still legal
    if (!isLegalToken(token))
        ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Scan_BadPScanToken, fMemoryManager);

    // Find the next token and remember the reader id
    XMLSize_t orgReader;
    XMLTokens curToken;
    bool retVal = true;

    ReaderMgrResetType  resetReaderMgr(&fReaderMgr, &ReaderMgr::reset);

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
                // That went ok, so scan for any miscellaneous stuff
                scanMiscellaneous();

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
        // This is a 'first failure' exception, so return failure
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

        // Return failure
        retVal = false;
    }
    catch(const OutOfMemoryException&)
    {
        throw;
    }

    // If we are not at the end, release the object that will
    // reset the ReaderMgr.
    if (retVal)
        resetReaderMgr.release();

    return retVal;
}



// ---------------------------------------------------------------------------
//  WFXMLScanner: Private helper methods.
// ---------------------------------------------------------------------------

//  This method handles the common initialization, to avoid having to do
//  it redundantly in multiple constructors.
void WFXMLScanner::commonInit()
{
    fEntityTable = new (fMemoryManager) ValueHashTableOf<XMLCh>(11, fMemoryManager);
    fAttrNameHashList = new (fMemoryManager)ValueVectorOf<XMLSize_t>(16, fMemoryManager);
    fAttrNSList = new (fMemoryManager) ValueVectorOf<XMLAttr*>(8, fMemoryManager);
    fElements = new (fMemoryManager) RefVectorOf<XMLElementDecl>(32, true, fMemoryManager);
    fElementLookup = new (fMemoryManager) RefHashTableOf<XMLElementDecl>(109, false, fMemoryManager);

    //  Add the default entity entries for the character refs that must always
    //  be present.
    fEntityTable->put((void*) XMLUni::fgAmp, chAmpersand);
    fEntityTable->put((void*) XMLUni::fgLT, chOpenAngle);
    fEntityTable->put((void*) XMLUni::fgGT, chCloseAngle);
    fEntityTable->put((void*) XMLUni::fgQuot, chDoubleQuote);
    fEntityTable->put((void*) XMLUni::fgApos, chSingleQuote);
}

void WFXMLScanner::cleanUp()
{
    delete fEntityTable;
    delete fAttrNameHashList;
    delete fAttrNSList;
    delete fElementLookup;
    delete fElements;
}

//  This method will reset the scanner data structures, and related plugged
//  in stuff, for a new scan session. We get the input source for the primary
//  XML entity, create the reader for it, and push it on the stack so that
//  upon successful return from here we are ready to go.
void WFXMLScanner::scanReset(const InputSource& src)
{
    //  For all installed handlers, send reset events. This gives them
    //  a chance to flush any cached data.
    if (fDocHandler)
        fDocHandler->resetDocument();
    if (fEntityHandler)
        fEntityHandler->resetEntities();
    if (fErrorReporter)
        fErrorReporter->resetErrors();

    //  Reset the element stack, and give it the latest ids for the special
    //  URIs it has to know about.
    fElemStack.reset
    (
        fEmptyNamespaceId
        , fUnknownNamespaceId
        , fXMLNamespaceId
        , fXMLNSNamespaceId
    );

    // Reset some status flags
    fInException = false;
    fStandalone = false;
    fErrorCount = 0;
    fHasNoDTD = true;
    fElementIndex = 0;

    // Reset elements lookup table
    fElementLookup->removeAll();

    //  Handle the creation of the XML reader object for this input source.
    //  This will provide us with transcoding and basic lexing services.
    XMLReader* newReader = fReaderMgr.createReader
    (
        src
        , true
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

    // Push this read onto the reader manager
    fReaderMgr.pushReader(newReader, 0);

    // and reset security-related things if necessary:
    if(fSecurityManager != 0) 
    {
        fEntityExpansionLimit = fSecurityManager->getEntityExpansionLimit();
        fEntityExpansionCount = 0;
    }
}

//  This method is called between markup in content. It scans for character
//  data that is sent to the document handler. It watches for any markup
//  characters that would indicate that the character data has ended. It also
//  handles expansion of general and character entities.
//
//  sendData() is a local static helper for this method which handles some
//  code that must be done in three different places here.
void WFXMLScanner::sendCharData(XMLBuffer& toSend)
{
    // If no data in the buffer, then nothing to do
    if (toSend.isEmpty())
        return;

    // Always assume its just char data if not validating
    if (fDocHandler)
        fDocHandler->docCharacters(toSend.getRawBuffer(), toSend.getLen(), false);

    // Reset buffer
    toSend.reset();
}

// ---------------------------------------------------------------------------
//  WFXMLScanner: Private scanning methods
// ---------------------------------------------------------------------------

//  This method will kick off the scanning of the primary content of the
//  document, i.e. the elements.
bool WFXMLScanner::scanContent()
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


void WFXMLScanner::scanEndTag(bool& gotData)
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
    const ElemStack::StackElem* topElem = fElemStack.popTop();

    // See if it was the root element, to avoid multiple calls below
    const bool isRoot = fElemStack.isEmpty();

    // Make sure that its the end of the element that we expect
    if (!fReaderMgr.skippedStringLong(topElem->fThisElement->getFullName()))
    {
        emitError
        (
            XMLErrs::ExpectedEndOfTagX
            , topElem->fThisElement->getFullName()
        );
        fReaderMgr.skipPastChar(chCloseAngle);
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

    // If we have a doc handler, tell it about the end tag
    if (fDocHandler)
    {
        fDocHandler->endElement
        (
            *topElem->fThisElement
            , uriId
            , isRoot
            , topElem->fThisElement->getElementName()->getPrefix()
        );
    }

    // If this was the root, then done with content
    gotData = !isRoot;
}

void WFXMLScanner::scanDocTypeDecl()
{
    // Just skips over it
    // REVISIT: Should we issue a warning
    static const XMLCh doctypeIE[] =
    {
        chOpenSquare, chCloseAngle, chNull
    };
    XMLCh nextCh = fReaderMgr.skipUntilIn(doctypeIE);

    if (nextCh == chOpenSquare)
        fReaderMgr.skipPastChar(chCloseSquare);

    fReaderMgr.skipPastChar(chCloseAngle);
}

bool WFXMLScanner::scanStartTag(bool& gotData)
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

    // See if its the root element
    const bool isRoot = fElemStack.isEmpty();

    //  Lets try to look up the element
    const XMLCh* qnameRawBuf = fQNameBuf.getRawBuffer();
    XMLElementDecl* elemDecl = fElementLookup->get(qnameRawBuf);

    if (!elemDecl) {

        if (fElementIndex < fElements->size()) {
            elemDecl = fElements->elementAt(fElementIndex);
        }
        else {
            elemDecl = new (fGrammarPoolMemoryManager) DTDElementDecl
            (
                fGrammarPoolMemoryManager
            );
            fElements->addElement(elemDecl);
        }

        elemDecl->setElementName(XMLUni::fgZeroLenString, qnameRawBuf, fEmptyNamespaceId);
        fElementLookup->put((void*)elemDecl->getFullName(), elemDecl);
        fElementIndex++;
    }

    // Expand the element stack and add the new element
    fElemStack.addLevel(elemDecl, fReaderMgr.getCurrentReaderNum());

    // Skip any whitespace after the name
    fReaderMgr.skipPastSpaces();

    //  We loop until we either see a /> or >, handling attribute/value
    //  pairs until we get there.
    XMLSize_t    attCount = 0;
    XMLSize_t    curAttListSize = fAttrList->size();
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
                    emitError(XMLErrs::UnterminatedStartTag, qnameRawBuf);
                    return false;
                }
                else
                {
                    // Something went really wrong
                    return false;
                }
            }

            //  See if this attribute is declared more than one for this element.
            const XMLCh* attNameRawBuf = fAttNameBuf.getRawBuffer();
            XMLSize_t attNameHash = XMLString::hash(attNameRawBuf, 109);

            if (attCount) {

                for (XMLSize_t k=0; k < attCount; k++) {

                    if (fAttrNameHashList->elementAt(k) == attNameHash) {
                        if (
                               XMLString::equals
                               (
                                   fAttrList->elementAt(k)->getName()
                                   , attNameRawBuf
                               )
                           )
                        {
                            emitError
                            (
                                XMLErrs::AttrAlreadyUsedInSTag
                                , attNameRawBuf
                                , qnameRawBuf
                            );
                            break;
                        }
                    }
                }
            }

            //  Skip any whitespace before the value and then scan the att
            //  value. This will come back normalized with entity refs and
            //  char refs expanded.
            fReaderMgr.skipPastSpaces();
            if (!scanAttValue(attNameRawBuf, fAttValueBuf))
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
                    emitError(XMLErrs::UnterminatedStartTag, qnameRawBuf);
                    return false;
                }
                else
                {
                    // Something went really wrong
                    return false;
                }
            }

            //  Add this attribute to the attribute list that we use to
            //  pass them to the handler. We reuse its existing elements
            //  but expand it as required.
            XMLAttr* curAtt;
            if (attCount >= curAttListSize)
            {
                curAtt = new (fMemoryManager) XMLAttr
                (
                    0
                    , attNameRawBuf
                    , XMLUni::fgZeroLenString
                    , fAttValueBuf.getRawBuffer()
                    , XMLAttDef::CData
                    , true
                    , fMemoryManager
                );
                fAttrList->addElement(curAtt);
                fAttrNameHashList->addElement(attNameHash);
            }
            else
            {
                curAtt = fAttrList->elementAt(attCount);
                curAtt->set
                (
                    0
                    , attNameRawBuf
                    , XMLUni::fgZeroLenString
                    , fAttValueBuf.getRawBuffer()
                );
                curAtt->setSpecified(true);
                fAttrNameHashList->setElementAt(attNameHash, attCount);
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
                emitError(XMLErrs::UnterminatedStartTag, qnameRawBuf);
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

    //  If empty, validate content right now if we are validating and then
    //  pop the element stack top. Else, we have to update the current stack
    //  top's namespace mapping elements.
    if (isEmpty)
    {
        // Pop the element stack back off since it'll never be used now
        fElemStack.popTop();

        // If the elem stack is empty, then it was an empty root
        if (isRoot)
            gotData = false;
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
//  namespace aware processing an done for non-namespace aware processing.
//
//  This method is called after we've scanned the < of a start tag. So we
//  have to get the element name, then scan the attributes, after which
//  we are either going to see >, />, or attributes followed by one of those
//  sequences.
bool WFXMLScanner::scanStartTagNS(bool& gotData)
{
    //  Assume we will still have data until proven otherwise. It will only
    //  ever be false if this is the root and its empty.
    gotData = true;

    //  The current position is after the open bracket, so we need to read in
    //  in the element name.
    int colonPosition;
    if (!fReaderMgr.getQName(fQNameBuf, &colonPosition))
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

	// Assume it won't be an empty tag
    bool isEmpty = false;

    // Skip any whitespace after the name
    fReaderMgr.skipPastSpaces();

    //  Lets try to look up the element
    const XMLCh* qnameRawBuf = fQNameBuf.getRawBuffer();
    XMLElementDecl* elemDecl = fElementLookup->get(qnameRawBuf);

    if (!elemDecl) {
        if (!XMLString::compareNString(qnameRawBuf, XMLUni::fgXMLNSColonString, 6))
            emitError(XMLErrs::NoXMLNSAsElementPrefix, qnameRawBuf);

        if (fElementIndex < fElements->size()) {
            elemDecl = fElements->elementAt(fElementIndex);
        }
        else {
            elemDecl = new (fGrammarPoolMemoryManager) DTDElementDecl
            (
                fGrammarPoolMemoryManager
            );
            fElements->addElement(elemDecl);
        }

        elemDecl->setElementName(qnameRawBuf, fEmptyNamespaceId);
        fElementLookup->put((void*)elemDecl->getFullName(), elemDecl);
        fElementIndex++;
    }

    // Expand the element stack and add the new element
    fElemStack.addLevel(elemDecl, fReaderMgr.getCurrentReaderNum());

    // reset NS attribute list
    fAttrNSList->removeAllElements();

    // We loop until we either see a /> or >, handling attribute/value
    // pairs until we get there.
    XMLSize_t attCount = 0;
    XMLSize_t curAttListSize = fAttrList->size();
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
            int colonPosition;
            if (!fReaderMgr.getQName(fAttNameBuf, &colonPosition))
            {                
                if (fAttNameBuf.isEmpty())
                    emitError(XMLErrs::ExpectedAttrName);
                else
                    emitError(XMLErrs::InvalidAttrName, fAttNameBuf.getRawBuffer()); 
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
                    emitError(XMLErrs::UnterminatedStartTag, qnameRawBuf);
                    return false;
                }
                else
                {
                    // Something went really wrong
                    return false;
                }
            }

            //  See if this attribute is declared more than one for this element.
            const XMLCh* attNameRawBuf = fAttNameBuf.getRawBuffer();
            XMLSize_t attNameHash = XMLString::hash(attNameRawBuf, 109);
            if (attCount) {

                for (XMLSize_t k=0; k < attCount; k++) {

                    if (fAttrNameHashList->elementAt(k) == attNameHash) {
                        if (XMLString::equals(
                                fAttrList->elementAt(k)->getQName()
                                , attNameRawBuf))
                        {
                            emitError
                            (
                                XMLErrs::AttrAlreadyUsedInSTag
                                , attNameRawBuf
                                , qnameRawBuf
                            );
                            break;
                        }
                    }
                }
            }

            //  Skip any whitespace before the value and then scan the att
            //  value. This will come back normalized with entity refs and
            //  char refs expanded.
            fReaderMgr.skipPastSpaces();
            if (!scanAttValue(attNameRawBuf, fAttValueBuf))
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
                    emitError(XMLErrs::UnterminatedStartTag, qnameRawBuf);
                    return false;
                }
                else
                {
                    // Something went really wrong
                    return false;
                }
            }

            //  Add this attribute to the attribute list that we use to
            //  pass them to the handler. We reuse its existing elements
            //  but expand it as required.
            const XMLCh* attValueRawBuf = fAttValueBuf.getRawBuffer();
            XMLAttr* curAtt = 0;
            if (attCount >= curAttListSize)
            {
                curAtt = new (fMemoryManager) XMLAttr
                (
                    fEmptyNamespaceId
                    , attNameRawBuf
                    , attValueRawBuf
                    , XMLAttDef::CData
                    , true
                    , fMemoryManager
                );
                fAttrList->addElement(curAtt);
                fAttrNameHashList->addElement(attNameHash);
            }
            else
            {
                curAtt = fAttrList->elementAt(attCount);
                curAtt->set
                (
                    fEmptyNamespaceId
                    , attNameRawBuf
                    , attValueRawBuf
                );
                curAtt->setSpecified(true);
                fAttrNameHashList->setElementAt(attNameHash, attCount);
            }

            // Map prefix to namespace
            const XMLCh* attPrefix = curAtt->getPrefix();
            const XMLCh* attLocalName = curAtt->getName();
            const XMLCh* namespaceURI = fAttValueBuf.getRawBuffer();

            if (attPrefix && *attPrefix) {
                if (XMLString::equals(attPrefix, XMLUni::fgXMLString)) {
                    curAtt->setURIId(fXMLNamespaceId);
                }
                else if (XMLString::equals(attPrefix, XMLUni::fgXMLNSString)) {

                    if (XMLString::equals(attLocalName, XMLUni::fgXMLNSString))
                        emitError(XMLErrs::NoUseOfxmlnsAsPrefix);
                    else if (XMLString::equals(attLocalName, XMLUni::fgXMLString)) {
                        if (!XMLString::equals(namespaceURI, XMLUni::fgXMLURIName))
                            emitError(XMLErrs::PrefixXMLNotMatchXMLURI);
                    }

                    if (!namespaceURI)
                        emitError(XMLErrs::NoEmptyStrNamespace, attNameRawBuf);
                    else if(!*namespaceURI && fXMLVersion == XMLReader::XMLV1_0)
                        emitError(XMLErrs::NoEmptyStrNamespace, attNameRawBuf);

                    fElemStack.addPrefix
                    (
                        attLocalName
                        , fURIStringPool->addOrFind(namespaceURI)
                    );
                    curAtt->setURIId(fXMLNSNamespaceId);
                }
                else {
                    fAttrNSList->addElement(curAtt);
                }
            }
            else {
                if (XMLString::equals(XMLUni::fgXMLNSString, attLocalName)) {

                    if (XMLString::equals(namespaceURI, XMLUni::fgXMLNSURIName))
                        emitError(XMLErrs::NoUseOfxmlnsURI);
                    else if (XMLString::equals(namespaceURI, XMLUni::fgXMLURIName))
                        emitError(XMLErrs::XMLURINotMatchXMLPrefix);

                    fElemStack.addPrefix
                    (
                        XMLUni::fgZeroLenString
                        , fURIStringPool->addOrFind(namespaceURI)
                    );
                }
            }

            // increment attribute count
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
                emitError(XMLErrs::UnterminatedStartTag, qnameRawBuf);
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
            emitError(XMLErrs::UnterminatedStartTag, qnameRawBuf);
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

    // Handle provided attributes that we did not map their prefixes
    for (unsigned int i=0; i < fAttrNSList->size(); i++) {

        XMLAttr* providedAttr = fAttrNSList->elementAt(i);

        providedAttr->setURIId
        (
	        resolvePrefix
            (
                providedAttr->getPrefix(),
                ElemStack::Mode_Attribute
            )
        );
    }

    if(attCount) {

        //
        // Decide if to use hash table to do duplicate checking
        //
        bool toUseHashTable = false;
        setAttrDupChkRegistry(attCount, toUseHashTable);

        // check for duplicate namespace attributes:
        // by checking for qualified names with the same local part and with prefixes 
        // which have been bound to namespace names that are identical. 
        XMLAttr* loopAttr;
        XMLAttr* curAtt;
        for (unsigned int attrIndex=0; attrIndex < attCount-1; attrIndex++) {
            loopAttr = fAttrList->elementAt(attrIndex);

            if (!toUseHashTable)
            {
                for (unsigned int curAttrIndex = attrIndex+1; curAttrIndex < attCount; curAttrIndex++) {
                    curAtt = fAttrList->elementAt(curAttrIndex);
                    if (curAtt->getURIId() == loopAttr->getURIId() &&
                        XMLString::equals(curAtt->getName(), loopAttr->getName())) {
                        emitError
                            ( 
                            XMLErrs::AttrAlreadyUsedInSTag
                            , curAtt->getName()
                            , elemDecl->getFullName()
                            );
                    }
                }
            }
            else 
            {
                if (fAttrDupChkRegistry->containsKey((void*)loopAttr->getName(), loopAttr->getURIId()))
                {
                    emitError
                    ( 
                    XMLErrs::AttrAlreadyUsedInSTag
                    , loopAttr->getName()
                    , elemDecl->getFullName()
                    );
                }

                fAttrDupChkRegistry->put((void*)loopAttr->getName(), loopAttr->getURIId(), loopAttr);
            }
        }  
    }

    // Resolve the qualified name to a URI.
    unsigned int uriId = resolvePrefix
    (
        elemDecl->getElementName()->getPrefix()
        , ElemStack::Mode_Element
    );

    // Now we can update the element stack
    fElemStack.setCurrentURI(uriId);

    // Tell the document handler about this start tag
    if (fDocHandler)
    {
        fDocHandler->startElement
        (
            *elemDecl
            , uriId
            , elemDecl->getElementName()->getPrefix()
            , *fAttrList
            , attCount
            , isEmpty
            , isRoot
        );
    }

    //  If empty, validate content right now if we are validating and then
    //  pop the element stack top. Else, we have to update the current stack
    //  top's namespace mapping elements.
    if (isEmpty)
    {
        // Pop the element stack back off since it'll never be used now
        fElemStack.popTop();

        // If the elem stack is empty, then it was an empty root
        if (isRoot)
            gotData = false;
    }

    return true;
}

// ---------------------------------------------------------------------------
//  XMLScanner: Private parsing methods
// ---------------------------------------------------------------------------
bool WFXMLScanner::scanAttValue(const XMLCh* const attrName
                              ,     XMLBuffer&   toFill)
{
    // Reset the target buffer
    toFill.reset();

    // Get the next char which must be a single or double quote
    XMLCh quoteCh;
    if (!fReaderMgr.skipIfQuote(quoteCh))
        return false;

    //  We have to get the current reader because we have to ignore closing
    //  quotes until we hit the same reader again.
    const XMLSize_t curReader = fReaderMgr.getCurrentReaderNum();

    //  Loop until we get the attribute value. Note that we use a double
    //  loop here to avoid the setup/teardown overhead of the exception
    //  handler on every round.
    XMLCh   nextCh;
    XMLCh   secondCh = 0;
    bool    gotLeadingSurrogate = false;
    bool    escaped;
    while (true)
    {
    try
    {
        while(true)
        {
            nextCh = fReaderMgr.getNextChar();

            if (!nextCh)
                ThrowXMLwithMemMgr(UnexpectedEOFException, XMLExcepts::Gen_UnexpectedEOF, fMemoryManager);

            // Check for our ending quote in the same entity
            if (nextCh == quoteCh)
            {
                if (curReader == fReaderMgr.getCurrentReaderNum())
                    return true;

                // Watch for spillover into a previous entity
                if (curReader > fReaderMgr.getCurrentReaderNum())
                {
                    emitError(XMLErrs::PartialMarkupInEntity);
                    return false;
                }
            }

            //  Check for an entity ref now, before we let it affect our
            //  whitespace normalization logic below. We ignore the empty flag
            //  in this one.
            escaped = false;
            if (nextCh == chAmpersand)
            {
                if (scanEntityRef(true, nextCh, secondCh, escaped) != EntityExp_Returned)
                {
                    gotLeadingSurrogate = false;
                    continue;
                }
            }
            else if ((nextCh >= 0xD800) && (nextCh <= 0xDBFF))
            {
                // Deal with surrogate pairs
                //  Its a leading surrogate. If we already got one, then
                //  issue an error, else set leading flag to make sure that
                //  we look for a trailing next time.
                if (gotLeadingSurrogate)
                {
                    emitError(XMLErrs::Expected2ndSurrogateChar);
                }
                else
                    gotLeadingSurrogate = true;
            }
            else
            {
                //  If its a trailing surrogate, make sure that we are
                //  prepared for that. Else, its just a regular char so make
                //  sure that we were not expected a trailing surrogate.
                if ((nextCh >= 0xDC00) && (nextCh <= 0xDFFF))
                {
                    // Its trailing, so make sure we were expecting it
                    if (!gotLeadingSurrogate)
                        emitError(XMLErrs::Unexpected2ndSurrogateChar);
                }
                else
                {
                    //  Its just a char, so make sure we were not expecting a
                    //  trailing surrogate.
                    if (gotLeadingSurrogate) {
                        emitError(XMLErrs::Expected2ndSurrogateChar);
                    }
                    // Its got to at least be a valid XML character
                    else if (!fReaderMgr.getCurrentReader()->isXMLChar(nextCh))
                    {
                        XMLCh tmpBuf[9];
                        XMLString::binToText
                        (
                            nextCh
                            , tmpBuf
                            , 8
                            , 16
                            , fMemoryManager
                        );
                        emitError(XMLErrs::InvalidCharacterInAttrValue, attrName, tmpBuf);
                    }
                }
                gotLeadingSurrogate = false;
            }

            //  If its not escaped, then make sure its not a < character, which
            //  is not allowed in attribute values.
            if (!escaped) {
                if (nextCh == chOpenAngle)
                    emitError(XMLErrs::BracketInAttrValue, attrName);
                else if (fReaderMgr.getCurrentReader()->isWhitespace(nextCh))
                    nextCh = chSpace;
            }

            // Else add it to the buffer
            toFill.append(nextCh);

            if (secondCh)
            {
                toFill.append(secondCh);
                secondCh=0;
            }
        }
    }
    catch(const EndOfEntityException&)
    {
        // Just eat it and continue.
        gotLeadingSurrogate = false;
        escaped = false;
    }
    }
    return true;
}


//  This method scans a CDATA section. It collects the character into one
//  of the temp buffers and calls the document handler, if any, with the
//  characters. It assumes that the <![CDATA string has been scanned before
//  this call.
void WFXMLScanner::scanCDSection()
{
    static const XMLCh CDataClose[] =
    {
            chCloseSquare, chCloseAngle, chNull
    };

    //  The next character should be the opening square bracket. If not
    //  issue an error, but then try to recover by skipping any whitespace
    //  and checking again.
    if (!fReaderMgr.skippedChar(chOpenSquare))
    {
        emitError(XMLErrs::ExpectedOpenSquareBracket);
        fReaderMgr.skipPastSpaces();

        // If we still don't find it, then give up, else keep going
        if (!fReaderMgr.skippedChar(chOpenSquare))
            return;
    }

    // Get a buffer for this
    XMLBufBid bbCData(&fBufMgr);

    //  We just scan forward until we hit the end of CDATA section sequence.
    //  CDATA is effectively a big escape mechanism so we don't treat markup
    //  characters specially here.
    bool            emittedError = false;
    bool    gotLeadingSurrogate = false;
    while (true)
    {
        const XMLCh nextCh = fReaderMgr.getNextChar();

        // Watch for unexpected end of file
        if (!nextCh)
        {
            emitError(XMLErrs::UnterminatedCDATASection);
            ThrowXMLwithMemMgr(UnexpectedEOFException, XMLExcepts::Gen_UnexpectedEOF, fMemoryManager);
        }

        //  If this is a close square bracket it could be our closing
        //  sequence.
        if (nextCh == chCloseSquare && fReaderMgr.skippedString(CDataClose))
        {
            //  make sure we were not expecting a trailing surrogate.
            if (gotLeadingSurrogate)
                emitError(XMLErrs::Expected2ndSurrogateChar);

            // If we have a doc handler, call it
            if (fDocHandler)
            {
                fDocHandler->docCharacters
                (
                    bbCData.getRawBuffer()
                    , bbCData.getLen()
                    , true
                );
            }

            // And we are done
            break;
        }

        //  Make sure its a valid character. But if we've emitted an error
        //  already, don't bother with the overhead since we've already told
        //  them about it.
        if (!emittedError)
        {
            // Deal with surrogate pairs
            if ((nextCh >= 0xD800) && (nextCh <= 0xDBFF))
            {
                //  Its a leading surrogate. If we already got one, then
                //  issue an error, else set leading flag to make sure that
                //  we look for a trailing next time.
                if (gotLeadingSurrogate)
                    emitError(XMLErrs::Expected2ndSurrogateChar);
                else
                    gotLeadingSurrogate = true;
            }
            else
            {
                //  If its a trailing surrogate, make sure that we are
                //  prepared for that. Else, its just a regular char so make
                //  sure that we were not expected a trailing surrogate.
                if ((nextCh >= 0xDC00) && (nextCh <= 0xDFFF))
                {
                    // Its trailing, so make sure we were expecting it
                    if (!gotLeadingSurrogate)
                        emitError(XMLErrs::Unexpected2ndSurrogateChar);
                }
                else
                {
                    //  Its just a char, so make sure we were not expecting a
                    //  trailing surrogate.
                    if (gotLeadingSurrogate)
                        emitError(XMLErrs::Expected2ndSurrogateChar);

                    // Its got to at least be a valid XML character
                    else if (!fReaderMgr.getCurrentReader()->isXMLChar(nextCh))
                    {
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
                        emittedError = true;
                    }
                }
                gotLeadingSurrogate = false;
            }
        }

        // Add it to the buffer
        bbCData.append(nextCh);
    }
}


void WFXMLScanner::scanCharData(XMLBuffer& toUse)
{
    //  We have to watch for the stupid ]]> sequence, which is illegal in
    //  character data. So this is a little state machine that handles that.
    enum States
    {
        State_Waiting
        , State_GotOne
        , State_GotTwo
    };

    // Reset the buffer before we start
    toUse.reset();

    // Turn on the 'throw at end' flag of the reader manager
    ThrowEOEJanitor jan(&fReaderMgr, true);

    //  In order to be more efficient we have to use kind of a deeply nested
    //  set of blocks here. The outer block puts on a try and catches end of
    //  entity exceptions. The inner loop is the per-character loop. If we
    //  put the try inside the inner loop, it would work but would require
    //  the exception handling code setup/teardown code to be invoked for
    //  each character.
    XMLCh   nextCh;
    XMLCh   secondCh = 0;
    States  curState = State_Waiting;
    bool    escaped = false;
    bool    gotLeadingSurrogate = false;
    bool    notDone = true;
    while (notDone)
    {
        try
        {
            while (true)
            {
                //  Eat through as many plain content characters as possible without
                //  needing special handling.  Moving most content characters here,
                //  in this one call, rather than running the overall loop once
                //  per content character, is a speed optimization.
                if (curState == State_Waiting  &&  !gotLeadingSurrogate)
                {
                     fReaderMgr.movePlainContentChars(toUse);
                }

                // Try to get another char from the source
                //   The code from here on down covers all contengencies,
                if (!fReaderMgr.getNextCharIfNot(chOpenAngle, nextCh))
                {
                    // If we were waiting for a trailing surrogate, its an error
                    if (gotLeadingSurrogate)
                        emitError(XMLErrs::Expected2ndSurrogateChar);

                    notDone = false;
                    break;
                }

                //  Watch for a reference. Note that the escapement mechanism
                //  is ignored in this content.
                escaped = false;
                if (nextCh == chAmpersand)
                {
                    sendCharData(toUse);

                    // Turn off the throwing at the end of entity during this
                    ThrowEOEJanitor jan(&fReaderMgr, false);

                    if (scanEntityRef(false, nextCh, secondCh, escaped) != EntityExp_Returned)
                    {
                        gotLeadingSurrogate = false;
                        continue;
                    }
                }
                else if ((nextCh >= 0xD800) && (nextCh <= 0xDBFF))
                {
                    // Deal with surrogate pairs
                    //  Its a leading surrogate. If we already got one, then
                    //  issue an error, else set leading flag to make sure that
                    //  we look for a trailing next time.
                    if (gotLeadingSurrogate)
                    {
                        emitError(XMLErrs::Expected2ndSurrogateChar);
                    }
                    else
                        gotLeadingSurrogate = true;
                }
                else
                {
                    //  If its a trailing surrogate, make sure that we are
                    //  prepared for that. Else, its just a regular char so make
                    //  sure that we were not expected a trailing surrogate.
                    if ((nextCh >= 0xDC00) && (nextCh <= 0xDFFF))
                    {
                        // Its trailing, so make sure we were expecting it
                        if (!gotLeadingSurrogate)
                            emitError(XMLErrs::Unexpected2ndSurrogateChar);
                    }
                    else
                    {
                        //  Its just a char, so make sure we were not expecting a
                        //  trailing surrogate.
                        if (gotLeadingSurrogate) {
                            emitError(XMLErrs::Expected2ndSurrogateChar);
                        }
                        // Its got to at least be a valid XML character
                        else if (!fReaderMgr.getCurrentReader()->isXMLChar(nextCh))
                        {
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
                    }
                    gotLeadingSurrogate = false;
                }

                // Keep the state machine up to date
                if (!escaped)
                {
                    if (nextCh == chCloseSquare)
                    {
                        if (curState == State_Waiting)
                            curState = State_GotOne;
                        else if (curState == State_GotOne)
                            curState = State_GotTwo;
                    }
                    else if (nextCh == chCloseAngle)
                    {
                        if (curState == State_GotTwo)
                            emitError(XMLErrs::BadSequenceInCharData);
                        curState = State_Waiting;
                    }
                    else
                    {
                        curState = State_Waiting;
                    }
                }
                else
                {
                    curState = State_Waiting;
                }

                // Add this char to the buffer
                toUse.append(nextCh);

                if (secondCh)
                {
                    toUse.append(secondCh);
                    secondCh=0;
                }
            }
        }
        catch(const EndOfEntityException& toCatch)
        {
            //  Some entity ended, so we have to send any accumulated
            //  chars and send an end of entity event.
            sendCharData(toUse);
            gotLeadingSurrogate = false;

            if (fDocHandler)
                fDocHandler->endEntityReference(toCatch.getEntity());
        }
    }

    // Send any char data that we accumulated into the buffer
    sendCharData(toUse);
}

InputSource* WFXMLScanner::resolveSystemId(const XMLCh* const /*sysId*/
                                          ,const XMLCh* const /*pubId*/)
{
    return 0;
}

//  This method will scan a general/character entity ref. It will either
//  expand a char ref and return it directly, or push a reader for a general
//  entity.
//
//  The return value indicates whether the char parameters hold the value
//  or whether the value was pushed as a reader, or that it failed.
//
//  The escaped flag tells the caller whether the returned parameter resulted
//  from a character reference, which escapes the character in some cases. It
//  only makes any difference if the return value indicates the value was
//  returned directly.
XMLScanner::EntityExpRes
WFXMLScanner::scanEntityRef(const bool
                            ,     XMLCh&  firstCh
                            ,     XMLCh&  secondCh
                            ,     bool&   escaped)
{
    // Assume no escape
    secondCh = 0;
    escaped = false;

    // We have to insure that its all in one entity
    const XMLSize_t curReader = fReaderMgr.getCurrentReaderNum();

    //  If the next char is a pound, then its a character reference and we
    //  need to expand it always.
    if (fReaderMgr.skippedChar(chPound))
    {
        //  Its a character reference, so scan it and get back the numeric
        //  value it represents.
        if (!scanCharRef(firstCh, secondCh))
            return EntityExp_Failed;

        escaped = true;

        if (curReader != fReaderMgr.getCurrentReaderNum())
            emitError(XMLErrs::PartialMarkupInEntity);

        return EntityExp_Returned;
    }

    // Expand it since its a normal entity ref
    XMLBufBid bbName(&fBufMgr);
    if (!fReaderMgr.getName(bbName.getBuffer()))
    {
        emitError(XMLErrs::ExpectedEntityRefName);
        return EntityExp_Failed;
    }

    //  Next char must be a semi-colon. But if its not, just emit
    //  an error and try to continue.
    if (!fReaderMgr.skippedChar(chSemiColon))
        emitError(XMLErrs::UnterminatedEntityRef, bbName.getRawBuffer());

    // Make sure we ended up on the same entity reader as the & char
    if (curReader != fReaderMgr.getCurrentReaderNum())
        emitError(XMLErrs::PartialMarkupInEntity);

    // Look up the name in the general entity pool
    // If it does not exist, then obviously an error
    if (!fEntityTable->containsKey(bbName.getRawBuffer()))
    {
        // XML 1.0 Section 4.1
        // Well-formedness Constraint for entity not found:
        //   In a document without any DTD, a document with only an internal DTD subset which contains no parameter entity references,
        //      or a document with "standalone='yes'", for an entity reference that does not occur within the external subset
        //      or a parameter entity
        if (fStandalone || fHasNoDTD)
            emitError(XMLErrs::EntityNotFound, bbName.getRawBuffer());

        return EntityExp_Failed;
    }

    // here's where we need to check if there's a SecurityManager,
    // how many entity references we've had
    if(fSecurityManager != 0 && ++fEntityExpansionCount > fEntityExpansionLimit) {
        XMLCh expLimStr[32];
        XMLString::sizeToText(fEntityExpansionLimit, expLimStr, 31, 10, fMemoryManager);
        emitError
        ( 
            XMLErrs::EntityExpansionLimitExceeded
            , expLimStr
        );
        // there seems nothing better to be done than to reset the entity expansion counter
        fEntityExpansionCount = 0;
    }

    firstCh = fEntityTable->get(bbName.getRawBuffer());
    escaped = true;
    return EntityExp_Returned;
}

// ---------------------------------------------------------------------------
//  WFXMLScanner: Grammar preparsing
// ---------------------------------------------------------------------------
Grammar* WFXMLScanner::loadGrammar(const   InputSource&
                                   , const short
                                   , const bool)
{
    // REVISIT: emit a warning or throw an exception
    return 0;
}


XERCES_CPP_NAMESPACE_END
