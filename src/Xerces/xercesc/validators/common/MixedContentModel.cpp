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
 * $Id: MixedContentModel.cpp 676911 2008-07-15 13:27:32Z amassari $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <string.h>
#include <xercesc/util/RuntimeException.hpp>
#include <xercesc/framework/XMLElementDecl.hpp>
#include <xercesc/validators/common/ContentSpecNode.hpp>
#include <xercesc/validators/common/MixedContentModel.hpp>
#include <xercesc/validators/common/CMStateSet.hpp>
#include <xercesc/validators/common/Grammar.hpp>
#include <xercesc/validators/schema/SubstitutionGroupComparator.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  MixedContentModel: Constructors and Destructor
// ---------------------------------------------------------------------------
MixedContentModel::MixedContentModel(const bool             dtd
                                   , ContentSpecNode* const parentContentSpec
                                   , const bool             ordered
                                   , MemoryManager* const   manager) :
   fCount(0)
 , fChildren(0)
 , fChildTypes(0)
 , fOrdered(ordered)
 , fDTD(dtd)
 , fMemoryManager(manager)
{
    //
    //  Create a vector of unsigned ints that will be filled in with the
    //  ids of the child nodes. It will be expanded as needed but we give
    //  it an initial capacity of 64 which should be more than enough for
    //  99% of the scenarios.
    //
    ValueVectorOf<QName*> children(64, fMemoryManager);
    ValueVectorOf<ContentSpecNode::NodeTypes> childTypes(64, fMemoryManager);

    //
    //  Get the parent element's content spec. This is the head of the tree
    //  of nodes that describes the content model. We will iterate this
    //  tree.
    //
    ContentSpecNode* curNode = parentContentSpec;
    if (!curNode)
        ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::CM_NoParentCSN, fMemoryManager);

    // And now call the private recursive method that iterates the tree
    buildChildList(curNode, children, childTypes);

    //
    //  And now we know how many elements we need in our member list. So
    //  fill them in.
    //
    fCount = children.size();
    fChildren = (QName**) fMemoryManager->allocate(fCount * sizeof(QName*)); //new QName*[fCount];
    fChildTypes = (ContentSpecNode::NodeTypes*) fMemoryManager->allocate
    (
        fCount * sizeof(ContentSpecNode::NodeTypes)
    ); //new ContentSpecNode::NodeTypes[fCount];
    for (XMLSize_t index = 0; index < fCount; index++) {
        fChildren[index] = new (fMemoryManager) QName(*children.elementAt(index));
        fChildTypes[index] = childTypes.elementAt(index);
    }
}

MixedContentModel::~MixedContentModel()
{
    for (XMLSize_t index = 0; index < fCount; index++) {
        delete fChildren[index];
    }
    fMemoryManager->deallocate(fChildren); //delete [] fChildren;
    fMemoryManager->deallocate(fChildTypes); //delete [] fChildTypes;
}


// ---------------------------------------------------------------------------
//  MixedContentModel: Getter methods
// ---------------------------------------------------------------------------
bool MixedContentModel::hasDups() const
{
    // Can't have dups if only one child
    if (fCount == 1)
        return false;

    for (XMLSize_t index = 0; index < fCount; index++)
    {
        const QName* curVal = fChildren[index];
        for (XMLSize_t iIndex = 0; iIndex < fCount; iIndex++)
        {
            if (iIndex == index)
                continue;

            if (fDTD) {
                if (XMLString::equals(curVal->getRawName(), fChildren[iIndex]->getRawName())) {
                    return true;
                }
            }
            else {
                if ((curVal->getURI() == fChildren[iIndex]->getURI()) &&
                    (XMLString::equals(curVal->getLocalPart(), fChildren[iIndex]->getLocalPart()))) {
                    return true;
                }
            }
        }
    }
    return false;
}


