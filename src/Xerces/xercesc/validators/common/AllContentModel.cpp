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
 * $Id: AllContentModel.cpp 676911 2008-07-15 13:27:32Z amassari $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/Janitor.hpp>
#include <xercesc/util/RuntimeException.hpp>
#include <xercesc/framework/XMLElementDecl.hpp>
#include <xercesc/framework/XMLValidator.hpp>
#include <xercesc/validators/common/ContentSpecNode.hpp>
#include <xercesc/validators/common/AllContentModel.hpp>
#include <xercesc/validators/schema/SubstitutionGroupComparator.hpp>
#include <xercesc/validators/schema/XercesElementWildcard.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  AllContentModel: Constructors and Destructor
// ---------------------------------------------------------------------------
AllContentModel::AllContentModel( ContentSpecNode* const parentContentSpec
                                , const bool             isMixed
                                , MemoryManager* const   manager) :
   fMemoryManager(manager)
 , fCount(0)
 , fChildren(0)
 , fChildOptional(0)
 , fNumRequired(0)
 , fIsMixed(isMixed)
 , fHasOptionalContent(false)
{
    //
    //  Create a vector of unsigned ints that will be filled in with the
    //  ids of the child nodes. It will be expanded as needed but we give
    //  it an initial capacity of 64 which should be more than enough for
    //  99% of the scenarios.
    //

    ValueVectorOf<QName*> children(64, fMemoryManager);
    ValueVectorOf<bool> childOptional(64, fMemoryManager);

    //
    //  Get the parent element's content spec. This is the head of the tree
    //  of nodes that describes the content model. We will iterate this
    //  tree.
    //
    ContentSpecNode* curNode = parentContentSpec;
    if (!curNode)
        ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::CM_NoParentCSN, fMemoryManager);

    // And now call the private recursive method that iterates the tree
    if (curNode->getType() == ContentSpecNode::All
        && curNode->getMinOccurs() == 0) {
        fHasOptionalContent = true;
    }
    buildChildList(curNode, children, childOptional);

    //
    //  And now we know how many elements we need in our member list. So
    //  fill them in.
    //
    fCount = children.size();
    fChildren = (QName**) fMemoryManager->allocate(fCount * sizeof(QName*)); //new QName*[fCount];
    fChildOptional = (bool*) fMemoryManager->allocate(fCount * sizeof(bool)); //new bool[fCount];
    for (unsigned int index = 0; index < fCount; index++) {
        fChildren[index] = new (fMemoryManager) QName(*(children.elementAt(index)));
        fChildOptional[index] = childOptional.elementAt(index);
    }
}

AllContentModel::~AllContentModel()
{
    for (XMLSize_t index = 0; index < fCount; index++)
        delete fChildren[index];
    fMemoryManager->deallocate(fChildren); //delete [] fChildren;
    fMemoryManager->deallocate(fChildOptional); //delete [] fChildOptional;
}

