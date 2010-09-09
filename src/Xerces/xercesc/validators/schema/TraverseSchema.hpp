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
 * $Id: TraverseSchema.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_TRAVERSESCHEMA_HPP)
#define XERCESC_INCLUDE_GUARD_TRAVERSESCHEMA_HPP

/**
  * Instances of this class get delegated to Traverse the Schema and
  * to populate the SchemaGrammar internal representation.
  */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMAttr.hpp>
#include <xercesc/framework/XMLBuffer.hpp>
#include <xercesc/framework/XMLErrorCodes.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xercesc/util/ValueVectorOf.hpp>
#include <xercesc/util/RefHash2KeysTableOf.hpp>
#include <xercesc/validators/common/ContentSpecNode.hpp>
#include <xercesc/validators/schema/SchemaGrammar.hpp>
#include <xercesc/validators/schema/SchemaInfo.hpp>
#include <xercesc/validators/schema/GeneralAttributeCheck.hpp>
#include <xercesc/validators/schema/XSDErrorReporter.hpp>
#include <xercesc/util/XMLResourceIdentifier.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Forward Declarations
// ---------------------------------------------------------------------------
class GrammarResolver;
class XMLEntityHandler;
class XMLScanner;
class DatatypeValidator;
class DatatypeValidatorFactory;
class QName;
class ComplexTypeInfo;
class XMLAttDef;
class NamespaceScope;
class SchemaAttDef;
class InputSource;
class XercesGroupInfo;
class XercesAttGroupInfo;
class IdentityConstraint;
class XSDLocator;
class XSDDOMParser;
class XMLErrorReporter;


class VALIDATORS_EXPORT TraverseSchema : public XMemory
{
public:
    // -----------------------------------------------------------------------
    //  Public Constructors/Destructor
    // -----------------------------------------------------------------------
    TraverseSchema
    (
          DOMElement* const       schemaRoot
        , XMLStringPool* const    uriStringPool
        , SchemaGrammar* const    schemaGrammar
        , GrammarResolver* const  grammarResolver
        , RefHash2KeysTableOf<SchemaInfo>* cachedSchemaInfoList
        , RefHash2KeysTableOf<SchemaInfo>* schemaInfoList
        , XMLScanner* const       xmlScanner
        , const XMLCh* const      schemaURL
        , XMLEntityHandler* const entityHandler
        , XMLErrorReporter* const errorReporter
        , MemoryManager* const    manager = XMLPlatformUtils::fgMemoryManager
        , bool multipleImport = false
    );

    ~TraverseSchema();

private:
  	 // This enumeration is defined here for compatibility with the CodeWarrior
  	 // compiler, which apparently doesn't like to accept default parameter
  	 // arguments that it hasn't yet seen. The Not_All_Context argument is
  	 // used in the declaration of checkMinMax, below.
  	 //
    // Flags indicate any special restrictions on minOccurs and maxOccurs
    // relating to "all".
    //    Not_All_Context    - not processing an <all>
    //    All_Element        - processing an <element> in an <all>
    //    Group_Ref_With_All - processing <group> reference that contained <all>
    //    All_Group          - processing an <all> group itself
    enum
	{
        Not_All_Context = 0
        , All_Element = 1
        , Group_Ref_With_All = 2
        , All_Group = 4
    };

    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    TraverseSchema(const TraverseSchema&);
    TraverseSchema& operator=(const TraverseSchema&);

    // -----------------------------------------------------------------------
    //  Init/CleanUp methods
    // -----------------------------------------------------------------------
    void init();
    void cleanUp();

