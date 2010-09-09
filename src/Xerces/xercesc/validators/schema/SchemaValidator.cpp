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
 * $Id: SchemaValidator.cpp 806488 2009-08-21 10:36:58Z amassari $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/Janitor.hpp>
#include <xercesc/framework/XMLDocumentHandler.hpp>
#include <xercesc/framework/XMLSchemaDescription.hpp>
#include <xercesc/framework/psvi/XSAnnotation.hpp>
#include <xercesc/internal/XMLReader.hpp>
#include <xercesc/internal/XMLScanner.hpp>
#include <xercesc/internal/ElemStack.hpp>
#include <xercesc/validators/datatype/DatatypeValidatorFactory.hpp>
#include <xercesc/validators/datatype/ListDatatypeValidator.hpp>
#include <xercesc/validators/datatype/UnionDatatypeValidator.hpp>
#include <xercesc/validators/datatype/ENTITYDatatypeValidator.hpp>
#include <xercesc/validators/datatype/IDDatatypeValidator.hpp>
#include <xercesc/validators/datatype/IDREFDatatypeValidator.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xercesc/validators/schema/SchemaValidator.hpp>
#include <xercesc/validators/schema/SubstitutionGroupComparator.hpp>
#include <xercesc/validators/schema/XercesGroupInfo.hpp>
#include <xercesc/validators/schema/XSDLocator.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  SchemaValidator: Constructors and Destructor
// ---------------------------------------------------------------------------
SchemaValidator::SchemaValidator( XMLErrorReporter* const errReporter
                                , MemoryManager* const    manager) :

    XMLValidator(errReporter)
    , fMemoryManager(manager)
    , fSchemaGrammar(0)
    , fGrammarResolver(0)
    , fXsiType(0)
    , fNil(false)
    , fNilFound(false)
    , fCurrentDatatypeValidator(0)
    , fNotationBuf(0)
    , fDatatypeBuffer(1023, manager)
    , fTrailing(false)
    , fSeenNonWhiteSpace(false)
    , fSeenId(false)
    , fTypeStack(0)
    , fMostRecentAttrValidator(0)
    , fErrorOccurred(false)
    , fElemIsSpecified(false)
{
    fTypeStack = new (fMemoryManager) ValueStackOf<ComplexTypeInfo*>(8, fMemoryManager);
}

SchemaValidator::~SchemaValidator()
{
    delete fXsiType;
    delete fTypeStack;

    if (fNotationBuf)
        delete fNotationBuf;
}

// ---------------------------------------------------------------------------
//  SchemaValidator: Implementation of the XMLValidator interface
// ---------------------------------------------------------------------------
bool SchemaValidator::checkContent (XMLElementDecl* const elemDecl
                                 , QName** const          children
                                 , XMLSize_t              childCount
                                 , XMLSize_t*             indexFailingChild)
{
    fErrorOccurred = false;
    fElemIsSpecified = false;

    //
    //  Look up the element id in our element decl pool. This will get us
    //  the element decl in our own way of looking at them.
    //
    if (!elemDecl)
        ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Val_InvalidElemId, fMemoryManager);

    //
    //  Get the content spec type of this element. This will tell us what
    //  to do to validate it.
    //
    // the top of the type stack always knows best...
    ComplexTypeInfo* currType = fTypeStack->pop();

    const SchemaElementDecl::ModelTypes modelType = (currType)
            ? (SchemaElementDecl::ModelTypes)(currType->getContentType())
            : ((SchemaElementDecl*)elemDecl)->getModelType();

    if (modelType == SchemaElementDecl::Empty  ||
        modelType == SchemaElementDecl::ElementOnlyEmpty)
    {
        //
        //  We can do this one here. It cannot have any children. If it does
        //  we return 0 as the index of the first bad child.
        //
        if (childCount) {
            fErrorOccurred = true;
            *indexFailingChild=0;
            return false;
        }
    }
    else if ((modelType == SchemaElementDecl::Mixed_Simple)
         ||  (modelType == SchemaElementDecl::Mixed_Complex)
         ||  (modelType == SchemaElementDecl::Children))
    {
        // if nillable, it's an error to have value
        // XML Schema REC: Validation Rule: Element Locally Valid (Element)
        // 3.2.1 The element information item must have no
        // character or element information item [children].
        //
        if (fNil) {
            if (childCount > 0 || !XMLString::equals(fDatatypeBuffer.getRawBuffer(), XMLUni::fgZeroLenString)) {
                emitError(XMLValid::NilAttrNotEmpty, elemDecl->getFullName());
                fErrorOccurred = true;
            }
        }
        else {
            // Get the element's content model or fault it in
            XMLContentModel* elemCM = (currType)
                    ? currType->getContentModel()
                    : ((SchemaElementDecl*)elemDecl)->getContentModel();

            // Ask it to validate and return its return
            unsigned int emptyNS = getScanner()->getEmptyNamespaceId();
            bool result = elemCM->validateContent(children, childCount, emptyNS, indexFailingChild, getScanner()->getMemoryManager());
            if (!result) {
                result = elemCM->validateContentSpecial(children
                                                      , childCount
                                                      , emptyNS
                                                      , fGrammarResolver
                                                      , fGrammarResolver->getStringPool()
                                                      , indexFailingChild
													  , getScanner()->getMemoryManager());
            }

            if(!result) {
                fErrorOccurred = true;
            }

            return result;
        }
    }
    else if (modelType == SchemaElementDecl::Simple || modelType == SchemaElementDecl::Any)
    {
        // Normally for SchemaElementDecl::Any, We pass no judgement on it and anything goes
        // but if there is a fXsiTypeValidator, we need to use it for validation
        if (modelType == SchemaElementDecl::Simple && childCount > 0) {
            emitError(XMLValid::SimpleTypeHasChild, elemDecl->getFullName());
            fErrorOccurred = true;
        }
        else
        {
            XMLCh* value = fDatatypeBuffer.getRawBuffer();
            XMLCh* elemDefaultValue = ((SchemaElementDecl*) elemDecl)->getDefaultValue();

            if (fNil)
            {
                if ((!XMLString::equals(value, XMLUni::fgZeroLenString))
                    || elemDefaultValue)
                {
                    emitError(XMLValid::NilAttrNotEmpty, elemDecl->getFullName());
                    fErrorOccurred = true;
                }
            }
			else if (fCurrentDatatypeValidator)
            {
                DatatypeValidator::ValidatorType eleDefDVType = fCurrentDatatypeValidator->getType();
                bool validateCanonical = false;
                if (eleDefDVType == DatatypeValidator::NOTATION)
                {
                    // if notation, need to bind URI to notation first
                    if (!fNotationBuf)
                        fNotationBuf = new (fMemoryManager) XMLBuffer(1023, fMemoryManager);

                    //  Make sure that this value maps to one of the
                    //  notation values in the enumList parameter. We don't have to
                    //  look it up in the notation pool (if a notation) because we
                    //  will look up the enumerated values themselves. If they are in
                    //  the notation pool (after the Grammar is parsed), then obviously
                    //  this value will be legal since it matches one of them.
                    int colonPos = -1;
                    unsigned int uriId = getScanner()->resolveQName(value, *fNotationBuf, ElemStack::Mode_Element, colonPos);

                    const XMLCh* uriText = getScanner()->getURIText(uriId);
                    if (uriText && *uriText) {
                        fNotationBuf->set(uriText);
                        fNotationBuf->append(chColon);
                        fNotationBuf->append(&value[colonPos + 1]);
                        value = fNotationBuf->getRawBuffer();
                    }
                }

                if (elemDefaultValue)
                {
                    if (XMLString::equals(value, XMLUni::fgZeroLenString))
                    {
                        fElemIsSpecified = true;
                        // if this element didn't specified any value
                        // use default value
                        if (getScanner()->getDocHandler())
                            getScanner()->getDocHandler()->docCharacters(elemDefaultValue, XMLString::stringLen(elemDefaultValue), false);

                        // Normally for default value, it has been validated already during TraverseSchema
                        // But if there was a xsi:type and this validator is fXsiTypeValidator,
                        // need to validate again
                        // we determine this if the current content dataype validator
                        // is neither the one in the element nor the one in the current
                        // complex type (if any)
                        if ((fCurrentDatatypeValidator != ((SchemaElementDecl*)elemDecl)->getDatatypeValidator())
                            && (!fTypeStack->peek() || (fCurrentDatatypeValidator != fTypeStack->peek()->getDatatypeValidator()))) {
                            value = elemDefaultValue;
                            validateCanonical = true;
                        }
                        else
                            value = 0;
                    }
                    else
                    {
                        // this element has specified some value
                        // if the flag is FIXED, then this value must be same as default value
                        if ((((SchemaElementDecl*)elemDecl)->getMiscFlags() & SchemaSymbols::XSD_FIXED) != 0)
                        {
                            if (fCurrentDatatypeValidator->compare(value, elemDefaultValue, fMemoryManager) != 0 )
                            {
                                emitError(XMLValid::FixedDifferentFromActual, elemDecl->getFullName());
                                fErrorOccurred = true;
                            }
                        }
                    }
                }

                if ((!fErrorOccurred) && value)
                {
                    try {
                        fCurrentDatatypeValidator->validate(value, getScanner()->getValidationContext(), fMemoryManager);
                        if (validateCanonical) {
                            XMLCh* canonical = (XMLCh*) fCurrentDatatypeValidator->getCanonicalRepresentation(value, fMemoryManager);
                            ArrayJanitor<XMLCh> tempCanonical(canonical, fMemoryManager);
                            fCurrentDatatypeValidator->validate(canonical, getScanner()->getValidationContext(), fMemoryManager);
                        }
                    }
                    catch (XMLException& idve)
                    {
                        emitError (XMLValid::DatatypeError, idve.getCode(), idve.getMessage());
                        fErrorOccurred = true;
                    }
                    catch(const OutOfMemoryException&) {
                        throw;
                    }
                    catch (...)
                    {
                        emitError(XMLValid::GenericError);
                        throw;
                    }
                }
            }
            else if (modelType == SchemaElementDecl::Simple)
            {
                emitError(XMLValid::NoDatatypeValidatorForSimpleType, elemDecl->getFullName());
                fErrorOccurred = true;
            }
            // modelType is any
            else if (elemDefaultValue)
            {
                if (XMLString::equals(value, XMLUni::fgZeroLenString))
                {
                    fElemIsSpecified = true;
                    // if this element didn't specified any value
                    // use default value
                    if (getScanner()->getDocHandler()) {
                        getScanner()->getDocHandler()->docCharacters(elemDefaultValue, XMLString::stringLen(elemDefaultValue), false);
                    }
                }
            }
        }
    }
    else
    {
        ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::CM_UnknownCMType, fMemoryManager);
    }

    // must rely on scanner to clear fDatatypeBuffer
    // since it may need to query its contents after this method completes
    fNil = false;
    fNilFound = false;
    fTrailing=false;
    fSeenNonWhiteSpace = false;
    fCurrentDatatypeValidator = 0;

    // Went ok, so return success
    return true;
}

