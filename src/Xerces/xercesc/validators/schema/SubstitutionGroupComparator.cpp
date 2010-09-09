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
 * $Id: SubstitutionGroupComparator.cpp 794273 2009-07-15 14:13:07Z amassari $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/framework/XMLGrammarPool.hpp>
#include <xercesc/framework/XMLSchemaDescription.hpp>
#include <xercesc/framework/psvi/XSAnnotation.hpp>
#include <xercesc/validators/schema/SubstitutionGroupComparator.hpp>
#include <xercesc/validators/common/Grammar.hpp>
#include <xercesc/validators/schema/SchemaGrammar.hpp>
#include <xercesc/validators/schema/ComplexTypeInfo.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>

XERCES_CPP_NAMESPACE_BEGIN

bool SubstitutionGroupComparator::isEquivalentTo(const QName* const anElement
                                               , const QName* const exemplar)
{
    if (!anElement && !exemplar)
        return true;

    if ((!anElement && exemplar) || (anElement && !exemplar))
        return false;


    if (XMLString::equals(anElement->getLocalPart(), exemplar->getLocalPart()) &&
        (anElement->getURI() == exemplar->getURI()))
        return true; // they're the same!

    if (!fGrammarResolver || !fStringPool )
    {
        ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::SubGrpComparator_NGR, anElement->getMemoryManager());
    }

    unsigned int uriId = anElement->getURI();
    if (uriId == XMLContentModel::gEOCFakeId ||
        uriId == XMLContentModel::gEpsilonFakeId ||
        uriId == XMLElementDecl::fgPCDataElemId ||
        uriId == XMLElementDecl::fgInvalidElemId)
        return false;

    const XMLCh* uri = fStringPool->getValueForId(uriId);
    const XMLCh* localpart = anElement->getLocalPart();

    // In addition to simply trying to find a chain between anElement and exemplar,
    // we need to make sure that no steps in the chain are blocked.
    // That is, at every step, we need to make sure that the element
    // being substituted for will permit being substituted
    // for, and whether the type of the element will permit derivations in
    // instance documents of this sort.

    if (!uri)
        return false;

    SchemaGrammar *sGrammar = (SchemaGrammar*) fGrammarResolver->getGrammar(uri);
    if (!sGrammar || sGrammar->getGrammarType() == Grammar::DTDGrammarType)
        return false;

    SchemaElementDecl* anElementDecl = (SchemaElementDecl*) sGrammar->getElemDecl(uriId, localpart, 0, Grammar::TOP_LEVEL_SCOPE);
    if (!anElementDecl)
        return false;

    SchemaElementDecl* pElemDecl = anElementDecl->getSubstitutionGroupElem();
    bool foundIt = false;

    while (pElemDecl) //(substitutionGroupFullName)
    {
        if (XMLString::equals(pElemDecl->getBaseName(), exemplar->getLocalPart()) &&
            (pElemDecl->getURI() == exemplar->getURI()))
        {
            // time to check for block value on element
            if((pElemDecl->getBlockSet() & SchemaSymbols::XSD_SUBSTITUTION) != 0)
                return false;

            foundIt = true;
            break;
        }

        pElemDecl = pElemDecl->getSubstitutionGroupElem();
    }//while

    if (!foundIt)
        return false;

    // this will contain anElement's complexType information.
    ComplexTypeInfo *aComplexType = anElementDecl->getComplexTypeInfo();
    int exemplarBlockSet = pElemDecl->getBlockSet();

    if(!aComplexType)
    {
        // check on simpleType case
        DatatypeValidator *anElementDV = anElementDecl->getDatatypeValidator();
        DatatypeValidator *exemplarDV = pElemDecl->getDatatypeValidator();

        return((anElementDV == 0) ||
            ((anElementDV == exemplarDV) ||
            ((exemplarBlockSet & SchemaSymbols::XSD_RESTRICTION) == 0)));
    }

    // 2.3 The set of all {derivation method}s involved in the derivation of D's {type definition} from C's {type definition} does not intersect with the union of the blocking constraint, C's {prohibited substitutions} (if C is complex, otherwise the empty set) and the {prohibited substitutions} (respectively the empty set) of any intermediate {type definition}s in the derivation of D's {type definition} from C's {type definition}.
    // prepare the combination of {derivation method} and
    // {disallowed substitution}
    int devMethod = 0;
    int blockConstraint = exemplarBlockSet;

    ComplexTypeInfo *exemplarComplexType = pElemDecl->getComplexTypeInfo();
    ComplexTypeInfo *tempType = aComplexType;;

    while (tempType != 0 &&
        tempType != exemplarComplexType)
    {
        devMethod |= tempType->getDerivedBy();
        tempType = tempType->getBaseComplexTypeInfo();
        if (tempType) {
            blockConstraint |= tempType->getBlockSet();
        }
    }
    if (tempType != exemplarComplexType) {
        return false;
    }
    if ((devMethod & blockConstraint) != 0) {
        return false;
    }

    return true;
}


bool SubstitutionGroupComparator::isAllowedByWildcard(SchemaGrammar* const pGrammar,
                                                      QName* const element,
                                                      unsigned int wuri, bool wother)
{
    // whether the uri is allowed directly by the wildcard
    unsigned int uriId = element->getURI();

    // Here we assume that empty string has id 1.
    //
    if ((!wother && uriId == wuri) ||
        (wother &&
         uriId != 1 &&
         uriId != wuri &&
         uriId != XMLContentModel::gEOCFakeId &&
         uriId != XMLContentModel::gEpsilonFakeId &&
         uriId != XMLElementDecl::fgPCDataElemId &&
         uriId != XMLElementDecl::fgInvalidElemId))
    {
        return true;
    }

    // get all elements that can substitute the current element
    RefHash2KeysTableOf<ElemVector>* theValidSubstitutionGroups = pGrammar->getValidSubstitutionGroups();

    if (!theValidSubstitutionGroups)
        return false;

    ValueVectorOf<SchemaElementDecl*>* subsElements = theValidSubstitutionGroups->get(element->getLocalPart(), uriId);

    if (!subsElements)
        return false;

    // then check whether there exists one element that is allowed by the wildcard
    XMLSize_t size = subsElements->size();

    for (XMLSize_t i = 0; i < size; i++)
    {
        unsigned int subUriId = subsElements->elementAt(i)->getElementName()->getURI();

        // Here we assume that empty string has id 1.
        //
        if ((!wother && subUriId == wuri) ||
            (wother &&
             subUriId != 1 &&
             subUriId != wuri &&
             subUriId != XMLContentModel::gEOCFakeId &&
             subUriId != XMLContentModel::gEpsilonFakeId &&
             subUriId != XMLElementDecl::fgPCDataElemId &&
             subUriId != XMLElementDecl::fgInvalidElemId))
        {
            return true;
        }
    }
    return false;
}

XERCES_CPP_NAMESPACE_END

/**
  * End of file SubstitutionGroupComparator.cpp
  */
