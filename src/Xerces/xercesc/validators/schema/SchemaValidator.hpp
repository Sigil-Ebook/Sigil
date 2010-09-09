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
 * $Id: SchemaValidator.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_SCHEMAVALIDATOR_HPP)
#define XERCESC_INCLUDE_GUARD_SCHEMAVALIDATOR_HPP

#include <xercesc/framework/XMLValidator.hpp>
#include <xercesc/framework/XMLBuffer.hpp>
#include <xercesc/util/ValueStackOf.hpp>
#include <xercesc/validators/common/ContentSpecNode.hpp>
#include <xercesc/validators/schema/SchemaGrammar.hpp>
#include <xercesc/validators/schema/XSDErrorReporter.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class GrammarResolver;
class DatatypeValidator;
class SchemaElementDecl;

//
//  This is a derivative of the abstract validator interface. This class
//  implements a validator that supports standard XML Schema semantics.
//  This class handles scanning the of the schema, and provides
//  the standard validation services against the Schema info it found.
//
class VALIDATORS_EXPORT SchemaValidator : public XMLValidator
{
public:
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    SchemaValidator
    (
          XMLErrorReporter* const errReporter = 0
          , MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
    );
    virtual ~SchemaValidator();

    // -----------------------------------------------------------------------
    //  Implementation of the XMLValidator interface
    // -----------------------------------------------------------------------
    virtual bool checkContent
    (
        XMLElementDecl* const   elemDecl
        , QName** const         children
        , XMLSize_t             childCount
        , XMLSize_t*            indexFailingChild
    );

    virtual void faultInAttr
    (
                XMLAttr&    toFill
        , const XMLAttDef&  attDef
    )   const;

    virtual void preContentValidation(bool reuseGrammar,
                                      bool validateDefAttr = false);

    virtual void postParseValidation();

    virtual void reset();

    virtual bool requiresNamespaces() const;

    virtual void validateAttrValue
    (
        const   XMLAttDef*                  attDef
        , const XMLCh* const                attrValue
        , bool                              preValidation = false
        , const XMLElementDecl*             elemDecl = 0
    );

    virtual void validateElement
    (
        const   XMLElementDecl*             elemDef
    );

    virtual Grammar* getGrammar() const;
    virtual void setGrammar(Grammar* aGrammar);

    // -----------------------------------------------------------------------
    //  Virtual DTD handler interface.
    // -----------------------------------------------------------------------
    virtual bool handlesDTD() const;

    // -----------------------------------------------------------------------
    //  Virtual Schema handler interface. handlesSchema() always return false.
    // -----------------------------------------------------------------------
    virtual bool handlesSchema() const;

    // -----------------------------------------------------------------------
    //  Schema Validator methods
    // -----------------------------------------------------------------------
    void normalizeWhiteSpace(DatatypeValidator* dV, const XMLCh* const value, XMLBuffer& toFill, bool bStandalone = false);

    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------
    void setGrammarResolver(GrammarResolver* grammarResolver);

    void setXsiType(const XMLCh* const        prefix
      , const XMLCh* const        localPart
       , const unsigned int        uriId);

    void setNillable(bool isNil);
    void resetNillable();
    void setErrorReporter(XMLErrorReporter* const errorReporter);
    void setExitOnFirstFatal(const bool newValue);
    void setDatatypeBuffer(const XMLCh* const value);
    void clearDatatypeBuffer();

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    ComplexTypeInfo* getCurrentTypeInfo() const;
    DatatypeValidator *getCurrentDatatypeValidator() const;
    DatatypeValidator *getMostRecentAttrValidator() const;
    bool getErrorOccurred() const;
    bool getIsElemSpecified() const;
    bool getIsXsiTypeSet() const;
    const XMLCh* getNormalizedValue() const;

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    SchemaValidator(const SchemaValidator&);
    SchemaValidator& operator=(const SchemaValidator&);

    // -----------------------------------------------------------------------
    //  Element Consistency Checking methods
    // -----------------------------------------------------------------------
    void checkRefElementConsistency(SchemaGrammar* const currentGrammar,
                                    const ComplexTypeInfo* const curTypeInfo,
                                    const XercesGroupInfo* const curGroup = 0);

