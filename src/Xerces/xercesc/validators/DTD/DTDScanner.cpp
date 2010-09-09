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
 * $Id: DTDScanner.cpp 833045 2009-11-05 13:21:27Z borisk $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/BinMemInputStream.hpp>
#include <xercesc/util/FlagJanitor.hpp>
#include <xercesc/util/Janitor.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/ValueStackOf.hpp>
#include <xercesc/util/UnexpectedEOFException.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/sax/InputSource.hpp>
#include <xercesc/framework/XMLDocumentHandler.hpp>
#include <xercesc/framework/XMLEntityHandler.hpp>
#include <xercesc/framework/XMLValidator.hpp>
#include <xercesc/internal/EndOfEntityException.hpp>
#include <xercesc/internal/XMLScanner.hpp>
#include <xercesc/validators/common/ContentSpecNode.hpp>
#include <xercesc/validators/common/MixedContentModel.hpp>
#include <xercesc/validators/DTD/DTDEntityDecl.hpp>
#include <xercesc/validators/DTD/DocTypeHandler.hpp>
#include <xercesc/validators/DTD/DTDScanner.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Local methods
// ---------------------------------------------------------------------------
//
//  This method automates the grunt work of looking at a char and see if its
//  a repetition suffix. If so, it creates a new correct rep node and wraps
//  the pass node in it. Otherwise, it returns the previous node.
//
static ContentSpecNode* makeRepNode(const XMLCh testCh,
                                    ContentSpecNode* const prevNode,
                                    MemoryManager* const manager)
{
    if (testCh == chQuestion)
    {
        return new (manager) ContentSpecNode
        (
            ContentSpecNode::ZeroOrOne
            , prevNode
            , 0
            , true
            , true
            , manager
        );
    }
     else if (testCh == chPlus)
    {
        return new (manager) ContentSpecNode
        (
            ContentSpecNode::OneOrMore
            , prevNode
            , 0
            , true
            , true
            , manager
        );
    }
     else if (testCh == chAsterisk)
    {
        return new (manager) ContentSpecNode
        (
            ContentSpecNode::ZeroOrMore
            , prevNode
            , 0
            , true
            , true
            , manager
        );
    }

    // Just return the incoming node
    return prevNode;
}

// ---------------------------------------------------------------------------
//  DTDValidator: Constructors and Destructor
// ---------------------------------------------------------------------------
DTDScanner::DTDScanner( DTDGrammar*           dtdGrammar
                      , DocTypeHandler* const docTypeHandler
                      , MemoryManager* const  grammarPoolMemoryManager
                      , MemoryManager* const  manager) :
    fMemoryManager(manager)
    , fGrammarPoolMemoryManager(grammarPoolMemoryManager)
    , fDocTypeHandler(docTypeHandler)
    , fDumAttDef(0)
    , fDumElemDecl(0)
    , fDumEntityDecl(0)
    , fInternalSubset(false)
    , fNextAttrId(1)
    , fDTDGrammar(dtdGrammar)
    , fBufMgr(0)
    , fReaderMgr(0)
    , fScanner(0)
    , fPEntityDeclPool(0)
    , fEmptyNamespaceId(0)
    , fDocTypeReaderId(0)
{
    fPEntityDeclPool = new (fMemoryManager) NameIdPool<DTDEntityDecl>(109, 128, fMemoryManager);
}

DTDScanner::~DTDScanner()
{
    delete fDumAttDef;
    delete fDumElemDecl;
    delete fDumEntityDecl;
    delete fPEntityDeclPool;
}

// -----------------------------------------------------------------------
//  Setter methods
// -----------------------------------------------------------------------
void DTDScanner::setScannerInfo(XMLScanner* const      owningScanner
                            , ReaderMgr* const      readerMgr
                            , XMLBufferMgr* const   bufMgr)
{
    // We don't own any of these, we just reference them
    fScanner = owningScanner;
    fReaderMgr = readerMgr;
    fBufMgr = bufMgr;

    if (fScanner->getDoNamespaces())
        fEmptyNamespaceId = fScanner->getEmptyNamespaceId();
    else
        fEmptyNamespaceId = 0;

    fDocTypeReaderId = fReaderMgr->getCurrentReaderNum();
}


// ---------------------------------------------------------------------------
//  DTDScanner: Private scanning methods
// ---------------------------------------------------------------------------
bool DTDScanner::checkForPERef(   const bool    inLiteral
                                , const bool    inMarkup)
{
    bool gotSpace = false;

    //
    //  See if we have any spaces up front. If so, then skip them and set
    //  the gotSpaces flag.
    //
    if (fReaderMgr->skippedSpace())
    {
        fReaderMgr->skipPastSpaces();
        gotSpace = true;
    }

    // If the next char is a percent, then expand the PERef
    if (!fReaderMgr->skippedChar(chPercent))
       return gotSpace;

    while (true)
    {
       if (!expandPERef(false, inLiteral, inMarkup, false))
          fScanner->emitError(XMLErrs::ExpectedEntityRefName);
       // And skip any more spaces in the expanded value
       if (fReaderMgr->skippedSpace())
       {
          fReaderMgr->skipPastSpaces();
          gotSpace = true;
       }
       if (!fReaderMgr->skippedChar(chPercent))
          break;
    }
    return gotSpace;
}


bool DTDScanner::expandPERef( const   bool    scanExternal
                                , const bool    inLiteral
                                , const bool    inMarkup
                                , const bool    throwEndOfExt)
{
    fScanner->setHasNoDTD(false);
    XMLBufBid bbName(fBufMgr);

    //
    //  If we are in the internal subset and in markup, then this is
    //  an error but we go ahead and do it anyway.
    //
    if (fInternalSubset && inMarkup)
        fScanner->emitError(XMLErrs::PERefInMarkupInIntSubset);

    if (!fReaderMgr->getName(bbName.getBuffer()))
    {
        fScanner->emitError(XMLErrs::ExpectedPEName);

        // Skip the semicolon if that's what we ended up on
        fReaderMgr->skippedChar(chSemiColon);
        return false;
    }

    // If no terminating semicolon, emit an error but try to keep going
    if (!fReaderMgr->skippedChar(chSemiColon))
        fScanner->emitError(XMLErrs::UnterminatedEntityRef, bbName.getRawBuffer());

    //
    //  Look it up in the PE decl pool and see if it exists. If not, just
    //  emit an error and continue.
    //
    XMLEntityDecl* decl = fPEntityDeclPool->getByKey(bbName.getRawBuffer());
    if (!decl)
    {
        // XML 1.0 Section 4.1
        if (fScanner->getStandalone()) {
            // no need to check fScanner->fHasNoDTD which is for sure false
            // since we are in expandPERef already
            fScanner->emitError(XMLErrs::EntityNotFound, bbName.getRawBuffer());
        }
        else {
            if (fScanner->getValidationScheme() == XMLScanner::Val_Always)
                fScanner->getValidator()->emitError(XMLValid::VC_EntityNotFound, bbName.getRawBuffer());
        }

        return false;
    }

    //
    // XML 1.0 Section 2.9
    //  If we are a standalone document, then it has to have been declared
    //  in the internal subset. Keep going though.
    //
    if (fScanner->getValidationScheme() == XMLScanner::Val_Always && fScanner->getStandalone() && !decl->getDeclaredInIntSubset())
        fScanner->getValidator()->emitError(XMLValid::VC_IllegalRefInStandalone, bbName.getRawBuffer());

    //
    //  Okee dokee, we found it. So create either a memory stream with
    //  the entity value contents, or a file stream if its an external
    //  entity.
    //
    if (decl->isExternal())
    {
        // And now create a reader to read this entity
        InputSource* srcUsed;
        XMLReader* reader = fReaderMgr->createReader
        (
            decl->getBaseURI()
            , decl->getSystemId()
            , decl->getPublicId()
            , false
            , inLiteral ? XMLReader::RefFrom_Literal : XMLReader::RefFrom_NonLiteral
            , XMLReader::Type_PE
            , XMLReader::Source_External
            , srcUsed
            , fScanner->getCalculateSrcOfs()
            , fScanner->getLowWaterMark()
            , fScanner->getDisableDefaultEntityResolution()
        );

        // Put a janitor on the source so its cleaned up on exit
        Janitor<InputSource> janSrc(srcUsed);

        // If the creation failed then throw an exception
        if (!reader)
            ThrowXMLwithMemMgr1(RuntimeException, XMLExcepts::Gen_CouldNotOpenExtEntity, srcUsed ? srcUsed->getSystemId() : decl->getSystemId(), fMemoryManager);

        // Set the 'throw at end' flag, to the one we were given
        reader->setThrowAtEnd(throwEndOfExt);

        //
        //  Push the reader. If its a recursive expansion, then emit an error
        //  and return an failure.
        //
        if (!fReaderMgr->pushReader(reader, decl))
        {
            fScanner->emitError(XMLErrs::RecursiveEntity, decl->getName());
            return false;
        }

        //
        //  If the caller wants us to scan the external entity, then lets
        //  do that now.
        //
        if (scanExternal)
        {
            XMLEntityHandler* entHandler = fScanner->getEntityHandler();

            // If we have an entity handler, tell it we are starting this entity
            if (entHandler)
                entHandler->startInputSource(*srcUsed);

            //
            //  Scan the external entity now. The parameter tells it that
            //  it is not in an include section. Get the current reader
            //  level so we can catch partial markup errors and be sure
            //  to get back to here if we get an exception out of the
            //  ext subset scan.
            //
            const XMLSize_t readerNum = fReaderMgr->getCurrentReaderNum();
            try
            {
                scanExtSubsetDecl(false, false);
            }
            catch(const OutOfMemoryException&)
            {
                throw;
            }
            catch(...)
            {
                // Pop the reader back to the original level
                fReaderMgr->cleanStackBackTo(readerNum);

                // End the input source, even though its not happy
                if (entHandler)
                    entHandler->endInputSource(*srcUsed);
                throw;
            }

            // If we have an entity handler, tell it we are ending this entity
            if (entHandler)
                entHandler->endInputSource(*srcUsed);
        }
        else {
            // If it starts with the XML string, then parse a text decl
            if (fScanner->checkXMLDecl(true))
                scanTextDecl();
        }
    }
     else
    {
        // Create a reader over a memory stream over the entity value
        XMLReader* valueReader = fReaderMgr->createIntEntReader
        (
            decl->getName()
            , inLiteral ? XMLReader::RefFrom_Literal : XMLReader::RefFrom_NonLiteral
            , XMLReader::Type_PE
            , decl->getValue()
            , decl->getValueLen()
            , false
        );

        //
        //  Trt to push the entity reader onto the reader manager stack,
        //  where it will become the subsequent input. If it fails, that
        //  means the entity is recursive, so issue an error. The reader
        //  will have just been discarded, but we just keep going.
        //
        if (!fReaderMgr->pushReader(valueReader, decl))
            fScanner->emitError(XMLErrs::RecursiveEntity, decl->getName());
    }

    return true;
}


bool DTDScanner::getQuotedString(XMLBuffer& toFill)
{
    // Reset the target buffer
    toFill.reset();

    // Get the next char which must be a single or double quote
    XMLCh quoteCh;
    if (!fReaderMgr->skipIfQuote(quoteCh))
        return false;

	XMLCh nextCh;
    // Get another char and see if it matches the starting quote char
    while ((nextCh=fReaderMgr->getNextChar())!=quoteCh)
    {
        //
        //  We should never get either an end of file null char here. If we
        //  do, just fail. It will be handled more gracefully in the higher
        //  level code that called us.
        //
        if (!nextCh)
            return false;

        // Else add it to the buffer
        toFill.append(nextCh);
    }
    return true;
}


