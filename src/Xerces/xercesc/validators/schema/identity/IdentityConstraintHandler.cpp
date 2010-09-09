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
 * $Id: IdentityConstraintHandler.cpp 803869 2009-08-13 12:56:21Z amassari $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include "IdentityConstraintHandler.hpp"

#include <xercesc/validators/schema/SchemaElementDecl.hpp>

#include <xercesc/validators/schema/identity/FieldActivator.hpp>
#include <xercesc/validators/schema/identity/ValueStore.hpp>
#include <xercesc/validators/schema/identity/IC_Selector.hpp>

#include <xercesc/util/OutOfMemoryException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

typedef JanitorMemFunCall<IdentityConstraintHandler>    CleanupType;

// ---------------------------------------------------------------------------
//  IdentityConstraintHandler: Constructors and Destructor
// ---------------------------------------------------------------------------
IdentityConstraintHandler::IdentityConstraintHandler(XMLScanner*    const scanner
                     , MemoryManager* const manager)
: fScanner(scanner)
, fMemoryManager(manager)
, fMatcherStack(0)
, fValueStoreCache(0)
, fFieldActivator(0)
{
    CleanupType cleanup(this, &IdentityConstraintHandler::cleanUp);

    try {

        fMatcherStack    = new (fMemoryManager) XPathMatcherStack(fMemoryManager);
        fValueStoreCache = new (fMemoryManager) ValueStoreCache(fMemoryManager);
        fFieldActivator  = new (fMemoryManager) FieldActivator(fValueStoreCache, fMatcherStack, fMemoryManager);

        fValueStoreCache->setScanner(scanner);
    }
    catch(const OutOfMemoryException&)
    {
        cleanup.release();

        throw;
    }

    cleanup.release();
}

IdentityConstraintHandler::~IdentityConstraintHandler()
{
    cleanUp();
}

// ---------------------------------------------------------------------------
//  IdentityConstraintHandler:  methods
// ---------------------------------------------------------------------------
void IdentityConstraintHandler::deactivateContext(      SchemaElementDecl* const elem
                                                , const XMLCh*             const content
                                                , ValidationContext*       validationContext /*=0*/
                                                , DatatypeValidator*       actualValidator /*=0*/)
{

    XMLSize_t oldCount = fMatcherStack->getMatcherCount();

    if (oldCount || elem->getIdentityConstraintCount()) 
    {

        for (XMLSize_t i = oldCount; i > 0; i--) 
        {
            XPathMatcher* matcher = fMatcherStack->getMatcherAt(i-1);
            matcher->endElement(*(elem), content, validationContext, actualValidator);
        }

        if (fMatcherStack->size() > 0) 
        {
            fMatcherStack->popContext();
        }

        // handle everything *but* keyref's.
        XMLSize_t newCount = fMatcherStack->getMatcherCount();

        for (XMLSize_t j = oldCount; j > newCount; j--) 
        {
            XPathMatcher* matcher = fMatcherStack->getMatcherAt(j-1);
            IdentityConstraint* ic = matcher->getIdentityConstraint();

            if (ic  && (ic->getType() != IdentityConstraint::ICType_KEYREF))
                fValueStoreCache->transplant(ic, matcher->getInitialDepth());
        }

        // now handle keyref's...
        for (XMLSize_t k = oldCount; k > newCount; k--)
        {
            XPathMatcher* matcher = fMatcherStack->getMatcherAt(k-1);
            IdentityConstraint* ic = matcher->getIdentityConstraint();

            if (ic && (ic->getType() == IdentityConstraint::ICType_KEYREF)) 
            {
                ValueStore* values = fValueStoreCache->getValueStoreFor(ic, matcher->getInitialDepth());

                if (values) { // nothing to do if nothing matched!
                    values->endDocumentFragment(fValueStoreCache);
                }
            }
        }

        fValueStoreCache->endElement();

    }
}

void IdentityConstraintHandler::activateIdentityConstraint
                     (      
                             SchemaElementDecl* const     elem
                     ,       int                          elemDepth
                     , const unsigned int                 uriId
                     , const XMLCh*                 const elemPrefix
                     , const RefVectorOf<XMLAttr>&        attrList
                     , const XMLSize_t                    attrCount
                     , ValidationContext*                 validationContext /*=0*/)
{

    XMLSize_t count = elem->getIdentityConstraintCount();

    if (count || fMatcherStack->getMatcherCount()) 
    {

        fValueStoreCache->startElement();
        fMatcherStack->pushContext();
        fValueStoreCache->initValueStoresFor( elem, elemDepth);

        for (XMLSize_t i = 0; i < count; i++) 
        {
            activateSelectorFor(elem->getIdentityConstraintAt(i), elemDepth);
        }

        // call all active identity constraints
        count = fMatcherStack->getMatcherCount();

        for (XMLSize_t j = 0; j < count; j++) 
        {
            XPathMatcher* matcher = fMatcherStack->getMatcherAt(j);
            matcher->startElement(*elem, uriId, elemPrefix, attrList, attrCount, validationContext);
        }
    }
}

void IdentityConstraintHandler::activateSelectorFor(      IdentityConstraint* const ic
                                   , const int                       initialDepth) 
{

    IC_Selector* selector = ic->getSelector();

    if (!selector)
        return;

    XPathMatcher* matcher = selector->createMatcher(fFieldActivator, initialDepth, fMemoryManager);

    fMatcherStack->addMatcher(matcher);
    matcher->startDocumentFragment();
}

// ---------------------------------------------------------------------------
//  IdentityConstraintHandler:  Getter methods
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  IdentityConstraintHandler: cleanUp methods
// ---------------------------------------------------------------------------
void IdentityConstraintHandler::cleanUp() 
{
    if (fMatcherStack)
        delete fMatcherStack;

    if (fValueStoreCache)
        delete fValueStoreCache;

    if (fFieldActivator)
        delete fFieldActivator;

}

void IdentityConstraintHandler::reset()
{
    fValueStoreCache->startDocument();
    fMatcherStack->clear();
}

XERCES_CPP_NAMESPACE_END

/**
  * End of file IdentityConstraintHandler.cpp
  */

