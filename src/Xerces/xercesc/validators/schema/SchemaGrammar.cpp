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
 * $Id: SchemaGrammar.cpp 883376 2009-11-23 15:45:23Z borisk $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/schema/SchemaGrammar.hpp>
#include <xercesc/validators/schema/ComplexTypeInfo.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xercesc/validators/schema/XercesGroupInfo.hpp>
#include <xercesc/validators/schema/XercesAttGroupInfo.hpp>
#include <xercesc/validators/schema/XMLSchemaDescriptionImpl.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/framework/psvi/XSAnnotation.hpp>

#include <xercesc/internal/XTemplateSerializer.hpp>
#include <xercesc/internal/ValidationContextImpl.hpp>

XERCES_CPP_NAMESPACE_BEGIN

typedef JanitorMemFunCall<SchemaGrammar>    CleanupType;

// ---------------------------------------------------------------------------
//  SchemaGrammar: Constructors and Destructor
// ---------------------------------------------------------------------------
SchemaGrammar::SchemaGrammar(MemoryManager* const manager) :
    fTargetNamespace(0)
    , fElemDeclPool(0)
    , fElemNonDeclPool(0)
    , fGroupElemDeclPool(0)
    , fNotationDeclPool(0)
    , fAttributeDeclRegistry(0)
    , fComplexTypeRegistry(0)
    , fGroupInfoRegistry(0)
    , fAttGroupInfoRegistry(0)
    , fValidSubstitutionGroups(0)
    , fValidationContext(0)
    , fMemoryManager(manager)
    , fGramDesc(0)
    , fAnnotations(0)
    , fValidated(false)
    , fDatatypeRegistry(manager)
    , fScopeCount (0)
    , fAnonTypeCount (0)
{
    CleanupType cleanup(this, &SchemaGrammar::cleanUp);

    //
    //  Init all the pool members.
    //
    //  <TBD> Investigate what the optimum values would be for the various
    //  pools.
    //
    fElemDeclPool = new (fMemoryManager) RefHash3KeysIdPool<SchemaElementDecl>(109, true, 128, fMemoryManager);

    try {
        // should not be necessary now that grammars, once built,
        // are read-only
        // fElemNonDeclPool = new (fMemoryManager) RefHash3KeysIdPool<SchemaElementDecl>(29, true, 128, fMemoryManager);
        fGroupElemDeclPool = new (fMemoryManager) RefHash3KeysIdPool<SchemaElementDecl>(109, false, 128, fMemoryManager);
        fNotationDeclPool = new (fMemoryManager) NameIdPool<XMLNotationDecl>(109, 128, fMemoryManager);
        fValidationContext = new (fMemoryManager) ValidationContextImpl(fMemoryManager);

        //REVISIT: use grammarPool to create
        fGramDesc = new (fMemoryManager) XMLSchemaDescriptionImpl(XMLUni::fgXMLNSURIName, fMemoryManager);

        // Create annotation table
        fAnnotations = new (fMemoryManager) RefHashTableOf<XSAnnotation, PtrHasher>
        (
            29, true, fMemoryManager
        );

        //
        //  Call our own reset method. This lets us have the pool setup stuff
        //  done in just one place (because this stame setup stuff has to be
        //  done every time we are reset.)
        //
        reset();
    }
    catch(const OutOfMemoryException&)
    {
        cleanup.release();

        throw;
    }

    cleanup.release();
}

SchemaGrammar::~SchemaGrammar()
{
    cleanUp();
}


// -----------------------------------------------------------------------
//  Virtual methods
// -----------------------------------------------------------------------
XMLElementDecl* SchemaGrammar::findOrAddElemDecl (const   unsigned int    uriId
        , const XMLCh* const    baseName
        , const XMLCh* const    prefixName
        , const XMLCh* const    qName
        , unsigned int          scope
        ,       bool&           wasAdded )
{
    // See it it exists
    SchemaElementDecl* retVal = (SchemaElementDecl*) getElemDecl(uriId, baseName, qName, scope);

    // if not, then add this in
    if (!retVal)
    {
        retVal = new (fMemoryManager) SchemaElementDecl
        (
            prefixName
            , baseName
            , uriId
            , SchemaElementDecl::Any
            , Grammar::TOP_LEVEL_SCOPE
            , fMemoryManager
        );
        if(!fElemNonDeclPool)
            fElemNonDeclPool = new (fMemoryManager) RefHash3KeysIdPool<SchemaElementDecl>(29, true, 128, fMemoryManager);
        const XMLSize_t elemId = fElemNonDeclPool->put((void*)retVal->getBaseName(), uriId, scope, retVal);
        retVal->setId(elemId);
        wasAdded = true;
    }
     else
    {
        wasAdded = false;
    }
    return retVal;
}