    // -----------------------------------------------------------------------
    //  Traversal methods
    // -----------------------------------------------------------------------
    /**
      * Traverse the Schema DOM tree
      */
    void                doTraverseSchema(const DOMElement* const schemaRoot);
    void                preprocessSchema(DOMElement* const schemaRoot,
                                         const XMLCh* const schemaURL,
                                         bool  multipleImport = false);
    void                traverseSchemaHeader(const DOMElement* const schemaRoot);
    XSAnnotation*       traverseAnnotationDecl(const DOMElement* const childElem,
                                               ValueVectorOf<DOMNode*>* const nonXSAttList,
                                               const bool topLevel = false);
    void                traverseInclude(const DOMElement* const childElem);
    void                traverseImport(const DOMElement* const childElem);
    void                traverseRedefine(const DOMElement* const childElem);
    void                traverseAttributeDecl(const DOMElement* const childElem,
                                              ComplexTypeInfo* const typeInfo,
                                              const bool topLevel = false);
    void                traverseSimpleContentDecl(const XMLCh* const typeName,
                                                  const XMLCh* const qualifiedName,
                                                  const DOMElement* const contentDecl,
                                                  ComplexTypeInfo* const typeInfo,
                                                  Janitor<XSAnnotation>* const janAnnot);
    void                traverseComplexContentDecl(const XMLCh* const typeName,
                                                  const DOMElement* const contentDecl,
                                                  ComplexTypeInfo* const typeInfo,
                                                  const bool isMixed,
                                                  Janitor<XSAnnotation>* const janAnnot);
    DatatypeValidator*  traverseSimpleTypeDecl(const DOMElement* const childElem,
                                               const bool topLevel = true,
                                               int baseRefContext = SchemaSymbols::XSD_EMPTYSET);
    int                 traverseComplexTypeDecl(const DOMElement* const childElem,
                                                const bool topLevel = true,
                                                const XMLCh* const recursingTypeName = 0);
    DatatypeValidator*  traverseByList(const DOMElement* const rootElem,
                                       const DOMElement* const contentElem,
                                       const XMLCh* const typeName,
                                       const XMLCh* const qualifiedName,
                                       const int finalSet,
                                       Janitor<XSAnnotation>* const janAnnot);
    DatatypeValidator*  traverseByRestriction(const DOMElement* const rootElem,
                                              const DOMElement* const contentElem,
                                              const XMLCh* const typeName,
                                              const XMLCh* const qualifiedName,
                                              const int finalSet,
                                              Janitor<XSAnnotation>* const janAnnot);
    DatatypeValidator*  traverseByUnion(const DOMElement* const rootElem,
                                        const DOMElement* const contentElem,
                                        const XMLCh* const typeName,
                                        const XMLCh* const qualifiedName,
                                        const int finalSet,
                                        int baseRefContext,
                                        Janitor<XSAnnotation>* const janAnnot);
    SchemaElementDecl*    traverseElementDecl(const DOMElement* const childElem,
                                            const bool topLevel = false);
    const XMLCh*        traverseNotationDecl(const DOMElement* const childElem);
    const XMLCh*        traverseNotationDecl(const DOMElement* const childElem,
                                             const XMLCh* const name,
                                             const XMLCh* const uriStr);
    ContentSpecNode*    traverseChoiceSequence(const DOMElement* const elemDecl,
                                               const int modelGroupType,
                                               bool& hasChildren);
    ContentSpecNode*    traverseAny(const DOMElement* const anyDecl);
    ContentSpecNode*    traverseAll(const DOMElement* const allElem,
                                    bool& hasChildren);
    XercesGroupInfo*    traverseGroupDecl(const DOMElement* const childElem,
                                          const bool topLevel = true);
    XercesAttGroupInfo* traverseAttributeGroupDecl(const DOMElement* const elem,
                                                   ComplexTypeInfo* const typeInfo,
                                                   const bool topLevel = false);
    XercesAttGroupInfo* traverseAttributeGroupDeclNS(const DOMElement* const elem,
                                                     const XMLCh* const uriStr,
                                                     const XMLCh* const name);
    SchemaAttDef*       traverseAnyAttribute(const DOMElement* const elem);
    void                traverseKey(const DOMElement* const icElem,
                                    SchemaElementDecl* const elemDecl);
    void                traverseUnique(const DOMElement* const icElem,
                                       SchemaElementDecl* const elemDecl);
    void                traverseKeyRef(const DOMElement* const icElem,
                                       SchemaElementDecl* const elemDecl);
    bool                traverseIdentityConstraint(IdentityConstraint* const ic,
                                                   const DOMElement* const icElem);

    // -----------------------------------------------------------------------
    //  Error Reporting methods
    // -----------------------------------------------------------------------
    void reportSchemaError(const XSDLocator* const aLocator,
                           const XMLCh* const msgDomain,
                           const int errorCode);
    void reportSchemaError(const XSDLocator* const aLocator,
                           const XMLCh* const msgDomain,
                           const int errorCode,
                           const XMLCh* const text1,
                           const XMLCh* const text2 = 0,
                           const XMLCh* const text3 = 0,
                           const XMLCh* const text4 = 0);
    void reportSchemaError(const DOMElement* const elem,
                           const XMLCh* const msgDomain,
                           const int errorCode);
    void reportSchemaError(const DOMElement* const elem,
                           const XMLCh* const msgDomain,
                           const int errorCode,
                           const XMLCh* const text1,
                           const XMLCh* const text2 = 0,
                           const XMLCh* const text3 = 0,
                           const XMLCh* const text4 = 0);
    void reportSchemaError(const DOMElement* const elem,
                           const XMLException&     except);