// ---------------------------------------------------------------------------
//  MixedContentModel: Implementation of the ContentModel virtual interface
// ---------------------------------------------------------------------------
//
//Under the XML Schema mixed model,
//the order and number of child elements appearing in an instance
//must agree with
//the order and number of child elements specified in the model.
//
bool
MixedContentModel::validateContent( QName** const         children
                                  , XMLSize_t             childCount
                                  , unsigned int
                                  , XMLSize_t*            indexFailingChild
                                  , MemoryManager*    const) const
{
    // must match order
    if (fOrdered) {
        unsigned int inIndex = 0;
        for (unsigned int outIndex = 0; outIndex < childCount; outIndex++) {

            // Get the current child out of the source index
            const QName* curChild = children[outIndex];

            // If its PCDATA, then we just accept that
            if (curChild->getURI() == XMLElementDecl::fgPCDataElemId)
                continue;

            ContentSpecNode::NodeTypes type = fChildTypes[inIndex];
            const QName* inChild = fChildren[inIndex];

            if (type == ContentSpecNode::Leaf) {
                if (fDTD) {
                    if (!XMLString::equals(inChild->getRawName(), curChild->getRawName())) {
                        *indexFailingChild=outIndex;
                        return false;
                    }
                }
                else {
                    if ((inChild->getURI() != curChild->getURI()) ||
                        (!XMLString::equals(inChild->getLocalPart(), curChild->getLocalPart()))) {
                        *indexFailingChild=outIndex;
                        return false;
                    }
                }
            }
            else if (type == ContentSpecNode::Any) {
            }
            else if (type == ContentSpecNode::Any_NS) {
                if (inChild->getURI() != curChild->getURI())
                {
                    *indexFailingChild=outIndex;
                    return false;
                }
            }
            else if (type == ContentSpecNode::Any_Other)
            {
                // Here we assume that empty string has id 1.
                //
                unsigned int uriId = curChild->getURI();
                if (uriId == 1 || uriId == inChild->getURI())
                {
                    *indexFailingChild=outIndex;
                    return false;
                }
            }

            // advance index
            inIndex++;
        }
    }

    // can appear in any order
    else {
        for (unsigned int outIndex = 0; outIndex < childCount; outIndex++) {
            // Get the current child out of the source index
            const QName* curChild = children[outIndex];

            // If its PCDATA, then we just accept that
            if (curChild->getURI() == XMLElementDecl::fgPCDataElemId)
                continue;

            // And try to find it in our list
            unsigned int inIndex = 0;
            for (; inIndex < fCount; inIndex++)
            {
                ContentSpecNode::NodeTypes type = fChildTypes[inIndex];
                const QName* inChild = fChildren[inIndex];

                if (type == ContentSpecNode::Leaf) {
                    if (fDTD) {
                        if (XMLString::equals(inChild->getRawName(), curChild->getRawName())) {
                            break;
                        }
                    }
                    else {
                        if ((inChild->getURI() == curChild->getURI()) &&
                            (XMLString::equals(inChild->getLocalPart(), curChild->getLocalPart()))) {
                            break;
                        }
                    }
                }
                else if (type == ContentSpecNode::Any) {
                    break;
                }
                else if (type == ContentSpecNode::Any_NS) {
                    if (inChild->getURI() == curChild->getURI())
                        break;
                }
                else if (type == ContentSpecNode::Any_Other)
                {
                    // Here we assume that empty string has id 1.
                    //
                    unsigned int uriId = curChild->getURI();
                    if (uriId != 1 && uriId != inChild->getURI())
                        break;
                }

                // REVISIT: What about checking for multiple ANY matches?
                //          The content model ambiguity *could* be checked
                //          by the caller before constructing the mixed
                //          content model.
            }
            // We did not find this one, so the validation failed
            if (inIndex == fCount)
            {
                *indexFailingChild=outIndex;
                return false;
            }
        }
    }

    // Everything seems to be in order, so return success
    return true;
}


