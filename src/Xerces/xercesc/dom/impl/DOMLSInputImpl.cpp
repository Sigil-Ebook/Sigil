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
 * $Id: DOMLSInputImpl.cpp 752848 2009-03-12 12:44:40Z amassari $
 */

#include "DOMLSInputImpl.hpp"

#include <xercesc/util/XMLString.hpp>

XERCES_CPP_NAMESPACE_BEGIN

DOMLSInputImpl::DOMLSInputImpl(MemoryManager* const manager /*= XMLPlatformUtils::fgMemoryManager*/)
:fStringData(0)
,fByteStream(0)
,fEncoding(0)
,fPublicId(0)
,fSystemId(0)
,fBaseURI(0)
,fIssueFatalErrorIfNotFound(true)
,fMemoryManager(manager)
{
}

DOMLSInputImpl::~DOMLSInputImpl()
{
    fMemoryManager->deallocate(fEncoding);
    fMemoryManager->deallocate(fPublicId);
    fMemoryManager->deallocate(fSystemId);
    fMemoryManager->deallocate(fBaseURI);
}

void DOMLSInputImpl::setStringData(const XMLCh* data)
{
    fStringData=data;
    setEncoding(XMLUni::fgXMLChEncodingString);
}

void DOMLSInputImpl::setByteStream(InputSource* stream)
{
    fByteStream=stream;
}

void DOMLSInputImpl::setEncoding(const XMLCh* const encodingStr)
{
    fMemoryManager->deallocate(fEncoding);
    fEncoding = XMLString::replicate(encodingStr, fMemoryManager);
}

void DOMLSInputImpl::setPublicId(const XMLCh* const publicId)
{
    fMemoryManager->deallocate(fPublicId);
    fPublicId = XMLString::replicate(publicId, fMemoryManager);
}

void DOMLSInputImpl::setSystemId(const XMLCh* const systemId)
{
    fMemoryManager->deallocate(fSystemId);
    fSystemId = XMLString::replicate(systemId, fMemoryManager);
}

void DOMLSInputImpl::setBaseURI(const XMLCh* const baseURI)
{
    fMemoryManager->deallocate(fBaseURI);
    fBaseURI = XMLString::replicate(baseURI, fMemoryManager);
}

void DOMLSInputImpl::setIssueFatalErrorIfNotFound(bool flag)
{
    fIssueFatalErrorIfNotFound=flag;
}

void DOMLSInputImpl::release()
{
    delete this;
}


XERCES_CPP_NAMESPACE_END