    // -----------------------------------------------------------------------
    //  Private Helper methods
    // -----------------------------------------------------------------------
    /**
      * Keep track of the xs:import found
      */
    bool isImportingNS(const int namespaceURI);
    void addImportedNS(const int namespaceURI);

    /**
      * Retrieved the Namespace mapping from the schema element
      */
    bool retrieveNamespaceMapping(const DOMElement* const elem);

    /**
      * Loop through the children, and traverse the corresponding schema type
      * type declaration (simpleType, complexType, import, ....)
      */
    void processChildren(const DOMElement* const root);
    void preprocessChildren(const DOMElement* const root);

    void preprocessImport(const DOMElement* const elemNode);
    void preprocessInclude(const DOMElement* const elemNode);
    void preprocessRedefine(const DOMElement* const elemNode);

    /**
      * Parameters:
      *   rootElem - top element for a given type declaration
      *   contentElem - content must be annotation? or some other simple content
      *   isEmpty: - true if (annotation?, smth_else), false if (annotation?)
      *   processAnnot - default is true, false if reprocessing a complex type
      *                  since we have already processed the annotation.
      *
      * Check for Annotation if it is present, traverse it. If a sibling is
      * found and it is not an annotation return it, otherwise return 0.
      * Used by traverseSimpleTypeDecl.
      */
    DOMElement* checkContent(const DOMElement* const rootElem,
                               DOMElement* const contentElem,
                               const bool isEmpty, bool processAnnot = true);

    /**
      * Parameters:
      *   contentElem - content element to check
      *
      * Check for identity constraints content.
      */
    const DOMElement* checkIdentityConstraintContent(const DOMElement* const contentElem);

    DatatypeValidator* getDatatypeValidator(const XMLCh* const uriStr,
                                            const XMLCh* const localPartStr);

    /**
      * Process simpleType content of a list|restriction|union
      * Return a dataype validator if valid type, otherwise 0.
      */
    DatatypeValidator* checkForSimpleTypeValidator(const DOMElement* const content,
                                                   int baseRefContext = SchemaSymbols::XSD_EMPTYSET);

    /**
      * Process complexType content of an element
      * Return a ComplexTypeInfo if valid type, otherwise 0.
      */
    ComplexTypeInfo* checkForComplexTypeInfo(const DOMElement* const content);

    /**
      * Return DatatypeValidator available for the baseTypeStr.
      */
    DatatypeValidator* findDTValidator(const DOMElement* const elem,
                                       const XMLCh* const derivedTypeName,
                                       const XMLCh* const baseTypeName,
                                       const int baseRefContext);

    const XMLCh* resolvePrefixToURI(const DOMElement* const elem,
                                    const XMLCh* const prefix);

    /**
      * Return the prefix for a given rawname string
      *
      * Function allocated, caller managed (facm) - pointer to be deleted by
      * caller.
      */
    const XMLCh* getPrefix(const XMLCh* const rawName);

    /**
      * Return the local for a given rawname string
      *
      * caller allocated, caller managed (cacm)
      */
    const XMLCh* getLocalPart(const XMLCh* const rawName);

    /**
      * Process a 'ref' of an Element declaration
      */
    SchemaElementDecl* processElementDeclRef(const DOMElement* const elem,
                                             const XMLCh* const refName);
    void processElemDeclAttrs(const DOMElement* const elem,
                              SchemaElementDecl* const elemDecl,
                              const XMLCh*& valConstraint,
                              bool isTopLevel = false);
    void processElemDeclIC(DOMElement* const elem,
                           SchemaElementDecl* const elemDecl);
    bool checkElemDeclValueConstraint(const DOMElement* const elem,
                                      SchemaElementDecl* const elemDecl,
                                      const XMLCh* const valConstraint,
                                      ComplexTypeInfo* const typeInfo,
                                      DatatypeValidator* const validator);

