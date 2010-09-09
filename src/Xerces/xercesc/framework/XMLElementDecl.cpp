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

/**
 * $Id: XMLElementDecl.cpp 679359 2008-07-24 11:15:19Z borisk $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/framework/XMLElementDecl.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/XMLUni.hpp>

#include <xercesc/validators/schema/SchemaElementDecl.hpp>
#include <xercesc/validators/DTD/DTDElementDecl.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  XMLElementDecl: Public, static data
// ---------------------------------------------------------------------------
const unsigned int  XMLElementDecl::fgInvalidElemId    = 0xFFFFFFFE;
const unsigned int  XMLElementDecl::fgPCDataElemId     = 0xFFFFFFFF;
const XMLCh         XMLElementDecl::fgPCDataElemName[] =
{
        chPound, chLatin_P, chLatin_C, chLatin_D, chLatin_A
    ,   chLatin_T, chLatin_A, chNull
};



// ---------------------------------------------------------------------------
//  XMLElementDecl: Destructor
// ---------------------------------------------------------------------------
XMLElementDecl::~XMLElementDecl()
{
    delete fElementName;
}

// ---------------------------------------------------------------------------
//  XMLElementDecl: Setter Methods
// ---------------------------------------------------------------------------
void
XMLElementDecl::setElementName(const XMLCh* const       prefix
                            , const XMLCh* const        localPart
                            , const int                 uriId )
{
    if (fElementName)
        fElementName->setName(prefix, localPart, uriId);
    else
        fElementName = new (fMemoryManager) QName(prefix, localPart, uriId, fMemoryManager);
}

void
XMLElementDecl::setElementName(const XMLCh* const       rawName
                            , const int                 uriId )
{
    if (fElementName)
        fElementName->setName(rawName, uriId);
    else
        fElementName = new (fMemoryManager) QName(rawName, uriId, fMemoryManager);
}

void
XMLElementDecl::setElementName(const QName* const    elementName)
{
    if (fElementName)
        fElementName->setValues(*elementName);
    else
        fElementName = new (fMemoryManager) QName(*elementName);
}

// ---------------------------------------------------------------------------
//  ElementDecl: Hidden constructors
// ---------------------------------------------------------------------------
XMLElementDecl::XMLElementDecl(MemoryManager* const manager) :

    fMemoryManager(manager)
    , fElementName(0)
    , fCreateReason(XMLElementDecl::NoReason)
    , fId(XMLElementDecl::fgInvalidElemId)
    , fExternalElement(false)
{
}

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_NOCREATE(XMLElementDecl)

void XMLElementDecl::serialize(XSerializeEngine& serEng)
{

    if (serEng.isStoring())
    {
        serEng<<fElementName;
        serEng<<(int) fCreateReason;
        serEng.writeSize (fId);
        serEng<<fExternalElement;
    }
    else
    {
        serEng>>fElementName;

        int i;
        serEng>>i;
        fCreateReason=(CreateReasons)i;

        serEng.readSize (fId);
        serEng>>fExternalElement;
    }

}

void
XMLElementDecl::storeElementDecl(XSerializeEngine&        serEng
                               , XMLElementDecl*    const element)
{
    if (element)
    {
        serEng<<(int) element->getObjectType();
        serEng<<element;
    }
    else
    {
        serEng<<(int) UnKnown;
    }
}

XMLElementDecl*
XMLElementDecl::loadElementDecl(XSerializeEngine& serEng)
{
    int type;
    serEng>>type;

    switch((XMLElementDecl::objectType)type)
    {
    case Schema:
        SchemaElementDecl* schemaElementDecl;
        serEng>>schemaElementDecl;
        return schemaElementDecl;
    case DTD:
        DTDElementDecl* dtdElementDecl;
        serEng>>dtdElementDecl;
        return dtdElementDecl;
    case UnKnown:
         //fall through
    default:
        return 0;
    }
}

XERCES_CPP_NAMESPACE_END
