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
 * $Id: XMLDTDDescriptionImpl.cpp 471747 2006-11-06 14:31:56Z amassari $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/DTD/XMLDTDDescriptionImpl.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  XMLDTDDescriptionImpl: constructor and destructor
// ---------------------------------------------------------------------------
XMLDTDDescriptionImpl::XMLDTDDescriptionImpl(const XMLCh*          const  systemId
                                           ,       MemoryManager*  const  memMgr  )
:XMLDTDDescription(memMgr)
,fSystemId(0)
,fRootName(0)
{
    if (systemId)
        fSystemId = XMLString::replicate(systemId, memMgr);
}

XMLDTDDescriptionImpl::~XMLDTDDescriptionImpl()
{
    if (fSystemId)
        XMLGrammarDescription::getMemoryManager()->deallocate((void*)fSystemId);

    if (fRootName)
        XMLGrammarDescription::getMemoryManager()->deallocate((void*)fRootName);
}
             
const XMLCh* XMLDTDDescriptionImpl::getGrammarKey() const
{
    return getSystemId();
}
              
const XMLCh* XMLDTDDescriptionImpl::getRootName() const
{ 
    return fRootName; 
}

const XMLCh* XMLDTDDescriptionImpl::getSystemId() const
{ 
    return fSystemId; 
}

void XMLDTDDescriptionImpl::setRootName(const XMLCh* const rootName)
{
    if (fRootName)
    {
        XMLGrammarDescription::getMemoryManager()->deallocate((void*)fRootName);
        fRootName = 0;
    }

    if (rootName)
        fRootName = XMLString::replicate(rootName, XMLGrammarDescription::getMemoryManager()); 
}        

void XMLDTDDescriptionImpl::setSystemId(const XMLCh* const systemId)
{
    if (fSystemId)
    {
        XMLGrammarDescription::getMemoryManager()->deallocate((void*)fSystemId);
        fSystemId = 0;
    }

    if (systemId)
        fSystemId = XMLString::replicate(systemId, XMLGrammarDescription::getMemoryManager()); 
}        

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_TOCREATE(XMLDTDDescriptionImpl)

void XMLDTDDescriptionImpl::serialize(XSerializeEngine& serEng)
{
    XMLDTDDescription::serialize(serEng);

    if (serEng.isStoring())
    {
        serEng.writeString(fSystemId);
        serEng.writeString(fRootName);
    }
    else
    {
        if (fSystemId)
        {
            XMLGrammarDescription::getMemoryManager()->deallocate((void*)fSystemId);
        }

        serEng.readString((XMLCh*&)fSystemId);

        //the original root name which came from the ctor needs deallocated
        if (fRootName)
        {
            XMLGrammarDescription::getMemoryManager()->deallocate((void*)fRootName);
        }

        serEng.readString((XMLCh*&)fRootName);
    }

}

XMLDTDDescriptionImpl::XMLDTDDescriptionImpl(MemoryManager* const memMgr)
:XMLDTDDescription(memMgr)
,fSystemId(0)
,fRootName(0)
{
}

XERCES_CPP_NAMESPACE_END