    /**
      * Process a 'ref' of an Attribute declaration
      */
    void processAttributeDeclRef(const DOMElement* const elem,
                                 ComplexTypeInfo* const typeInfo,
                                 const XMLCh* const refName,
                                 const XMLCh* const useVal,
                                 const XMLCh* const defaultVal,
                                 const XMLCh* const fixedVal);

    /**
      * Process a 'ref' on a group
      */
    XercesGroupInfo* processGroupRef(const DOMElement* const elem,
                                     const XMLCh* const refName);

    /**
      * Process a 'ref' on a attributeGroup
      */
    XercesAttGroupInfo* processAttributeGroupRef(const DOMElement* const elem,
                                                 const XMLCh* const refName,
                                                 ComplexTypeInfo* const typeInfo);

    /**
      * Parse block & final items
      */
    int parseBlockSet(const DOMElement* const elem, const int blockType, const bool isRoot = false);
    int parseFinalSet(const DOMElement* const elem, const int finalType, const bool isRoot = false);

    /**
      * Return true if a name is an identity constraint, otherwise false
      */
    bool isIdentityConstraintName(const XMLCh* const constraintName);

    /**
      * If 'typeStr' belongs to a different schema, return that schema URI,
      * otherwise return 0;
      */
    const XMLCh* checkTypeFromAnotherSchema(const DOMElement* const elem,
                                            const XMLCh* const typeStr);

    /**
      * Return the datatype validator for a given element type attribute if
      * the type is a simple type
      */
    DatatypeValidator* getElementTypeValidator(const DOMElement* const elem,
                                               const XMLCh* const typeStr,
                                               bool& noErrorDetected,
                                               const XMLCh* const otherSchemaURI);

    /**
      * Return the complexType info for a given element type attribute if
      * the type is a complex type
      */
    ComplexTypeInfo* getElementComplexTypeInfo(const DOMElement* const elem,
                                               const XMLCh* const typeStr,
                                               const XMLCh* const otherSchemaURI);

    /**
      * Return global schema element declaration for a given element name
      */
    SchemaElementDecl* getGlobalElemDecl(const DOMElement* const elem,
                                         const XMLCh* const name);

    /**
      * Check validity constraint of a substitutionGroup attribute in
      * an element declaration
      */
    bool isSubstitutionGroupValid(const DOMElement* const elem,
                                  const SchemaElementDecl* const elemDecl,
                                  const ComplexTypeInfo* const typeInfo,
                                  const DatatypeValidator* const validator,
                                  const XMLCh* const elemName,
                                  const bool toEmit = true);

    bool isSubstitutionGroupCircular(SchemaElementDecl* const elemDecl,
                                     SchemaElementDecl* const subsElemDecl);

    void processSubstitutionGroup(const DOMElement* const elem,
                                  SchemaElementDecl* const elemDecl,
                                  ComplexTypeInfo*& typeInfo,
                                  DatatypeValidator*& validator,
                                  const XMLCh* const subsElemQName);

    /**
      * Create a 'SchemaElementDecl' object and add it to SchemaGrammar
      */
    SchemaElementDecl* createSchemaElementDecl(const DOMElement* const elem,
                                               const XMLCh* const name,
                                               bool& isDuplicate,
                                               const XMLCh*& valConstraint,
                                               const bool topLevel);

    /**
      * Return the value of a given attribute name from an element node
      */
    const XMLCh* getElementAttValue(const DOMElement* const elem,
                                    const XMLCh* const attName,
                                    const DatatypeValidator::ValidatorType attType = DatatypeValidator::UnKnown);

    /* return minOccurs */
    int checkMinMax(ContentSpecNode* const specNode,
                     const DOMElement* const elem,
                     const int allContext = Not_All_Context);

    /**
      * Process complex content for a complexType
      */
    void processComplexContent(const DOMElement* const elem,
                               const XMLCh* const typeName,
                               const DOMElement* const childElem,
                               ComplexTypeInfo* const typeInfo,
                               const XMLCh* const baseLocalPart,
                               const bool isMixed,
                               const bool isBaseAnyType = false);

    /**
      * Process "base" information for a complexType
      */
    void processBaseTypeInfo(const DOMElement* const elem,
                             const XMLCh* const baseName,
                             const XMLCh* const localPart,
                             const XMLCh* const uriStr,
                             ComplexTypeInfo* const typeInfo);

