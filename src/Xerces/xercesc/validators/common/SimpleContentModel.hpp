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
 * $Id: SimpleContentModel.hpp 901107 2010-01-20 08:45:02Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_SIMPLECONTENTMODEL_HPP)
#define XERCESC_INCLUDE_GUARD_SIMPLECONTENTMODEL_HPP

#include <xercesc/framework/XMLContentModel.hpp>
#include <xercesc/validators/common/ContentSpecNode.hpp>

XERCES_CPP_NAMESPACE_BEGIN

//
//  SimpleContentModel is a derivative of the abstract content model base
//  class that handles a small set of simple content models that are just
//  way overkill to give the DFA treatment.
//
//  DESCRIPTION:
//
//  This guy handles the following scenarios:
//
//      a
//      a?
//      a*
//      a+
//      a,b
//      a|b
//
//  These all involve a unary operation with one element type, or a binary
//  operation with two elements. These are very simple and can be checked
//  in a simple way without a DFA and without the overhead of setting up a
//  DFA for such a simple check.
//
//  NOTE:   Pass the XMLElementDecl::fgPCDataElemId value to represent a
//          PCData node. Pass XMLElementDecl::fgInvalidElemId for unused element
//
class SimpleContentModel : public XMLContentModel
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    SimpleContentModel
    (
        const bool                        dtd
      , QName* const                      firstChild
      , QName* const                      secondChild
      , const ContentSpecNode::NodeTypes  cmOp
      , MemoryManager* const              manager = XMLPlatformUtils::fgMemoryManager
    );

    ~SimpleContentModel();


    // -----------------------------------------------------------------------
    //  Implementation of the ContentModel virtual interface
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

    virtual ContentLeafNameTypeVector *getContentLeafNameTypeVector() const;

    virtual unsigned int getNextState(unsigned int currentState,
                                      XMLSize_t    elementIndex) const;

    virtual bool handleRepetitions( const QName* const curElem,
                                    unsigned int curState,
                                    unsigned int currentLoop,
                                    unsigned int& nextState,
                                    unsigned int& nextLoop,
                                    XMLSize_t elementIndex,
                                    SubstitutionGroupComparator * comparator) const;

    virtual void checkUniqueParticleAttribution
    (
        SchemaGrammar*    const pGrammar
      , GrammarResolver*  const pGrammarResolver
      , XMLStringPool*    const pStringPool
      , XMLValidator*     const pValidator
      , unsigned int*     const pContentSpecOrgURI
      , const XMLCh*            pComplexTypeName = 0
    ) ;

 private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    SimpleContentModel();
    SimpleContentModel(const SimpleContentModel&);
    SimpleContentModel& operator=(const SimpleContentModel&);


    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fFirstChild
    //  fSecondChild
    //      The first (and optional second) child node. The
    //      operation code tells us whether the second child is used or not.
    //
    //  fOp
    //      The operation that this object represents. Since this class only
    //      does simple contents, there is only ever a single operation
    //      involved (i.e. the children of the operation are always one or
    //      two leafs.)
    //
    //  fDTD
    //      Boolean to allow DTDs to validate even with namespace support. */
    //
    // -----------------------------------------------------------------------
    QName*                     fFirstChild;
    QName*                     fSecondChild;
    ContentSpecNode::NodeTypes fOp;
    bool                       fDTD;
    MemoryManager* const       fMemoryManager;
};


// ---------------------------------------------------------------------------
//  SimpleContentModel: Constructors and Destructor
// ---------------------------------------------------------------------------
inline SimpleContentModel::SimpleContentModel
(
      const bool                       dtd
    , QName* const                     firstChild
    , QName* const                     secondChild
    , const ContentSpecNode::NodeTypes cmOp
     , MemoryManager* const            manager
)
    : fFirstChild(0)
    , fSecondChild(0)
    , fOp(cmOp)
    , fDTD(dtd)
    , fMemoryManager(manager)
{
    if (firstChild)
        fFirstChild = new (manager) QName(*firstChild);
    else
        fFirstChild = new (manager) QName(XMLUni::fgZeroLenString, XMLUni::fgZeroLenString, XMLElementDecl::fgInvalidElemId, manager);

    if (secondChild)
        fSecondChild = new (manager) QName(*secondChild);
    else
        fSecondChild = new (manager) QName(XMLUni::fgZeroLenString, XMLUni::fgZeroLenString, XMLElementDecl::fgInvalidElemId, manager);
}

inline SimpleContentModel::~SimpleContentModel()
{
    delete fFirstChild;
    delete fSecondChild;
}


// ---------------------------------------------------------------------------
//  SimpleContentModel: Virtual methods
// ---------------------------------------------------------------------------
inline unsigned int
SimpleContentModel::getNextState(unsigned int,
                                 XMLSize_t) const {

    return XMLContentModel::gInvalidTrans;
}

inline bool
SimpleContentModel::handleRepetitions( const QName* const /*curElem*/,
                                       unsigned int /*curState*/,
                                       unsigned int /*currentLoop*/,
                                       unsigned int& /*nextState*/,
                                       unsigned int& /*nextLoop*/,
                                       XMLSize_t /*elementIndex*/,
                                       SubstitutionGroupComparator * /*comparator*/) const
{
    return true;
}

XERCES_CPP_NAMESPACE_END

#endif