XMLAttDef*
DTDScanner::scanAttDef(DTDElementDecl& parentElem, XMLBuffer& bufToUse)
{
    // Check for PE ref or optional whitespace
    checkForPERef(false, true);

    // Get the name of the attribute
    if (!fReaderMgr->getName(bufToUse))
    {
        fScanner->emitError(XMLErrs::ExpectedAttrName);
        return 0;
    }

    //
    //  Look up this attribute in the parent element's attribute list. If
    //  it already exists, then use the dummy.
    //
    DTDAttDef* decl = parentElem.getAttDef(bufToUse.getRawBuffer());
    if (decl)
    {
        // It already exists, so put out a warning
        fScanner->emitError
        (
            XMLErrs::AttListAlreadyExists
            , bufToUse.getRawBuffer()
            , parentElem.getFullName()
        );

        // Use the dummy decl to parse into and set its name to the name we got
        if (!fDumAttDef)
        {
            fDumAttDef = new (fMemoryManager) DTDAttDef(fMemoryManager);
            fDumAttDef->setId(fNextAttrId++);
        }
        fDumAttDef->setName(bufToUse.getRawBuffer());
        decl = fDumAttDef;
    }
     else
    {
        //
        //  It does not already exist so create a new one, give it the next
        //  available unique id, and add it
        //
        decl = new (fGrammarPoolMemoryManager) DTDAttDef
        (
            bufToUse.getRawBuffer()
            , XMLAttDef::CData
            , XMLAttDef::Implied
            , fGrammarPoolMemoryManager
        );
        decl->setId(fNextAttrId++);
        decl->setExternalAttDeclaration(isReadingExternalEntity());
        parentElem.addAttDef(decl);
    }

    // Set a flag to indicate whether we are doing a dummy parse
    const bool isIgnored = (decl == fDumAttDef);

    // Space is required here, so check for PE ref, and require space
    if (!checkForPERef(false, true))
        fScanner->emitError(XMLErrs::ExpectedWhitespace);

    //
    //  Next has to be one of the attribute type strings. This tells us what
    //  is to follow.
    //
    if (fReaderMgr->skippedString(XMLUni::fgCDATAString))
    {
        decl->setType(XMLAttDef::CData);
    }
     else if (fReaderMgr->skippedString(XMLUni::fgIDString))
    {
        if (!fReaderMgr->skippedString(XMLUni::fgRefString))
            decl->setType(XMLAttDef::ID);
        else if (!fReaderMgr->skippedChar(chLatin_S))
            decl->setType(XMLAttDef::IDRef);
        else
            decl->setType(XMLAttDef::IDRefs);
    }
     else if (fReaderMgr->skippedString(XMLUni::fgEntitString))
    {
        if (fReaderMgr->skippedChar(chLatin_Y))
        {
            decl->setType(XMLAttDef::Entity);
        }
         else if (fReaderMgr->skippedString(XMLUni::fgIESString))
        {
            decl->setType(XMLAttDef::Entities);
        }
         else
        {
            fScanner->emitError
            (
                XMLErrs::ExpectedAttributeType
                , decl->getFullName()
                , parentElem.getFullName()
            );
            return 0;
        }
    }
     else if (fReaderMgr->skippedString(XMLUni::fgNmTokenString))
    {
        if (fReaderMgr->skippedChar(chLatin_S))
            decl->setType(XMLAttDef::NmTokens);
        else
            decl->setType(XMLAttDef::NmToken);
    }
     else if (fReaderMgr->skippedString(XMLUni::fgNotationString))
    {
        // Check for PE ref and require space
        if (!checkForPERef(false, true))
            fScanner->emitError(XMLErrs::ExpectedWhitespace);

        decl->setType(XMLAttDef::Notation);
        if (!scanEnumeration(*decl, bufToUse, true))
            return 0;

        // Set the value as the enumeration for this decl
        decl->setEnumeration(bufToUse.getRawBuffer());
    }
     else if (fReaderMgr->skippedChar(chOpenParen))
    {
        decl->setType(XMLAttDef::Enumeration);
        if (!scanEnumeration(*decl, bufToUse, false))
            return 0;

        // Set the value as the enumeration for this decl
        decl->setEnumeration(bufToUse.getRawBuffer());
    }
     else
    {
        fScanner->emitError
        (
            XMLErrs::ExpectedAttributeType
            , decl->getFullName()
            , parentElem.getFullName()
        );
        return 0;
    }

    // Space is required here, so check for PE ref, and require space
    if (!checkForPERef(false, true))
        fScanner->emitError(XMLErrs::ExpectedWhitespace);

    // And then scan for the optional default value declaration
    scanDefaultDecl(*decl);

    // If validating, then do a couple of validation constraints
    if (fScanner->getValidationScheme() == XMLScanner::Val_Always)
    {
        if (decl->getType() == XMLAttDef::ID)
        {
            if ((decl->getDefaultType() != XMLAttDef::Implied)
            &&  (decl->getDefaultType() != XMLAttDef::Required))
            {
                fScanner->getValidator()->emitError(XMLValid::BadIDAttrDefType, decl->getFullName());
            }
        }

        // if attdef is xml:space, check correct enumeration (default|preserve)
        const XMLCh fgXMLSpace[] = { chLatin_x, chLatin_m, chLatin_l, chColon, chLatin_s, chLatin_p, chLatin_a, chLatin_c, chLatin_e, chNull };

        if (XMLString::equals(decl->getFullName(),fgXMLSpace)) {
            const XMLCh fgPreserve[] = { chLatin_p, chLatin_r, chLatin_e, chLatin_s, chLatin_e, chLatin_r, chLatin_v, chLatin_e, chNull };
            const XMLCh fgDefault[] = { chLatin_d, chLatin_e, chLatin_f, chLatin_a, chLatin_u, chLatin_l, chLatin_t, chNull };
            bool ok = false;
            if (decl->getType() == XMLAttDef::Enumeration) {
                BaseRefVectorOf<XMLCh>* enumVector = XMLString::tokenizeString(decl->getEnumeration(), fMemoryManager);
                XMLSize_t size = enumVector->size();
                ok = (size == 1 &&
                     (XMLString::equals(enumVector->elementAt(0), fgDefault) ||
                      XMLString::equals(enumVector->elementAt(0), fgPreserve))) ||
                     (size == 2 &&
                     (XMLString::equals(enumVector->elementAt(0), fgDefault) &&
                      XMLString::equals(enumVector->elementAt(1), fgPreserve))) ||
                     (size == 2 &&
                     (XMLString::equals(enumVector->elementAt(1), fgDefault) &&
                      XMLString::equals(enumVector->elementAt(0), fgPreserve)));
                delete enumVector;
            }
            if (!ok)
                fScanner->getValidator()->emitError(XMLValid::IllegalXMLSpace);
        }
    }

    // If we have a doc type handler, tell it about this attdef.
    if (fDocTypeHandler)
        fDocTypeHandler->attDef(parentElem, *decl, isIgnored);
    return decl;
}


void DTDScanner::scanAttListDecl()
{
    // Space is required here, so check for a PE ref
    if (!checkForPERef(false, true))
    {
        fScanner->emitError(XMLErrs::ExpectedWhitespace);
        fReaderMgr->skipPastChar(chCloseAngle);
        return;
    }

    //
    //  Next should be the name of the element it belongs to, so get a buffer
    //  and get the name into it.
    //
    XMLBufBid bbName(fBufMgr);
    if (!fReaderMgr->getName(bbName.getBuffer()))
    {
        fScanner->emitError(XMLErrs::ExpectedElementName);
        fReaderMgr->skipPastChar(chCloseAngle);
        return;
    }

    //
    //  Find this element's declaration. If it has not been declared yet,
    //  we will force one into the list, but not mark it as declared.
    //
    DTDElementDecl* elemDecl = (DTDElementDecl*) fDTDGrammar->getElemDecl(fEmptyNamespaceId, 0, bbName.getRawBuffer(), Grammar::TOP_LEVEL_SCOPE);
    if (!elemDecl)
    {
        //
        //  Lets fault in a declaration and add it to the pool. We mark
        //  it having been created because of an attlist. Later, if its
        //  declared, this will be updated.
        //
        elemDecl = new (fGrammarPoolMemoryManager) DTDElementDecl
        (
            bbName.getRawBuffer()
            , fEmptyNamespaceId
            , DTDElementDecl::Any
            , fGrammarPoolMemoryManager
        );
        elemDecl->setCreateReason(XMLElementDecl::AttList);
        elemDecl->setExternalElemDeclaration(isReadingExternalEntity());
        fDTDGrammar->putElemDecl((XMLElementDecl*) elemDecl);
    }

    // If we have a doc type handler, tell it the att list is starting
    if (fDocTypeHandler)
        fDocTypeHandler->startAttList(*elemDecl);

    //
    //  Now we loop until we are done with all of the attributes in this
    //  list. We need a buffer to use for local processing.
    //
    XMLBufBid   bbTmp(fBufMgr);
    XMLBuffer&  tmpBuf = bbTmp.getBuffer();
    bool        seenAnId = false;
    while (true)
    {
        // Get the next char out and see what it tells us to do
        const XMLCh nextCh = fReaderMgr->peekNextChar();

        // Watch for EOF
        if (!nextCh)
            ThrowXMLwithMemMgr(UnexpectedEOFException, XMLExcepts::Gen_UnexpectedEOF, fMemoryManager);

        if (nextCh == chCloseAngle)
        {
            // We are done with this attribute list
            fReaderMgr->getNextChar();
            break;
        }
         else if (fReaderMgr->getCurrentReader()->isWhitespace(nextCh))
        {
            //
            //  If advanced callbacks are enabled and we have a doc
            //  type handler, then gather up the white space and call
            //  back on the doctype handler. Otherwise, just skip
            //  whitespace.
            //
            if (fDocTypeHandler)
            {
                fReaderMgr->getSpaces(tmpBuf);
                fDocTypeHandler->doctypeWhitespace
                (
                    tmpBuf.getRawBuffer()
                    , tmpBuf.getLen()
                );
            }
             else
            {
                fReaderMgr->skipPastSpaces();
            }
        }
         else if (nextCh == chPercent)
        {
            // Eat the percent and expand the ref
            fReaderMgr->getNextChar();
            expandPERef(false, false, true);
        }
         else
        {
            //
            //  It must be an attribute name, so scan it. We let
            //  it use our local buffer for its name scanning.
            //
            XMLAttDef* attDef = scanAttDef(*elemDecl, tmpBuf);

            if (!attDef)
            {
                fReaderMgr->skipPastChar(chCloseAngle);
                break;
            }

            //
            //  If we are validating and its an ID type, then we have to
            //  make sure that we have not seen an id attribute yet. Set
            //  the flag to say that we've seen one now also.
            //
            if (fScanner->getValidationScheme() == XMLScanner::Val_Always)
            {
                if (attDef->getType() == XMLAttDef::ID)
                {
                    if (seenAnId)
                        fScanner->getValidator()->emitError(XMLValid::MultipleIdAttrs, elemDecl->getFullName());
                    seenAnId = true;
                }
            }
        }
    }

    // If we have a doc type handler, tell it the att list is ending
    if (fDocTypeHandler)
        fDocTypeHandler->endAttList(*elemDecl);
}


