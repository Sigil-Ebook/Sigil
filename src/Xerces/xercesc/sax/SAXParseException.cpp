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
 * $Id: SAXParseException.cpp 672273 2008-06-27 13:57:00Z borisk $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    <xercesc/util/XMLString.hpp>
#include    <xercesc/sax/Locator.hpp>
#include    <xercesc/sax/SAXParseException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  SAXParseException: Constructors and Destructor
// ---------------------------------------------------------------------------
SAXParseException::SAXParseException(const  XMLCh* const    message
                                    , const Locator&        locator
                                    , MemoryManager* const  manager) :
    SAXException(message, manager)
    , fColumnNumber(locator.getColumnNumber())
    , fLineNumber(locator.getLineNumber())
    , fPublicId(XMLString::replicate(locator.getPublicId(), manager))
    , fSystemId(XMLString::replicate(locator.getSystemId(), manager))
{
}

SAXParseException::SAXParseException(const  XMLCh* const    message
                                    , const XMLCh* const    publicId
                                    , const XMLCh* const    systemId
                                    , const XMLFileLoc   lineNumber
                                    , const XMLFileLoc   columnNumber
                                    , MemoryManager* const  manager) :
    SAXException(message, manager)
    , fColumnNumber(columnNumber)
    , fLineNumber(lineNumber)
    , fPublicId(XMLString::replicate(publicId, manager))
    , fSystemId(XMLString::replicate(systemId, manager))
{
}

SAXParseException::SAXParseException(const SAXParseException& toCopy) :

    SAXException(toCopy)
    , fColumnNumber(toCopy.fColumnNumber)
    , fLineNumber(toCopy.fLineNumber)
    , fPublicId(0)
    , fSystemId(0)
{
    fPublicId = XMLString::replicate(toCopy.fPublicId, toCopy.fMemoryManager);
    fSystemId = XMLString::replicate(toCopy.fSystemId, toCopy.fMemoryManager);
}

SAXParseException::~SAXParseException()
{
    fMemoryManager->deallocate(fPublicId);//XMLString::release(&fPublicId);
    fMemoryManager->deallocate(fSystemId);//XMLString::release(&fSystemId);
}


// ---------------------------------------------------------------------------
//  SAXParseException: Public operators
// ---------------------------------------------------------------------------
SAXParseException&
SAXParseException::operator=(const SAXParseException& toAssign)
{
    if (this == &toAssign)
        return *this;

    fMemoryManager->deallocate(fPublicId);//XMLString::release(&fPublicId);
    fMemoryManager->deallocate(fSystemId);//XMLString::release(&fSystemId);

    this->SAXException::operator =(toAssign);
    fColumnNumber = toAssign.fColumnNumber;
    fLineNumber = toAssign.fLineNumber;

    fPublicId = XMLString::replicate(toAssign.fPublicId, fMemoryManager);
    fSystemId = XMLString::replicate(toAssign.fSystemId, fMemoryManager);

    return *this;
}


// ---------------------------------------------------------------------------
//  SAXParseException: Getter methods
// ---------------------------------------------------------------------------
const XMLCh* SAXParseException::getPublicId() const
{
    return fPublicId;
}

const XMLCh* SAXParseException::getSystemId() const
{
    return fSystemId;
}

XMLFileLoc SAXParseException::getLineNumber() const
{
    return fLineNumber;
}

XMLFileLoc SAXParseException::getColumnNumber() const
{
    return fColumnNumber;
}

XERCES_CPP_NAMESPACE_END