// ---------------------------------------------------------------------------
//  AllContentModel: Implementation of the ContentModel virtual interface
// ---------------------------------------------------------------------------
//
//Under the XML Schema mixed model,
//the order and number of child elements appearing in an instance
//must agree with
//the order and number of child elements specified in the model.
//
bool
AllContentModel::validateContent( QName** const         children
                                , XMLSize_t             childCount
                                , unsigned int
                                , XMLSize_t*            indexFailingChild
                                , MemoryManager*    const manager) const
{
    // If <all> had minOccurs of zero and there are
    // no children to validate, trivially validate
    if (childCount == 0 && (fHasOptionalContent || !fNumRequired))
        return true;

    // keep track of the required element seen
    XMLSize_t numRequiredSeen = 0;

    if(childCount > 0)
    {        
        // Check for duplicate element
        bool* elementSeen = (bool*) manager->allocate(fCount*sizeof(bool)); //new bool[fCount];

        const ArrayJanitor<bool> jan(elementSeen, manager);

        // initialize the array
        for (XMLSize_t i = 0; i < fCount; i++)
            elementSeen[i] = false;

        for (XMLSize_t outIndex = 0; outIndex < childCount; outIndex++) {
            // Get the current child out of the source index
            const QName* curChild = children[outIndex];

            // If it's PCDATA, then we just accept that
            if (fIsMixed && curChild->getURI() == XMLElementDecl::fgPCDataElemId)
                continue;

            // And try to find it in our list
            XMLSize_t inIndex = 0;
            for (; inIndex < fCount; inIndex++)
            {
                const QName* inChild = fChildren[inIndex];
                if ((inChild->getURI() == curChild->getURI()) &&
                    (XMLString::equals(inChild->getLocalPart(), curChild->getLocalPart()))) {
                    // found it
                    // If this element was seen already, indicate an error was
                    // found at the duplicate index.
                    if (elementSeen[inIndex]) {
                        *indexFailingChild=outIndex;
                        return false;
                    }
                    else
                        elementSeen[inIndex] = true;

                    if (!fChildOptional[inIndex])
                        numRequiredSeen++;

                    break;
                }
            }

            // We did not find this one, so the validation failed
            if (inIndex == fCount) {
                *indexFailingChild=outIndex;
                return false;
            }

        }
    }

    // Were all the required elements of the <all> encountered?
    if (numRequiredSeen != fNumRequired) {
        *indexFailingChild=childCount;
        return false;
    }

    // Everything seems to be ok, so return success
    return true;
}


bool AllContentModel::validateContentSpecial(QName** const          children
                                          , XMLSize_t               childCount
                                          , unsigned int
                                          , GrammarResolver*  const pGrammarResolver
                                          , XMLStringPool*    const pStringPool
                                          , XMLSize_t*              indexFailingChild
                                          , MemoryManager*    const manager) const
{
    // If <all> had minOccurs of zero and there are
    // no children to validate, trivially validate
    if (childCount == 0 && (fHasOptionalContent || !fNumRequired))
        return true;

    // keep track of the required element seen
    XMLSize_t numRequiredSeen = 0;

    if(childCount > 0)
    {
        SubstitutionGroupComparator comparator(pGrammarResolver, pStringPool);

        // Check for duplicate element
        bool* elementSeen = (bool*) manager->allocate(fCount*sizeof(bool)); //new bool[fCount];

        const ArrayJanitor<bool> jan(elementSeen, manager);

        // initialize the array
        for (XMLSize_t i = 0; i < fCount; i++)
            elementSeen[i] = false;

        for (XMLSize_t outIndex = 0; outIndex < childCount; outIndex++) {
            // Get the current child out of the source index
            QName* const curChild = children[outIndex];

            // If it's PCDATA, then we just accept that
            if (fIsMixed && curChild->getURI() == XMLElementDecl::fgPCDataElemId)
                continue;

            // And try to find it in our list
            XMLSize_t inIndex = 0;
            for (; inIndex < fCount; inIndex++)
            {
                QName* const inChild = fChildren[inIndex];
                if ( comparator.isEquivalentTo(curChild, inChild)) {
                    // match
                    // If this element was seen already, indicate an error was
                    // found at the duplicate index.
                    if (elementSeen[inIndex]) {
                        *indexFailingChild=outIndex;
                        return false;
                    }
                    else
                        elementSeen[inIndex] = true;

                    if (!fChildOptional[inIndex])
                        numRequiredSeen++;

                    break;
                }
            }

            // We did not find this one, so the validation failed
            if (inIndex == fCount) {
                *indexFailingChild=outIndex;
                return false;
            }

        }
    }

    // Were all the required elements of the <all> encountered?
    if (numRequiredSeen != fNumRequired) {
        *indexFailingChild=childCount;
        return false;
    }

    // Everything seems to be ok, so return success
    return true;

}