//
//  This method is called to scan the value of an attribute in content. This
//  involves some normalization and replacement of general entity and
//  character references.
//
//  End of entity's must be dealt with here. During DTD scan, they can come
//  from external entities. During content, they can come from any entity.
//  We just eat the end of entity and continue with our scan until we come
//  to the closing quote. If an unterminated value causes us to go through
//  subsequent entities, that will cause errors back in the calling code,
//  but there's little we can do about it here.
//
bool DTDScanner::scanAttValue(const   XMLCh* const        attrName
                                ,       XMLBuffer&          toFill
                                , const XMLAttDef::AttTypes type)
{
    enum States
    {
        InWhitespace
        , InContent
    };

    // Reset the target buffer
    toFill.reset();

    // Get the next char which must be a single or double quote
    XMLCh quoteCh;
    if (!fReaderMgr->skipIfQuote(quoteCh))
        return false;

    //
    //  We have to get the current reader because we have to ignore closing
    //  quotes until we hit the same reader again.
    //
    const XMLSize_t curReader = fReaderMgr->getCurrentReaderNum();

    //
    //  Loop until we get the attribute value. Note that we use a double
    //  loop here to avoid the setup/teardown overhead of the exception
    //  handler on every round.
    //
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
            nextCh = fReaderMgr->getNextChar();

            if (!nextCh)
                ThrowXMLwithMemMgr(UnexpectedEOFException, XMLExcepts::Gen_UnexpectedEOF, fMemoryManager);

            // Check for our ending quote in the same entity
            if (nextCh == quoteCh)
            {
                if (curReader == fReaderMgr->getCurrentReaderNum())
                    return true;

                // Watch for spillover into a previous entity
                if (curReader > fReaderMgr->getCurrentReaderNum())
                {
                    fScanner->emitError(XMLErrs::PartialMarkupInEntity);
                    return false;
                }
            }

            //
            //  Check for an entity ref now, before we let it affect our
            //  whitespace normalization logic below. We ignore the empty flag
            //  in this one.
            //
            escaped = false;
            if (nextCh == chAmpersand)
            {
                if (scanEntityRef(nextCh, secondCh, escaped) != EntityExp_Returned)
                {
                    gotLeadingSurrogate = false;
                    continue;
                }
            }
            else if ((nextCh >= 0xD800) && (nextCh <= 0xDBFF))
            {
                // Check for correct surrogate pairs
                if (gotLeadingSurrogate)
                    fScanner->emitError(XMLErrs::Expected2ndSurrogateChar);
                else
                    gotLeadingSurrogate = true;
            }
             else
            {
                if (gotLeadingSurrogate)
                {
                    if ((nextCh < 0xDC00) || (nextCh > 0xDFFF))
                        fScanner->emitError(XMLErrs::Expected2ndSurrogateChar);
                }
                // Its got to at least be a valid XML character
                else if (!fReaderMgr->getCurrentReader()->isXMLChar(nextCh))
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
                    fScanner->emitError
                    (
                        XMLErrs::InvalidCharacterInAttrValue
                        , attrName
                        , tmpBuf
                    );
                }

                gotLeadingSurrogate = false;
            }

            //
            //  If its not escaped, then make sure its not a < character, which
            //  is not allowed in attribute values.
            //
            if (!escaped && (nextCh == chOpenAngle))
                fScanner->emitError(XMLErrs::BracketInAttrValue, attrName);

            //
            //  If the attribute is a CDATA type we do simple replacement of
            //  tabs and new lines with spaces, if the character is not escaped
            //  by way of a char ref.
            //
            //  Otherwise, we do the standard non-CDATA normalization of
            //  compressing whitespace to single spaces and getting rid of
            //  leading and trailing whitespace.
            //
            if (type == XMLAttDef::CData)
            {
                if (!escaped)
                {
                    if ((nextCh == 0x09) || (nextCh == 0x0A) || (nextCh == 0x0D))
                        nextCh = chSpace;
                }
            }
             else
            {
                if (curState == InWhitespace)
                {
                    if (!fReaderMgr->getCurrentReader()->isWhitespace(nextCh))
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
                    if (fReaderMgr->getCurrentReader()->isWhitespace(nextCh))
                    {
                        curState = InWhitespace;
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


bool DTDScanner::scanCharRef(XMLCh& first, XMLCh& second)
{
    bool gotOne = false;
    unsigned int value = 0;

    //
    //  Set the radix. Its supposed to be a lower case x if hex. But, in
    //  order to recover well, we check for an upper and put out an error
    //  for that.
    //
    unsigned int radix = 10;

    if (fReaderMgr->skippedChar(chLatin_x))
    {
        radix = 16;
    }
     else if (fReaderMgr->skippedChar(chLatin_X))
    {
        fScanner->emitError(XMLErrs::HexRadixMustBeLowerCase);
        radix = 16;
    }

    while (true)
    {
        const XMLCh nextCh = fReaderMgr->peekNextChar();

        // Watch for EOF
        if (!nextCh)
            ThrowXMLwithMemMgr(UnexpectedEOFException, XMLExcepts::Gen_UnexpectedEOF, fMemoryManager);

        // Break out on the terminating semicolon
        if (nextCh == chSemiColon)
        {
            fReaderMgr->getNextChar();
            break;
        }

        //
        //  Convert this char to a binary value, or bail out if its not
        //  one.
        //
        unsigned int nextVal;
        if ((nextCh >= chDigit_0) && (nextCh <= chDigit_9))
            nextVal = (unsigned int)(nextCh - chDigit_0);
        else if ((nextCh >= chLatin_A) && (nextCh <= chLatin_F))
            nextVal= (unsigned int)(10 + (nextCh - chLatin_A));
        else if ((nextCh >= chLatin_a) && (nextCh <= chLatin_f))
            nextVal = (unsigned int)(10 + (nextCh - chLatin_a));
        else
        {
            //
            //  If we got at least a sigit, then do an unterminated ref
            //  error. Else, do an expected a numerical ref thing.
            //
            if (gotOne)
                fScanner->emitError(XMLErrs::UnterminatedCharRef);
            else
                fScanner->emitError(XMLErrs::ExpectedNumericalCharRef);

            return false;
        }

        //
        //  Make sure its valid for the radix. If not, then just eat the
        //  digit and go on after issueing an error. Else, update the
        //  running value with this new digit.
        //
        if (nextVal >= radix)
        {
            XMLCh tmpStr[2];
            tmpStr[0] = nextCh;
            tmpStr[1] = chNull;
            fScanner->emitError(XMLErrs::BadDigitForRadix, tmpStr);
        }
         else
        {
            value = (value * radix) + nextVal;
        }

        // Indicate that we got at least one good digit
        gotOne = true;

        // Eat the char we just processed
        fReaderMgr->getNextChar();
    }

    // Return the char (or chars)
    // And check if the character expanded is valid or not
    if (value >= 0x10000 && value <= 0x10FFFF)
    {
        value -= 0x10000;
        first  = XMLCh((value >> 10) + 0xD800);
        second = XMLCh((value & 0x3FF) + 0xDC00);
    }
    else if (value <= 0xFFFD)
    {
        first  = XMLCh(value);
        second = 0;
        if (!fReaderMgr->getCurrentReader()->isXMLChar(first) && !fReaderMgr->getCurrentReader()->isControlChar(first)) {
            // Character reference was not in the valid range
            fScanner->emitError(XMLErrs::InvalidCharacterRef);
            return false;
        }
    }
    else {
        // Character reference was not in the valid range
        fScanner->emitError(XMLErrs::InvalidCharacterRef);
        return false;
    }

    return true;
}


ContentSpecNode*
DTDScanner::scanChildren(const DTDElementDecl& elemDecl, XMLBuffer& bufToUse)
{
    // Check for a PE ref here, but don't require spaces
    checkForPERef(false, true);

    ValueStackOf<XMLSize_t>* arrNestedDecl=NULL;
    //
    //  We know that the caller just saw an opening parenthesis, so we need
    //  to parse until we hit the end of it; if we find several parenthesis,
    //  store them in an array to be processed later.
    //
    //  We have to check for one up front, since it could be something like
    //  (((a)*)) etc...
    //
    ContentSpecNode* curNode = 0;
    while(fReaderMgr->skippedChar(chOpenParen))
    {
        // to check entity nesting
        const XMLSize_t curReader = fReaderMgr->getCurrentReaderNum();
        if(arrNestedDecl==NULL)
            arrNestedDecl=new (fMemoryManager) ValueStackOf<XMLSize_t>(5, fMemoryManager);
        arrNestedDecl->push(curReader);

        // Check for a PE ref here, but don't require spaces
        checkForPERef(false, true);
    }

    // We must find a leaf node here, either standalone or nested in the parenthesis
    if (!fReaderMgr->getName(bufToUse))
    {
        fScanner->emitError(XMLErrs::ExpectedElementName);
        return 0;
    }

    //
    //  Create a leaf node for it. If we can find the element id for
    //  this element, then use it. Else, we have to fault in an element
    //  decl, marked as created because of being in a content model.
    //
    XMLElementDecl* decl = fDTDGrammar->getElemDecl(fEmptyNamespaceId, 0, bufToUse.getRawBuffer(), Grammar::TOP_LEVEL_SCOPE);
    if (!decl)
    {
        decl = new (fGrammarPoolMemoryManager) DTDElementDecl
        (
            bufToUse.getRawBuffer()
            , fEmptyNamespaceId
            , DTDElementDecl::Any
            , fGrammarPoolMemoryManager
        );
        decl->setCreateReason(XMLElementDecl::InContentModel);
        decl->setExternalElemDeclaration(isReadingExternalEntity());
        fDTDGrammar->putElemDecl(decl);
    }
    curNode = new (fGrammarPoolMemoryManager) ContentSpecNode
    (
        decl->getElementName()
        , fGrammarPoolMemoryManager
    );

    // Check for a PE ref here, but don't require spaces
    const bool gotSpaces = checkForPERef(false, true);

    // Check for a repetition character after the leaf
    XMLCh repCh = fReaderMgr->peekNextChar();
    ContentSpecNode* tmpNode = makeRepNode(repCh, curNode, fGrammarPoolMemoryManager);
    if (tmpNode != curNode)
    {
        if (gotSpaces)
        {
            if (fScanner->emitErrorWillThrowException(XMLErrs::UnexpectedWhitespace))
            {
                delete tmpNode;
            }
            fScanner->emitError(XMLErrs::UnexpectedWhitespace);
        }
        fReaderMgr->getNextChar();
        curNode = tmpNode;
    }

    while(arrNestedDecl==NULL || !arrNestedDecl->empty())
    {
        // Check for a PE ref here, but don't require spaces
        checkForPERef(false, true);

        //
        //  Ok, the next character tells us what kind of content this particular
        //  model this particular parentesized section is. Its either a choice if
        //  we see ',', a sequence if we see '|', or a single leaf node if we see
        //  a closing paren.
        //
        const XMLCh opCh = fReaderMgr->peekNextChar();

        if ((opCh != chComma)
        &&  (opCh != chPipe)
        &&  (opCh != chCloseParen))
        {
            // Not a legal char, so delete our node and return failure
            delete curNode;
            fScanner->emitError(XMLErrs::ExpectedSeqChoiceLeaf);
            return 0;
        }

        //
        //  Create the head node of the correct type. We need this to remember
        //  the top of the local tree. If it was a single subexpr, then just
        //  set the head node to the current node. For the others, we'll build
        //  the tree off the second child as we move across.
        //
        ContentSpecNode* headNode = 0;
        ContentSpecNode::NodeTypes curType = ContentSpecNode::UnknownType;
        if (opCh == chComma)
        {
            curType = ContentSpecNode::Sequence;
            headNode = new (fGrammarPoolMemoryManager) ContentSpecNode
            (
                curType
                , curNode
                , 0
                , true
                , true
                , fGrammarPoolMemoryManager
            );
            curNode = headNode;
        }
         else if (opCh == chPipe)
        {
            curType = ContentSpecNode::Choice;
            headNode = new (fGrammarPoolMemoryManager) ContentSpecNode
            (
                curType
                , curNode
                , 0
                , true
                , true
                , fGrammarPoolMemoryManager
            );
            curNode = headNode;
        }
         else
        {
            headNode = curNode;
            fReaderMgr->getNextChar();
        }

        //
        //  If it was a sequence or choice, we just loop until we get to the
        //  end of our section, adding each new leaf or sub expression to the
        //  right child of the current node, and making that new node the current
        //  node.
        //
        if ((opCh == chComma) || (opCh == chPipe))
        {
            ContentSpecNode* lastNode = 0;
            while (true)
            {
                //
                //  The next thing must either be another | or , character followed
                //  by another leaf or subexpression, or a closing parenthesis, or a
                //  PE ref.
                //
                if (fReaderMgr->lookingAtChar(chPercent))
                {
                    checkForPERef(false, true);
                }
                 else if (fReaderMgr->skippedSpace())
                {
                    // Just skip whitespace
                    fReaderMgr->skipPastSpaces();
                }
                 else if (fReaderMgr->skippedChar(chCloseParen))
                {
                    //
                    //  We've hit the end of this section, so break out. But, we
                    //  need to see if we left a partial sequence of choice node
                    //  without a second node. If so, we have to undo that and
                    //  put its left child into the right node of the previous
                    //  node.
                    //
                    if ((curNode->getType() == ContentSpecNode::Choice)
                    ||  (curNode->getType() == ContentSpecNode::Sequence))
                    {
                        if (!curNode->getSecond())
                        {
                            ContentSpecNode* saveFirst = curNode->orphanFirst();
                            lastNode->setSecond(saveFirst);
                            curNode = lastNode;
                        }
                    }
                    break;
                }
                 else if (fReaderMgr->skippedChar(opCh))
                {
                    // Check for a PE ref here, but don't require spaces
                    checkForPERef(false, true);

                    if (fReaderMgr->skippedChar(chOpenParen))
                    {
                        const XMLSize_t curReader = fReaderMgr->getCurrentReaderNum();

                        // Recurse to handle this new guy
                        ContentSpecNode* subNode;
                        try {
                            subNode = scanChildren(elemDecl, bufToUse);
                        }
                        catch (const XMLErrs::Codes)
                        {
                            delete headNode;
                            throw;
                        }

                        // If it failed, we are done, clean up here and return failure
                        if (!subNode)
                        {
                            delete headNode;
                            return 0;
                        }

                        if (curReader != fReaderMgr->getCurrentReaderNum() && fScanner->getValidationScheme() == XMLScanner::Val_Always)
                            fScanner->getValidator()->emitError(XMLValid::PartialMarkupInPE);

                        // Else patch it in and make it the new current
                        ContentSpecNode* newCur = new (fGrammarPoolMemoryManager) ContentSpecNode
                        (
                            curType
                            , subNode
                            , 0
                            , true
                            , true
                            , fGrammarPoolMemoryManager
                        );
                        curNode->setSecond(newCur);
                        lastNode = curNode;
                        curNode = newCur;
                    }
                     else
                    {
                        //
                        //  Got to be a leaf node, so get a name. If we cannot get
                        //  one, then clean up and get outa here.
                        //
                        if (!fReaderMgr->getName(bufToUse))
                        {
                            delete headNode;
                            fScanner->emitError(XMLErrs::ExpectedElementName);
                            return 0;
                        }

                        //
                        //  Create a leaf node for it. If we can find the element
                        //  id for this element, then use it. Else, we have to
                        //  fault in an element decl, marked as created because
                        //  of being in a content model.
                        //
                        XMLElementDecl* decl = fDTDGrammar->getElemDecl(fEmptyNamespaceId, 0, bufToUse.getRawBuffer(), Grammar::TOP_LEVEL_SCOPE);
                        if (!decl)
                        {
                            decl = new (fGrammarPoolMemoryManager) DTDElementDecl
                            (
                                bufToUse.getRawBuffer()
                                , fEmptyNamespaceId
                                , DTDElementDecl::Any
                                , fGrammarPoolMemoryManager
                            );
                            decl->setCreateReason(XMLElementDecl::InContentModel);
                            decl->setExternalElemDeclaration(isReadingExternalEntity());
                            fDTDGrammar->putElemDecl(decl);
                        }

                        ContentSpecNode* tmpLeaf = new (fGrammarPoolMemoryManager) ContentSpecNode
                        (
                            decl->getElementName()
                            , fGrammarPoolMemoryManager
                        );

                        // Check for a repetition character after the leaf
                        const XMLCh repCh = fReaderMgr->peekNextChar();
                        ContentSpecNode* tmpLeaf2 = makeRepNode(repCh, tmpLeaf, fGrammarPoolMemoryManager);
                        if (tmpLeaf != tmpLeaf2)
                            fReaderMgr->getNextChar();

                        //
                        //  Create a new sequence or choice node, with the leaf
                        //  (or rep surrounding it) we just got as its first node.
                        //  Make the new node the second node of the current node,
                        //  and then make it the current node.
                        //
                        ContentSpecNode* newCur = new (fGrammarPoolMemoryManager) ContentSpecNode
                        (
                            curType
                            , tmpLeaf2
                            , 0
                            , true
                            , true
                            , fGrammarPoolMemoryManager
                        );
                        curNode->setSecond(newCur);
                        lastNode = curNode;
                        curNode = newCur;
                    }
                }
                 else
                {
                    // Cannot be valid
                    delete headNode;  // emitError may do a throw so need to clean-up first
                    if (opCh == chComma)
                    {
                        fScanner->emitError(XMLErrs::ExpectedChoiceOrCloseParen);
                    }
                     else
                    {
                        fScanner->emitError
                        (
                            XMLErrs::ExpectedSeqOrCloseParen
                            , elemDecl.getFullName()
                        );
                    }                
                    return 0;
                }
            }
        }

        //
        //  We saw the terminating parenthesis so lets check for any repetition
        //  character, and create a node for that, making the head node the child
        //  of it.
        //
        const XMLCh repCh = fReaderMgr->peekNextChar();
        curNode = makeRepNode(repCh, headNode, fGrammarPoolMemoryManager);
        if (curNode != headNode)
            fReaderMgr->getNextChar();

        // prepare for recursion
        if(arrNestedDecl==NULL)
            break;
        else
        {
            // If that failed, no need to go further, return failure
            if (!curNode)
                return 0;

            const XMLSize_t curReader = arrNestedDecl->pop();
            if (curReader != fReaderMgr->getCurrentReaderNum() && fScanner->getValidationScheme() == XMLScanner::Val_Always)
                fScanner->getValidator()->emitError(XMLValid::PartialMarkupInPE);

            if(arrNestedDecl->empty())
            {
                delete arrNestedDecl;
                arrNestedDecl=NULL;
            }
        }
    }

    return curNode;
}


//
//  We get here after the '<!--' part of the comment. We scan past the
//  terminating '-->' It will calls the appropriate handler with the comment
//  text, if one is provided. A comment can be in either the document or
//  the DTD, so the fInDocument flag is used to know which handler to send
//  it to.
//
void DTDScanner::scanComment()
{
    enum States
    {
        InText
        , OneDash
        , TwoDashes
    };

    // Get a buffer for this
    XMLBufBid bbComment(fBufMgr);

    //
    //  Get the comment text into a temp buffer. Be sure to use temp buffer
    //  two here, since its to be used for stuff that is potentially longer
    //  than just a name.
    //
    bool   gotLeadingSurrogate = false;
    States curState = InText;
    while (true)
    {
        // Get the next character
        const XMLCh nextCh = fReaderMgr->getNextChar();

        //  Watch for an end of file
        if (!nextCh)
        {
            fScanner->emitError(XMLErrs::UnterminatedComment);
            ThrowXMLwithMemMgr(UnexpectedEOFException, XMLExcepts::Gen_UnexpectedEOF, fMemoryManager);
        }

        // Check for correct surrogate pairs
        if ((nextCh >= 0xD800) && (nextCh <= 0xDBFF))
        {
            if (gotLeadingSurrogate)
                fScanner->emitError(XMLErrs::Expected2ndSurrogateChar);
            else
                gotLeadingSurrogate = true;
        }
        else
        {
            if (gotLeadingSurrogate)
            {
                if ((nextCh < 0xDC00) || (nextCh > 0xDFFF))
                    fScanner->emitError(XMLErrs::Expected2ndSurrogateChar);
            }
            // Its got to at least be a valid XML character
            else if (!fReaderMgr->getCurrentReader()->isXMLChar(nextCh)) {

                XMLCh tmpBuf[9];
                XMLString::binToText
                (
                    nextCh
                    , tmpBuf
                    , 8
                    , 16
                    , fMemoryManager
                );
                fScanner->emitError(XMLErrs::InvalidCharacter, tmpBuf);
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
            //
            //  If its another dash, then we change to the two dashes states.
            //  Otherwise, we have to put in the deficit dash and the new
            //  character and go back to InText.
            //
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
                fScanner->emitError(XMLErrs::IllegalSequenceInComment);
                fReaderMgr->skipPastChar(chCloseAngle);
                return;
            }
            break;
        }
    }

    // If there is a doc type handler, then pass on the comment stuff
    if (fDocTypeHandler)
        fDocTypeHandler->doctypeComment(bbComment.getRawBuffer());
}


bool DTDScanner::scanContentSpec(DTDElementDecl& toFill)
{
    //
    //  Check for for a couple of the predefined content type strings. If
    //  its not one of these, its got to be a parenthesized reg ex type
    //  expression.
    //
    if (fReaderMgr->skippedString(XMLUni::fgEmptyString))
    {
        toFill.setModelType(DTDElementDecl::Empty);
        return true;
    }

    if (fReaderMgr->skippedString(XMLUni::fgAnyString))
    {
        toFill.setModelType(DTDElementDecl::Any);
        return true;
    }

    // Its got to be a parenthesized regular expression
    if (!fReaderMgr->skippedChar(chOpenParen))
    {
        fScanner->emitError
        (
            XMLErrs::ExpectedContentSpecExpr
            , toFill.getFullName()
        );
        return false;
    }

    // Get the current reader id, so we can test for partial markup
    const XMLSize_t curReader = fReaderMgr->getCurrentReaderNum();

    // We could have a PE ref here, but don't require space
    checkForPERef(false, true);

    //
    //  Now we look for a PCDATA string. If its PCDATA, then it must be a
    //  MIXED model. Otherwise, it must be a regular list of children in
    //  a regular expression perhaps.
    //
    bool status;
    if (fReaderMgr->skippedString(XMLUni::fgPCDATAString))
    {
        // Set the model to mixed
        toFill.setModelType(DTDElementDecl::Mixed_Simple);
        status = scanMixed(toFill);

        //
        //  If we are validating we have to check that there are no multiple
        //  uses of any child elements.
        //
        if (fScanner->getValidationScheme() == XMLScanner::Val_Always)
        {
            if (((const MixedContentModel*)toFill.getContentModel())->hasDups())
                fScanner->getValidator()->emitError(XMLValid::RepElemInMixed);
        }
    }
     else
    {
        //
        //  We have to do a recursive scan of the content model. Create a
        //  buffer for it to use, for efficiency. It returns the top ofthe
        //  content spec node tree, which we set if successful.
        //
        toFill.setModelType(DTDElementDecl::Children);
        XMLBufBid bbTmp(fBufMgr);
        ContentSpecNode* resNode = scanChildren(toFill, bbTmp.getBuffer());
        status = (resNode != 0);
        if (status)
            toFill.setContentSpec(resNode);
    }

    // Make sure we are on the same reader as where we started
    if (curReader != fReaderMgr->getCurrentReaderNum() && fScanner->getValidationScheme() == XMLScanner::Val_Always)
        fScanner->getValidator()->emitError(XMLValid::PartialMarkupInPE);

    return status;
}


void DTDScanner::scanDefaultDecl(DTDAttDef& toFill)
{
    if (fReaderMgr->skippedString(XMLUni::fgRequiredString))
    {
        toFill.setDefaultType(XMLAttDef::Required);
        return;
    }

    if (fReaderMgr->skippedString(XMLUni::fgImpliedString))
    {
        toFill.setDefaultType(XMLAttDef::Implied);
        return;
    }

    if (fReaderMgr->skippedString(XMLUni::fgFixedString))
    {
        //
        //  There must be space before the fixed value. If there is not, then
        //  emit an error but keep going.
        //
        if (!fReaderMgr->skippedSpace())
            fScanner->emitError(XMLErrs::ExpectedWhitespace);
        else
            fReaderMgr->skipPastSpaces();
        toFill.setDefaultType(XMLAttDef::Fixed);
    }
     else
    {
        toFill.setDefaultType(XMLAttDef::Default);
    }

    //
    //  If we got here, its fixed or default, so we need to get a value.
    //  If we don't, then emit an error but just set the default value to
    //  an empty string and try to keep going.
    //
    // Check for PE ref or optional whitespace
    checkForPERef(false, true);

    XMLBufBid bbValue(fBufMgr);
    if (!scanAttValue(toFill.getFullName(), bbValue.getBuffer(), toFill.getType()))
        fScanner->emitError(XMLErrs::ExpectedDefAttrDecl);

    toFill.setValue(bbValue.getRawBuffer());
}


//
//  This is called after seeing '<!ELEMENT' which indicates that an element
//  markup is starting. This guy scans the rest of it and adds it to the
//  element decl pool if it has not already been declared.
//
void DTDScanner::scanElementDecl()
{
    //
    //  Space is legal (required actually) here so check for a PE ref. If
    //  we don't get our whitespace, then issue and error, but try to keep
    //  going.
    //
    if (!checkForPERef(false, true))
        fScanner->emitError(XMLErrs::ExpectedWhitespace);

    // Get a buffer for the element name and scan in the name
    XMLBufBid bbName(fBufMgr);
    if (!fReaderMgr->getName(bbName.getBuffer()))
    {
        fScanner->emitError(XMLErrs::ExpectedElementName);
        fReaderMgr->skipPastChar(chCloseAngle);
        return;
    }

    // Look this guy up in the element decl pool
    DTDElementDecl* decl = (DTDElementDecl*) fDTDGrammar->getElemDecl(fEmptyNamespaceId, 0, bbName.getRawBuffer(), Grammar::TOP_LEVEL_SCOPE);

    //
    //  If it does not exist, then we need to create it. If it does and
    //  its marked as declared, then that's an error, but we still need to
    //  scan over the content model so use the dummy declaration that the
    //  parsing code can fill in.
    //
    if (decl)
    {
        if (decl->isDeclared())
        {
            if (fScanner->getValidationScheme() == XMLScanner::Val_Always)
                fScanner->getValidator()->emitError(XMLValid::ElementAlreadyExists, bbName.getRawBuffer());

            if (!fDumElemDecl)
                fDumElemDecl = new (fMemoryManager) DTDElementDecl
                (
                    bbName.getRawBuffer()
                    , fEmptyNamespaceId
                    , DTDElementDecl::Any
                    , fMemoryManager
                );
            else
                fDumElemDecl->setElementName(bbName.getRawBuffer(),fEmptyNamespaceId);
        }
    }
     else
    {
        //
        //  Create the new empty declaration to fill in and put it into
        //  the decl pool.
        //
        decl = new (fGrammarPoolMemoryManager) DTDElementDecl
        (
            bbName.getRawBuffer()
            , fEmptyNamespaceId
            , DTDElementDecl::Any
            , fGrammarPoolMemoryManager
        );
        fDTDGrammar->putElemDecl(decl);
    }

    // Set a flag for whether we will ignore this one
    const bool isIgnored = (decl == fDumElemDecl);

    // Mark this one if being externally declared
    decl->setExternalElemDeclaration(isReadingExternalEntity());

    // Mark this one as being declared
    decl->setCreateReason(XMLElementDecl::Declared);

    // Another check for a PE ref, with at least required whitespace
    if (!checkForPERef(false, true))
        fScanner->emitError(XMLErrs::ExpectedWhitespace);

    // And now scan the content model for this guy.
    if (!scanContentSpec(*decl))
    {
        fReaderMgr->skipPastChar(chCloseAngle);
        return;
    }

    // Another check for a PE ref, but we don't require whitespace here
    checkForPERef(false, true);

    // And we should have the ending angle bracket
    if (!fReaderMgr->skippedChar(chCloseAngle))
    {
        fScanner->emitError(XMLErrs::UnterminatedElementDecl, bbName.getRawBuffer());
        fReaderMgr->skipPastChar(chCloseAngle);
    }

    //
    //  If we have a DTD handler tell it about the new element decl. We
    //  tell it if its one that can be ignored, cause its an override of a
    //  previously existing decl. If it is being ignored, only call back
    //  if advanced callbacks are enabled.
    //
    if (fDocTypeHandler)
        fDocTypeHandler->elementDecl(*decl, isIgnored);
}


//
//  This method will process a general or parameter entity reference. The
//  entity name and entity text will be stored in the entity pool. The value
//  of the entity will be scanned for any other parameter entity or char
//  references which will be expanded. So the stored value can only have
//  general entity references when done.
//
void DTDScanner::scanEntityDecl()
{
    //
    //  Space is required here, but we cannot check for a PE Ref since
    //  there could be a legal (no-ref) percent sign here. Since any
    //  entity that ended here would be illegal, we just skip spaces
    //  and then check for a percent.
    //
    if (!fReaderMgr->lookingAtSpace())
        fScanner->emitError(XMLErrs::ExpectedWhitespace);
    else
        fReaderMgr->skipPastSpaces();
    bool isPEDecl = fReaderMgr->skippedChar(chPercent);

    //
    //  If a PE decl, then check if it is followed by a space; if it is so, 
    //  eat the percent and check for spaces or a PE ref on the other side of it. 
    //  Otherwise, it has to be an entity reference for a general entity.
    //
    if (isPEDecl)
    {
        if(!fReaderMgr->getCurrentReader()->isWhitespace(fReaderMgr->peekNextChar()))
        {
            isPEDecl=false;
            while (true)
            {
               if (!expandPERef(false, false, true, false))
                  fScanner->emitError(XMLErrs::ExpectedEntityRefName);
               // And skip any more spaces in the expanded value
               if (fReaderMgr->skippedSpace())
                  fReaderMgr->skipPastSpaces();
               if (!fReaderMgr->skippedChar(chPercent))
                  break;
            }
        }
        else if (!checkForPERef(false, true))
            fScanner->emitError(XMLErrs::ExpectedWhitespace);
    }

    //
    //  Now lets get a name, which should be the name of the entity. We
    //  have to get a buffer for this.
    //
    XMLBufBid bbName(fBufMgr);
    if (!fReaderMgr->getName(bbName.getBuffer()))
    {
        fScanner->emitError(XMLErrs::ExpectedPEName);
        fReaderMgr->skipPastChar(chCloseAngle);
        return;
    }

    // If namespaces are enabled, then no colons allowed
    if (fScanner->getDoNamespaces())
    {
        if (XMLString::indexOf(bbName.getRawBuffer(), chColon) != -1)
            fScanner->emitError(XMLErrs::ColonNotLegalWithNS);
    }

    //
    //  See if this entity already exists. If so, then the existing one
    //  takes precendence. So we use the local dummy decl to parse into
    //  and just ignore the results.
    //
    DTDEntityDecl* entityDecl;
    if (isPEDecl)
        entityDecl = fPEntityDeclPool->getByKey(bbName.getRawBuffer());
    else
        entityDecl = fDTDGrammar->getEntityDecl(bbName.getRawBuffer());

    if (entityDecl)
    {
        if (!fDumEntityDecl)
            fDumEntityDecl = new (fMemoryManager) DTDEntityDecl(fMemoryManager);
        fDumEntityDecl->setName(bbName.getRawBuffer());
        entityDecl = fDumEntityDecl;
    }
     else
    {
        // Its not in existence already, then create an entity decl for it
        entityDecl = new (fGrammarPoolMemoryManager) DTDEntityDecl(bbName.getRawBuffer(), false, fGrammarPoolMemoryManager);

        //
        //  Set the declaration location. The parameter indicates whether its
        //  declared in the content/internal subset, so we know whether or not
        //  its in the external subset.
        //
        entityDecl->setDeclaredInIntSubset(fInternalSubset);

        // Add it to the appropriate entity decl pool
        if (isPEDecl)
            fPEntityDeclPool->put(entityDecl);
         else
            fDTDGrammar->putEntityDecl(entityDecl);
    }

    // Set a flag that indicates whether we are ignoring this one
    const bool isIgnored = (entityDecl == fDumEntityDecl);

    // Set the PE flag on it
    entityDecl->setIsParameter(isPEDecl);

    //
    //  Space is legal (required actually) here so check for a PE ref. If
    //  we don't get our whitespace, then issue an error, but try to keep
    //  going.
    //
    if (!checkForPERef(false, true))
        fScanner->emitError(XMLErrs::ExpectedWhitespace);

    // save the hasNoDTD status for Entity Constraint Checking
    bool hasNoDTD = fScanner->getHasNoDTD();
    if (hasNoDTD && isPEDecl)
        fScanner->setHasNoDTD(false);

    // According to the type call the value scanning method
    if (!scanEntityDef(*entityDecl, isPEDecl))
    {
        fReaderMgr->skipPastChar(chCloseAngle);
        fScanner->setHasNoDTD(true);
        fScanner->emitError(XMLErrs::ExpectedEntityValue);
        return;
    }
    if (hasNoDTD)
        fScanner->setHasNoDTD(true);

    // Space is legal (but not required) here so check for a PE ref
    checkForPERef(false, true);

    // And then we have to have the closing angle bracket
    if (!fReaderMgr->skippedChar(chCloseAngle))
    {
        fScanner->emitError(XMLErrs::UnterminatedEntityDecl, entityDecl->getName());
        fReaderMgr->skipPastChar(chCloseAngle);
    }

    //
    //  If we have a doc type handler, then call it. But only call it for
    //  ignored elements if advanced callbacks are enabled.
    //
    if (fDocTypeHandler)
        fDocTypeHandler->entityDecl(*entityDecl, isPEDecl, isIgnored);
}


//
//  This method will scan a general/character entity ref. It will either
//  expand a char ref and return the value directly, or it will expand
//  a general entity and a reader for it onto the reader stack.
//
//  The return value indicates whether the value was returned directly or
//  pushed as a reader or it failed.
//
//  The escaped flag tells the caller whether the returnd parameter resulted
//  from a character reference, which escapes the character in some cases. It
//  only makes any difference if the return indicates the value was returned
//  directly.
//
//  NOTE: This is only called when scanning attribute values, so we always
//  expand general entities.
//
DTDScanner::EntityExpRes
DTDScanner::scanEntityRef(XMLCh& firstCh, XMLCh& secondCh, bool& escaped)
{
    // Assume no escape and no second char
    escaped = false;
    secondCh = 0;

    // We have to insure its all done in a single entity
    const XMLSize_t curReader = fReaderMgr->getCurrentReaderNum();

    //
    //  If the next char is a pound, then its a character reference and we
    //  need to expand it always.
    //
    if (fReaderMgr->skippedChar(chPound))
    {
        //
        //  Its a character reference, so scan it and get back the numeric
        //  value it represents. If it fails, just return immediately.
        //
        if (!scanCharRef(firstCh, secondCh))
            return EntityExp_Failed;

        if (curReader != fReaderMgr->getCurrentReaderNum())
            fScanner->emitError(XMLErrs::PartialMarkupInEntity);

        // Its now escaped since it was a char ref
        escaped = true;
        return EntityExp_Returned;
    }

    // Get the name of the general entity
    XMLBufBid bbName(fBufMgr);
    if (!fReaderMgr->getName(bbName.getBuffer()))
    {
        fScanner->emitError(XMLErrs::ExpectedEntityRefName);
        return EntityExp_Failed;
    }

    //
    //  Next char must be a semi-colon. But if its not, just emit
    //  an error and try to continue.
    //
    if (!fReaderMgr->skippedChar(chSemiColon))
        fScanner->emitError(XMLErrs::UnterminatedEntityRef, bbName.getRawBuffer());

    // Make sure it was all in one entity reader
    if (curReader != fReaderMgr->getCurrentReaderNum())
        fScanner->emitError(XMLErrs::PartialMarkupInEntity);

    // Look it up the name the general entity pool
    XMLEntityDecl* decl = fDTDGrammar->getEntityDecl(bbName.getRawBuffer());

    // If it does not exist, then obviously an error
    if (!decl)
    {
        // XML 1.0 Section 4.1
        if (fScanner->getStandalone() || fScanner->getHasNoDTD()) {
            fScanner->emitError(XMLErrs::EntityNotFound, bbName.getRawBuffer());
        }
        else {
            if (fScanner->getValidationScheme() == XMLScanner::Val_Always)
                fScanner->getValidator()->emitError(XMLValid::VC_EntityNotFound, bbName.getRawBuffer());
        }

        return EntityExp_Failed;
    }


    //
    // XML 1.0 Section 4.1
    //  If we are a standalone document, then it has to have been declared
    //  in the internal subset.
    //
    if (fScanner->getStandalone() && !decl->getDeclaredInIntSubset())
        fScanner->emitError(XMLErrs::IllegalRefInStandalone, bbName.getRawBuffer());

    //
    //  If its a special char reference, then its escaped and we can return
    //  it directly.
    //
    if (decl->getIsSpecialChar())
    {
        firstCh = decl->getValue()[0];
        escaped = true;
        return EntityExp_Returned;
    }

    if (decl->isExternal())
    {
        // If its unparsed, then its not valid here
        // XML 1.0 Section 4.4.4 the appearance of a reference to an unparsed entity is forbidden.
        if (decl->isUnparsed())
        {
            fScanner->emitError(XMLErrs::NoUnparsedEntityRefs, bbName.getRawBuffer());
            return EntityExp_Failed;
        }

        // We are in an attribute value, so not valid.
        // XML 1.0 Section 4.4.4 a reference to an external entity in an attribute value is forbidden.
        fScanner->emitError(XMLErrs::NoExtRefsInAttValue);

        // And now create a reader to read this entity
        InputSource* srcUsed;
        XMLReader* reader = fReaderMgr->createReader
        (
            decl->getBaseURI()
            , decl->getSystemId()
            , decl->getPublicId()
            , false
            , XMLReader::RefFrom_NonLiteral
            , XMLReader::Type_General
            , XMLReader::Source_External
            , srcUsed
            , fScanner->getCalculateSrcOfs()
            , fScanner->getLowWaterMark()
            , fScanner->getDisableDefaultEntityResolution()
        );

        // Put a janitor on the source so it gets cleaned up on exit
        Janitor<InputSource> janSrc(srcUsed);

        //
        //  If the creation failed then throw an exception
        //
        if (!reader)
            ThrowXMLwithMemMgr1(RuntimeException, XMLExcepts::Gen_CouldNotOpenExtEntity, srcUsed ? srcUsed->getSystemId() : decl->getSystemId(), fMemoryManager);

        //
        //  Push the reader. If its a recursive expansion, then emit an error
        //  and return an failure.
        //
        if (!fReaderMgr->pushReader(reader, decl))
        {
            fScanner->emitError(XMLErrs::RecursiveEntity, decl->getName());
            return EntityExp_Failed;
        }

        // If it starts with the XML string, then parse a text decl
        if (fScanner->checkXMLDecl(true))
            scanTextDecl();
    }
     else
    {
        //
        //  Create a reader over a memory stream over the entity value
        //  We force it to assume UTF-16 by passing in an encoding
        //  string. This way it won't both trying to predecode the
        //  first line, looking for an XML/TextDecl.
        //
        XMLReader* valueReader = fReaderMgr->createIntEntReader
        (
            decl->getName()
            , XMLReader::RefFrom_NonLiteral
            , XMLReader::Type_General
            , decl->getValue()
            , decl->getValueLen()
            , false
        );

        //
        //  Trt to push the entity reader onto the reader manager stack,
        //  where it will become the subsequent input. If it fails, that
        //  means the entity is recursive, so issue an error. The reader
        //  will have just been discarded, but we just keep going.
        //
        if (!fReaderMgr->pushReader(valueReader, decl))
            fScanner->emitError(XMLErrs::RecursiveEntity, decl->getName());
    }

    return EntityExp_Pushed;
}


//
//  This method will scan a quoted literal of an entity value. It has to
//  deal with replacement of PE references; however, since this is a DTD
//  scanner, all such entity literals are in entity decls and therefore
//  general entities are not expanded.
//
bool DTDScanner::scanEntityLiteral(XMLBuffer& toFill)
{
    toFill.reset();

    // Get the next char which must be a single or double quote
    XMLCh quoteCh;
    if (!fReaderMgr->skipIfQuote(quoteCh))
        return false;

    // Get a buffer for pulling in entity names when we see GE refs
    XMLBufBid bbName(fBufMgr);
    XMLBuffer& nameBuf = bbName.getBuffer();

    // Remember the current reader
    const XMLSize_t orgReader = fReaderMgr->getCurrentReaderNum();

    //
    //  Loop until we see the ending quote character, handling any references
    //  in the process.
    //
    XMLCh   nextCh;
    XMLCh   secondCh = 0;
    bool    gotLeadingSurrogate = false;
    while (true)
    {
        nextCh = fReaderMgr->getNextChar();

        //
        //  Watch specifically for EOF and issue a more meaningful error
        //  if that occurs (since an unterminated quoted char can cause
        //  this easily.)
        //
        if (!nextCh)
        {
            fScanner->emitError(XMLErrs::UnterminatedEntityLiteral);
            ThrowXMLwithMemMgr(UnexpectedEOFException, XMLExcepts::Gen_UnexpectedEOF, fMemoryManager);
        }

        //
        //  Break out on our terminating quote char when we are back in the
        //  same reader. Otherwise, we might trigger on a nested quote char
        //  in an expanded entity.
        //
        if ((nextCh == quoteCh)
        &&  (fReaderMgr->getCurrentReaderNum() == orgReader))
        {
            break;
        }

        if (nextCh == chPercent)
        {
            //
            //  Put the PE's value on the reader stack and then jump back
            //  to the top to start processing it. The parameter indicates
            //  that it should not scan the reference's content as an external
            //  subset.
            //
            expandPERef(false, true, true);
            continue;
        }

        //
        //  Ok, now that all the other special stuff is checked, we can
        //  look for a general entity. In here, we cannot have a naked &
        //  and will only expand numerical char refs or the intrinsic char
        //  refs. Others will be left alone.
        //
        if (nextCh == chAmpersand)
        {
            //
            //  Here, we only expand numeric char refs, but not any general
            //  entities. However, the stupid XML spec requires that we check
            //  and make sure it does refer to a general entity if its not
            //  a char ref (i.e. no naked '&' chars.)
            //
            if (fReaderMgr->skippedChar(chPound))
            {
                // If it failed, then just jump back to the top and try to pick up
                if (!scanCharRef(nextCh, secondCh))
                {
                    gotLeadingSurrogate = false;
                    continue;
                }
            }
             else
            {
                if (!fReaderMgr->getName(nameBuf))
                {
                    fScanner->emitError(XMLErrs::ExpectedEntityRefName);
                }
                 else
                {
                    //
                    //  Since we are not expanding any of this, we have to
                    //  put the amp and name into the target buffer as data.
                    //
                    toFill.append(chAmpersand);
                    toFill.append(nameBuf.getRawBuffer());

                    // Make sure we skipped a trailing semicolon
                    if (!fReaderMgr->skippedChar(chSemiColon))
                    {
                        fScanner->emitError
                        (
                            XMLErrs::UnterminatedEntityRef
                            , nameBuf.getRawBuffer()
                        );
                    }

                    // And make the new character the semicolon
                    nextCh = chSemiColon;
                }

                // Either way here we reset the surrogate flag
                gotLeadingSurrogate = false;
            }
        }
        else if ((nextCh >= 0xD800) && (nextCh <= 0xDBFF))
        {
            if (gotLeadingSurrogate)
                fScanner->emitError(XMLErrs::Expected2ndSurrogateChar);
            else
                gotLeadingSurrogate = true;
        }
         else
        {
            if (gotLeadingSurrogate)
            {
                if ((nextCh < 0xDC00) || (nextCh > 0xDFFF))
                    fScanner->emitError(XMLErrs::Expected2ndSurrogateChar);
            }
             else if (!fReaderMgr->getCurrentReader()->isXMLChar(nextCh))
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
                fScanner->emitError(XMLErrs::InvalidCharacter, tmpBuf);
                fReaderMgr->skipPastChar(quoteCh);
                return false;
            }
            gotLeadingSurrogate = false;
        }

        // Looks ok, so add it to the literal
        toFill.append(nextCh);

        if (secondCh)
        {
            toFill.append(secondCh);
            secondCh=0;
        }
    }

    //
    //  If we got here and did not get back to the original reader level,
    //  then we propogated some entity out of the literal, so issue an
    //  error, but don't fail.
    //
    if (fReaderMgr->getCurrentReaderNum() != orgReader && fScanner->getValidationScheme() == XMLScanner::Val_Always)
        fScanner->getValidator()->emitError(XMLValid::PartialMarkupInPE);

    return true;
}


//
//  This method is called after the entity name has been scanned, and any
//  PE referenced following the name is handled. The passed decl will be
//  filled in with the info scanned.
//
bool DTDScanner::scanEntityDef(DTDEntityDecl& decl, const bool isPEDecl)
{
    // Its got to be an entity literal
    if (fReaderMgr->lookingAtChar(chSingleQuote)
    ||  fReaderMgr->lookingAtChar(chDoubleQuote))
    {
        // Get a buffer for the literal
        XMLBufBid bbValue(fBufMgr);

        if (!scanEntityLiteral(bbValue.getBuffer()))
            return false;

        // Set it on the entity decl
        decl.setValue(bbValue.getRawBuffer());
        return true;
    }

    //
    //  Its got to be an external entity, so there must be an external id.
    //  Get buffers for them and scan an external id into them.
    //
    XMLBufBid bbPubId(fBufMgr);
    XMLBufBid bbSysId(fBufMgr);
    if (!scanId(bbPubId.getBuffer(), bbSysId.getBuffer(), IDType_External))
        return false;

    decl.setIsExternal(true);
    ReaderMgr::LastExtEntityInfo lastInfo;
    fReaderMgr->getLastExtEntityInfo(lastInfo);

    // Fill in the id fields of the decl with the info we got
    const XMLCh* publicId = bbPubId.getRawBuffer();
    const XMLCh* systemId = bbSysId.getRawBuffer();
    decl.setPublicId((publicId && *publicId) ? publicId : 0);
    decl.setSystemId((systemId && *systemId) ? systemId : 0);
    decl.setBaseURI((lastInfo.systemId && *lastInfo.systemId) ? lastInfo.systemId : 0);

    // If its a PE decl, we are done
    bool gotSpaces = checkForPERef(false, true);
    if (isPEDecl)
    {
        //
        //  Check for a common error here. NDATA is not allowed for PEs
        //  so check for the NDATA string. If found give a nice meaningful
        //  error and continue parsing to eat the NDATA text.
        //
        if (gotSpaces)
        {
            if (fReaderMgr->skippedString(XMLUni::fgNDATAString))
                fScanner->emitError(XMLErrs::NDATANotValidForPE);
        }
         else
        {
            return true;
        }
    }

    // If looking at close angle now, we are done
    if (fReaderMgr->lookingAtChar(chCloseAngle))
        return true;

    // Else we had to have seem the whitespace
    if (!gotSpaces)
        fScanner->emitError(XMLErrs::ExpectedWhitespace);

    // We now have to see a notation data string
    if (!fReaderMgr->skippedString(XMLUni::fgNDATAString))
        fScanner->emitError(XMLErrs::ExpectedNDATA);

    // Space is required here, but try to go on if not
    if (!checkForPERef(false, true))
        fScanner->emitError(XMLErrs::ExpectedWhitespace);

    // Get a name
    XMLBufBid bbName(fBufMgr);
    if (!fReaderMgr->getName(bbName.getBuffer()))
    {
        fScanner->emitError(XMLErrs::ExpectedNotationName);
        return false;
    }

    // Set the decl's notation name
    decl.setNotationName(bbName.getRawBuffer());

    return true;
}


//
//  This method is called after an attribute decl name or a notation decl has
//  been scanned and then an opening parenthesis was see, indicating the list
//  of values. It scans the enumeration values and creates a single string
//  which has a single space between each value.
//
//  The terminating close paren ends this scan.
//
bool DTDScanner::scanEnumeration( const   DTDAttDef&  attDef
                                    ,       XMLBuffer&  toFill
                                    , const bool        notation)
{
    // Reset the passed buffer
    toFill.reset();

    // Check for PE ref but don't require space
    checkForPERef(false, true);

    // If this is a notation, we need an opening paren
    if (notation)
    {
        if (!fReaderMgr->skippedChar(chOpenParen))
            fScanner->emitError(XMLErrs::ExpectedOpenParen);
    }

    // We need a local buffer to use as well
    XMLBufBid bbTmp(fBufMgr);

    while (true)
    {
        // Space is allowed here for either type so check for PE ref
        checkForPERef(false, true);

        // And then get either a name or a name token
        bool success;
        if (notation)
            success = fReaderMgr->getName(bbTmp.getBuffer());
        else
            success = fReaderMgr->getNameToken(bbTmp.getBuffer());

        if (!success)
        {
            fScanner->emitError
            (
                XMLErrs::ExpectedEnumValue
                , attDef.getFullName()
            );
            return false;
        }

        // Append this value to the target value
        toFill.append(bbTmp.getRawBuffer(), bbTmp.getLen());

        // Space is allowed here for either type so check for PE ref
        checkForPERef(false, true);

        // Check for the terminating paren
        if (fReaderMgr->skippedChar(chCloseParen))
            break;

        // And append a space separator
        toFill.append(chSpace);

        // Check for the pipe character separator
        if (!fReaderMgr->skippedChar(chPipe))
        {
            fScanner->emitError(XMLErrs::ExpectedEnumSepOrParen);
            return false;
        }
    }
    return true;
}


bool DTDScanner::scanEq()
{
    fReaderMgr->skipPastSpaces();
    if (fReaderMgr->skippedChar(chEqual))
    {
        fReaderMgr->skipPastSpaces();
        return true;
    }
    return false;
}


//
//  This method is called when an external entity reference is seen in the
//  DTD or an external DTD subset is encountered, and their contents pushed
//  onto the reader stack. This method will scan that contents.
//
void DTDScanner::scanExtSubsetDecl(const bool inIncludeSect, const bool isDTD)
{
    // Indicate we are in the external subset now
    FlagJanitor<bool> janContentFlag(&fInternalSubset, false);


    bool bAcceptDecl = !inIncludeSect;

    // Get a buffer for whitespace
    XMLBufBid bbSpace(fBufMgr);

    //
    //  If we have a doc type handler and we are not being called recursively
    //  to handle an include section, tell it the ext subset starts
    //
    if (fDocTypeHandler && isDTD && !inIncludeSect)
        fDocTypeHandler->startExtSubset();

    //
    //  We have to play a trick here if the current entity we are parsing
    //  is a PE. Because the spooling code will put out a whitespace before
    //  and after an expanded PE if its being scanned outside the context of
    //  a literal entity, this will confuse this external subset code.
    //
    //  So, we see if that is what is happening and, if so, eat the single
    //  space, a check for the <?xml string. If we find it, we parse that
    //  markup right now and put the space back.
    //
    if (fReaderMgr->isScanningPERefOutOfLiteral())
    {
        if (fReaderMgr->skippedSpace())
        {
            if (fScanner->checkXMLDecl(true))
            {
                scanTextDecl();
                bAcceptDecl = false;

                // <TBD> Figure out how to do this
                // fReaderMgr->unGet(chSpace);
            }
        }
    }

    // Get the current reader number
    const XMLSize_t orgReader = fReaderMgr->getCurrentReaderNum();

    //
    //  Loop until we hit the end of the external subset entity. Note that
    //  we use a double loop here in order to avoid the overhead of doing
    //  the exception setup/teardown work on every loop.
    //
    bool inMarkup = false;
    bool inCharData = false;
    while (true)
    {
        bool bDoBreak=false;    // workaround for Borland bug with 'break' in 'catch'
        try
        {
            while (true)
            {
                const XMLCh nextCh = fReaderMgr->peekNextChar();

                if (!nextCh)
                {
                    return; // nothing left
                }
                else if (nextCh == chOpenAngle)
                {
                    // Get the reader we started this on
                    // XML 1.0 P28a Well-formedness constraint: PE Between Declarations
                    const XMLSize_t orgReader = fReaderMgr->getCurrentReaderNum();
                    bool wasInPE = (fReaderMgr->getCurrentReader()->getType() == XMLReader::Type_PE);

                    //
                    //  Now scan the markup. Set the flag so that we will know that
                    //  we were in markup if an end of entity exception occurs.
                    //
                    fReaderMgr->getNextChar();
                    inMarkup = true;
                    scanMarkupDecl(bAcceptDecl);
                    inMarkup = false;

                    //
                    //  And see if we got back to the same level. If not, then its
                    //  a partial markup error.
                    //
                    if (fReaderMgr->getCurrentReaderNum() != orgReader){
                        if (wasInPE)
                            fScanner->emitError(XMLErrs::PEBetweenDecl);
                        else if (fScanner->getValidationScheme() == XMLScanner::Val_Always)
                            fScanner->getValidator()->emitError(XMLValid::PartialMarkupInPE);
                    }

                }
                else if (fReaderMgr->getCurrentReader()->isWhitespace(nextCh))
                {
                    //
                    //  If we have a doc type handler, and advanced callbacks are
                    //  enabled, then gather up whitespace and call back. Otherwise
                    //  just skip whitespaces.
                    //
                    if (fDocTypeHandler)
                    {
                        inCharData = true;
                        fReaderMgr->getSpaces(bbSpace.getBuffer());
                        inCharData = false;

                        fDocTypeHandler->doctypeWhitespace
                        (
                            bbSpace.getRawBuffer()
                            , bbSpace.getLen()
                        );
                    }
                    else
                    {
                        //
                        //  If we hit an end of entity in the middle of white
                        //  space, that's fine. We'll just come back in here
                        //  again on the next round and skip some more.
                        //
                        fReaderMgr->skipPastSpaces();
                    }
                }
                else if (nextCh == chPercent)
                {
                    //
                    //  Expand (and scan if external) the reference value. Tell
                    //  it to throw an end of entity exception at the end of the
                    //  entity.
                    //
                    fReaderMgr->getNextChar();
                    expandPERef(true, false, false, true);
                }
                else if (inIncludeSect && (nextCh == chCloseSquare))
                {
                    //
                    //  Its the end of a conditional include section. So scan it and
                    //  decrement the include depth counter.
                    //
                    fReaderMgr->getNextChar();
                    if (!fReaderMgr->skippedChar(chCloseSquare))
                    {
                        fScanner->emitError(XMLErrs::ExpectedEndOfConditional);
                        fReaderMgr->skipPastChar(chCloseAngle);
                    }
                    else if (!fReaderMgr->skippedChar(chCloseAngle))
                    {
                        fScanner->emitError(XMLErrs::ExpectedEndOfConditional);
                        fReaderMgr->skipPastChar(chCloseAngle);
                    }
                    return;
                }
                else
                {
                    fReaderMgr->getNextChar();
                    if (!fReaderMgr->getCurrentReader()->isXMLChar(nextCh))
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
                        fScanner->emitError(XMLErrs::InvalidCharacter, tmpBuf);
                    }
                    else
                    {
                        fScanner->emitError(XMLErrs::InvalidDocumentStructure);
                    }

                    // Try to get realigned
                    static const XMLCh toSkip[] =
                    {
                        chPercent, chCloseSquare, chOpenAngle, chNull
                    };
                    fReaderMgr->skipUntilInOrWS(toSkip);
                }
                bAcceptDecl = false;
            }
        }
        catch(const EndOfEntityException& toCatch)
        {
            //
            //  If the external entity ended while we were in markup, then that's
            //  a partial markup error.
            //
            if (inMarkup)
            {
                fScanner->emitError(XMLErrs::PartialMarkupInEntity);
                inMarkup = false;
            }

            // If we were in char data, then send what we got
            if (inCharData)
            {
                // Send what we got, then rethrow
                if (fDocTypeHandler)
                {
                    fDocTypeHandler->doctypeWhitespace
                    (
                        bbSpace.getRawBuffer()
                        , bbSpace.getLen()
                    );
                }
                inCharData = false;
            }

            //
            //  If the entity that just ended was the entity that we started
            //  on, then this is the end of the external subset.
            //
            if (orgReader == toCatch.getReaderNum())
                bDoBreak=true;
        }
        if(bDoBreak)
            break;
    }

    // If we have a doc type handler, tell it the ext subset ends
    if (fDocTypeHandler && isDTD && !inIncludeSect)
        fDocTypeHandler->endExtSubset();
}


//
//  This method will scan for an id, either public or external.
//
//
// [75] ExternalID ::= 'SYSTEM' S SystemLiteral
//                     | 'PUBLIC' S PubidLiteral S SystemLiteral
// [83] PublicID ::= 'PUBLIC' S PubidLiteral
//
bool DTDScanner::scanId(          XMLBuffer&  pubIdToFill
                            ,       XMLBuffer&  sysIdToFill
                            , const IDTypes     whatKind)
{
    // Clean out both return buffers
    pubIdToFill.reset();
    sysIdToFill.reset();

    //
    //  Check first for the system id first. If we find it, and system id
    //  is one of the legal values, then lets try to scan it.
    //
    // 'SYSTEM' S SystemLiteral
    if (fReaderMgr->skippedString(XMLUni::fgSysIDString))
    {
        // If they were looking for a public id, then we failed
        if (whatKind == IDType_Public)
        {
            fScanner->emitError(XMLErrs::ExpectedPublicId);
            return false;
        }

        // We must skip spaces
        bool skippedSomething;
        fReaderMgr->skipPastSpaces(skippedSomething);
        if (!skippedSomething)
        {
            fScanner->emitError(XMLErrs::ExpectedWhitespace);
            return false;
        }

        // Get the system literal value
        return scanSystemLiteral(sysIdToFill);
    }

    // Now scan for public id
    // 'PUBLIC' S PubidLiteral S SystemLiteral
    //  or
    // 'PUBLIC' S PubidLiteral

    // If we don't have any public id string => Error
    if (!fReaderMgr->skippedString(XMLUni::fgPubIDString)) {
        fScanner->emitError(XMLErrs::ExpectedSystemOrPublicId);
        return false;
    }

    //
    //  So following this we must have whitespace, a public literal, whitespace,
    //  and a system literal.
    //
    bool skippedSomething;
    fReaderMgr->skipPastSpaces(skippedSomething);
    if (!skippedSomething)
    {
        fScanner->emitError(XMLErrs::ExpectedWhitespace);

        //
        //  Just in case, if they just forgot the whitespace but the next char
        //  is a single or double quote, then keep going.
        //
        const XMLCh chPeek = fReaderMgr->peekNextChar();
        if ((chPeek != chDoubleQuote) && (chPeek != chSingleQuote))
            return false;
    }

    if (!scanPublicLiteral(pubIdToFill))
        return false;

    // If they wanted a public id, then this is all
    if (whatKind == IDType_Public)
        return true;

    // check if there is any space follows
    bool hasSpace;
    fReaderMgr->skipPastSpaces(hasSpace);

    //
    //  In order to recover best here we need to see if
    //  the next thing is a quote or not
    //
    const XMLCh chPeek = fReaderMgr->peekNextChar();
    const bool bIsQuote =  ((chPeek == chDoubleQuote)
                         || (chPeek == chSingleQuote));

    if (!hasSpace)
    {
        if (whatKind == IDType_External)
        {
            //
            //  If its an external Id, then we need to see the system id.
            //  So, emit the error. But, if the next char is a quote, don't
            //  give up since its probably going to work. The user just
            //  missed the separating space. Otherwise, fail.
            //
            fScanner->emitError(XMLErrs::ExpectedWhitespace);
            if (!bIsQuote)
                return false;
        }
         else
        {
            //
            //  We can legally return here. But, if the next char is a quote,
            //  then that's probably not what was desired, since its probably
            //  just that space was forgotten and there really is a system
            //  id to follow.
            //
            //  So treat it like missing whitespace if so and keep going.
            //  Else, just return success.
            //
            if (bIsQuote)
                fScanner->emitError(XMLErrs::ExpectedWhitespace);
             else
                return true;
        }
    }

    if (bIsQuote) {
        // there is a quote coming, scan the system literal
        if (!scanSystemLiteral(sysIdToFill))
            return false;
    }
    else {
        // no quote, if expecting exteral id, this is an error
        if (whatKind == IDType_External)
            fScanner->emitError(XMLErrs::ExpectedQuotedString);
    }

    return true;
}


//
//  This method will scan the contents of an ignored section. It assumes that
//  we already are in the body, i.e. we've seen <![IGNORE[ at this point. So
//  we have to just scan until we see a matching ]]> closing markup.
//
void DTDScanner::scanIgnoredSection()
{
    //
    //  Depth starts at one because we are already in one section and want
    //  to parse until we hit its end.
    //
    unsigned long depth = 1;
    bool gotLeadingSurrogate = false;
    while (true)
    {
        const XMLCh nextCh = fReaderMgr->getNextChar();

        if (!nextCh)
            ThrowXMLwithMemMgr(UnexpectedEOFException, XMLExcepts::Gen_UnexpectedEOF, fMemoryManager);

        if (nextCh == chOpenAngle)
        {
            if (fReaderMgr->skippedChar(chBang)
            &&  fReaderMgr->skippedChar(chOpenSquare))
            {
                depth++;
            }
        }
         else if (nextCh == chCloseSquare)
        {
            if (fReaderMgr->skippedChar(chCloseSquare))
            {
                while (fReaderMgr->skippedChar(chCloseSquare))
                {
                    // Do nothing, just skip them
                }

                if (fReaderMgr->skippedChar(chCloseAngle))
                {
                    depth--;
                    if (!depth)
                        break;
                }
            }
        }
        // Deal with surrogate pairs
        else if ((nextCh >= 0xD800) && (nextCh <= 0xDBFF))
        {
            //  Its a leading surrogate. If we already got one, then
            //  issue an error, else set leading flag to make sure that
            //  we look for a trailing next time.
            if (gotLeadingSurrogate)
                fScanner->emitError(XMLErrs::Expected2ndSurrogateChar);
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
                    fScanner->emitError(XMLErrs::Unexpected2ndSurrogateChar);
            }
            else
            {
                //  Its just a char, so make sure we were not expecting a
                //  trailing surrogate.
                if (gotLeadingSurrogate)
                    fScanner->emitError(XMLErrs::Expected2ndSurrogateChar);

                // Its got to at least be a valid XML character
                else if (!fReaderMgr->getCurrentReader()->isXMLChar(nextCh))
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
                    fScanner->emitError(XMLErrs::InvalidCharacter, tmpBuf);
                }
            }
            gotLeadingSurrogate = false;
        }
    }
}


