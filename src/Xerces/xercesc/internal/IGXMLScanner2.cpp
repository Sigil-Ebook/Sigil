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
 * $Id: IGXMLScanner2.cpp 925236 2010-03-19 14:29:47Z borisk $
 */

// ---------------------------------------------------------------------------
//  This file holds some of the grunt work methods of IGXMLScanner.cpp to keep
//  it a little more readable.
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/internal/IGXMLScanner.hpp>
#include <xercesc/internal/EndOfEntityException.hpp>
#include <xercesc/util/UnexpectedEOFException.hpp>
#include <xercesc/util/XMLUri.hpp>
#include <xercesc/framework/LocalFileInputSource.hpp>
#include <xercesc/framework/URLInputSource.hpp>
#include <xercesc/framework/XMLDocumentHandler.hpp>
#include <xercesc/framework/XMLEntityHandler.hpp>
#include <xercesc/framework/XMLPScanToken.hpp>
#include <xercesc/framework/XMLRefInfo.hpp>
#include <xercesc/framework/XMLGrammarPool.hpp>
#include <xercesc/framework/psvi/PSVIAttributeList.hpp>
#include <xercesc/framework/psvi/PSVIElement.hpp>
#include <xercesc/framework/psvi/XSAnnotation.hpp>
#include <xercesc/validators/common/ContentLeafNameTypeVector.hpp>
#include <xercesc/validators/DTD/DTDGrammar.hpp>
#include <xercesc/validators/DTD/DTDValidator.hpp>
#include <xercesc/validators/DTD/XMLDTDDescriptionImpl.hpp>
#include <xercesc/validators/datatype/DatatypeValidator.hpp>
#include <xercesc/validators/schema/XMLSchemaDescriptionImpl.hpp>
#include <xercesc/validators/schema/SchemaGrammar.hpp>
#include <xercesc/validators/schema/SchemaValidator.hpp>
#include <xercesc/validators/schema/TraverseSchema.hpp>
#include <xercesc/validators/schema/SubstitutionGroupComparator.hpp>
#include <xercesc/validators/schema/XSDDOMParser.hpp>
#include <xercesc/validators/schema/identity/IdentityConstraintHandler.hpp>
#include <xercesc/validators/schema/identity/ValueStore.hpp>
#include <xercesc/util/XMLStringTokenizer.hpp>

XERCES_CPP_NAMESPACE_BEGIN

inline XMLAttDefList& getAttDefList(bool              isSchemaGrammar
                                  , ComplexTypeInfo*  currType
                                  , XMLElementDecl*   elemDecl);

// ---------------------------------------------------------------------------
//  IGXMLScanner: Private helper methods
// ---------------------------------------------------------------------------