void SchemaValidator::faultInAttr (XMLAttr&    toFill, const XMLAttDef&  attDef)   const
{
    //
    //  At this level, we cannot set the URI id. So we just set it to zero
    //  and leave it at that. The scanner, who called us, will look at the
    //  prefix we stored (if any), resolve it, and store the URL id if any.
    //
    SchemaAttDef* schemaAttDef = (SchemaAttDef*) &attDef;
    QName* attName = schemaAttDef->getAttName();

    toFill.set
    (
          attName->getURI()
        , attName->getLocalPart()
        , attName->getPrefix()
        , schemaAttDef->getValue()
        , schemaAttDef->getType()
    );
}

void SchemaValidator::reset()
{
    fTrailing = false;
    fSeenNonWhiteSpace = false;
    fSeenId = false;
	fTypeStack->removeAllElements();
    delete fXsiType;
    fXsiType = 0;
    fCurrentDatatypeValidator = 0;
    fNil = false;
    fNilFound = false;
    fDatatypeBuffer.reset();
    fErrorOccurred = false;
}

bool SchemaValidator::requiresNamespaces() const
{
    return true;
}

void SchemaValidator::validateAttrValue (const XMLAttDef*      attDef
                                       , const XMLCh* const    attrValue
                                       , bool                  preValidation
                                       , const XMLElementDecl* elemDecl)
{
    fErrorOccurred = false;

    //turn on IdRefList checking
    getScanner()->getValidationContext()->toCheckIdRefList(true);

    //
    //  Get quick refs to lot of the stuff in the passed objects in
    //  order to simplify the code below, which will reference them very
    //  often.
    //
    XMLAttDef::AttTypes            type      = attDef->getType();
    const XMLAttDef::DefAttTypes   defType   = attDef->getDefaultType();

    //
    //  If the default type is fixed, then make sure the passed value maps
    //  to the fixed value.
    //
    //  If during preContentValidation, the value we are validating is the fixed value itself
    //  so no need to compare.
    //  Only need to do this for regular attribute value validation
    //
    if ((defType == XMLAttDef::Fixed || defType == XMLAttDef::Required_And_Fixed) && !preValidation)
    {
        const XMLCh* const valueText = attDef->getValue();
        if (!XMLString::equals(attrValue, valueText)) {
            emitError(XMLValid::NotSameAsFixedValue, attDef->getFullName(), attrValue, valueText);
            fErrorOccurred = true;
        }
    }

    // An empty string cannot be valid for non_CDATA any of the other types
    if (!attrValue[0] && type != XMLAttDef::Simple)
    {
        emitError(XMLValid::InvalidEmptyAttValue, attDef->getFullName());
        // accords with original DOMTypeInfo implementation, but this does not feel right.
        fMostRecentAttrValidator = DatatypeValidatorFactory::getBuiltInRegistry()->get(SchemaSymbols::fgDT_ANYSIMPLETYPE);
        fErrorOccurred = true;
        return;
    }

    DatatypeValidator* attDefDV = ((SchemaAttDef*) attDef)->getDatatypeValidator();
    if (!attDefDV) {
        emitError(XMLValid::NoDatatypeValidatorForAttribute, attDef->getFullName());
        fErrorOccurred = true;
    }
    else {
        DatatypeValidator::ValidatorType attDefDVType = attDefDV->getType();
        ValidationContext *context = getScanner()->getValidationContext();
        try {

            // first, if notation, need to bind URI to notation first
            if (attDefDVType == DatatypeValidator::NOTATION)
            {
                //
                //  Make sure that this value maps to one of the
                //  notation values in the enumList parameter. We don't have to
                //  look it up in the notation pool (if a notation) because we
                //  will look up the enumerated values themselves. If they are in
                //  the notation pool (after the Grammar is parsed), then obviously
                //  this value will be legal since it matches one of them.
                //
                XMLBuffer notationBuf(1023, fMemoryManager);
                int colonPos = -1;
                unsigned int uriId = getScanner()->resolveQName(attrValue, notationBuf, ElemStack::Mode_Element, colonPos);
                const XMLCh* uriText = getScanner()->getURIText(uriId);
                if (uriText && *uriText) {
                    notationBuf.set(uriText);
                    notationBuf.append(chColon);
                    notationBuf.append(&attrValue[colonPos + 1]);
                }
                else {
                    notationBuf.set(attrValue);
                }

                attDefDV->validate(notationBuf.getRawBuffer()
                                 , context
                                 , fMemoryManager);
            }
            else {
                attDefDV->validate(attrValue
                                 , context
                                 , fMemoryManager);
            }

        }
        catch (XMLException& idve) {
            fErrorOccurred = true;
            emitError (XMLValid::DatatypeError, idve.getCode(), idve.getMessage());
        }
        catch(const OutOfMemoryException&)
        {
            throw;
        }
        catch (...) {
            emitError(XMLValid::GenericError);
            fMostRecentAttrValidator = DatatypeValidatorFactory::getBuiltInRegistry()->get(SchemaSymbols::fgDT_ANYSIMPLETYPE);
            fErrorOccurred = true;
            throw;
        }
        fMostRecentAttrValidator = attDefDV;
        // now we can look for ID's, entities, ...

        // set up the entitydeclpool in ENTITYDatatypeValidator
        // and the idreflist in ID/IDREFDatatypeValidator

        // indicate if this attribute is of type ID
        bool thisIsAnId = false;

        if (attDefDVType == DatatypeValidator::List) {
            DatatypeValidator* itemDTV = ((ListDatatypeValidator*)attDefDV)->getItemTypeDTV();
            DatatypeValidator::ValidatorType itemDTVType = itemDTV->getType();
            if (itemDTVType == DatatypeValidator::ID) {
                thisIsAnId = true;
            }
            else if (itemDTVType == DatatypeValidator::IDREF) {
                // if in prevalidatoin, do not add attDef to IDREFList
                if (preValidation)
                    //todo: when to setIdRefList back to non-null
                    getScanner()->getValidationContext()->toCheckIdRefList(false);
            }
        }
        else if (attDefDVType == DatatypeValidator::Union) {
            DatatypeValidator *memberDTV = context->getValidatingMemberType();
            // actual type for DOMTypeInfo is memberDTV
            fMostRecentAttrValidator = memberDTV;
            // no member datatype validator if there was an error
            if(memberDTV)
            {
                DatatypeValidator::ValidatorType memberDTVType = memberDTV->getType();
                if (memberDTVType == DatatypeValidator::ID) {
                    thisIsAnId = true;
                }
                else if (memberDTVType == DatatypeValidator::IDREF) {
                    // if in prevalidatoin, do not add attDef to IDREFList
                    if (preValidation)
                        getScanner()->getValidationContext()->toCheckIdRefList(false);
                }
            }
        }
        else if (attDefDVType == DatatypeValidator::ID) {
            thisIsAnId = true;
        }
        else if (attDefDVType == DatatypeValidator::IDREF) {
            // if in prevalidation, do not add attDef to IDREFList
            if (preValidation)
                getScanner()->getValidationContext()->toCheckIdRefList(false);
        }
        if (thisIsAnId) {
            if (fSeenId) {
                emitError
                (
                    XMLValid::MultipleIdAttrs
                    , elemDecl->getFullName()
                );
                fErrorOccurred = true;
            }
            else
                fSeenId = true;
        }

    }

    if(fErrorOccurred) {
        fMostRecentAttrValidator = DatatypeValidatorFactory::getBuiltInRegistry()->get(SchemaSymbols::fgDT_ANYSIMPLETYPE);
    }
    fTrailing = false;
    fSeenNonWhiteSpace = false;
}