//
//  This method scans the entire internal subset. All we can have here is
//  decl markup, and PE references. The expanded PE references must contain
//  whole markup, so we don't have to worry about their content at this
//  level. We just scan them, expand them, push them, and parse their content
//  right there, via the expandERef() method.
//
bool DTDScanner::scanInternalSubset()
{
    // Indicate we are in the internal subset now
    FlagJanitor<bool> janContentFlag(&fInternalSubset, true);

    // If we have a doc type handler, tell it the internal subset starts
    if (fDocTypeHandler)
        fDocTypeHandler->startIntSubset();

    // Get a buffer for whitespace
    XMLBufBid bbSpace(fBufMgr);

    bool noErrors = true;
    while (true)
    {
        const XMLCh nextCh = fReaderMgr->peekNextChar();

        //
        //  If we get an end of file marker, just unget it and return a
        //  failure status. The caller will then see the end of file and
        //  faill out correctly.
        //
        if (!nextCh)
            return false;

        // Watch for the end of internal subset marker
        if (nextCh == chCloseSquare)
        {
            fReaderMgr->getNextChar();
            break;
        }

        if (nextCh == chPercent)
        {
            //
            //  Expand (and scan if external) the reference value. Tell
            //  it to set the reader to cause an end of entity exception
            //  when this reader dies, which is what the scanExtSubset
            //  method wants (who is called to scan this.)
            //
            fReaderMgr->getNextChar();
            expandPERef(true, false, false, true);
        }
         else if (nextCh == chOpenAngle)
        {
            // Remember this reader before we start the scan, for checking
            // XML 1.0 P28a Well-formedness constraint: PE Between Declarations
            const XMLSize_t orgReader = fReaderMgr->getCurrentReaderNum();
            bool wasInPE = (fReaderMgr->getCurrentReader()->getType() == XMLReader::Type_PE);

            // And scan this markup
            fReaderMgr->getNextChar();
            scanMarkupDecl(false);

            // If we did not get back to entry level, then partial markup
            if (fReaderMgr->getCurrentReaderNum() != orgReader) {
                if (wasInPE)
                    fScanner->emitError(XMLErrs::PEBetweenDecl);
                else if (fScanner->getValidationScheme() == XMLScanner::Val_Always)
                    fScanner->getValidator()->emitError(XMLValid::PartialMarkupInPE);
            }
        }
         else if (fReaderMgr->getCurrentReader()->isWhitespace(nextCh))
        {
            //
            //  IF we are doing advanced callbacks and have a doc type
            //  handler, then get the whitespace and call the doc type
            //  handler with it. Otherwise, just skip whitespace.
            //
            if (fDocTypeHandler)
            {
                fReaderMgr->getSpaces(bbSpace.getBuffer());
                fDocTypeHandler->doctypeWhitespace
                (
                    bbSpace.getRawBuffer()
                    , bbSpace.getLen()
                );
            }
             else
            {
                fReaderMgr->skipPastSpaces();
            }
        }
         else
        {
            // Not valid, so emit an error
            XMLCh tmpBuf[9];
            XMLString::binToText
            (
                fReaderMgr->getNextChar()
                , tmpBuf
                , 8
                , 16
                , fMemoryManager
            );
            fScanner->emitError
            (
                XMLErrs::InvalidCharacterInIntSubset
                , tmpBuf
            );

            //
            //  If an '>', then probably an abnormally terminated
            //  internal subset so just return.
            //
            if (nextCh == chCloseAngle)
            {
                noErrors = false;
                break;
            }

            //
            //  Otherwise, try to sync back up by scanning forward for
            //  a reasonable start character.
            //
            static const XMLCh toSkip[] =
            {
                chPercent, chCloseSquare, chOpenAngle, chNull
            };
            fReaderMgr->skipUntilInOrWS(toSkip);
        }
    }

    // If we have a doc type handler, tell it the internal subset ends
    if (fDocTypeHandler)
        fDocTypeHandler->endIntSubset();

    return noErrors;
}


