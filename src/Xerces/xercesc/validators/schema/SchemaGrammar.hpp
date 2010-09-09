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
 * $Id: SchemaGrammar.hpp 883376 2009-11-23 15:45:23Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_SCHEMAGRAMMAR_HPP)
#define XERCESC_INCLUDE_GUARD_SCHEMAGRAMMAR_HPP

#include <xercesc/framework/XMLNotationDecl.hpp>
#include <xercesc/util/RefHash3KeysIdPool.hpp>
#include <xercesc/util/NameIdPool.hpp>
#include <xercesc/util/StringPool.hpp>
#include <xercesc/validators/common/Grammar.hpp>
#include <xercesc/validators/schema/SchemaElementDecl.hpp>
#include <xercesc/util/ValueVectorOf.hpp>
#include <xercesc/validators/datatype/IDDatatypeValidator.hpp>
#include <xercesc/validators/datatype/DatatypeValidatorFactory.hpp>
#include <xercesc/framework/XMLSchemaDescription.hpp>
#include <xercesc/framework/ValidationContext.hpp>

XERCES_CPP_NAMESPACE_BEGIN

//
// This class stores the Schema information
//  NOTE: Schemas are not namespace aware, so we just use regular NameIdPool
//  data structures to store element and attribute decls. They are all set
//  to be in the global namespace and the full QName is used as the base name
//  of the decl. This means that all the URI parameters below are expected
//  to be null pointers (and anything else will cause an exception.)
//

// ---------------------------------------------------------------------------
//  Forward Declarations
// ---------------------------------------------------------------------------
class ComplexTypeInfo;
class XercesGroupInfo;
class XercesAttGroupInfo;
class XSAnnotation;

// ---------------------------------------------------------------------------
//  typedef declaration
// ---------------------------------------------------------------------------
typedef ValueVectorOf<SchemaElementDecl*> ElemVector;