    /**
      * Check if base is from another schema
      */
    bool isBaseFromAnotherSchema(const XMLCh* const baseURI);

    /**
      * Get complexType infp from another schema
      */
    ComplexTypeInfo* getTypeInfoFromNS(const DOMElement* const elem,
                                       const XMLCh* const uriStr,
                                       const XMLCh* const localPart);

    DatatypeValidator*
    getAttrDatatypeValidatorNS(const DOMElement* const elem,
                               const XMLCh* localPart,
                               const XMLCh* typeURI);

    /**
      * Returns true if a DOM Element is an attribute or attribute group
      */
    bool isAttrOrAttrGroup(const DOMElement* const elem);

    /**
      * Process attributes of a complex type
      */
    void processAttributes(const DOMElement* const elem,
                           const DOMElement* const attElem,
                           ComplexTypeInfo* const typeInfo,
                           const bool isBaseAnyType = false);

    /**
      * Generate a name for an anonymous type
      */
    const XMLCh* genAnonTypeName(const XMLCh* const prefix);

    void defaultComplexTypeInfo(ComplexTypeInfo* const typeInfo);

    /**
      * Resolve a schema location attribute value to an input source.
      * Caller to delete the returned object.
      */
    InputSource* resolveSchemaLocation
    (
        const XMLCh* const loc
        , const XMLResourceIdentifier::ResourceIdentifierType resourceIdentitiferType
        , const XMLCh* const nameSpace=0
    );

    void restoreSchemaInfo(SchemaInfo* const toRestore,
                           SchemaInfo::ListType const aListType = SchemaInfo::INCLUDE,
                           const unsigned int saveScope = Grammar::TOP_LEVEL_SCOPE);
    void  popCurrentTypeNameStack();

    /**
      * Check whether a mixed content is emptiable or not.
      * Needed to validate element constraint values (defualt, fixed)
      */
    bool emptiableParticle(const ContentSpecNode* const specNode);

    void checkFixedFacet(const DOMElement* const, const XMLCh* const,
                         const DatatypeValidator* const, unsigned int&);
    void buildValidSubstitutionListF(const DOMElement* const elem,
                                     SchemaElementDecl* const,
                                     SchemaElementDecl* const);
    void buildValidSubstitutionListB(const DOMElement* const elem,
                                     SchemaElementDecl* const,
                                     SchemaElementDecl* const);

    void checkEnumerationRequiredNotation(const DOMElement* const elem,
                                          const XMLCh* const name,
                                          const XMLCh* const typeStr);

    void processElements(const DOMElement* const elem,
                         ComplexTypeInfo* const baseTypeInfo,
                         ComplexTypeInfo* const newTypeInfo);

    void processElements(const DOMElement* const elem,
                         XercesGroupInfo* const fromGroup,
                         ComplexTypeInfo* const typeInfo);

    void copyGroupElements(const DOMElement* const elem,
                           XercesGroupInfo* const fromGroup,
                           XercesGroupInfo* const toGroup,
                           ComplexTypeInfo* const typeInfo);

    void copyAttGroupAttributes(const DOMElement* const elem,
                                XercesAttGroupInfo* const fromAttGroup,
                                XercesAttGroupInfo* const toAttGroup,
                                ComplexTypeInfo* const typeInfo);

    void checkForEmptyTargetNamespace(const DOMElement* const elem);

    /**
      * Attribute wild card intersection.
      *
      * Note:
      *    The first parameter will be the result of the intersection, so
      *    we need to make sure that first parameter is a copy of the
      *    actual attribute definition we need to intersect with.
      *
      *    What we need to wory about is: type, defaultType, namespace,
      *    and URI. All remaining data members should be the same.
      */
    void attWildCardIntersection(SchemaAttDef* const resultWildCart,
                                 const SchemaAttDef* const toCompareWildCard);

    /**
      * Attribute wild card union.
      *
      * Note:
      *    The first parameter will be the result of the union, so
      *    we need to make sure that first parameter is a copy of the
      *    actual attribute definition we need to intersect with.
      *
      *    What we need to wory about is: type, defaultType, namespace,
      *    and URI. All remaining data members should be the same.
      */
    void attWildCardUnion(SchemaAttDef* const resultWildCart,
                          const SchemaAttDef* const toCompareWildCard);

    void copyWildCardData(const SchemaAttDef* const srcWildCard,
                          SchemaAttDef* const destWildCard);

