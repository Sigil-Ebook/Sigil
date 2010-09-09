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
 * $Id: SchemaAttDef.cpp 679359 2008-07-24 11:15:19Z borisk $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/framework/XMLElementDecl.hpp>
#include <xercesc/validators/schema/SchemaAttDef.hpp>

#include <xercesc/internal/XTemplateSerializer.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  SchemaAttDef: Implementation of the XMLAttDef interface
// ---------------------------------------------------------------------------
const XMLCh* SchemaAttDef::getFullName() const
{
   return fAttName->getRawName();
}

// ---------------------------------------------------------------------------
//  SchemaAttDef: Constructors and Destructor
// ---------------------------------------------------------------------------
SchemaAttDef::SchemaAttDef(MemoryManager* const manager) :
    XMLAttDef(XMLAttDef::CData, XMLAttDef::Implied, manager)
    , fElemId(XMLElementDecl::fgInvalidElemId)
    , fPSVIScope(PSVIDefs::SCP_ABSENT)
    , fAttName(0)
    , fDatatypeValidator(0)
    , fNamespaceList(0)
    , fBaseAttDecl(0)
{
}

SchemaAttDef::SchemaAttDef( const XMLCh* const           prefix
                          , const XMLCh* const           localPart
                          , const int                    uriId
                          , const XMLAttDef::AttTypes    type
                          , const XMLAttDef::DefAttTypes defType
                          , MemoryManager* const         manager) :
    XMLAttDef(type, defType, manager)
    , fElemId(XMLElementDecl::fgInvalidElemId)
    , fPSVIScope(PSVIDefs::SCP_ABSENT)
    , fDatatypeValidator(0)
    , fNamespaceList(0)
    , fBaseAttDecl(0)
{
    fAttName = new (manager) QName(prefix, localPart, uriId, manager);
}

SchemaAttDef::SchemaAttDef( const XMLCh* const           prefix
                          , const XMLCh* const           localPart
                          , const int                    uriId
                          , const XMLCh* const           attValue
                          , const XMLAttDef::AttTypes    type
                          , const XMLAttDef::DefAttTypes defType
                          , const XMLCh* const           enumValues
                          , MemoryManager* const         manager) :

    XMLAttDef(attValue, type, defType, enumValues, manager)
    , fElemId(XMLElementDecl::fgInvalidElemId)
    , fPSVIScope(PSVIDefs::SCP_ABSENT)
    , fDatatypeValidator(0)
    , fNamespaceList(0)
    , fBaseAttDecl(0)
{
    fAttName = new (manager) QName(prefix, localPart, uriId, manager);
}

SchemaAttDef::SchemaAttDef(const SchemaAttDef* other) :

    XMLAttDef(other->getValue(), other->getType(),
              other->getDefaultType(), other->getEnumeration(),
              other->getMemoryManager())
    , fElemId(XMLElementDecl::fgInvalidElemId)
    , fPSVIScope(other->fPSVIScope)
    , fAttName(0)
    , fDatatypeValidator(other->fDatatypeValidator)
    , fNamespaceList(0)
    , fBaseAttDecl(other->fBaseAttDecl)
{
    QName* otherName = other->getAttName();
    fAttName = new (getMemoryManager()) QName(otherName->getPrefix(),
                         otherName->getLocalPart(), otherName->getURI(),
                         getMemoryManager());

    if (other->fNamespaceList && other->fNamespaceList->size()) {
        fNamespaceList = new (getMemoryManager()) ValueVectorOf<unsigned int>(*(other->fNamespaceList));
    }
}

SchemaAttDef::~SchemaAttDef()
{
   delete fAttName;
   delete fNamespaceList;
}


// ---------------------------------------------------------------------------
//  SchemaAttDef: Setter methods
// ---------------------------------------------------------------------------
void SchemaAttDef::setAttName(const XMLCh* const        prefix
                            , const XMLCh* const        localPart
                            , const int                 uriId )
{
   fAttName->setName(prefix, localPart, uriId);
}

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_TOCREATE(SchemaAttDef)

void SchemaAttDef::serialize(XSerializeEngine& serEng)
{

    XMLAttDef::serialize(serEng);

    if (serEng.isStoring())
    {
        serEng.writeSize (fElemId);
        serEng<<(int)fPSVIScope;

        serEng<<fAttName;

        DatatypeValidator::storeDV(serEng, fDatatypeValidator);

        /***
         * Serialize ValueVectorOf<unsigned int>
         ***/
        XTemplateSerializer::storeObject(fNamespaceList, serEng);

        serEng<<fBaseAttDecl;
    }
    else
    {

        serEng.readSize (fElemId);
        int i;
        serEng>>i;
        fPSVIScope = (PSVIDefs::PSVIScope)i;

        serEng>>fAttName;

        fDatatypeValidator    = DatatypeValidator::loadDV(serEng);

        /***
         * Deserialize ValueVectorOf<unsigned int>
         ***/
        XTemplateSerializer::loadObject(&fNamespaceList, 8, false, serEng);

        serEng>>fBaseAttDecl;
    }
}


XERCES_CPP_NAMESPACE_END
