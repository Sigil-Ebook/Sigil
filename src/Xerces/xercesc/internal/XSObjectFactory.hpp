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
 * $Id: XSObjectFactory.hpp 678409 2008-07-21 13:08:10Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XSOBJECTFACTORY_HPP)
#define XERCESC_INCLUDE_GUARD_XSOBJECTFACTORY_HPP

#include <xercesc/framework/psvi/XSConstants.hpp>
#include <xercesc/util/RefHashTableOf.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XSObject;
class XSAttributeUse;
class XSAttributeDeclaration;
class XSModel;
class XSElementDeclaration;
class XSSimpleTypeDefinition;
class XSComplexTypeDefinition;
class XSModelGroupDefinition;
class XSAttributeGroupDefinition;
class XSWildcard;
class XSParticle;
class XSAnnotation;
class XSNamespaceItem;
class XSNotationDeclaration;
class SchemaAttDef;
class SchemaElementDecl;
class DatatypeValidator;
class ContentSpecNode;
class ComplexTypeInfo;
class XercesGroupInfo;
class XercesAttGroupInfo;
class XSIDCDefinition;
class IdentityConstraint;
class XMLNotationDecl;

/**
 * Factory class to create various XSObject(s)
 * Used by XSModel
 */
class XMLPARSER_EXPORT XSObjectFactory : public XMemory
{
public:
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    XSObjectFactory(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    ~XSObjectFactory();

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and destructor
    // -----------------------------------------------------------------------
    XSObjectFactory(const XSObjectFactory&);
    XSObjectFactory& operator=(const XSObjectFactory&);

    // -----------------------------------------------------------------------
    //  factory methods
    // -----------------------------------------------------------------------
    XSParticle* createModelGroupParticle
    (
        const ContentSpecNode* const node
        , XSModel* const             xsModel
    );

    XSAttributeDeclaration* addOrFind
    (
        SchemaAttDef* const attDef
        , XSModel* const xsModel
        , XSComplexTypeDefinition* const enclosingTypeDef = 0
    );

    XSSimpleTypeDefinition* addOrFind
    (
        DatatypeValidator* const validator
        , XSModel* const xsModel
        , bool isAnySimpleType = false
    );

    XSElementDeclaration* addOrFind
    (
        SchemaElementDecl* const elemDecl
        , XSModel* const xsModel
        , XSComplexTypeDefinition* const enclosingTypeDef = 0
    );

    XSComplexTypeDefinition* addOrFind
    (
        ComplexTypeInfo* const typeInfo
        , XSModel* const xsModel
    );

    XSIDCDefinition* addOrFind
    (
        IdentityConstraint* const ic
        , XSModel* const xsModel
    );

    XSNotationDeclaration* addOrFind
    (
        XMLNotationDecl* const notDecl
        , XSModel* const xsModel
    );

    XSAttributeUse* createXSAttributeUse
    (
        XSAttributeDeclaration* const xsAttDecl
        , XSModel* const xsModel
    );
    XSWildcard* createXSWildcard
    (
        SchemaAttDef* const attDef
        , XSModel* const xsModel
    );

    XSWildcard* createXSWildcard
    (
        const ContentSpecNode* const rootNode
        , XSModel* const xsModel
    );

    XSModelGroupDefinition* createXSModelGroupDefinition
    (
        XercesGroupInfo* const groupInfo
        , XSModel* const xsModel
    );

    XSAttributeGroupDefinition* createXSAttGroupDefinition
    (
        XercesAttGroupInfo* const attGroupInfo
        , XSModel* const xsModel
    );

    // -----------------------------------------------------------------------
    //  Helper methods
    // -----------------------------------------------------------------------
    // creates a particle corresponding to an element
    XSParticle* createElementParticle
    (
        const ContentSpecNode* const rootNode
        , XSModel* const             xsModel
    );

    // creates a particle corresponding to a wildcard
    XSParticle* createWildcardParticle
    (
        const ContentSpecNode* const rootNode
        , XSModel* const             xsModel
    );

    XSAnnotation* getAnnotationFromModel
    (
        XSModel* const xsModel
        , const void* const key
    );

    void buildAllParticles
    (
        const ContentSpecNode* const rootNode
        , XSParticleList* const particleList
        , XSModel* const xsModel
    );

    void buildChoiceSequenceParticles
    (
        const ContentSpecNode* const rootNode
        , XSParticleList* const particleList
        , XSModel* const xsModel
    );

    void putObjectInMap
    (
        void* key
        , XSObject* const object
    );

    XSObject* getObjectFromMap
    (
        void* key
    );

    void processFacets
    (
        DatatypeValidator* const dv
        , XSModel* const xsModel
        , XSSimpleTypeDefinition* const xsST
    );

    void processAttUse
    (
        SchemaAttDef* const attDef
        , XSAttributeUse* const xsAttUse
    );

    bool isMultiValueFacetDefined(DatatypeValidator* const dv);

    // make XSModel our friend
    friend class XSModel;

    // -----------------------------------------------------------------------
    //  Private Data Members
    //
    //  fMemoryManager
    //      The memory manager used to create various XSObject(s).
    // -----------------------------------------------------------------------
    MemoryManager*                       fMemoryManager;
    RefHashTableOf<XSObject, PtrHasher>* fXercesToXSMap;
    RefVectorOf<XSObject>*               fDeleteVector;
};

inline XSObject* XSObjectFactory::getObjectFromMap(void* key)
{
    return fXercesToXSMap->get(key);
}


XERCES_CPP_NAMESPACE_END

#endif