void SchemaValidator::validateElement(const   XMLElementDecl*  elemDef)
{
    ComplexTypeInfo* elemTypeInfo = ((SchemaElementDecl*)elemDef)->getComplexTypeInfo();
    fTypeStack->push(elemTypeInfo);
    fCurrentDatatypeValidator = (elemTypeInfo)
            ? elemTypeInfo->getDatatypeValidator()
            : ((SchemaElementDecl*)elemDef)->getDatatypeValidator();

    fErrorOccurred = false;

    if (fXsiType) {
        // handle "xsi:type" right here
        DatatypeValidator *xsiTypeDV = 0;
        unsigned int uri = fXsiType->getURI();
        const XMLCh* localPart = fXsiType->getLocalPart();

        if (uri != XMLElementDecl::fgInvalidElemId &&
            uri != XMLElementDecl::fgPCDataElemId &&
            uri != XMLContentModel::gEpsilonFakeId &&
            uri != XMLContentModel::gEOCFakeId) {

            // retrieve Grammar for the uri
            const XMLCh* uriStr = getScanner()->getURIText(uri);
            SchemaGrammar* sGrammar = (SchemaGrammar*) fGrammarResolver->getGrammar(uriStr);
            if (!sGrammar) {

                // Check built-in simple types
                if (XMLString::equals(uriStr, SchemaSymbols::fgURI_SCHEMAFORSCHEMA)) {

                    xsiTypeDV = fGrammarResolver->getDatatypeValidator(uriStr, localPart);

                    if (!xsiTypeDV) {
                        emitError(XMLValid::BadXsiType, fXsiType->getRawName());
                        fErrorOccurred = true;
                    }
                    else {
                        if (elemTypeInfo || (fCurrentDatatypeValidator
                                && !fCurrentDatatypeValidator->isSubstitutableBy(xsiTypeDV))) {
                            // the type is not derived from ancestor
                            emitError(XMLValid::NonDerivedXsiType, fXsiType->getRawName(), elemDef->getFullName());
                            fErrorOccurred = true;
                        }
                        else if(fCurrentDatatypeValidator != xsiTypeDV) 
                        {
                            // the type is derived from ancestor
                            if ((((SchemaElementDecl*)elemDef)->getBlockSet() & SchemaSymbols::XSD_RESTRICTION) != 0) {
                                emitError(XMLValid::ElemNoSubforBlock, elemDef->getFullName());
                                fErrorOccurred = true;
                            }
                            if (elemDef->hasAttDefs()) {
                                // if we have an attribute but xsi:type's type is simple, we have a problem...
                                emitError(XMLValid::NonDerivedXsiType, fXsiType->getRawName(), elemDef->getFullName());
                                fErrorOccurred = true;
                            }
                        }
                        fCurrentDatatypeValidator = xsiTypeDV;
                    }
                }
                else {
                    // Grammar not found
                    emitError(XMLValid::GrammarNotFound, uriStr);
                    fErrorOccurred = true;
                }
            }
            else if (sGrammar->getGrammarType() != Grammar::SchemaGrammarType) {
                emitError(XMLValid::GrammarNotFound, uriStr);
                fErrorOccurred = true;
            }
            else {
                // retrieve complexType registry and DatatypeValidator registry
                RefHashTableOf<ComplexTypeInfo>* complexTypeRegistry = sGrammar->getComplexTypeRegistry();
                if (!complexTypeRegistry) {
                    emitError(XMLValid::BadXsiType, fXsiType->getRawName());
                    fErrorOccurred = true;
                }
                else {

                    // retrieve the typeInfo specified in xsi:type
                    XMLBuffer aBuffer(1023, fMemoryManager);
                    aBuffer.set(uriStr);
                    aBuffer.append(chComma);
                    aBuffer.append(localPart);
                    ComplexTypeInfo* typeInfo = complexTypeRegistry->get(aBuffer.getRawBuffer());

                    if (typeInfo) {
                        // typeInfo is found
                        if (typeInfo->getAbstract()) {
                            emitError(XMLValid::NoAbstractInXsiType, aBuffer.getRawBuffer());
                            fErrorOccurred = true;
                        }
                        else
                        {
                            if (elemTypeInfo)
                            {
                                ComplexTypeInfo* tempType = typeInfo;
                                while (tempType) {
                                    if (tempType == elemTypeInfo)
                                        break;
                                    tempType = tempType->getBaseComplexTypeInfo();
                                }

                                if (!tempType) {
                                    emitError(XMLValid::NonDerivedXsiType, fXsiType->getRawName(), elemDef->getFullName());
                                    fErrorOccurred = true;
                                }
                                else if(elemTypeInfo != typeInfo) {
                                    // perform the check on the entire inheritance chain
                                    ComplexTypeInfo* tempType = typeInfo;
                                    while (tempType) {
                                        if (tempType == elemTypeInfo)
                                            break;
                                        int derivationMethod = tempType->getDerivedBy();
                                        if ((((SchemaElementDecl*)elemDef)->getBlockSet() & derivationMethod) != 0) {
                                            emitError(XMLValid::ElemNoSubforBlock, elemDef->getFullName());
                                            fErrorOccurred = true;
                                        }
                                        if ((elemTypeInfo->getBlockSet() & derivationMethod) != 0) {
                                            emitError(XMLValid::TypeNoSubforBlock, elemTypeInfo->getTypeName());
                                            fErrorOccurred = true;
                                        }
                                        tempType = tempType->getBaseComplexTypeInfo();
                                    }
                                }
                            }
                            else
                            {
                                // if the original type is a simple type, check derivation ok.
                                if (fCurrentDatatypeValidator && !fCurrentDatatypeValidator->isSubstitutableBy(typeInfo->getDatatypeValidator())) {
                                    // the type is not derived from ancestor
                                    emitError(XMLValid::NonDerivedXsiType, fXsiType->getRawName(), elemDef->getFullName());
                                    fErrorOccurred = true;
                                }
                            }

                            if (!fErrorOccurred)
                            {
                                fTypeStack->pop();
                                fTypeStack->push(typeInfo);
                                fCurrentDatatypeValidator = typeInfo->getDatatypeValidator();
                            }
                        }
                    }
                    else
                    {
                        // typeInfo not found
                        xsiTypeDV = fGrammarResolver->getDatatypeValidator(uriStr, localPart);

                        if (!xsiTypeDV) {
                            emitError(XMLValid::BadXsiType, fXsiType->getRawName());
                            fErrorOccurred = true;
                        }
                        else {
                            if (fCurrentDatatypeValidator && !fCurrentDatatypeValidator->isSubstitutableBy(xsiTypeDV)) {
                                // the type is not derived from ancestor
                                emitError(XMLValid::NonDerivedXsiType, fXsiType->getRawName(), elemDef->getFullName());
                                fErrorOccurred = true;
                            }
                            else if(fCurrentDatatypeValidator != xsiTypeDV)
                            {
                                DatatypeValidator::ValidatorType derivedType=xsiTypeDV->getType();
                                if((derivedType == DatatypeValidator::List || derivedType == DatatypeValidator::Union) && fCurrentDatatypeValidator==0)
                                {
                                    // the substitution is always allowed if the type is list or union and the base type was xs:anySimpleType
                                }
                                else
                                {
                                    // the type is derived from ancestor
                                    if ((((SchemaElementDecl*)elemDef)->getBlockSet() & SchemaSymbols::XSD_RESTRICTION) != 0) {
                                        emitError(XMLValid::ElemNoSubforBlock, elemDef->getFullName());
                                        fErrorOccurred = true;
                                    }
                                    if (elemDef->hasAttDefs()) {
                                        // if we have an attribute but xsi:type's type is simple, we have a problem...
                                        emitError(XMLValid::NonDerivedXsiType, fXsiType->getRawName(), elemDef->getFullName());
                                        fErrorOccurred = true;
                                    }
                                }
                            }

                            fCurrentDatatypeValidator = xsiTypeDV;
                        }
                    }
                }
            }
        }

        delete fXsiType;
        fXsiType = 0;
    }
    else {
        //
        // xsi:type was not specified...
        // If the corresponding type is abstract, detect an error
        //
        if (elemTypeInfo && elemTypeInfo->getAbstract()) {
            emitError(XMLValid::NoUseAbstractType, elemDef->getFullName());
            fErrorOccurred = true;
        }
    }

    //
    // Check whether this element is abstract.  If so, an error
    //
    int miscFlags = ((SchemaElementDecl*)elemDef)->getMiscFlags();
    if ((miscFlags & SchemaSymbols::XSD_ABSTRACT) != 0) {
        emitError(XMLValid::NoDirectUseAbstractElement, elemDef->getFullName());
        fErrorOccurred = true;
    }

    //
    // Check whether this element allows Nillable
    //
    if (fNilFound && (miscFlags & SchemaSymbols::XSD_NILLABLE) == 0 ) {
        fNil = false;
        fNilFound = false;
        emitError(XMLValid::NillNotAllowed, elemDef->getFullName());
        fErrorOccurred = true;
    }

    fDatatypeBuffer.reset();
    fTrailing = false;
    fSeenNonWhiteSpace = false;
    fSeenId = false;
}

