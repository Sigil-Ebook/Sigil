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
 * $Id: DOMLSOutputImpl.cpp 471747 2006-11-06 14:31:56Z amassari $
 */

#include "DOMLSOutputImpl.hpp"

#include <xercesc/util/XMLString.hpp>

XERCES_CPP_NAMESPACE_BEGIN

DOMLSOutputImpl::DOMLSOutputImpl(MemoryManager* const manager /*= XMLPlatformUtils::fgMemoryManager*/)
:fByteStream(0)
,fEncoding(0)
,fSystemId(0)
,fMemoryManager(manager)
{
}

DOMLSOutputImpl::~DOMLSOutputImpl()
{
    fMemoryManager->deallocate(fEncoding);
    fMemoryManager->deallocate(fSystemId);
}

void DOMLSOutputImpl::setByteStream(XMLFormatTarget* stream)
{
    fByteStream=stream;
}

void DOMLSOutputImpl::setEncoding(const XMLCh* const encodingStr)
{
    fMemoryManager->deallocate(fEncoding);
    fEncoding = XMLString::replicate(encodingStr, fMemoryManager);
}

void DOMLSOutputImpl::setSystemId(const XMLCh* const systemId)
{
    fMemoryManager->deallocate(fSystemId);
    fSystemId = XMLString::replicate(systemId, fMemoryManager);
}

void DOMLSOutputImpl::release()
{
    delete this;
}


XERCES_CPP_NAMESPACE_END

