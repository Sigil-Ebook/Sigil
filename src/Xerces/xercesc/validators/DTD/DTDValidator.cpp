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
 * $Id: DTDValidator.cpp 729944 2008-12-29 17:03:32Z amassari $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/Janitor.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/internal/ReaderMgr.hpp>
#include <xercesc/internal/XMLScanner.hpp>
#include <xercesc/validators/DTD/DTDValidator.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  DTDValidator: Constructors and Destructor
// ---------------------------------------------------------------------------
DTDValidator::DTDValidator(XMLErrorReporter* const errReporter) :

    XMLValidator(errReporter)
    , fDTDGrammar(0)
{
    reset();
}

DTDValidator::~DTDValidator()
{
}


// ---------------------------------------------------------------------------
//  DTDValidator: Implementation of the XMLValidator interface
// ---------------------------------------------------------------------------
bool DTDValidator::checkContent(XMLElementDecl* const elemDecl
                              , QName** const         children
                              , XMLSize_t             childCount
                              , XMLSize_t*         indexFailingChild)
{
    //
    //  Look up the element id in our element decl pool. This will get us
    //  the element decl in our own way of looking at them.
    //
    if (!elemDecl)
        ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Val_InvalidElemId, getScanner()->getMemoryManager());

    //
    //  Get the content spec type of this element. This will tell us what
    //  to do to validate it.
    //
    const DTDElementDecl::ModelTypes modelType = ((DTDElementDecl*) elemDecl)->getModelType();

    if (modelType == DTDElementDecl::Empty)
    {
        //
        //  We can do this one here. It cannot have any children. If it does
        //  we return 0 as the index of the first bad child.
        //
        if (childCount)
        {
            *indexFailingChild=0;
            return false;
        }
    }
     else if (modelType == DTDElementDecl::Any)
    {
        // We pass no judgement on this one, anything goes
    }
     else if ((modelType == DTDElementDecl::Mixed_Simple)
          ||  (modelType == DTDElementDecl::Children))
    {
        // Get the element's content model or fault it in
        const XMLContentModel* elemCM = elemDecl->getContentModel();

        // Ask it to validate and return its return
        return elemCM->validateContent(children, childCount, getScanner()->getEmptyNamespaceId(), indexFailingChild, getScanner()->getMemoryManager());
    }
     else
    {
        ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::CM_UnknownCMType, getScanner()->getMemoryManager());
    }

    // Went ok, so return success
    return true;
}


void DTDValidator::faultInAttr(XMLAttr& toFill, const XMLAttDef& attDef) const
{
    toFill.set(0, attDef.getFullName(), attDef.getValue(), attDef.getType());
}

void DTDValidator::reset()
{
}


bool DTDValidator::requiresNamespaces() const
{
    // Namespaces are not supported for DTDs
    return false;
}