    // -----------------------------------------------------------------------
    //  Particle Derivation Checking methods
    // -----------------------------------------------------------------------
    void checkParticleDerivation(SchemaGrammar* const currentGrammar,
                                 const ComplexTypeInfo* const typeInfo);
    void checkParticleDerivationOk(SchemaGrammar* const currentGrammar,
                                   ContentSpecNode* const curNode,
                                   const int derivedScope,
                                   ContentSpecNode* const baseNode,
                                   const int baseScope,
                                   const ComplexTypeInfo* const baseInfo = 0,
                                   const bool toCheckOccurrence = true);
    ContentSpecNode* checkForPointlessOccurrences(ContentSpecNode* const specNode,
                                                  const ContentSpecNode::NodeTypes nodeType,
                                                  ValueVectorOf<ContentSpecNode*>* const nodes);
    void gatherChildren(const ContentSpecNode::NodeTypes parentNodeType,
                        ContentSpecNode* const specNode,
                        ValueVectorOf<ContentSpecNode*>* const nodes);
    bool isOccurrenceRangeOK(const int min1, const int max1, const int min2, const int max2);
    void checkNSCompat(const ContentSpecNode* const derivedSpecNode,
                       const ContentSpecNode* const baseSpecNode,
                       const bool toCheckOccurence);
    bool wildcardEltAllowsNamespace(const ContentSpecNode* const baseSpecNode,
                                    const unsigned int derivedURI);
    void checkNameAndTypeOK(SchemaGrammar* const currentGrammar,
                            const ContentSpecNode* const derivedSpecNode,
                            const int derivedScope,
                            const ContentSpecNode* const baseSpecNode,
                            const int baseScope,
                            const ComplexTypeInfo* const baseInfo = 0);
    SchemaElementDecl* findElement(const int scope,
                                   const unsigned int uriIndex,
                                   const XMLCh* const name,
                                   SchemaGrammar* const grammar,
                                   const ComplexTypeInfo* const typeInfo = 0);
    void checkICRestriction(const SchemaElementDecl* const derivedElemDecl,
                            const SchemaElementDecl* const baseElemDecl,
                            const XMLCh* const derivedElemName,
                            const XMLCh* const baseElemName);
    void checkTypesOK(const SchemaElementDecl* const derivedElemDecl,
                      const SchemaElementDecl* const baseElemDecl,
                      const XMLCh* const derivedElemName);
    void checkRecurseAsIfGroup(SchemaGrammar* const currentGrammar,
                               ContentSpecNode* const derivedSpecNode,
                               const int derivedScope,
                               const ContentSpecNode* const baseSpecNode,
                               const int baseScope,
                               ValueVectorOf<ContentSpecNode*>* const nodes,
                               const ComplexTypeInfo* const baseInfo);
    void checkRecurse(SchemaGrammar* const currentGrammar,
                      const ContentSpecNode* const derivedSpecNode,
                      const int derivedScope,
                      ValueVectorOf<ContentSpecNode*>* const derivedNodes,
                      const ContentSpecNode* const baseSpecNode,
                      const int baseScope,
                      ValueVectorOf<ContentSpecNode*>* const baseNodes,
                      const ComplexTypeInfo* const baseInfo,
                      const bool toLax = false);
    void checkNSSubset(const ContentSpecNode* const derivedSpecNode,
                       const ContentSpecNode* const baseSpecNode);
    bool checkNSSubsetChoiceRoot(const ContentSpecNode* const derivedSpecNode,
                       const ContentSpecNode* const baseSpecNode);
    bool checkNSSubsetChoice(const ContentSpecNode* const derivedSpecNode,
                       const ContentSpecNode* const baseSpecNode);
    bool isWildCardEltSubset(const ContentSpecNode* const derivedSpecNode,
                             const ContentSpecNode* const baseSpecNode);
    void checkNSRecurseCheckCardinality(SchemaGrammar* const currentGrammar,
                                        const ContentSpecNode* const derivedSpecNode,
                                        ValueVectorOf<ContentSpecNode*>* const derivedNodes,
                                        const int derivedScope,
                                        ContentSpecNode* const baseSpecNode,
                                        const bool toCheckOccurence);
    void checkRecurseUnordered(SchemaGrammar* const currentGrammar,
                               const ContentSpecNode* const derivedSpecNode,
                               ValueVectorOf<ContentSpecNode*>* const derivedNodes,
                               const int derivedScope,
                               ContentSpecNode* const baseSpecNode,
                               ValueVectorOf<ContentSpecNode*>* const baseNodes,
                               const int baseScope,
                               const ComplexTypeInfo* const baseInfo);
    void checkMapAndSum(SchemaGrammar* const currentGrammar,
                        const ContentSpecNode* const derivedSpecNode,
                        ValueVectorOf<ContentSpecNode*>* const derivedNodes,
                        const int derivedScope,
                        ContentSpecNode* const baseSpecNode,
                        ValueVectorOf<ContentSpecNode*>* const baseNodes,
                        const int baseScope,
                        const ComplexTypeInfo* const baseInfo);
    ContentSpecNode* getNonUnaryGroup(ContentSpecNode* const pNode);