//
//  This method is called once we see a < in the input of an int/ext subset,
//  which indicates the start of some sort of markup.
//
void DTDScanner::scanMarkupDecl(const bool parseTextDecl)
{
    //
    //  We only have two valid first characters here. One is a ! which opens
    //  some markup decl. The other is a ?, which could begin either a PI
    //  or a text decl. If parseTextDecl is false, we cannot accept a text
    //  decl.
    //
    const XMLCh nextCh = fReaderMgr->getNextChar();

    if (nextCh == chBang)
    {
        if (fReaderMgr->skippedChar(chDash))
        {
            if (fReaderMgr->skippedChar(chDash))
            {
                scanComment();
            }
             else
            {
                fScanner->emitError(XMLErrs::CommentsMustStartWith);
                fReaderMgr->skipPastChar(chCloseAngle);
            }
        }
         else if (fReaderMgr->skippedChar(chOpenSquare))
        {
            //
            //  Its a conditional section. This is only valid in the external
            //  subset, so issue an error if we aren't there.
            //
            if (fInternalSubset)
            {
                fScanner->emitError(XMLErrs::ConditionalSectInIntSubset);
                fReaderMgr->skipPastChar(chCloseAngle);
                return;
            }

            // A PE ref can happen here, but space is not required
            checkForPERef(false, true);

            if (fReaderMgr->skippedString(XMLUni::fgIncludeString))
            {
                checkForPERef(false, true);

                // Check for the following open square bracket
                if (!fReaderMgr->skippedChar(chOpenSquare))
                    fScanner->emitError(XMLErrs::ExpectedINCLUDEBracket);

                // Get the reader we started this on
                const XMLSize_t orgReader = fReaderMgr->getCurrentReaderNum();

                checkForPERef(false, true);

                //
                //  Recurse back to the ext subset call again, telling it its
                //  in an include section.
                //
                scanExtSubsetDecl(true, false);

                //
                //  And see if we got back to the same level. If not, then its
                //  a partial markup error.
                //
                if (fReaderMgr->getCurrentReaderNum() != orgReader && fScanner->getValidationScheme() == XMLScanner::Val_Always)
                    fScanner->getValidator()->emitError(XMLValid::PartialMarkupInPE);

            }
             else if (fReaderMgr->skippedString(XMLUni::fgIgnoreString))
            {
                checkForPERef(false, true);

                // Check for the following open square bracket
                if (!fReaderMgr->skippedChar(chOpenSquare))
                    fScanner->emitError(XMLErrs::ExpectedINCLUDEBracket);

                // Get the reader we started this on
                const XMLSize_t orgReader = fReaderMgr->getCurrentReaderNum();

                // And scan over the ignored part
                scanIgnoredSection();

                //
                //  And see if we got back to the same level. If not, then its
                //  a partial markup error.
                //
                if (fReaderMgr->getCurrentReaderNum() != orgReader && fScanner->getValidationScheme() == XMLScanner::Val_Always)
                    fScanner->getValidator()->emitError(XMLValid::PartialMarkupInPE);

            }
             else
            {
                fScanner->emitError(XMLErrs::ExpectedIncOrIgn);
                fReaderMgr->skipPastChar(chCloseAngle);
            }
        }
         else if (fReaderMgr->skippedString(XMLUni::fgAttListString))
        {
            scanAttListDecl();
        }
         else if (fReaderMgr->skippedString(XMLUni::fgElemString))
        {
            scanElementDecl();
        }
         else if (fReaderMgr->skippedString(XMLUni::fgEntityString))
        {
            scanEntityDecl();
        }
         else if (fReaderMgr->skippedString(XMLUni::fgNotationString))
        {
            scanNotationDecl();
        }
         else
        {
            fScanner->emitError(XMLErrs::ExpectedMarkupDecl);
            fReaderMgr->skipPastChar(chCloseAngle);
        }
    }
     else if (nextCh == chQuestion)
    {
        // It could be a PI or the XML declaration. Check for Decl
        if (fScanner->checkXMLDecl(false))
        {
            // If we are not accepting text decls, its an error
            if (parseTextDecl)
            {
                scanTextDecl();
            }
             else
            {
                // Emit the error and skip past this markup
                fScanner->emitError(XMLErrs::TextDeclNotLegalHere);
                fReaderMgr->skipPastChar(chCloseAngle);
            }
        }
         else
        {
            // It has to be a PI
            scanPI();
        }
    }
     else
    {
        // Can't be valid so emit error and try to skip past end of this decl
        fScanner->emitError(XMLErrs::ExpectedMarkupDecl);
        fReaderMgr->skipPastChar(chCloseAngle);
    }
}