    /**
      * Check that the attributes of a type derived by restriction satisfy
      * the constraints of derivation valid restriction
      */
    void checkAttDerivationOK(const DOMElement* const elem,
                              const ComplexTypeInfo* const baseTypeInfo,
                              const ComplexTypeInfo* const childTypeInfo);
    void checkAttDerivationOK(const DOMElement* const elem,
                              const XercesAttGroupInfo* const baseAttGrpInfo,
                              const XercesAttGroupInfo* const childAttGrpInfo);

    /**
      * Check whether a namespace value is valid with respect to wildcard
      * constraint
      */
    bool wildcardAllowsNamespace(const SchemaAttDef* const baseAttWildCard,
                                 const unsigned int nameURI);

    /**
      * Check whether a namespace constraint is an intensional subset of
      * another namespace constraint
      */
    bool isWildCardSubset(const SchemaAttDef* const baseAttWildCard,
                          const SchemaAttDef* const childAttWildCard);

    bool openRedefinedSchema(const DOMElement* const redefineElem);

    /**
      * The purpose of this method is twofold:
      * 1. To find and appropriately modify all information items
      * in redefinedSchema with names that are redefined by children of
      * redefineElem.
      * 2.  To make sure the redefine element represented by
      * redefineElem is valid as far as content goes and with regard to
      * properly referencing components to be redefined.
      *
      *	No traversing is done here!
      * This method also takes actions to find and, if necessary, modify
      * the names of elements in <redefine>'s in the schema that's being
      * redefined.
      */
    void renameRedefinedComponents(const DOMElement* const redefineElem,
                                   SchemaInfo* const redefiningSchemaInfo,
                                   SchemaInfo* const redefinedSchemaInfo);

    /**
      * This method returns true if the redefine component is valid, and if
      * it was possible to revise it correctly.
      */
    bool validateRedefineNameChange(const DOMElement* const redefineChildElem,
                                    const XMLCh* const redefineChildElemName,
                                    const XMLCh* const redefineChildDeclName,
                                    const int redefineNameCounter,
                                    SchemaInfo* const redefiningSchemaInfo);

	/**
      * This function looks among the children of 'redefineChildElem' for a
      * component of type 'redefineChildComponentName'. If it finds one, it
      * evaluates whether its ref attribute contains a reference to
      * 'refChildTypeName'. If it does, it returns 1 + the value returned by
      * calls to itself on all other children.  In all other cases it returns
      * 0 plus the sum of the values returned by calls to itself on
      * redefineChildElem's children. It also resets the value of ref so that
      * it will refer to the renamed type from the schema being redefined.
      */
    int changeRedefineGroup(const DOMElement* const redefineChildElem,
                            const XMLCh* const redefineChildComponentName,
                            const XMLCh* const redefineChildTypeName,
                            const int redefineNameCounter);

    /** This simple function looks for the first occurrence of a
      * 'redefineChildTypeName' item in the redefined schema and appropriately
      * changes the value of its name. If it turns out that what we're looking
      * for is in a <redefine> though, then we just rename it--and it's
      * reference--to be the same.
      */
    void fixRedefinedSchema(const DOMElement* const elem,
                            SchemaInfo* const redefinedSchemaInfo,
                            const XMLCh* const redefineChildComponentName,
                            const XMLCh* const redefineChildTypeName,
                            const int redefineNameCounter);

    void getRedefineNewTypeName(const XMLCh* const oldTypeName,
                                const int redefineCounter,
                                XMLBuffer& newTypeName);

    /**
      * This purpose of this method is threefold:
      * 1. To extract the schema information of included/redefined schema.
      * 2. Rename redefined components.
      * 3. Process components of included/redefined schemas
      */
    void preprocessRedefineInclude(SchemaInfo* const currSchemaInfo);

    /**
      * Update the list of valid substitution groups in the case of circular
      * import.
      */
    void updateCircularSubstitutionList(SchemaInfo* const aSchemaInfo);

    void processKeyRefFor(SchemaInfo* const aSchemaInfo,
                          ValueVectorOf<SchemaInfo*>* const infoList);

    void processAttValue(const XMLCh* const attVal, XMLBuffer& aBuf);

    // routine to generate synthetic annotations
    XSAnnotation* generateSyntheticAnnotation(const DOMElement* const elem
                                             , ValueVectorOf<DOMNode*>* nonXSAttList);