void SchemaValidator::preContentValidation(bool,
                                           bool validateDefAttr)
{
    //  Lets go through all the grammar in the GrammarResolver
    //    and validate those that has not been validated yet
    //
    //  Lets enumerate all of the elements in the element decl pool
    //    and put out an error for any that did not get declared.
    //    We also check all of the attributes as well.
    //
    //  And enumerate all the complextype info in the grammar
    //    and do Unique Particle Attribution Checking

    RefHashTableOfEnumerator<Grammar> grammarEnum = fGrammarResolver->getGrammarEnumerator();
    while (grammarEnum.hasMoreElements())
    {
        SchemaGrammar& sGrammar = (SchemaGrammar&) grammarEnum.nextElement();
        if (sGrammar.getGrammarType() != Grammar::SchemaGrammarType || sGrammar.getValidated())
             continue;

        sGrammar.setValidated(true);

        RefHash3KeysIdPoolEnumerator<SchemaElementDecl> elemEnum = sGrammar.getElemEnumerator();

        while (elemEnum.hasMoreElements())
        {
            SchemaElementDecl& curElem = elemEnum.nextElement();

            //  First check if declared or not
            //
            //  See if this element decl was ever marked as declared. If
            //  not, then put out an error. In some cases its just
            //  a warning, such as being referenced in a content model.
            //
            const SchemaElementDecl::CreateReasons reason = curElem.getCreateReason();

            if (reason != XMLElementDecl::Declared)
            {
                if (reason == XMLElementDecl::AttList)
                {
                    getScanner()->emitError
                    (
                        XMLErrs::UndeclaredElemInAttList
                        , curElem.getFullName()
                    );
                }
                 else if (reason == XMLElementDecl::AsRootElem)
                {
                    emitError
                    (
                        XMLValid::UndeclaredElemInDocType
                        , curElem.getFullName()
                    );
                }
                 else if (reason == XMLElementDecl::InContentModel)
                {
                    getScanner()->emitError
                    (
                        XMLErrs::UndeclaredElemInCM
                        , curElem.getFullName()
                    );
                }
                else
                {
                }
            }

            //
            //  Then check all of the attributes of the current element.
            //  We check for:
            //
            //  1) Multiple ID attributes
            //  2) That all of the default values of attributes are
            //      valid for their type.
            //  3) That for any notation types, that their lists
            //      of possible values refer to declared notations.
            //
            if (curElem.hasAttDefs()) {
                XMLAttDefList& attDefList = curElem.getAttDefList();
                bool seenId = false;

                for(XMLSize_t i=0; i<attDefList.getAttDefCount(); i++)
                {
                    const XMLAttDef& curAttDef = attDefList.getAttDef(i);

                    if (curAttDef.getType() == XMLAttDef::ID)
                    {
                        if (seenId)
                        {
                            emitError
                            (
                                XMLValid::MultipleIdAttrs
                                , curElem.getFullName()
                            );
                            break;
                        }

                        seenId = true;
                    }
                     else if (curAttDef.getType() == XMLAttDef::Notation && curAttDef.getEnumeration())
                    {
                        //
                        //  We need to verify that all of its possible values
                        //  (in the enum list) refer to valid notations.
                        //
                        XMLCh* list = XMLString::replicate(curAttDef.getEnumeration(), fMemoryManager);
                        ArrayJanitor<XMLCh> janList(list, fMemoryManager);

                        //
                        //  Search forward for a space or a null. If a null,
                        //  we are done. If a space, cap it and look it up.
                        //
                        bool    breakFlag = false;
                        XMLCh*  listPtr = list;
                        XMLCh*  lastPtr = listPtr;
                        while (true)
                        {
                            while (*listPtr && (*listPtr != chSpace))
                                listPtr++;

                            //
                            //  If at the end, indicate we need to break after
                            //  this one. Else, cap it off here.
                            //
                            if (!*listPtr)
                                breakFlag = true;
                            else
                                *listPtr = chNull;

                            if (!sGrammar.getNotationDecl(lastPtr))
                            {
                                emitError
                                (
                                    XMLValid::UnknownNotRefAttr
                                    , curAttDef.getFullName()
                                    , lastPtr
                                );
                            }

                            // Break out if we hit the end last time
                            if (breakFlag)
                                break;

                            // Else move upwards and try again
                            listPtr++;
                            lastPtr = listPtr;
                        }
                    }

                    // If it has a default/fixed value, then validate it
                    if (validateDefAttr && curAttDef.getValue())
                    {
                        validateAttrValue
                        (
                            &curAttDef
                            , curAttDef.getValue()
                            , true
                            , &curElem
                        );
                    }
                }
            }
        }

        //  For each complex type info, check the Unique Particle Attribution
        if (getScanner()->getValidationSchemaFullChecking()) {
            RefHashTableOf<ComplexTypeInfo>* complexTypeRegistry = sGrammar.getComplexTypeRegistry();

            RefHashTableOfEnumerator<ComplexTypeInfo> complexTypeEnum(complexTypeRegistry, false, fMemoryManager);
            while (complexTypeEnum.hasMoreElements())
            {
                ComplexTypeInfo& curTypeInfo = complexTypeEnum.nextElement();
                curTypeInfo.checkUniqueParticleAttribution(&sGrammar, fGrammarResolver, fGrammarResolver->getStringPool(), this);
                checkParticleDerivation(&sGrammar, &curTypeInfo);
                checkRefElementConsistency(&sGrammar, &curTypeInfo);
            }

            RefHashTableOf<XercesGroupInfo>* groupInfoRegistry = sGrammar.getGroupInfoRegistry();
            RefHashTableOfEnumerator<XercesGroupInfo> groupEnum(groupInfoRegistry, false, fMemoryManager);

            while (groupEnum.hasMoreElements()) {

                XercesGroupInfo& curGroup = groupEnum.nextElement();
                XercesGroupInfo* baseGroup = curGroup.getBaseGroup();

                if (baseGroup) {
                    try {
                        checkParticleDerivationOk(&sGrammar, curGroup.getContentSpec(), curGroup.getScope(),
                                                  baseGroup->getContentSpec(), baseGroup->getScope());
                    }
                    catch (const XMLException& excep) {
                        fSchemaErrorReporter.emitError(excep, curGroup.getLocator());
					}
                }

                if (curGroup.getCheckElementConsistency())
                    checkRefElementConsistency(&sGrammar, 0, &curGroup);
            }
        }
    }
}

void SchemaValidator::postParseValidation()
{
    //
    //  At this time, there is nothing to do here. The scanner itself handles
    //  ID/IDREF validation, since that is the same no matter what kind of
    //  validator.
    //
}

// ---------------------------------------------------------------------------
//  SchemaValidator: Validator method
// ---------------------------------------------------------------------------
// Do Schema Normalization depends on the WhiteSpace Facet
// preserve : No normalization is done
// replace  : All occurrences of #x9 (tab), #xA (linefeed) and #xD (carriage return)
//            are replaced with #x20 (space).
// collapse : Subsequent to the replacements specified above under replace,
//            contiguous sequences of #x20s are collapsed to a single #x20,
//            and initial and/or final #x20s are deleted.
//
void SchemaValidator::normalizeWhiteSpace(DatatypeValidator* dV, const XMLCh* const value, XMLBuffer& toFill, bool bStandalone /*= false*/)
{
    toFill.reset();

    //empty string
    if (!*value)
        return;

    if(bStandalone)
        fTrailing = fSeenNonWhiteSpace = false;

    short wsFacet = dV->getWSFacet();

    //  Loop through the chars of the source value and normalize it
    //  according to the whitespace facet
    XMLCh nextCh;
    const XMLCh* srcPtr = value;
    XMLReader* fCurReader = getReaderMgr()->getCurrentReader();

    if (wsFacet == DatatypeValidator::REPLACE)
    {
        while (*srcPtr)
        {
            nextCh = *srcPtr++;
            if (fCurReader->isWhitespace(nextCh))
                nextCh = chSpace;
            // Add this char to the target buffer
            toFill.append(nextCh);
        }
    }
    else // COLLAPSE
    {
        enum States
        {
            InWhitespace
            , InContent
        };

        States curState = fTrailing ? InWhitespace : InContent;
        while (*srcPtr)
        {
            nextCh = *srcPtr++;
            if (curState == InContent)
            {
                if (fCurReader->isWhitespace(nextCh))
                {
                    curState = InWhitespace;
                    continue;
                }
                fSeenNonWhiteSpace = true;
            }
            else if (curState == InWhitespace)
            {
                if (fCurReader->isWhitespace(nextCh))
                    continue;
                if (fSeenNonWhiteSpace)
                    toFill.append(chSpace);
                curState = InContent;
                fSeenNonWhiteSpace = true;
            }
            // Add this char to the target buffer
            toFill.append(nextCh);
        }

        if (fCurReader->isWhitespace(*(srcPtr-1)))
          fTrailing = true;
        else
          fTrailing = false;
    }
    if(bStandalone)
        fTrailing = fSeenNonWhiteSpace = false;
}


