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
 * $Id: DGXMLScanner.cpp 833045 2009-11-05 13:21:27Z borisk $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/internal/DGXMLScanner.hpp>
#include <xercesc/util/Janitor.hpp>
#include <xercesc/util/RuntimeException.hpp>
#include <xercesc/util/UnexpectedEOFException.hpp>
#include <xercesc/util/XMLUri.hpp>
#include <xercesc/framework/URLInputSource.hpp>
#include <xercesc/framework/LocalFileInputSource.hpp>
#include <xercesc/framework/XMLDocumentHandler.hpp>
#include <xercesc/framework/XMLEntityHandler.hpp>
#include <xercesc/framework/XMLPScanToken.hpp>
#include <xercesc/framework/XMLGrammarPool.hpp>
#include <xercesc/framework/XMLDTDDescription.hpp>
#include <xercesc/internal/EndOfEntityException.hpp>
#include <xercesc/validators/common/GrammarResolver.hpp>
#include <xercesc/validators/DTD/DocTypeHandler.hpp>
#include <xercesc/validators/DTD/DTDScanner.hpp>
#include <xercesc/validators/DTD/DTDValidator.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/util/XMLResourceIdentifier.hpp>

XERCES_CPP_NAMESPACE_BEGIN


typedef JanitorMemFunCall<DGXMLScanner> CleanupType;
typedef JanitorMemFunCall<ReaderMgr>    ReaderMgrResetType;


