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
 * $Id: DFAContentModel.hpp 677705 2008-07-17 20:15:32Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DFACONTENTMODEL_HPP)
#define XERCESC_INCLUDE_GUARD_DFACONTENTMODEL_HPP

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/util/ArrayIndexOutOfBoundsException.hpp>
#include <xercesc/framework/XMLContentModel.hpp>
#include <xercesc/validators/common/ContentLeafNameTypeVector.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class ContentSpecNode;
class CMLeaf;
class CMRepeatingLeaf;
class CMNode;
class CMStateSet;

//
//  DFAContentModel is the heavy weight derivative of ContentModel that does
//  all of the non-trivial element content validation. This guy does the full
//  bore regular expression to DFA conversion to create a DFA that it then
//  uses in its validation algorithm.
//
//  NOTE:   Upstream work insures that this guy will never see a content model
//          with PCDATA in it. Any model with PCDATA is 'mixed' and is handled
//          via the MixedContentModel class, since mixed models are very
//          constrained in form and easily handled via a special case. This
//          also makes our life much easier here.
//
class DFAContentModel : public XMLContentModel
{
public:
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    DFAContentModel
    ( 
          const bool             dtd
        , ContentSpecNode* const elemContentSpec
        , MemoryManager* const   manager = XMLPlatformUtils::fgMemoryManager
    );
    DFAContentModel
    (
          const bool             dtd
        , ContentSpecNode* const elemContentSpec
        , const bool             isMixed
        , MemoryManager* const   manager
    );

    virtual ~DFAContentModel();


    // -----------------------------------------------------------------------
    //  Implementation of the virtual content model interface
    // -----------------------------------------------------------------------
    virtual bool validateContent
    (
        QName** const         children
      , XMLSize_t             childCount
      , unsigned int          emptyNamespaceId
      , XMLSize_t*            indexFailingChild
      , MemoryManager*  const manager = XMLPlatformUtils::fgMemoryManager
    ) const;

    virtual bool validateContentSpecial
    (
        QName** const           children
      , XMLSize_t               childCount
      , unsigned int            emptyNamespaceId
      , GrammarResolver*  const pGrammarResolver
      , XMLStringPool*    const pStringPool
      , XMLSize_t*              indexFailingChild
      , MemoryManager*    const manager = XMLPlatformUtils::fgMemoryManager
    ) const;

    virtual void checkUniqueParticleAttribution
    (
        SchemaGrammar*    const pGrammar
      , GrammarResolver*  const pGrammarResolver
      , XMLStringPool*    const pStringPool
      , XMLValidator*     const pValidator
      , unsigned int*     const pContentSpecOrgURI
      , const XMLCh*            pComplexTypeName = 0
    ) ;

    virtual ContentLeafNameTypeVector* getContentLeafNameTypeVector() const ;

    virtual unsigned int getNextState(unsigned int currentState,
                                      XMLSize_t    elementIndex) const;

    virtual bool handleRepetitions( const QName* const curElem,
                                    unsigned int curState,
                                    unsigned int currentLoop,
                                    unsigned int& nextState,
                                    unsigned int& nextLoop,
                                    XMLSize_t elementIndex,
                                    SubstitutionGroupComparator * comparator) const;

private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    DFAContentModel();
    DFAContentModel(const DFAContentModel&);
    DFAContentModel& operator=(const DFAContentModel&);


    // -----------------------------------------------------------------------
    //  Private helper methods
    // -----------------------------------------------------------------------
    void buildDFA(ContentSpecNode* const curNode);
    CMNode* buildSyntaxTree(ContentSpecNode* const curNode, unsigned int& curIndex);
    unsigned int* makeDefStateList() const;
    unsigned int countLeafNodes(ContentSpecNode* const curNode);

    class Occurence : public XMemory
    {
    public:
        Occurence(int minOcc, int maxOcc, int eltIndex);

