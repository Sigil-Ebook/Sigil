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
 * $Id: InputSource.cpp 471747 2006-11-06 14:31:56Z amassari $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    <xercesc/sax/InputSource.hpp>
#include    <xercesc/util/XMLString.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  InputSource: Destructor
// ---------------------------------------------------------------------------
InputSource::~InputSource()
{
    fMemoryManager->deallocate(fEncoding);
    fMemoryManager->deallocate(fPublicId);
    fMemoryManager->deallocate(fSystemId);
}


// ---------------------------------------------------------------------------
//  InputSource: Setter methods
// ---------------------------------------------------------------------------
void InputSource::setEncoding(const XMLCh* const encodingStr)
{
    fMemoryManager->deallocate(fEncoding);
    fEncoding = XMLString::replicate(encodingStr, fMemoryManager);
}


void InputSource::setPublicId(const XMLCh* const publicId)
{
    fMemoryManager->deallocate(fPublicId);
    fPublicId = XMLString::replicate(publicId, fMemoryManager);
}


void InputSource::setSystemId(const XMLCh* const systemId)
{
    fMemoryManager->deallocate(fSystemId);
    fSystemId = XMLString::replicate(systemId, fMemoryManager);
}



// ---------------------------------------------------------------------------
//  InputSource: Hidden Constructors
// ---------------------------------------------------------------------------
InputSource::InputSource(MemoryManager* const manager) :

    fMemoryManager(manager)
    , fEncoding(0)
    , fPublicId(0)
    , fSystemId(0)
    , fFatalErrorIfNotFound(true)
{
}

InputSource::InputSource(const XMLCh* const systemId,
                         MemoryManager* const manager) :

    fMemoryManager(manager)
    , fEncoding(0)
    , fPublicId(0)
    , fSystemId(0)
    , fFatalErrorIfNotFound(true)
{
    fSystemId = XMLString::replicate(systemId, fMemoryManager);
}

InputSource::InputSource(const  XMLCh* const   systemId
                        , const XMLCh* const   publicId
                        , MemoryManager* const manager) :

    fMemoryManager(manager)
    , fEncoding(0)
    , fPublicId(0)
    , fSystemId(0)
    , fFatalErrorIfNotFound(true)
{
    fPublicId = XMLString::replicate(publicId, fMemoryManager);
    fSystemId = XMLString::replicate(systemId, fMemoryManager);
}

InputSource::InputSource(const char* const systemId,
                         MemoryManager* const manager) :

    fMemoryManager(manager)
    , fEncoding(0)
    , fPublicId(0)
    , fSystemId(0)
    , fFatalErrorIfNotFound(true)
{
    fSystemId = XMLString::transcode(systemId, fMemoryManager);
}

InputSource::InputSource(const  char* const systemId
                        , const char* const publicId
                        , MemoryManager* const manager) :

    fMemoryManager(manager)
    , fEncoding(0)
    , fPublicId(0)
    , fSystemId(0)
    , fFatalErrorIfNotFound(true)
{
    fPublicId = XMLString::transcode(publicId, fMemoryManager);
    fSystemId = XMLString::transcode(systemId, fMemoryManager);
}

XERCES_CPP_NAMESPACE_END