//
//  This method is called for a mixed model element's content mode. We've
//  already scanned past the '(PCDATA' part by the time we get here. So
//  everything else is element names separated by | characters until we
//  hit the end. The passed element decl's content model is filled in with
//  the information found.
//
bool DTDScanner::scanMixed(DTDElementDecl& toFill)
{
    //
    //  The terminating star is only required if there is something more
    //  than (PCDATA).
    //
    bool starRequired = false;

    // Get a buffer to be used below to get element names
    XMLBufBid bbName(fBufMgr);
    XMLBuffer& nameBuf = bbName.getBuffer();

    //
    //  Create an initial content spec node. Its just a leaf node with a
    //  PCDATA element id. This current node pointer will be pushed down the
    //  tree as we go.
    //
    ContentSpecNode* curNode = new (fGrammarPoolMemoryManager) ContentSpecNode
    (
        new (fGrammarPoolMemoryManager) QName
        (
            XMLUni::fgZeroLenString
            , XMLUni::fgZeroLenString
            , XMLElementDecl::fgPCDataElemId
            , fGrammarPoolMemoryManager
        )
        , false
        , fGrammarPoolMemoryManager
    );

    //
    //  Set the initial leaf as the temporary head. If we hit the first choice
    //  node, it will be set up here. When done, this is the node that's set
    //  as the content spec for the element.
    //
    ContentSpecNode* headNode = curNode;

    // Remember the original node so we can sense the first choice node
    ContentSpecNode* orgNode = curNode;

    //
    //  We just loop around, getting the | character at the top and then
    //  looking for the next element name. We keep up with the last node
    //  and add each new one to its right node.
    //
    while (true)
    {
        //
        //  First of all we check for some grunt work details of skipping
        //  whitespace, expand PE refs, and catching invalid reps.
        //
        if (fReaderMgr->lookingAtChar(chPercent))
        {
            // Expand it and continue
            checkForPERef(false, true);
        }
         else if (fReaderMgr->skippedChar(chAsterisk))
        {
            //
            //  Tell them they can't have reps in mixed model, but eat
            //  it and keep going if we are allowed to.
            //
            if (fScanner->emitErrorWillThrowException(XMLErrs::NoRepInMixed))
            {
                delete headNode;
            }
            fScanner->emitError(XMLErrs::NoRepInMixed);
        }
         else if (fReaderMgr->skippedSpace())
        {
            // Spaces are ok at this point, just eat them and continue
            fReaderMgr->skipPastSpaces();
        }
         else
        {
            if (!fReaderMgr->skippedChar(chPipe))
            {
                // Has to be the closing paren now.
                if (!fReaderMgr->skippedChar(chCloseParen))
                {
                    delete headNode;
                    fScanner->emitError(XMLErrs::UnterminatedContentModel, toFill.getElementName()->getLocalPart());                     
                    return false;
                }

                bool starSkipped = true;
                if (!fReaderMgr->skippedChar(chAsterisk)) {

                    starSkipped = false;

                    if (starRequired)
                    {
                        if (fScanner->emitErrorWillThrowException(XMLErrs::ExpectedAsterisk))
                        {
                            delete headNode;
                        }
                        fScanner->emitError(XMLErrs::ExpectedAsterisk);
                    }
                }

                //
                //  Create a zero or more node and make the original head
                //  node its first child.
                //
                if (starRequired || starSkipped) {
                    headNode = new (fGrammarPoolMemoryManager) ContentSpecNode
                    (
                        ContentSpecNode::ZeroOrMore
                        , headNode
                        , 0
                        , true
                        , true
                        , fGrammarPoolMemoryManager
                    );
                }

                // Store the head node as the content spec of the element.
                toFill.setContentSpec(headNode);
                break;
            }

            // Its more than just a PCDATA, so an ending star will be required now
            starRequired = true;

            // Space is legal here so check for a PE ref, but don't require space
            checkForPERef(false, true);

            // Get a name token
            if (!fReaderMgr->getName(nameBuf))
            {
                delete headNode;
                fScanner->emitError(XMLErrs::ExpectedElementName);
                return false;
            }

            //
            //  Create a leaf node for it. If we can find the element id for
            //  this element, then use it. Else, we have to fault in an element
            //  decl, marked as created because of being in a content model.
            //
            XMLElementDecl* decl = fDTDGrammar->getElemDecl(fEmptyNamespaceId, 0, nameBuf.getRawBuffer(), Grammar::TOP_LEVEL_SCOPE);
            if (!decl)
            {
                decl = new (fGrammarPoolMemoryManager) DTDElementDecl
                (
                    nameBuf.getRawBuffer()
                    , fEmptyNamespaceId
                    , DTDElementDecl::Any
                    , fGrammarPoolMemoryManager
                );
                decl->setCreateReason(XMLElementDecl::InContentModel);
                decl->setExternalElemDeclaration(isReadingExternalEntity());
                fDTDGrammar->putElemDecl(decl);
            }

            //
            //  If the current node is the original node, this is the first choice
            //  node, so create an initial choice node with the current node and
            //  the new element id. Store this as the head node.
            //
            //  Otherwise, we have to steal the right node of the previous choice
            //  and weave in another choice node there, which has the old choice
            //  as its left and the new leaf as its right.
            //
            if (curNode == orgNode)
            {
                curNode = new (fGrammarPoolMemoryManager) ContentSpecNode
                (
                    ContentSpecNode::Choice
                    , curNode
                    , new (fGrammarPoolMemoryManager) ContentSpecNode
                      (
                          decl->getElementName()
                          , fGrammarPoolMemoryManager
                      )
                    , true
                    , true
                    , fGrammarPoolMemoryManager
                );

                // Remember the top node
                headNode = curNode;
            }
             else
            {
                ContentSpecNode* oldRight = curNode->orphanSecond();
                curNode->setSecond
                (
                    new (fGrammarPoolMemoryManager) ContentSpecNode
                    (
                        ContentSpecNode::Choice
                        , oldRight
                        , new (fGrammarPoolMemoryManager) ContentSpecNode
                          (
                              decl->getElementName()
                              , fGrammarPoolMemoryManager
                          )
                        , true
                        , true
                        , fGrammarPoolMemoryManager
                    )
                );

                // Make the new right node the current node
                curNode = curNode->getSecond();
            }
        }
    }

    return true;
}