// ---------------------------------------------------------------------------
//  SchemaValidator: Particle Derivation Checking
// ---------------------------------------------------------------------------
void SchemaValidator::checkRefElementConsistency(SchemaGrammar* const currentGrammar,
                                                 const ComplexTypeInfo* const curTypeInfo,
                                                 const XercesGroupInfo* const curGroup) {

    XMLSize_t elemCount = (curTypeInfo) ? curTypeInfo->elementCount() : curGroup->elementCount();
    int elemScope = (curTypeInfo) ? curTypeInfo->getScopeDefined() : curGroup->getScope();
    XSDLocator* typeInfoLocator = (curTypeInfo) ? curTypeInfo->getLocator() : curGroup->getLocator();

    for (XMLSize_t i=0; i < elemCount; i++) {

        const SchemaElementDecl* elemDecl = (curTypeInfo) ? curTypeInfo->elementAt(i) : curGroup->elementAt(i);

        if (elemDecl->isGlobalDecl()) {

            unsigned int elemURI = elemDecl->getURI();
            const XMLCh* elemName = elemDecl->getBaseName();
            const SchemaElementDecl* other = (SchemaElementDecl*)
                currentGrammar->getElemDecl(elemURI, elemName, 0, elemScope);

            if (other
                && (elemDecl->getComplexTypeInfo() != other->getComplexTypeInfo() ||
                    elemDecl->getDatatypeValidator() != other->getDatatypeValidator())) {
                fSchemaErrorReporter.emitError(XMLErrs::DuplicateElementDeclaration,
                                               XMLUni::fgXMLErrDomain, typeInfoLocator, elemName, 0, 0, 0, fMemoryManager);
                continue;
            }

            RefHash2KeysTableOf<ElemVector>* validSubsGroups = currentGrammar->getValidSubstitutionGroups();
            ValueVectorOf<SchemaElementDecl*>* subsElements = validSubsGroups->get(elemName, elemURI);

            if (subsElements) {

                XMLSize_t subsElemSize = subsElements->size();

                for (XMLSize_t j=0; j < subsElemSize; j++) {

                    SchemaElementDecl* subsElem = subsElements->elementAt(j);
                    const XMLCh* subsElemName = subsElem->getBaseName();
                    other = (SchemaElementDecl*)
                        currentGrammar->getElemDecl(subsElem->getURI(), subsElemName, 0, elemScope);

                    if (other
                        && (subsElem->getComplexTypeInfo() != other->getComplexTypeInfo()
                            || subsElem->getDatatypeValidator() != other->getDatatypeValidator())) {
                        fSchemaErrorReporter.emitError(XMLErrs::DuplicateElementDeclaration,
                                                       XMLUni::fgXMLErrDomain, typeInfoLocator, elemName, 0, 0, 0, fMemoryManager);
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
//  SchemaValidator: Particle Derivation Checking
// ---------------------------------------------------------------------------
void SchemaValidator::checkParticleDerivation(SchemaGrammar* const currentGrammar,
                                              const ComplexTypeInfo* const curTypeInfo) {

    ComplexTypeInfo* baseTypeInfo = 0;
    ContentSpecNode* curSpecNode = 0;

    if (curTypeInfo->getDerivedBy() == SchemaSymbols::XSD_RESTRICTION
        && ((baseTypeInfo = curTypeInfo->getBaseComplexTypeInfo()) != 0)
        && ((curSpecNode = curTypeInfo->getContentSpec()) != 0)) {

        try {
            checkParticleDerivationOk(currentGrammar, curSpecNode,
                                      curTypeInfo->getScopeDefined(),
                                      baseTypeInfo->getContentSpec(),
                                      baseTypeInfo->getScopeDefined(), baseTypeInfo);
        }
        catch (const XMLException& excep) {
            fSchemaErrorReporter.emitError(excep, curTypeInfo->getLocator());
        }
    }
}

ContentSpecNode* SchemaValidator::getNonUnaryGroup(ContentSpecNode* const pNode) {

    int pNodeType = (pNode->getType() & 0x0f);
    if (pNodeType == ContentSpecNode::Leaf
        || pNodeType == ContentSpecNode::Any
        || pNodeType == ContentSpecNode::Any_Other
        || pNodeType == ContentSpecNode::Any_NS)
        return pNode;

    if (pNode->getMinOccurs() == 1 && pNode->getMaxOccurs() == 1
        && pNode->getFirst() && !pNode->getSecond())
        return getNonUnaryGroup(pNode->getFirst());

    return pNode;
}

void SchemaValidator::checkParticleDerivationOk(SchemaGrammar* const aGrammar,
                                                ContentSpecNode* const curNode,
                                                const int derivedScope,
                                                ContentSpecNode* const baseNode,
                                                const int baseScope,
                                                const ComplexTypeInfo* const baseInfo,
                                                const bool toCheckOccurence) {

    // Check for pointless occurrences of all, choice, sequence.  The result is
    // the contentspec which is not pointless. If the result is a non-pointless
    // group, Vector is filled  in with the children of interest
    if (curNode && !baseNode)
        ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::PD_EmptyBase, fMemoryManager);

    if (!curNode)
        return;

    ContentSpecNode* curSpecNode = getNonUnaryGroup(curNode);
    ContentSpecNode* baseSpecNode = getNonUnaryGroup(baseNode);
    ValueVectorOf<ContentSpecNode*> curVector(8, fMemoryManager);
    ValueVectorOf<ContentSpecNode*> baseVector(8, fMemoryManager);
    ContentSpecNode::NodeTypes curNodeType = curSpecNode->getType();
    ContentSpecNode::NodeTypes baseNodeType = baseSpecNode->getType();

    if ((curNodeType & 0x0f) == ContentSpecNode::Sequence ||
        (curNodeType & 0x0f) == ContentSpecNode::Choice ||
        curNodeType == ContentSpecNode::All) {
        curSpecNode = checkForPointlessOccurrences(curSpecNode, curNodeType, &curVector);
    }

    if ((baseNodeType & 0x0f) == ContentSpecNode::Sequence ||
        (baseNodeType & 0x0f) == ContentSpecNode::Choice ||
        baseNodeType == ContentSpecNode::All) {
        baseSpecNode = checkForPointlessOccurrences(baseSpecNode, baseNodeType, &baseVector);
    }

    curNodeType = curSpecNode->getType();
    baseNodeType = baseSpecNode->getType();

    switch (curNodeType & 0x0f) {
    case ContentSpecNode::Leaf:
        {
            switch (baseNodeType & 0x0f) {
            case ContentSpecNode::Leaf:
                {
                    checkNameAndTypeOK(aGrammar, curSpecNode, derivedScope, baseSpecNode, baseScope, baseInfo);
                    return;
                }
            case ContentSpecNode::Any:
            case ContentSpecNode::Any_Other:
            case ContentSpecNode::Any_NS:
                {
                    checkNSCompat(curSpecNode, baseSpecNode, toCheckOccurence);
                    return;
                }
            case ContentSpecNode::Choice:
            case ContentSpecNode::Sequence:
            case ContentSpecNode::All:
                {
                    checkRecurseAsIfGroup(aGrammar, curSpecNode, derivedScope,
                                          baseSpecNode, baseScope, &baseVector, baseInfo);
                    return;
                }
            default:
                {
                    ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::PD_InvalidContentType, fMemoryManager);
                }
            }
        }
    case ContentSpecNode::Any:
    case ContentSpecNode::Any_Other:
    case ContentSpecNode::Any_NS:
        {
            switch (baseNodeType & 0x0f) {
            case ContentSpecNode::Any:
            case ContentSpecNode::Any_Other:
            case ContentSpecNode::Any_NS:
                {
                     checkNSSubset(curSpecNode, baseSpecNode);
                     return;
                }
            case ContentSpecNode::Choice:
            case ContentSpecNode::Sequence:
            case ContentSpecNode::All:
            case ContentSpecNode::Leaf:
                {
                    if (baseNodeType == ContentSpecNode::Any_NS_Choice) {
                        if (checkNSSubsetChoiceRoot(curSpecNode, baseSpecNode)) {
                            return;
                        }
                    }

                    ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::PD_ForbiddenRes1, fMemoryManager);
                }
            default:
                {
                    ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::PD_InvalidContentType, fMemoryManager);
                }
            }
        }
    case ContentSpecNode::All:
        {
            switch (baseNodeType & 0x0f) {
            case ContentSpecNode::Any:
            case ContentSpecNode::Any_Other:
            case ContentSpecNode::Any_NS:
                {
                    checkNSRecurseCheckCardinality(aGrammar, curSpecNode, &curVector, derivedScope, baseSpecNode, toCheckOccurence);
                    return;
                }
            case ContentSpecNode::All:
                {
                    checkRecurse(aGrammar, curSpecNode, derivedScope, &curVector,
                                 baseSpecNode, baseScope, &baseVector, baseInfo);
                    return;
                }
            case ContentSpecNode::Choice:
            case ContentSpecNode::Sequence:
            case ContentSpecNode::Leaf:
                {
                    ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::PD_ForbiddenRes2, fMemoryManager);
                }
            default:
                {
                    ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::PD_InvalidContentType, fMemoryManager);
                }
            }
        }
    case ContentSpecNode::Choice:
        {
            switch (baseNodeType & 0x0f) {
            case ContentSpecNode::Any:
            case ContentSpecNode::Any_Other:
            case ContentSpecNode::Any_NS:
                {
                    checkNSRecurseCheckCardinality(aGrammar, curSpecNode, &curVector, derivedScope, baseSpecNode, toCheckOccurence);
                    return;
                }
            case ContentSpecNode::Choice:
                {
                    checkRecurse(aGrammar, curSpecNode, derivedScope, &curVector,
                                 baseSpecNode, baseScope, &baseVector, baseInfo, true);
                    return;
                }
            case ContentSpecNode::All:
            case ContentSpecNode::Sequence:
            case ContentSpecNode::Leaf:
                {
                    ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::PD_ForbiddenRes3, fMemoryManager);
                }
            default:
                {
                    ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::PD_InvalidContentType, fMemoryManager);
                }
            }
        }
    case ContentSpecNode::Sequence:
        {
            switch (baseNodeType & 0x0f) {
            case ContentSpecNode::Any:
            case ContentSpecNode::Any_Other:
            case ContentSpecNode::Any_NS:
                {
                    checkNSRecurseCheckCardinality(aGrammar, curSpecNode, &curVector, derivedScope, baseSpecNode, toCheckOccurence);
                    return;
                }
            case ContentSpecNode::All:
                {
                    checkRecurseUnordered(aGrammar, curSpecNode, &curVector, derivedScope,
                                          baseSpecNode, &baseVector, baseScope, baseInfo);
                    return;
                }
            case ContentSpecNode::Sequence:
                {
                    checkRecurse(aGrammar, curSpecNode, derivedScope, &curVector,
                                 baseSpecNode, baseScope, &baseVector, baseInfo);
                    return;
                }
            case ContentSpecNode::Choice:
                {
                    checkMapAndSum(aGrammar, curSpecNode, &curVector, derivedScope,
                                   baseSpecNode, &baseVector, baseScope, baseInfo);
                    return;
                }
            case ContentSpecNode::Leaf:
                {
                    ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::PD_ForbiddenRes4, fMemoryManager);
                }
            default:
                {
                    ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::PD_InvalidContentType, fMemoryManager);
                }
            }
        }
    }
}

