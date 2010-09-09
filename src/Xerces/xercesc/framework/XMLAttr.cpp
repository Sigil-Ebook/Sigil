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
 * $Id: XMLAttr.cpp 901107 2010-01-20 08:45:02Z borisk $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/framework/XMLAttr.hpp>
#include <xercesc/framework/MemoryManager.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  XMLAttr: Constructors and Destructor
// ---------------------------------------------------------------------------
XMLAttr::XMLAttr(MemoryManager* const manager) :

      fSpecified(false)
    , fType(XMLAttDef::CData)
    , fValueBufSz(0)
    , fValue(0)
    , fAttName(0)
    , fMemoryManager(manager)
{
    fAttName = new (fMemoryManager) QName(fMemoryManager);
}

typedef JanitorMemFunCall<XMLAttr>  CleanupType;

XMLAttr::XMLAttr(   const   unsigned int        uriId
                    , const XMLCh* const        attrName
                    , const XMLCh* const        attrPrefix
                    , const XMLCh* const        attrValue
                    , const XMLAttDef::AttTypes type
                    , const bool                specified
                    , MemoryManager* const      manager
                    , DatatypeValidator*
                    , const bool /*isSchema*/ ):

      fSpecified(specified)
    , fType(type)
    , fValueBufSz(0)
    , fValue(0)
    , fAttName(0)
    , fMemoryManager(manager)
{
    CleanupType cleanup(this, &XMLAttr::cleanUp);

    try
    {
        //
        //  Just call the local setters to set up everything. Too much
        //  work is required to replicate that functionality here.
        //
        fAttName = new (fMemoryManager) QName(attrPrefix, attrName, uriId, fMemoryManager);
        setValue(attrValue);
    }
    catch(const OutOfMemoryException&)
    {
        cleanup.release();

        throw;
    }

    cleanup.release();
}

XMLAttr::XMLAttr(   const   unsigned int        uriId
                    , const XMLCh* const        rawName
                    , const XMLCh* const        attrValue
                    , const XMLAttDef::AttTypes type
                    , const bool                specified
                    , MemoryManager* const      manager
                    , DatatypeValidator *
                    , const bool /*isSchema*/ ):

      fSpecified(specified)
    , fType(type)
    , fValueBufSz(0)
    , fValue(0)
    , fAttName(0)
    , fMemoryManager(manager)
{
    CleanupType cleanup(this, &XMLAttr::cleanUp);

    try
    {
        //  Just call the local setters to set up everything. Too much
        //  work is required to replicate that functionality here.
        fAttName = new (fMemoryManager) QName(rawName, uriId, fMemoryManager);
        setValue(attrValue);
    }
    catch(const OutOfMemoryException&)
    {
        cleanup.release();

        throw;
    }

    cleanup.release();
}


// ---------------------------------------------------------------------------
//  XMLAttr: Getter methods
// ---------------------------------------------------------------------------
const XMLCh* XMLAttr::getQName() const
{
    return fAttName->getRawName();
}


// ---------------------------------------------------------------------------
//  XMLAttr: Setter methods
// ---------------------------------------------------------------------------
void XMLAttr::setName(  const   unsigned int    uriId
                        , const XMLCh* const    attrName
                        , const XMLCh* const    attrPrefix)
{
    fAttName->setName(attrPrefix, attrName, uriId);
}


void XMLAttr::setURIId(const unsigned int uriId)
{
    fAttName->setURI(uriId);
}


void XMLAttr::setValue(const XMLCh* const newValue)
{
    const XMLSize_t newLen = XMLString::stringLen(newValue);
    if (!fValueBufSz || (newLen > fValueBufSz))
    {
        fMemoryManager->deallocate(fValue); //delete [] fValue;
        fValue = 0;
        fValueBufSz = newLen + 8;
        fValue = (XMLCh*) fMemoryManager->allocate((fValueBufSz+1) * sizeof(XMLCh)); //new XMLCh[fValueBufSz + 1];
    }
    XMLString::moveChars(fValue, newValue, newLen + 1);
}


// ---------------------------------------------------------------------------
//  XMLAttr: Private, helper methods
// ---------------------------------------------------------------------------
void XMLAttr::cleanUp()
{
    delete fAttName;
    fMemoryManager->deallocate(fValue); //delete [] fValue;
}

XERCES_CPP_NAMESPACE_END