//
//  This method is called when we see a '<!NOTATION' string while scanning
//  markup decl. It parses out the notation and its id and stores a new
//  notation decl object in the notation decl pool.
//
void DTDScanner::scanNotationDecl()
{
    // Space is required here so check for a PE ref, and require space
    if (!checkForPERef(false, true))
    {
        fScanner->emitError(XMLErrs::ExpectedWhitespace);
        fReaderMgr->skipPastChar(chCloseAngle);
        return;
    }

    //
    //  And now we get a name, which is the name of the notation. Get a
    //  buffer for the name.
    //
    XMLBufBid bbName(fBufMgr);
    if (!fReaderMgr->getName(bbName.getBuffer()))
    {
        fScanner->emitError(XMLErrs::ExpectedNotationName);
        fReaderMgr->skipPastChar(chCloseAngle);
        return;
    }

    // If namespaces are enabled, then no colons allowed
    if (fScanner->getDoNamespaces())
    {
        if (XMLString::indexOf(bbName.getRawBuffer(), chColon) != -1)
            fScanner->emitError(XMLErrs::ColonNotLegalWithNS);
    }

    // Space is required here so check for a PE ref, and require space
    if (!checkForPERef(false, true))
    {
        fScanner->emitError(XMLErrs::ExpectedWhitespace);
        fReaderMgr->skipPastChar(chCloseAngle);
        return;
    }

    //
    //  And scan an external or public id. We need buffers to use for both
    //  of these.
    //
    XMLBufBid bbPubId(fBufMgr);
    XMLBufBid bbSysId(fBufMgr);
    if (!scanId(bbPubId.getBuffer(), bbSysId.getBuffer(), IDType_Either))
    {
        fReaderMgr->skipPastChar(chCloseAngle);
        return;
    }

    // We can have an optional space or PE ref here
    checkForPERef(false, true);

    //
    //  See if it already exists. If so, add it to the notatino decl pool.
    //  Otherwise, if advanced callbacks are on, create a temp one and
    //  call out for that one.
    //
    XMLNotationDecl* decl = fDTDGrammar->getNotationDecl(bbName.getRawBuffer());
    bool isIgnoring = (decl != 0);
    if (isIgnoring)
    {
        fScanner->emitError(XMLErrs::NotationAlreadyExists, bbName.getRawBuffer());
    }
     else
    {
        // Fill in a new notation declaration and add it to the pool
        const XMLCh* publicId = bbPubId.getRawBuffer();
        const XMLCh* systemId = bbSysId.getRawBuffer();
        ReaderMgr::LastExtEntityInfo lastInfo;
        fReaderMgr->getLastExtEntityInfo(lastInfo);

        decl = new (fGrammarPoolMemoryManager) XMLNotationDecl
        (
            bbName.getRawBuffer()
            , (publicId && *publicId) ? publicId : 0
            , (systemId && *systemId) ? systemId : 0
            , (lastInfo.systemId && *lastInfo.systemId) ? lastInfo.systemId : 0
            , fGrammarPoolMemoryManager
        );
        fDTDGrammar->putNotationDecl(decl);
    }

    //
    //  If we have a document type handler, then tell it about this. If we
    //  are ignoring it, only call out if advanced callbacks are enabled.
    //
    if (fDocTypeHandler)
    {
        fDocTypeHandler->notationDecl
        (
            *decl
            , isIgnoring
        );
    }

    // And one more optional space or PE ref
    checkForPERef(false, true);

    // And skip the terminating bracket
    if (!fReaderMgr->skippedChar(chCloseAngle))
        fScanner->emitError(XMLErrs::UnterminatedNotationDecl);
}


