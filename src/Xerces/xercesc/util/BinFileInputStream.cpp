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
 * $Id: BinFileInputStream.cpp 670359 2008-06-22 13:43:45Z borisk $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/BinFileInputStream.hpp>
#include <xercesc/util/Janitor.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLExceptMsgs.hpp>
#include <xercesc/util/XMLString.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  BinFileInputStream: Constructors and Destructor
// ---------------------------------------------------------------------------
BinFileInputStream::BinFileInputStream(const XMLCh* const fileName
                                       , MemoryManager* const manager) :

    fSource(XMLPlatformUtils::openFile(fileName, manager))
  , fMemoryManager(manager)
{
}

BinFileInputStream::BinFileInputStream(const char* const fileName,
                                       MemoryManager* const manager) :

    fSource(XMLPlatformUtils::openFile(fileName, manager))
  , fMemoryManager(manager)
{
}

BinFileInputStream::BinFileInputStream(const FileHandle toAdopt
                                       , MemoryManager* const manager) :

    fSource(toAdopt)
  , fMemoryManager(manager)
{
}

BinFileInputStream::~BinFileInputStream()
{
    if (getIsOpen())
        XMLPlatformUtils::closeFile(fSource, fMemoryManager);
}


// ---------------------------------------------------------------------------
//  BinFileInputStream: Getter methods
// ---------------------------------------------------------------------------
XMLFilePos BinFileInputStream::getSize() const
{
    return XMLPlatformUtils::fileSize(fSource, fMemoryManager);
}


// ---------------------------------------------------------------------------
//  BinFileInputStream: Stream management methods
// ---------------------------------------------------------------------------
void BinFileInputStream::reset()
{
    XMLPlatformUtils::resetFile(fSource, fMemoryManager);
}


// ---------------------------------------------------------------------------
//  BinFileInputStream: Implementation of the input stream interface
// ---------------------------------------------------------------------------
XMLFilePos BinFileInputStream::curPos() const
{
    return XMLPlatformUtils::curFilePos(fSource, fMemoryManager);
}

XMLSize_t
BinFileInputStream::readBytes(          XMLByte* const  toFill
                                , const XMLSize_t       maxToRead)
{
    //
    //  Read up to the maximum bytes requested. We return the number
    //  actually read.
    //
    return XMLPlatformUtils::readFileBuffer(fSource, maxToRead, toFill, fMemoryManager);
}

const XMLCh* BinFileInputStream::getContentType() const
{
    return 0;
}

XERCES_CPP_NAMESPACE_END
