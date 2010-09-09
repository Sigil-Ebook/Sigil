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
 * $Id: Wrapper4DOMLSInput.cpp 471747 2006-11-06 14:31:56Z amassari $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/framework/Wrapper4DOMLSInput.hpp>
#include <xercesc/dom/DOMLSInput.hpp>
#include <xercesc/dom/DOMLSResourceResolver.hpp>
#include <xercesc/util/NullPointerException.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/framework/LocalFileInputSource.hpp>
#include <xercesc/framework/URLInputSource.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Wrapper4DOMLSInput: Constructor and Destructor
// ---------------------------------------------------------------------------
Wrapper4DOMLSInput::Wrapper4DOMLSInput(DOMLSInput* const inputSource,
                                       DOMLSResourceResolver* entityResolver,
                                       const bool adoptFlag,
                                       MemoryManager* const  manager) :
    InputSource(manager)
    , fAdoptInputSource(adoptFlag)
    , fForceXMLChEncoding(false)
    , fInputSource(inputSource)
    , fEntityResolver(entityResolver)
{
    if (!inputSource)
        ThrowXMLwithMemMgr(NullPointerException, XMLExcepts::CPtr_PointerIsZero, getMemoryManager());
}

Wrapper4DOMLSInput::~Wrapper4DOMLSInput()
{
    if (fAdoptInputSource)
        delete fInputSource;
}


// ---------------------------------------------------------------------------
//  Wrapper4DOMLSInput: Getter methods
// ---------------------------------------------------------------------------
bool Wrapper4DOMLSInput::getIssueFatalErrorIfNotFound() const
{
    return fInputSource->getIssueFatalErrorIfNotFound();
}

const XMLCh* Wrapper4DOMLSInput::getEncoding() const
{
    if(fForceXMLChEncoding)
        return XMLUni::fgXMLChEncodingString;
    return fInputSource->getEncoding();
}

const XMLCh* Wrapper4DOMLSInput::getSystemId() const
{
    return fInputSource->getSystemId();
}

const XMLCh* Wrapper4DOMLSInput::getPublicId() const
{
    return fInputSource->getPublicId();
}


// ---------------------------------------------------------------------------
//  Wrapper4DOMLSInput: Setter methods
// ---------------------------------------------------------------------------
void Wrapper4DOMLSInput::setIssueFatalErrorIfNotFound(const bool flag)
{
    fInputSource->setIssueFatalErrorIfNotFound(flag);
}


void Wrapper4DOMLSInput::setEncoding(const XMLCh* const encodingStr)
{
    fInputSource->setEncoding(encodingStr);
}


void Wrapper4DOMLSInput::setPublicId(const XMLCh* const publicId)
{
    fInputSource->setPublicId(publicId);
}


void Wrapper4DOMLSInput::setSystemId(const XMLCh* const systemId)
{
    fInputSource->setSystemId(systemId);
}


// ---------------------------------------------------------------------------
//  Wrapper4DOMLSInput: Stream methods
// ---------------------------------------------------------------------------
BinInputStream* Wrapper4DOMLSInput::makeStream() const
{
    // The LSParser will use the LSInput object to determine how to read data. The LSParser will look at the different inputs specified in the 
    // LSInput in the following order to know which one to read from, the first one that is not null and not an empty string will be used:
    //   1. LSInput.characterStream
    //   2. LSInput.byteStream
    //   3. LSInput.stringData
    //   4. LSInput.systemId
    //   5. LSInput.publicId
    InputSource* binStream=fInputSource->getByteStream();
    if(binStream)
        return binStream->makeStream();
    const XMLCh* xmlString=fInputSource->getStringData();
    if(xmlString)
    {
        MemBufInputSource is((const XMLByte*)xmlString, XMLString::stringLen(xmlString)*sizeof(XMLCh), "", false, getMemoryManager());
        is.setCopyBufToStream(false);
        return is.makeStream();
    }
    const XMLCh* szSystemId=fInputSource->getSystemId();
    if(szSystemId)
    {
        XMLURL urlTmp(getMemoryManager());
        if (urlTmp.setURL(szSystemId, fInputSource->getBaseURI(), urlTmp) && !urlTmp.isRelative())
        {
            URLInputSource src(urlTmp, getMemoryManager());
            return src.makeStream();
        }          
        LocalFileInputSource src(szSystemId, getMemoryManager());
        return src.makeStream();
    }
    const XMLCh* szPublicId=fInputSource->getPublicId();
    if(szPublicId && fEntityResolver)
    {
        DOMLSInput* is = fEntityResolver->resolveResource(XMLUni::fgDOMDTDType, 0, szPublicId, 0, fInputSource->getBaseURI());
        if (is)
            return Wrapper4DOMLSInput(is, fEntityResolver, true, getMemoryManager()).makeStream();
    }

    return 0;
}

XERCES_CPP_NAMESPACE_END

