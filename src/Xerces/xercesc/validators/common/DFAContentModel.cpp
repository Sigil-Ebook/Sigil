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
 * $Id: DFAContentModel.cpp 901107 2010-01-20 08:45:02Z borisk $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/RuntimeException.hpp>
#include <xercesc/framework/XMLBuffer.hpp>
#include <xercesc/framework/XMLElementDecl.hpp>
#include <xercesc/framework/XMLValidator.hpp>
#include <xercesc/validators/common/CMAny.hpp>
#include <xercesc/validators/common/CMBinaryOp.hpp>
#include <xercesc/validators/common/CMLeaf.hpp>
#include <xercesc/validators/common/CMRepeatingLeaf.hpp>
#include <xercesc/validators/common/CMUnaryOp.hpp>
#include <xercesc/validators/common/DFAContentModel.hpp>
#include <xercesc/validators/common/ContentSpecNode.hpp>
#include <xercesc/validators/common/Grammar.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xercesc/validators/schema/SubstitutionGroupComparator.hpp>
#include <xercesc/validators/schema/XercesElementWildcard.hpp>
#include <xercesc/util/RefHashTableOf.hpp>
#include <xercesc/util/XMLInteger.hpp>
#include <math.h>

XERCES_CPP_NAMESPACE_BEGIN

struct CMStateSetHasher
{
  XMLSize_t getHashVal(const void *const key, XMLSize_t mod)
  {
    const CMStateSet* const pkey = (const CMStateSet*) key;
    return ((pkey->hashCode()) % mod);
  }

  bool equals(const void *const key1, const void *const key2)
  {
    const CMStateSet* const pkey1 = (const CMStateSet*) key1;
    const CMStateSet* const pkey2 = (const CMStateSet*) key2;
    return (*pkey1==*pkey2);
  }
};

// ---------------------------------------------------------------------------
//  DFAContentModel: Constructors and Destructor
// ---------------------------------------------------------------------------
DFAContentModel::DFAContentModel( const bool             dtd
                                , ContentSpecNode* const elemContentSpec
                                , MemoryManager* const   manager) :

    fElemMap(0)
    , fElemMapType(0)
    , fElemMapSize(0)
    , fEmptyOk(false)
    , fEOCPos(0)
    , fFinalStateFlags(0)
    , fFollowList(0)
    , fHeadNode(0)
    , fLeafCount(0)
    , fLeafList(0)
    , fLeafListType(0)
    , fTransTable(0)
    , fTransTableSize(0)
    , fCountingStates(0)
    , fDTD(dtd)
    , fIsMixed(false)
    , fLeafNameTypeVector(0)
    , fMemoryManager(manager)
{
    // And build the DFA data structures
    buildDFA(elemContentSpec);
}

DFAContentModel::DFAContentModel( const bool             dtd
                                , ContentSpecNode* const elemContentSpec
                                , const bool             isMixed
                                , MemoryManager* const   manager):

    fElemMap(0)
    , fElemMapType(0)
    , fElemMapSize(0)
    , fEmptyOk(false)
    , fEOCPos(0)
    , fFinalStateFlags(0)
    , fFollowList(0)
    , fHeadNode(0)
    , fLeafCount(0)
    , fLeafList(0)
    , fLeafListType(0)
    , fTransTable(0)
    , fTransTableSize(0)
    , fCountingStates(0)
    , fDTD(dtd)
    , fIsMixed(isMixed)
    , fLeafNameTypeVector(0)
    , fMemoryManager(manager)
{
    // And build the DFA data structures
    buildDFA(elemContentSpec);
}

DFAContentModel::~DFAContentModel()
{
    //
    //  Clean up all the stuff that is not just temporary representation
    //  data that was cleaned up after building the DFA.
    //
    fMemoryManager->deallocate(fFinalStateFlags); //delete [] fFinalStateFlags;

    unsigned int index;
    for (index = 0; index < fTransTableSize; index++)
        fMemoryManager->deallocate(fTransTable[index]); //delete [] fTransTable[index];
    fMemoryManager->deallocate(fTransTable); //delete [] fTransTable;

    if(fCountingStates)
    {
        for (unsigned int j = 0; j < fTransTableSize; ++j)
            delete fCountingStates[j];
        fMemoryManager->deallocate(fCountingStates);
    }

    for (index = 0; index < fLeafCount; index++)
        delete fElemMap[index];
    fMemoryManager->deallocate(fElemMap); //delete [] fElemMap;

    fMemoryManager->deallocate(fElemMapType); //delete [] fElemMapType;
    fMemoryManager->deallocate(fLeafListType); //delete [] fLeafListType;

    delete fLeafNameTypeVector;
}


// ---------------------------------------------------------------------------
//  DFAContentModel: Implementation of the ContentModel virtual interface
// ---------------------------------------------------------------------------
bool
DFAContentModel::validateContent( QName** const        children
                                , XMLSize_t            childCount
                                , unsigned int
                                , XMLSize_t*           indexFailingChild
                                , MemoryManager*    const) const
{
    //
    //  If there are no children, then either we fail on the 0th element
    //  or we return success. It depends upon whether this content model
    //  accepts empty content, which we determined earlier.
    //
    if (!childCount)
    {
        // success
        if(fEmptyOk)
            return true;
        *indexFailingChild=0;
        return false;
    }

    //
    //  Lets loop through the children in the array and move our way
    //  through the states. Note that we use the fElemMap array to map
    //  an element index to a state index.
    //
    unsigned int curState = 0;
    unsigned int nextState = 0;
    unsigned int loopCount = 0;
    unsigned int childIndex = 0;
    for (; childIndex < childCount; childIndex++)
    {
        // Get the current element index out
        const QName* curElem = children[childIndex];
        const XMLCh* curElemRawName = 0;
        if (fDTD)
            curElemRawName = curElem->getRawName();

        // If this is text in a Schema mixed content model, skip it.
        if ( fIsMixed &&
            ( curElem->getURI() == XMLElementDecl::fgPCDataElemId))
            continue;

        // Look up this child in our element map
        unsigned int elemIndex = 0;
        for (; elemIndex < fElemMapSize; elemIndex++)
        {
            const QName* inElem  = fElemMap[elemIndex];
            if (fDTD) {
                if (XMLString::equals(inElem->getRawName(), curElemRawName)) {
                    nextState = fTransTable[curState][elemIndex];
                    if (nextState != XMLContentModel::gInvalidTrans)
                        break;
                }
            }
            else {
                ContentSpecNode::NodeTypes type = fElemMapType[elemIndex];
                if (type == ContentSpecNode::Leaf)
                {
                    if ((inElem->getURI() == curElem->getURI()) &&
                    (XMLString::equals(inElem->getLocalPart(), curElem->getLocalPart()))) {
                        nextState = fTransTable[curState][elemIndex];
                        if (nextState != XMLContentModel::gInvalidTrans)
                            break;
                    }
                }
                else if ((type & 0x0f)== ContentSpecNode::Any)
                {
                    nextState = fTransTable[curState][elemIndex];
                    if (nextState != XMLContentModel::gInvalidTrans)
                        break;
                }
                else if ((type & 0x0f) == ContentSpecNode::Any_NS)
                {
                    if (inElem->getURI() == curElem->getURI())
                    {
                        nextState = fTransTable[curState][elemIndex];
                        if (nextState != XMLContentModel::gInvalidTrans)
                            break;
                    }
                }
                else if ((type & 0x0f) == ContentSpecNode::Any_Other)
                {
                    // Here we assume that empty string has id 1.
                    //
                    unsigned int uriId = curElem->getURI();
                    if (uriId != 1 && uriId != inElem->getURI()) {
                        nextState = fTransTable[curState][elemIndex];
                        if (nextState != XMLContentModel::gInvalidTrans)
                            break;
                    }
                }
            }
        }//for elemIndex

        // If "nextState" is -1, we found a match, but the transition is invalid
        if (nextState == XMLContentModel::gInvalidTrans)
        {
            *indexFailingChild=childIndex;
            return false;
        }

        // If we didn't find it, then obviously not valid
        if (elemIndex == fElemMapSize)
        {
            *indexFailingChild=childIndex;
            return false;
        }

        unsigned int nextLoop = 0;
        if(!handleRepetitions(curElem, curState, loopCount, nextState, nextLoop, elemIndex, 0))
        {
            *indexFailingChild=childIndex;
            return false;
        }

        curState = nextState;
        loopCount = nextLoop;
        nextState = 0;

    }//for childIndex

    //
    //  We transitioned all the way through the input list. However, that
    //  does not mean that we ended in a final state. So check whether
    //  our ending state is a final state.
    //
    if (!fFinalStateFlags[curState])
    {
        *indexFailingChild=childIndex;
        return false;
    }

    // verify if we exited before the minOccurs was satisfied
    if (fCountingStates != 0) {
        Occurence* o = fCountingStates[curState];
        if (o != 0 && loopCount < (unsigned int)o->minOccurs) {
            // not enough loops on the current state to be considered final.
            *indexFailingChild=childIndex;
            return false;
        }
    }

    //success
    return true;
}