ContentSpecNode*
SchemaValidator::checkForPointlessOccurrences(ContentSpecNode* const specNode,
                                              const ContentSpecNode::NodeTypes nodeType,
                                              ValueVectorOf<ContentSpecNode*>* const nodes) {

    ContentSpecNode* rightNode = specNode->getSecond();
    int min = specNode->getMinOccurs();
    int max = specNode->getMaxOccurs();

    if (!rightNode) {

         gatherChildren(nodeType, specNode->getFirst(), nodes);

         if (nodes->size() == 1 && min == 1 && max == 1) {
            return nodes->elementAt(0);
        }

        return specNode;
    }

    gatherChildren(nodeType, specNode->getFirst(), nodes);
    gatherChildren(nodeType, rightNode, nodes);

    return specNode;
}

void SchemaValidator::gatherChildren(const ContentSpecNode::NodeTypes parentNodeType,
                                    ContentSpecNode* const specNode,
                                    ValueVectorOf<ContentSpecNode*>* const nodes) {

    if (!specNode) {
        return;
    }

    int min = specNode->getMinOccurs();
    int max = specNode->getMaxOccurs();
    ContentSpecNode::NodeTypes nodeType = specNode->getType();
    ContentSpecNode* rightNode = specNode->getSecond();

    if (nodeType == ContentSpecNode::Leaf ||
        (nodeType & 0x0f) == ContentSpecNode::Any ||
        (nodeType & 0x0f) == ContentSpecNode::Any_NS ||
        (nodeType & 0x0f) == ContentSpecNode::Any_Other) {
        nodes->addElement(specNode);
    }
    else if (min !=1 || max != 1) {
        nodes->addElement(specNode);
    }
    else if (!rightNode) {
        gatherChildren(nodeType, specNode->getFirst(), nodes);
    }
    else if ((parentNodeType & 0x0f) == (nodeType & 0x0f)) {

        gatherChildren(nodeType, specNode->getFirst(), nodes);
        gatherChildren(nodeType, rightNode, nodes);
    }
    else {
        nodes->addElement(specNode);
    }
}

void
SchemaValidator::checkNSCompat(const ContentSpecNode* const derivedSpecNode,
                               const ContentSpecNode* const baseSpecNode,
                               const bool toCheckOccurence) {

    // check Occurrence ranges
    if (toCheckOccurence &&
        !isOccurrenceRangeOK(derivedSpecNode->getMinOccurs(), derivedSpecNode->getMaxOccurs(),
                             baseSpecNode->getMinOccurs(), baseSpecNode->getMaxOccurs())) {
        ThrowXMLwithMemMgr1(RuntimeException, XMLExcepts::PD_OccurRangeE,
                  derivedSpecNode->getElement()->getLocalPart(), fMemoryManager);
    }

    // check wildcard subset
    if (!wildcardEltAllowsNamespace(baseSpecNode, derivedSpecNode->getElement()->getURI())) {
        ThrowXMLwithMemMgr1(RuntimeException, XMLExcepts::PD_NSCompat1,
                  derivedSpecNode->getElement()->getLocalPart(), fMemoryManager);
    }
}

bool
SchemaValidator::wildcardEltAllowsNamespace(const ContentSpecNode* const baseSpecNode,
                                            const unsigned int derivedURI) {

    ContentSpecNode::NodeTypes nodeType = baseSpecNode->getType();

    if ((nodeType & 0x0f) == ContentSpecNode::Any) {
        return true;
    }

    unsigned int baseURI = baseSpecNode->getElement()->getURI();

    if ((nodeType & 0x0f) == ContentSpecNode::Any_NS) {
        if (derivedURI == baseURI) {
           return true;
        }
    }
    else { // must be ANY_OTHER
        if (derivedURI != baseURI && derivedURI != getScanner()->getEmptyNamespaceId()) {
            return true;
        }
    }

    return false;
}

void
SchemaValidator::checkNameAndTypeOK(SchemaGrammar* const currentGrammar,
                                    const ContentSpecNode* const derivedSpecNode,
                                    const int derivedScope,
                                    const ContentSpecNode* const baseSpecNode,
                                    const int baseScope,
                                    const ComplexTypeInfo* const baseInfo) {

    if (derivedSpecNode->getMaxOccurs() == 0)
        return;

    unsigned int derivedURI = derivedSpecNode->getElement()->getURI();

    // case of mixed complex types with attributes only
    if (derivedURI == XMLElementDecl::fgPCDataElemId) {
        return;
    }

    SchemaGrammar* dGrammar = currentGrammar;

    if (derivedURI != getScanner()->getEmptyNamespaceId())
    {
        const XMLCh* dURI = fGrammarResolver->getStringPool()->getValueForId(derivedURI);
        dGrammar= (SchemaGrammar*) fGrammarResolver->getGrammar(dURI);
    }

    if (!dGrammar) { //something is wrong
        return;
    }

    const XMLCh* derivedName = derivedSpecNode->getElement()->getLocalPart();

    SchemaElementDecl* derivedElemDecl = findElement(derivedScope, derivedURI, derivedName, dGrammar);

    if (!derivedElemDecl) {
        return;
    }

    const XMLCh* baseName = baseSpecNode->getElement()->getLocalPart();
    unsigned int baseURI = baseSpecNode->getElement()->getURI();
    bool subsGroup = false;

    if (!XMLString::equals(derivedName, baseName) || derivedURI != baseURI) {
        // Check if derived is substitutable for base.
        //
        SchemaElementDecl* e = derivedElemDecl->getSubstitutionGroupElem ();

        for (; e != 0; e = e->getSubstitutionGroupElem ()) {
            if (XMLString::equals(e->getBaseName (), baseName) && e->getURI () == baseURI) {
                break;
            }
        }

        if (e == 0) {
            ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::PD_NameTypeOK1, fMemoryManager);
        }

        subsGroup = true;
    }

    if (!isOccurrenceRangeOK(derivedSpecNode->getMinOccurs(), derivedSpecNode->getMaxOccurs(),
                             baseSpecNode->getMinOccurs(), baseSpecNode->getMaxOccurs())) {
        ThrowXMLwithMemMgr1(RuntimeException, XMLExcepts::PD_OccurRangeE, derivedName, fMemoryManager);
    }

    // Find the schema grammar for the base element using the base type as
    // a reference if it is available (it is unavailable if we are checking
    // element group restriction which happens in redefine).
    //
    SchemaGrammar* bGrammar = dGrammar;

    if (baseInfo)
    {
        const XMLCh* baseTypeURI = baseInfo->getTypeUri ();

        if (baseTypeURI != 0 && *baseTypeURI != 0) // Non-empty namespace.
            bGrammar= (SchemaGrammar*) fGrammarResolver->getGrammar(baseTypeURI);

        if (!bGrammar) { //something is wrong
            return;
        }
    }

    SchemaElementDecl* baseElemDecl =
        findElement(baseScope, baseURI, baseName, bGrammar, baseInfo);

    if (!baseElemDecl) {
        return;
    }

    int derivedFlags = derivedElemDecl->getMiscFlags();
    int baseFlags = baseElemDecl->getMiscFlags();

    if (((baseFlags & SchemaSymbols::XSD_NILLABLE) == 0) &&
		((derivedFlags & SchemaSymbols::XSD_NILLABLE) != 0)) {
        ThrowXMLwithMemMgr1(RuntimeException, XMLExcepts::PD_NameTypeOK2, derivedName, fMemoryManager);
    }

    const XMLCh* derivedDefVal = derivedElemDecl->getDefaultValue();
    const XMLCh* baseDefVal = baseElemDecl->getDefaultValue();

    if (baseDefVal && (baseFlags & SchemaSymbols::XSD_FIXED) != 0 &&
        ((derivedFlags & SchemaSymbols::XSD_FIXED) == 0 ||
         !XMLString::equals(derivedDefVal, baseDefVal))) {
        ThrowXMLwithMemMgr1(RuntimeException, XMLExcepts::PD_NameTypeOK3, derivedName, fMemoryManager);
    }

    int derivedBlockSet = derivedElemDecl->getBlockSet();
    int baseBlockSet = baseElemDecl->getBlockSet();

    if ((derivedBlockSet & baseBlockSet) != baseBlockSet) {
        ThrowXMLwithMemMgr1(RuntimeException, XMLExcepts::PD_NameTypeOK4, derivedName, fMemoryManager);
    }

    // check identity constraints
    checkICRestriction(derivedElemDecl, baseElemDecl, derivedName, baseName);

    // check that the derived element's type is derived from the base's.
    if (!subsGroup)
        checkTypesOK(derivedElemDecl, baseElemDecl, derivedName);
}

