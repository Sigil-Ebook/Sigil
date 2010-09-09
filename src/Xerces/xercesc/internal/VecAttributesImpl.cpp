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
 * $Id: VecAttributesImpl.cpp 672311 2008-06-27 16:05:01Z borisk $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/Janitor.hpp>
#include <xercesc/internal/VecAttributesImpl.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Constructors and Destructor
// ---------------------------------------------------------------------------
VecAttributesImpl::VecAttributesImpl() :

    fAdopt(false)
    , fCount(0)
    , fVector(0)
    , fScanner(0)
{
}

VecAttributesImpl::~VecAttributesImpl()
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
XMLSize_t VecAttributesImpl::getLength() const
{
    return fCount;
}

const XMLCh* VecAttributesImpl::getURI(const XMLSize_t index) const
{
    // since this func really needs to be const, like the rest, not sure how we
    // make it const and re-use the fURIBuffer member variable.  we're currently
    // creating a buffer each time you need a URI.  there has to be a better
    // way to do this...

    //XMLBuffer tempBuf;
    if (index >= fCount) {


        return 0;
     }
    //fValidator->getURIText(fVector->elementAt(index)->getURIId(), tempBuf) ;
    //return tempBuf.getRawBuffer() ;
    return fScanner->getURIText(fVector->elementAt(index)->getURIId());
}

const XMLCh* VecAttributesImpl::getLocalName(const XMLSize_t index) const
{
    if (index >= fCount) {
        return 0;
     }
    return fVector->elementAt(index)->getName();
}

const XMLCh* VecAttributesImpl::getQName(const XMLSize_t index) const
{
    if (index >= fCount) {
        return 0;
     }
    return fVector->elementAt(index)->getQName();
}

const XMLCh* VecAttributesImpl::getType(const XMLSize_t index) const
{
    if (index >= fCount) {
        return 0;
     }
    return XMLAttDef::getAttTypeString(fVector->elementAt(index)->getType(), fVector->getMemoryManager());
}

const XMLCh* VecAttributesImpl::getValue(const XMLSize_t index) const
{
    if (index >= fCount) {
        return 0;
     }
    return fVector->elementAt(index)->getValue();
}

bool VecAttributesImpl::getIndex(const XMLCh* const uri,
                                 const XMLCh* const localPart,
                                 XMLSize_t& index) const
{
  //
  //  Search the vector for the attribute with the given name and return
  //  its type.
  //
  XMLBuffer uriBuffer(1023, fVector->getMemoryManager()) ;
  for (index = 0; index < fCount; index++)
  {
    const XMLAttr* curElem = fVector->elementAt(index);

    fScanner->getURIText(curElem->getURIId(), uriBuffer) ;

    if ( (XMLString::equals(curElem->getName(), localPart)) &&
         (XMLString::equals(uriBuffer.getRawBuffer(), uri)) )
      return true;
  }
  return false;
}

int VecAttributesImpl::getIndex(const XMLCh* const uri, const XMLCh* const localPart ) const
{
    //
    //  Search the vector for the attribute with the given name and return
    //  its type.
    //
    XMLBuffer uriBuffer(1023, fVector->getMemoryManager()) ;
    for (XMLSize_t index = 0; index < fCount; index++)
    {
        const XMLAttr* curElem = fVector->elementAt(index);

        fScanner->getURIText(curElem->getURIId(), uriBuffer) ;

        if ( (XMLString::equals(curElem->getName(), localPart)) &&
             (XMLString::equals(uriBuffer.getRawBuffer(), uri)) )
            return (int)index ;
    }
    return -1;
}

bool VecAttributesImpl::getIndex(const XMLCh* const qName,
                                 XMLSize_t& index) const
{
  //
  //  Search the vector for the attribute with the given name and return
  //  its type.
  //
  for (index = 0; index < fCount; index++)
  {
    const XMLAttr* curElem = fVector->elementAt(index);

    if (XMLString::equals(curElem->getQName(), qName))
      return true;
  }
  return false;
}

int VecAttributesImpl::getIndex(const XMLCh* const qName ) const
{
    //
    //  Search the vector for the attribute with the given name and return
    //  its type.
    //
    for (XMLSize_t index = 0; index < fCount; index++)
    {
        const XMLAttr* curElem = fVector->elementAt(index);

        if (XMLString::equals(curElem->getQName(), qName))
            return (int)index ;
    }
    return -1;
}

const XMLCh* VecAttributesImpl::getType(const XMLCh* const uri, const XMLCh* const localPart ) const
{
  XMLSize_t i;

  if (getIndex(uri, localPart, i))
    return getType(i);
  else
    return 0;
}

const XMLCh* VecAttributesImpl::getType(const XMLCh* const qName) const
{
  XMLSize_t i;

  if (getIndex(qName, i))
    return getType(i);
  else
    return 0;
}

const XMLCh* VecAttributesImpl::getValue(const XMLCh* const uri, const XMLCh* const localPart ) const
{
  XMLSize_t i;

  if (getIndex(uri, localPart, i))
    return getValue(i);
  else
    return 0;
}

const XMLCh* VecAttributesImpl::getValue(const XMLCh* const qName) const
{
    XMLSize_t i;

  if (getIndex(qName, i))
    return getValue(i);
  else
    return 0;
}

// ---------------------------------------------------------------------------
//  Setter methods
// ---------------------------------------------------------------------------
void VecAttributesImpl::setVector(const   RefVectorOf<XMLAttr>* const srcVec
                                , const XMLSize_t                count
                                , const XMLScanner * const        scanner
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
    fScanner = scanner ;
}

XERCES_CPP_NAMESPACE_END