void
DTDValidator::validateAttrValue(const   XMLAttDef*      attDef
                                , const XMLCh* const    attrValue
                                , bool                  preValidation
                                , const XMLElementDecl*)
{
    //
    //  Get quick refs to lost of of the stuff in the passed objects in
    //  order to simplify the code below, which will reference them very
    //  often.
    //
    const XMLAttDef::AttTypes       type = attDef->getType();
    const XMLAttDef::DefAttTypes    defType = attDef->getDefaultType();
    const XMLCh* const              valueText = attDef->getValue();
    const XMLCh* const              fullName = attDef->getFullName();
    const XMLCh* const              enumList = attDef->getEnumeration();

    //
    //  If the default type is fixed, then make sure the passed value maps
    //  to the fixed value.
    //  If during preContentValidation, the value we are validating is the fixed value itself
    //  so no need to compare.
    //  Only need to do this for regular attribute value validation
    //
    if (defType == XMLAttDef::Fixed && !preValidation)
    {
        if (!XMLString::equals(attrValue, valueText))
            emitError(XMLValid::NotSameAsFixedValue, fullName, attrValue, valueText);
    }

    //
    //  If its a CDATA attribute, then we are done with any DTD level
    //  validation else do the rest.
    //
    if (type == XMLAttDef::CData)
        return;



    // An empty string cannot be valid for any of the other types
    if (!attrValue[0])
    {
        emitError(XMLValid::InvalidEmptyAttValue, fullName);
        return;
    }

    // See whether we are doing multiple values or not
    const bool multipleValues =
    (
        (type == XMLAttDef::IDRefs)
        || (type == XMLAttDef::Entities)
        || (type == XMLAttDef::NmTokens)
        || (type == XMLAttDef::Notation)
        || (type == XMLAttDef::Enumeration)
    );

    // And whether we must check for a first name char
    const bool firstNameChar =
    (
        (type == XMLAttDef::ID)
        || (type == XMLAttDef::IDRef)
        || (type == XMLAttDef::IDRefs)
        || (type == XMLAttDef::Entity)
        || (type == XMLAttDef::Entities)
        || (type == XMLAttDef::Notation)
    );

    // Whether it requires ref checking stuff
    const bool isARefType
    (
        (type == XMLAttDef::ID)
        || (type == XMLAttDef::IDRef)
        || (type == XMLAttDef::IDRefs)
    );

    // Some trigger flags to avoid issuing redundant errors and whatnot    
    bool alreadyCapped = false;

    //
    //  Make a copy of the text that we can mangle and get a pointer we can
    //  move through the value
    //

    // Use a stack-based buffer, when possible...
    XMLCh   tempBuffer[100];

    XMLCh* pszTmpVal = 0;

    ArrayJanitor<XMLCh> janTmpVal(0);

    if (XMLString::stringLen(attrValue) < sizeof(tempBuffer) / sizeof(tempBuffer[0]))
    {
        XMLString::copyString(tempBuffer, attrValue);
        pszTmpVal = tempBuffer;
    }
    else
    {
        janTmpVal.reset(XMLString::replicate(attrValue, getScanner()->getMemoryManager()), getScanner()->getMemoryManager());
        pszTmpVal = janTmpVal.get();
    }

    XMLCh* valPtr = pszTmpVal;

    bool doNamespace = getScanner()->getDoNamespaces();

    while (true)
    {
        //
        //  Make sure the first character is a valid first name char, i.e.
        //  if its a Name value. For NmToken values we don't treat the first
        //  char any differently.
        //
        if (firstNameChar)
        {
            // If its not, emit and error but try to keep going
            if (!getReaderMgr()->getCurrentReader()->isFirstNameChar(*valPtr))
                emitError(XMLValid::AttrValNotName, valPtr, fullName);
            valPtr++;
        }

        // Make sure all the remaining chars are valid name chars
        while (*valPtr)
        {
            //
            //  If we hit a whitespace, its either a break between two
            //  or more values, or an error if we have a single value.
            //
            //
            //   XML1.0-3rd
            //
            //   [6]   Names   ::=   Name (#x20 Name)*
            //   [8]   Nmtokens   ::=   Nmtoken (#x20 Nmtoken)*
            //
            //   only and only ONE #x20 is allowed to be the delimiter
            //
            if (*valPtr==chSpace)
            {
                if (!multipleValues)
                {
                    emitError(XMLValid::NoMultipleValues, fullName);
                    return;
                }

                break;
            }

            // Now this attribute can be of type
            //     ID, IDREF, IDREFS, ENTITY, ENTITIES, NOTATION, NMTOKEN, NMTOKENS, ENUMERATION
            //  All these must be valid XMLName
            // If namespace is enabled, colon is not allowed in the first 6

            if (doNamespace && *valPtr == chColon && firstNameChar)
                emitError(XMLValid::ColonNotValidWithNS);

            if (!getReaderMgr()->getCurrentReader()->isNameChar(*valPtr))
            {
                emitError(XMLValid::AttrValNotName, valPtr, fullName);
                return;
            }
            valPtr++;
        }

        //
        //  Cap it off at the current non-name char. If already capped,
        //  then remember this.
        //
        if (!(*valPtr))
            alreadyCapped = true;
        *valPtr = 0;

        //
        //  If this type of attribute requires that we track reference
        //  stuff, then handle that.
        //
        if (isARefType)
        {
            if ((type == XMLAttDef::ID)
            ||  (type == XMLAttDef::IDRef)
            ||  (type == XMLAttDef::IDRefs))
            {
                XMLRefInfo* find = getScanner()->getIDRefList()->get(pszTmpVal);
                if (find)
                {
                    if (find->getDeclared() && (type == XMLAttDef::ID))
                        emitError(XMLValid::ReusedIDValue, pszTmpVal);
                }
                 else
                {
                    find = new (getScanner()->getMemoryManager()) XMLRefInfo
                    (
                        pszTmpVal
                        , false
                        , false
                        , getScanner()->getMemoryManager()
                    );
                    getScanner()->getIDRefList()->put((void*)find->getRefName(), find);
                }

                //
                //  Mark it declared or used, which might be redundant in some cases
                //  but not worth checking
                //
                if (type == XMLAttDef::ID)
                    find->setDeclared(true);
                else {
                    if (!preValidation) {
                        find->setUsed(true);
                    }
                }
            }
        }
         else if (!preValidation && ((type == XMLAttDef::Entity) || (type == XMLAttDef::Entities)))
        {
            //
            //  If its refering to a entity, then look up the name in the
            //  general entity pool. If not there, then its an error. If its
            //  not an external unparsed entity, then its an error.
            //
            //  In case of pre-validation, the above errors should be ignored.
            //
            const XMLEntityDecl* decl = fDTDGrammar->getEntityDecl(pszTmpVal);
            if (decl)
            {
                if (!decl->isUnparsed())
                    emitError(XMLValid::BadEntityRefAttr, pszTmpVal, fullName);
            }
             else
            {
                emitError
                (
                    XMLValid::UnknownEntityRefAttr
                    , fullName
                    , pszTmpVal
                );
            }
        }
         else if ((type == XMLAttDef::Notation) || (type == XMLAttDef::Enumeration))
        {
            //
            //  Make sure that this value maps to one of the enumeration or
            //  notation values in the enumList parameter. We don't have to
            //  look it up in the notation pool (if a notation) because we
            //  will look up the enumerated values themselves. If they are in
            //  the notation pool (after the DTD is parsed), then obviously
            //  this value will be legal since it matches one of them.
            //
            if (!XMLString::isInList(pszTmpVal, enumList))
                emitError(XMLValid::DoesNotMatchEnumList, pszTmpVal, fullName);
        }

        // If not doing multiple values, then we are done
        if (!multipleValues)
            break;

        //
        //  If we are at the end, then break out now, else move up to the
        //  next char and update the base pointer.
        //
        if (alreadyCapped)
            break;

        valPtr++;
        pszTmpVal = valPtr;
    }

}

