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
 * $Id: SchemaElementDecl.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_SCHEMAELEMENTDECL_HPP)
#define XERCESC_INCLUDE_GUARD_SCHEMAELEMENTDECL_HPP

#include <xercesc/util/QName.hpp>
#include <xercesc/validators/common/Grammar.hpp>
#include <xercesc/validators/schema/ComplexTypeInfo.hpp>
#include <xercesc/validators/schema/identity/IdentityConstraint.hpp>
#include <xercesc/validators/datatype/DatatypeValidator.hpp>
#include <xercesc/validators/schema/PSVIDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class ContentSpecNode;
class SchemaAttDefList;

//
//  This class is a derivative of the basic element decl. This one implements
//  the virtuals so that they work for a Schema.
//
class VALIDATORS_EXPORT SchemaElementDecl : public XMLElementDecl
{
public :

    // -----------------------------------------------------------------------
    //  Class specific types
    //
    //  ModelTypes
    //      Indicates the type of content model that an element has. This
    //      indicates how the content model is represented and validated.
    // -----------------------------------------------------------------------
    enum ModelTypes
    {
        Empty
        , Any
        , Mixed_Simple
        , Mixed_Complex
        , Children
        , Simple
        , ElementOnlyEmpty
        , ModelTypes_Count
    };

    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    SchemaElementDecl(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    SchemaElementDecl
    (
          const XMLCh* const   prefix
        , const XMLCh* const   localPart
        , const int            uriId
        , const ModelTypes     modelType = Any
        , const unsigned int   enclosingScope = Grammar::TOP_LEVEL_SCOPE
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    SchemaElementDecl
    (
          const QName* const   elementName
        , const ModelTypes     modelType = Any
        , const unsigned int   enclosingScope = Grammar::TOP_LEVEL_SCOPE
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    ~SchemaElementDecl();


    // -----------------------------------------------------------------------
    //  The virtual element decl interface
    // -----------------------------------------------------------------------
    virtual XMLAttDefList& getAttDefList() const;
    virtual CharDataOpts getCharDataOpts() const;
    virtual bool hasAttDefs() const;
    virtual const ContentSpecNode* getContentSpec() const;
    virtual ContentSpecNode* getContentSpec();
    virtual void setContentSpec(ContentSpecNode* toAdopt);
    virtual XMLContentModel* getContentModel();
    virtual void setContentModel(XMLContentModel* const newModelToAdopt);
    virtual const XMLCh* getFormattedContentModel ()   const;


    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    const SchemaAttDef* getAttDef(const XMLCh* const baseName, const int uriId) const;
    SchemaAttDef* getAttDef(const XMLCh* const baseName, const int uriId);
    const SchemaAttDef* getAttWildCard() const;
    SchemaAttDef* getAttWildCard();
    ModelTypes getModelType() const;
    PSVIDefs::PSVIScope getPSVIScope() const;
    DatatypeValidator* getDatatypeValidator() const;
    unsigned int getEnclosingScope() const;
    int getFinalSet() const;
    int getBlockSet() const;
    int getMiscFlags() const;
    XMLCh* getDefaultValue() const;
    ComplexTypeInfo* getComplexTypeInfo() const;
    virtual bool isGlobalDecl() const;
    SchemaElementDecl* getSubstitutionGroupElem() const;

    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------
    void setModelType(const SchemaElementDecl::ModelTypes toSet);
    void setPSVIScope(const PSVIDefs::PSVIScope toSet);
    void setDatatypeValidator(DatatypeValidator* newDatatypeValidator);
    void setEnclosingScope(const unsigned int enclosingScope);
    void setFinalSet(const int finalSet);
    void setBlockSet(const int blockSet);
    void setMiscFlags(const int flags);
    void setDefaultValue(const XMLCh* const value);
    void setComplexTypeInfo(ComplexTypeInfo* const typeInfo);	
    void setAttWildCard(SchemaAttDef* const attWildCard);
    void setSubstitutionGroupElem(SchemaElementDecl* const elemDecl);  

    // -----------------------------------------------------------------------
    //  IC methods
    // -----------------------------------------------------------------------
    void addIdentityConstraint(IdentityConstraint* const ic);
    XMLSize_t getIdentityConstraintCount() const;
    IdentityConstraint* getIdentityConstraintAt(XMLSize_t index) const;

    /***
     * Support for Serialization/De-serialization
     ***/
    DECL_XSERIALIZABLE(SchemaElementDecl)

    virtual XMLElementDecl::objectType  getObjectType() const;

private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    SchemaElementDecl(const SchemaElementDecl&);
    SchemaElementDecl& operator=(const SchemaElementDecl&);

    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fModelType
    //      The content model type of this element. This tells us what kind
    //      of content model to create.
    //
    //  fDatatypeValidator
    //      The DatatypeValidator used to validate this element type.
    //
    //  fEnclosingScope
    //      The enclosing scope where this element is declared.
    //
    //  fFinalSet
    //      The value set of the 'final' attribute.
    //
    //  fBlockSet
    //      The value set of the 'block' attribute.
    //
    //  fMiscFlags
    //      Stores 'abstract/nullable' values
    //
    //  fDefaultValue
    //      The default/fixed value
    //
    //  fComplexTypeInfo
    //      Stores complex type information
    //      (no need to delete - handled by schema grammar)
    //
    //  fAttDefs
    //      The list of attributes that are faulted in for this element
    //      when ComplexTypeInfo does not exist.  We want to keep track
    //      of these faulted in attributes to avoid duplicate redundant
    //      error.      
    //    	
    //  fIdentityConstraints
    //      Store information about an element identity constraints.
    //
    //  fAttWildCard
    //      Store wildcard attribute in the case of an element with a type of
    //      'anyType'.
    //
    //  fSubstitutionGroupElem
    //      The substitution group element declaration.   
    // -----------------------------------------------------------------------

    // -----------------------------------------------------------------------
    ModelTypes                         fModelType;
    PSVIDefs::PSVIScope                fPSVIScope;

    unsigned int                       fEnclosingScope;
    int                                fFinalSet;
    int                                fBlockSet;
    int                                fMiscFlags;    
    XMLCh*                             fDefaultValue;
    ComplexTypeInfo*                   fComplexTypeInfo;
    RefHash2KeysTableOf<SchemaAttDef>* fAttDefs;        	
    RefVectorOf<IdentityConstraint>*   fIdentityConstraints;
    SchemaAttDef*                      fAttWildCard;
    SchemaElementDecl*                 fSubstitutionGroupElem;
    DatatypeValidator*                 fDatatypeValidator; 
};

// ---------------------------------------------------------------------------
//  SchemaElementDecl: XMLElementDecl virtual interface implementation
// ---------------------------------------------------------------------------
inline ContentSpecNode* SchemaElementDecl::getContentSpec()
{
    if (fComplexTypeInfo != 0) {
        return fComplexTypeInfo->getContentSpec();
    }

    return 0;
}

inline const ContentSpecNode* SchemaElementDecl::getContentSpec() const
{
    if (fComplexTypeInfo != 0) {
        return fComplexTypeInfo->getContentSpec();
    }

    return 0;
}

inline void
SchemaElementDecl::setContentSpec(ContentSpecNode*)
{
    //Handled by complexType
}

inline XMLContentModel* SchemaElementDecl::getContentModel()
{
    if (fComplexTypeInfo != 0) {
        return fComplexTypeInfo->getContentModel();
    }
    return 0;
}

inline void
SchemaElementDecl::setContentModel(XMLContentModel* const)
{
    //Handled by complexType
}


// ---------------------------------------------------------------------------
//  SchemaElementDecl: Getter methods
// ---------------------------------------------------------------------------
inline SchemaElementDecl::ModelTypes SchemaElementDecl::getModelType() const
{
    if (fComplexTypeInfo) {
        return (SchemaElementDecl::ModelTypes) fComplexTypeInfo->getContentType();
    }

    return fModelType;
}

inline PSVIDefs::PSVIScope SchemaElementDecl::getPSVIScope() const
{
    return fPSVIScope;
}

inline DatatypeValidator* SchemaElementDecl::getDatatypeValidator() const
{
    return fDatatypeValidator;
}

inline unsigned int SchemaElementDecl::getEnclosingScope() const
{
    return fEnclosingScope;
}

inline int SchemaElementDecl::getFinalSet() const
{
    return fFinalSet;
}

inline int SchemaElementDecl::getBlockSet() const
{
    return fBlockSet;
}

inline int SchemaElementDecl::getMiscFlags() const
{
    return fMiscFlags;
}

inline XMLCh* SchemaElementDecl::getDefaultValue() const
{
    return fDefaultValue;
}

inline ComplexTypeInfo* SchemaElementDecl::getComplexTypeInfo() const
{
    return fComplexTypeInfo;
}

inline const SchemaAttDef* SchemaElementDecl::getAttWildCard() const {
    return fAttWildCard;
}

inline SchemaAttDef* SchemaElementDecl::getAttWildCard() {
    return fAttWildCard;
}

inline bool SchemaElementDecl::isGlobalDecl() const {

    return (fEnclosingScope == Grammar::TOP_LEVEL_SCOPE);
}

inline SchemaElementDecl*
SchemaElementDecl::getSubstitutionGroupElem() const {

    return fSubstitutionGroupElem;
}

// ---------------------------------------------------------------------------
//  SchemaElementDecl: Setter methods
// ---------------------------------------------------------------------------
inline void
SchemaElementDecl::setModelType(const SchemaElementDecl::ModelTypes toSet)
{
    fModelType = toSet;
}

inline void
SchemaElementDecl::setPSVIScope(const PSVIDefs::PSVIScope toSet)
{
    fPSVIScope = toSet;
}

inline void SchemaElementDecl::setDatatypeValidator(DatatypeValidator* newDatatypeValidator)
{
    fDatatypeValidator = newDatatypeValidator;
}

inline void SchemaElementDecl::setEnclosingScope(const unsigned int newEnclosingScope)
{
    fEnclosingScope = newEnclosingScope;
}

inline void SchemaElementDecl::setFinalSet(const int finalSet)
{
    fFinalSet = finalSet;
}

inline void SchemaElementDecl::setBlockSet(const int blockSet)
{
    fBlockSet = blockSet;
}

inline void SchemaElementDecl::setMiscFlags(const int flags)
{
    fMiscFlags = flags;
}

inline void SchemaElementDecl::setDefaultValue(const XMLCh* const value)
{
    if (fDefaultValue) {
        getMemoryManager()->deallocate(fDefaultValue);//delete[] fDefaultValue;
    }

    fDefaultValue = XMLString::replicate(value, getMemoryManager());
}

inline void
SchemaElementDecl::setComplexTypeInfo(ComplexTypeInfo* const typeInfo)
{
    fComplexTypeInfo = typeInfo;
}

inline void
SchemaElementDecl::setAttWildCard(SchemaAttDef* const attWildCard) {

    if (fAttWildCard)
        delete fAttWildCard;

    fAttWildCard = attWildCard;
}

inline void
SchemaElementDecl::setSubstitutionGroupElem(SchemaElementDecl* const elemDecl) {

    fSubstitutionGroupElem = elemDecl;
}

// ---------------------------------------------------------------------------
//  SchemaElementDecl: IC methods
// ---------------------------------------------------------------------------
inline void
SchemaElementDecl::addIdentityConstraint(IdentityConstraint* const ic) {

    if (!fIdentityConstraints) {
        fIdentityConstraints = new (getMemoryManager()) RefVectorOf<IdentityConstraint>(16, true, getMemoryManager());
    }

    fIdentityConstraints->addElement(ic);
}

inline XMLSize_t SchemaElementDecl::getIdentityConstraintCount() const {

    if (fIdentityConstraints) {
        return fIdentityConstraints->size();
    }

    return 0;
}

inline IdentityConstraint*
SchemaElementDecl::getIdentityConstraintAt(XMLSize_t index) const {

    if (fIdentityConstraints) {
        return fIdentityConstraints->elementAt(index);
    }

    return 0;
}

XERCES_CPP_NAMESPACE_END

#endif
