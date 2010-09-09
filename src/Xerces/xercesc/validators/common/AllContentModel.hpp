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
 * $Id: AllContentModel.hpp 901107 2010-01-20 08:45:02Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_ALLCONTENTMODEL_HPP)
#define XERCESC_INCLUDE_GUARD_ALLCONTENTMODEL_HPP

#include <xercesc/framework/XMLContentModel.hpp>
#include <xercesc/util/ValueVectorOf.hpp>
#include <xercesc/validators/common/ContentLeafNameTypeVector.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class ContentSpecNode;

//
//  AllContentModel is a derivative of the abstract content model base
//  class that handles the special case of <all> feature in schema. If a model
//  is <all>, all non-optional children must appear
//
//  So, all we have to do is to keep an array of the possible children and
//  validate by just looking up each child being validated by looking it up
//  in the list, and make sure all non-optional children appear.
//
class AllContentModel : public XMLContentModel
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    AllContentModel
    (
          ContentSpecNode* const parentContentSpec
		, const bool             isMixed
        , MemoryManager* const   manager = XMLPlatformUtils::fgMemoryManager
    );

    ~AllContentModel();

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
      , ValueVectorOf<bool>&                       toType
    );

    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    AllContentModel();
    AllContentModel(const AllContentModel&);
    AllContentModel& operator=(const AllContentModel&);


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
    //  fChildOptional
    //      The corresponding list of optional state of each child in fChildren
    //      True if the child is optional (i.e. minOccurs = 0).
    //
    //  fNumRequired
    //      The number of required children in <all> (i.e. minOccurs = 1)
    //
    //  fIsMixed
    //      AllContentModel with mixed PCDATA.
    // -----------------------------------------------------------------------
    MemoryManager* fMemoryManager;
    XMLSize_t       fCount;
    QName**         fChildren;
    bool*           fChildOptional;
    unsigned int    fNumRequired;
    bool            fIsMixed;
    bool            fHasOptionalContent;
};

inline ContentLeafNameTypeVector* AllContentModel::getContentLeafNameTypeVector() const
{
	return 0;
}

inline unsigned int
AllContentModel::getNextState(unsigned int,
                              XMLSize_t) const {

    return XMLContentModel::gInvalidTrans;
}

inline bool
AllContentModel::handleRepetitions( const QName* const /*curElem*/,
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
