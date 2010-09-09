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
 * $Id: DTDGrammar.hpp 883368 2009-11-23 15:28:19Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DTDGRAMMAR_HPP)
#define XERCESC_INCLUDE_GUARD_DTDGRAMMAR_HPP

#include <xercesc/util/RefHashTableOf.hpp>
#include <xercesc/util/NameIdPool.hpp>
#include <xercesc/util/StringPool.hpp>
#include <xercesc/validators/common/Grammar.hpp>
#include <xercesc/validators/DTD/DTDElementDecl.hpp>
#include <xercesc/validators/DTD/DTDEntityDecl.hpp>
#include <xercesc/framework/XMLDTDDescription.hpp>


XERCES_CPP_NAMESPACE_BEGIN

//
// This class stores the DTD information
//  NOTE: DTDs are not namespace aware, so we just use regular NameIdPool
//  data structures to store element and attribute decls. They are all set
//  to be in the global namespace and the full QName is used as the base name
//  of the decl. This means that all the URI parameters below are expected
//  to be null pointers (and anything else will cause an exception.)
//

class VALIDATORS_EXPORT DTDGrammar : public Grammar
{
public:
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    DTDGrammar(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    virtual ~DTDGrammar();

    // -----------------------------------------------------------------------
    //  Implementation of Virtual Interface
    // -----------------------------------------------------------------------
    virtual Grammar::GrammarType getGrammarType() const;
    virtual const XMLCh* getTargetNamespace() const;

    // this method should only be used while the grammar is being
    // constructed, not while it is being used
    // in a validation episode!
    virtual XMLElementDecl* findOrAddElemDecl
    (
        const   unsigned int    uriId
        , const XMLCh* const    baseName
        , const XMLCh* const    prefixName
        , const XMLCh* const    qName
        , unsigned int          scope
        ,       bool&           wasAdded
    ) ;

    virtual XMLSize_t getElemId
    (
        const   unsigned int    uriId
        , const XMLCh* const    baseName
        , const XMLCh* const    qName
        , unsigned int          scope
    )   const ;

    virtual const XMLElementDecl* getElemDecl
    (
        const   unsigned int    uriId
        , const XMLCh* const    baseName
        , const XMLCh* const    qName
        , unsigned int          scope
    )   const ;

    virtual XMLElementDecl* getElemDecl
    (
        const   unsigned int    uriId
        , const XMLCh* const    baseName
        , const XMLCh* const    qName
        , unsigned int          scope
    );

    virtual const XMLElementDecl* getElemDecl
    (
        const   unsigned int    elemId
    )   const;

    virtual XMLElementDecl* getElemDecl
    (
        const   unsigned int    elemId
    );

    virtual const XMLNotationDecl* getNotationDecl
    (
        const   XMLCh* const    notName
    )   const;

    virtual XMLNotationDecl* getNotationDecl
    (
        const   XMLCh* const    notName
    );

    virtual bool getValidated() const;

    virtual XMLElementDecl* putElemDecl
    (
        const   unsigned int    uriId
        , const XMLCh* const    baseName
        , const XMLCh* const    prefixName
        , const XMLCh* const    qName
        , unsigned int          scope
        , const bool            notDeclared = false
    );

    virtual XMLSize_t putElemDecl
    (
        XMLElementDecl* const elemDecl
        , const bool          notDeclared = false
    )   ;

    virtual XMLSize_t putNotationDecl
    (
        XMLNotationDecl* const notationDecl
    )   const;

    virtual void setValidated(const bool newState);

    virtual void reset();

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------

    const DTDEntityDecl* getEntityDecl(const XMLCh* const entName) const;
    DTDEntityDecl* getEntityDecl(const XMLCh* const entName);
    NameIdPool<DTDEntityDecl>* getEntityDeclPool();
    const NameIdPool<DTDEntityDecl>* getEntityDeclPool() const;
    NameIdPoolEnumerator<DTDElementDecl> getElemEnumerator() const;
    NameIdPoolEnumerator<DTDEntityDecl> getEntityEnumerator() const;
    NameIdPoolEnumerator<XMLNotationDecl> getNotationEnumerator() const;

    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------

    virtual void                    setGrammarDescription( XMLGrammarDescription*);
    virtual XMLGrammarDescription*  getGrammarDescription() const;

    // -----------------------------------------------------------------------
    //  Content management methods
    // -----------------------------------------------------------------------
    XMLSize_t putEntityDecl(DTDEntityDecl* const entityDecl) const;

    /***
     * Support for Serialization/De-serialization
     ***/
    DECL_XSERIALIZABLE(DTDGrammar)

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    DTDGrammar(const DTDGrammar &);
    DTDGrammar& operator = (const  DTDGrammar&);

    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fElemDeclPool
    //      This is the element decl pool. It contains all of the elements
    //      declared in the DTD (and their associated attributes.)
    //
    //  fElemNonDeclPool
    //      This is the element decl pool that is is populated as new elements
    //      are seen in the XML document (not declared in the DTD), and they
    //      are given default characteristics.
    //
    //  fEntityDeclPool
    //      This is a pool of EntityDecl objects, which contains all of the
    //      general entities that are declared in the DTD subsets, plus the
    //      default entities (such as &gt; &lt; ...) defined by the XML Standard.
    //
    //  fNotationDeclPool
    //      This is a pool of NotationDecl objects, which contains all of the
    //      notations declared in the DTD subsets.
    //
    //  fValidated
    //      Indicates if the content of the Grammar has been pre-validated
    //      or not. When using a cached grammar, no need for pre content
    //      validation.
    //
    //  fGramDesc: adopted
    //
    // -----------------------------------------------------------------------
    static NameIdPool<DTDEntityDecl>* fDefaultEntities;
    MemoryManager*                    fMemoryManager;
    NameIdPool<DTDElementDecl>*       fElemDeclPool;
    NameIdPool<DTDElementDecl>*       fElemNonDeclPool;
    NameIdPool<DTDEntityDecl>*        fEntityDeclPool;
    NameIdPool<XMLNotationDecl>*      fNotationDeclPool;
    XMLDTDDescription*                fGramDesc;

    bool                              fValidated;

    friend class XMLInitializer;
};

// ---------------------------------------------------------------------------
//  DTDGrammar: Getter methods
// ---------------------------------------------------------------------------
inline NameIdPoolEnumerator<DTDElementDecl>
DTDGrammar::getElemEnumerator() const
{
    return NameIdPoolEnumerator<DTDElementDecl>(fElemDeclPool, fMemoryManager);
}

inline NameIdPoolEnumerator<DTDEntityDecl>
DTDGrammar::getEntityEnumerator() const
{
    return NameIdPoolEnumerator<DTDEntityDecl>(fEntityDeclPool, fMemoryManager);
}

inline NameIdPoolEnumerator<XMLNotationDecl>
DTDGrammar::getNotationEnumerator() const
{
    return NameIdPoolEnumerator<XMLNotationDecl>(fNotationDeclPool, fMemoryManager);
}

inline const DTDEntityDecl*
DTDGrammar::getEntityDecl(const XMLCh* const entName) const
{
    DTDEntityDecl* decl = fDefaultEntities->getByKey(entName);

    if (!decl)
        return fEntityDeclPool->getByKey(entName);

    return decl;
}

inline DTDEntityDecl* DTDGrammar::getEntityDecl(const XMLCh* const entName)
{
    DTDEntityDecl* decl = fDefaultEntities->getByKey(entName);

    if (!decl)
        return fEntityDeclPool->getByKey(entName);

    return decl;
}


inline NameIdPool<DTDEntityDecl>* DTDGrammar::getEntityDeclPool()
{
    return fEntityDeclPool;
}

inline const NameIdPool<DTDEntityDecl>* DTDGrammar::getEntityDeclPool() const
{
    return fEntityDeclPool;
}

// -----------------------------------------------------------------------
//  Setter methods
// -----------------------------------------------------------------------
inline XMLSize_t DTDGrammar::putEntityDecl(DTDEntityDecl* const entityDecl)   const
{
    return fEntityDeclPool->put(entityDecl);
}


// ---------------------------------------------------------------------------
//  DTDGrammar: Virtual methods
// ---------------------------------------------------------------------------
inline Grammar::GrammarType DTDGrammar::getGrammarType() const {
    return Grammar::DTDGrammarType;
}

inline const XMLCh* DTDGrammar::getTargetNamespace() const {
    return XMLUni::fgZeroLenString;
}

// Element Decl
inline XMLSize_t DTDGrammar::getElemId (const   unsigned int
                                              , const XMLCh* const
                                              , const XMLCh* const    qName
                                              , unsigned int) const
{
    //
    //  In this case, we don't return zero to mean 'not found', so we have to
    //  map it to the official not found value if we don't find it.
    //
    const DTDElementDecl* decl = fElemDeclPool->getByKey(qName);
    if (!decl)
        return XMLElementDecl::fgInvalidElemId;
    return decl->getId();
}

inline const XMLElementDecl* DTDGrammar::getElemDecl( const   unsigned int
                                              , const XMLCh* const
                                              , const XMLCh* const    qName
                                              , unsigned int)   const
{
    const XMLElementDecl* elemDecl = fElemDeclPool->getByKey(qName);

    if (!elemDecl && fElemNonDeclPool)
        elemDecl = fElemNonDeclPool->getByKey(qName);

    return elemDecl;
}

inline XMLElementDecl* DTDGrammar::getElemDecl (const   unsigned int
                                              , const XMLCh* const
                                              , const XMLCh* const    qName
                                              , unsigned int)
{
    XMLElementDecl* elemDecl = fElemDeclPool->getByKey(qName);

    if (!elemDecl && fElemNonDeclPool)
        elemDecl = fElemNonDeclPool->getByKey(qName);

    return elemDecl;
}

inline const XMLElementDecl* DTDGrammar::getElemDecl(const unsigned int elemId) const
{
    // Look up this element decl by id
    return fElemDeclPool->getById(elemId);
}

inline XMLElementDecl* DTDGrammar::getElemDecl(const unsigned int elemId)
{
    // Look up this element decl by id
    return fElemDeclPool->getById(elemId);
}

inline XMLSize_t
DTDGrammar::putElemDecl(XMLElementDecl* const elemDecl,
                        const bool notDeclared)
{
    if (notDeclared)
    {
        if(!fElemNonDeclPool)
            fElemNonDeclPool = new (fMemoryManager) NameIdPool<DTDElementDecl>(29, 128, fMemoryManager);
        return fElemNonDeclPool->put((DTDElementDecl*) elemDecl);
    }

    return fElemDeclPool->put((DTDElementDecl*) elemDecl);
}

// Notation Decl
inline const XMLNotationDecl* DTDGrammar::getNotationDecl(const XMLCh* const notName) const
{
    return fNotationDeclPool->getByKey(notName);
}

inline XMLNotationDecl* DTDGrammar::getNotationDecl(const XMLCh* const notName)
{
    return fNotationDeclPool->getByKey(notName);
}

inline XMLSize_t DTDGrammar::putNotationDecl(XMLNotationDecl* const notationDecl)   const
{
    return fNotationDeclPool->put(notationDecl);
}

inline bool DTDGrammar::getValidated() const
{
    return fValidated;
}

inline void DTDGrammar::setValidated(const bool newState)
{
    fValidated = newState;
}

XERCES_CPP_NAMESPACE_END

#endif