bool DFAContentModel::validateContentSpecial(QName** const            children
                                            , XMLSize_t               childCount
                                            , unsigned int
                                            , GrammarResolver*  const pGrammarResolver
                                            , XMLStringPool*    const pStringPool
                                            , XMLSize_t*              indexFailingChild
                                            , MemoryManager*    const) const
{

    SubstitutionGroupComparator comparator(pGrammarResolver, pStringPool);

    if (childCount == 0)
    {
        if(fEmptyOk)
            return true;
        *indexFailingChild=0;
        return false;
    }

    //
    //  Lets loop through the children in the array and move our way
    //  through the states. Note that we use the fElemMap array to map
    //  an element index to a state index.
    //
    unsigned int curState = 0;
    unsigned int loopCount = 0;
    unsigned int nextState = 0;
    unsigned int childIndex = 0;
    for (; childIndex < childCount; childIndex++)
    {
        // Get the current element index out
        QName* curElem = children[childIndex];

        // If this is text in a Schema mixed content model, skip it.
        if ( fIsMixed &&
            ( curElem->getURI() == XMLElementDecl::fgPCDataElemId))
            continue;

        // Look up this child in our element map
        unsigned int elemIndex = 0;
        for (; elemIndex < fElemMapSize; elemIndex++)
        {
            QName* inElem  = fElemMap[elemIndex];
            ContentSpecNode::NodeTypes type = fElemMapType[elemIndex];
            if (type == ContentSpecNode::Leaf)
            {
                if (comparator.isEquivalentTo(curElem, inElem) )
                {
                    nextState = fTransTable[curState][elemIndex];
                    if (nextState != XMLContentModel::gInvalidTrans)
                        break;
                }

            }
            else if ((type & 0x0f)== ContentSpecNode::Any)
            {
                nextState = fTransTable[curState][elemIndex];
                if (nextState != XMLContentModel::gInvalidTrans)
                    break;
            }
            else if ((type & 0x0f) == ContentSpecNode::Any_NS)
            {
                if (inElem->getURI() == curElem->getURI())
                {
                    nextState = fTransTable[curState][elemIndex];
                    if (nextState != XMLContentModel::gInvalidTrans)
                        break;
                }
            }
            else if ((type & 0x0f) == ContentSpecNode::Any_Other)
            {
                // Here we assume that empty string has id 1.
                //
                unsigned int uriId = curElem->getURI();
                if (uriId != 1 && uriId != inElem->getURI())
                {
                    nextState = fTransTable[curState][elemIndex];
                    if (nextState != XMLContentModel::gInvalidTrans)
                        break;
                }
            }
        }//for elemIndex

        // If "nextState" is -1, we found a match, but the transition is invalid
        if (nextState == XMLContentModel::gInvalidTrans)
        {
            *indexFailingChild=childIndex;
            return false;
        }

        // If we didn't find it, then obviously not valid
        if (elemIndex == fElemMapSize)
        {
            *indexFailingChild=childIndex;
            return false;
        }

        unsigned int nextLoop = 0;
        if(!handleRepetitions(curElem, curState, loopCount, nextState, nextLoop, elemIndex, &comparator))
        {
            *indexFailingChild=childIndex;
            return false;
        }

        curState = nextState;
        loopCount = nextLoop;
        nextState = 0;

    }//for childIndex

    //
    //  We transitioned all the way through the input list. However, that
    //  does not mean that we ended in a final state. So check whether
    //  our ending state is a final state.
    //
    if (!fFinalStateFlags[curState])
    {
        *indexFailingChild=childIndex;
        return false;
    }

    // verify if we exited before the minOccurs was satisfied
    if (fCountingStates != 0) {
        Occurence* o = fCountingStates[curState];
        if (o != 0) {
            if (loopCount < (unsigned int)o->minOccurs) {
                // not enough loops on the current state.
                *indexFailingChild=childIndex;
                return false;
            }
        }
    }

    //success
    return true;
}