void AllContentModel::checkUniqueParticleAttribution
    (
        SchemaGrammar*    const pGrammar
      , GrammarResolver*  const pGrammarResolver
      , XMLStringPool*    const pStringPool
      , XMLValidator*     const pValidator
      , unsigned int*     const pContentSpecOrgURI
      , const XMLCh*            pComplexTypeName /*= 0*/
    )
{
    SubstitutionGroupComparator comparator(pGrammarResolver, pStringPool);

    XMLSize_t i, j;

    // rename back
    for (i = 0; i < fCount; i++) {
        unsigned int orgURIIndex = fChildren[i]->getURI();
        fChildren[i]->setURI(pContentSpecOrgURI[orgURIIndex]);
    }

    // check whether there is conflict between any two leaves
    for (i = 0; i < fCount; i++) {
        for (j = i+1; j < fCount; j++) {
            // If this is text in a Schema mixed content model, skip it.
            if ( fIsMixed &&
                 (( fChildren[i]->getURI() == XMLElementDecl::fgPCDataElemId) ||
                  ( fChildren[j]->getURI() == XMLElementDecl::fgPCDataElemId)))
                continue;

            if (XercesElementWildcard::conflict(pGrammar,
                                                ContentSpecNode::Leaf,
                                                fChildren[i],
                                                ContentSpecNode::Leaf,
                                                fChildren[j],
                                                &comparator)) {
                pValidator->emitError(XMLValid::UniqueParticleAttributionFail,
                                      pComplexTypeName,
                                      fChildren[i]->getRawName(),
                                      fChildren[j]->getRawName());
             }
         }
    }
}

// ---------------------------------------------------------------------------
//  AllContentModel: Private helper methods
// ---------------------------------------------------------------------------
void
AllContentModel::buildChildList(ContentSpecNode* const       curNode
                              , ValueVectorOf<QName*>&       toFill
                              , ValueVectorOf<bool>&         toOptional)
{
    // Get the type of spec node our current node is
    const ContentSpecNode::NodeTypes curType = curNode->getType();

    if (curType == ContentSpecNode::All)
    {
        // Get both the child node pointers
        ContentSpecNode* leftNode = curNode->getFirst();
        ContentSpecNode* rightNode = curNode->getSecond();

        // Recurse on the left and right nodes
        buildChildList(leftNode, toFill, toOptional);
        if(rightNode)
            buildChildList(rightNode, toFill, toOptional);
    }
    else if (curType == ContentSpecNode::Leaf)
    {
        // At leaf, add the element to list of elements permitted in the all
        toFill.addElement(curNode->getElement());
        toOptional.addElement(false);
        fNumRequired++;
    }
    else if (curType == ContentSpecNode::ZeroOrOne)
    {
        // At ZERO_OR_ONE node, subtree must be an element
        // that was specified with minOccurs=0, maxOccurs=1
        ContentSpecNode* leftNode = curNode->getFirst();
        if (leftNode->getType() != ContentSpecNode::Leaf)
            ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::CM_UnknownCMSpecType, fMemoryManager);

        toFill.addElement(leftNode->getElement());
        toOptional.addElement(true);
    }
    // only allow ZeroOrMore when it's the father of a Loop
    else if (curType == ContentSpecNode::ZeroOrMore &&
             curNode->getFirst()!=0 &&
             curNode->getFirst()->getType()==ContentSpecNode::Loop)
    {
        ContentSpecNode* leftNode = curNode->getFirst();
        buildChildList(leftNode, toFill, toOptional);
    }
    else if (curType == ContentSpecNode::Loop)
    {
        // At leaf, add the element to list of elements permitted in the all
        int i;
        for(i=0;i<curNode->getMinOccurs();i++)
        {
            toFill.addElement(curNode->getElement());
            toOptional.addElement(false);
            fNumRequired++;
        }
        if(curNode->getMaxOccurs()!=-1)
            for(i=0;i<(curNode->getMaxOccurs() - curNode->getMinOccurs());i++)
            {
                toFill.addElement(curNode->getElement());
                toOptional.addElement(true);
            }
    }
    else
        ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::CM_UnknownCMSpecType, fMemoryManager);
}

XERCES_CPP_NAMESPACE_END
