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
 * $Id: IC_Selector.cpp 803869 2009-08-13 12:56:21Z amassari $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/framework/XMLAttr.hpp>
#include <xercesc/validators/schema/identity/IC_Selector.hpp>
#include <xercesc/validators/schema/identity/XercesXPath.hpp>
#include <xercesc/validators/schema/identity/IdentityConstraint.hpp>
#include <xercesc/validators/schema/identity/FieldActivator.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  SelectorMatcher: Constructors and Destructor
// ---------------------------------------------------------------------------
SelectorMatcher::SelectorMatcher(XercesXPath* const xpath,
                                 IC_Selector* const selector,
                                 FieldActivator* const fieldActivator,
                                 const int initialDepth,
                                 MemoryManager* const manager)
    : XPathMatcher(xpath, selector->getIdentityConstraint(), manager)
    , fInitialDepth(initialDepth)
    , fElementDepth(0)
    , fMatchedDepth(-1)
    , fSelector(selector)
    , fFieldActivator(fieldActivator)
{
}

// ---------------------------------------------------------------------------
//  FieldMatcher: XMLDocumentHandler methods
// ---------------------------------------------------------------------------
void SelectorMatcher::startDocumentFragment() {

    XPathMatcher::startDocumentFragment();
    fElementDepth = 0;
    fMatchedDepth = -1;
}

void SelectorMatcher::startElement(const XMLElementDecl& elemDecl,
                                   const unsigned int urlId,
                                   const XMLCh* const elemPrefix,
                                   const RefVectorOf<XMLAttr>& attrList,
                                   const XMLSize_t attrCount,
                                   ValidationContext* validationContext /*=0*/) 
{

    XPathMatcher::startElement(elemDecl, urlId, elemPrefix, attrList, attrCount, validationContext);
    fElementDepth++;

    // activate the fields, if selector is matched
    unsigned char matched = isMatched();
    if ((fMatchedDepth == -1 && ((matched & XP_MATCHED) == XP_MATCHED))
        || ((matched & XP_MATCHED_D) == XP_MATCHED_D)) {

        IdentityConstraint* ic = fSelector->getIdentityConstraint();
        XMLSize_t count = ic->getFieldCount();

        fMatchedDepth = fElementDepth;
        fFieldActivator->startValueScopeFor(ic, fInitialDepth);

        for (XMLSize_t i = 0; i < count; i++) {

            XPathMatcher* matcher = fFieldActivator->activateField(ic->getFieldAt(i), fInitialDepth);
            matcher->startElement(elemDecl, urlId, elemPrefix, attrList, attrCount, validationContext);
        }
    }
}

void SelectorMatcher::endElement(const XMLElementDecl& elemDecl,
                                 const XMLCh* const elemContent, 
                                 ValidationContext* validationContext /*=0*/,
                                 DatatypeValidator* actualValidator /*=0*/) 
{

    XPathMatcher::endElement(elemDecl, elemContent, validationContext, actualValidator);

    if (fElementDepth-- == fMatchedDepth) {

        fMatchedDepth = -1;
        fFieldActivator->endValueScopeFor(fSelector->getIdentityConstraint(), fInitialDepth);
    }
}

// ---------------------------------------------------------------------------
//  IC_Selector: Constructors and Destructor
// ---------------------------------------------------------------------------
IC_Selector::IC_Selector(XercesXPath* const xpath,
                         IdentityConstraint* const identityConstraint)
    : fXPath(xpath)
    , fIdentityConstraint(identityConstraint)
{
}


IC_Selector::~IC_Selector()
{
    delete fXPath;
}

// ---------------------------------------------------------------------------
//  IC_Selector: operators
// ---------------------------------------------------------------------------
bool IC_Selector::operator ==(const IC_Selector& other) const {

    return (*fXPath == *(other.fXPath));
}

bool IC_Selector::operator !=(const IC_Selector& other) const {

    return !operator==(other);
}

// ---------------------------------------------------------------------------
//  IC_Selector: Factory methods
// ---------------------------------------------------------------------------
XPathMatcher* IC_Selector::createMatcher(FieldActivator* const fieldActivator,
                                         const int initialDepth,
                                         MemoryManager* const manager) {

    return new (manager) SelectorMatcher(fXPath, this, fieldActivator, initialDepth, manager);
}

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_TOCREATE(IC_Selector)

void IC_Selector::serialize(XSerializeEngine& serEng)
{
    if (serEng.isStoring())
    {
        serEng<<fXPath;
        
        IdentityConstraint::storeIC(serEng, fIdentityConstraint);
    }
    else
    {
        serEng>>fXPath;

        fIdentityConstraint = IdentityConstraint::loadIC(serEng);
    }

}

IC_Selector::IC_Selector(MemoryManager* const )
:fXPath(0)
,fIdentityConstraint(0)
{
}

XERCES_CPP_NAMESPACE_END

/**
  * End of file IC_Selector.cpp
  */

