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
 * $Id: SchemaElementDecl.cpp 609971 2008-01-08 13:30:47Z amassari $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/validators/schema/SchemaAttDefList.hpp>
#include <xercesc/validators/schema/SchemaElementDecl.hpp>
#include <xercesc/validators/schema/identity/IdentityConstraint.hpp>

#include <xercesc/internal/XTemplateSerializer.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  SchemaElementDecl: Constructors and Destructor
// ---------------------------------------------------------------------------
SchemaElementDecl::SchemaElementDecl(MemoryManager* const manager) :
    XMLElementDecl(manager)
    , fModelType(Any)
    , fPSVIScope(PSVIDefs::SCP_ABSENT)
    , fEnclosingScope(Grammar::TOP_LEVEL_SCOPE)
    , fFinalSet(0)
    , fBlockSet(0)    
    , fMiscFlags(0)
    , fDefaultValue(0)
    , fComplexTypeInfo(0)
    , fAttDefs(0)        
    , fIdentityConstraints(0)
    , fAttWildCard(0)
    , fSubstitutionGroupElem(0)
    , fDatatypeValidator(0)    
{
}

SchemaElementDecl::SchemaElementDecl(const XMLCh* const                  prefix
                                   , const XMLCh* const                  localPart
                                   , const int                           uriId
                                   , const SchemaElementDecl::ModelTypes type
                                   , const unsigned int                  enclosingScope
                                   , MemoryManager* const                manager) :
    XMLElementDecl(manager)
    , fModelType(type)
    , fPSVIScope(PSVIDefs::SCP_ABSENT)
    , fEnclosingScope(enclosingScope)
    , fFinalSet(0)
    , fBlockSet(0)    
    , fMiscFlags(0)
    , fDefaultValue(0)
    , fComplexTypeInfo(0)
    , fAttDefs(0)          
    , fIdentityConstraints(0)
    , fAttWildCard(0)
    , fSubstitutionGroupElem(0)
    , fDatatypeValidator(0)
{
    setElementName(prefix, localPart, uriId);
}

SchemaElementDecl::SchemaElementDecl(const QName* const                  elementName
                                   , const SchemaElementDecl::ModelTypes type
                                   , const unsigned int                  enclosingScope
                                   , MemoryManager* const                manager) :
    XMLElementDecl(manager)
    , fModelType(type)
    , fPSVIScope(PSVIDefs::SCP_ABSENT)
    , fEnclosingScope(enclosingScope)
    , fFinalSet(0)
    , fBlockSet(0)    
    , fMiscFlags(0)
    , fDefaultValue(0)
    , fComplexTypeInfo(0)
    , fAttDefs(0)        
    , fIdentityConstraints(0)
    , fAttWildCard(0)
    , fSubstitutionGroupElem(0)
    , fDatatypeValidator(0)
{
    setElementName(elementName);
}

SchemaElementDecl::~SchemaElementDecl()
{
    getMemoryManager()->deallocate(fDefaultValue);//delete [] fDefaultValue;
    delete fAttDefs;
    delete fIdentityConstraints;
    delete fAttWildCard;
}


// ---------------------------------------------------------------------------
//  SchemaElementDecl: XMLElementDecl virtual interface implementation
// ---------------------------------------------------------------------------
XMLAttDefList& SchemaElementDecl::getAttDefList() const
{
    if (!fComplexTypeInfo)
	{
        ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::DV_InvalidOperation, getMemoryManager());
    }

	return fComplexTypeInfo->getAttDefList();
}


XMLElementDecl::CharDataOpts SchemaElementDecl::getCharDataOpts() const
{
    SchemaElementDecl::ModelTypes modelType = fModelType;

    if (fComplexTypeInfo) {
        modelType = (SchemaElementDecl::ModelTypes) fComplexTypeInfo->getContentType();
    }

    XMLElementDecl::CharDataOpts retVal;
    switch(modelType)
    {
        case Children :
        case ElementOnlyEmpty :
            retVal = XMLElementDecl::SpacesOk;
            break;

        case Empty :
            retVal = XMLElementDecl::NoCharData;
            break;

        default :
            retVal = XMLElementDecl::AllCharData;
            break;
    }
    return retVal;
}


bool SchemaElementDecl::hasAttDefs() const
{
    if (fComplexTypeInfo) {
        return fComplexTypeInfo->hasAttDefs();
    }

    // If the collection hasn't been faulted in, then no att defs
    return false;

}

const XMLCh*
SchemaElementDecl::getFormattedContentModel() const
{
    if (fComplexTypeInfo) {
        return fComplexTypeInfo->getFormattedContentModel();
    }
    return 0;
}

// ---------------------------------------------------------------------------
//  SchemaElementDecl: Getter methods
// ---------------------------------------------------------------------------
const SchemaAttDef* SchemaElementDecl::getAttDef(const XMLCh* const baseName, const int uriId) const
{
    if (fComplexTypeInfo) {
        return fComplexTypeInfo->getAttDef(baseName, uriId);
    }

    // If no complex type, then return a null
    return 0;

}

SchemaAttDef* SchemaElementDecl::getAttDef(const XMLCh* const baseName, const int uriId)
{
    if (fComplexTypeInfo) {
        return fComplexTypeInfo->getAttDef(baseName, uriId);
    }

    // If no complex type, then return a null
    return 0;
}

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_TOCREATE(SchemaElementDecl)

void SchemaElementDecl::serialize(XSerializeEngine& serEng)
{

    XMLElementDecl::serialize(serEng);

    if (serEng.isStoring())
    {
        serEng<<(int)fModelType;
        serEng<<(int)fPSVIScope;        

        serEng<<fEnclosingScope;
        serEng<<fFinalSet;
        serEng<<fBlockSet;
        serEng<<fMiscFlags;

        serEng.writeString(fDefaultValue);

        serEng<<fComplexTypeInfo;

        /***
         * Serialize RefHash2KeysTableOf<SchemaAttDef>* fAttDefs;
         ***/

        XTemplateSerializer::storeObject(fAttDefs, serEng);        
        
        /***
         * Serialize RefVectorOf<IdentityConstraint>*   fIdentityConstraints;
         ***/
        XTemplateSerializer::storeObject(fIdentityConstraints, serEng);

        serEng<<fAttWildCard;
        serEng<<fSubstitutionGroupElem;
        DatatypeValidator::storeDV(serEng, fDatatypeValidator);                
    }
    else
    {
        int i;
        serEng>>i;
        fModelType = (ModelTypes)i;
        serEng>>i;
        fPSVIScope = (PSVIDefs::PSVIScope)i;   

        serEng>>fEnclosingScope;
        serEng>>fFinalSet;
        serEng>>fBlockSet;
        serEng>>fMiscFlags;

        serEng.readString(fDefaultValue);

        serEng>>fComplexTypeInfo;

        /***
         * DeSerialize RefHash2KeysTableOf<SchemaAttDef>* fAttDefs;
         ***/
        XTemplateSerializer::loadObject(&fAttDefs, 29, true, serEng);                

        /***
         * DeSerialize RefVectorOf<IdentityConstraint>*   fIdentityConstraints;
         ***/
        XTemplateSerializer::loadObject(&fIdentityConstraints, 16, true, serEng);

        serEng>>fAttWildCard;
        serEng>>fSubstitutionGroupElem;
        fDatatypeValidator = DatatypeValidator::loadDV(serEng);
    }
}

XMLElementDecl::objectType  SchemaElementDecl::getObjectType() const
{
    return Schema;
}

XERCES_CPP_NAMESPACE_END