bool DFAContentModel::handleRepetitions(const QName* const curElem,
                                        unsigned int curState,
                                        unsigned int currentLoop,
                                        unsigned int& nextState,
                                        unsigned int& nextLoop,
                                        XMLSize_t elemIndex,
                                        SubstitutionGroupComparator * comparator) const
{
    nextLoop = 0;
    if (fCountingStates != 0) {
        nextLoop = currentLoop;
        Occurence* o = fCountingStates[curState];
        if (o != 0) {
            if (curState == nextState) {
                if (++nextLoop > (unsigned int)o->maxOccurs && o->maxOccurs != -1) {
                    // It's likely that we looped too many times on the current state
                    // however it's possible that we actually matched another particle
                    // which allows the same name.
                    //
                    // Consider:
                    //
                    // <xs:sequence>
                    //  <xs:element name="foo" type="xs:string" minOccurs="3" maxOccurs="3"/>
                    //  <xs:element name="foo" type="xs:string" fixed="bar"/>
                    // </xs:sequence>
                    //
                    // and
                    //
                    // <xs:sequence>
                    //  <xs:element name="foo" type="xs:string" minOccurs="3" maxOccurs="3"/>
                    //  <xs:any namespace="##any" processContents="skip"/>
                    // </xs:sequence>
                    //
                    // In the DFA there will be two transitions from the current state which
                    // allow "foo". Note that this is not a UPA violation. The ambiguity of which
                    // transition to take is resolved by the current value of the counter. Since
                    // we've already seen enough instances of the first "foo" perhaps there is
                    // another element declaration or wildcard deeper in the element map which
                    // matches.
                    unsigned int tempNextState = 0;

                    while (++elemIndex < fElemMapSize) {
                        QName* inElem  = fElemMap[elemIndex];
                        ContentSpecNode::NodeTypes type = fElemMapType[elemIndex];
                        if (type == ContentSpecNode::Leaf)
                        {
                            if(comparator!=0) {
                                if (comparator->isEquivalentTo(curElem, inElem) )
                                {
                                    tempNextState = fTransTable[curState][elemIndex];
                                    if (tempNextState != XMLContentModel::gInvalidTrans)
                                        break;
                                }
                            }
                            else if (fDTD) {
                                if (XMLString::equals(inElem->getRawName(), curElem->getRawName())) {
                                    tempNextState = fTransTable[curState][elemIndex];
                                    if (tempNextState != XMLContentModel::gInvalidTrans)
                                        break;
                                }
                            }
                            else {
                                if ((inElem->getURI() == curElem->getURI()) &&
                                (XMLString::equals(inElem->getLocalPart(), curElem->getLocalPart()))) {
                                    tempNextState = fTransTable[curState][elemIndex];
                                    if (tempNextState != XMLContentModel::gInvalidTrans)
                                        break;
                                }
                            }
                        }
                        else if ((type & 0x0f)== ContentSpecNode::Any)
                        {
                            tempNextState = fTransTable[curState][elemIndex];
                            if (tempNextState != XMLContentModel::gInvalidTrans)
                                break;
                        }
                        else if ((type & 0x0f) == ContentSpecNode::Any_NS)
                        {
                            if (inElem->getURI() == curElem->getURI())
                            {
                                tempNextState = fTransTable[curState][elemIndex];
                                if (tempNextState != XMLContentModel::gInvalidTrans)
                                    break;
                            }
                        }
                        else if ((type & 0x0f) == ContentSpecNode::Any_Other)
                        {
                            // Here we assume that empty string has id 1.
                            //
                            unsigned int uriId = curElem->getURI();
                            if (uriId != 1 && uriId != inElem->getURI())
                            {
                                tempNextState = fTransTable[curState][elemIndex];
                                if (tempNextState != XMLContentModel::gInvalidTrans)
                                    break;
                            }
                        }
                    }

                    // if we still can't find a match, report the error
                    if (elemIndex == fElemMapSize)
                        return false;

                    // if we found a match, set the next state and reset the
                    // counter if the next state is a counting state.
                    nextState = tempNextState;
                    Occurence* o = fCountingStates[nextState];
                    if (o != 0) {
                      nextLoop = (elemIndex == XMLSize_t (o->elemIndex)) ? 1 : 0;
                    }
                }
            }
            else if (nextLoop < (unsigned int)o->minOccurs) {
                // not enough loops on the current state.
                return false;
            }
            else {
                // Exiting a counting state. If we're entering a new
                // counting state, reset the counter.
                o = fCountingStates[nextState];
                if (o != 0) {
                  nextLoop = (elemIndex == XMLSize_t (o->elemIndex)) ? 1 : 0;
                }
            }
        }
        else {
            o = fCountingStates[nextState];
            if (o != 0) {
                // Entering a new counting state. Reset the counter.
                // If we've already seen one instance of the looping
                // particle set the counter to 1, otherwise set it
                // to 0.
              nextLoop = (elemIndex == XMLSize_t (o->elemIndex)) ? 1 : 0;
            }
        }
    }
    return true;
}

