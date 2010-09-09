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
 * $Id: XSAXMLScanner.cpp 833045 2009-11-05 13:21:27Z borisk $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/internal/XSAXMLScanner.hpp>

#include <xercesc/sax/InputSource.hpp>
#include <xercesc/framework/XMLEntityHandler.hpp>
#include <xercesc/framework/XMLDocumentHandler.hpp>
#include <xercesc/framework/psvi/XSAnnotation.hpp>
#include <xercesc/validators/schema/SchemaValidator.hpp>


XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  XSAXMLScanner: Constructors and Destructor
// ---------------------------------------------------------------------------
XSAXMLScanner::XSAXMLScanner( GrammarResolver* const grammarResolver
                            , XMLStringPool* const   uriStringPool
                            , SchemaGrammar* const   xsaGrammar
                            , MemoryManager* const manager) :

    SGXMLScanner(0, grammarResolver, manager)
{
    fSchemaGrammar = xsaGrammar;
    setURIStringPool(uriStringPool);
}

XSAXMLScanner::~XSAXMLScanner()
{
}

// ---------------------------------------------------------------------------
//  XSAXMLScanner: SGXMLScanner virtual methods
// ---------------------------------------------------------------------------
//  This method will kick off the scanning of the primary content of the
void XSAXMLScanner::scanEndTag(bool& gotData)
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
    unsigned int uriId = fElemStack.getCurrentURI();

    // Make sure that its the end of the element that we expect
    const XMLCh *elemName = fElemStack.getCurrentSchemaElemName();
    const ElemStack::StackElem* topElem = fElemStack.popTop();
    if (!fReaderMgr.skippedStringLong(elemName))
    {
        emitError
        (
            XMLErrs::ExpectedEndOfTagX, elemName
        );
        fReaderMgr.skipPastChar(chCloseAngle);
        return;
    }

    // See if it was the root element, to avoid multiple calls below
    const bool isRoot = fElemStack.isEmpty();

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
            XMLErrs::UnterminatedEndTag, topElem->fThisElement->getFullName()
        );
    }

    //  If validation is enabled, then lets pass him the list of children and
    //  this element and let him validate it.
    if (fValidate)
    {
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

    // now we can reset the datatype buffer, since the
    // application has had a chance to copy the characters somewhere else
    ((SchemaValidator *)fValidator)->clearDatatypeBuffer();

    // If we have a doc handler, tell it about the end tag
    if (fDocHandler)
    {
        if (topElem->fPrefixColonPos != -1)
            fPrefixBuf.set(elemName, topElem->fPrefixColonPos);
        else
            fPrefixBuf.reset();
        fDocHandler->endElement
        (
            *topElem->fThisElement
            , uriId
            , isRoot
            , fPrefixBuf.getRawBuffer()
        );
    }

    // If this was the root, then done with content
    gotData = !isRoot;

    if (gotData) {

        // Restore the grammar
        fGrammar = fElemStack.getCurrentGrammar();
        fGrammarType = fGrammar->getGrammarType();
        fValidator->setGrammar(fGrammar);

        // Restore the validation flag
        fValidate = fElemStack.getValidationFlag();
    }
}

