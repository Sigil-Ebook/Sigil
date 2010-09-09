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
 * $Id: VecAttrListImpl.cpp 672273 2008-06-27 13:57:00Z borisk $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/Janitor.hpp>
#include <xercesc/internal/VecAttrListImpl.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Constructors and Destructor
// ---------------------------------------------------------------------------
VecAttrListImpl::VecAttrListImpl() :

    fAdopt(false)
    , fCount(0)
    , fVector(0)
{
}

VecAttrListImpl::~VecAttrListImpl()
{
    //
    //  Note that some compilers can't deal with the fact that the pointer
    //  is to a const object, so we have to cast off the const'ness here!
    //
    if (fAdopt)
        delete (RefVectorOf<XMLAttr>*)fVector;
}


// ---------------------------------------------------------------------------
//  Implementation of the attribute list interface
// ---------------------------------------------------------------------------
XMLSize_t VecAttrListImpl::getLength() const
{
    return fCount;
}

const XMLCh* VecAttrListImpl::getName(const XMLSize_t index) const
{
    if (index >= fCount) {
        return 0;
    }
    return fVector->elementAt(index)->getQName();
}

const XMLCh* VecAttrListImpl::getType(const XMLSize_t index) const
{
    if (index >= fCount) {
        return 0;
    }
    return XMLAttDef::getAttTypeString(fVector->elementAt(index)->getType(), fVector->getMemoryManager());
}

const XMLCh* VecAttrListImpl::getValue(const XMLSize_t index) const
{
    if (index >= fCount) {
        return 0;
    }
    return fVector->elementAt(index)->getValue();
}

const XMLCh* VecAttrListImpl::getType(const XMLCh* const name) const
{
    //
    //  Search the vector for the attribute with the given name and return
    //  its type.
    //
    for (XMLSize_t index = 0; index < fCount; index++)
    {
        const XMLAttr* curElem = fVector->elementAt(index);

        if (XMLString::equals(curElem->getQName(), name))
            return XMLAttDef::getAttTypeString(curElem->getType(), fVector->getMemoryManager());
    }
    return 0;
}

const XMLCh* VecAttrListImpl::getValue(const XMLCh* const name) const
{
    //
    //  Search the vector for the attribute with the given name and return
    //  its type.
    //
    for (XMLSize_t index = 0; index < fCount; index++)
    {
        const XMLAttr* curElem = fVector->elementAt(index);

        if (XMLString::equals(curElem->getQName(), name))
            return curElem->getValue();
    }
    return 0;
}

const XMLCh* VecAttrListImpl::getValue(const char* const name) const
{
    // Temporarily transcode the name for lookup
    XMLCh* wideName = XMLString::transcode(name, XMLPlatformUtils::fgMemoryManager);
    ArrayJanitor<XMLCh> janName(wideName, XMLPlatformUtils::fgMemoryManager);

    //
    //  Search the vector for the attribute with the given name and return
    //  its type.
    //
    for (XMLSize_t index = 0; index < fCount; index++)
    {
        const XMLAttr* curElem = fVector->elementAt(index);

        if (XMLString::equals(curElem->getQName(), wideName))
            return curElem->getValue();
    }
    return 0;
}



// ---------------------------------------------------------------------------
//  Setter methods
// ---------------------------------------------------------------------------
void VecAttrListImpl::setVector(const   RefVectorOf<XMLAttr>* const srcVec
                                , const XMLSize_t                   count
                                , const bool                        adopt)
{
    //
    //  Delete the previous vector (if any) if we are adopting. Note that some
    //  compilers can't deal with the fact that the pointer is to a const
    //  object, so we have to cast off the const'ness here!
    //
    if (fAdopt)
        delete (RefVectorOf<XMLAttr>*)fVector;

    fAdopt = adopt;
    fCount = count;
    fVector = srcVec;
}

XERCES_CPP_NAMESPACE_END