// ---------------------------------------------------------------------------
//  DFAContentModel: Private helper methods
// ---------------------------------------------------------------------------
void DFAContentModel::buildDFA(ContentSpecNode* const curNode)
{
    unsigned int index;

    //
    //  The first step we need to take is to rewrite the content model using
    //  our CMNode objects, and in the process get rid of any repetition short
    //  cuts, converting them into '*' style repetitions or getting rid of
    //  repetitions altogether.
    //
    //  The conversions done are:
    //
    //  x+ -> (x|x*)
    //  x? -> (x|epsilon)
    //
    //  This is a relatively complex scenario. What is happening is that we
    //  create a top level binary node of which the special EOC value is set
    //  as the right side node. The the left side is set to the rewritten
    //  syntax tree. The source is the original content model info from the
    //  decl pool. The rewrite is done by buildSyntaxTree() which recurses the
    //  decl pool's content of the element and builds a new tree in the
    //  process.
    //
    //  Note that, during this operation, we set each non-epsilon leaf node's
    //  DFA state position and count the number of such leafs, which is left
    //  in the fLeafCount member.
    //
    fLeafCount=countLeafNodes(curNode);
    fEOCPos = fLeafCount++;

    //  We need to build an array of references to the non-epsilon
    //  leaf nodes. We will put them in the array according to their position values
    //
    fLeafList = (CMLeaf**) fMemoryManager->allocate(fLeafCount*sizeof(CMLeaf*)); //new CMLeaf*[fLeafCount];
    fLeafListType = (ContentSpecNode::NodeTypes*) fMemoryManager->allocate
    (
        fLeafCount * sizeof(ContentSpecNode::NodeTypes)
    ); //new ContentSpecNode::NodeTypes[fLeafCount];
    //
    //  And, moving onward... We now need to build the follow position sets
    //  for all the nodes. So we allocate an array of pointers to state sets,
    //  one for each leaf node (i.e. each significant DFA position.)
    //
    fFollowList = (CMStateSet**) fMemoryManager->allocate
    (
        fLeafCount * sizeof(CMStateSet*)
    ); //new CMStateSet*[fLeafCount];
    for (index = 0; index < fLeafCount; index++)
        fFollowList[index] = new (fMemoryManager) CMStateSet(fLeafCount, fMemoryManager);

    //  The buildSyntaxTree function will recursively iterate over the ContentSpecNode
    //  and build the CMNode hierarchy; it will also put every leaf node in the fLeafList
    //  array, then calculate the first and last position sets of each node. This is
    //  cached away in each of the nodes.
    //
    //  Along the way we also set the leaf count in each node as the maximum
    //  state count. They must know this in order to create their first/last
    //  position sets.
    //
    unsigned int counter=0;
    CMNode* nodeOrgContent = buildSyntaxTree(curNode, counter);
    //
    //  Check to see whether this content model can handle an empty content,
    //  which is something we need to optimize by looking now before we
    //  throw away the info that would tell us that.
    //
    //  If the left node of the head (the top level of the original content)
    //  is nullable, then its true.
    //
    fEmptyOk = nodeOrgContent->isNullable();

    //
    //  And handle specially the EOC node, which also must be numbered and
    //  counted as a non-epsilon leaf node. It could not be handled in the
    //  above tree build because it was created before all that started. We
    //  save the EOC position since its used during the DFA building loop.
    //
    CMLeaf* nodeEOC = new (fMemoryManager) CMLeaf
    (
        new (fMemoryManager) QName
        (
            XMLUni::fgZeroLenString
            , XMLUni::fgZeroLenString
            , XMLContentModel::gEOCFakeId
            , fMemoryManager
        )
        , fEOCPos
        , true
        , fLeafCount
        , fMemoryManager
    );
    fHeadNode = new (fMemoryManager) CMBinaryOp
    (
        ContentSpecNode::Sequence
        , nodeOrgContent
        , nodeEOC
        , fLeafCount
        , fMemoryManager
    );

    //  Put also the final EOC node in the leaf array
    fLeafList[counter] = new (fMemoryManager) CMLeaf
    (
        nodeEOC->getElement()
        , nodeEOC->getPosition()
        , fLeafCount
        , fMemoryManager
    );
    fLeafListType[counter] = ContentSpecNode::Leaf;

    //
    //  Now handle our top level. We use our left child's last pos set and our
    //  right child's first pos set, so get them now for convenience.
    //
    const CMStateSet& last  = nodeOrgContent->getLastPos();
    const CMStateSet& first = nodeEOC->getFirstPos();

    //
    //  Now, for every position which is in our left child's last set
    //  add all of the states in our right child's first set to the
    //  follow set for that position.
    //
    CMStateSetEnumerator enumLast(&last);
    while(enumLast.hasMoreElements())
    {
        XMLSize_t index=enumLast.nextElement();
        *fFollowList[index] |= first;
    }

    //
    //  And finally the big push... Now we build the DFA using all the states
    //  and the tree we've built up. First we set up the various data
    //  structures we are going to use while we do this.
    //
    //  First of all we need an array of unique element ids in our content
    //  model. For each transition table entry, we need a set of contiguous
    //  indices to represent the transitions for a particular input element.
    //  So we need to a zero based range of indexes that map to element types.
    //  This element map provides that mapping.
    //
    fElemMap = (QName**) fMemoryManager->allocate
    (
        fLeafCount * sizeof(QName*)
    ); //new QName*[fLeafCount];
    fElemMapType = (ContentSpecNode::NodeTypes*) fMemoryManager->allocate
    (
        fLeafCount * sizeof(ContentSpecNode::NodeTypes)
    ); //new ContentSpecNode::NodeTypes[fLeafCount];
    fElemMapSize = 0;

    Occurence** elemOccurenceMap=0;
    for (unsigned int outIndex = 0; outIndex < fLeafCount; outIndex++)
    {
        fElemMap[outIndex] = new (fMemoryManager) QName(fMemoryManager);

        if ( (fLeafListType[outIndex] & 0x0f) != ContentSpecNode::Leaf )
            if (!fLeafNameTypeVector)
                fLeafNameTypeVector = new (fMemoryManager) ContentLeafNameTypeVector(fMemoryManager);

        // Get the current leaf's element index
        CMLeaf* leaf=fLeafList[outIndex];
        const QName* element = leaf->getElement();
        const XMLCh* elementRawName = 0;
        if (fDTD && element)
            elementRawName = element->getRawName();

        // See if the current leaf node's element index is in the list
        unsigned int inIndex = 0;

        for (; inIndex < fElemMapSize; inIndex++)
        {
            const QName* inElem = fElemMap[inIndex];
            if (fDTD) {
                if (XMLString::equals(inElem->getRawName(), elementRawName)) {
                    break;
                }
            }
            else {
                if ((fElemMapType[inIndex] == fLeafListType[outIndex]) &&
                    (inElem->getURI() == element->getURI()) &&
                    (XMLString::equals(inElem->getLocalPart(), element->getLocalPart()))) {
                    break;
                }
            }
        }

        // If it was not in the list, then add it and bump the map size
        if (inIndex == fElemMapSize)
        {
            fElemMap[fElemMapSize]->setValues(*element);
            if(leaf->isRepeatableLeaf())
            {
                if (elemOccurenceMap == 0) {
                    elemOccurenceMap = (Occurence**)fMemoryManager->allocate(fLeafCount*sizeof(Occurence*));
                    memset(elemOccurenceMap, 0, fLeafCount*sizeof(Occurence*));
                }
                elemOccurenceMap[fElemMapSize] = new (fMemoryManager) Occurence(((CMRepeatingLeaf*)leaf)->getMinOccurs(), ((CMRepeatingLeaf*)leaf)->getMaxOccurs(), fElemMapSize);
            }
            fElemMapType[fElemMapSize] = fLeafListType[outIndex];
            ++fElemMapSize;
        }
    }

    // set up the fLeafNameTypeVector object if there is one.
    if (fLeafNameTypeVector) {
        fLeafNameTypeVector->setValues(fElemMap, fElemMapType, fElemMapSize);
    }

    /***
     * Optimization(Jan, 2001); We sort fLeafList according to
     * elemIndex which is *uniquely* associated to each leaf.
     * We are *assuming* that each element appears in at least one leaf.
     **/
    // don't forget to delete it
#ifdef OPTIMIZED_BUT_STILL_LINEAR_SEARCH
    int *fLeafSorter = (int*) fMemoryManager->allocate
    (
        (fLeafCount + fElemMapSize) * sizeof(int)
    ); //new int[fLeafCount + fElemMapSize];
    unsigned int fSortCount = 0;

    for (unsigned int elemIndex = 0; elemIndex < fElemMapSize; elemIndex++)
    {
        const QName* element = fElemMap[elemIndex];
        const XMLCh* elementRawName = 0;
        if (fDTD && element)
            elementRawName = element->getRawName();

        for (unsigned int leafIndex = 0; leafIndex < fLeafCount; leafIndex++)
        {
            const QName* leaf = fLeafList[leafIndex]->getElement();
            if (fDTD) {
                if (XMLString::equals(leaf->getRawName(), elementRawName)) {
                    fLeafSorter[fSortCount++] = leafIndex;
                }
            }
            else {
                if ((fElemMapType[elemIndex] == fLeafListType[leafIndex]) &&
                    (leaf->getURI() == element->getURI()) &&
                    (XMLString::equals(leaf->getLocalPart(), element->getLocalPart()))) {
                      fLeafSorter[fSortCount++] = leafIndex;
                }
            }
        }
        fLeafSorter[fSortCount++] = -1;
    }
#endif

    // instead of using a single array with -1 to separate elements, use a bidimensional map
    unsigned int** fLeafSorter = (unsigned int**)fMemoryManager->allocate(fElemMapSize * sizeof(unsigned int*));
    unsigned int* tmpSorter = (unsigned int*)fMemoryManager->allocate(fLeafCount * sizeof(unsigned int));
    for (unsigned int elemIndex = 0; elemIndex < fElemMapSize; elemIndex++)
    {
        const QName* element = fElemMap[elemIndex];
        const XMLCh* elementRawName = 0;
        if (fDTD && element)
            elementRawName = element->getRawName();

        unsigned int fSortCount=0;
        for (unsigned int leafIndex = 0; leafIndex < fLeafCount; leafIndex++)
        {
            const QName* leaf = fLeafList[leafIndex]->getElement();
            if (fDTD) {
                if (XMLString::equals(leaf->getRawName(), elementRawName)) {
                    tmpSorter[fSortCount++] = leafIndex;
                }
            }
            else {
                if ((fElemMapType[elemIndex] == fLeafListType[leafIndex]) &&
                    (leaf->getURI() == element->getURI()) &&
                    (XMLString::equals(leaf->getLocalPart(), element->getLocalPart()))) {
                      tmpSorter[fSortCount++] = leafIndex;
                }
            }
        }

        fLeafSorter[elemIndex]=(unsigned int*)fMemoryManager->allocate((fSortCount+1) * sizeof(unsigned int));
        fLeafSorter[elemIndex][0]=fSortCount;
        for (unsigned int index=0;index<fSortCount;index++)
            fLeafSorter[elemIndex][index+1]=tmpSorter[index];
    }
    fMemoryManager->deallocate(tmpSorter);

    //
    //  Next lets create some arrays, some that that hold transient info
    //  during the DFA build and some that are permament. These are kind of
    //  sticky since we cannot know how big they will get, but we don't want
    //  to use any collection type classes because of performance.
    //
    //  Basically they will probably be about fLeafCount*2 on average, but can
    //  be as large as 2^(fLeafCount*2), worst case. So we start with
    //  fLeafCount*4 as a middle ground. This will be very unlikely to ever
    //  have to expand though, it if does, the overhead will be somewhat ugly.
    //
    unsigned int curArraySize = fLeafCount * 4;
    CMStateSet** statesToDo = (CMStateSet**)
        fMemoryManager->allocate
        (
            curArraySize * sizeof(CMStateSet*)
        ); //new const CMStateSet*[curArraySize];
    fFinalStateFlags = (bool*) fMemoryManager->allocate
    (
        curArraySize * sizeof(bool)
    ); //new bool[curArraySize];
    fTransTable = (unsigned int**) fMemoryManager->allocate
    (
        curArraySize * sizeof(unsigned int*)
    ); //new unsigned int*[curArraySize];

    //
    //  Ok we start with the initial set as the first pos set of the head node
    //  (which is the seq node that holds the content model and the EOC node.)
    //
    CMStateSet* setT = new (fMemoryManager) CMStateSet(fHeadNode->getFirstPos());

    //
    // Note on memory leak: Bugzilla#2707:
    // ===================================
    // The CMBinary, pointed to by fHeadNode, shall be released by
    // deleted by itself.
    //
    // fLeafList[] maintains its **OWN** copy of CMLeaf to avoid double deletion
    // of CMLeaf.
    //

    delete fHeadNode;

    //
    //  Init our two state flags. Basically the unmarked state counter is
    //  always chasing the current state counter. When it catches up, that
    //  means we made a pass through that did not add any new states to the
    //  lists, at which time we are done. We could have used a expanding array
    //  of flags which we used to mark off states as we complete them, but
    //  this is easier though less readable maybe.
    //
    unsigned int unmarkedState = 0;
    unsigned int curState = 0;

    //
    //  Init the first transition table entry, and put the initial state
    //  into the states to do list, then bump the current state.
    //
    fTransTable[curState] = makeDefStateList();
    statesToDo[curState] = setT;
    curState++;

    //
    // the stateTable is an auxiliary means to fast
    // identification of new state created (instead
    // of sequential loop statesToDo to find out),
    // while the role that statesToDo plays remain unchanged.
    //
    RefHashTableOf<XMLInteger, CMStateSetHasher> *stateTable =
        new (fMemoryManager) RefHashTableOf<XMLInteger, CMStateSetHasher>
        (
            curArraySize
            , true
            , fMemoryManager
        );
    //stateTable->put((CMStateSet*)setT, new (fMemoryManager) XMLInteger(0));

    //
    //  Ok, almost done with the algorithm from hell... We now enter the
    //  loop where we go until the states done counter catches up with
    //  the states to do counter.
    //
    CMStateSet* newSet = 0;
    while (unmarkedState < curState)
    {
        //
        //  Get the next unmarked state out of the list of states to do.
        //  And get the associated transition table entry.
        //
        setT = statesToDo[unmarkedState];
        unsigned int* transEntry = fTransTable[unmarkedState];

        // Mark this one final if it contains the EOC state
        fFinalStateFlags[unmarkedState] = setT->getBit(fEOCPos);

        // Bump up the unmarked state count, marking this state done
        unmarkedState++;

#ifdef OPTIMIZED_BUT_STILL_LINEAR_SEARCH
        // Optimization(Jan, 2001)
        unsigned int sorterIndex = 0;
        // Optimization(Jan, 2001)
#endif

        // Loop through each possible input symbol in the element map
        for (unsigned int elemIndex = 0; elemIndex < fElemMapSize; elemIndex++)
        {
            //
            //  Build up a set of states which is the union of all of the
            //  follow sets of DFA positions that are in the current state. If
            //  we gave away the new set last time through then create a new
            //  one. Otherwise, zero out the existing one.
            //
            if (!newSet)
                newSet = new (fMemoryManager) CMStateSet
                (
                    fLeafCount
                    , fMemoryManager
                );
            else
                newSet->zeroBits();

#ifdef OBSOLETED
// unoptimized code
            for (unsigned int leafIndex = 0; leafIndex < fLeafCount; leafIndex++)
            {
                // If this leaf index (DFA position) is in the current set...
                if (setT->getBit(leafIndex))
                {
                    //
                    //  If this leaf is the current input symbol, then we want
                    //  to add its follow list to the set of states to transition
                    //  to from the current state.
                    //
                    const QName* leaf = fLeafList[leafIndex]->getElement();
                    const QName* element = fElemMap[elemIndex];
                    if (fDTD) {
                        if (XMLString::equals(leaf->getRawName(), element->getRawName())) {
                            *newSet |= *fFollowList[leafIndex];
                        }
                    }
                    else {
                        if ((leaf->getURI() == element->getURI()) &&
                            (XMLString::equals(leaf->getLocalPart(), element->getLocalPart()))) {
                            *newSet |= *fFollowList[leafIndex];
                        }
                    }
                }
            } // for leafIndex
#endif

#ifdef OPTIMIZED_BUT_STILL_LINEAR_SEARCH
            // Optimization(Jan, 2001)
            int leafIndex = fLeafSorter[sorterIndex++];

            while (leafIndex != -1)
            {
                // If this leaf index (DFA position) is in the current set...
                if (setT->getBit(leafIndex))
                {
                    //
                    //  If this leaf is the current input symbol, then we
                    //  want to add its follow list to the set of states to
                    //  transition to from the current state.
                    //
                    *newSet |= *fFollowList[leafIndex];
                }
                leafIndex = fLeafSorter[sorterIndex++];
            } // while (leafIndex != -1)
#endif

            unsigned int* fLeafIndexes=fLeafSorter[elemIndex];
            unsigned int fNumItems=fLeafIndexes[0];
            if(fNumItems!=0)
            {
                // The algorithm requires finding the leaf that is present both in the bitfield of the current state, and in the
                // list of places where the currently tested item can appear. When this occurs, the follow list of this parent item
                // is added to the bitfield representing the next state.
                // Both the bitfield and the list of places are sorted, so we can analyze them in two ways; either iterating over the
                // parent items, testing the bitfield for the existence of the parent (N times a constant Tb), or by iterating over the
                // bitfield (restricted to the range of the sorted list of places), using a binary search to locate the leaf in the
                // sorted list of places (M times log(N) testing operations Ts)
                // Assuming that the time to test a bit is roughly the same of the time needed to compute the average of two integers,
                // plus a couple of comparisons and additions, we compare N agains M*log(N) to decide which algorithm should be faster given
                // the two sets
                if(fNumItems <= setT->getBitCountInRange(fLeafIndexes[1], fLeafIndexes[fNumItems])*log((float)fNumItems))
                {
                    for(unsigned int i=1; i<=fNumItems; ++i)
                        if(setT->getBit(fLeafIndexes[i]))
                        {
                            //
                            //  If this leaf is the current input symbol, then we
                            //  want to add its follow list to the set of states to
                            //  transition to from the current state.
                            //
                            *newSet |= *fFollowList[ fLeafIndexes[i] ];
                        }
                }
                else
                {
                    // Further optimization: given that the bitfield enumerator returns the numbers in order,
                    // every time we raise the lower marker we know it will true also for the next bits, so
                    // the next binary search will not start from 1 but from this index
                    unsigned int lowIndex = 1;
                    // Start the enumerator from the first index in the sorted list of places,
                    // as nothing before that point will match
                    CMStateSetEnumerator enumBits(setT, fLeafIndexes[1]);
                    while(enumBits.hasMoreElements())
                    {
                        unsigned int bitIndex=enumBits.nextElement();
                        // if this leaf is greater than the last index in the sorted list of places,
                        // nothing can be found from now on, so get out of here
                        if(bitIndex > fLeafIndexes[fNumItems])
                            break;

                        // Check if this leaf index (DFA position) is in the current set
                        // (using binary search: the indexes are sorted)
                        unsigned int first=lowIndex,last=fNumItems,i;
                        while(first<=last)
                        {
                            i=(first+last)/2;
                            if(fLeafIndexes[i]>bitIndex)
                                last=i-1;
                            else if(fLeafIndexes[i]<bitIndex)
                                lowIndex=first=i+1;
                            else
                            {
                                //
                                //  If this leaf is the current input symbol, then we
                                //  want to add its follow list to the set of states to
                                //  transition to from the current state.
                                //
                                *newSet |= *fFollowList[bitIndex];
                                break;
                            }
                        }
                    }
                }
            }

            //
            //  If this new set is not empty, then see if its in the list
            //  of states to do. If not, then add it.
            //
            if (!newSet->isEmpty())
            {
                //
                //  Search the 'states to do' list to see if this new
                //  state set is already in there.
                //
                /***
                unsigned int stateIndex = 0;
                for (; stateIndex < curState; stateIndex++)
                {
                    if (*statesToDo[stateIndex] == *newSet)
                        break;
                }
                ***/

                XMLInteger *stateObj = stateTable->get(newSet);
                unsigned int stateIndex = (stateObj == 0 ? curState : stateObj->intValue());

                // If we did not find it, then add it
                if (stateIndex == curState)
                {
                    //
                    //  Put this new state into the states to do and init
                    //  a new entry at the same index in the transition
                    //  table.
                    //
                    statesToDo[curState] = newSet;
                    fTransTable[curState] = makeDefStateList();
                    stateTable->put
                    (
                        newSet
                        , new (fMemoryManager) XMLInteger(curState)
                    );

                    // We now have a new state to do so bump the count
                    curState++;

                    //
                    //  Null out the new set to indicate we adopted it. This
                    //  will cause the creation of a new set on the next time
                    //  around the loop.
                    //
                    newSet = 0;
                }

                //
                //  Now set this state in the transition table's entry for this
                //  element (using its index), with the DFA state we will move
                //  to from the current state when we see this input element.
                //
                transEntry[elemIndex] = stateIndex;

                // Expand the arrays if we're full
                if (curState == curArraySize)
                {
                    //
                    //  Yikes, we overflowed the initial array size, so we've
                    //  got to expand all of these arrays. So adjust up the
                    //  size by 50% and allocate new arrays.
                    //
                    const unsigned int newSize = (unsigned int)(curArraySize * 1.5);
                    CMStateSet** newToDo = (CMStateSet**)
                        fMemoryManager->allocate
                        (
                            newSize * sizeof(CMStateSet*)
                        ); //new const CMStateSet*[newSize];
                    bool* newFinalFlags = (bool*) fMemoryManager->allocate
                    (
                        newSize * sizeof(bool)
                    ); //new bool[newSize];
                    unsigned int** newTransTable = (unsigned int**)
                        fMemoryManager->allocate
                        (
                            newSize * sizeof(unsigned int*)
                        ); //new unsigned int*[newSize];

                    // Copy over all of the existing content
                    for (unsigned int expIndex = 0; expIndex < curArraySize; expIndex++)
                    {
                        newToDo[expIndex] = statesToDo[expIndex];
                        newFinalFlags[expIndex] = fFinalStateFlags[expIndex];
                        newTransTable[expIndex] = fTransTable[expIndex];
                    }

                    // Clean up the old stuff
                    fMemoryManager->deallocate(statesToDo); //delete [] statesToDo;
                    fMemoryManager->deallocate(fFinalStateFlags); //delete [] fFinalStateFlags;
                    fMemoryManager->deallocate(fTransTable); //delete [] fTransTable;

                    // Store the new array size and pointers
                    curArraySize = newSize;
                    statesToDo = newToDo;
                    fFinalStateFlags = newFinalFlags;
                    fTransTable = newTransTable;
                } //if (curState == curArraySize)
            } //if (!newSet->isEmpty())
        } // for elemIndex
    } //while

    // Store the current state count in the trans table size
    fTransTableSize = curState;

    //
    // Fill in the occurence information for each looping state
    // if we're using counters.
    //
    if (elemOccurenceMap != 0) {
        fCountingStates = (Occurence**)fMemoryManager->allocate(fTransTableSize*sizeof(Occurence));
        memset(fCountingStates, 0, fTransTableSize*sizeof(Occurence*));
        for (unsigned int i = 0; i < fTransTableSize; ++i) {
            unsigned int * transitions = fTransTable[i];
            for (unsigned int j = 0; j < fElemMapSize; ++j) {
                if (i == transitions[j]) {
                    Occurence* old=elemOccurenceMap[j];
                    if(old!=0)
                        fCountingStates[i] = new (fMemoryManager) Occurence(old->minOccurs, old->maxOccurs, old->elemIndex);
                    break;
                }
            }
        }
        for (unsigned int j = 0; j < fLeafCount; ++j) {
            if(elemOccurenceMap[j]!=0)
                delete elemOccurenceMap[j];
        }
        fMemoryManager->deallocate(elemOccurenceMap);
    }

    // If the last temp set was not stored, then clean it up
    if (newSet)
        delete newSet;

    //
    //  Now we can clean up all of the temporary data that was needed during
    //  DFA build.
    //

    for (index = 0; index < fLeafCount; index++)
        delete fFollowList[index];
    fMemoryManager->deallocate(fFollowList); //delete [] fFollowList;

    //
    // removeAll() will delete all data, XMLInteger,
    // while the keys are to be deleted by the
    // deletion of statesToDo.
    //
    delete stateTable;

    for (index = 0; index < curState; index++)
        delete statesToDo[index];
    fMemoryManager->deallocate(statesToDo); //delete [] statesToDo;

    for (index = 0; index < fLeafCount; index++)
        delete fLeafList[index];
    fMemoryManager->deallocate(fLeafList); //delete [] fLeafList;

#ifdef OPTIMIZED_BUT_STILL_LINEAR_SEARCH
    fMemoryManager->deallocate(fLeafSorter); //delete [] fLeafSorter;
#endif
    for (index=0; index < fElemMapSize; index++)
        fMemoryManager->deallocate(fLeafSorter[index]);
    fMemoryManager->deallocate(fLeafSorter);
}