XMLElementDecl* SchemaGrammar::putElemDecl (const   unsigned int    uriId
        , const XMLCh* const    baseName
        , const XMLCh* const    prefixName
        , const XMLCh* const
        , unsigned int          scope
        , const bool            notDeclared)
{
    SchemaElementDecl* retVal = new (fMemoryManager) SchemaElementDecl
    (
        prefixName
        , baseName
        , uriId
        , SchemaElementDecl::Any
        , Grammar::TOP_LEVEL_SCOPE
        , fMemoryManager
    );
    if(notDeclared)
    {
        if(!fElemNonDeclPool)
            fElemNonDeclPool = new (fMemoryManager) RefHash3KeysIdPool<SchemaElementDecl>(29, true, 128, fMemoryManager);
        retVal->setId(fElemNonDeclPool->put((void*)retVal->getBaseName(), uriId, scope, retVal));
    } else
    {
        retVal->setId(fElemDeclPool->put((void*)retVal->getBaseName(), uriId, scope, retVal));
    }
    return retVal;
}

void SchemaGrammar::reset()
{
    //
    //  We need to reset all of the pools.
    //
    fElemDeclPool->removeAll();
    if(fElemNonDeclPool)
        fElemNonDeclPool->removeAll();
    fGroupElemDeclPool->removeAll();
    fNotationDeclPool->removeAll();
    fAnnotations->removeAll();
    fValidated = false;
}


void SchemaGrammar::cleanUp()
{
    delete fElemDeclPool;
    if(fElemNonDeclPool)
        delete fElemNonDeclPool;
    delete fGroupElemDeclPool;
    delete fNotationDeclPool;
    fMemoryManager->deallocate(fTargetNamespace);//delete [] fTargetNamespace;
    delete fAttributeDeclRegistry;
    delete fComplexTypeRegistry;
    delete fGroupInfoRegistry;
    delete fAttGroupInfoRegistry;
    delete fValidSubstitutionGroups;
    delete fValidationContext;
    delete fGramDesc;
    delete fAnnotations;
}

void SchemaGrammar::setGrammarDescription(XMLGrammarDescription* gramDesc)
{
    if ((!gramDesc) ||
        (gramDesc->getGrammarType() != Grammar::SchemaGrammarType))
        return;

    if (fGramDesc)
        delete fGramDesc;

    //adopt the grammar Description
    fGramDesc = (XMLSchemaDescription*) gramDesc;
}

// ---------------------------------------------------------------------------
//  SchemaGrammar: Helper methods
// ---------------------------------------------------------------------------
void SchemaGrammar::putAnnotation(void* key, XSAnnotation* const annotation)
{
    fAnnotations->put(key, annotation);
}

void SchemaGrammar::addAnnotation(XSAnnotation* const annotation)
{
    XSAnnotation* lAnnot = fAnnotations->get(this);

    if (lAnnot)
        lAnnot->setNext(annotation);
    else
        fAnnotations->put(this, annotation);
}

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_TOCREATE(SchemaGrammar)

