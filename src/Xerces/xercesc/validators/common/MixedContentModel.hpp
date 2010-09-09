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
 * $Id: MixedContentModel.hpp 901107 2010-01-20 08:45:02Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_MIXEDCONTENTMODEL_HPP)
#define XERCESC_INCLUDE_GUARD_MIXEDCONTENTMODEL_HPP

#include <xercesc/util/ValueVectorOf.hpp>
#include <xercesc/framework/XMLContentModel.hpp>
#include <xercesc/validators/common/ContentLeafNameTypeVector.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class ContentSpecNode;

//
//  MixedContentModel is a derivative of the abstract content model base
//  class that handles the special case of mixed model elements. If an element
//  is mixed model, it has PCDATA as its first possible content, followed
//  by an alternation of the possible children. The children cannot have any
//  numeration or order, so it must look like this:
//
//  <!ELEMENT Foo ((#PCDATA|a|b|c|)*)>
//
//  So, all we have to do is to keep an array of the possible children and
//  validate by just looking up each child being validated by looking it up
//  in the list.
//
class MixedContentModel : public XMLContentModel
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    MixedContentModel
    (
        const bool                dtd
        , ContentSpecNode* const  parentContentSpec
		, const bool              ordered = false
        , MemoryManager* const    manager = XMLPlatformUtils::fgMemoryManager
    );

    ~MixedContentModel();


    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    bool hasDups() const;

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
    )   const;

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
    //  Private helper methods
    // -----------------------------------------------------------------------
    void buildChildList
    (
        ContentSpecNode* const                     curNode
      , ValueVectorOf<QName*>&                     toFill
      , ValueVectorOf<ContentSpecNode::NodeTypes>& toType
    );

    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    MixedContentModel();
    MixedContentModel(const MixedContentModel&);
    MixedContentModel& operator=(const MixedContentModel&);


    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fCount
    //      The count of possible children in the fChildren member.
    //
    //  fChildren
    //      The list of possible children that we have to accept. This array
    //      is allocated as large as needed in the constructor.
    //
    //  fChildTypes
    //      The type of the children to support ANY.
    //
    //  fOrdered
    //      True if mixed content model is ordered. DTD mixed content models
    //      are <em>always</em> unordered.
    //
    //  fDTD
    //      Boolean to allow DTDs to validate even with namespace support.
    //
    // -----------------------------------------------------------------------
    XMLSize_t                   fCount;
    QName**                     fChildren;
    ContentSpecNode::NodeTypes* fChildTypes;
    bool                        fOrdered;
    bool                        fDTD;
    MemoryManager*              fMemoryManager;
};

inline ContentLeafNameTypeVector* MixedContentModel::getContentLeafNameTypeVector() const
{
	return 0;
}

inline unsigned int
MixedContentModel::getNextState(unsigned int,
                                XMLSize_t) const {

    return XMLContentModel::gInvalidTrans;
}

inline bool
MixedContentModel::handleRepetitions( const QName* const /*curElem*/,
                                      unsigned int /*curState*/,
                                      unsigned int /*currentLoop*/,
                                      unsigned int& /*nextState*/,
                                      unsigned int& /*nextLoop*/,
                                      XMLSize_t /*elementIndex*/,
                                      SubstitutionGroupComparator * /*comparator*/) const
{
    return true;
}

inline void MixedContentModel::checkUniqueParticleAttribution
    (
        SchemaGrammar*    const
      , GrammarResolver*  const
      , XMLStringPool*    const
      , XMLValidator*     const
      , unsigned int*     const pContentSpecOrgURI
      , const XMLCh*            /*pComplexTypeName*/ /*= 0*/
    )
{
    // rename back
    unsigned int i = 0;
    for (i = 0; i < fCount; i++) {
        unsigned int orgURIIndex = fChildren[i]->getURI();
        if ((orgURIIndex != XMLContentModel::gEOCFakeId) &&
            (orgURIIndex != XMLElementDecl::fgInvalidElemId) &&
            (orgURIIndex != XMLElementDecl::fgPCDataElemId))
            fChildren[i]->setURI(pContentSpecOrgURI[orgURIIndex]);
    }

    // for mixed content model, it's only a sequence
    // UPA checking is not necessary
}

XERCES_CPP_NAMESPACE_END

#endif