unsigned int DFAContentModel::countLeafNodes(ContentSpecNode* const curNode)
{
    unsigned int count = 0;

    // Get the spec type of the passed node
    const ContentSpecNode::NodeTypes curType = curNode->getType();

    if ((curType & 0x0f) == ContentSpecNode::Any
        || (curType & 0x0f) == ContentSpecNode::Any_Other
        || (curType & 0x0f) == ContentSpecNode::Any_NS
        || curType == ContentSpecNode::Leaf
        || curType == ContentSpecNode::Loop)
    {
        count++;
    }
    else
    {
        //
        //  Its not a leaf, so we have to recurse its left and maybe right
        //  nodes. Save both values before we recurse and trash the node.
        //
        ContentSpecNode* leftNode = curNode->getFirst();
        ContentSpecNode* rightNode = curNode->getSecond();

        // Detect if we have a deep tree that can be analyzed using a loop instead of recursion
        unsigned int nLoopCount=0;
        ContentSpecNode* cursor=curNode;
        while(cursor->getType()==ContentSpecNode::Sequence && cursor->getFirst() && cursor->getFirst()->getSecond()==rightNode)
        {
            nLoopCount++;
            cursor=cursor->getFirst();
        }
        if(nLoopCount!=0)
        {
            count += countLeafNodes(cursor);
            for(unsigned int i=0;i<nLoopCount;i++)
                count += countLeafNodes(rightNode);
            return count;
        }
        if(leftNode)
            count+=countLeafNodes(leftNode);
        if(rightNode)
            count+=countLeafNodes(rightNode);
    }
    return count;
}