class VALIDATORS_EXPORT SchemaGrammar : public Grammar
{
public:
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    SchemaGrammar(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    virtual ~SchemaGrammar();

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
    RefHash3KeysIdPoolEnumerator<SchemaElementDecl> getElemEnumerator() const;
    NameIdPoolEnumerator<XMLNotationDecl> getNotationEnumerator() const;
    RefHashTableOf<XMLAttDef>* getAttributeDeclRegistry() const;
    RefHashTableOf<ComplexTypeInfo>* getComplexTypeRegistry() const;
    RefHashTableOf<XercesGroupInfo>* getGroupInfoRegistry() const;
    RefHashTableOf<XercesAttGroupInfo>* getAttGroupInfoRegistry() const;
    DatatypeValidatorFactory* getDatatypeRegistry();
    RefHash2KeysTableOf<ElemVector>* getValidSubstitutionGroups() const;

    // @deprecated
    ValidationContext*          getValidationContext() const;

    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------
    void setTargetNamespace(const XMLCh* const targetNamespace);
    void setAttributeDeclRegistry(RefHashTableOf<XMLAttDef>* const attReg);
    void setComplexTypeRegistry(RefHashTableOf<ComplexTypeInfo>* const other);
    void setGroupInfoRegistry(RefHashTableOf<XercesGroupInfo>* const other);
    void setAttGroupInfoRegistry(RefHashTableOf<XercesAttGroupInfo>* const other);
    void setValidSubstitutionGroups(RefHash2KeysTableOf<ElemVector>* const);

    virtual void                    setGrammarDescription( XMLGrammarDescription*);
    virtual XMLGrammarDescription*  getGrammarDescription() const;

    // -----------------------------------------------------------------------
    //  Helper methods
    // -----------------------------------------------------------------------
    XMLSize_t putGroupElemDecl
    (
        XMLElementDecl* const elemDecl
    )   const;

    // -----------------------------------------------------------------------
    // Annotation management methods
    // -----------------------------------------------------------------------
    /**
      * Add annotation to the list of annotations for a given key
      */
    void putAnnotation(void* key, XSAnnotation* const annotation);

    /**
      * Add global annotation
      *
      * Note: XSAnnotation acts as a linked list
      */
    void addAnnotation(XSAnnotation* const annotation);

    /**
     * Retrieve the annotation that is associated with the specified key
     *
     * @param  key   represents a schema component object (i.e. SchemaGrammar)
     * @return XSAnnotation associated with the key object
     */
    XSAnnotation* getAnnotation(const void* const key);

    /**
     * Retrieve the annotation that is associated with the specified key
     *
     * @param  key   represents a schema component object (i.e. SchemaGrammar)
     * @return XSAnnotation associated with the key object
     */
    const XSAnnotation* getAnnotation(const void* const key) const;

    /**
      * Get global annotation
      */
    XSAnnotation* getAnnotation();
    const XSAnnotation* getAnnotation() const;

    /**
      * Get annotation hash table, to enumerate through them
      */
    RefHashTableOf<XSAnnotation, PtrHasher>*  getAnnotations();
    const RefHashTableOf<XSAnnotation, PtrHasher>*  getAnnotations() const;

    /**
     * Get/set scope count.
     */
    unsigned int getScopeCount () const;
    void setScopeCount (unsigned int);

    /**
     * Get/set anonymous type count.
     */
    unsigned int getAnonTypeCount () const;
    void setAnonTypeCount (unsigned int);

    /***
     * Support for Serialization/De-serialization
     ***/
    DECL_XSERIALIZABLE(SchemaGrammar)

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    SchemaGrammar(const SchemaGrammar&);
    SchemaGrammar& operator=(const SchemaGrammar&);

    // -----------------------------------------------------------------------
    //  Helper methods
    // -----------------------------------------------------------------------
    void cleanUp();

    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fElemDeclPool
    //      This is the element decl pool. It contains all of the elements
    //      declared in the Schema (and their associated attributes.)
    //
    //  fElemNonDeclPool
    //      This is the element decl pool that is is populated as new elements
    //      are seen in the XML document (not declared in the Schema), and they
    //      are given default characteristics.
    //
    //  fGroupElemDeclPool
    //      This is the element decl pool for elements in a group that are
    //      referenced in different scope. It contains all of the elements
    //      declared in the Schema (and their associated attributes.)
    //
    //  fNotationDeclPool
    //      This is a pool of NotationDecl objects, which contains all of the
    //      notations declared in the Schema.
    //
    //  fTargetNamespace
    //      Target name space for this grammar.
    //
    //  fAttributeDeclRegistry
    //      Global attribute declarations
    //
    //  fComplexTypeRegistry
    //      Stores complexType declaration info
    //
    //  fGroupInfoRegistry
    //      Stores global <group> declaration info
    //
    //  fAttGroupInfoRegistry
    //      Stores global <attributeGroup> declaration info
    //
    //  fDatatypeRegistry
    //      Datatype validator factory
    //
    //  fValidSubstitutionGroups
    //      Valid list of elements that can substitute a given element
    //
    //  fIDRefList
    //      List of ids of schema declarations extracted during schema grammar
    //      traversal
    //
    //  fValidated
    //      Indicates if the content of the Grammar has been pre-validated
    //      or not (UPA checking, etc.). When using a cached grammar, no need
    //      for pre content validation.
    //
    //  fGramDesc: adopted
    //
    // -----------------------------------------------------------------------
    XMLCh*                                   fTargetNamespace;
    RefHash3KeysIdPool<SchemaElementDecl>*   fElemDeclPool;
    RefHash3KeysIdPool<SchemaElementDecl>*   fElemNonDeclPool;
    RefHash3KeysIdPool<SchemaElementDecl>*   fGroupElemDeclPool;
    NameIdPool<XMLNotationDecl>*             fNotationDeclPool;
    RefHashTableOf<XMLAttDef>*               fAttributeDeclRegistry;
    RefHashTableOf<ComplexTypeInfo>*         fComplexTypeRegistry;
    RefHashTableOf<XercesGroupInfo>*         fGroupInfoRegistry;
    RefHashTableOf<XercesAttGroupInfo>*      fAttGroupInfoRegistry;
    RefHash2KeysTableOf<ElemVector>*         fValidSubstitutionGroups;
    // @deprecated
    ValidationContext*                       fValidationContext;
    MemoryManager*                           fMemoryManager;
    XMLSchemaDescription*                    fGramDesc;
    RefHashTableOf<XSAnnotation, PtrHasher>* fAnnotations;

    bool                                   fValidated;
    DatatypeValidatorFactory               fDatatypeRegistry;

    unsigned int                             fScopeCount;
    unsigned int                             fAnonTypeCount;
};


// ---------------------------------------------------------------------------
//  SchemaGrammar: Getter methods
// ---------------------------------------------------------------------------
inline RefHash3KeysIdPoolEnumerator<SchemaElementDecl>
SchemaGrammar::getElemEnumerator() const
{
    return RefHash3KeysIdPoolEnumerator<SchemaElementDecl>(fElemDeclPool, false, fMemoryManager);
}

inline NameIdPoolEnumerator<XMLNotationDecl>
SchemaGrammar::getNotationEnumerator() const
{
    return NameIdPoolEnumerator<XMLNotationDecl>(fNotationDeclPool, fMemoryManager);
}

inline RefHashTableOf<XMLAttDef>* SchemaGrammar::getAttributeDeclRegistry() const {

    return fAttributeDeclRegistry;
}

inline RefHashTableOf<ComplexTypeInfo>*
SchemaGrammar::getComplexTypeRegistry() const {

    return fComplexTypeRegistry;
}

inline RefHashTableOf<XercesGroupInfo>*
SchemaGrammar::getGroupInfoRegistry() const {

    return fGroupInfoRegistry;
}

inline RefHashTableOf<XercesAttGroupInfo>*
SchemaGrammar::getAttGroupInfoRegistry() const {

    return fAttGroupInfoRegistry;
}

inline DatatypeValidatorFactory* SchemaGrammar::getDatatypeRegistry() {

    return &fDatatypeRegistry;
}

inline RefHash2KeysTableOf<ElemVector>*
SchemaGrammar::getValidSubstitutionGroups() const {

    return fValidSubstitutionGroups;
}

// @deprecated
inline ValidationContext* SchemaGrammar::getValidationContext() const {

    return fValidationContext;
}

inline XMLGrammarDescription* SchemaGrammar::getGrammarDescription() const
{
    return fGramDesc;
}

inline XSAnnotation* SchemaGrammar::getAnnotation(const void* const key)
{
    return fAnnotations->get(key);
}

inline const XSAnnotation* SchemaGrammar::getAnnotation(const void* const key) const
{
    return fAnnotations->get(key);
}

inline XSAnnotation* SchemaGrammar::getAnnotation()
{
    return fAnnotations->get(this);
}

inline const XSAnnotation* SchemaGrammar::getAnnotation() const
{
    return fAnnotations->get(this);
}

inline RefHashTableOf<XSAnnotation, PtrHasher>* SchemaGrammar::getAnnotations()
{
    return fAnnotations;
}

inline const RefHashTableOf<XSAnnotation, PtrHasher>* SchemaGrammar::getAnnotations() const
{
    return fAnnotations;
}
// -----------------------------------------------------------------------
//  Setter methods
// -----------------------------------------------------------------------
inline void SchemaGrammar::setTargetNamespace(const XMLCh* const targetNamespace)
{
    if (fTargetNamespace)
        fMemoryManager->deallocate(fTargetNamespace);//delete [] fTargetNamespace;
    fTargetNamespace = XMLString::replicate(targetNamespace, fMemoryManager);
}

inline void
SchemaGrammar::setAttributeDeclRegistry(RefHashTableOf<XMLAttDef>* const attReg) {

    fAttributeDeclRegistry = attReg;
}

inline void
SchemaGrammar::setComplexTypeRegistry(RefHashTableOf<ComplexTypeInfo>* const other) {

    fComplexTypeRegistry = other;
}

inline void
SchemaGrammar::setGroupInfoRegistry(RefHashTableOf<XercesGroupInfo>* const other) {

    fGroupInfoRegistry = other;
}

inline void
SchemaGrammar::setAttGroupInfoRegistry(RefHashTableOf<XercesAttGroupInfo>* const other) {

    fAttGroupInfoRegistry = other;
}

inline void
SchemaGrammar::setValidSubstitutionGroups(RefHash2KeysTableOf<ElemVector>* const other) {

    fValidSubstitutionGroups = other;
}


// ---------------------------------------------------------------------------
//  SchemaGrammar: Virtual methods
// ---------------------------------------------------------------------------
inline Grammar::GrammarType SchemaGrammar::getGrammarType() const {
    return Grammar::SchemaGrammarType;
}

inline const XMLCh* SchemaGrammar::getTargetNamespace() const {
    return fTargetNamespace;
}

// Element Decl
inline XMLSize_t SchemaGrammar::getElemId (const   unsigned int  uriId
                                              , const XMLCh* const    baseName
                                              , const XMLCh* const
                                              , unsigned int          scope ) const
{
    //
    //  In this case, we don't return zero to mean 'not found', so we have to
    //  map it to the official not found value if we don't find it.
    //
    const SchemaElementDecl* decl = fElemDeclPool->getByKey(baseName, uriId, scope);
    if (!decl) {

        decl = fGroupElemDeclPool->getByKey(baseName, uriId, scope);

        if (!decl)
            return XMLElementDecl::fgInvalidElemId;
    }
    return decl->getId();
}

inline const XMLElementDecl* SchemaGrammar::getElemDecl( const   unsigned int  uriId
                                              , const XMLCh* const    baseName
                                              , const XMLCh* const
                                              , unsigned int          scope )   const
{
    const SchemaElementDecl* decl = fElemDeclPool->getByKey(baseName, uriId, scope);

    if (!decl) {

        decl = fGroupElemDeclPool->getByKey(baseName, uriId, scope);

        if (!decl && fElemNonDeclPool)
            decl = fElemNonDeclPool->getByKey(baseName, uriId, scope);
    }

    return decl;
}

inline XMLElementDecl* SchemaGrammar::getElemDecl (const   unsigned int  uriId
                                              , const XMLCh* const    baseName
                                              , const XMLCh* const
                                              , unsigned int          scope )
{
    SchemaElementDecl* decl = fElemDeclPool->getByKey(baseName, uriId, scope);

    if (!decl) {

        decl = fGroupElemDeclPool->getByKey(baseName, uriId, scope);

        if (!decl && fElemNonDeclPool)
            decl = fElemNonDeclPool->getByKey(baseName, uriId, scope);
    }

    return decl;
}

inline const XMLElementDecl* SchemaGrammar::getElemDecl(const unsigned int elemId) const
{
    // Look up this element decl by id
    const SchemaElementDecl* decl = fElemDeclPool->getById(elemId);

    if (!decl)
        decl = fGroupElemDeclPool->getById(elemId);

    return decl;
}

inline XMLElementDecl* SchemaGrammar::getElemDecl(const unsigned int elemId)
{
    // Look up this element decl by id
    SchemaElementDecl* decl = fElemDeclPool->getById(elemId);

    if (!decl)
        decl = fGroupElemDeclPool->getById(elemId);

    return decl;
}

inline XMLSize_t
SchemaGrammar::putElemDecl(XMLElementDecl* const elemDecl,
                           const bool notDeclared)
{
    if (notDeclared)
    {
        if(!fElemNonDeclPool)
            fElemNonDeclPool = new (fMemoryManager) RefHash3KeysIdPool<SchemaElementDecl>(29, true, 128, fMemoryManager);
        return fElemNonDeclPool->put(elemDecl->getBaseName(), elemDecl->getURI(), ((SchemaElementDecl* )elemDecl)->getEnclosingScope(), (SchemaElementDecl*) elemDecl);
    }

    return fElemDeclPool->put(elemDecl->getBaseName(), elemDecl->getURI(), ((SchemaElementDecl* )elemDecl)->getEnclosingScope(), (SchemaElementDecl*) elemDecl);
}

inline XMLSize_t SchemaGrammar::putGroupElemDecl (XMLElementDecl* const elemDecl)   const
{
    return fGroupElemDeclPool->put(elemDecl->getBaseName(), elemDecl->getURI(), ((SchemaElementDecl* )elemDecl)->getEnclosingScope(), (SchemaElementDecl*) elemDecl);
}

// Notation Decl
inline const XMLNotationDecl* SchemaGrammar::getNotationDecl(const XMLCh* const notName) const
{
    return fNotationDeclPool->getByKey(notName);
}

inline XMLNotationDecl* SchemaGrammar::getNotationDecl(const XMLCh* const notName)
{
    return fNotationDeclPool->getByKey(notName);
}

inline XMLSize_t SchemaGrammar::putNotationDecl(XMLNotationDecl* const notationDecl)   const
{
    return fNotationDeclPool->put(notationDecl);
}

inline bool SchemaGrammar::getValidated() const
{
    return fValidated;
}

inline void SchemaGrammar::setValidated(const bool newState)
{
    fValidated = newState;
}

inline unsigned int
SchemaGrammar::getScopeCount () const
{
  return fScopeCount;
}

inline void
SchemaGrammar::setScopeCount (unsigned int scopeCount)
{
  fScopeCount = scopeCount;
}

inline unsigned int
SchemaGrammar::getAnonTypeCount () const
{
  return fAnonTypeCount;
}

inline void
SchemaGrammar::setAnonTypeCount (unsigned int count)
{
  fAnonTypeCount = count;
}

XERCES_CPP_NAMESPACE_END

#endif