SchemaElementDecl*
SchemaValidator::findElement(const int scope, const unsigned int uriIndex,
                             const XMLCh* const name,
                             SchemaGrammar* const grammar,
                             const ComplexTypeInfo* const typeInfo) {

    // check for element at given scope first
    SchemaElementDecl* elemDecl = (SchemaElementDecl*) grammar->getElemDecl(uriIndex, name, 0, scope);

    // if not found, check at global scope
    if (!elemDecl) {

        elemDecl = (SchemaElementDecl*)
            grammar->getElemDecl(uriIndex, name, 0, Grammar::TOP_LEVEL_SCOPE);

        // if still not found, and base is specified, look it up there
        if (!elemDecl && typeInfo) {

            const ComplexTypeInfo* baseInfo = typeInfo;

            while (baseInfo) {

                elemDecl = (SchemaElementDecl*)
                    grammar->getElemDecl(uriIndex, name, 0, baseInfo->getScopeDefined());

                if (elemDecl) {
                   break;
                }

                baseInfo = baseInfo->getBaseComplexTypeInfo();
            }
        }
    }

    return elemDecl;
}

void
SchemaValidator::checkICRestriction(const SchemaElementDecl* const derivedElemDecl,
                                   const SchemaElementDecl* const baseElemDecl,
                                   const XMLCh* const derivedElemName,
                                   const XMLCh* const baseElemName) {

    // REVIST - need to get more clarification
    XMLSize_t derivedICCount = derivedElemDecl->getIdentityConstraintCount();
    XMLSize_t baseICCount = baseElemDecl->getIdentityConstraintCount();

    if (derivedICCount > baseICCount) {
        ThrowXMLwithMemMgr2(RuntimeException, XMLExcepts::PD_NameTypeOK6, derivedElemName, baseElemName, fMemoryManager);
    }

    for (XMLSize_t i=0; i < derivedICCount; i++) {

        bool found = false;
        IdentityConstraint* ic= derivedElemDecl->getIdentityConstraintAt(i);

        for (XMLSize_t j=0; j < baseICCount; j++) {
            if (*ic == *(baseElemDecl->getIdentityConstraintAt(j))) {

                found = true;
                break;
            }
        }

        if (!found) {
            ThrowXMLwithMemMgr2(RuntimeException, XMLExcepts::PD_NameTypeOK7, derivedElemName, baseElemName, fMemoryManager);
        }
    }
}

void
SchemaValidator::checkTypesOK(const SchemaElementDecl* const derivedElemDecl,
                              const SchemaElementDecl* const baseElemDecl,
                              const XMLCh* const derivedElemName) {

    SchemaElementDecl::ModelTypes baseType = baseElemDecl->getModelType();

    if (baseType == SchemaElementDecl::Any) {
        return;
    }

    ComplexTypeInfo* rInfo = derivedElemDecl->getComplexTypeInfo();
    ComplexTypeInfo* bInfo = baseElemDecl->getComplexTypeInfo();

    if (derivedElemDecl->getModelType() == SchemaElementDecl::Simple) {

        if (baseType != SchemaElementDecl::Simple) {
            ThrowXMLwithMemMgr1(RuntimeException, XMLExcepts::PD_NameTypeOK5, derivedElemName, fMemoryManager);
        }

        if (!rInfo) {

            DatatypeValidator* bDV = baseElemDecl->getDatatypeValidator();

            if (bInfo || bDV == 0 ||
				!bDV->isSubstitutableBy(derivedElemDecl->getDatatypeValidator())) {
                ThrowXMLwithMemMgr1(RuntimeException, XMLExcepts::PD_NameTypeOK5, derivedElemName, fMemoryManager);
            }

            return;
        }
    }

    if (rInfo == bInfo)
        return;

    for (; rInfo && rInfo != bInfo; rInfo = rInfo->getBaseComplexTypeInfo()) {
        if (rInfo->getDerivedBy() != SchemaSymbols::XSD_RESTRICTION) {

            rInfo = 0;
            break;
        }
    }

    if (!rInfo) {
        ThrowXMLwithMemMgr1(RuntimeException, XMLExcepts::PD_NameTypeOK5, derivedElemName, fMemoryManager);
    }
}

void
SchemaValidator::checkRecurseAsIfGroup(SchemaGrammar* const currentGrammar,
                                       ContentSpecNode* const derivedSpecNodeIn,
                                       const int derivedScope,
                                       const ContentSpecNode* const baseSpecNode,
                                       const int baseScope,
                                       ValueVectorOf<ContentSpecNode*>* const baseNodes,
                                       const ComplexTypeInfo* const baseInfo) {

    ContentSpecNode::NodeTypes baseType = baseSpecNode->getType();
    bool toLax = false;

    //Treat the element as if it were in a group of the same variety as base
    ContentSpecNode derivedGroupNode(baseType, derivedSpecNodeIn, 0, false, true, fMemoryManager);
    const ContentSpecNode* const derivedSpecNode = &derivedGroupNode;

    if ((baseSpecNode->getType() & 0x0f) == ContentSpecNode::Choice) {
        toLax = true;
    }

    // Instead of calling this routine, inline it
    // checkRecurse(currentGrammar, &derivedGroupNode, derivedScope, &derivedNodes,
    //             baseSpecNode, baseScope, baseNodes, baseInfo, toLax);

    if (!isOccurrenceRangeOK(derivedSpecNode->getMinOccurs(), derivedSpecNode->getMaxOccurs(),
                             baseSpecNode->getMinOccurs(), baseSpecNode->getMaxOccurs())) {
        ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::PD_Recurse1, fMemoryManager);
    }

    // check for mapping of children
    XMLExcepts::Codes codeToThrow = XMLExcepts::NoError;
    XMLSize_t count2= baseNodes->size();
    XMLSize_t current = 0;

    {
        bool matched = false;

        for (XMLSize_t j = current; j < count2; j++) {

            ContentSpecNode* baseNode = baseNodes->elementAt(j);
            current++;

            bool bDoBreak=false;    // workaround for Borland bug with 'break' in 'catch'
            try {

                checkParticleDerivationOk(currentGrammar, derivedSpecNodeIn,
                                          derivedScope, baseNode, baseScope, baseInfo);
                matched = true;
                break;
            }
            catch(const XMLException&) {
                if (!toLax && baseNode->getMinTotalRange()) {
                    bDoBreak=true;
                }
            }
            if(bDoBreak)
                break;
        }

        // did not find a match
        if (!matched) {
            codeToThrow = XMLExcepts::PD_Recurse2;
        }
    }

    // Now, see if there are some elements in the base we didn't match up
    // in case of Sequence or All
    if (!toLax && codeToThrow == XMLExcepts::NoError) {
        for (XMLSize_t j = current; j < count2; j++) {
            if (baseNodes->elementAt(j)->getMinTotalRange() * baseSpecNode->getMinOccurs()) { //!emptiable
                codeToThrow =  XMLExcepts::PD_Recurse2;
                break;
            }
        }
    }

    if (codeToThrow != XMLExcepts::NoError) {
        ThrowXMLwithMemMgr(RuntimeException, codeToThrow, fMemoryManager);
    }
}

void
SchemaValidator::checkRecurse(SchemaGrammar* const currentGrammar,
                              const ContentSpecNode* const derivedSpecNode,
                              const int derivedScope,
                              ValueVectorOf<ContentSpecNode*>* const derivedNodes,
                              const ContentSpecNode* const baseSpecNode,
                              const int baseScope,
                              ValueVectorOf<ContentSpecNode*>* const baseNodes,
                              const ComplexTypeInfo* const baseInfo,
                              const bool toLax) {

    if (!isOccurrenceRangeOK(derivedSpecNode->getMinOccurs(), derivedSpecNode->getMaxOccurs(),
                             baseSpecNode->getMinOccurs(), baseSpecNode->getMaxOccurs())) {
        ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::PD_Recurse1, fMemoryManager);
    }

    // check for mapping of children
    XMLExcepts::Codes codeToThrow = XMLExcepts::NoError;
    XMLSize_t count1= derivedNodes->size();
    XMLSize_t count2= baseNodes->size();
    XMLSize_t current = 0;

    for (XMLSize_t i=0; i<count1; i++) {

        bool matched = false;

        for (XMLSize_t j = current; j < count2; j++) {

            ContentSpecNode* baseNode = baseNodes->elementAt(j);
            current++;

            bool bDoBreak=false;    // workaround for Borland bug with 'break' in 'catch'
            try {

                checkParticleDerivationOk(currentGrammar, derivedNodes->elementAt(i),
                                          derivedScope, baseNode, baseScope, baseInfo);
                matched = true;
                break;
            }
            catch(const XMLException&) {
                if (!toLax && baseNode->getMinTotalRange()) {
                    bDoBreak=true;
                }
            }
            if(bDoBreak)
                break;
        }

        // did not find a match
        if (!matched) {

            codeToThrow = XMLExcepts::PD_Recurse2;
            break;
        }
    }

    // Now, see if there are some elements in the base we didn't match up
    // in case of Sequence or All
    if (!toLax && codeToThrow == XMLExcepts::NoError) {
        for (XMLSize_t j = current; j < count2; j++) {
            if (baseNodes->elementAt(j)->getMinTotalRange()) { //!emptiable
                codeToThrow =  XMLExcepts::PD_Recurse2;
                break;
            }
        }
    }

    if (codeToThrow != XMLExcepts::NoError) {
        ThrowXMLwithMemMgr(RuntimeException, codeToThrow, fMemoryManager);
    }
}