CMNode* DFAContentModel::buildSyntaxTree(ContentSpecNode* const curNode
                                         , unsigned int&        curIndex)
{
    // Initialize a return node pointer
    CMNode* retNode = 0;

    // Get the spec type of the passed node
    const ContentSpecNode::NodeTypes curType = curNode->getType();

    if ((curType & 0x0f) == ContentSpecNode::Any
        || (curType & 0x0f) == ContentSpecNode::Any_Other
        || (curType & 0x0f) == ContentSpecNode::Any_NS)
    {
        retNode = new (fMemoryManager) CMAny
        (
            curType
            , curNode->getElement()->getURI()
            , curIndex
            , fLeafCount
            , fMemoryManager
        );
        fLeafList[curIndex] = new (fMemoryManager) CMLeaf
        (
            new (fMemoryManager) QName
            (
                XMLUni::fgZeroLenString
                , XMLUni::fgZeroLenString
                , curNode->getElement()->getURI()
                , fMemoryManager
            )
            , curIndex
            , true
            , fLeafCount
            , fMemoryManager
        );
        fLeafListType[curIndex] = curType;
        ++curIndex;
    }
    else if (curType == ContentSpecNode::Leaf)
    {
        //
        //  Create a new leaf node, and pass it the current leaf count, which
        //  is its DFA state position. Bump the leaf count after storing it.
        //  This makes the positions zero based since we store first and then
        //  increment.
        //
        retNode = new (fMemoryManager) CMLeaf
        (
            curNode->getElement()
            , curIndex
            , fLeafCount
            , fMemoryManager
        );
        fLeafList[curIndex] = new (fMemoryManager) CMLeaf
        (
            curNode->getElement()
            , curIndex
            , fLeafCount
            , fMemoryManager
        );
        fLeafListType[curIndex] = ContentSpecNode::Leaf;
        ++curIndex;
    }
    else if (curType == ContentSpecNode::Loop)
    {
        //
        //  Create a new leaf node, and pass it the current leaf count, which
        //  is its DFA state position. Bump the leaf count after storing it.
        //  This makes the positions zero based since we store first and then
        //  increment.
        //
        retNode = new (fMemoryManager) CMRepeatingLeaf
        (
            curNode->getFirst()->getElement()
            , curNode->getMinOccurs()
            , curNode->getMaxOccurs()
            , curIndex
            , fLeafCount
            , fMemoryManager
        );
        fLeafList[curIndex] = new (fMemoryManager) CMRepeatingLeaf
        (
            curNode->getFirst()->getElement()
            , curNode->getMinOccurs()
            , curNode->getMaxOccurs()
            , curIndex
            , fLeafCount
            , fMemoryManager
        );
        fLeafListType[curIndex] = curNode->getFirst()->getType();
        ++curIndex;
    }
     else
    {
        //
        //  Its not a leaf, so we have to recurse its left and maybe right
        //  nodes. Save both values before we recurse and trash the node.
        //
        ContentSpecNode* leftNode = curNode->getFirst();
        ContentSpecNode* rightNode = curNode->getSecond();

        // Detect if we have a deep tree that can be analyzed using a loop instead of recursion
        unsigned int nLoopCount=0;
        ContentSpecNode* cursor=curNode;
        while(cursor->getType()==ContentSpecNode::Sequence && cursor->getFirst() && cursor->getFirst()->getSecond()==rightNode)
        {
            nLoopCount++;
            cursor=cursor->getFirst();
        }
        if(nLoopCount!=0)
        {
            retNode = buildSyntaxTree(cursor, curIndex);
            for(unsigned int i=0;i<nLoopCount;i++)
            {
                CMNode* newRight = buildSyntaxTree(rightNode, curIndex);
                //
                //  Now handle our level. We use our left child's last pos set and our
                //  right child's first pos set, so get them now for convenience.
                //
                const CMStateSet& last  = retNode->getLastPos();
                const CMStateSet& first = newRight->getFirstPos();

                //
                //  Now, for every position which is in our left child's last set
                //  add all of the states in our right child's first set to the
                //  follow set for that position.
                //
                CMStateSetEnumerator enumLast(&last);
                while(enumLast.hasMoreElements())
                {
                    XMLSize_t index=enumLast.nextElement();
                    *fFollowList[index] |= first;
                }
                retNode = new (fMemoryManager) CMBinaryOp
                (
                    ContentSpecNode::Sequence
                    , retNode
                    , newRight
                    , fLeafCount
                    , fMemoryManager
                );
            }
            return retNode;
        }

        if (((curType & 0x0f) == ContentSpecNode::Choice)
        ||   ((curType & 0x0f) == ContentSpecNode::Sequence))
        {
            //
            //  Recurse on both children, and return a binary op node with the
            //  two created sub nodes as its children. The node type is the
            //  same type as the source.
            //
            CMNode* newLeft = buildSyntaxTree(leftNode, curIndex);
            CMNode* newRight = buildSyntaxTree(rightNode, curIndex);
            if(((curType & 0x0f) == ContentSpecNode::Sequence))
            {
                //
                //  Now handle our level. We use our left child's last pos set and our
                //  right child's first pos set, so get them now for convenience.
                //
                const CMStateSet& last  = newLeft->getLastPos();
                const CMStateSet& first = newRight->getFirstPos();

                //
                //  Now, for every position which is in our left child's last set
                //  add all of the states in our right child's first set to the
                //  follow set for that position.
                //
                CMStateSetEnumerator enumLast(&last);
                while(enumLast.hasMoreElements())
                {
                    XMLSize_t index=enumLast.nextElement();
                    *fFollowList[index] |= first;
                }
            }
            retNode = new (fMemoryManager) CMBinaryOp
            (
                curType
                , newLeft
                , newRight
                , fLeafCount
                , fMemoryManager
            );
        }
         else if (curType == ContentSpecNode::ZeroOrMore
               || curType == ContentSpecNode::ZeroOrOne
               || curType == ContentSpecNode::OneOrMore)
        {
            CMNode* newChild = buildSyntaxTree(leftNode, curIndex);
            if (curType == ContentSpecNode::ZeroOrMore
               || curType == ContentSpecNode::OneOrMore)
            {
                //
                //  Now handle our level. We use our own first and last position
                //  sets, so get them up front.
                //
                const CMStateSet& first = newChild->getFirstPos();
                const CMStateSet& last  = newChild->getLastPos();

                //
                //  For every position which is in our last position set, add all
                //  of our first position states to the follow set for that
                //  position.
                //
                CMStateSetEnumerator enumLast(&last);
                while(enumLast.hasMoreElements())
                {
                    XMLSize_t index=enumLast.nextElement();
                    *fFollowList[index] |= first;
                }
            }
            // This one is fine as is, just change to our form
            retNode = new (fMemoryManager) CMUnaryOp
            (
                curType
                , newChild
                , fLeafCount
                , fMemoryManager
            );
        }
         else
        {
            ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::CM_UnknownCMSpecType, fMemoryManager);
        }
    }
    // fault in the first and last pos, then delete it children
    retNode->getFirstPos();
    retNode->getLastPos();
    retNode->orphanChild();
    return retNode;
}