//
//  Scans a PI and calls the appropriate callbacks. A PI can happen in either
//  the document or the DTD, so it calls the appropriate handler according
//  to the fInDocument flag.
//
//  At entry we have just scanned the <? part, and need to now start on the
//  PI target name.
//
void DTDScanner::scanPI()
{
    const XMLCh* namePtr = 0;
    const XMLCh* targetPtr = 0;

    //
    //  If there are any spaces here, then warn about it. If we aren't in
    //  'first error' mode, then we'll come back and can easily pick up
    //  again by just skipping them.
    //
    if (fReaderMgr->lookingAtSpace())
    {
        fScanner->emitError(XMLErrs::PINameExpected);
        fReaderMgr->skipPastSpaces();
    }

    // Get a buffer for the PI name and scan it in
    XMLBufBid bbName(fBufMgr);
    if (!fReaderMgr->getName(bbName.getBuffer()))
    {
        fScanner->emitError(XMLErrs::PINameExpected);
        fReaderMgr->skipPastChar(chCloseAngle);
        return;
    }

    // Point the name pointer at the raw data
    namePtr = bbName.getRawBuffer();

    // See if it issome form of 'xml' and emit a warning
    //if (!XMLString::compareIString(namePtr, XMLUni::fgXMLString))
    if (bbName.getLen() == 3 &&
        (((namePtr[0] == chLatin_x) || (namePtr[0] == chLatin_X)) &&
         ((namePtr[1] == chLatin_m) || (namePtr[1] == chLatin_M)) &&
         ((namePtr[2] == chLatin_l) || (namePtr[2] == chLatin_L))))       
        fScanner->emitError(XMLErrs::NoPIStartsWithXML);

    // If namespaces are enabled, then no colons allowed
    if (fScanner->getDoNamespaces())
    {
        if (XMLString::indexOf(namePtr, chColon) != -1)
            fScanner->emitError(XMLErrs::ColonNotLegalWithNS);
    }

    //
    //  If we don't hit a space next, then the PI has no target. If we do
    //  then get out the target. Get a buffer for it as well
    //
    XMLBufBid bbTarget(fBufMgr);
    if (fReaderMgr->skippedSpace())
    {
        // Skip any leading spaces
        fReaderMgr->skipPastSpaces();

        bool gotLeadingSurrogate = false;

        // It does have a target, so lets move on to deal with that.
        while (1)
        {
            const XMLCh nextCh = fReaderMgr->getNextChar();

            // Watch for an end of file, which is always bad here
            if (!nextCh)
            {
                fScanner->emitError(XMLErrs::UnterminatedPI);
                ThrowXMLwithMemMgr(UnexpectedEOFException, XMLExcepts::Gen_UnexpectedEOF, fMemoryManager);
            }

            // Watch for potential terminating character
            if (nextCh == chQuestion)
            {
                // It must be followed by '>' to be a termination of the target
                if (fReaderMgr->skippedChar(chCloseAngle))
                    break;
            }

            // Check for correct surrogate pairs
            if ((nextCh >= 0xD800) && (nextCh <= 0xDBFF))
            {
                if (gotLeadingSurrogate)
                    fScanner->emitError(XMLErrs::Expected2ndSurrogateChar);
                else
                    gotLeadingSurrogate = true;
            }
             else
            {
                if (gotLeadingSurrogate)
                {
                    if ((nextCh < 0xDC00) || (nextCh > 0xDFFF))
                        fScanner->emitError(XMLErrs::Expected2ndSurrogateChar);
                }
                // Its got to at least be a valid XML character
                else if (!fReaderMgr->getCurrentReader()->isXMLChar(nextCh)) {

                    XMLCh tmpBuf[9];
                    XMLString::binToText
                    (
                        nextCh
                        , tmpBuf
                        , 8
                        , 16
                        , fMemoryManager
                    );
                    fScanner->emitError(XMLErrs::InvalidCharacter, tmpBuf);
                }

                gotLeadingSurrogate = false;
            }
            bbTarget.append(nextCh);
        }
    }
     else
    {
        // No target, but make sure its terminated ok
        if (!fReaderMgr->skippedChar(chQuestion))
        {
            fScanner->emitError(XMLErrs::UnterminatedPI);
            fReaderMgr->skipPastChar(chCloseAngle);
            return;
        }

        if (!fReaderMgr->skippedChar(chCloseAngle))
        {
            fScanner->emitError(XMLErrs::UnterminatedPI);
            fReaderMgr->skipPastChar(chCloseAngle);
            return;
        }
    }

    // Point the target pointer at the raw data
    targetPtr = bbTarget.getRawBuffer();

    //
    //  If we have a handler, then call it.
    //
    if (fDocTypeHandler)
    {
        fDocTypeHandler->doctypePI
        (
            namePtr
            , targetPtr
        );
    }
}


//
//  This method scans a public literal. It must be quoted and all of its
//  characters must be valid public id characters. The quotes are discarded
//  and the results are returned.
//
bool DTDScanner::scanPublicLiteral(XMLBuffer& toFill)
{
    toFill.reset();

    // Get the next char which must be a single or double quote
    XMLCh quoteCh;
    if (!fReaderMgr->skipIfQuote(quoteCh)) {
        fScanner->emitError(XMLErrs::ExpectedQuotedString);
        return false;
    }

    while (true)
    {
        const XMLCh nextCh = fReaderMgr->getNextChar();

        // Watch for EOF
        if (!nextCh)
            ThrowXMLwithMemMgr(UnexpectedEOFException, XMLExcepts::Gen_UnexpectedEOF, fMemoryManager);

        if (nextCh == quoteCh)
            break;

        //
        //  If its not a valid public id char, then report it but keep going
        //  since that's the best recovery scheme.
        //
        if (!fReaderMgr->getCurrentReader()->isPublicIdChar(nextCh))
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
            fScanner->emitError(XMLErrs::InvalidPublicIdChar, tmpBuf);
        }

        toFill.append(nextCh);
    }
    return true;
}


//
//  This method handles scanning in a quoted system literal. It expects to
//  start on the open quote and returns after eating the ending quote. There
//  are not really any restrictions on the contents of system literals.
//
bool DTDScanner::scanSystemLiteral(XMLBuffer& toFill)
{
    toFill.reset();

    // Get the next char which must be a single or double quote
    XMLCh quoteCh;
    if (!fReaderMgr->skipIfQuote(quoteCh)) {
        fScanner->emitError(XMLErrs::ExpectedQuotedString);
        return false;
    }

	XMLCh nextCh;
    // Break out on terminating quote
    while ((nextCh=fReaderMgr->getNextChar())!=quoteCh)
    {
        // Watch for EOF
        if (!nextCh)
            ThrowXMLwithMemMgr(UnexpectedEOFException, XMLExcepts::Gen_UnexpectedEOF, fMemoryManager);
        toFill.append(nextCh);
    }
    return true;
}



//
//  This method is called to scan a text decl line, which can be the first
//  line in an external entity or external subset.
//
//  On entry the <? has been scanned, and next should be 'xml' followed by
//  some whitespace, version string, etc...
//    [77] TextDecl::= '<?xml' VersionInfo? EncodingDecl S? '?>'
//
void DTDScanner::scanTextDecl()
{
    // Skip any subsequent whitespace before the version string
    fReaderMgr->skipPastSpaces();

    // Next should be the version string
    XMLBufBid bbVersion(fBufMgr);
    if (fReaderMgr->skippedString(XMLUni::fgVersionString))
    {
        if (!scanEq())
        {
            fScanner->emitError(XMLErrs::ExpectedEqSign);
            fReaderMgr->skipPastChar(chCloseAngle);
            return;
        }

        //
        //  Followed by a single or double quoted version. Get a buffer for
        //  the string.
        //
        if (!getQuotedString(bbVersion.getBuffer()))
        {
            fScanner->emitError(XMLErrs::BadXMLVersion);
            fReaderMgr->skipPastChar(chCloseAngle);
            return;
        }

        // If its not our supported version, issue an error but continue
        if (XMLString::equals(bbVersion.getRawBuffer(), XMLUni::fgVersion1_1)) {
            if (fScanner->getXMLVersion() != XMLReader::XMLV1_1)
        	    fScanner->emitError(XMLErrs::UnsupportedXMLVersion, bbVersion.getRawBuffer());
        }
        else if (!XMLString::equals(bbVersion.getRawBuffer(), XMLUni::fgVersion1_0))
            fScanner->emitError(XMLErrs::UnsupportedXMLVersion, bbVersion.getRawBuffer());
    }

    // Ok, now we must have an encoding string
    XMLBufBid bbEncoding(fBufMgr);
    fReaderMgr->skipPastSpaces();
    bool gotEncoding = false;
    if (fReaderMgr->skippedString(XMLUni::fgEncodingString))
    {
        // There must be a equal sign next
        if (!scanEq())
        {
            fScanner->emitError(XMLErrs::ExpectedEqSign);
            fReaderMgr->skipPastChar(chCloseAngle);
            return;
        }

        // Followed by a single or double quoted version string
        getQuotedString(bbEncoding.getBuffer());
        if (bbEncoding.isEmpty() || !XMLString::isValidEncName(bbEncoding.getRawBuffer()))
        {
            fScanner->emitError(XMLErrs::BadXMLEncoding, bbEncoding.getRawBuffer());
            fReaderMgr->skipPastChar(chCloseAngle);
            return;
        }

        // Indicate that we got an encoding
        gotEncoding = true;
    }

    //
    // Encoding declarations are required in the external entity
    // if there is a text declaration present
    //
    if (!gotEncoding)
    {
      fScanner->emitError(XMLErrs::EncodingRequired);
      fReaderMgr->skipPastChar(chCloseAngle);
      return;

    }

    fReaderMgr->skipPastSpaces();
    if (!fReaderMgr->skippedChar(chQuestion))
    {
        fScanner->emitError(XMLErrs::UnterminatedXMLDecl);
        fReaderMgr->skipPastChar(chCloseAngle);
    }
     else if (!fReaderMgr->skippedChar(chCloseAngle))
    {
        fScanner->emitError(XMLErrs::UnterminatedXMLDecl);
        fReaderMgr->skipPastChar(chCloseAngle);
    }

    //
    //  If we have a document type handler and advanced callbacks are on,
    //  then call the TextDecl callback
    //
    if (fDocTypeHandler)
    {
        fDocTypeHandler->TextDecl
        (
            bbVersion.getRawBuffer()
            , bbEncoding.getRawBuffer()
        );
    }

    //
    //  If we got an encoding string, then we have to call back on the reader
    //  to tell it what the encoding is.
    //
    if (!bbEncoding.isEmpty())
    {
        if (!fReaderMgr->getCurrentReader()->setEncoding(bbEncoding.getRawBuffer()))
            fScanner->emitError(XMLErrs::ContradictoryEncoding, bbEncoding.getRawBuffer());
    }
}

XERCES_CPP_NAMESPACE_END
