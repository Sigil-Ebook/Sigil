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
 * $Id: ComplexTypeInfo.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_COMPLEXTYPEINFO_HPP)
#define XERCESC_INCLUDE_GUARD_COMPLEXTYPEINFO_HPP


/**
  * The class act as a place holder to store complex type information.
  *
  * The class is intended for internal use.
  */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/RefHash2KeysTableOf.hpp>
#include <xercesc/util/RefVectorOf.hpp>
#include <xercesc/framework/XMLElementDecl.hpp>
#include <xercesc/framework/XMLContentModel.hpp>
#include <xercesc/validators/schema/SchemaAttDef.hpp>
#include <xercesc/internal/XSerializable.hpp>

XERCES_CPP_NAMESPACE_BEGIN


// ---------------------------------------------------------------------------
//  Forward Declarations
// ---------------------------------------------------------------------------
class DatatypeValidator;
class ContentSpecNode;
class SchemaAttDefList;
class SchemaElementDecl;
class XSDLocator;


class VALIDATORS_EXPORT ComplexTypeInfo : public XSerializable, public XMemory
{
public:
    // -----------------------------------------------------------------------
    //  Public Constructors/Destructor
    // -----------------------------------------------------------------------
    ComplexTypeInfo(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    ~ComplexTypeInfo();

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    bool                     getAbstract() const;
    bool                     getAdoptContentSpec() const;
    bool                     containsAttWithTypeId() const;
    bool                     getPreprocessed() const;
    int                      getDerivedBy() const;
    int                      getBlockSet() const;
    int                      getFinalSet() const;
    unsigned int             getScopeDefined() const;
    unsigned int             getElementId() const;
    int                      getContentType() const;
    XMLSize_t                elementCount() const;
    XMLCh*                   getTypeName() const;
    DatatypeValidator*       getBaseDatatypeValidator() const;
    DatatypeValidator*       getDatatypeValidator() const;
    ComplexTypeInfo*         getBaseComplexTypeInfo() const;
    ContentSpecNode*         getContentSpec() const;
    const SchemaAttDef*      getAttWildCard() const;
    SchemaAttDef*            getAttWildCard();
    const SchemaAttDef*      getAttDef(const XMLCh* const baseName,
                                       const int uriId) const;
    SchemaAttDef*            getAttDef(const XMLCh* const baseName,
                                       const int uriId);
    XMLAttDefList&           getAttDefList() const;
    const SchemaElementDecl* elementAt(const XMLSize_t index) const;
    SchemaElementDecl*       elementAt(const XMLSize_t index);
    XMLContentModel*         getContentModel(const bool checkUPA = false);
    const XMLCh*             getFormattedContentModel ()   const;
    XSDLocator*              getLocator() const;
    const XMLCh*             getTypeLocalName() const;
    const XMLCh*             getTypeUri() const;

    /**
     * returns true if this type is anonymous
     **/
    bool getAnonymous() const;

    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------
    void setAbstract(const bool isAbstract);
    void setAdoptContentSpec(const bool toAdopt);
    void setAttWithTypeId(const bool value);
    void setPreprocessed(const bool aValue = true);
    void setDerivedBy(const int derivedBy);
    void setBlockSet(const int blockSet);
    void setFinalSet(const int finalSet);
    void setScopeDefined(const unsigned int scopeDefined);
    void setElementId(const unsigned int elemId);
    void setTypeName(const XMLCh* const typeName);
    void setContentType(const int contentType);
    void setBaseDatatypeValidator(DatatypeValidator* const baseValidator);
    void setDatatypeValidator(DatatypeValidator* const validator);
    void setBaseComplexTypeInfo(ComplexTypeInfo* const typeInfo);
    void setContentSpec(ContentSpecNode* const toAdopt);
    void setAttWildCard(SchemaAttDef* const toAdopt);
    void addAttDef(SchemaAttDef* const toAdd);
    void addElement(SchemaElementDecl* const toAdd);
    void setLocator(XSDLocator* const aLocator);

    /**
     * sets this type to be anonymous
     **/
    void setAnonymous();

    // -----------------------------------------------------------------------
    //  Helper methods
    // -----------------------------------------------------------------------
    bool hasAttDefs() const;
    bool contains(const XMLCh* const attName);
    void checkUniqueParticleAttribution
    (
        SchemaGrammar*    const pGrammar
      , GrammarResolver*  const pGrammarResolver
      , XMLStringPool*    const pStringPool
      , XMLValidator*     const pValidator
    ) ;

    /**
      * Return a singleton that represents 'anyType'
      *
      * @param emptyNSId the uri id of the empty namespace
      */
    static ComplexTypeInfo* getAnyType(unsigned int emptyNSId);

    /**
      *  Notification that lazy data has been deleted
      */
    static void reinitAnyType();

    /***
     * Support for Serialization/De-serialization
     ***/
    DECL_XSERIALIZABLE(ComplexTypeInfo)

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    ComplexTypeInfo(const ComplexTypeInfo& elemInfo);
    ComplexTypeInfo& operator= (const ComplexTypeInfo& other);

    // -----------------------------------------------------------------------
    //  Private helper methods
    // -----------------------------------------------------------------------
    void faultInAttDefList() const;
    bool useRepeatingLeafNodes(ContentSpecNode* particle);
    XMLContentModel* makeContentModel(bool checkUPA = false);
    XMLCh* formatContentModel () const ;
    ContentSpecNode* expandContentModel(ContentSpecNode* const curNode, int minOccurs, int maxOccurs, bool bAllowCompactSyntax);
    ContentSpecNode* convertContentSpecTree(ContentSpecNode* const curNode, bool checkUPA, bool bAllowCompactSyntax);
    void resizeContentSpecOrgURI();

    // -----------------------------------------------------------------------
    //  Private data members
    // -----------------------------------------------------------------------
    bool                               fAnonymous;
    bool                               fAbstract;
    bool                               fAdoptContentSpec;
    bool                               fAttWithTypeId;
    bool                               fPreprocessed;
    int                                fDerivedBy;
    int                                fBlockSet;
    int                                fFinalSet;
    unsigned int                       fScopeDefined;
    int                                fContentType;

    unsigned int                       fElementId;
    unsigned int                       fUniqueURI;
    unsigned int                       fContentSpecOrgURISize;

    XMLCh*                             fTypeName;
    XMLCh*                             fTypeLocalName;
    XMLCh*                             fTypeUri;
    DatatypeValidator*                 fBaseDatatypeValidator;
    DatatypeValidator*                 fDatatypeValidator;
    ComplexTypeInfo*                   fBaseComplexTypeInfo;
    ContentSpecNode*                   fContentSpec;
    SchemaAttDef*                      fAttWildCard;
    SchemaAttDefList*                  fAttList;
    RefVectorOf<SchemaElementDecl>*    fElements;
    RefHash2KeysTableOf<SchemaAttDef>* fAttDefs;
    XMLContentModel*                   fContentModel;
    XMLCh*                             fFormattedModel;
    unsigned int*                      fContentSpecOrgURI;
    XSDLocator*                        fLocator;
    MemoryManager*                     fMemoryManager;

    static ComplexTypeInfo*            fAnyType;

    friend class XMLInitializer;
};

// ---------------------------------------------------------------------------
//  ComplexTypeInfo: Getter methods
// ---------------------------------------------------------------------------
inline bool ComplexTypeInfo::getAbstract() const {

    return fAbstract;
}

inline bool ComplexTypeInfo::getAdoptContentSpec() const {

    return fAdoptContentSpec;
}

inline bool ComplexTypeInfo::containsAttWithTypeId() const {

    return fAttWithTypeId;
}

inline bool ComplexTypeInfo::getPreprocessed() const {

    return fPreprocessed;
}

inline int ComplexTypeInfo::getDerivedBy() const {

    return fDerivedBy;
}

inline int ComplexTypeInfo::getBlockSet() const {

    return fBlockSet;
}

inline int ComplexTypeInfo::getFinalSet() const {

    return fFinalSet;
}

inline unsigned int ComplexTypeInfo::getScopeDefined() const {

    return fScopeDefined;
}

inline unsigned int ComplexTypeInfo::getElementId() const {

    return fElementId;
}

inline int ComplexTypeInfo::getContentType() const {

    return fContentType;
}

inline XMLSize_t ComplexTypeInfo::elementCount() const {

    if (fElements) {
        return fElements->size();
    }

    return 0;
}

inline XMLCh* ComplexTypeInfo::getTypeName() const {
    return fTypeName;
}

inline DatatypeValidator* ComplexTypeInfo::getBaseDatatypeValidator() const {

    return fBaseDatatypeValidator;
}

inline DatatypeValidator* ComplexTypeInfo::getDatatypeValidator() const {

    return fDatatypeValidator;
}

inline ComplexTypeInfo* ComplexTypeInfo::getBaseComplexTypeInfo() const {

    return fBaseComplexTypeInfo;
}

inline ContentSpecNode* ComplexTypeInfo::getContentSpec() const {

    return fContentSpec;
}

inline const SchemaAttDef* ComplexTypeInfo::getAttWildCard() const {

    return fAttWildCard;
}

inline SchemaAttDef* ComplexTypeInfo::getAttWildCard() {

    return fAttWildCard;
}

inline const SchemaAttDef* ComplexTypeInfo::getAttDef(const XMLCh* const baseName,
                                                      const int uriId) const {

    return fAttDefs->get(baseName, uriId);
}

inline SchemaAttDef* ComplexTypeInfo::getAttDef(const XMLCh* const baseName,
                                                const int uriId)
{
    return fAttDefs->get(baseName, uriId);
}

inline SchemaElementDecl*
ComplexTypeInfo::elementAt(const XMLSize_t index) {

    if (!fElements) {
        return 0; // REVISIT - need to throw an exception
    }

    return fElements->elementAt(index);
}

inline const SchemaElementDecl*
ComplexTypeInfo::elementAt(const XMLSize_t index) const {

    if (!fElements) {
        return 0; // REVISIT - need to throw an exception
    }

    return fElements->elementAt(index);
}

inline XMLContentModel* ComplexTypeInfo::getContentModel(const bool checkUPA)
{
    if (!fContentModel && fContentSpec)
        fContentModel = makeContentModel(checkUPA);

    return fContentModel;
}

inline XSDLocator* ComplexTypeInfo::getLocator() const
{
    return fLocator;
}

inline bool ComplexTypeInfo::getAnonymous() const {
    return fAnonymous;
}

inline const XMLCh* ComplexTypeInfo::getTypeLocalName() const
{
    return fTypeLocalName;
}

inline const XMLCh* ComplexTypeInfo::getTypeUri() const
{
   return fTypeUri;
}

// ---------------------------------------------------------------------------
//  ComplexTypeInfo: Setter methods
// ---------------------------------------------------------------------------
inline void ComplexTypeInfo::setAbstract(const bool isAbstract) {

    fAbstract = isAbstract;
}

inline void ComplexTypeInfo::setAdoptContentSpec(const bool toAdopt) {

    fAdoptContentSpec = toAdopt;
}

inline void ComplexTypeInfo::setAttWithTypeId(const bool value) {

    fAttWithTypeId = value;
}

inline void ComplexTypeInfo::setPreprocessed(const bool aValue) {

    fPreprocessed = aValue;
}

inline void ComplexTypeInfo::setDerivedBy(const int derivedBy) {

    fDerivedBy = derivedBy;
}

inline void ComplexTypeInfo::setBlockSet(const int blockSet) {

    fBlockSet = blockSet;
}

inline void ComplexTypeInfo::setFinalSet(const int finalSet) {

    fFinalSet = finalSet;
}

inline void ComplexTypeInfo::setScopeDefined(const unsigned int scopeDefined) {

    fScopeDefined = scopeDefined;
}

inline void ComplexTypeInfo::setElementId(const unsigned int elemId) {

    fElementId = elemId;
}

inline void
ComplexTypeInfo::setContentType(const int contentType) {

    fContentType = contentType;
}

inline void ComplexTypeInfo::setTypeName(const XMLCh* const typeName) {

    fMemoryManager->deallocate(fTypeName);//delete [] fTypeName;
    fMemoryManager->deallocate(fTypeLocalName);//delete [] fTypeLocalName;
    fMemoryManager->deallocate(fTypeUri);//delete [] fTypeUri;

    if (typeName)
    {
        fTypeName = XMLString::replicate(typeName, fMemoryManager);

        int index = XMLString::indexOf(fTypeName, chComma);
        XMLSize_t length = XMLString::stringLen(fTypeName);
        fTypeLocalName = (XMLCh*) fMemoryManager->allocate
        (
            (length - index + 1) * sizeof(XMLCh)
        ); //new XMLCh[length - index + 1];
        XMLString::subString(fTypeLocalName, fTypeName, index + 1, length, fMemoryManager);

        fTypeUri = (XMLCh*) fMemoryManager->allocate
        (
            (index + 1) * sizeof(XMLCh)
        ); //new XMLCh[index + 1];
        XMLString::subString(fTypeUri, fTypeName, 0, index, fMemoryManager);
    }
    else
    {
        fTypeName = fTypeLocalName = fTypeUri = 0;
    }
}

inline void
ComplexTypeInfo::setBaseDatatypeValidator(DatatypeValidator* const validator) {

    fBaseDatatypeValidator = validator;
}

inline void
ComplexTypeInfo::setDatatypeValidator(DatatypeValidator* const validator) {

    fDatatypeValidator = validator;
}

inline void
ComplexTypeInfo::setBaseComplexTypeInfo(ComplexTypeInfo* const typeInfo) {

    fBaseComplexTypeInfo = typeInfo;
}

inline void ComplexTypeInfo::addElement(SchemaElementDecl* const elem) {

    if (!fElements) {
        fElements = new (fMemoryManager) RefVectorOf<SchemaElementDecl>(8, false, fMemoryManager);
    }
    else if (fElements->containsElement(elem)) {
        return;
    }

    fElements->addElement(elem);
}

inline void ComplexTypeInfo::setAttWildCard(SchemaAttDef* const toAdopt) {

    if (fAttWildCard) {
       delete fAttWildCard;
    }

    fAttWildCard = toAdopt;
}

inline void ComplexTypeInfo::setAnonymous() {
    fAnonymous = true;
}

// ---------------------------------------------------------------------------
//  ComplexTypeInfo: Helper methods
// ---------------------------------------------------------------------------
inline bool ComplexTypeInfo::hasAttDefs() const
{
    return !fAttDefs->isEmpty();
}

inline bool ComplexTypeInfo::contains(const XMLCh* const attName) {

    RefHash2KeysTableOfEnumerator<SchemaAttDef> enumDefs(fAttDefs, false, fMemoryManager);

    while (enumDefs.hasMoreElements()) {

        if (XMLString::equals(attName, enumDefs.nextElement().getAttName()->getLocalPart())) {
            return true;
        }
    }

    return false;
}

XERCES_CPP_NAMESPACE_END

#endif

/**
  * End of file ComplexTypeInfo.hpp
  */