void DTDValidator::preContentValidation(bool
#if defined(XERCES_DEBUG)
										reuseGrammar
#endif
                                       ,bool validateDefAttr)
{
    //
    //  Lets enumerate all of the elements in the element decl pool
    //  and put out an error for any that did not get declared.
    //  We also check all of the attributes as well.
    //
    NameIdPoolEnumerator<DTDElementDecl> elemEnum = fDTDGrammar->getElemEnumerator();
    fDTDGrammar->setValidated(true);
    while (elemEnum.hasMoreElements())
    {
        const DTDElementDecl& curElem = elemEnum.nextElement();
        const DTDElementDecl::CreateReasons reason = curElem.getCreateReason();

        //
        //  See if this element decl was ever marked as declared. If
        //  not, then put out an error. In some cases its just
        //  a warning, such as being referenced in a content model.
        //
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
                // It's ok that the root element is not declared in the DTD
                /*
                emitError
                (
                    XMLValid::UndeclaredElemInDocType
                    , curElem.getFullName()
                );*/
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
                #if defined(XERCES_DEBUG)
                  if(reuseGrammar && reason == XMLElementDecl::JustFaultIn){
                  }
                  else
                      ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::DTD_UnknownCreateReason, getScanner()->getMemoryManager());
                #endif
            }
        }

        //
        //  Check all of the attributes of the current element.
        //  We check for:
        //
        //  1) Multiple ID attributes
        //  2) That all of the default values of attributes are
        //      valid for their type.
        //  3) That for any notation types, that their lists
        //      of possible values refer to declared notations.
        //
        //  4) XML1.0(3rd edition)
        //
        //     Validity constraint: One Notation Per Element Type
        //     An element type MUST NOT have more than one NOTATION attribute specified.
        //
        //     Validity constraint: No Notation on Empty Element
        //     For compatibility, an attribute of type NOTATION MUST NOT be declared on an element declared EMPTY.
        //
        //     Validity constraint: No Duplicate Tokens
        //     The notation names in a single NotationType attribute declaration, as well as 
        //     the NmTokens in a single Enumeration attribute declaration, MUST all be distinct.
        //

        XMLAttDefList& attDefList = curElem.getAttDefList();
        bool seenId = false;
        bool seenNOTATION = false;
        bool elemEmpty = (curElem.getModelType() == DTDElementDecl::Empty);

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
             else if (curAttDef.getType() == XMLAttDef::Notation)
            {
                if (seenNOTATION)
                {
                    emitError
                    (
                        XMLValid::ElemOneNotationAttr
                      , curElem.getFullName()
                    );

                    break;
                }

                seenNOTATION = true;

                // no notation attribute on empty element
                if (elemEmpty)
                {
                    emitError
                   (
                      XMLValid::EmptyElemNotationAttr
                    , curElem.getFullName()
                    , curAttDef.getFullName()
                    );

                    break;
                }

                //go through enumeration list to check
                // distinct 
                // notation declaration
                if (curAttDef.getEnumeration())
                {
                    checkTokenList(curAttDef, true);
                }
             }
             else if (curAttDef.getType() == XMLAttDef::Enumeration )
             {
                //go through enumeration list to check
                // distinct only
                if (curAttDef.getEnumeration())
                {
                    checkTokenList(curAttDef, false);
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

    //
    //  And enumerate all of the general entities. If any of them
    //  reference a notation, then make sure the notation exists.
    //
    NameIdPoolEnumerator<DTDEntityDecl> entEnum = fDTDGrammar->getEntityEnumerator();
    while (entEnum.hasMoreElements())
    {
        const DTDEntityDecl& curEntity = entEnum.nextElement();

        if (!curEntity.getNotationName())
            continue;

        // It has a notation name, so look it up
        if (!fDTDGrammar->getNotationDecl(curEntity.getNotationName()))
        {
            emitError
            (
                XMLValid::NotationNotDeclared
                , curEntity.getNotationName()
            );
        }
    }
}

void DTDValidator::postParseValidation()
{
    //
    //  At this time, there is nothing to do here. The scanner itself handles
    //  ID/IDREF validation, since that is the same no matter what kind of
    //  validator.
    //
}

//
//  We need to verify that all of its possible values
//  (in the enum list) 
//   is distinct and
//   refer to valid notations if toValidateNotation is set on
//
void DTDValidator::checkTokenList(const XMLAttDef&  curAttDef
                                ,       bool        toValidateNotation)
{

    XMLCh* list = XMLString::replicate(curAttDef.getEnumeration(), getScanner()->getMemoryManager());
    ArrayJanitor<XMLCh> janList(list, getScanner()->getMemoryManager());

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
            *listPtr++ = chNull;

        //distinction check
        //there should be no same token found in the remaining list
        if (XMLString::isInList(lastPtr, listPtr))
        {
            emitError
                (
                XMLValid::AttrDupToken
                , curAttDef.getFullName()
                , lastPtr
                );
        }

        if (toValidateNotation && !fDTDGrammar->getNotationDecl(lastPtr))
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
        lastPtr = listPtr;
    }
}

XERCES_CPP_NAMESPACE_END