void SchemaValidator::checkNSSubset(const ContentSpecNode* const derivedSpecNode,
                                    const ContentSpecNode* const baseSpecNode) {

    // check Occurrence ranges
    if (!isOccurrenceRangeOK(derivedSpecNode->getMinOccurs(), derivedSpecNode->getMaxOccurs(),
                             baseSpecNode->getMinOccurs(), baseSpecNode->getMaxOccurs())) {
        ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::PD_NSSubset1, fMemoryManager);
    }

    if (!isWildCardEltSubset(derivedSpecNode, baseSpecNode)) {
        ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::PD_NSSubset2, fMemoryManager);
    }
}

bool SchemaValidator::checkNSSubsetChoiceRoot(const ContentSpecNode* const derivedSpecNode,
                                    const ContentSpecNode* const baseSpecNode) {
    bool found = false;

    if (baseSpecNode->getType() == ContentSpecNode::Any_NS_Choice) {
        const ContentSpecNode* first = baseSpecNode->getFirst();
        const ContentSpecNode* second = baseSpecNode->getSecond();

        if (first) {
            found = checkNSSubsetChoiceRoot(derivedSpecNode, first);
            if (found) return true;
        }
        if (second) {
            found = checkNSSubsetChoiceRoot(derivedSpecNode, second);
            if (found) return true;
        }
    }
    else { // should be Any_NS
        found = checkNSSubsetChoice(derivedSpecNode, baseSpecNode);
    }

    return found;
}

bool SchemaValidator::checkNSSubsetChoice(const ContentSpecNode* const derivedSpecNode,
                                    const ContentSpecNode* const baseSpecNode) {

    // check Occurrence ranges
    if (!isOccurrenceRangeOK(derivedSpecNode->getMinOccurs(), derivedSpecNode->getMaxOccurs(),
                             baseSpecNode->getMinOccurs(), baseSpecNode->getMaxOccurs())) {
        return false;
    }

    if (!isWildCardEltSubset(derivedSpecNode, baseSpecNode)) {
        return false;
    }
    return true;
}

bool
SchemaValidator::isWildCardEltSubset(const ContentSpecNode* const derivedSpecNode,
                                     const ContentSpecNode* const baseSpecNode) {

    ContentSpecNode::NodeTypes baseType = baseSpecNode->getType();

    if ((baseType & 0x0f) == ContentSpecNode::Any) {
        return true;
    }

    ContentSpecNode::NodeTypes derivedType = derivedSpecNode->getType();
    unsigned int baseURI = baseSpecNode->getElement()->getURI();
    unsigned int derivedURI = derivedSpecNode->getElement()->getURI();

    // Below we assume that empty string has id 1.
    //
    if (((derivedType & 0x0f) == ContentSpecNode::Any_Other) &&
        ((baseType & 0x0f) == ContentSpecNode::Any_Other) &&
        (baseURI == derivedURI || baseURI == 1)) {
        return true;
    }

    if ((derivedType & 0x0f) == ContentSpecNode::Any_NS) {

        if (((baseType & 0x0f) == ContentSpecNode::Any_NS) &&
            baseURI == derivedURI) {
            return true;
        }

        if (((baseType & 0x0f) == ContentSpecNode::Any_Other) &&
            (derivedURI == 1 || baseURI != derivedURI)) {
            return true;
        }
    }

    return false;
}

void
SchemaValidator::checkNSRecurseCheckCardinality(SchemaGrammar* const currentGrammar,
                                                const ContentSpecNode* const derivedSpecNode,
                                                ValueVectorOf<ContentSpecNode*>* const derivedNodes,
                                                const int derivedScope,
                                                ContentSpecNode* const baseSpecNode,
                                                const bool toCheckOccurence) {

    // Implement total range check
    int derivedMin = derivedSpecNode->getMinTotalRange();
    int derivedMax = derivedSpecNode->getMaxTotalRange();

    // check Occurrence ranges
    if (toCheckOccurence &&
        !isOccurrenceRangeOK(derivedMin, derivedMax, baseSpecNode->getMinOccurs(),
                              baseSpecNode->getMaxOccurs())) {
        ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::PD_NSRecurseCheckCardinality1, fMemoryManager);
    }

    // Check that each member of the group is a valid restriction of the wildcard
    XMLSize_t nodesCount = derivedNodes->size();

    for (XMLSize_t i = 0; i < nodesCount; i++) {
        checkParticleDerivationOk(currentGrammar, derivedNodes->elementAt(i), derivedScope, baseSpecNode, -1, 0, false);
    }
}

void
SchemaValidator::checkRecurseUnordered(SchemaGrammar* const currentGrammar,
                                       const ContentSpecNode* const derivedSpecNode,
                                       ValueVectorOf<ContentSpecNode*>* const derivedNodes,
                                       const int derivedScope,
                                       ContentSpecNode* const baseSpecNode,
                                       ValueVectorOf<ContentSpecNode*>* const baseNodes,
                                       const int baseScope,
                                       const ComplexTypeInfo* const baseInfo) {

    // check Occurrence ranges
    if (!isOccurrenceRangeOK(derivedSpecNode->getMinOccurs(), derivedSpecNode->getMaxOccurs(),
                             baseSpecNode->getMinOccurs(), baseSpecNode->getMaxOccurs())) {
        ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::PD_Recurse1, fMemoryManager);
    }

    XMLExcepts::Codes  codeToThrow = XMLExcepts::NoError;
    XMLSize_t          derivedCount= derivedNodes->size();
    XMLSize_t          baseCount = baseNodes->size();
    bool*              foundIt = (bool*) fMemoryManager->allocate
    (
        baseCount * sizeof(bool)
    );//new bool[baseCount];
    ArrayJanitor<bool> janFoundIt(foundIt, fMemoryManager);

    for (XMLSize_t k=0; k < baseCount; k++) {
        foundIt[k] = false;
    }

    // check for mapping of children
    for (XMLSize_t i = 0; i < derivedCount; i++) {

        ContentSpecNode* derivedNode = derivedNodes->elementAt(i);
        bool matched = false;

        for (XMLSize_t j = 0; j < baseCount; j++) {

            try {

                checkParticleDerivationOk(currentGrammar, derivedNode, derivedScope,
                                          baseNodes->elementAt(j), baseScope, baseInfo);

                if (foundIt[j]) {
                    break;
                }

                foundIt[j] = true;
                matched = true;
                break;
            }
            catch (const XMLException&) {
            }
        }

        // didn't find a match.
        if (!matched) {

	        codeToThrow = XMLExcepts::PD_RecurseUnordered;
            break;
        }
    }

    // For all unmapped particles in base, check to see it it's emptiable or not
    if (codeToThrow == XMLExcepts::NoError) {
        for (XMLSize_t j=0; j < baseCount; j++) {
            if (!foundIt[j] && baseNodes->elementAt(j)->getMinTotalRange()) {

	            codeToThrow = XMLExcepts::PD_RecurseUnordered;
                break;
            }
        }
    }

    if (codeToThrow != XMLExcepts::NoError) {
        ThrowXMLwithMemMgr(RuntimeException, codeToThrow, fMemoryManager);
    }
}

void
SchemaValidator::checkMapAndSum(SchemaGrammar* const currentGrammar,
                                const ContentSpecNode* const derivedSpecNode,
                                ValueVectorOf<ContentSpecNode*>* const derivedNodes,
                                const int derivedScope,
                                ContentSpecNode* const baseSpecNode,
                                ValueVectorOf<ContentSpecNode*>* const baseNodes,
                                const int baseScope,
                                const ComplexTypeInfo* const baseInfo) {

    // check Occurrence ranges
    XMLSize_t derivedCount = derivedNodes->size();
    XMLSize_t baseCount = baseNodes->size();
    int derivedMin = derivedSpecNode->getMinOccurs() * (unsigned int)derivedCount;
    int derivedMax = derivedSpecNode->getMaxOccurs();

    if (derivedMax != SchemaSymbols::XSD_UNBOUNDED) {
        derivedMax *= (unsigned int)derivedCount;
    }

    if (!isOccurrenceRangeOK(derivedMin, derivedMax, baseSpecNode->getMinOccurs(),
                             baseSpecNode->getMaxOccurs())) {
        ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::PD_Recurse1, fMemoryManager);
    }

    // check for mapping of children
    for (XMLSize_t i = 0; i < derivedCount; i++) {

        ContentSpecNode* derivedNode = derivedNodes->elementAt(i);
        bool matched = false;

        for (XMLSize_t j = 0; j < baseCount && !matched; j++) {

            try {

                checkParticleDerivationOk(currentGrammar, derivedNode, derivedScope,
                                          baseNodes->elementAt(j), baseScope, baseInfo);
                matched = true;
            }
            catch (const XMLException&) {
            }
        }

        // didn't find a match.
        if (!matched) {
	        ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::PD_MapAndSum, fMemoryManager);
        }
    }

}

XERCES_CPP_NAMESPACE_END
