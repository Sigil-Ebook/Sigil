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
 * $Id: URLInputSource.cpp 471747 2006-11-06 14:31:56Z amassari $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/BinFileInputStream.hpp>
#include <xercesc/util/Janitor.hpp>
#include <xercesc/util/XMLURL.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/framework/URLInputSource.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  URLInputSource: Constructors and Destructor
// ---------------------------------------------------------------------------
URLInputSource::URLInputSource( const XMLURL&         urlId
                              , MemoryManager* const  manager) :

    InputSource(manager)
    , fURL(urlId)
{
    setSystemId(fURL.getURLText());
}

URLInputSource::URLInputSource( const XMLCh* const    baseId
                              , const XMLCh* const    systemId
                              , MemoryManager* const  manager) :
    InputSource(manager)
    , fURL(baseId, systemId)
{
    // Create a URL that will build up the full URL and store as the system id
    setSystemId(fURL.getURLText());
}

URLInputSource::URLInputSource( const XMLCh* const    baseId
                              , const XMLCh* const    systemId
                              , const XMLCh* const    publicId
                              , MemoryManager* const  manager) :
    InputSource(0, publicId, manager)
    , fURL(baseId, systemId)
{
    setSystemId(fURL.getURLText());
}

URLInputSource::URLInputSource( const XMLCh* const    baseId
                              , const char* const     systemId
                              , MemoryManager* const  manager) :
    InputSource(manager)
    , fURL(baseId, systemId)
{
    setSystemId(fURL.getURLText());
}

URLInputSource::URLInputSource( const   XMLCh* const   baseId
                                , const char* const    systemId
                                , const char* const    publicId
                                , MemoryManager* const  manager) :
    InputSource(0, publicId, manager)
    , fURL(baseId, systemId)
{
    setSystemId(fURL.getURLText());
}

URLInputSource::~URLInputSource()
{
}


// ---------------------------------------------------------------------------
//  URLInputSource: Implementation of the input source interface
// ---------------------------------------------------------------------------
BinInputStream* URLInputSource::makeStream() const
{
    // Ask the URL to create us an appropriate input stream
    return fURL.makeNewStream();
}

XERCES_CPP_NAMESPACE_END