    // routine to validate annotations
    void validateAnnotations();

    // -----------------------------------------------------------------------
    //  Private constants
    // -----------------------------------------------------------------------
    enum
    {
        ES_Block
        , C_Block
        , S_Final
        , EC_Final
        , ECS_Final
    };

    enum ExceptionCodes
    {
        NoException = 0,
        InvalidComplexTypeInfo = 1,
        RecursingElement = 2
    };

    enum
    {
        Elem_Def_Qualified = 1,
        Attr_Def_Qualified = 2
    };

    // -----------------------------------------------------------------------
    //  Private data members
    // -----------------------------------------------------------------------
    bool                                           fFullConstraintChecking;
    int                                            fTargetNSURI;
    int                                            fEmptyNamespaceURI;
    unsigned int                                   fCurrentScope;
    unsigned int                                   fScopeCount;
    unsigned int                                   fAnonXSTypeCount;
    XMLSize_t                                      fCircularCheckIndex;
    const XMLCh*                                   fTargetNSURIString;
    DatatypeValidatorFactory*                      fDatatypeRegistry;
    GrammarResolver*                               fGrammarResolver;
    SchemaGrammar*                                 fSchemaGrammar;
    XMLEntityHandler*                              fEntityHandler;
    XMLErrorReporter*                              fErrorReporter;
    XMLStringPool*                                 fURIStringPool;
    XMLStringPool*                                 fStringPool;
    XMLBuffer                                      fBuffer;
    XMLScanner*                                    fScanner;
    RefHashTableOf<XMLAttDef>*                     fAttributeDeclRegistry;
    RefHashTableOf<ComplexTypeInfo>*               fComplexTypeRegistry;
    RefHashTableOf<XercesGroupInfo>*               fGroupRegistry;
    RefHashTableOf<XercesAttGroupInfo>*            fAttGroupRegistry;
    RefHashTableOf<ElemVector>*                    fIC_ElementsNS;
    RefHashTableOf<SchemaInfo, PtrHasher>*         fPreprocessedNodes;
    SchemaInfo*                                    fSchemaInfo;
    XercesGroupInfo*                               fCurrentGroupInfo;
    XercesAttGroupInfo*                            fCurrentAttGroupInfo;
    ComplexTypeInfo*                               fCurrentComplexType;
    ValueVectorOf<unsigned int>*                   fCurrentTypeNameStack;
    ValueVectorOf<unsigned int>*                   fCurrentGroupStack;
    ValueVectorOf<SchemaElementDecl*>*             fIC_Elements;
    ValueVectorOf<const DOMElement*>*              fDeclStack;
    ValueVectorOf<unsigned int>**                  fGlobalDeclarations;
    ValueVectorOf<DOMNode*>*                       fNonXSAttList;
    ValueVectorOf<int>*                            fImportedNSList;
    RefHashTableOf<ValueVectorOf<DOMElement*>, PtrHasher>* fIC_NodeListNS;
    RefHash2KeysTableOf<XMLCh>*                    fNotationRegistry;
    RefHash2KeysTableOf<XMLCh>*                    fRedefineComponents;
    RefHash2KeysTableOf<IdentityConstraint>*       fIdentityConstraintNames;
    RefHash2KeysTableOf<ElemVector>*               fValidSubstitutionGroups;
    RefHash2KeysTableOf<SchemaInfo>*               fSchemaInfoList;
    RefHash2KeysTableOf<SchemaInfo>*               fCachedSchemaInfoList;
    XSDDOMParser*                                  fParser;
    XSDErrorReporter                               fXSDErrorReporter;
    XSDLocator*                                    fLocator;
    MemoryManager*                                 fMemoryManager;
    MemoryManager*                                 fGrammarPoolMemoryManager;
    XSAnnotation*                                  fAnnotation;
    GeneralAttributeCheck                          fAttributeCheck;

    friend class GeneralAttributeCheck;
    friend class NamespaceScopeManager;
};


// ---------------------------------------------------------------------------
//  TraverseSchema: Helper methods
// ---------------------------------------------------------------------------
inline const XMLCh* TraverseSchema::getPrefix(const XMLCh* const rawName) {

    int colonIndex = XMLString::indexOf(rawName, chColon);

    if (colonIndex == -1 || colonIndex == 0) {
        return XMLUni::fgZeroLenString;
    }

    fBuffer.set(rawName, colonIndex);

    return fStringPool->getValueForId(fStringPool->addOrFind(fBuffer.getRawBuffer()));
}