bool MixedContentModel::validateContentSpecial(QName** const          children
                                            , XMLSize_t               childCount
                                            , unsigned int
                                            , GrammarResolver*  const pGrammarResolver
                                            , XMLStringPool*    const pStringPool
                                            , XMLSize_t*              indexFailingChild
                                            , MemoryManager*    const) const
{

    SubstitutionGroupComparator comparator(pGrammarResolver, pStringPool);

    // must match order
    if (fOrdered) {
        unsigned int inIndex = 0;
        for (unsigned int outIndex = 0; outIndex < childCount; outIndex++) {

            // Get the current child out of the source index
            QName* curChild = children[outIndex];

            // If its PCDATA, then we just accept that
            if (curChild->getURI() == XMLElementDecl::fgPCDataElemId)
                continue;

            ContentSpecNode::NodeTypes type = fChildTypes[inIndex];
            QName* inChild = fChildren[inIndex];

            if (type == ContentSpecNode::Leaf) {
                if ( !comparator.isEquivalentTo(curChild, inChild))
                {
                    *indexFailingChild=outIndex;
                    return false;
                }
            }
            else if (type == ContentSpecNode::Any) {
            }
            else if (type == ContentSpecNode::Any_NS) {
                if (inChild->getURI() != curChild->getURI())
                {
                    *indexFailingChild=outIndex;
                    return false;
                }
            }
            else if (type == ContentSpecNode::Any_Other)
            {
                // Here we assume that empty string has id 1.
                //
                unsigned int uriId = curChild->getURI();
                if (uriId == 1 || uriId == inChild->getURI())
                {
                    *indexFailingChild=outIndex;
                    return false;
                }
            }

            // advance index
            inIndex++;
        }
    }

    // can appear in any order
    else {
        for (unsigned int outIndex = 0; outIndex < childCount; outIndex++) {
            // Get the current child out of the source index
            QName* curChild = children[outIndex];

            // If its PCDATA, then we just accept that
            if (curChild->getURI() == XMLElementDecl::fgPCDataElemId)
                continue;

            // And try to find it in our list
            unsigned int inIndex = 0;
            for (; inIndex < fCount; inIndex++)
            {
                ContentSpecNode::NodeTypes type = fChildTypes[inIndex];
                QName* inChild = fChildren[inIndex];

                if (type == ContentSpecNode::Leaf) {
                    if ( comparator.isEquivalentTo(curChild, inChild))
                        break;
                }
                else if (type == ContentSpecNode::Any) {
                    break;
                }
                else if (type == ContentSpecNode::Any_NS) {
                    if (inChild->getURI() == curChild->getURI())
                        break;
                }
                else if (type == ContentSpecNode::Any_Other)
                {
                  // Here we assume that empty string has id 1.
                  //
                  unsigned int uriId = curChild->getURI();
                  if (uriId != 1 && uriId != inChild->getURI())
                    break;
                }

                // REVISIT: What about checking for multiple ANY matches?
                //          The content model ambiguity *could* be checked
                //          by the caller before constructing the mixed
                //          content model.
            }
            // We did not find this one, so the validation failed
            if (inIndex == fCount)
            {
                *indexFailingChild=outIndex;
                return false;
            }
        }
    }

    // Everything seems to be in order, so return success
    return true;
}

// ---------------------------------------------------------------------------
//  MixedContentModel: Private helper methods
// ---------------------------------------------------------------------------
void
MixedContentModel::buildChildList(  ContentSpecNode* const       curNode
                                  , ValueVectorOf<QName*>&       toFill
                                  , ValueVectorOf<ContentSpecNode::NodeTypes>& toType)
{
    // Get the type of spec node our current node is
    const ContentSpecNode::NodeTypes curType = curNode->getType();

    // If its a leaf, then store its id in the target list
    if ((curType == ContentSpecNode::Leaf)      ||
        (curType == ContentSpecNode::Any)       ||
        (curType == ContentSpecNode::Any_Other) ||
        (curType == ContentSpecNode::Any_NS)   )
    {
        toFill.addElement(curNode->getElement());
        toType.addElement(curType);
        return;
    }

    // Get both the child node pointers
    ContentSpecNode* leftNode = curNode->getFirst();
    ContentSpecNode* rightNode = curNode->getSecond();

    // And recurse according to the type of node
    if (((curType & 0x0f) == ContentSpecNode::Choice)
    ||  ((curType & 0x0f) == ContentSpecNode::Sequence))
    {
        // Recurse on the left and right nodes
        buildChildList(leftNode, toFill, toType);

        // The last node of a choice or sequence has a null right
        if (rightNode)
            buildChildList(rightNode, toFill, toType);
    }
    else if ((curType == ContentSpecNode::OneOrMore)
         ||  (curType == ContentSpecNode::ZeroOrOne)
         ||  (curType == ContentSpecNode::ZeroOrMore))
    {
        // Just do the left node on this one
        buildChildList(leftNode, toFill, toType);
    }
}

XERCES_CPP_NAMESPACE_END