//
//  gInvalidTrans is used to represent bad transitions in the transition table
//  entry for each state. So each entry is initialized to that value. This
//  method creates a new entry and initializes it.
//
unsigned int* DFAContentModel::makeDefStateList() const
{
    unsigned int* retArray = (unsigned int*) fMemoryManager->allocate
    (
        fElemMapSize * sizeof(unsigned int)
    ); //new unsigned int[fElemMapSize];
    for (unsigned int index = 0; index < fElemMapSize; index++)
        retArray[index] = XMLContentModel::gInvalidTrans;
    return retArray;
}

ContentLeafNameTypeVector* DFAContentModel::getContentLeafNameTypeVector() const
{
   //later change it to return the data member
    return fLeafNameTypeVector;
}

void DFAContentModel::checkUniqueParticleAttribution (SchemaGrammar*    const pGrammar,
                                                      GrammarResolver*  const pGrammarResolver,
                                                      XMLStringPool*    const pStringPool,
                                                      XMLValidator*     const pValidator,
                                                      unsigned int*     const pContentSpecOrgURI,
                                                      const XMLCh*            pComplexTypeName /*= 0*/)
{

    SubstitutionGroupComparator comparator(pGrammarResolver, pStringPool);

    unsigned int i, j, k;

    // Rename the URI back
    for (i = 0; i < fElemMapSize; i++) {

        unsigned int orgURIIndex = fElemMap[i]->getURI();

        if ((orgURIIndex != XMLContentModel::gEOCFakeId) &&
            (orgURIIndex != XMLContentModel::gEpsilonFakeId) &&
            (orgURIIndex != XMLElementDecl::fgInvalidElemId) &&
            (orgURIIndex != XMLElementDecl::fgPCDataElemId)) {
            fElemMap[i]->setURI(pContentSpecOrgURI[orgURIIndex]);
        }
    }

    // Unique Particle Attribution
    // Store the conflict results between any two elements in fElemMap
    // 0 - not yet tested, 1 - conflict, (-1) - no conflict
    signed char** conflictTable = (signed char**) fMemoryManager->allocate
    (
        fElemMapSize * sizeof(signed char*)
    );

    // initialize the conflict table
    for (j = 0; j < fElemMapSize; j++) {
        conflictTable[j] = (signed char*) fMemoryManager->allocate
        (
            fElemMapSize * sizeof(signed char)
        );
        memset(conflictTable[j], 0, fElemMapSize*sizeof(signed char));
    }

    // for each state, check whether it has overlap transitions
    for (i = 0; i < fTransTableSize; i++) {
        for (j = 0; j < fElemMapSize; j++) {
            for (k = j+1; k < fElemMapSize; k++) {
                if (fTransTable[i][j] != XMLContentModel::gInvalidTrans &&
                    fTransTable[i][k] != XMLContentModel::gInvalidTrans &&
                    conflictTable[j][k] == 0) {

                    // If this is text in a Schema mixed content model, skip it.
                    if ( fIsMixed &&
                         (( fElemMap[j]->getURI() == XMLElementDecl::fgPCDataElemId) ||
                          ( fElemMap[k]->getURI() == XMLElementDecl::fgPCDataElemId)))
                        continue;

                    if (XercesElementWildcard::conflict(pGrammar,
                                                        fElemMapType[j],
                                                        fElemMap[j],
                                                        fElemMapType[k],
                                                        fElemMap[k],
                                                        &comparator)) {
                        if (fCountingStates != 0) {
                            Occurence* o = fCountingStates[i];
                            // If "i" is a counting state and exactly one of the transitions
                            // loops back to "i" then the two particles do not overlap if
                            // minOccurs == maxOccurs.
                            if (o != 0 &&
                                ((fTransTable[i][j] == i) ^ (fTransTable[i][k] == i)) &&
                                o->minOccurs == o->maxOccurs) {
                                conflictTable[j][k] = -1;
                                continue;
                            }
                        }
                       conflictTable[j][k] = 1;

                       XMLBuffer buf1(1023, fMemoryManager);
                       if (((fElemMapType[j] & 0x0f) == ContentSpecNode::Any) ||
                           ((fElemMapType[j] & 0x0f) == ContentSpecNode::Any_NS))
                           buf1.set(SchemaSymbols::fgATTVAL_TWOPOUNDANY);
                       else if ((fElemMapType[j] & 0x0f) == ContentSpecNode::Any_Other)
                           buf1.set(SchemaSymbols::fgATTVAL_TWOPOUNDOTHER);
                       else
                           buf1.set(fElemMap[j]->getRawName());

                       XMLBuffer buf2(1023, fMemoryManager);
                       if (((fElemMapType[k] & 0x0f) == ContentSpecNode::Any) ||
                           ((fElemMapType[k] & 0x0f) == ContentSpecNode::Any_NS))
                           buf2.set(SchemaSymbols::fgATTVAL_TWOPOUNDANY);
                       else if ((fElemMapType[k] & 0x0f) == ContentSpecNode::Any_Other)
                           buf2.set(SchemaSymbols::fgATTVAL_TWOPOUNDOTHER);
                       else
                           buf2.set(fElemMap[k]->getRawName());

                       pValidator->emitError(XMLValid::UniqueParticleAttributionFail,
                                             pComplexTypeName,
                                             buf1.getRawBuffer(),
                                             buf2.getRawBuffer());
                    }
                    else
                        conflictTable[j][k] = -1;
                }
            }
        }
    }

    for (i = 0; i < fElemMapSize; i++)
        fMemoryManager->deallocate(conflictTable[i]);
    fMemoryManager->deallocate(conflictTable);
}

XERCES_CPP_NAMESPACE_END