inline const XMLCh* TraverseSchema::getLocalPart(const XMLCh* const rawName) {

    int    colonIndex = XMLString::indexOf(rawName, chColon);
    XMLSize_t rawNameLen = XMLString::stringLen(rawName);

    if (XMLSize_t(colonIndex + 1) == rawNameLen) {
        return XMLUni::fgZeroLenString;
    }

    if (colonIndex == -1) {
        fBuffer.set(rawName, rawNameLen);
    }
    else {

        fBuffer.set(rawName + colonIndex + 1, rawNameLen - colonIndex - 1);
    }

    return fStringPool->getValueForId(fStringPool->addOrFind(fBuffer.getRawBuffer()));
}

inline void
TraverseSchema::checkForEmptyTargetNamespace(const DOMElement* const elem) {

    const XMLCh* targetNS = getElementAttValue(elem, SchemaSymbols::fgATT_TARGETNAMESPACE);

    if (targetNS && !*targetNS) {
        reportSchemaError(elem, XMLUni::fgXMLErrDomain, XMLErrs::InvalidTargetNSValue);
    }
}

inline bool TraverseSchema::isBaseFromAnotherSchema(const XMLCh* const baseURI)
{
    if (!XMLString::equals(baseURI,fTargetNSURIString)
        && !XMLString::equals(baseURI, SchemaSymbols::fgURI_SCHEMAFORSCHEMA)
        && (baseURI && *baseURI)) {
        //REVISIT, !!!! a hack: for schema that has no
        //target namespace, e.g. personal-schema.xml
        return true;
    }

    return false;
}

inline bool TraverseSchema::isAttrOrAttrGroup(const DOMElement* const elem) {

    const XMLCh* elementName = elem->getLocalName();

    if (XMLString::equals(elementName, SchemaSymbols::fgELT_ATTRIBUTE) ||
        XMLString::equals(elementName, SchemaSymbols::fgELT_ATTRIBUTEGROUP) ||
        XMLString::equals(elementName, SchemaSymbols::fgELT_ANYATTRIBUTE)) {
        return true;
    }

    return false;
}

inline const XMLCh* TraverseSchema::genAnonTypeName(const XMLCh* const prefix) {

    XMLCh anonCountStr[16]; // a count of 15 digits should be enough

    XMLString::sizeToText(fAnonXSTypeCount++, anonCountStr, 15, 10, fMemoryManager);
    fBuffer.set(prefix);
    fBuffer.append(anonCountStr);

    return fStringPool->getValueForId(fStringPool->addOrFind(fBuffer.getRawBuffer()));
}

inline void TraverseSchema::popCurrentTypeNameStack() {

    XMLSize_t stackSize = fCurrentTypeNameStack->size();

    if (stackSize != 0) {
        fCurrentTypeNameStack->removeElementAt(stackSize - 1);
    }
}

inline void
TraverseSchema::copyWildCardData(const SchemaAttDef* const srcWildCard,
                                 SchemaAttDef* const destWildCard) {

    destWildCard->getAttName()->setURI(srcWildCard->getAttName()->getURI());
    destWildCard->setType(srcWildCard->getType());
    destWildCard->setDefaultType(srcWildCard->getDefaultType());
}

inline void TraverseSchema::getRedefineNewTypeName(const XMLCh* const oldTypeName,
                                                   const int redefineCounter,
                                                   XMLBuffer& newTypeName) {

    newTypeName.set(oldTypeName);

    for (int i=0; i < redefineCounter; i++) {
        newTypeName.append(SchemaSymbols::fgRedefIdentifier);
    }
}

inline bool TraverseSchema::isImportingNS(const int namespaceURI) {

    if (!fImportedNSList)
        return false;

    return (fImportedNSList->containsElement(namespaceURI));
}

inline void TraverseSchema::addImportedNS(const int namespaceURI) {

    if (!fImportedNSList) {
        fImportedNSList = new (fMemoryManager) ValueVectorOf<int>(4, fMemoryManager);
    }

    if (!fImportedNSList->containsElement(namespaceURI))
        fImportedNSList->addElement(namespaceURI);
}

XERCES_CPP_NAMESPACE_END

#endif

/**
  * End of file TraverseSchema.hpp
  */