        int minOccurs;
        int maxOccurs;
        int elemIndex;
    };

    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fElemMap
    //  fElemMapSize
    //      This is the map of unique input symbol elements to indices into
    //      each state's per-input symbol transition table entry. This is part
    //      of the built DFA information that must be kept around to do the
    //      actual validation.
    //
    //  fElemMapType
    //      This is a map of whether the element map contains information
    //      related to ANY models.
    //
    //  fEmptyOk
    //      This is an optimization. While building the transition table we
    //      can see whether this content model would approve of an empty
    //      content (which could happen if everything was optional.) So we
    //      set this flag and short circuit that check, which would otherwise
    //      be ugly and time consuming if we tried to determine it at each
    //      validation call.
    //
    //  fEOCPos
    //      The NFA position of the special EOC (end of content) node. This
    //      is saved away since its used during the DFA build.
    //
    //  fFinalStateFlags
    //      This is an array of booleans, one per state (there are
    //      fTransTableSize states in the DFA) that indicates whether that
    //      state is a final state.
    //
    //  fFollowList
    //      The list of follow positions for each NFA position (i.e. for each
    //      non-epsilon leaf node.) This is only used during the building of
    //      the DFA, and is let go afterwards.
    //
    //  fHeadNode
    //      This is the head node of our intermediate representation. It is
    //      only non-null during the building of the DFA (just so that it
    //      does not have to be passed all around.) Once the DFA is built,
    //      this is no longer required so its deleted.
    //
    //  fLeafCount
    //      The count of leaf nodes. This is an important number that set some
    //      limits on the sizes of data structures in the DFA process.
    //
    //  fLeafList
    //      An array of non-epsilon leaf nodes, which is used during the DFA
    //      build operation, then dropped. These are just references to nodes
    //      pointed to by fHeadNode, so we don't have to clean them up, just
    //      the actually leaf list array itself needs cleanup.
    //
    //  fLeafListType
    //      Array mapping ANY types to the leaf list.
    //
    //  fTransTable
    //  fTransTableSize
    //      This is the transition table that is the main by product of all
    //      of the effort here. It is an array of arrays of ints. The first
    //      dimension is the number of states we end up with in the DFA. The
    //      second dimensions is the number of unique elements in the content
    //      model (fElemMapSize). Each entry in the second dimension indicates
    //      the new state given that input for the first dimension's start
    //      state.
    //
    //      The fElemMap array handles mapping from element indexes to
    //      positions in the second dimension of the transition table.
    //
    //      fTransTableSize is the number of valid entries in the transition
    //      table, and in the other related tables such as fFinalStateFlags.
    //
    //  fCountingStates
    //      This is the table holding the minOccurs/maxOccurs for elements
    //      that can be repeated a finite number of times.
    //
    //  fDTD
    //      Boolean to allow DTDs to validate even with namespace support.
    //
    //  fIsMixed
    //      DFA ContentModel with mixed PCDATA.
    // -----------------------------------------------------------------------
    QName**                     fElemMap;
    ContentSpecNode::NodeTypes* fElemMapType;
    unsigned int                fElemMapSize;
    bool                        fEmptyOk;
    unsigned int                fEOCPos;
    bool*                       fFinalStateFlags;
    CMStateSet**                fFollowList;
    CMNode*                     fHeadNode;
    unsigned int                fLeafCount;
    CMLeaf**                    fLeafList;
    ContentSpecNode::NodeTypes* fLeafListType;
    unsigned int**              fTransTable;
    unsigned int                fTransTableSize;
    Occurence**                 fCountingStates;
    bool                        fDTD;
    bool                        fIsMixed;
    ContentLeafNameTypeVector * fLeafNameTypeVector;
    MemoryManager*              fMemoryManager;
};


inline unsigned int
DFAContentModel::getNextState(unsigned int currentState,
                              XMLSize_t    elementIndex) const {

    if (currentState == XMLContentModel::gInvalidTrans) {
        return XMLContentModel::gInvalidTrans;
    }

    if (currentState >= fTransTableSize || elementIndex >= fElemMapSize) {
        ThrowXMLwithMemMgr(ArrayIndexOutOfBoundsException, XMLExcepts::Array_BadIndex, fMemoryManager);
    }

    return fTransTable[currentState][elementIndex];
}

inline
DFAContentModel::Occurence::Occurence(int minOcc, int maxOcc, int eltIndex)
{
    minOccurs = minOcc;
    maxOccurs = maxOcc;
    elemIndex = eltIndex;
}

XERCES_CPP_NAMESPACE_END

#endif