//  This method is called from scanStartTagNS() to build up the list of
//  XMLAttr objects that will be passed out in the start tag callout. We
//  get the key/value pairs from the raw scan of explicitly provided attrs,
//  which have not been normalized. And we get the element declaration from
//  which we will get any defaulted or fixed attribute defs and add those
//  in as well.
XMLSize_t
IGXMLScanner::buildAttList(const  RefVectorOf<KVStringPair>&  providedAttrs
                          , const XMLSize_t                   attCount
                          ,       XMLElementDecl*             elemDecl
                          ,       RefVectorOf<XMLAttr>&       toFill)
{
    //  If doing DTD's, Ask the element to clear the 'provided' flag on all of the att defs
    //  that it owns, and to return us a boolean indicating whether it has
    //  any defs.  If schemas are being validated, the complexType
    // at the top of the SchemaValidator's stack will
    // know what's best.  REVISIT:  don't modify grammar at all; eliminate
    // this step...
    ComplexTypeInfo *currType = 0;
    DatatypeValidator *currDV = 0;
    if(fGrammar->getGrammarType() == Grammar::SchemaGrammarType && fValidate)
    {
        currType = ((SchemaValidator*)fValidator)->getCurrentTypeInfo();
        if (!currType) {
            currDV = ((SchemaValidator*)fValidator)->getCurrentDatatypeValidator();
        }
    }

    const bool hasDefs = (currType && fValidate)
            ? currType->hasAttDefs()
            : elemDecl->hasAttDefs();

    // another set of attributes; increment element counter
    fElemCount++;

    //  If there are no expliclitily provided attributes and there are no
    //  defined attributes for the element, the we don't have anything to do.
    //  So just return zero in this case.
    if (!hasDefs && !attCount)
        return 0;

    // Keep up with how many attrs we end up with total
    XMLSize_t retCount = 0;

    //  And get the current size of the output vector. This lets us use
    //  existing elements until we fill it, then start adding new ones.
    const XMLSize_t curAttListSize = toFill.size();

    //  We need a buffer into which raw scanned attribute values will be
    //  normalized.
    XMLBufBid bbNormal(&fBufMgr);
    XMLBuffer& normBuf = bbNormal.getBuffer();

    //
    // Decide if to use hash table to do duplicate checking
    //
    bool toUseHashTable = false;
    if (fGrammarType == Grammar::DTDGrammarType)
    {
        setAttrDupChkRegistry(attCount, toUseHashTable);
    }

    XMLBufBid bbPrefix(&fBufMgr);
    XMLBuffer& prefixBuf = bbPrefix.getBuffer();

    //  Loop through our explicitly provided attributes, which are in the raw
    //  scanned form, and build up XMLAttr objects.
    XMLSize_t index;
    const XMLCh* prefPtr, *suffPtr;
    for (index = 0; index < attCount; index++)
    {
        PSVIItem::VALIDITY_STATE attrValid = PSVIItem::VALIDITY_VALID;
        PSVIItem::ASSESSMENT_TYPE attrAssessed = PSVIItem::VALIDATION_FULL;
        const KVStringPair* curPair = providedAttrs.elementAt(index);

        //  We have to split the name into its prefix and name parts. Then
        //  we map the prefix to its URI.
        const XMLCh* const namePtr = curPair->getKey();

        const int colonInd = fRawAttrColonList[index];
        unsigned int uriId;
        if (colonInd != -1)
        {
            prefixBuf.set(namePtr, colonInd);
            prefPtr = prefixBuf.getRawBuffer();
            suffPtr = namePtr + colonInd + 1;
            //  Map the prefix to a URI id
            uriId = resolvePrefix(prefPtr, ElemStack::Mode_Attribute);
        }
        else
        {
            // No colon, so we just have a name with no prefix
            prefPtr = XMLUni::fgZeroLenString;
            suffPtr = namePtr;
            // an empty prefix is always the empty namespace, when dealing with attributes
            uriId = fEmptyNamespaceId;
        }

        //  If the uri comes back as the xmlns or xml URI or its just a name
        //  and that name is 'xmlns', then we handle it specially. So set a
        //  boolean flag that lets us quickly below know which we are dealing
        //  with.
        const bool isNSAttr = (uriId == fEmptyNamespaceId)?
                                XMLString::equals(suffPtr, XMLUni::fgXMLNSString) :
                                (uriId == fXMLNSNamespaceId || XMLString::equals(getURIText(uriId), SchemaSymbols::fgURI_XSI));


        //  If its not a special case namespace attr of some sort, then we
        //  do normal checking and processing.
        XMLAttDef::AttTypes attType = XMLAttDef::CData;
        DatatypeValidator *attrValidator = 0;
        PSVIAttribute *psviAttr = 0;
        bool otherXSI = false;

        if (isNSAttr && fGrammarType == Grammar::SchemaGrammarType)
        {
            if(!fUndeclaredAttrRegistry->putIfNotPresent(suffPtr, uriId))
            {
                emitError
                (
                    XMLErrs::AttrAlreadyUsedInSTag
                    , namePtr
                    , elemDecl->getFullName()
                );
                fPSVIElemContext.fErrorOccurred = true;
            }
            else
            {
                bool ValueValidate = false;
                bool tokenizeBuffer = false;

                if (uriId == fXMLNSNamespaceId)
                {
                    attrValidator = DatatypeValidatorFactory::getBuiltInRegistry()->get(SchemaSymbols::fgDT_ANYURI);
                }
                else if (XMLString::equals(getURIText(uriId), SchemaSymbols::fgURI_XSI))
                {
                    if (XMLString::equals(suffPtr, SchemaSymbols::fgATT_NILL))
                    {
                        attrValidator = DatatypeValidatorFactory::getBuiltInRegistry()->get(SchemaSymbols::fgDT_BOOLEAN);

                        ValueValidate = true;
                    }
                    else if (XMLString::equals(suffPtr, SchemaSymbols::fgXSI_SCHEMALOCATION))
                    {
                        // use anyURI as the validator
                        // tokenize the data and use the anyURI data for each piece
                        attrValidator = DatatypeValidatorFactory::getBuiltInRegistry()->get(SchemaSymbols::fgDT_ANYURI);
                        //We should validate each value in the schema location however
                        //this lead to a performance degradation of around 4%.  Since
                        //the first value of each pair needs to match what is in the
                        //schema document and the second value needs to be valid in
                        //order to open the document we won't validate it.  Need to
                        //do performance analysis of the anyuri datatype.
                        //ValueValidate = true;
                        ValueValidate = false;
                        tokenizeBuffer = true;
                    }
                    else if (XMLString::equals(suffPtr, SchemaSymbols::fgXSI_NONAMESPACESCHEMALOCATION))
                    {
                        attrValidator = DatatypeValidatorFactory::getBuiltInRegistry()->get(SchemaSymbols::fgDT_ANYURI);
                        //We should validate this value however
                        //this lead to a performance degradation of around 4%.  Since
                        //the value needs to be valid in
                        //order to open the document we won't validate it.  Need to
                        //do performance analysis of the anyuri datatype.
                        //ValueValidate = true;
                        ValueValidate = false;
                    }
                    else if (XMLString::equals(suffPtr, SchemaSymbols::fgXSI_TYPE))
                    {
                        attrValidator = DatatypeValidatorFactory::getBuiltInRegistry()->get(SchemaSymbols::fgDT_QNAME);

                        ValueValidate = true;
                    }
                    else {
                        otherXSI = true;
                    }
                }

                if (!otherXSI) {
                    normalizeAttRawValue
                    (
                        namePtr
                        , curPair->getValue()
                        , normBuf
                    );

                    if (fValidate && attrValidator && ValueValidate)
                    {
                        ((SchemaValidator*) fValidator)->normalizeWhiteSpace(attrValidator, normBuf.getRawBuffer(), normBuf, true);

                        ValidationContext* const    theContext =
                            getValidationContext();

                        if (theContext)
                        {
                            try
                            {
                                if (tokenizeBuffer) {
                                    XMLStringTokenizer tokenizer(normBuf.getRawBuffer(), fMemoryManager);
                                    while (tokenizer.hasMoreTokens()) {
                                        attrValidator->validate(
                                            tokenizer.nextToken(),
                                            theContext,
                                            fMemoryManager);
                                    }
                                }
                                else {
                                    attrValidator->validate(
                                        normBuf.getRawBuffer(),
                                        theContext,
                                        fMemoryManager);
                                }
                            }
                            catch (const XMLException& idve)
                            {
                                fValidator->emitError (XMLValid::DatatypeError, idve.getCode(), idve.getMessage());
                            }
                        }
                    }

                    if(getPSVIHandler())
                    {
                        psviAttr = fPSVIAttrList->getPSVIAttributeToFill(suffPtr, fURIStringPool->getValueForId(uriId));
                        XSSimpleTypeDefinition *validatingType = (attrValidator)
                            ? (XSSimpleTypeDefinition *)fModel->getXSObject(attrValidator)
                            : 0;
                        // no attribute declarations for these...
                        psviAttr->reset(
                            fRootElemName
                            , PSVIItem::VALIDITY_NOTKNOWN
                            , PSVIItem::VALIDATION_NONE
                            , validatingType
                            , 0
                            , 0
                            , false
                            , 0
                            , attrValidator
                            );
                    }
                }
            }
        }

        if (!isNSAttr || fGrammarType == Grammar::DTDGrammarType || otherXSI)
        {
            // Some checking for attribute wild card first (for schema)
            bool laxThisOne = false;
            bool skipThisOne = false;

            XMLAttDef* attDefForWildCard = 0;
            XMLAttDef*  attDef = 0;

            if (fGrammarType == Grammar::SchemaGrammarType) {

                //retrieve the att def
                SchemaAttDef* attWildCard = 0;
                if (currType) {
                    attDef = currType->getAttDef(suffPtr, uriId);
                    attWildCard = currType->getAttWildCard();
                }
                else if (!currDV) { // check explicitly-set wildcard
                    attDef = ((SchemaElementDecl*)elemDecl)->getAttDef(suffPtr, uriId);
                    attWildCard = ((SchemaElementDecl*)elemDecl)->getAttWildCard();
                }

                // if not found or faulted in - check for a matching wildcard attribute
                // if no matching wildcard attribute, check (un)qualifed cases and flag
                // appropriate errors
                if (!attDef || (attDef->getCreateReason() == XMLAttDef::JustFaultIn)) {

                    if (attWildCard) {
                        //if schema, see if we should lax or skip the validation of this attribute
                        if (anyAttributeValidation(attWildCard, uriId, skipThisOne, laxThisOne)) {

                            if(!skipThisOne)
                            {
                                SchemaGrammar* sGrammar = (SchemaGrammar*) fGrammarResolver->getGrammar(getURIText(uriId));
                                if (sGrammar && sGrammar->getGrammarType() == Grammar::SchemaGrammarType) {
                                    RefHashTableOf<XMLAttDef>* attRegistry = sGrammar->getAttributeDeclRegistry();
                                    if (attRegistry) {
                                        attDefForWildCard = attRegistry->get(suffPtr);
                                    }
                                }
                            }
                        }
                    }
                    else if (currType) {
                        // not found, see if the attDef should be qualified or not
                        if (uriId == fEmptyNamespaceId) {
                            attDef = currType->getAttDef(suffPtr
                                            , fURIStringPool->getId(fGrammar->getTargetNamespace()));
                            if (fValidate
                                && attDef
                                && attDef->getCreateReason() != XMLAttDef::JustFaultIn) {
                                // the attribute should be qualified
                                fValidator->emitError
                                (
                                    XMLValid::AttributeNotQualified
                                    , attDef->getFullName()
                                );
                                if(fGrammarType == Grammar::SchemaGrammarType) {
                                    fPSVIElemContext.fErrorOccurred = true;
                                    if (getPSVIHandler())
                                    {
                                        attrValid = PSVIItem::VALIDITY_INVALID;
                                    }
                                }
                            }
                        }
                        else {
                            attDef = currType->getAttDef(suffPtr
                                            , fEmptyNamespaceId);
                            if (fValidate
                                && attDef
                                && attDef->getCreateReason() != XMLAttDef::JustFaultIn) {
                                // the attribute should be qualified
                                fValidator->emitError
                                (
                                    XMLValid::AttributeNotUnQualified
                                    , attDef->getFullName()
                                );
                                if(fGrammarType == Grammar::SchemaGrammarType) {
                                    fPSVIElemContext.fErrorOccurred = true;
                                    if (getPSVIHandler())
                                    {
                                        attrValid = PSVIItem::VALIDITY_INVALID;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            //  Find this attribute within the parent element. We pass both
            //  the uriID/name and the raw QName buffer, since we don't know
            //  how the derived validator and its elements store attributes.
            else
            {
                if(fGrammarType == Grammar::DTDGrammarType)
                    attDef = ((DTDElementDecl *)elemDecl)->getAttDef ( namePtr);
            }

            // now need to prepare for duplicate detection
            if(attDef)
            {
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
                    fPSVIElemContext.fErrorOccurred = true;
                }
            }
            else
            {
                if(fGrammarType == Grammar::DTDGrammarType)
                {
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
                else // schema grammar
                {
                    if(!fUndeclaredAttrRegistry->putIfNotPresent(suffPtr, uriId))
                    {
                        emitError
                        (
                            XMLErrs::AttrAlreadyUsedInSTag
                            , namePtr
                            , elemDecl->getFullName()
                        );
                        fPSVIElemContext.fErrorOccurred = true;
                    }
                }
            }

            if(fGrammarType == Grammar::SchemaGrammarType )
            {
                // if we've found either an attDef or an attDefForWildCard,
                // then we're doing full validation and it may still be valid.
                if(!attDef && !attDefForWildCard)
                {
                    if(!laxThisOne && !skipThisOne)
                    {
                        fPSVIElemContext.fErrorOccurred = true;
                    }
                    if(getPSVIHandler())
                    {
                        if(!laxThisOne && !skipThisOne)
                        {
                            attrValid = PSVIItem::VALIDITY_INVALID;
                        }
                        else if(laxThisOne)
                        {
                            attrValid = PSVIItem::VALIDITY_NOTKNOWN;
                            attrAssessed = PSVIItem::VALIDATION_PARTIAL;
                        }
                        else
                        {
                            attrValid = PSVIItem::VALIDITY_NOTKNOWN;
                            attrAssessed = PSVIItem::VALIDATION_NONE;
                        }
                    }
                }
            }

            bool errorCondition = fValidate && !attDefForWildCard && !attDef;
            if (errorCondition && !skipThisOne && !laxThisOne)
            {
                //
                //  Its not valid for this element, so issue an error if we are
                //  validating.
                //
                XMLBufBid bbMsg(&fBufMgr);
                XMLBuffer& bufMsg = bbMsg.getBuffer();
                if (uriId != fEmptyNamespaceId) {
                    XMLBufBid bbURI(&fBufMgr);
                    XMLBuffer& bufURI = bbURI.getBuffer();

                    getURIText(uriId, bufURI);

                    bufMsg.append(chOpenCurly);
                    bufMsg.append(bufURI.getRawBuffer());
                    bufMsg.append(chCloseCurly);
                }
                bufMsg.append(suffPtr);
                fValidator->emitError
                (
                    XMLValid::AttNotDefinedForElement
                    , bufMsg.getRawBuffer()
                    , elemDecl->getFullName()
                );
            }

            //  Now normalize the raw value since we have the attribute type. We
            //  don't care about the return status here. If it failed, an error
            //  was issued, which is all we care about.
            if (attDefForWildCard) {
                normalizeAttValue(
                    attDefForWildCard, namePtr, curPair->getValue(), normBuf
                );

                //  If we found an attdef for this one, then lets validate it.
                const XMLCh* xsNormalized = normBuf.getRawBuffer();
                DatatypeValidator* tempDV = ((SchemaAttDef*) attDefForWildCard)->getDatatypeValidator();
                if (tempDV && tempDV->getWSFacet() != DatatypeValidator::PRESERVE)
                {
                    // normalize the attribute according to schema whitespace facet
                    ((SchemaValidator*) fValidator)->normalizeWhiteSpace(tempDV, xsNormalized, fWSNormalizeBuf, true);
                    xsNormalized = fWSNormalizeBuf.getRawBuffer();

                    if (fNormalizeData && fValidate) {
                        normBuf.set(xsNormalized);
                    }
                }

                if (fValidate ) {
                    fValidator->validateAttrValue(
                        attDefForWildCard, xsNormalized, false, elemDecl
                    );
                    attrValidator = ((SchemaValidator*)fValidator)->getMostRecentAttrValidator();
                    if(((SchemaValidator *)fValidator)->getErrorOccurred())
                    {
                        fPSVIElemContext.fErrorOccurred = true;
                        if(getPSVIHandler())
                            attrValid = PSVIItem::VALIDITY_INVALID;
                    }
                }
                else { // no decl; default DOMTypeInfo to anySimpleType
                    attrValidator = DatatypeValidatorFactory::getBuiltInRegistry()->get(SchemaSymbols::fgDT_ANYSIMPLETYPE);
                }

                // Save the type for later use
                attType = attDefForWildCard->getType();
            }
            else {
                normalizeAttValue(
                    attDef, namePtr, curPair->getValue(), normBuf
                );

                //  If we found an attdef for this one, then lets validate it.
                if (attDef)
                {
                    const XMLCh* xsNormalized = normBuf.getRawBuffer();
                    if (fGrammarType == Grammar::SchemaGrammarType)
                    {
                        DatatypeValidator* tempDV = ((SchemaAttDef*) attDef)->getDatatypeValidator();
                        if (tempDV && tempDV->getWSFacet() != DatatypeValidator::PRESERVE)
                        {
                            // normalize the attribute according to schema whitespace facet
                            ((SchemaValidator*) fValidator)->normalizeWhiteSpace(tempDV, xsNormalized, fWSNormalizeBuf, true);
                            xsNormalized = fWSNormalizeBuf.getRawBuffer();
                            if (fNormalizeData && fValidate && !skipThisOne) {
                                normBuf.set(xsNormalized);
                            }
                        }
                    }

                    if (fValidate && !skipThisOne)
                    {
                        fValidator->validateAttrValue(
                            attDef, xsNormalized, false, elemDecl
                        );

                        if(fGrammarType == Grammar::SchemaGrammarType)
                        {
                            attrValidator = ((SchemaValidator*)fValidator)->getMostRecentAttrValidator();
                            if(((SchemaValidator *)fValidator)->getErrorOccurred())
                            {
                                fPSVIElemContext.fErrorOccurred = true;
                                if (getPSVIHandler())
                                    attrValid = PSVIItem::VALIDITY_INVALID;
                            }
                        }
                    }
                    else if(fGrammarType == Grammar::SchemaGrammarType) {
                        attrValidator = DatatypeValidatorFactory::getBuiltInRegistry()->get(SchemaSymbols::fgDT_ANYSIMPLETYPE);
                    }
                }
                else // no attDef at all; default to anySimpleType
                {
                    if(fGrammarType == Grammar::SchemaGrammarType) {
                        attrValidator = DatatypeValidatorFactory::getBuiltInRegistry()->get(SchemaSymbols::fgDT_ANYSIMPLETYPE);
                    }
                }

                // Save the type for later use
                if (attDef)
                {
                    attType = attDef->getType();
                }
            }

            // now fill in the PSVIAttributes entry for this attribute:
            if(getPSVIHandler() && fGrammarType == Grammar::SchemaGrammarType)
            {
                psviAttr = fPSVIAttrList->getPSVIAttributeToFill(suffPtr, fURIStringPool->getValueForId(uriId));
                SchemaAttDef *actualAttDef = 0;
                if(attDef)
                    actualAttDef = (SchemaAttDef *)attDef;
                else if (attDefForWildCard)
                    actualAttDef = (SchemaAttDef *)attDefForWildCard;
                if(actualAttDef)
                {
                    XSAttributeDeclaration *attrDecl = (XSAttributeDeclaration *)fModel->getXSObject(actualAttDef);
                    DatatypeValidator * attrDataType = actualAttDef->getDatatypeValidator();
                    XSSimpleTypeDefinition *validatingType = (XSSimpleTypeDefinition *)fModel->getXSObject(attrDataType);
                    if(attrValid != PSVIItem::VALIDITY_VALID)
                    {
                        psviAttr->reset
                        (
                            fRootElemName
                            , attrValid
                            , attrAssessed
                            , validatingType
                            , 0
                            , actualAttDef->getValue()
                            , false
                            , attrDecl
                            , 0
                        );
                    }
                    else
                    {
                        XSSimpleTypeDefinition *memberType = 0;
                        if(validatingType->getVariety() == XSSimpleTypeDefinition::VARIETY_UNION)
                            memberType = (XSSimpleTypeDefinition *)fModel->getXSObject(attrValidator);
                        psviAttr->reset
                        (
                            fRootElemName
                            , attrValid
                            , attrAssessed
                            , validatingType
                            , memberType
                            , actualAttDef->getValue()
                            , false
                            , attrDecl
                            , (memberType)?attrValidator:attrDataType
                        );
                    }
                }
                else
                {
                    psviAttr->reset
                    (
                        fRootElemName
                        , attrValid
                        , attrAssessed
                        , 0
                        , 0
                        , 0
                        , false
                        , 0
                        , 0
                    );
                }
            }
        }

        //  Add this attribute to the attribute list that we use to pass them
        //  to the handler. We reuse its existing elements but expand it as
        //  required.
        XMLAttr* curAttr;

        // check for duplicate namespace attributes:
        // by checking for qualified names with the same local part and with prefixes
        // which have been bound to namespace names that are identical.
        if (fGrammarType == Grammar::DTDGrammarType) {
            if (!toUseHashTable)
            {
                for (XMLSize_t attrIndex=0; attrIndex < retCount; attrIndex++) {
                    curAttr = toFill.elementAt(attrIndex);
                    if (uriId == curAttr->getURIId() &&
                        XMLString::equals(suffPtr, curAttr->getName())) {
                        emitError
                        (

                         XMLErrs::AttrAlreadyUsedInSTag
                        , curAttr->getName()
                        , elemDecl->getFullName()
                        );
                    }
                }
            }
            else
            {
                if (fAttrDupChkRegistry->containsKey((void*)suffPtr, uriId))
                {
                    emitError
                        (
                        XMLErrs::AttrAlreadyUsedInSTag
                        , suffPtr
                        , elemDecl->getFullName()
                        );
                }
            }
        }

        if (retCount >= curAttListSize)
        {
            curAttr = new (fMemoryManager) XMLAttr
            (
                uriId
                , suffPtr
                , prefPtr
                , normBuf.getRawBuffer()
                , attType
                , true
                , fMemoryManager
            );
            toFill.addElement(curAttr);
        }
        else
        {
            curAttr = toFill.elementAt(retCount);
            curAttr->set
            (
                uriId
                , suffPtr
                , prefPtr
                , normBuf.getRawBuffer()
                , attType
            );
            curAttr->setSpecified(true);
        }

        if (toUseHashTable)
        {
            fAttrDupChkRegistry->put((void*)suffPtr, uriId, curAttr);
        }

        if(psviAttr)
            psviAttr->setValue(curAttr->getValue());

        // Bump the count of attrs in the list
        retCount++;
    }

    //  Now, if there are any attributes declared by this element, let's
    //  go through them and make sure that any required ones are provided,
    //  and fault in any fixed ones and defaulted ones that are not provided
    //  literally.
    if (hasDefs)
    {
        // Check after all specified attrs are scanned
        // (1) report error for REQUIRED attrs that are missing (V_TAGc)
        // (2) add default attrs if missing (FIXED and NOT_FIXED)


        XMLAttDefList &attDefList = getAttDefList(fGrammarType == Grammar::SchemaGrammarType, currType, elemDecl);

        for(XMLSize_t i=0; i<attDefList.getAttDefCount(); i++)
        {
            // Get the current att def, for convenience and its def type
            const XMLAttDef *curDef = &attDefList.getAttDef(i);
            const XMLAttDef::DefAttTypes defType = curDef->getDefaultType();
            unsigned int *attCountPtr = fAttDefRegistry->get((void *)curDef);
            if (!attCountPtr || *attCountPtr < fElemCount)
            { // did not occur
                // note that since there is no attribute information
                // item present, there is no PSVI infoset to augment here *except*
                // that the element is invalid

                //the attribute is not provided
                if (fValidate)
                {
                    // If we are validating and its required, then an error
                    if ((defType == XMLAttDef::Required) ||
                        (defType == XMLAttDef::Required_And_Fixed)  )

                    {
                        fValidator->emitError
                        (
                            XMLValid::RequiredAttrNotProvided
                            , curDef->getFullName()
                        );
                        if(fGrammarType == Grammar::SchemaGrammarType)
                        {
                            fPSVIElemContext.fErrorOccurred = true;
                        }
                    }
                    else if ((defType == XMLAttDef::Default) ||
                            (defType == XMLAttDef::Fixed)  )
                    {
                        if (fStandalone && curDef->isExternal())
                        {
                            // XML 1.0 Section 2.9
                            // Document is standalone, so attributes must not be defaulted.
                            fValidator->emitError(XMLValid::NoDefAttForStandalone, curDef->getFullName(), elemDecl->getFullName());
                            if(fGrammarType == Grammar::SchemaGrammarType)
                            {
                                fPSVIElemContext.fErrorOccurred = true;
                            }
                        }
                    }
                }

                //  Fault in the value if needed, and bump the att count.
                if ((defType == XMLAttDef::Default)
                    ||  (defType == XMLAttDef::Fixed))
                {
                    // Let the validator pass judgement on the attribute value
                    if (fValidate)
                    {
                        fValidator->validateAttrValue
                        (
                            curDef
                            , curDef->getValue()
                            , false
                            , elemDecl
                        );
                    }

                    XMLAttr* curAtt;
                    if (retCount >= curAttListSize)
                    {
                        curAtt = new (fMemoryManager) XMLAttr(fMemoryManager);
                        fValidator->faultInAttr(*curAtt, *curDef);
                        fAttrList->addElement(curAtt);
                    }
                    else
                    {
                        curAtt = fAttrList->elementAt(retCount);
                        fValidator->faultInAttr(*curAtt, *curDef);
                    }

                    if (fGrammarType == Grammar::DTDGrammarType)
                    {
                        //  Map the new attribute's prefix to a URI id and store
                        //  that in the attribute object.
                        curAtt->setURIId
                        (
                            resolvePrefix(curAtt->getPrefix(), ElemStack::Mode_Attribute)
                        );
                    }

                    // Indicate it was not explicitly specified and bump count
                    curAtt->setSpecified(false);
                    retCount++;
                    if(getPSVIHandler() && fGrammarType == Grammar::SchemaGrammarType)
                    {
                        QName *attName = ((SchemaAttDef *)curDef)->getAttName();
                        PSVIAttribute *defAttrToFill = fPSVIAttrList->getPSVIAttributeToFill
                        (
                            attName->getLocalPart(), fURIStringPool->getValueForId( attName->getURI())
                        );
                        XSAttributeDeclaration *defAttrDecl = (XSAttributeDeclaration *)fModel->getXSObject((void *)curDef);
                        DatatypeValidator * attrDataType = ((SchemaAttDef *)curDef)->getDatatypeValidator();
                        XSSimpleTypeDefinition *defAttrType =
                            (XSSimpleTypeDefinition*)fModel->getXSObject(attrDataType);
                        // would have occurred during validation of default value
                        if(((SchemaValidator *)fValidator)->getErrorOccurred())
                        {
                            defAttrToFill->reset(
                                fRootElemName
                                , PSVIItem::VALIDITY_INVALID
                                , PSVIItem::VALIDATION_FULL
                                , defAttrType
                                , 0
                                , curDef->getValue()
                                , true
                                , defAttrDecl
                                , 0
                            );
                        }
                        else
                        {
                            XSSimpleTypeDefinition *defAttrMemberType = 0;
                            if(defAttrType->getVariety() == XSSimpleTypeDefinition::VARIETY_UNION)
                            {
                                defAttrMemberType = (XSSimpleTypeDefinition *)fModel->getXSObject
                                (
                                    ((SchemaValidator*)fValidator)->getMostRecentAttrValidator()
                                );
                            }
                            defAttrToFill->reset(
                                fRootElemName
                                , PSVIItem::VALIDITY_VALID
                                , PSVIItem::VALIDATION_FULL
                                , defAttrType
                                , defAttrMemberType
                                , curDef->getValue()
                                , true
                                , defAttrDecl
                                , (defAttrMemberType)?((SchemaValidator *)fValidator)->getMostRecentAttrValidator():attrDataType
                            );
                        }
                        defAttrToFill->setValue(curDef->getValue());
                    }
                }
            }
            else if(attCountPtr)
            {
                //attribute is provided
                // (schema) report error for PROHIBITED attrs that are present (V_TAGc)
                if (defType == XMLAttDef::Prohibited && fValidate)
                {
                    fValidator->emitError
                    (
                        XMLValid::ProhibitedAttributePresent
                        , curDef->getFullName()
                    );
                    if(fGrammarType == Grammar::SchemaGrammarType)
                    {
                        fPSVIElemContext.fErrorOccurred = true;
                        if (getPSVIHandler())
                        {
                            QName *attQName = ((SchemaAttDef *)curDef)->getAttName();
                            // bad luck...
                            PSVIAttribute *prohibitedAttr = fPSVIAttrList->getAttributePSVIByName
                            (
                                attQName->getLocalPart(),
                                fURIStringPool->getValueForId(attQName->getURI())
                            );
                            prohibitedAttr->updateValidity(PSVIItem::VALIDITY_INVALID);
                        }
                    }
                }
            }
        }
    }
    return retCount;
}


//  This method will take a raw attribute value and normalize it according to
//  the rules of the attribute type. It will put the resulting value into the
//  passed buffer.
//
//  This code assumes that escaped characters in the original value (via char
//  refs) are prefixed by a 0xFFFF character. This is because some characters
//  are legal if escaped only. And some escape chars are not subject to
//  normalization rules.
bool IGXMLScanner::normalizeAttValue( const   XMLAttDef* const    attDef
                                      , const XMLCh* const        attName
                                      , const XMLCh* const        value
                                      ,       XMLBuffer&          toFill)
{
    // A simple state value for a whitespace processing state machine
    enum States
    {
        InWhitespace
        , InContent
    };

    // Get the type and name
    const XMLAttDef::AttTypes type = (attDef)?attDef->getType():XMLAttDef::CData;

    // Assume its going to go fine, and empty the target buffer in preperation
    bool retVal = true;
    toFill.reset();

    //  Loop through the chars of the source value and normalize it according
    //  to the type.
    XMLCh nextCh;
    const XMLCh* srcPtr = value;

    if (type == XMLAttDef::CData || type > XMLAttDef::Notation) {
        //  Get the next character from the source. We have to watch for
        //  escaped characters (which are indicated by a 0xFFFF value followed
        //  by the char that was escaped.)
        while ((nextCh = *srcPtr++)!=0)
        {
            switch(nextCh)
            {
            // Do we have an escaped character ?
            case 0xFFFF:
                nextCh = *srcPtr++;
                break;
            case 0x09:
            case 0x0A:
            case 0x0D:
                // Check Validity Constraint for Standalone document declaration
                // XML 1.0, Section 2.9
                if (fStandalone && fValidate && attDef && attDef->isExternal())
                {
                     // Can't have a standalone document declaration of "yes" if  attribute
                     // values are subject to normalisation
                     fValidator->emitError(XMLValid::NoAttNormForStandalone, attName);
                }
                nextCh = chSpace;
                break;
            case chOpenAngle:
                //  If its not escaped, then make sure its not a < character, which is
                //  not allowed in attribute values.
                emitError(XMLErrs::BracketInAttrValue, attName);
                retVal = false;
                break;
            }

            // Add this char to the target buffer
            toFill.append(nextCh);
        }
    }
    else {
        States curState = InContent;
        bool firstNonWS = false;
        //  Get the next character from the source. We have to watch for
        //  escaped characters (which are indicated by a 0xFFFF value followed
        //  by the char that was escaped.)
        while ((nextCh = *srcPtr)!=0)
        {
            // Do we have an escaped character ?
            if (nextCh == 0xFFFF)
            {
                nextCh = *++srcPtr;
            }
            else if (nextCh == chOpenAngle) {
                //  If its not escaped, then make sure its not a < character, which is
                //  not allowed in attribute values.
                emitError(XMLErrs::BracketInAttrValue, attName);
                retVal = false;
            }

            if (curState == InWhitespace)
            {
                if (!fReaderMgr.getCurrentReader()->isWhitespace(nextCh))
                {
                    if (firstNonWS)
                        toFill.append(chSpace);
                    curState = InContent;
                    firstNonWS = true;
                }
                else
                {
                    srcPtr++;
                    continue;
                }
            }
            else if (curState == InContent)
            {
                if (fReaderMgr.getCurrentReader()->isWhitespace(nextCh))
                {
                    curState = InWhitespace;
                    srcPtr++;

                    // Check Validity Constraint for Standalone document declaration
                    // XML 1.0, Section 2.9
                    if (fStandalone && fValidate && attDef && attDef->isExternal())
                    {
                        if (!firstNonWS || (nextCh != chSpace) || (!*srcPtr) || fReaderMgr.getCurrentReader()->isWhitespace(*srcPtr))
                        {
                            // Can't have a standalone document declaration of "yes" if  attribute
                            // values are subject to normalisation
                            fValidator->emitError(XMLValid::NoAttNormForStandalone, attName);
                        }
                    }
                    continue;
                }
                firstNonWS = true;
            }

            // Add this char to the target buffer
            toFill.append(nextCh);

            // And move up to the next character in the source
            srcPtr++;
        }
    }

    return retVal;
}

//  This method will just normalize the input value as CDATA without
//  any standalone checking.
bool IGXMLScanner::normalizeAttRawValue( const   XMLCh* const        attrName
                                      , const XMLCh* const        value
                                      ,       XMLBuffer&          toFill)
{
    // Assume its going to go fine, and empty the target buffer in preperation
    bool retVal = true;
    toFill.reset();

    //  Loop through the chars of the source value and normalize it according
    //  to the type.
    bool escaped;
    XMLCh nextCh;
    const XMLCh* srcPtr = value;
    while (*srcPtr)
    {
        //  Get the next character from the source. We have to watch for
        //  escaped characters (which are indicated by a 0xFFFF value followed
        //  by the char that was escaped.)
        nextCh = *srcPtr;
        escaped = (nextCh == 0xFFFF);
        if (escaped)
            nextCh = *++srcPtr;

        //  If its not escaped, then make sure its not a < character, which is
        //  not allowed in attribute values.
        if (!escaped && (*srcPtr == chOpenAngle))
        {
            emitError(XMLErrs::BracketInAttrValue, attrName);
            retVal = false;
        }

        if (!escaped)
        {
            //  NOTE: Yes this is a little redundant in that a 0x20 is
            //  replaced with an 0x20. But its faster to do this (I think)
            //  than checking for 9, A, and D separately.
            if (fReaderMgr.getCurrentReader()->isWhitespace(nextCh))
                nextCh = chSpace;
        }

        // Add this char to the target buffer
        toFill.append(nextCh);

        // And move up to the next character in the source
        srcPtr++;
    }
    return retVal;
}

//  This method will reset the scanner data structures, and related plugged
//  in stuff, for a new scan session. We get the input source for the primary
//  XML entity, create the reader for it, and push it on the stack so that
//  upon successful return from here we are ready to go.
void IGXMLScanner::scanReset(const InputSource& src)
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

    // Clear transient schema info list.
    //
    fSchemaInfoList->removeAll ();

    // fModel may need updating, as fGrammarResolver could have cleaned it
    if(fModel && getPSVIHandler())
        fModel = fGrammarResolver->getXSModel();

    {
        XMLDTDDescriptionImpl   theDTDDescription(XMLUni::fgDTDEntityString, fMemoryManager);
        fDTDGrammar = (DTDGrammar*) fGrammarResolver->getGrammar(&theDTDDescription);
    }

    if (!fDTDGrammar) {

        fDTDGrammar = new (fGrammarPoolMemoryManager) DTDGrammar(fGrammarPoolMemoryManager);
        fGrammarResolver->putGrammar(fDTDGrammar);
    }
    else
        fDTDGrammar->reset();

    fGrammar = fDTDGrammar;
    fGrammarType = fGrammar->getGrammarType();
    fRootGrammar = 0;

    if (fValidatorFromUser) {
        if (fValidator->handlesDTD())
            fValidator->setGrammar(fGrammar);
        else if (fValidator->handlesSchema()) {

            ((SchemaValidator*) fValidator)->setErrorReporter(fErrorReporter);
            ((SchemaValidator*) fValidator)->setGrammarResolver(fGrammarResolver);
            ((SchemaValidator*) fValidator)->setExitOnFirstFatal(fExitOnFirstFatal);
        }
    }
    else {
        // set fValidator as fDTDValidator
        fValidator = fDTDValidator;
        fValidator->setGrammar(fGrammar);
    }

    // Reset validation
    fValidate = (fValScheme == Val_Always) ? true : false;

    // Ignore skipDTDValidation flag if no schema processing is taking place */
    fSkipDTDValidation = fSkipDTDValidation && fDoSchema;

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

    // Reset IdentityConstraints
    if (fICHandler)
        fICHandler->reset();

    //  Reset the element stack, and give it the latest ids for the special
    //  URIs it has to know about.
    fElemStack.reset
    (
        fEmptyNamespaceId
        , fUnknownNamespaceId
        , fXMLNamespaceId
        , fXMLNSNamespaceId
    );

    if (!fSchemaNamespaceId)
        fSchemaNamespaceId  = fURIStringPool->addOrFind(SchemaSymbols::fgURI_XSI);

    // Reset some status flags
    fInException = false;
    fStandalone = false;
    fErrorCount = 0;
    fHasNoDTD = true;
    fSeeXsi = false;

    // Reset PSVI context
    // note that we always need this around for DOMTypeInfo
    if (!fPSVIElement)
        fPSVIElement = new (fMemoryManager) PSVIElement(fMemoryManager);

    if (!fErrorStack)
    {
        fErrorStack = new (fMemoryManager) ValueStackOf<bool>(8, fMemoryManager);
    }
    else
    {
        fErrorStack->removeAllElements();
    }

    resetPSVIElemContext();

    // Reset the validators
    fDTDValidator->reset();
    fDTDValidator->setErrorReporter(fErrorReporter);
    fSchemaValidator->reset();
    fSchemaValidator->setErrorReporter(fErrorReporter);
    fSchemaValidator->setExitOnFirstFatal(fExitOnFirstFatal);
    fSchemaValidator->setGrammarResolver(fGrammarResolver);
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
    fElemCount = 0;
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
    fDTDElemNonDeclPool->removeAll();
}


//  This method is called between markup in content. It scans for character
//  data that is sent to the document handler. It watches for any markup
//  characters that would indicate that the character data has ended. It also
//  handles expansion of general and character entities.
//
//  sendData() is a local static helper for this method which handles some
//  code that must be done in three different places here.
void IGXMLScanner::sendCharData(XMLBuffer& toSend)
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
        const XMLCh* rawBuf = toSend.getRawBuffer();
        XMLSize_t len = toSend.getLen();

        // And see if the current element is a 'Children' style content model
        const ElemStack::StackElem* topElem = fElemStack.topElement();

        // Get the character data opts for the current element
        XMLElementDecl::CharDataOpts charOpts = XMLElementDecl::AllCharData;
        if(fGrammar->getGrammarType() == Grammar::SchemaGrammarType)
        {
            // And see if the current element is a 'Children' style content model
            ComplexTypeInfo *currType = ((SchemaValidator*)fValidator)->getCurrentTypeInfo();
            if(currType)
            {
                SchemaElementDecl::ModelTypes modelType = (SchemaElementDecl::ModelTypes) currType->getContentType();
                if(modelType == SchemaElementDecl::Children ||
                   modelType == SchemaElementDecl::ElementOnlyEmpty)
                    charOpts = XMLElementDecl::SpacesOk;
                else if(modelType == SchemaElementDecl::Empty)
                    charOpts = XMLElementDecl::NoCharData;
            }
        } else // DTD grammar
            charOpts = topElem->fThisElement->getCharDataOpts();

        if (charOpts == XMLElementDecl::NoCharData)
        {
            // They definitely cannot handle any type of char data
            fValidator->emitError(XMLValid::NoCharDataInCM);
            //if(fGrammarType == Grammar::SchemaGrammarType)
            //{
              //  if (getPSVIHandler())
              //  {
                    // REVISIT:
                    // PSVIElement->setValidity(PSVIItem::VALIDITY_INVALID);
              //  }
           // }
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
                if (fGrammarType != Grammar::SchemaGrammarType)
                {
                    if (fDocHandler)
                        fDocHandler->docCharacters(rawBuf, len, false);
                }
                else
                {
                    XMLSize_t xsLen;
                    const XMLCh* xsNormalized;
                    SchemaValidator *schemaValidator = (SchemaValidator *)fValidator;
                    DatatypeValidator* tempDV = ((SchemaValidator*) fValidator)->getCurrentDatatypeValidator();
                    if (tempDV && tempDV->getWSFacet() != DatatypeValidator::PRESERVE)
                    {
                        // normalize the character according to schema whitespace facet
                        ((SchemaValidator*) fValidator)->normalizeWhiteSpace(tempDV, rawBuf, fWSNormalizeBuf);
                        xsNormalized = fWSNormalizeBuf.getRawBuffer();
                        xsLen = fWSNormalizeBuf.getLen();
                    }
                    else {
                        xsNormalized = rawBuf;
                        xsLen = len ;
                    }

                    // tell the schema validation about the character data for checkContent later
                    schemaValidator->setDatatypeBuffer(xsNormalized);

                    // call all active identity constraints
                    if (toCheckIdentityConstraint() && fICHandler->getMatcherCount()) {
                        fContent.append(xsNormalized, xsLen);
                    }

                    if (fDocHandler) {
                        if (fNormalizeData) {
                           fDocHandler->docCharacters(xsNormalized, xsLen, false);
                        }
                        else {
                            fDocHandler->docCharacters(rawBuf, len, false);
                        }
                    }
                }
            }
        }
        else
        {
            //  If they can take any char data, then send it. Otherwise, they
            //  can only handle whitespace and can't handle this stuff so
            //  issue an error.
            if (charOpts == XMLElementDecl::AllCharData)
            {
                if (fGrammarType != Grammar::SchemaGrammarType)
                {
                    if (fDocHandler)
                        fDocHandler->docCharacters(rawBuf, len, false);
                }
                else
                {
                    XMLSize_t xsLen;
                    const XMLCh* xsNormalized;
                    SchemaValidator *schemaValidator = (SchemaValidator*)fValidator;
                    DatatypeValidator* tempDV = ((SchemaValidator*) fValidator)->getCurrentDatatypeValidator();
                    if (tempDV && tempDV->getWSFacet() != DatatypeValidator::PRESERVE)
                    {
                        // normalize the character according to schema whitespace facet
                        ((SchemaValidator*) fValidator)->normalizeWhiteSpace(tempDV, rawBuf, fWSNormalizeBuf);
                        xsNormalized = fWSNormalizeBuf.getRawBuffer();
                        xsLen = fWSNormalizeBuf.getLen();
                    }
                    else {
                        xsNormalized = rawBuf;
                        xsLen = len;
                    }

                    // tell the schema validation about the character data for checkContent later
                    schemaValidator->setDatatypeBuffer(xsNormalized);

                    // call all active identity constraints
                    if (toCheckIdentityConstraint() && fICHandler->getMatcherCount()) {
                        fContent.append(xsNormalized, xsLen);
                    }

                    if (fDocHandler) {
                        if (fNormalizeData) {
                            fDocHandler->docCharacters(xsNormalized, xsLen, false);
                        }
                        else {
                            fDocHandler->docCharacters(rawBuf, len, false);
                        }
                    }
                }
            }
            else
            {
                fValidator->emitError(XMLValid::NoCharDataInCM);
                if(fGrammarType == Grammar::SchemaGrammarType)
                {
                    if (getPSVIHandler())
                    {
                        // REVISIT:
                        // PSVIAttribute->setValidity(PSVIItem::VALIDITY_INVALID);
                    }
                }
            }
        }
    }
    else
    {
        // call all active identity constraints
        if (fGrammarType == Grammar::SchemaGrammarType) {

            if (toCheckIdentityConstraint() && fICHandler->getMatcherCount())
                fContent.append(toSend.getRawBuffer(), toSend.getLen());
        }

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
void IGXMLScanner::updateNSMap(const  XMLCh* const    attrName
                            , const XMLCh* const    attrValue)
{
    updateNSMap(attrName, attrValue, XMLString::indexOf(attrName, chColon));
}

void IGXMLScanner::updateNSMap(const  XMLCh* const    attrName
                            , const XMLCh* const    attrValue
                            , const int colonOfs)
{
    // We need a buffer to normalize the attribute value into
    XMLBufBid bbNormal(&fBufMgr);
    XMLBuffer& normalBuf = bbNormal.getBuffer();

    //  Normalize the value into the passed buffer. In this case, we don't
    //  care about the return value. An error was issued for the error, which
    //  is all we care about here.
    normalizeAttRawValue(attrName, attrValue, normalBuf);
    XMLCh* namespaceURI = normalBuf.getRawBuffer();

    //  We either have the default prefix (""), or we point it into the attr
    //  name parameter. Note that the xmlns is not the prefix we care about
    //  here. To us, the 'prefix' is really the local part of the attrName
    //  parameter.
    //
    //  Check 1. xxx is not xmlns
    //        2. if xxx is xml, then yyy must match XMLUni::fgXMLURIName, and vice versa
    //        3. yyy is not XMLUni::fgXMLNSURIName
    //        4. if xxx is not null, then yyy cannot be an empty string.
    const XMLCh* prefPtr = XMLUni::fgZeroLenString;
    if (colonOfs != -1) {
        prefPtr = &attrName[colonOfs + 1];

        if (XMLString::equals(prefPtr, XMLUni::fgXMLNSString))
            emitError(XMLErrs::NoUseOfxmlnsAsPrefix);
        else if (XMLString::equals(prefPtr, XMLUni::fgXMLString)) {
            if (!XMLString::equals(namespaceURI, XMLUni::fgXMLURIName))
                emitError(XMLErrs::PrefixXMLNotMatchXMLURI);
        }

        if (!namespaceURI)
            emitError(XMLErrs::NoEmptyStrNamespace, attrName);
        else if(!*namespaceURI && fXMLVersion == XMLReader::XMLV1_0)
            emitError(XMLErrs::NoEmptyStrNamespace, attrName);
    }

    if (XMLString::equals(namespaceURI, XMLUni::fgXMLNSURIName))
        emitError(XMLErrs::NoUseOfxmlnsURI);
    else if (XMLString::equals(namespaceURI, XMLUni::fgXMLURIName)) {
        if (!XMLString::equals(prefPtr, XMLUni::fgXMLString))
            emitError(XMLErrs::XMLURINotMatchXMLPrefix);
    }

    //  Ok, we have to get the unique id for the attribute value, which is the
    //  URI that this value should be mapped to. The validator has the
    //  namespace string pool, so we ask him to find or add this new one. Then
    //  we ask the element stack to add this prefix to URI Id mapping.
    fElemStack.addPrefix
    (
        prefPtr
        , fURIStringPool->addOrFind(namespaceURI)
    );
}

void IGXMLScanner::scanRawAttrListforNameSpaces(XMLSize_t attCount)
{
    //  Make an initial pass through the list and find any xmlns attributes or
    //  schema attributes.
    //  When we find one, send it off to be used to update the element stack's
    //  namespace mappings.
    for (XMLSize_t index = 0; index < attCount; index++)
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
    if (fDoSchema && fSeeXsi)
    {
        //  Schema Xsi Type yyyy (e.g. xsi:type="yyyyy")
        XMLBufBid bbXsi(&fBufMgr);
        XMLBuffer& fXsiType = bbXsi.getBuffer();

        for (XMLSize_t index = 0; index < attCount; index++)
        {
            // each attribute has the prefix:suffix="value"
            const KVStringPair* curPair = fRawAttrList->elementAt(index);
            const XMLCh* rawPtr = curPair->getKey();
            const XMLCh* prefPtr = XMLUni::fgZeroLenString;
            int   colonInd = fRawAttrColonList[index];

            if (colonInd != -1) {

                fURIBuf.set(rawPtr, colonInd);
                prefPtr = fURIBuf.getRawBuffer();
            }

            // if schema URI has been seen, scan for the schema location and uri
            // and resolve the schema grammar; or scan for schema type
            if (resolvePrefix(prefPtr, ElemStack::Mode_Attribute) == fSchemaNamespaceId) {

                const XMLCh* valuePtr = curPair->getValue();
                const XMLCh* suffPtr = &rawPtr[colonInd + 1];

                if (XMLString::equals(suffPtr, SchemaSymbols::fgXSI_SCHEMALOCATION))
                    parseSchemaLocation(valuePtr);
                else if (XMLString::equals(suffPtr, SchemaSymbols::fgXSI_NONAMESPACESCHEMALOCATION))
                    resolveSchemaGrammar(valuePtr, XMLUni::fgZeroLenString);

                if ((!fValidator || !fValidator->handlesSchema()) &&
                    (XMLString::equals(suffPtr, SchemaSymbols::fgXSI_TYPE) ||
                     XMLString::equals(suffPtr, SchemaSymbols::fgATT_NILL)))
                {
                  // If we are in the DTD mode, try to switch to the Schema
                  // mode. For that we need to find any XML Schema grammar
                  // that we can switch to. Such a grammar can only come
                  // from the cache (if it came from the schemaLocation
                  // attribute, we would be in the Schema mode already).
                  //
                  XMLGrammarPool* pool = fGrammarResolver->getGrammarPool ();
                  RefHashTableOfEnumerator<Grammar> i = pool->getGrammarEnumerator ();

                  while (i.hasMoreElements ())
                  {
                    Grammar& gr (i.nextElement ());

                    if (gr.getGrammarType () == Grammar::SchemaGrammarType)
                    {
                      switchGrammar (gr.getTargetNamespace ());
                      break;
                    }
                  }
                }

                if( fValidator && fValidator->handlesSchema() )
                {
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
        }

        if (fValidator && fValidator->handlesSchema()) {
            if (!fXsiType.isEmpty()) {
                int colonPos = -1;
                unsigned int uriId = resolveQName (
                      fXsiType.getRawBuffer()
                    , fPrefixBuf
                    , ElemStack::Mode_Element
                    , colonPos
                );
                ((SchemaValidator*)fValidator)->setXsiType(fPrefixBuf.getRawBuffer(), fXsiType.getRawBuffer() + colonPos + 1, uriId);
            }
        }
    }
}

void IGXMLScanner::parseSchemaLocation(const XMLCh* const schemaLocationStr, bool ignoreLoadSchema)
{
    XMLCh* locStr = XMLString::replicate(schemaLocationStr, fMemoryManager);
    ArrayJanitor<XMLCh> janLoc(locStr, fMemoryManager);

    processSchemaLocation(locStr);
    XMLSize_t size = fLocationPairs->size();

    if (size % 2 != 0 ) {
        emitError(XMLErrs::BadSchemaLocation);
    } else {
        // We need a buffer to normalize the attribute value into
        XMLBuffer normalBuf(1023, fMemoryManager);
        for(XMLSize_t i=0; i<size; i=i+2) {
            normalizeAttRawValue(SchemaSymbols::fgXSI_SCHEMALOCATION, fLocationPairs->elementAt(i), normalBuf);
            resolveSchemaGrammar(fLocationPairs->elementAt(i+1), normalBuf.getRawBuffer(), ignoreLoadSchema);
        }
    }
}

void IGXMLScanner::resolveSchemaGrammar(const XMLCh* const loc, const XMLCh* const uri, bool ignoreLoadSchema) {

    Grammar* grammar = 0;

    {
        XMLSchemaDescriptionImpl    theSchemaDescription(uri, fMemoryManager);
        theSchemaDescription.setLocationHints(loc);
        grammar = fGrammarResolver->getGrammar(&theSchemaDescription);
    }

    // If multi-import is enabled, make sure the existing grammar came
    // from the import directive. Otherwise we may end up reloading
    // the same schema that came from the external grammar pool. Ideally,
    // we would move fSchemaInfoList to XMLGrammarPool so that it survives
    // the destruction of the scanner in which case we could rely on the
    // same logic we use to weed out duplicate schemas below.
    //
    if (!grammar ||
        grammar->getGrammarType() == Grammar::DTDGrammarType ||
        (getHandleMultipleImports() &&
         ((XMLSchemaDescription*)grammar->getGrammarDescription())->
         getContextType () == XMLSchemaDescription::CONTEXT_IMPORT))
    {
      if (fLoadSchema || ignoreLoadSchema)
      {
        XSDDOMParser parser(0, fMemoryManager, 0);

        parser.setValidationScheme(XercesDOMParser::Val_Never);
        parser.setDoNamespaces(true);
        parser.setUserEntityHandler(fEntityHandler);
        parser.setUserErrorReporter(fErrorReporter);

        //Normalize loc
        XMLBufBid nnSys(&fBufMgr);
        XMLBuffer& normalizedSysId = nnSys.getBuffer();
        XMLString::removeChar(loc, 0xFFFF, normalizedSysId);
        const XMLCh* normalizedURI = normalizedSysId.getRawBuffer();

        // Create a buffer for expanding the system id
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
            XMLResourceIdentifier resourceIdentifier(XMLResourceIdentifier::SchemaGrammar,
                            expSysId.getRawBuffer(), uri, XMLUni::fgZeroLenString, lastInfo.systemId,
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
                return;

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

        // Put a janitor on the input source
        Janitor<InputSource> janSrc(srcToFill);

        // Check if this exact schema has already been seen.
        //
        const XMLCh* sysId = srcToFill->getSystemId();
        unsigned int uriId = (uri && *uri) ? fURIStringPool->addOrFind(uri) : fEmptyNamespaceId;
        SchemaInfo* importSchemaInfo = 0;

        if (fUseCachedGrammar)
          importSchemaInfo = fCachedSchemaInfoList->get(sysId, uriId);

        if (!importSchemaInfo && !fToCacheGrammar)
          importSchemaInfo = fSchemaInfoList->get(sysId, uriId);

        if (importSchemaInfo)
        {
          // We haven't added any new grammars so it is safe to just
          // return.
          //
          return;
        }

        // Should just issue warning if the schema is not found
        bool flag = srcToFill->getIssueFatalErrorIfNotFound();
        srcToFill->setIssueFatalErrorIfNotFound(false);

        parser.parse(*srcToFill);

        // Reset the InputSource
        srcToFill->setIssueFatalErrorIfNotFound(flag);

        if (parser.getSawFatal() && fExitOnFirstFatal)
            emitError(XMLErrs::SchemaScanFatalError);

        DOMDocument* document = parser.getDocument(); //Our Grammar

        if (document != 0) {

            DOMElement* root = document->getDocumentElement();// This is what we pass to TraverserSchema
            if (root != 0)
            {
                const XMLCh* newUri = root->getAttribute(SchemaSymbols::fgATT_TARGETNAMESPACE);
                bool newGrammar = false;
                if (!XMLString::equals(newUri, uri)) {
                    if (fValidate || fValScheme == Val_Auto) {
                        fValidator->emitError(XMLValid::WrongTargetNamespace, loc, uri);
                    }

                    grammar = fGrammarResolver->getGrammar(newUri);
                    newGrammar = true;
                }

                if (!grammar ||
                    grammar->getGrammarType() == Grammar::DTDGrammarType ||
                    (getHandleMultipleImports() &&
                     ((XMLSchemaDescription*)grammar->getGrammarDescription())->
                     getContextType () == XMLSchemaDescription::CONTEXT_IMPORT))
                {
                    // If we switched namespace URI, recheck the schema info.
                    //
                    if (newGrammar)
                    {
                      unsigned int newUriId = (newUri && *newUri) ? fURIStringPool->addOrFind(newUri) : fEmptyNamespaceId;

                      if (fUseCachedGrammar)
                        importSchemaInfo = fCachedSchemaInfoList->get(sysId, newUriId);

                      if (!importSchemaInfo && !fToCacheGrammar)
                        importSchemaInfo = fSchemaInfoList->get(sysId, newUriId);

                      if (importSchemaInfo)
                        return;
                    }

                    //  Since we have seen a grammar, set our validation flag
                    //  at this point if the validation scheme is auto
                    if (fValScheme == Val_Auto && !fValidate) {
                        fValidate = true;
                        fElemStack.setValidationFlag(fValidate);
                    }

                    // we have seen a schema, so set up the fValidator as fSchemaValidator
                    if (!fValidator->handlesSchema())
                    {
                        if (fValidatorFromUser) {
                            // the fValidator is from user
                            ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Gen_NoSchemaValidator, fMemoryManager);
                        }
                        else {
                            fValidator = fSchemaValidator;
                        }
                    }

                    bool grammarFound = grammar &&
                      grammar->getGrammarType() == Grammar::SchemaGrammarType;

                    SchemaGrammar* schemaGrammar;

                    if (grammarFound) {
                      schemaGrammar = (SchemaGrammar*) grammar;
                    }
                    else {
                      schemaGrammar = new (fGrammarPoolMemoryManager) SchemaGrammar(fGrammarPoolMemoryManager);
                    }

                    XMLSchemaDescription* gramDesc = (XMLSchemaDescription*) schemaGrammar->getGrammarDescription();
                    gramDesc->setContextType(XMLSchemaDescription::CONTEXT_PREPARSE);
                    gramDesc->setLocationHints(sysId);

                    TraverseSchema traverseSchema
                    (
                        root
                        , fURIStringPool
                        , schemaGrammar
                        , fGrammarResolver
                        , fUseCachedGrammar ? fCachedSchemaInfoList : fSchemaInfoList
                        , fToCacheGrammar ? fCachedSchemaInfoList : fSchemaInfoList
                        , this
                        , sysId
                        , fEntityHandler
                        , fErrorReporter
                        , fMemoryManager
                        , grammarFound
                    );

                    // Reset the now invalid schema roots in the collected
                    // schema info entries.
                    //
                    {
                      RefHash2KeysTableOfEnumerator<SchemaInfo> i (
                        fToCacheGrammar ? fCachedSchemaInfoList : fSchemaInfoList);

                      while (i.hasMoreElements ())
                        i.nextElement().resetRoot ();
                    }

                    if (fGrammarType == Grammar::DTDGrammarType) {
                        fGrammar = schemaGrammar;
                        fGrammarType = Grammar::SchemaGrammarType;
                        fValidator->setGrammar(fGrammar);
                    }

                    if (fValidate) {
                        //  validate the Schema scan so far
                        fValidator->preContentValidation(false);
                    }
                }
            }
        }
      }
    }
    else
    {
        //  Since we have seen a grammar, set our validation flag
        //  at this point if the validation scheme is auto
        if (fValScheme == Val_Auto && !fValidate) {
            fValidate = true;
            fElemStack.setValidationFlag(fValidate);
        }

        // we have seen a schema, so set up the fValidator as fSchemaValidator
        if (!fValidator->handlesSchema())
        {
            if (fValidatorFromUser) {
                // the fValidator is from user
                ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Gen_NoSchemaValidator, fMemoryManager);
            }
            else {
                fValidator = fSchemaValidator;
            }
        }

        if (fGrammarType == Grammar::DTDGrammarType) {
            fGrammar = grammar;
            fGrammarType = Grammar::SchemaGrammarType;
            fValidator->setGrammar(fGrammar);
        }
    }

    // fModel may need updating:
    if(getPSVIHandler())
        fModel = fGrammarResolver->getXSModel();
}

InputSource* IGXMLScanner::resolveSystemId(const XMLCh* const sysId
                                          ,const XMLCh* const pubId)
{
    //Normalize sysId
    XMLBufBid nnSys(&fBufMgr);
    XMLBuffer& normalizedSysId = nnSys.getBuffer();
    XMLString::removeChar(sysId, 0xFFFF, normalizedSysId);
    const XMLCh* normalizedURI = normalizedSysId.getRawBuffer();

    // Create a buffer for expanding the system id
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
//  IGXMLScanner: Private grammar preparsing methods
// ---------------------------------------------------------------------------
Grammar* IGXMLScanner::loadXMLSchemaGrammar(const InputSource& src,
                                            const bool toCache)
{
   // Reset the validators
    fSchemaValidator->reset();
    fSchemaValidator->setErrorReporter(fErrorReporter);
    fSchemaValidator->setExitOnFirstFatal(fExitOnFirstFatal);
    fSchemaValidator->setGrammarResolver(fGrammarResolver);

    if (fValidatorFromUser)
        fValidator->reset();

    if (!fValidator->handlesSchema()) {
        if (fValidatorFromUser && fValidate)
            ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Gen_NoSchemaValidator, fMemoryManager);
        else {
            fValidator = fSchemaValidator;
        }
    }

    XSDDOMParser parser(0, fMemoryManager, 0);

    parser.setValidationScheme(XercesDOMParser::Val_Never);
    parser.setDoNamespaces(true);
    parser.setUserEntityHandler(fEntityHandler);
    parser.setUserErrorReporter(fErrorReporter);

    // Should just issue warning if the schema is not found
    bool flag = src.getIssueFatalErrorIfNotFound();
    ((InputSource&) src).setIssueFatalErrorIfNotFound(false);

    parser.parse(src);

    // Reset the InputSource
    ((InputSource&) src).setIssueFatalErrorIfNotFound(flag);

    if (parser.getSawFatal() && fExitOnFirstFatal)
        emitError(XMLErrs::SchemaScanFatalError);

    DOMDocument* document = parser.getDocument(); //Our Grammar

    if (document != 0) {

        DOMElement* root = document->getDocumentElement();// This is what we pass to TraverserSchema
        if (root != 0)
        {
            const XMLCh* nsUri = root->getAttribute(SchemaSymbols::fgATT_TARGETNAMESPACE);
            Grammar* grammar = fGrammarResolver->getGrammar(nsUri);

            // Check if this exact schema has already been seen.
            //
            const XMLCh* sysId = src.getSystemId();
            SchemaInfo* importSchemaInfo = 0;

            if (grammar)
            {
              if (nsUri && *nsUri)
                importSchemaInfo = fCachedSchemaInfoList->get(sysId, fURIStringPool->addOrFind(nsUri));
              else
                importSchemaInfo = fCachedSchemaInfoList->get(sysId, fEmptyNamespaceId);
            }

            if (!importSchemaInfo)
            {
              bool grammarFound = grammar &&
                grammar->getGrammarType() == Grammar::SchemaGrammarType &&
                getHandleMultipleImports();

              SchemaGrammar* schemaGrammar;

              if (grammarFound)
                schemaGrammar = (SchemaGrammar*) grammar;
              else
                schemaGrammar = new (fGrammarPoolMemoryManager) SchemaGrammar(fGrammarPoolMemoryManager);

              XMLSchemaDescription* gramDesc = (XMLSchemaDescription*) schemaGrammar->getGrammarDescription();
              gramDesc->setContextType(XMLSchemaDescription::CONTEXT_PREPARSE);
              gramDesc->setLocationHints(sysId);

              TraverseSchema traverseSchema
                (
                  root
                  , fURIStringPool
                  , schemaGrammar
                  , fGrammarResolver
                  , fCachedSchemaInfoList
                  , toCache ? fCachedSchemaInfoList : fSchemaInfoList
                  , this
                  , sysId
                  , fEntityHandler
                  , fErrorReporter
                  , fMemoryManager
                  , grammarFound
                );

              grammar = schemaGrammar;

              // Reset the now invalid schema roots in the collected
              // schema info entries.
              //
              {
                RefHash2KeysTableOfEnumerator<SchemaInfo> i (
                  toCache ? fCachedSchemaInfoList : fSchemaInfoList);

                while (i.hasMoreElements ())
                  i.nextElement().resetRoot ();
              }
            }

            if (fValidate) {
              //  validate the Schema scan so far
              fValidator->setGrammar(grammar);
              fValidator->preContentValidation(false);
            }

            if (toCache) {
              fGrammarResolver->cacheGrammars();
            }

            if(getPSVIHandler())
              fModel = fGrammarResolver->getXSModel();

            return grammar;
        }
    }

    return 0;
}



// ---------------------------------------------------------------------------
//  IGXMLScanner: Private parsing methods
// ---------------------------------------------------------------------------

//  This method is called to do a raw scan of an attribute value. It does not
//  do normalization (since we don't know their types yet.) It just scans the
//  value and does entity expansion.
//
//  End of entity's must be dealt with here. During DTD scan, they can come
//  from external entities. During content, they can come from any entity.
//  We just eat the end of entity and continue with our scan until we come
//  to the closing quote. If an unterminated value causes us to go through
//  subsequent entities, that will cause errors back in the calling code,
//  but there's little we can do about it here.
bool IGXMLScanner::basicAttrValueScan(const XMLCh* const attrName, XMLBuffer& toFill)
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
    while (true)
    {
        try
        {
            while(true)
            {
                XMLCh nextCh = fReaderMgr.getNextChar();

                if (nextCh != quoteCh)
                {
                    if (nextCh != chAmpersand)
                    {
                        if ((nextCh < 0xD800) || (nextCh > 0xDFFF))
                        {
                            // Its got to at least be a valid XML character
                            if (!fReaderMgr.getCurrentReader()->isXMLChar(nextCh))
                            {
                                if (nextCh == 0)
                                    ThrowXMLwithMemMgr(UnexpectedEOFException, XMLExcepts::Gen_UnexpectedEOF, fMemoryManager);

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
                        } else // its a surrogate
                        {
                            // Deal with surrogate pairs

                            //  we expect a a leading surrogate.
                            if (nextCh <= 0xDBFF)
                            {
                                toFill.append(nextCh);

                                //  process the trailing surrogate
                                nextCh = fReaderMgr.getNextChar();

                                //  it should be a trailing surrogate.
                                if ((nextCh < 0xDC00) || (nextCh > 0xDFFF))
                                {
                                    emitError(XMLErrs::Expected2ndSurrogateChar);
                                }
                            } else
                            {
                                //  Its a trailing surrogate, but we are not expecting it
                                emitError(XMLErrs::Unexpected2ndSurrogateChar);
                            }
                        }
                    } else // its a chAmpersand
                    {
                        //  Check for an entity ref . We ignore the empty flag in
                        //  this one.

                        bool    escaped;
                        XMLCh   firstCh;
                        XMLCh   secondCh
                            ;
                        // If it was not returned directly, then jump back up
                        if (scanEntityRef(true, firstCh, secondCh, escaped) == EntityExp_Returned)
                        {
                            //  If it was escaped, then put in a 0xFFFF value. This will
                            //  be used later during validation and normalization of the
                            //  value to know that the following character was via an
                            //  escape char.
                            if (escaped)
                                toFill.append(0xFFFF);

                            toFill.append(firstCh);
                            if (secondCh)
                                toFill.append(secondCh);
                        }
                        continue;
                    }
                } else // its a quoteCh
                {
                    //  Check for our ending quote. It has to be in the same entity
                    //  as where we started. Quotes in nested entities are ignored.

                    if (curReader == fReaderMgr.getCurrentReaderNum())
                    {
                        return true;
                    }

                    // Watch for spillover into a previous entity
                    if (curReader > fReaderMgr.getCurrentReaderNum())
                    {
                        emitError(XMLErrs::PartialMarkupInEntity);
                        return false;
                    }
                }

                // add it to the buffer
                toFill.append(nextCh);

            }
        }
        catch(const EndOfEntityException&)
        {
            // Just eat it and continue.
        }
    }
    return true;
}


bool IGXMLScanner::scanAttValue(  const   XMLAttDef* const    attDef
                                  , const XMLCh* const        attrName
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
void IGXMLScanner::scanCDSection()
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
    const ElemStack::StackElem* topElem = fElemStack.topElement();

    // Get the character data opts for the current element
    XMLElementDecl::CharDataOpts charOpts = XMLElementDecl::AllCharData;
    if(fGrammar->getGrammarType() == Grammar::SchemaGrammarType)
    {
        // And see if the current element is a 'Children' style content model
        ComplexTypeInfo *currType = ((SchemaValidator*)fValidator)->getCurrentTypeInfo();
        if(currType)
        {
            SchemaElementDecl::ModelTypes modelType = (SchemaElementDecl::ModelTypes) currType->getContentType();
            if(modelType == SchemaElementDecl::Children ||
               modelType == SchemaElementDecl::ElementOnlyEmpty)
                charOpts = XMLElementDecl::SpacesOk;
            else if(modelType == SchemaElementDecl::Empty)
                charOpts = XMLElementDecl::NoCharData;
        }
    } else // DTD grammar
        charOpts = topElem->fThisElement->getCharDataOpts();

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
                    if(fGrammarType == Grammar::SchemaGrammarType)
                    {
                        if (getPSVIHandler())
                        {
                            // REVISIT:
                            // PSVIElement->setValidity(PSVIItem::VALIDITY_INVALID);
                        }
                    }
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

            if (fGrammarType == Grammar::SchemaGrammarType) {

                XMLSize_t xsLen = bbCData.getLen();
                const XMLCh* xsNormalized = bbCData.getRawBuffer();
                DatatypeValidator* tempDV = ((SchemaValidator*) fValidator)->getCurrentDatatypeValidator();
                if (tempDV && tempDV->getWSFacet() != DatatypeValidator::PRESERVE)
                {
                    // normalize the character according to schema whitespace facet
                    ((SchemaValidator*) fValidator)->normalizeWhiteSpace(tempDV, xsNormalized, fWSNormalizeBuf);
                    xsNormalized = fWSNormalizeBuf.getRawBuffer();
                    xsLen = fWSNormalizeBuf.getLen();
                    if (fNormalizeData && fValidate) {
                        bbCData.set(xsNormalized);
                    }
                }

                if (fValidate) {

                    // tell the schema validation about the character data for checkContent later
                    ((SchemaValidator*)fValidator)->setDatatypeBuffer(xsNormalized);

                    if (charOpts != XMLElementDecl::AllCharData)
                    {
                        // They definitely cannot handle any type of char data
                        fValidator->emitError(XMLValid::NoCharDataInCM);
                        if (getPSVIHandler())
                        {
                            // REVISIT:
                            // PSVIElement->setValidity(PSVIItem::VALIDITY_INVALID);
                        }
                    }
                }

                // call all active identity constraints
                if (toCheckIdentityConstraint() && fICHandler->getMatcherCount()) {
                    fContent.append(xsNormalized, xsLen);
                }
            }
            else {
                if (fValidate) {

                    if (charOpts != XMLElementDecl::AllCharData)
                    {
                        // They definitely cannot handle any type of char data
                        fValidator->emitError(XMLValid::NoCharDataInCM);
                    }
                }
            }

            // If we have a doc handler, call it
            if (fDocHandler)
            {
                fDocHandler->docCharacters(
                    bbCData.getRawBuffer(), bbCData.getLen(), true
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


void IGXMLScanner::scanCharData(XMLBuffer& toUse)
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
                XMLElementDecl::CharDataOpts charOpts = XMLElementDecl::AllCharData;
                if(fGrammar->getGrammarType() == Grammar::SchemaGrammarType)
                {
                    // And see if the current element is a 'Children' style content model
                    ComplexTypeInfo *currType = ((SchemaValidator*)fValidator)->getCurrentTypeInfo();
                    if(currType)
                    {
                        SchemaElementDecl::ModelTypes modelType = (SchemaElementDecl::ModelTypes) currType->getContentType();
                        if(modelType == SchemaElementDecl::Children ||
                           modelType == SchemaElementDecl::ElementOnlyEmpty)
                            charOpts = XMLElementDecl::SpacesOk;
                        else if(modelType == SchemaElementDecl::Empty)
                            charOpts = XMLElementDecl::NoCharData;
                    }
                } else // DTD grammar
                    charOpts = topElem->fThisElement->getCharDataOpts();

                if (charOpts == XMLElementDecl::SpacesOk)  // => Element Content
                {
                    // Error - standalone should have a value of "no" as whitespace detected in an
                    // element type with element content whose element declaration was external
                    //
                    fValidator->emitError(XMLValid::NoWSForStandalone);
                    if(fGrammarType == Grammar::SchemaGrammarType)
                    {
                        if (getPSVIHandler())
                        {
                            // REVISIT:
                            // PSVIElement->setValidity(PSVIItem::VALIDITY_INVALID);
                        }
                    }
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
IGXMLScanner::EntityExpRes
IGXMLScanner::scanEntityRef(  const   bool    inAttVal
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
            // there seems nothing  better to be done than to reset the entity expansion counter
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


bool IGXMLScanner::switchGrammar(const XMLCh* const newGrammarNameSpace)
{
    Grammar* tempGrammar = fGrammarResolver->getGrammar(newGrammarNameSpace);

    if (!tempGrammar && !fSkipDTDValidation) {
        // This is a case where namespaces is on with a DTD grammar.
        tempGrammar = fDTDGrammar;
    }
    if (!tempGrammar) {
        return false;
    }
    else {

        Grammar::GrammarType tempGrammarType = tempGrammar->getGrammarType();
        if (tempGrammarType == Grammar::SchemaGrammarType && !fValidator->handlesSchema()) {
            if (fValidatorFromUser)
                ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Gen_NoSchemaValidator, fMemoryManager);
            else {
                fValidator = fSchemaValidator;
            }
        }
        else if (tempGrammarType == Grammar::DTDGrammarType) {
            if (fSkipDTDValidation) {
                return false;
            }

            if (!fValidator->handlesDTD()) {
                if (fValidatorFromUser)
                    ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Gen_NoDTDValidator, fMemoryManager);
                else {
                    fValidator = fDTDValidator;
                }
            }
        }

        fGrammarType = tempGrammarType;
        fGrammar = tempGrammar;
        fValidator->setGrammar(fGrammar);
        return true;
    }
}

// check if we should skip or lax the validation of the element
// if skip - no validation
// if lax - validate only if the element if found
bool IGXMLScanner::laxElementValidation(QName* element, ContentLeafNameTypeVector* cv,
                                        const XMLContentModel* const cm,
                                        const XMLSize_t parentElemDepth)
{
    bool skipThisOne = false;
    bool laxThisOne = false;
    unsigned int elementURI = element->getURI();
    unsigned int currState = fElemState[parentElemDepth];
    unsigned int currLoop = fElemLoopState[parentElemDepth];

    if (currState == XMLContentModel::gInvalidTrans) {
        return laxThisOne;
    }

    SubstitutionGroupComparator comparator(fGrammarResolver, fURIStringPool);

    if (cv) {
        XMLSize_t i = 0;
        XMLSize_t leafCount = cv->getLeafCount();
        unsigned int nextState = 0;

        for (; i < leafCount; i++) {

            QName* fElemMap = cv->getLeafNameAt(i);
            unsigned int uri = fElemMap->getURI();
            ContentSpecNode::NodeTypes type = cv->getLeafTypeAt(i);

            if (type == ContentSpecNode::Leaf) {
                if (((uri == elementURI)
                      && XMLString::equals(fElemMap->getLocalPart(), element->getLocalPart()))
                    || comparator.isEquivalentTo(element, fElemMap)) {

                    nextState = cm->getNextState(currState, i);

                    if (nextState != XMLContentModel::gInvalidTrans)
                        break;
                }
            } else if ((type & 0x0f) == ContentSpecNode::Any) {
                nextState = cm->getNextState(currState, i);
                if (nextState != XMLContentModel::gInvalidTrans)
                    break;
            }
            else if ((type & 0x0f) == ContentSpecNode::Any_Other) {
                if (uri != elementURI && elementURI != fEmptyNamespaceId) {
                    nextState = cm->getNextState(currState, i);
                    if (nextState != XMLContentModel::gInvalidTrans)
                        break;
                }
            }
            else if ((type & 0x0f) == ContentSpecNode::Any_NS) {
                if (uri == elementURI) {
                    nextState = cm->getNextState(currState, i);
                    if (nextState != XMLContentModel::gInvalidTrans)
                        break;
                }
            }

        } // for

        if (i == leafCount) { // no match
            fElemState[parentElemDepth] = XMLContentModel::gInvalidTrans;
            fElemLoopState[parentElemDepth] = 0;
            return laxThisOne;
        }

        unsigned int nextLoop = 0;
        if(!cm->handleRepetitions(element, currState, currLoop, nextState, nextLoop, i, &comparator)) {
            fElemState[parentElemDepth] = XMLContentModel::gInvalidTrans;
            fElemLoopState[parentElemDepth] = 0;
            return laxThisOne;
        }

        ContentSpecNode::NodeTypes type = cv->getLeafTypeAt(i);
        if ((type & 0x0f) == ContentSpecNode::Any ||
            (type & 0x0f) == ContentSpecNode::Any_Other ||
            (type & 0x0f) == ContentSpecNode::Any_NS)
        {
            if (type == ContentSpecNode::Any_Skip ||
                type == ContentSpecNode::Any_NS_Skip ||
                type == ContentSpecNode::Any_Other_Skip) {
                skipThisOne = true;
            }
            else if (type == ContentSpecNode::Any_Lax ||
                     type == ContentSpecNode::Any_NS_Lax ||
                     type == ContentSpecNode::Any_Other_Lax) {
                laxThisOne = true;
            }
        }
        fElemState[parentElemDepth] = nextState;
        fElemLoopState[parentElemDepth] = nextLoop;
    } // if

    if (skipThisOne) {
        fValidate = false;
        fElemStack.setValidationFlag(fValidate);
    }

    return laxThisOne;
}


// check if there is an AnyAttribute, and if so, see if we should lax or skip
// if skip - no validation
// if lax - validate only if the attribute if found
bool IGXMLScanner::anyAttributeValidation(SchemaAttDef* attWildCard, unsigned int uriId, bool& skipThisOne, bool& laxThisOne)
{
    XMLAttDef::AttTypes wildCardType = attWildCard->getType();
    bool anyEncountered = false;
    skipThisOne = false;
    laxThisOne = false;
    if (wildCardType == XMLAttDef::Any_Any)
        anyEncountered = true;
    else if (wildCardType == XMLAttDef::Any_Other) {
        if (attWildCard->getAttName()->getURI() != uriId
            && uriId != fEmptyNamespaceId)
            anyEncountered = true;
    }
    else if (wildCardType == XMLAttDef::Any_List) {
        ValueVectorOf<unsigned int>* nameURIList = attWildCard->getNamespaceList();
        XMLSize_t listSize = (nameURIList) ? nameURIList->size() : 0;

        if (listSize) {
            for (XMLSize_t i=0; i < listSize; i++) {
                if (nameURIList->elementAt(i) == uriId)
                    anyEncountered = true;
            }
        }
    }

    if (anyEncountered) {
        XMLAttDef::DefAttTypes   defType   = attWildCard->getDefaultType();
        if (defType == XMLAttDef::ProcessContents_Skip) {
            // attribute should just be bypassed,
            skipThisOne = true;
        }
        else if (defType == XMLAttDef::ProcessContents_Lax) {
            laxThisOne = true;
        }
    }

    return anyEncountered;
}

inline XMLAttDefList& getAttDefList(bool              isSchemaGrammar
                                  , ComplexTypeInfo*  currType
                                  , XMLElementDecl*   elemDecl)
{
    if (isSchemaGrammar && currType)
        return currType->getAttDefList();
    else
        return elemDecl->getAttDefList();
}

XERCES_CPP_NAMESPACE_END
