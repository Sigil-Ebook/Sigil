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
 * $Id: XMLContentModel.hpp 677705 2008-07-17 20:15:32Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XMLCONTENTMODEL_HPP)
#define XERCESC_INCLUDE_GUARD_XMLCONTENTMODEL_HPP

#include <xercesc/util/XMemory.hpp>
#include <xercesc/util/QName.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class ContentLeafNameTypeVector;
class GrammarResolver;
class XMLStringPool;
class XMLValidator;
class SchemaGrammar;
class SubstitutionGroupComparator;

/**
 *  This class defines the abstract interface for all content models. All
 *  elements have a content model against which (if validating) its content
 *  is checked. Each type of validator (DTD, Schema, etc...) can have
 *  different types of content models, and even with each type of validator
 *  there can be specialized content models. So this simple class provides
 *  the abstract API via which all the types of contents models are dealt
 *  with generically. Its pretty simple.
 */
class XMLPARSER_EXPORT XMLContentModel : public XMemory
{
public:
    // ---------------------------------------------------------------------------
    //  Public static data
    //
    //  gInvalidTrans
    //      This value represents an invalid transition in each line of the
    //      transition table.
    //
    //  gEOCFakeId
    //  gEpsilonFakeId
    //      We have to put in a couple of special CMLeaf nodes to represent
    //      special values, using fake element ids that we know won't conflict
    //      with real element ids.
    //
    //
    // ---------------------------------------------------------------------------
    static const unsigned int   gInvalidTrans;
    static const unsigned int   gEOCFakeId;
    static const unsigned int   gEpsilonFakeId;

    // -----------------------------------------------------------------------
    //  Constructors are hidden, only the virtual Destructor is exposed
    // -----------------------------------------------------------------------
    /** @name Destructor */
    //@{
    virtual ~XMLContentModel()
    {
    }
    //@}


    // -----------------------------------------------------------------------
    //  The virtual content model interface provided by derived classes
    // -----------------------------------------------------------------------
	virtual bool validateContent
    (
        QName** const         children
      , XMLSize_t             childCount
      , unsigned int          emptyNamespaceId
      , XMLSize_t*            indexFailingChild
      , MemoryManager*  const manager = XMLPlatformUtils::fgMemoryManager
    ) const = 0;

	virtual bool validateContentSpecial
    (
        QName** const           children
      , XMLSize_t               childCount
      , unsigned int            emptyNamespaceId
      , GrammarResolver*  const pGrammarResolver
      , XMLStringPool*    const pStringPool
      , XMLSize_t*              indexFailingChild
      , MemoryManager*    const manager = XMLPlatformUtils::fgMemoryManager
    ) const =0;

	virtual void checkUniqueParticleAttribution
    (
        SchemaGrammar*    const pGrammar
      , GrammarResolver*  const pGrammarResolver
      , XMLStringPool*    const pStringPool
      , XMLValidator*     const pValidator
      , unsigned int*     const pContentSpecOrgURI
      , const XMLCh*            pComplexTypeName = 0
    ) =0;

    virtual ContentLeafNameTypeVector* getContentLeafNameTypeVector()
	  const = 0;

    virtual unsigned int getNextState(unsigned int currentState,
                                      XMLSize_t    elementIndex) const = 0;

    virtual bool handleRepetitions( const QName* const curElem,
                                    unsigned int curState,
                                    unsigned int currentLoop,
                                    unsigned int& nextState,
                                    unsigned int& nextLoop,
                                    XMLSize_t elementIndex,
                                    SubstitutionGroupComparator * comparator) const = 0;

protected :
    // -----------------------------------------------------------------------
    //  Hidden Constructors
    // -----------------------------------------------------------------------
    XMLContentModel()
    {
    }


private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XMLContentModel(const XMLContentModel&);
    XMLContentModel& operator=(const XMLContentModel&);
};

XERCES_CPP_NAMESPACE_END

#endif