bool XSAXMLScanner::scanStartTag(bool& gotData)
{
    //  Assume we will still have data until proven otherwise. It will only
    //  ever be false if this is the root and its empty.
    gotData = true;

    // Reset element content
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
    const XMLCh* qnameRawBuf = fQNameBuf.getRawBuffer();
    bool isEmpty;
    XMLSize_t attCount = rawAttrScan(qnameRawBuf, *fRawAttrList, isEmpty);

    // save the contentleafname and currentscope before addlevel, for later use
    ContentLeafNameTypeVector* cv = 0;
    XMLContentModel* cm = 0;
    unsigned int currentScope = Grammar::TOP_LEVEL_SCOPE;
    bool laxThisOne = false;
    if (!isRoot)
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
        else {
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

    //  Make an initial pass through the list and find any xmlns attributes or
    //  schema attributes.
    if (attCount)
        scanRawAttrListforNameSpaces(attCount);

    //  Resolve the qualified name to a URI and name so that we can look up
    //  the element decl for this element. We have now update the prefix to
    //  namespace map so we should get the correct element now.
    unsigned int uriId = resolveQNameWithColon
    (
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
    XMLElementDecl* elemDecl = fGrammar->getElemDecl
    (
        uriId, nameRawBuf, qnameRawBuf, currentScope
    );

    if (!elemDecl)
    {
        // URI is different, so we try to switch grammar
        if (uriId != fURIStringPool->getId(fGrammar->getTargetNamespace())) {
            switchGrammar(getURIText(uriId), laxThisOne);
        }

        // look for a global element declaration
        elemDecl = fGrammar->getElemDecl(
            uriId, nameRawBuf, qnameRawBuf, Grammar::TOP_LEVEL_SCOPE
        );

        if (!elemDecl)
        {
            // if still not found, look in list of undeclared elements
            elemDecl = fElemNonDeclPool->getByKey(
                nameRawBuf, uriId, (int)Grammar::TOP_LEVEL_SCOPE);

            if (!elemDecl)
            {
                elemDecl = new (fMemoryManager) SchemaElementDecl
                (
                    fPrefixBuf.getRawBuffer(), nameRawBuf, uriId
                    , SchemaElementDecl::Any, Grammar::TOP_LEVEL_SCOPE
                    , fMemoryManager
                );

                elemDecl->setId (fElemNonDeclPool->put(
                      (void*)elemDecl->getBaseName(),
                      uriId,
                      (int)Grammar::TOP_LEVEL_SCOPE,
                      (SchemaElementDecl*)elemDecl));

                wasAdded = true;
            }
		}
    }

    //  We do something different here according to whether we found the
    //  element or not.
    bool bXsiTypeSet= (fValidator)?((SchemaValidator*)fValidator)->getIsXsiTypeSet():false;
    if (wasAdded || !elemDecl->isDeclared())
    {
        if (laxThisOne && !bXsiTypeSet) {
            fValidate = false;
            fElemStack.setValidationFlag(fValidate);
        }

        // If validating then emit an error
        if (fValidate)
        {
            // This is to tell the reuse Validator that this element was
            // faulted-in, was not an element in the grammar pool originally
            elemDecl->setCreateReason(XMLElementDecl::JustFaultIn);

            if(!bXsiTypeSet)
                fValidator->emitError
                (
                    XMLValid::ElementNotDefined, elemDecl->getFullName()
                );
        }
    }

    //  Now we can update the element stack to set the current element
    //  decl. We expanded the stack above, but couldn't store the element
    //  decl because we didn't know it yet.
    fElemStack.setElement(elemDecl, fReaderMgr.getCurrentReaderNum());
    fElemStack.setCurrentURI(uriId);

    if (isRoot) {
        fRootElemName = XMLString::replicate(qnameRawBuf, fMemoryManager);
    }

    //  Validate the element
    if (fValidate) {
        fValidator->validateElement(elemDecl);
    }

    // squirrel away the element's QName, so that we can do an efficient
    // end-tag match
    fElemStack.setCurrentSchemaElemName(fQNameBuf.getRawBuffer());

    ComplexTypeInfo* typeinfo = (fValidate)
        ? ((SchemaValidator*)fValidator)->getCurrentTypeInfo()
        : ((SchemaElementDecl*) elemDecl)->getComplexTypeInfo();

    if (typeinfo)
    {
        currentScope = typeinfo->getScopeDefined();

        // switch grammar if the typeinfo has a different grammar
        XMLCh* typeName = typeinfo->getTypeName();
        int comma = XMLString::indexOf(typeName, chComma);
        if (comma > 0)
        {
            XMLBufBid bbPrefix(&fBufMgr);
            XMLBuffer& prefixBuf = bbPrefix.getBuffer();

            prefixBuf.append(typeName, comma);
            switchGrammar(prefixBuf.getRawBuffer(), laxThisOne);
        }
    }
    fElemStack.setCurrentScope(currentScope);

    // Set element next state
    if (elemDepth >= fElemStateSize) {
        resizeElemState();
    }

    fElemState[elemDepth] = 0;
    fElemLoopState[elemDepth] = 0;
    fElemStack.setCurrentGrammar(fGrammar);

    //  If this is the first element and we are validating, check the root
    //  element.
    if (!isRoot && parentValidation) {
        fElemStack.addChild(elemDecl->getElementName(), true);
    }

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

    // Since the element may have default values, call start tag now regardless if it is empty or not
    // If we have a document handler, then tell it about this start tag
    if (fDocHandler)
    {
        fDocHandler->startElement
        (
            *elemDecl, uriId, fPrefixBuf.getRawBuffer(), *fAttrList
            , attCount, false, isRoot
        );
    } // may be where we output something...

    //  If empty, validate content right now if we are validating and then
    //  pop the element stack top. Else, we have to update the current stack
    //  top's namespace mapping elements.
    if (isEmpty)
    {
        // Pop the element stack back off since it'll never be used now
        fElemStack.popTop();

        // If validating, then insure that its legal to have no content
        if (fValidate)
        {
            XMLSize_t failure;
            bool res = fValidator->checkContent(elemDecl, 0, 0, &failure);
            if (!res)
            {
                // REVISIT:  in the case of xsi:type, this may
                // return the wrong string...
                fValidator->emitError
                (
                    XMLValid::ElementNotValidForContent
                    , elemDecl->getFullName()
                    , elemDecl->getFormattedContentModel()
                );
            }
        }

        // If we have a doc handler, tell it about the end tag
        if (fDocHandler)
        {
            fDocHandler->endElement
            (
                *elemDecl, uriId, isRoot, fPrefixBuf.getRawBuffer()
            );
        }

        // If the elem stack is empty, then it was an empty root
        if (isRoot) {
            gotData = false;
        }
        else
        {
            // Restore the grammar
            fGrammar = fElemStack.getCurrentGrammar();
            fGrammarType = fGrammar->getGrammarType();
            fValidator->setGrammar(fGrammar);

            // Restore the validation flag
            fValidate = fElemStack.getValidationFlag();
        }
    }

    return true;
}

// ---------------------------------------------------------------------------
//  XSAXMLScanner: XMLScanner virtual methods
// ---------------------------------------------------------------------------
//  This method will reset the scanner data structures, and related plugged
//  in stuff, for a new scan session. We get the input source for the primary
//  XML entity, create the reader for it, and push it on the stack so that
//  upon successful return from here we are ready to go.
void XSAXMLScanner::scanReset(const InputSource& src)
{
    fGrammar = fSchemaGrammar;
    fGrammarType = Grammar::SchemaGrammarType;
    fRootGrammar = fSchemaGrammar;

    fValidator->setGrammar(fGrammar);

    // Reset validation
    fValidate = true;

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
    if (fRootElemName) {
        fMemoryManager->deallocate(fRootElemName);//delete [] fRootElemName;
    }

    fRootElemName = 0;

    //  Reset the element stack, and give it the latest ids for the special
    //  URIs it has to know about.
    fElemStack.reset
    (
        fEmptyNamespaceId, fUnknownNamespaceId, fXMLNamespaceId, fXMLNSNamespaceId
    );

    if (!fSchemaNamespaceId)
        fSchemaNamespaceId  = fURIStringPool->addOrFind(SchemaSymbols::fgURI_XSI);

    // Reset some status flags
    fInException = false;
    fStandalone = false;
    fErrorCount = 0;
    fHasNoDTD = true;
    fSeeXsi = false;
    fDoNamespaces = true;
    fDoSchema = true;

    // Reset the validators
    fSchemaValidator->reset();
    fSchemaValidator->setErrorReporter(fErrorReporter);
    fSchemaValidator->setExitOnFirstFatal(fExitOnFirstFatal);
    fSchemaValidator->setGrammarResolver(fGrammarResolver);

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
    fElemCount = 0;
    if (fUIntPoolRowTotal >= 32)
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
}


void XSAXMLScanner::scanRawAttrListforNameSpaces(XMLSize_t attCount)
{
    //  Make an initial pass through the list and find any xmlns attributes or
    //  schema attributes.
    //  When we find one, send it off to be used to update the element stack's
    //  namespace mappings.
    XMLSize_t index = 0;
    for (index = 0; index < attCount; index++)
    {
        // each attribute has the prefix:suffix="value"
        const KVStringPair* curPair = fRawAttrList->elementAt(index);
        const XMLCh* rawPtr = curPair->getKey();

        //  If either the key begins with "xmlns:" or its just plain
        //  "xmlns", then use it to update the map.
        if (!XMLString::compareNString(rawPtr, XMLUni::fgXMLNSColonString, 6)
        ||  XMLString::equals(rawPtr, XMLUni::fgXMLNSString))
        {
            const XMLCh* valuePtr = curPair->getValue();

            updateNSMap(rawPtr, valuePtr, fRawAttrColonList[index]);

            // if the schema URI is seen in the the valuePtr, set the boolean seeXsi
            if (XMLString::equals(valuePtr, SchemaSymbols::fgURI_XSI)) {
                fSeeXsi = true;
            }
        }
    }

    // walk through the list again to deal with "xsi:...."
    if (fSeeXsi)
    {
        //  Schema Xsi Type yyyy (e.g. xsi:type="yyyyy")
        XMLBufBid bbXsi(&fBufMgr);
        XMLBuffer& fXsiType = bbXsi.getBuffer();

        QName attName(fMemoryManager);

        for (index = 0; index < attCount; index++)
        {
            // each attribute has the prefix:suffix="value"
            const KVStringPair* curPair = fRawAttrList->elementAt(index);
            const XMLCh* rawPtr = curPair->getKey();

            attName.setName(rawPtr, fEmptyNamespaceId);
            const XMLCh* prefPtr = attName.getPrefix();

            // if schema URI has been seen, scan for the schema location and uri
            // and resolve the schema grammar; or scan for schema type
            if (resolvePrefix(prefPtr, ElemStack::Mode_Attribute) == fSchemaNamespaceId) {

                const XMLCh* valuePtr = curPair->getValue();
                const XMLCh* suffPtr = attName.getLocalPart();

                if (XMLString::equals(suffPtr, SchemaSymbols::fgXSI_TYPE)) 
                {
                    // normalize the attribute according to schema whitespace facet
                    DatatypeValidator* tempDV = DatatypeValidatorFactory::getBuiltInRegistry()->get(SchemaSymbols::fgDT_QNAME);
                    ((SchemaValidator*) fValidator)->normalizeWhiteSpace(tempDV, valuePtr, fXsiType, true);
                }
                else if (XMLString::equals(suffPtr, SchemaSymbols::fgATT_NILL))
                {
                    // normalize the attribute according to schema whitespace facet
                    XMLBuffer& fXsiNil = fBufMgr.bidOnBuffer();
                    DatatypeValidator* tempDV = DatatypeValidatorFactory::getBuiltInRegistry()->get(SchemaSymbols::fgDT_BOOLEAN);
                    ((SchemaValidator*) fValidator)->normalizeWhiteSpace(tempDV, valuePtr, fXsiNil, true);
                    if(XMLString::equals(fXsiNil.getRawBuffer(), SchemaSymbols::fgATTVAL_TRUE))
                        ((SchemaValidator*)fValidator)->setNillable(true);
                    else if(XMLString::equals(fXsiNil.getRawBuffer(), SchemaSymbols::fgATTVAL_FALSE))
                        ((SchemaValidator*)fValidator)->setNillable(false);
                    else
                        emitError(XMLErrs::InvalidAttValue, fXsiNil.getRawBuffer(), valuePtr);
                    fBufMgr.releaseBuffer(fXsiNil);
                }
            }
        }

        if (!fXsiType.isEmpty())
        {
            int colonPos = -1;
            unsigned int uriId = resolveQName
            (
                fXsiType.getRawBuffer(), fPrefixBuf, ElemStack::Mode_Element, colonPos
            );
            ((SchemaValidator*)fValidator)->setXsiType(fPrefixBuf.getRawBuffer(), fXsiType.getRawBuffer() + colonPos + 1, uriId);
        }
    }
}

void XSAXMLScanner::switchGrammar( const XMLCh* const uriStr
                                 , bool laxValidate)
{
    Grammar* tempGrammar = 0;

    if (XMLString::equals(uriStr, SchemaSymbols::fgURI_SCHEMAFORSCHEMA)) {
        tempGrammar = fSchemaGrammar;
    }
    else {
        tempGrammar = fGrammarResolver->getGrammar(uriStr);
    }

    if (tempGrammar && tempGrammar->getGrammarType() == Grammar::SchemaGrammarType)
    {
        fGrammar = tempGrammar;
        fGrammarType = Grammar::SchemaGrammarType;
        fValidator->setGrammar(fGrammar);
    }
    else if(!laxValidate) {
        fValidator->emitError(XMLValid::GrammarNotFound, uriStr);
    }
}

XERCES_CPP_NAMESPACE_END
