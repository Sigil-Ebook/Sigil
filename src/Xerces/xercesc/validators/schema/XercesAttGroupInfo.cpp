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
 * $Id: XercesAttGroupInfo.cpp 676911 2008-07-15 13:27:32Z amassari $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/schema/XercesAttGroupInfo.hpp>
#include <xercesc/util/QName.hpp>

#include <xercesc/internal/XTemplateSerializer.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  XercesAttGroupInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
XercesAttGroupInfo::XercesAttGroupInfo(MemoryManager* const manager)
    : fTypeWithId(false)
    , fNameId(0)
    , fNamespaceId(0)
    , fAttributes(0)
    , fAnyAttributes(0)
    , fCompleteWildCard(0)
    , fMemoryManager(manager)
{

}

XercesAttGroupInfo::XercesAttGroupInfo(unsigned int attGroupNameId,
                                       unsigned int attGroupNamespaceId,
                                       MemoryManager* const manager)
    : fTypeWithId(false)
    , fNameId(attGroupNameId)
    , fNamespaceId(attGroupNamespaceId)
    , fAttributes(0)
    , fAnyAttributes(0)
    , fCompleteWildCard(0)
    , fMemoryManager(manager)
{

}

XercesAttGroupInfo::~XercesAttGroupInfo()
{
    delete fAttributes;
    delete fAnyAttributes;
    delete fCompleteWildCard;
}

bool XercesAttGroupInfo::containsAttribute(const XMLCh* const name,
                                           const unsigned int uri) {

    if (fAttributes) {

        XMLSize_t attCount = fAttributes->size();

        if (attCount) {

            for (XMLSize_t i=0; i < attCount; i++) {

                QName* attName = fAttributes->elementAt(i)->getAttName();

                if (attName->getURI() == uri &&
                    XMLString::equals(attName->getLocalPart(),name)) {
                    return true;
                }
            }
        }
    }

    return false;
}

// ---------------------------------------------------------------------------
//  XercesAttGroupInfo: Getter methods
// ---------------------------------------------------------------------------
const SchemaAttDef* XercesAttGroupInfo::getAttDef(const XMLCh* const baseName,
                                                  const int uriId) const {

    // If no list, then return a null
    if (!fAttributes)
        return 0;

    XMLSize_t attSize = fAttributes->size();

    for (XMLSize_t i=0; i<attSize; i++) {

        const SchemaAttDef* attDef = fAttributes->elementAt(i);
        QName* attName = attDef->getAttName();

        if (uriId == (int) attName->getURI() &&
			XMLString::equals(baseName, attName->getLocalPart())) {

            return attDef;
        }
    }

    return 0;
}

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_TOCREATE(XercesAttGroupInfo)

void XercesAttGroupInfo::serialize(XSerializeEngine& serEng)
{

    if (serEng.isStoring())
    {
        serEng<<fTypeWithId;
        serEng<<fNameId;
        serEng<<fNamespaceId;

        /***
         *
         * Serialize RefVectorOf<SchemaAttDef>* fAttributes;
         *
         ***/
        XTemplateSerializer::storeObject(fAttributes, serEng);

        /***
         *
         * Serialize RefVectorOf<SchemaAttDef>* fAnyAttributes;
         *
         ***/
        XTemplateSerializer::storeObject(fAnyAttributes, serEng);

        serEng<<fCompleteWildCard;
    }
    else
    {
        serEng>>fTypeWithId;
        serEng>>fNameId;
        serEng>>fNamespaceId;

        /***
         *
         * Deserialize RefVectorOf<SchemaAttDef>* fAttributes;
         *
         ***/
        XTemplateSerializer::loadObject(&fAttributes, 4, true, serEng);

        /***
         *
         * Deserialize RefVectorOf<SchemaAttDef>* fAnyAttributes;
         *
         ***/

        XTemplateSerializer::loadObject(&fAnyAttributes, 2, true, serEng);

        serEng>>fCompleteWildCard;
    }

}

XERCES_CPP_NAMESPACE_END

/**
  * End of file XercesAttGroupInfo.cpp
  */