    // -----------------------------------------------------------------------
    //  Private data members
    //
    // -----------------------------------------------------------------------
    //  The following comes from or set by the Scanner
    //  fSchemaGrammar
    //      The current schema grammar used by the validator
    //
    //  fGrammarResolver
    //      All the schema grammar stored
    //
    //  fXsiType
    //      Store the Schema Type Attribute Value if schema type is specified
    //
    //  fNil
    //      Indicates if a nil value is acceptable
    //  fNilFound
    //      Indicates if Nillable has been set
    // -----------------------------------------------------------------------
    //  The following used internally in the validator
    //
    //  fCurrentDatatypeValidator
    //      The validator used for validating the content of elements
    //      with simple types
    //
    //  fDatatypeBuffer
    //      Buffer for simple type element string content
    //
    //  fTrailing
    //      Previous chunk had a trailing space
    //
    //  fSeenNonWhiteSpace
    //      Seen a non-whitespace character in the previous chunk
    //
    //  fSeenId
    //      Indicate if an attribute of ID type has been seen already, reset per element.
    //
    //  fSchemaErrorReporter
    //      Report schema process errors
    //
    //  fTypeStack
    //      Stack of complex type declarations.
    //
    //  fMostRecentAttrValidator
    //      DatatypeValidator that validated attribute most recently processed
    //
    //  fErrorOccurred
    //      whether an error occurred in the most recent operation
    // -----------------------------------------------------------------------
    MemoryManager*                  fMemoryManager;
    SchemaGrammar*                  fSchemaGrammar;
    GrammarResolver*                fGrammarResolver;
    QName*                          fXsiType;
    bool                            fNil;
    bool                            fNilFound;
    DatatypeValidator*              fCurrentDatatypeValidator;
    XMLBuffer*                      fNotationBuf;
    XMLBuffer                       fDatatypeBuffer;
    bool                            fTrailing;
    bool                            fSeenNonWhiteSpace;
    bool                            fSeenId;
    XSDErrorReporter                fSchemaErrorReporter;
    ValueStackOf<ComplexTypeInfo*>* fTypeStack;
    DatatypeValidator *             fMostRecentAttrValidator;
    bool                            fErrorOccurred;
    bool                            fElemIsSpecified;
};


// ---------------------------------------------------------------------------
//  SchemaValidator: Setter methods
// ---------------------------------------------------------------------------
inline void SchemaValidator::setGrammarResolver(GrammarResolver* grammarResolver) {
    fGrammarResolver = grammarResolver;
}

inline void SchemaValidator::setXsiType(const XMLCh* const        prefix
      , const XMLCh* const        localPart
       , const unsigned int        uriId)
{
    delete fXsiType;
    fXsiType = new (fMemoryManager) QName(prefix, localPart, uriId, fMemoryManager);
}

inline void SchemaValidator::setNillable(bool isNil) {
    fNil = isNil;
    fNilFound = true;
}

inline void SchemaValidator::resetNillable() {
    fNil = false;
    fNilFound = false;
}

inline void SchemaValidator::setExitOnFirstFatal(const bool newValue) {

    fSchemaErrorReporter.setExitOnFirstFatal(newValue);
}

inline void SchemaValidator::setDatatypeBuffer(const XMLCh* const value)
{
    fDatatypeBuffer.append(value);
}

inline void SchemaValidator::clearDatatypeBuffer()
{
    fDatatypeBuffer.reset();
}

// ---------------------------------------------------------------------------
//  SchemaValidator: Getter methods
// ---------------------------------------------------------------------------
inline ComplexTypeInfo* SchemaValidator::getCurrentTypeInfo() const {
    if (fTypeStack->empty())
        return 0;
    return fTypeStack->peek();
}

inline DatatypeValidator * SchemaValidator::getCurrentDatatypeValidator() const
{
    return fCurrentDatatypeValidator;
}
inline DatatypeValidator *SchemaValidator::getMostRecentAttrValidator() const
{
    return fMostRecentAttrValidator;
}

// ---------------------------------------------------------------------------
//  Virtual interface
// ---------------------------------------------------------------------------
inline Grammar* SchemaValidator::getGrammar() const {
    return fSchemaGrammar;
}

inline void SchemaValidator::setGrammar(Grammar* aGrammar) {
    fSchemaGrammar = (SchemaGrammar*) aGrammar;
}

inline void SchemaValidator::setErrorReporter(XMLErrorReporter* const errorReporter) {

    XMLValidator::setErrorReporter(errorReporter);
    fSchemaErrorReporter.setErrorReporter(errorReporter);
}

// ---------------------------------------------------------------------------
//  SchemaValidator: DTD handler interface
// ---------------------------------------------------------------------------
inline bool SchemaValidator::handlesDTD() const
{
    // No DTD scanning
    return false;
}

// ---------------------------------------------------------------------------
//  SchemaValidator: Schema handler interface
// ---------------------------------------------------------------------------
inline bool SchemaValidator::handlesSchema() const
{
    return true;
}

// ---------------------------------------------------------------------------
//  SchemaValidator: Particle derivation checking
// ---------------------------------------------------------------------------
inline bool
SchemaValidator::isOccurrenceRangeOK(const int min1, const int max1,
                                     const int min2, const int max2) {

    if (min1 >= min2 &&
        (max2 == SchemaSymbols::XSD_UNBOUNDED ||
         (max1 != SchemaSymbols::XSD_UNBOUNDED && max1 <= max2))) {
        return true;
    }
    return false;
}

inline bool SchemaValidator::getErrorOccurred() const
{
    return fErrorOccurred;
}

inline bool SchemaValidator::getIsElemSpecified() const
{
    return fElemIsSpecified;
}

inline const XMLCh* SchemaValidator::getNormalizedValue() const
{
    return fDatatypeBuffer.getRawBuffer();
}

inline bool SchemaValidator::getIsXsiTypeSet() const
{
    return (fXsiType!=0);
}

XERCES_CPP_NAMESPACE_END

#endif