void SchemaGrammar::serialize(XSerializeEngine& serEng)
{

    /***
     * don't serialize ValidationContext* fValidationContext;
     *                                    fElemNonDeclPool
     ***/

    Grammar::serialize(serEng);

    if (serEng.isStoring())
    {
        //serialize DatatypeValidatorFactory first
        fDatatypeRegistry.serialize(serEng);

        /***
         *
         * Serialize RefHash3KeysIdPool<SchemaElementDecl>* fElemDeclPool;
         * Serialize RefHash3KeysIdPool<SchemaElementDecl>* fGroupElemDeclPool;
         *
        ***/
        XTemplateSerializer::storeObject(fElemDeclPool, serEng);
        XTemplateSerializer::storeObject(fGroupElemDeclPool, serEng);

        /***
         * Serialize NameIdPool<XMLNotationDecl>*           fNotationDeclPool;
         ***/
        XTemplateSerializer::storeObject(fNotationDeclPool, serEng);

        /***
         *
         * Serialize RefHashTableOf<XMLAttDef>*             fAttributeDeclRegistry;
         * Serialize RefHashTableOf<ComplexTypeInfo>*       fComplexTypeRegistry;
         * Serialize RefHashTableOf<XercesGroupInfo>*       fGroupInfoRegistry;
         * Serialize RefHashTableOf<XercesAttGroupInfo>*    fAttGroupInfoRegistry;
         * Serialize RefHashTableOf<XMLRefInfo>*            fIDRefList;
         *
         ***/

        XTemplateSerializer::storeObject(fAttributeDeclRegistry, serEng);
        XTemplateSerializer::storeObject(fComplexTypeRegistry, serEng);
        XTemplateSerializer::storeObject(fGroupInfoRegistry, serEng);
        XTemplateSerializer::storeObject(fAttGroupInfoRegistry, serEng);

        /***
         * Serialize RefHash2KeysTableOf<ElemVector>*       fValidSubstitutionGroups;
         ***/
        XTemplateSerializer::storeObject(fValidSubstitutionGroups, serEng);

        /***
         * Serialize RefHashTableOf<XSAnnotation>*       fAnnotations;
         ***/
        XTemplateSerializer::storeObject(fAnnotations, serEng);

        serEng.writeString(fTargetNamespace);
        serEng<<fValidated;

        /***
         * serialize() method shall be used to store object
         * which has been created in ctor
         ***/
        fGramDesc->serialize(serEng);

    }
    else
    {
        fDatatypeRegistry.serialize(serEng);

        /***
         *
         * Deserialize RefHash3KeysIdPool<SchemaElementDecl>* fElemDeclPool;
         * Deserialize RefHash3KeysIdPool<SchemaElementDecl>* fGroupElemDeclPool;
         *
        ***/
        XTemplateSerializer::loadObject(&fElemDeclPool, 109, true, 128, serEng);
        XTemplateSerializer::loadObject(&fGroupElemDeclPool, 109, true, 128, serEng);

        /***
         * Deserialize NameIdPool<XMLNotationDecl>*           fNotationDeclPool;
         ***/
        XTemplateSerializer::loadObject(&fNotationDeclPool, 109, 128, serEng);

        /***
         *
         * Deserialize RefHashTableOf<XMLAttDef>*             fAttributeDeclRegistry;
         * Deserialize RefHashTableOf<ComplexTypeInfo>*       fComplexTypeRegistry;
         * Deserialize RefHashTableOf<XercesGroupInfo>*       fGroupInfoRegistry;
         * Deserialize RefHashTableOf<XercesAttGroupInfo>*    fAttGroupInfoRegistry;
         * Deserialize RefHashTableOf<XMLRefInfo>*            fIDRefList;
         *
         ***/

        XTemplateSerializer::loadObject(&fAttributeDeclRegistry, 29, true, serEng);
        XTemplateSerializer::loadObject(&fComplexTypeRegistry, 29, true, serEng);
        XTemplateSerializer::loadObject(&fGroupInfoRegistry, 13, true, serEng);
        XTemplateSerializer::loadObject(&fAttGroupInfoRegistry, 13, true, serEng);

        /***
         * Deserialize RefHash2KeysTableOf<ElemVector>*       fValidSubstitutionGroups;
         ***/
        XTemplateSerializer::loadObject(&fValidSubstitutionGroups, 29, true, serEng);

        /***
         * Deserialize RefHashTableOf<XSAnnotation>*       fAnnotations;
         ***/
        XTemplateSerializer::loadObject(&fAnnotations, 29, true, serEng);

        serEng.readString(fTargetNamespace);
        serEng>>fValidated;

        /***
         * serialize() method shall be used to load object
         * which has been created in ctor
         ***/
        fGramDesc->serialize(serEng);

    }
}

XERCES_CPP_NAMESPACE_END