// ---------------------------------------------------------------------------
//  DGXMLScanner: Constructors and Destructor
// ---------------------------------------------------------------------------
DGXMLScanner::DGXMLScanner(XMLValidator* const valToAdopt
                         , GrammarResolver* const grammarResolver
                         , MemoryManager* const manager) :

    XMLScanner(valToAdopt, grammarResolver, manager)
    , fAttrNSList(0)
    , fDTDValidator(0)
    , fDTDGrammar(0)
    , fDTDElemNonDeclPool(0)
    , fElemCount(0)
    , fAttDefRegistry(0)
    , fUndeclaredAttrRegistry(0)
{
    CleanupType cleanup(this, &DGXMLScanner::cleanUp);

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

DGXMLScanner::DGXMLScanner( XMLDocumentHandler* const docHandler
                          , DocTypeHandler* const     docTypeHandler
                          , XMLEntityHandler* const   entityHandler
                          , XMLErrorReporter* const   errHandler
                          , XMLValidator* const       valToAdopt
                          , GrammarResolver* const    grammarResolver
                          , MemoryManager* const      manager) :

    XMLScanner(docHandler, docTypeHandler, entityHandler, errHandler, valToAdopt, grammarResolver, manager)
    , fAttrNSList(0)
    , fDTDValidator(0)
    , fDTDGrammar(0)
    , fDTDElemNonDeclPool(0)
    , fElemCount(0)
    , fAttDefRegistry(0)
    , fUndeclaredAttrRegistry(0)
{
    CleanupType cleanup(this, &DGXMLScanner::cleanUp);

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

DGXMLScanner::~DGXMLScanner()
{
    cleanUp();
}

// ---------------------------------------------------------------------------
//  XMLScanner: Getter methods
// ---------------------------------------------------------------------------
NameIdPool<DTDEntityDecl>* DGXMLScanner::getEntityDeclPool()
{
    if(!fGrammar)
        return 0;
    return ((DTDGrammar*)fGrammar)->getEntityDeclPool();
}

const NameIdPool<DTDEntityDecl>* DGXMLScanner::getEntityDeclPool() const
{
    if(!fGrammar)
        return 0;
    return ((DTDGrammar*)fGrammar)->getEntityDeclPool();
}

// ---------------------------------------------------------------------------
//  DGXMLScanner: Main entry point to scan a document
// ---------------------------------------------------------------------------
void DGXMLScanner::scanDocument(const InputSource& src)
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


bool DGXMLScanner::scanNext(XMLPScanToken& token)
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
//  DGXMLScanner: Private scanning methods
// ---------------------------------------------------------------------------

//  This method will kick off the scanning of the primary content of the
//  document, i.e. the elements.
bool DGXMLScanner::scanContent()
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


void DGXMLScanner::scanEndTag(bool& gotData)
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

    //  Pop the stack of the element we are supposed to be ending. Remember
    //  that we don't own this. The stack just keeps them and reuses them.
    const ElemStack::StackElem* topElem = fElemStack.popTop();
    XMLElementDecl *tempElement = topElem->fThisElement;

    // See if it was the root element, to avoid multiple calls below
    const bool isRoot = fElemStack.isEmpty();

    // Make sure that its the end of the element that we expect
    if (!fReaderMgr.skippedStringLong(tempElement->getFullName()))
    {
        emitError
        (
            XMLErrs::ExpectedEndOfTagX
            , tempElement->getFullName()
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

    //  If validation is enabled, then lets pass him the list of children and
    //  this element and let him validate it.
    if (fValidate)
    {

       //
       // XML1.0-3rd
       // Validity Constraint:
       // The declaration matches EMPTY and the element has no content (not even
       // entity references, comments, PIs or white space).
       //
       if ( (topElem->fCommentOrPISeen)               &&
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
       if ( (topElem->fReferenceEscaped)               &&
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
    }

    // If we have a doc handler, tell it about the end tag
    if (fDocHandler)
    {
        fDocHandler->endElement
        (
            *topElem->fThisElement
            , uriId
            , isRoot
            , (fDoNamespaces)
                ? topElem->fThisElement->getElementName()->getPrefix()
                : XMLUni::fgZeroLenString
        );
    }

    // If this was the root, then done with content
    gotData = !isRoot;
}


//  This method handles the high level logic of scanning the DOCType
//  declaration. This calls the DTDScanner and kicks off both the scanning of
//  the internal subset and the scanning of the external subset, if any.
//
//  When we get here the '<!DOCTYPE' part has already been scanned, which is
//  what told us that we had a doc type decl to parse.
void DGXMLScanner::scanDocTypeDecl()
{
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
        // put this in the undeclared pool so it gets deleted...
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
            if(srcUsed) {
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

bool DGXMLScanner::scanStartTag(bool& gotData)
{
    //  Assume we will still have data until proven otherwise. It will only
    //  ever be false if this is the root and its empty.
    gotData = true;

    //  Get the QName. In this case, we are not doing namespaces, so we just
    //  use it as is and don't have to break it into parts.

    bool validName = fReaderMgr.getName(fQNameBuf);
    if (!validName)
    {
        if (fQNameBuf.isEmpty())
            emitError(XMLErrs::ExpectedElementName);
        else
            emitError(XMLErrs::InvalidElementName, fQNameBuf.getRawBuffer());
        fReaderMgr.skipToChar(chOpenAngle);
        return false;
    }

    // Assume it won't be an empty tag
    bool isEmpty = false;

    // See if its the root element
    const bool isRoot = fElemStack.isEmpty();

    //  Lets try to look up the element in the validator's element decl pool
    //  We can pass bogus values for the URI id and the base name. We know that
    //  this can only be called if we are doing a DTD style validator and that
    //  he will only look at the QName.
    //
    //  We *do not* tell him to fault in a decl if he does not find one - NG.
    bool wasAdded = false;
    const XMLCh* qnameRawBuf = fQNameBuf.getRawBuffer();

    XMLElementDecl* elemDecl = fGrammar->getElemDecl
    (
        fEmptyNamespaceId
        , 0
        , qnameRawBuf
        , Grammar::TOP_LEVEL_SCOPE
    );
    // look in the undeclared pool:
    if(!elemDecl)
    {
        elemDecl = fDTDElemNonDeclPool->getByKey(qnameRawBuf);
    }
    if(!elemDecl)
    {
        wasAdded = true;
        elemDecl = new (fMemoryManager) DTDElementDecl
        (
            qnameRawBuf
            , fEmptyNamespaceId
            , DTDElementDecl::Any
            , fMemoryManager
        );
        elemDecl->setId(fDTDElemNonDeclPool->put((DTDElementDecl*)elemDecl));
    }

    if (fValidate) {

        if (wasAdded)
        {
            // This is to tell the reuse Validator that this element was
            // faulted-in, was not an element in the validator pool originally
            elemDecl->setCreateReason(XMLElementDecl::JustFaultIn);

            fValidator->emitError
            (
                XMLValid::ElementNotDefined
                , qnameRawBuf
            );
        }
        // If its not marked declared, then emit an error
        else if (!elemDecl->isDeclared())
        {
            fValidator->emitError
            (
                XMLValid::ElementNotDefined
                , qnameRawBuf
            );
        }


        fValidator->validateElement(elemDecl);
    }

    // Expand the element stack and add the new element
    fElemStack.addLevel(elemDecl, fReaderMgr.getCurrentReaderNum());

    //  If this is the first element and we are validating, check the root
    //  element.
    if (isRoot)
    {
        fRootGrammar = fGrammar;

        if (fValidate)
        {
            //  If a DocType exists, then check if it matches the root name there.
            if (fRootElemName && !XMLString::equals(qnameRawBuf, fRootElemName))
                fValidator->emitError(XMLValid::RootElemNotLikeDocType);
        }
    }
    else if (fValidate)
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
                if (fReaderMgr.getCurrentReader()->isWhitespace(nextCh))
                {
                    // Ok, skip by them and peek another char
                    fReaderMgr.skipPastSpaces();
                    nextCh = fReaderMgr.peekNextChar();
                }
                 else
                {
                    // Emit the error but keep on going
                    emitError(XMLErrs::ExpectedWhitespace);
                }
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

            validName = fReaderMgr.getName(fAttNameBuf);
            if (!validName)
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

            //  Add this attribute to the attribute list that we use to
            //  pass them to the handler. We reuse its existing elements
            //  but expand it as required.
            // Note that we want to this first since this will
            // make a copy of the namePtr; we can then make use of
            // that copy in the hashtable lookup that checks
            // for duplicates.  This will mean we may have to update
            // the type of the XMLAttr later.
            XMLAttr* curAtt;
            const XMLCh* attrValue = fAttValueBuf.getRawBuffer();

            if (attCount >= curAttListSize) {
                curAtt = new (fMemoryManager) XMLAttr(fMemoryManager);
                fAttrList->addElement(curAtt);
            }
            else {
                curAtt = fAttrList->elementAt(attCount);
            }

            curAtt->setSpecified(true);

            // NO NAMESPACE CODE
            {
                curAtt->set(
                    0, namePtr, XMLUni::fgZeroLenString, XMLUni::fgZeroLenString
                    , (attDef)?attDef->getType():XMLAttDef::CData
                );

                // now need to prepare for duplicate detection
                if (attDef) {
                    unsigned int *curCountPtr = fAttDefRegistry->get(attDef);
                    if (!curCountPtr) {
                        curCountPtr = getNewUIntPtr();
                        *curCountPtr = fElemCount;
                        fAttDefRegistry->put(attDef, curCountPtr);
                    }
                    else if (*curCountPtr < fElemCount) {
                        *curCountPtr = fElemCount;
                    }
                    else {
                        emitError(
                            XMLErrs::AttrAlreadyUsedInSTag
                            , attDef->getFullName(), elemDecl->getFullName()
                        );
                    }
                }
                else
                {
                    // reset namePtr so it refers to newly-allocated memory
                    namePtr = (XMLCh *)curAtt->getQName();
                    if (!fUndeclaredAttrRegistry->putIfNotPresent(namePtr, 0))
                    {
                        emitError(
                            XMLErrs::AttrAlreadyUsedInSTag
                            , namePtr, elemDecl->getFullName()
                        );
                    }
                }
            }

            if (fValidate)
            {
                if (attDef) {
                    // Let the validator pass judgement on the attribute value
                    fValidator->validateAttrValue(
                        attDef, fAttValueBuf.getRawBuffer(), false, elemDecl
                    );
                }
                else
                {
                    fValidator->emitError
                    (
                        XMLValid::AttNotDefinedForElement
                        , fAttNameBuf.getRawBuffer(), qnameRawBuf
                    );
                }
            }

            // must set the newly-minted value on the XMLAttr:
            curAtt->setValue(attrValue);
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

    //  Now lets get the fAttrList filled in. This involves faulting in any
    //  defaulted and fixed attributes and normalizing the values of any that
    //  we got explicitly.
    //
    //  We update the attCount value with the total number of attributes, but
    //  it goes in with the number of values we got during the raw scan of
    //  explictly provided attrs above.
    attCount = buildAttList(attCount, elemDecl, *fAttrList);

    //  If we have a document handler, then tell it about this start tag. We
    //  don't have any URI id to send along, so send fEmptyNamespaceId. We also do not send
    //  any prefix since its just one big name if we are not doing namespaces.
    unsigned int uriId = fEmptyNamespaceId;
    if (fDocHandler)
    {
        fDocHandler->startElement
        (
            *elemDecl
            , uriId
            , 0
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
                    , qnameRawBuf
                    , elemDecl->getFormattedContentModel()
                );
            }
        }

        // Pop the element stack back off since it'll never be used now
        fElemStack.popTop();

        // If the elem stack is empty, then it was an empty root
        if (isRoot)
            gotData = false;
    }

    return true;
}


bool DGXMLScanner::scanStartTagNS(bool& gotData)
{
    //  Assume we will still have data until proven otherwise. It will only
    //  ever be false if this is the root and its empty.
    gotData = true;

    //  Get the QName. In this case, we are not doing namespaces, so we just
    //  use it as is and don't have to break it into parts.

    int  colonPosition;
    bool validName = fReaderMgr.getQName(fQNameBuf, &colonPosition);
    if (!validName)
    {
        if (fQNameBuf.isEmpty())
            emitError(XMLErrs::ExpectedElementName);
        else
            emitError(XMLErrs::InvalidElementName, fQNameBuf.getRawBuffer());
        fReaderMgr.skipToChar(chOpenAngle);
        return false;
    }

    // Assume it won't be an empty tag
    bool isEmpty = false;

    // See if its the root element
    const bool isRoot = fElemStack.isEmpty();

    //  Lets try to look up the element in the validator's element decl pool
    //  We can pass bogus values for the URI id and the base name. We know that
    //  this can only be called if we are doing a DTD style validator and that
    //  he will only look at the QName.
    //
    //  We *do not* tell him to fault in a decl if he does not find one - NG.
    bool wasAdded = false;
    const XMLCh* qnameRawBuf = fQNameBuf.getRawBuffer();

    XMLElementDecl* elemDecl = fGrammar->getElemDecl
    (
        fEmptyNamespaceId
        , 0
        , qnameRawBuf
        , Grammar::TOP_LEVEL_SCOPE
    );
    // look in the undeclared pool:
    if(!elemDecl)
    {
        elemDecl = fDTDElemNonDeclPool->getByKey(qnameRawBuf);
    }
    if(!elemDecl)
    {
        wasAdded = true;
        elemDecl = new (fMemoryManager) DTDElementDecl
        (
            qnameRawBuf
            , fEmptyNamespaceId
            , DTDElementDecl::Any
            , fMemoryManager
        );
        elemDecl->setId(fDTDElemNonDeclPool->put((DTDElementDecl*)elemDecl));
    }

    if (fValidate) {

        if (wasAdded)
        {
            // This is to tell the reuse Validator that this element was
            // faulted-in, was not an element in the validator pool originally
            elemDecl->setCreateReason(XMLElementDecl::JustFaultIn);

            fValidator->emitError
            (
                XMLValid::ElementNotDefined
                , qnameRawBuf
            );
        }
        // If its not marked declared, then emit an error
        else if (!elemDecl->isDeclared())
        {
            fValidator->emitError
            (
                XMLValid::ElementNotDefined
                , qnameRawBuf
            );
        }


        fValidator->validateElement(elemDecl);
    }

    // Expand the element stack and add the new element
    fElemStack.addLevel(elemDecl, fReaderMgr.getCurrentReaderNum());

    //  If this is the first element and we are validating, check the root
    //  element.
    if (isRoot)
    {
        fRootGrammar = fGrammar;

        if (fValidate)
        {
            //  If a DocType exists, then check if it matches the root name there.
            if (fRootElemName && !XMLString::equals(qnameRawBuf, fRootElemName))
                fValidator->emitError(XMLValid::RootElemNotLikeDocType);
        }
    }
    else if (fValidate)
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
                if (fReaderMgr.getCurrentReader()->isWhitespace(nextCh))
                {
                    // Ok, skip by them and peek another char
                    fReaderMgr.skipPastSpaces();
                    nextCh = fReaderMgr.peekNextChar();
                }
                 else
                {
                    // Emit the error but keep on going
                    emitError(XMLErrs::ExpectedWhitespace);
                }
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

            validName = fReaderMgr.getQName(fAttNameBuf, &colonPosition);
            if (!validName)
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

            //  Add this attribute to the attribute list that we use to
            //  pass them to the handler. We reuse its existing elements
            //  but expand it as required.
            // Note that we want to this first since this will
            // make a copy of the namePtr; we can then make use of
            // that copy in the hashtable lookup that checks
            // for duplicates.  This will mean we may have to update
            // the type of the XMLAttr later.
            XMLAttr* curAtt;
            const XMLCh* attrValue = fAttValueBuf.getRawBuffer();

            if (attCount >= curAttListSize) {
                curAtt = new (fMemoryManager) XMLAttr(fMemoryManager);
                fAttrList->addElement(curAtt);
            }
            else {
                curAtt = fAttrList->elementAt(attCount);
            }

            curAtt->setSpecified(true);
            // DO NAMESPACES
            {
                curAtt->set(
                    fEmptyNamespaceId, namePtr, XMLUni::fgZeroLenString
                    , (attDef)? attDef->getType() : XMLAttDef::CData
                );

                // each attribute has the prefix:suffix="value"
                const XMLCh* attPrefix = curAtt->getPrefix();
                const XMLCh* attLocalName = curAtt->getName();

                if (attPrefix && *attPrefix) {
                    if (XMLString::equals(attPrefix, XMLUni::fgXMLString)) {
                        curAtt->setURIId(fXMLNamespaceId);
                    }
                    else if (XMLString::equals(attPrefix, XMLUni::fgXMLNSString)) {
                        curAtt->setURIId(fXMLNSNamespaceId);
                        updateNSMap(attPrefix, attLocalName, attrValue);
                    }
                    else {
                        fAttrNSList->addElement(curAtt);
                    }
                }
                else if (XMLString::equals(XMLUni::fgXMLNSString, attLocalName))
                {
                    updateNSMap(attPrefix, XMLUni::fgZeroLenString, attrValue);
                }

                // NOTE: duplicate attribute check will be done, when we map
                //       namespaces to all attributes
                if (attDef) {
                    unsigned int *curCountPtr = fAttDefRegistry->get(attDef);
                    if (!curCountPtr) {
                        curCountPtr = getNewUIntPtr();
                        *curCountPtr = fElemCount;
                        fAttDefRegistry->put(attDef, curCountPtr);
                   }
                    else if (*curCountPtr < fElemCount) {
                        *curCountPtr = fElemCount;
                    }
                }
            }

            if (fValidate)
            {
                if (attDef) {
                    // Let the validator pass judgement on the attribute value
                    fValidator->validateAttrValue(
                        attDef, fAttValueBuf.getRawBuffer(), false, elemDecl
                    );
                }
                else
                {
                    fValidator->emitError
                    (
                        XMLValid::AttNotDefinedForElement
                        , fAttNameBuf.getRawBuffer(), qnameRawBuf
                    );
                }
            }

            // must set the newly-minted value on the XMLAttr:
            curAtt->setValue(attrValue);
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

    //  Make an initial pass through the list and find any xmlns attributes.
    if (attCount)
      scanAttrListforNameSpaces(fAttrList, attCount, elemDecl);

    if(attCount)
    {
        // clean up after ourselves:
        // clear the map used to detect duplicate attributes
        fUndeclaredAttrRegistry->removeAll();
    }

    //  Now lets get the fAttrList filled in. This involves faulting in any
    //  defaulted and fixed attributes and normalizing the values of any that
    //  we got explicitly.
    //
    //  We update the attCount value with the total number of attributes, but
    //  it goes in with the number of values we got during the raw scan of
    //  explictly provided attrs above.
    attCount = buildAttList(attCount, elemDecl, *fAttrList);

    //  If we have a document handler, then tell it about this start tag. We
    //  don't have any URI id to send along, so send fEmptyNamespaceId. We also do not send
    //  any prefix since its just one big name if we are not doing namespaces.
    if (fDocHandler)
    {
        unsigned int uriId = resolvePrefix
            (
                elemDecl->getElementName()->getPrefix()
                , ElemStack::Mode_Element
            );

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
                    , qnameRawBuf
                    , elemDecl->getFormattedContentModel()
                );
            }
        }

        // Pop the element stack back off since it'll never be used now
        fElemStack.popTop();

        // If the elem stack is empty, then it was an empty root
        if (isRoot)
            gotData = false;
    }

    return true;
}

// ---------------------------------------------------------------------------
//  DGXMLScanner: Grammar preparsing
// ---------------------------------------------------------------------------
Grammar* DGXMLScanner::loadGrammar(const   InputSource& src
                                   , const short        grammarType
                                   , const bool         toCache)
{
    Grammar* loadedGrammar = 0;

    ReaderMgrResetType  resetReaderMgr(&fReaderMgr, &ReaderMgr::reset);

    try
    {
        fGrammarResolver->cacheGrammarFromParse(false);
        fGrammarResolver->useCachedGrammarInParse(false);
        fRootGrammar = 0;

        if (fValScheme == Val_Auto) {
            fValidate = true;
        }

        // Reset some status flags
        fInException = false;
        fStandalone = false;
        fErrorCount = 0;
        fHasNoDTD = true;

        if (grammarType == Grammar::DTDGrammarType) {
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

    return loadedGrammar;
}

Grammar* DGXMLScanner::loadDTDGrammar(const InputSource& src,
                                      const bool toCache)
{
    // Reset the validators
    fDTDValidator->reset();
    if (fValidatorFromUser)
        fValidator->reset();

    fDTDGrammar = new (fGrammarPoolMemoryManager) DTDGrammar(fGrammarPoolMemoryManager);
    fGrammarResolver->putGrammar(fDTDGrammar);
    fGrammar = fDTDGrammar;
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
        (DTDGrammar*)fGrammar
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
//  DGXMLScanner: Private helper methods
// ---------------------------------------------------------------------------
//  This method handles the common initialization, to avoid having to do
//  it redundantly in multiple constructors.
void DGXMLScanner::commonInit()
{
    //  And we need one for the raw attribute scan. This just stores key/
    //  value string pairs (prior to any processing.)
    fAttrNSList = new (fMemoryManager) ValueVectorOf<XMLAttr*>(8, fMemoryManager);

    //  Create the Validator and init them
    fDTDValidator = new (fMemoryManager) DTDValidator();
    initValidator(fDTDValidator);
    fDTDElemNonDeclPool = new (fMemoryManager) NameIdPool<DTDElementDecl>(29, 128, fMemoryManager);
    fAttDefRegistry = new (fMemoryManager) RefHashTableOf<unsigned int, PtrHasher>
    (
        131, false, fMemoryManager
    );
    fUndeclaredAttrRegistry = new (fMemoryManager) Hash2KeysSetOf<StringHasher>(7, fMemoryManager);

    if (fValidator)
    {
        if (!fValidator->handlesDTD())
           ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Gen_NoDTDValidator, fMemoryManager);
    }
    else
    {
        fValidator = fDTDValidator;
    }
}

void DGXMLScanner::cleanUp()
{
    delete fAttrNSList;
    delete fDTDValidator;
    delete fDTDElemNonDeclPool;
    delete fAttDefRegistry;
    delete fUndeclaredAttrRegistry;
}


//  This method is called from scanStartTagNS() to build up the list of
//  XMLAttr objects that will be passed out in the start tag callout. We
//  get the key/value pairs from the raw scan of explicitly provided attrs,
//  which have not been normalized. And we get the element declaration from
//  which we will get any defaulted or fixed attribute defs and add those
//  in as well.
XMLSize_t
DGXMLScanner::buildAttList(const XMLSize_t              attCount
                          ,       XMLElementDecl*       elemDecl
                          ,       RefVectorOf<XMLAttr>& toFill)
{
    //  Ask the element to clear the 'provided' flag on all of the att defs
    //  that it owns, and to return us a boolean indicating whether it has
    //  any defs.
    const bool hasDefs = elemDecl->hasAttDefs();

    //  If there are no expliclitily provided attributes and there are no
    //  defined attributes for the element, the we don't have anything to do.
    //  So just return zero in this case.
    if (!hasDefs && !attCount)
        return 0;

    // Keep up with how many attrs we end up with total
    XMLSize_t retCount = attCount;

    //  And get the current size of the output vector. This lets us use
    //  existing elements until we fill it, then start adding new ones.
    const XMLSize_t curAttListSize = toFill.size();

    //  Ok, so lets get an enumerator for the attributes of this element
    //  and run through them for well formedness and validity checks. But
    //  make sure that we had any attributes before we do it, since the list
    //  would have have gotten faulted in anyway.
    if (hasDefs)
    {
        XMLAttDefList& attDefList = elemDecl->getAttDefList();
        for(XMLSize_t i=0; i<attDefList.getAttDefCount(); i++)
        {
            // Get the current att def, for convenience and its def type
            XMLAttDef& curDef = attDefList.getAttDef(i);

            unsigned int *attCountPtr = fAttDefRegistry->get(&curDef);
            if (!attCountPtr || *attCountPtr < fElemCount)
            { // did not occur
                const XMLAttDef::DefAttTypes defType = curDef.getDefaultType();

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
                    if (retCount >= curAttListSize)
                    {
                        if (fDoNamespaces)
                        {
                            curAtt = new (fMemoryManager) XMLAttr
                            (
                                fEmptyNamespaceId
                                , curDef.getFullName()
                                , curDef.getValue()
                                , curDef.getType()
                                , false
                                , fMemoryManager
                            );
                        }
                        else
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
                        }

                        fAttrList->addElement(curAtt);
                    }
                    else
                    {
                        curAtt = fAttrList->elementAt(retCount);
                        if (fDoNamespaces)
                        {
                            curAtt->set
                            (
                                fEmptyNamespaceId
                                , curDef.getFullName()
                                , curDef.getValue()
                                , curDef.getType()
                            );
                        }
                        else
                        {
                            curAtt->set
                            (
                                0
                                , curDef.getFullName()
                                , XMLUni::fgZeroLenString
                                , curDef.getValue()
                                , curDef.getType()
                            );
                        }
                        curAtt->setSpecified(false);
                    }

                    if (fDoNamespaces)
                    {
                        //  Map the new attribute's prefix to a URI id and store
                        //  that in the attribute object.
                        const XMLCh* attPrefix = curAtt->getPrefix();
                        if (attPrefix && *attPrefix) {
                            curAtt->setURIId
                            (
                                resolvePrefix(attPrefix, ElemStack::Mode_Attribute)
                            );
                        }
                    }

                    retCount++;
                }
            }
        }
    }

    return retCount;
}


//  This method will reset the scanner data structures, and related plugged
//  in stuff, for a new scan session. We get the input source for the primary
//  XML entity, create the reader for it, and push it on the stack so that
//  upon successful return from here we are ready to go.
void DGXMLScanner::scanReset(const InputSource& src)
{

    //  This call implicitly tells us that we are going to reuse the scanner
    //  if it was previously used. So tell the validator to reset itself.
    //
    //  But, if the fUseCacheGrammar flag is set, then don't reset it.
    //
    //  NOTE:   The ReaderMgr is flushed on the way out, because that is
    //          required to insure that files are closed.
    fGrammarResolver->cacheGrammarFromParse(fToCacheGrammar);
    fGrammarResolver->useCachedGrammarInParse(fUseCachedGrammar);

    fDTDGrammar = new (fGrammarPoolMemoryManager) DTDGrammar(fGrammarPoolMemoryManager);
    fGrammarResolver->putGrammar(fDTDGrammar);
    fGrammar = fDTDGrammar;
    fRootGrammar = 0;
    fValidator->setGrammar(fGrammar);

    // Reset validation
    fValidate = (fValScheme == Val_Always) ? true : false;

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

    // Reset the Root Element Name
    fMemoryManager->deallocate(fRootElemName);//delete [] fRootElemName;
    fRootElemName = 0;

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

    // Reset the validators
    fDTDValidator->reset();
    fDTDValidator->setErrorReporter(fErrorReporter);
    if (fValidatorFromUser)
        fValidator->reset();

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
    if(fUIntPoolRowTotal >= 32)
    { // 8 KB tied up with validating attributes...
        fAttDefRegistry->removeAll();
        recreateUIntPool();
    }
    else
    {
        // note that this will implicitly reset the values of the hashtables,
        // though their buckets will still be tied up
        resetUIntPool();
    }
    fUndeclaredAttrRegistry->removeAll();
    fAttrNSList->removeAllElements();
}


//  This method is called between markup in content. It scans for character
//  data that is sent to the document handler. It watches for any markup
//  characters that would indicate that the character data has ended. It also
//  handles expansion of general and character entities.
//
//  sendData() is a local static helper for this method which handles some
//  code that must be done in three different places here.
void DGXMLScanner::sendCharData(XMLBuffer& toSend)
{
    // If no data in the buffer, then nothing to do
    if (toSend.isEmpty())
        return;

    //  We do different things according to whether we are validating or
    //  not. If not, its always just characters; else, it depends on the
    //  current element's content model.
    if (fValidate)
    {
        // Get the raw data we need for the callback
        const XMLCh* const rawBuf = toSend.getRawBuffer();
        const XMLSize_t len = toSend.getLen();

        // And see if the current element is a 'Children' style content model
        const ElemStack::StackElem* topElem = fElemStack.topElement();

        // Get the character data opts for the current element
        XMLElementDecl::CharDataOpts charOpts = topElem->fThisElement->getCharDataOpts();

        if (charOpts == XMLElementDecl::NoCharData)
        {
            // They definitely cannot handle any type of char data
            fValidator->emitError(XMLValid::NoCharDataInCM);
        }
        else if (fReaderMgr.getCurrentReader()->isAllSpaces(rawBuf, len))
        {
            //  Its all spaces. So, if they can take spaces, then send it
            //  as ignorable whitespace. If they can handle any char data
            //  send it as characters.
            if (charOpts == XMLElementDecl::SpacesOk) {
                if (fDocHandler)
                    fDocHandler->ignorableWhitespace(rawBuf, len, false);
            }
            else if (charOpts == XMLElementDecl::AllCharData)
            {
                if (fDocHandler)
                    fDocHandler->docCharacters(rawBuf, len, false);
            }
        }
        else
        {
            //  If they can take any char data, then send it. Otherwise, they
            //  can only handle whitespace and can't handle this stuff so
            //  issue an error.
            if (charOpts == XMLElementDecl::AllCharData)
            {
                if (fDocHandler)
                    fDocHandler->docCharacters(rawBuf, len, false);
            }
            else
            {
                fValidator->emitError(XMLValid::NoCharDataInCM);
            }
        }
    }
    else
    {
        // Always assume its just char data if not validating
        if (fDocHandler)
            fDocHandler->docCharacters(toSend.getRawBuffer(), toSend.getLen(), false);
    }

    // Reset buffer
    toSend.reset();
}



//  This method is called with a key/value string pair that represents an
//  xmlns="yyy" or xmlns:xxx="yyy" attribute. This method will update the
//  current top of the element stack based on this data. We know that when
//  we get here, that it is one of these forms, so we don't bother confirming
//  it.
//
//  But we have to ensure
//      1. xxx is not xmlns
//      2. if xxx is xml, then yyy must match XMLUni::fgXMLURIName, and vice versa
//      3. yyy is not XMLUni::fgXMLNSURIName
//      4. if xxx is not null, then yyy cannot be an empty string.
void DGXMLScanner::updateNSMap(const    XMLCh* const attrPrefix
                               , const  XMLCh* const attrLocalName
                               , const  XMLCh* const attrValue)
{
    //  We either have the default prefix (""), or we point it into the attr
    //  name parameter. Note that the xmlns is not the prefix we care about
    //  here. To us, the 'prefix' is really the local part of the attrName
    //  parameter.
    //
    //  Check 1. xxx is not xmlns
    //        2. if xxx is xml, then yyy must match XMLUni::fgXMLURIName, and vice versa
    //        3. yyy is not XMLUni::fgXMLNSURIName
    //        4. if xxx is not null, then yyy cannot be an empty string.
    if (attrPrefix && *attrPrefix) {

        if (XMLString::equals(attrLocalName, XMLUni::fgXMLNSString))
            emitError(XMLErrs::NoUseOfxmlnsAsPrefix);
        else if (XMLString::equals(attrLocalName, XMLUni::fgXMLString)) {
            if (!XMLString::equals(attrValue, XMLUni::fgXMLURIName))
                emitError(XMLErrs::PrefixXMLNotMatchXMLURI);
        }

        if (!attrValue)
            emitError(XMLErrs::NoEmptyStrNamespace, attrLocalName);
        else if(!*attrValue && fXMLVersion == XMLReader::XMLV1_0)
            emitError(XMLErrs::NoEmptyStrNamespace, attrLocalName);
    }

    if (XMLString::equals(attrValue, XMLUni::fgXMLNSURIName))
        emitError(XMLErrs::NoUseOfxmlnsURI);
    else if (XMLString::equals(attrValue, XMLUni::fgXMLURIName)) {
        if (!XMLString::equals(attrLocalName, XMLUni::fgXMLString))
            emitError(XMLErrs::XMLURINotMatchXMLPrefix);
    }

    //  Ok, we have to get the unique id for the attribute value, which is the
    //  URI that this value should be mapped to. The validator has the
    //  namespace string pool, so we ask him to find or add this new one. Then
    //  we ask the element stack to add this prefix to URI Id mapping.
    fElemStack.addPrefix
    (
        attrLocalName
        , fURIStringPool->addOrFind(attrValue)
    );
}

void DGXMLScanner::scanAttrListforNameSpaces(RefVectorOf<XMLAttr>* theAttrList, XMLSize_t attCount,
                                                XMLElementDecl*       elemDecl)
{
    // Map prefixes to uris
    for (XMLSize_t i=0; i < fAttrNSList->size(); i++) {
        XMLAttr* providedAttr = fAttrNSList->elementAt(i);
        providedAttr->setURIId(
            resolvePrefix(providedAttr->getPrefix(), ElemStack::Mode_Attribute)
        );
    }

    fAttrNSList->removeAllElements();

     // Decide if to use hash table to do duplicate checking
    bool toUseHashTable = false;

	setAttrDupChkRegistry(attCount, toUseHashTable);
    for (XMLSize_t index = 0; index < attCount; index++)
    {
        // check for duplicate namespace attributes:
        // by checking for qualified names with the same local part and with prefixes
        // which have been bound to namespace names that are identical.
        XMLAttr* curAttr = theAttrList->elementAt(index);
        if (!toUseHashTable)
        {
            XMLAttr* loopAttr;
            for (XMLSize_t attrIndex=0; attrIndex < index; attrIndex++) {
                loopAttr = theAttrList->elementAt(attrIndex);
                if (loopAttr->getURIId() == curAttr->getURIId() &&
                    XMLString::equals(loopAttr->getName(), curAttr->getName())) {
                    emitError(
                        XMLErrs::AttrAlreadyUsedInSTag, curAttr->getName()
                        , elemDecl->getFullName()
                    );
                }
            }
        }
        else
        {
            if (fAttrDupChkRegistry->containsKey((void*)curAttr->getName(), curAttr->getURIId()))
            {
                emitError(
                    XMLErrs::AttrAlreadyUsedInSTag
                    , curAttr->getName(), elemDecl->getFullName()
                );
            }

            fAttrDupChkRegistry->put((void*)curAttr->getName(), curAttr->getURIId(), curAttr);
        }
    }
}

InputSource* DGXMLScanner::resolveSystemId(const XMLCh* const sysId
                                          ,const XMLCh* const pubId)
{
    //Normalize sysId
    XMLBufBid nnSys(&fBufMgr);
    XMLBuffer& normalizedSysId = nnSys.getBuffer();
    XMLString::removeChar(sysId, 0xFFFF, normalizedSysId);
    const XMLCh* normalizedURI = normalizedSysId.getRawBuffer();

    // Create a buffer for expanding the normalized system id
    XMLBufBid bbSys(&fBufMgr);
    XMLBuffer& expSysId = bbSys.getBuffer();

    //  Allow the entity handler to expand the system id if they choose
    //  to do so.
    InputSource* srcToFill = 0;
    if (fEntityHandler)
    {
        if (!fEntityHandler->expandSystemId(normalizedURI, expSysId))
            expSysId.set(normalizedURI);

        ReaderMgr::LastExtEntityInfo lastInfo;
        fReaderMgr.getLastExtEntityInfo(lastInfo);
        XMLResourceIdentifier resourceIdentifier(XMLResourceIdentifier::ExternalEntity,
                            expSysId.getRawBuffer(), 0, pubId, lastInfo.systemId,
                            &fReaderMgr);
        srcToFill = fEntityHandler->resolveEntity(&resourceIdentifier);
    }
    else
    {
        expSysId.set(normalizedURI);
    }

    //  If they didn't create a source via the entity handler, then we
    //  have to create one on our own.
    if (!srcToFill)
    {
        if (fDisableDefaultEntityResolution)
            return srcToFill;

        ReaderMgr::LastExtEntityInfo lastInfo;
        fReaderMgr.getLastExtEntityInfo(lastInfo);

        XMLURL urlTmp(fMemoryManager);
        if ((!urlTmp.setURL(lastInfo.systemId, expSysId.getRawBuffer(), urlTmp)) ||
            (urlTmp.isRelative()))
        {
            if (!fStandardUriConformant)
            {
                XMLBufBid  ddSys(&fBufMgr);
                XMLBuffer& resolvedSysId = ddSys.getBuffer();
                XMLUri::normalizeURI(expSysId.getRawBuffer(), resolvedSysId);

                srcToFill = new (fMemoryManager) LocalFileInputSource
                (
                    lastInfo.systemId
                    , resolvedSysId.getRawBuffer()
                    , fMemoryManager
                );
            }
            else
                ThrowXMLwithMemMgr(MalformedURLException, XMLExcepts::URL_MalformedURL, fMemoryManager);
        }
        else
        {
            if (fStandardUriConformant && urlTmp.hasInvalidChar())
                ThrowXMLwithMemMgr(MalformedURLException, XMLExcepts::URL_MalformedURL, fMemoryManager);
            srcToFill = new (fMemoryManager) URLInputSource(urlTmp, fMemoryManager);
        }
    }

    return srcToFill;
}

// ---------------------------------------------------------------------------
//  DGXMLScanner: Private parsing methods
// ---------------------------------------------------------------------------
bool DGXMLScanner::scanAttValue(  const   XMLAttDef* const    attDef
                                  , const XMLCh *const attrName
                                  ,       XMLBuffer&          toFill)
{
    enum States
    {
        InWhitespace
        , InContent
    };

    // Get the type and name
    const XMLAttDef::AttTypes type = (attDef)
                        ?attDef->getType()
                        :XMLAttDef::CData;

    // Reset the target buffer
    toFill.reset();

    // Get the next char which must be a single or double quote
    XMLCh quoteCh;
    if (!fReaderMgr.skipIfQuote(quoteCh))
        return false;

    //  We have to get the current reader because we have to ignore closing
    //  quotes until we hit the same reader again.
    const XMLSize_t curReader = fReaderMgr.getCurrentReaderNum();

    // Get attribute def - to check to see if it's declared externally or not
    bool  isAttExternal = (attDef)
                        ?attDef->isExternal()
                        :false;

    //  Loop until we get the attribute value. Note that we use a double
    //  loop here to avoid the setup/teardown overhead of the exception
    //  handler on every round.
    XMLCh   nextCh;
    XMLCh   secondCh = 0;
    States  curState = InContent;
    bool    firstNonWS = false;
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
                    if (!fReaderMgr.getCurrentReader()->isXMLChar(nextCh))
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
            if (!escaped && (nextCh == chOpenAngle))
                emitError(XMLErrs::BracketInAttrValue, attrName);

            //  If the attribute is a CDATA type we do simple replacement of
            //  tabs and new lines with spaces, if the character is not escaped
            //  by way of a char ref.
            //
            //  Otherwise, we do the standard non-CDATA normalization of
            //  compressing whitespace to single spaces and getting rid of leading
            //  and trailing whitespace.
            if (type == XMLAttDef::CData)
            {
                if (!escaped)
                {
                    if ((nextCh == 0x09) || (nextCh == 0x0A) || (nextCh == 0x0D))
                    {
                        // Check Validity Constraint for Standalone document declaration
                        // XML 1.0, Section 2.9
                        if (fStandalone && fValidate && isAttExternal)
                        {
                             // Can't have a standalone document declaration of "yes" if  attribute
                             // values are subject to normalisation
                             fValidator->emitError(XMLValid::NoAttNormForStandalone, attrName);
                        }
                        nextCh = chSpace;
                    }
                }
            }
            else
            {
                if (curState == InWhitespace)
                {
                    if ((escaped && nextCh != chSpace) || !fReaderMgr.getCurrentReader()->isWhitespace(nextCh))
                    {
                        if (firstNonWS)
                            toFill.append(chSpace);
                        curState = InContent;
                        firstNonWS = true;
                    }
                    else
                    {
                        continue;
                    }
                }
                else if (curState == InContent)
                {
                    if ((nextCh == chSpace) ||
                        (fReaderMgr.getCurrentReader()->isWhitespace(nextCh) && !escaped))
                    {
                        curState = InWhitespace;

                        // Check Validity Constraint for Standalone document declaration
                        // XML 1.0, Section 2.9
                        if (fStandalone && fValidate && isAttExternal)
                        {
                            if (!firstNonWS || (nextCh != chSpace) || (fReaderMgr.lookingAtSpace()))
                            {
                                 // Can't have a standalone document declaration of "yes" if  attribute
                                 // values are subject to normalisation
                                 fValidator->emitError(XMLValid::NoAttNormForStandalone, attrName);
                            }
                        }
                        continue;
                    }
                    firstNonWS = true;
                }
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
void DGXMLScanner::scanCDSection()
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
    bool     gotLeadingSurrogate = false;

    // Get the character data opts for the current element
    const ElemStack::StackElem* topElem = fElemStack.topElement();
    XMLElementDecl::CharDataOpts charOpts =  topElem->fThisElement->getCharDataOpts();

    while (true)
    {
        const XMLCh nextCh = fReaderMgr.getNextChar();

        // Watch for unexpected end of file
        if (!nextCh)
        {
            emitError(XMLErrs::UnterminatedCDATASection);
            ThrowXMLwithMemMgr(UnexpectedEOFException, XMLExcepts::Gen_UnexpectedEOF, fMemoryManager);
        }

        if (fValidate && fStandalone && (fReaderMgr.getCurrentReader()->isWhitespace(nextCh)))
        {
            // This document is standalone; this ignorable CDATA whitespace is forbidden.
            // XML 1.0, Section 2.9
            // And see if the current element is a 'Children' style content model
            if (topElem->fThisElement->isExternal()) {

                if (charOpts == XMLElementDecl::SpacesOk) // Element Content
                {
                    // Error - standalone should have a value of "no" as whitespace detected in an
                    // element type with element content whose element declaration was external
                    fValidator->emitError(XMLValid::NoWSForStandalone);
                }
            }
        }

        //  If this is a close square bracket it could be our closing
        //  sequence.
        if (nextCh == chCloseSquare && fReaderMgr.skippedString(CDataClose))
        {
            //  make sure we were not expecting a trailing surrogate.
            if (gotLeadingSurrogate)
                emitError(XMLErrs::Expected2ndSurrogateChar);

            if (fValidate) {

                if (charOpts != XMLElementDecl::AllCharData)
                {
                    // They definitely cannot handle any type of char data
                    fValidator->emitError(XMLValid::NoCharDataInCM);
                }
            }

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


void DGXMLScanner::scanCharData(XMLBuffer& toUse)
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
                    else
                    {
                        if (escaped && !fElemStack.isEmpty())
                            fElemStack.setReferenceEscaped();
                    }
                }
                else if ((nextCh >= 0xD800) && (nextCh <= 0xDBFF))
                {
                    // Deal with surrogate pairs
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

                        // Make sure the returned char is a valid XML char
                        if (!fReaderMgr.getCurrentReader()->isXMLChar(nextCh))
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

    // Check the validity constraints as per XML 1.0 Section 2.9
    if (fValidate && fStandalone)
    {
        // See if the text contains whitespace
        // Get the raw data we need for the callback
        const XMLCh* rawBuf = toUse.getRawBuffer();
        const XMLSize_t len = toUse.getLen();
        const bool isSpaces = fReaderMgr.getCurrentReader()->containsWhiteSpace(rawBuf, len);

        if (isSpaces)
        {
            // And see if the current element is a 'Children' style content model
            const ElemStack::StackElem* topElem = fElemStack.topElement();

            if (topElem->fThisElement->isExternal()) {

                // Get the character data opts for the current element
                XMLElementDecl::CharDataOpts charOpts =  topElem->fThisElement->getCharDataOpts();

                if (charOpts == XMLElementDecl::SpacesOk)  // => Element Content
                {
                    // Error - standalone should have a value of "no" as whitespace detected in an
                    // element type with element content whose element declaration was external
                    //
                    fValidator->emitError(XMLValid::NoWSForStandalone);
                }
            }
        }
    }
    // Send any char data that we accumulated into the buffer
    sendCharData(toUse);
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
DGXMLScanner::EntityExpRes
DGXMLScanner::scanEntityRef(  const   bool    inAttVal
                            ,       XMLCh&  firstCh
                            ,       XMLCh&  secondCh
                            ,       bool&   escaped)
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

    int  colonPosition;
    bool validName = fDoNamespaces ? fReaderMgr.getQName(bbName.getBuffer(), &colonPosition) :
                                     fReaderMgr.getName(bbName.getBuffer());
    if (!validName)
    {
        if (bbName.isEmpty())
            emitError(XMLErrs::ExpectedEntityRefName);
        else
            emitError(XMLErrs::InvalidEntityRefName, bbName.getRawBuffer());
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
    XMLEntityDecl* decl = fDTDGrammar->getEntityDecl(bbName.getRawBuffer());

    // If it does not exist, then obviously an error
    if (!decl)
    {
        // XML 1.0 Section 4.1
        // Well-formedness Constraint for entity not found:
        //   In a document without any DTD, a document with only an internal DTD subset which contains no parameter entity references,
        //      or a document with "standalone='yes'", for an entity reference that does not occur within the external subset
        //      or a parameter entity
        //
        // Else it's Validity Constraint
        if (fStandalone || fHasNoDTD)
            emitError(XMLErrs::EntityNotFound, bbName.getRawBuffer());
        else {
            if (fValidate)
                fValidator->emitError(XMLValid::VC_EntityNotFound, bbName.getRawBuffer());
        }

        return EntityExp_Failed;
    }

    // XML 1.0 Section 4.1
    //  If we are a standalone document, then it has to have been declared
    //  in the internal subset.
    if (fStandalone && !decl->getDeclaredInIntSubset())
        emitError(XMLErrs::IllegalRefInStandalone, bbName.getRawBuffer());

    if (decl->isExternal())
    {
        // If its unparsed, then its not valid here
        if (decl->isUnparsed())
        {
            emitError(XMLErrs::NoUnparsedEntityRefs, bbName.getRawBuffer());
            return EntityExp_Failed;
        }

        // If we are in an attribute value, then not valid but keep going
        if (inAttVal)
            emitError(XMLErrs::NoExtRefsInAttValue);

        // And now create a reader to read this entity
        InputSource* srcUsed;
        XMLReader* reader = fReaderMgr.createReader
        (
            decl->getBaseURI()
            , decl->getSystemId()
            , decl->getPublicId()
            , false
            , XMLReader::RefFrom_NonLiteral
            , XMLReader::Type_General
            , XMLReader::Source_External
            , srcUsed
            , fCalculateSrcOfs
            , fLowWaterMark
            , fDisableDefaultEntityResolution
        );

        // Put a janitor on the source so it gets cleaned up on exit
        Janitor<InputSource> janSrc(srcUsed);

        //  If the creation failed, and its not because the source was empty,
        //  then emit an error and return.
        if (!reader)
            ThrowXMLwithMemMgr1(RuntimeException, XMLExcepts::Gen_CouldNotOpenExtEntity, srcUsed ? srcUsed->getSystemId() : decl->getSystemId(), fMemoryManager);

        //  Push the reader. If its a recursive expansion, then emit an error
        //  and return an failure.
        if (!fReaderMgr.pushReader(reader, decl))
        {
            emitError(XMLErrs::RecursiveEntity, decl->getName());
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
            // there seems nothing better to do than reset the entity expansion counter
            fEntityExpansionCount = 0;
        }

        //  Do a start entity reference event.
        //
        //  <TBD> For now, we supress them in att values. Later, when
        //  the stuff is in place to correctly allow DOM to handle them
        //  we'll turn this back on.
        if (fDocHandler && !inAttVal)
            fDocHandler->startEntityReference(*decl);

        // If it starts with the XML string, then parse a text decl
        if (checkXMLDecl(true))
            scanXMLDecl(Decl_Text);
    }
    else
    {
        //  If its one of the special char references, then we can return
        //  it as a character, and its considered escaped.
        if (decl->getIsSpecialChar())
        {
            firstCh = decl->getValue()[0];
            escaped = true;
            return EntityExp_Returned;
        }

        //  Create a reader over a memory stream over the entity value
        //  We force it to assume UTF-16 by passing in an encoding
        //  string. This way it won't both trying to predecode the
        //  first line, looking for an XML/TextDecl.
        XMLReader* valueReader = fReaderMgr.createIntEntReader
        (
            decl->getName()
            , XMLReader::RefFrom_NonLiteral
            , XMLReader::Type_General
            , decl->getValue()
            , decl->getValueLen()
            , false
        );

        //  Try to push the entity reader onto the reader manager stack,
        //  where it will become the subsequent input. If it fails, that
        //  means the entity is recursive, so issue an error. The reader
        //  will have just been discarded, but we just keep going.
        if (!fReaderMgr.pushReader(valueReader, decl))
            emitError(XMLErrs::RecursiveEntity, decl->getName());

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
        }

        //  Do a start entity reference event.
        //
        //  <TBD> For now, we supress them in att values. Later, when
        //  the stuff is in place to correctly allow DOM to handle them
        //  we'll turn this back on.
        if (fDocHandler && !inAttVal)
            fDocHandler->startEntityReference(*decl);

        // If it starts with the XML string, then it's an error
        if (checkXMLDecl(true)) {
            emitError(XMLErrs::TextDeclNotLegalHere);
            fReaderMgr.skipPastChar(chCloseAngle);
        }
    }
    return EntityExp_Pushed;
}


XERCES_CPP_NAMESPACE_END
