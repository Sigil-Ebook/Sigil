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
 * $Id: XMLNotationDecl.cpp 679359 2008-07-24 11:15:19Z borisk $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/framework/XMLNotationDecl.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  XMLNotationDecl: Constructors and operators
// ---------------------------------------------------------------------------
XMLNotationDecl::XMLNotationDecl(MemoryManager* const manager) :

    fId(0)
    , fNameSpaceId(0)
    , fName(0)
    , fPublicId(0)
    , fSystemId(0)
    , fBaseURI(0)
    , fMemoryManager(manager)
{
}

typedef JanitorMemFunCall<XMLNotationDecl>  CleanupType;

XMLNotationDecl::XMLNotationDecl( const XMLCh* const   notName
                                , const XMLCh* const   pubId
                                , const XMLCh* const   sysId
                                , const XMLCh* const   baseURI
                                , MemoryManager* const manager) :
    fId(0)
    , fNameSpaceId(0)
    , fName(0)
    , fPublicId(0)
    , fSystemId(0)
    , fBaseURI(0)
    , fMemoryManager(manager)
{
    CleanupType cleanup(this, &XMLNotationDecl::cleanUp);

    try
    {
        fName = XMLString::replicate(notName, fMemoryManager);
        fPublicId = XMLString::replicate(pubId, fMemoryManager);
        fSystemId = XMLString::replicate(sysId, fMemoryManager);
        fBaseURI  = XMLString::replicate(baseURI, fMemoryManager);
    }
    catch(const OutOfMemoryException&)
    {
        cleanup.release();

        throw;
    }

    cleanup.release();
}

XMLNotationDecl::~XMLNotationDecl()
{
    cleanUp();
}


// -----------------------------------------------------------------------
//  Setter methods
// -----------------------------------------------------------------------
void XMLNotationDecl::setName(const XMLCh* const notName)
{
    // Clean up the current name stuff and replicate the passed name
    if (fName)
        fMemoryManager->deallocate(fName);

    fName = XMLString::replicate(notName, fMemoryManager);
}



// ---------------------------------------------------------------------------
//  XMLNotationDecl: Private helper methods
// ---------------------------------------------------------------------------
void XMLNotationDecl::cleanUp()
{
    fMemoryManager->deallocate(fName);
    fMemoryManager->deallocate(fPublicId);
    fMemoryManager->deallocate(fSystemId);
    fMemoryManager->deallocate(fBaseURI);
}

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_TOCREATE(XMLNotationDecl)

void XMLNotationDecl::serialize(XSerializeEngine& serEng)
{
    if (serEng.isStoring())
    {
        serEng.writeSize (fId);
        serEng<<fNameSpaceId;
        serEng.writeString(fName);
        serEng.writeString(fPublicId);
        serEng.writeString(fSystemId);
        serEng.writeString(fBaseURI);
    }
    else
    {
        serEng.readSize (fId);
        serEng>>fNameSpaceId;
        serEng.readString(fName);
        serEng.readString(fPublicId);
        serEng.readString(fSystemId);
        serEng.readString(fBaseURI);
    }
}


XERCES_CPP_NAMESPACE_END
