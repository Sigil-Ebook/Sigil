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
 * $Id: BinMemOutputStream.cpp 932887 2010-04-11 13:04:59Z borisk $
 */

#include <xercesc/internal/BinMemOutputStream.hpp>
#include <xercesc/util/XMLString.hpp>
#include <string.h>

XERCES_CPP_NAMESPACE_BEGIN

BinMemOutputStream::BinMemOutputStream( XMLSize_t            initCapacity
                                      , MemoryManager* const manager)
: fMemoryManager(manager)
, fDataBuf(0)
, fIndex(0)
, fCapacity(initCapacity)
{
    // Buffer is one larger than capacity, to allow for zero term
    fDataBuf = (XMLByte*) fMemoryManager->allocate
    (
        (fCapacity + 4) * sizeof(XMLByte)
    );

    // Keep it null terminated
    fDataBuf[0] = XMLByte(0);
}

BinMemOutputStream::~BinMemOutputStream()
{
    fMemoryManager->deallocate(fDataBuf);
}

void BinMemOutputStream::writeBytes( const XMLByte*     const      toGo
                                   , const XMLSize_t               maxToWrite)
{

    if (maxToWrite) 
    {
        ensureCapacity(maxToWrite);
        memcpy(&fDataBuf[fIndex], toGo, maxToWrite * sizeof(XMLByte));
        fIndex += maxToWrite;
    }

}

const XMLByte* BinMemOutputStream::getRawBuffer() const
{
    fDataBuf[fIndex] = 0;
    fDataBuf[fIndex + 1] = 0;
    fDataBuf[fIndex + 2] = 0;
    fDataBuf[fIndex + 3] = 0;
    return fDataBuf;
}

void BinMemOutputStream::reset()
{
    fIndex = 0;
    for (int i = 0; i < 4; i++)
    {
        fDataBuf[fIndex + i] = 0;
    }
}

XMLFilePos BinMemOutputStream::curPos() const
{
    return fIndex;
}

XMLFilePos BinMemOutputStream::getSize() const
{
    return fCapacity;
}

// ---------------------------------------------------------------------------
//  BinMemOutputStream: Private helper methods
// ---------------------------------------------------------------------------
void BinMemOutputStream::ensureCapacity(const XMLSize_t extraNeeded)
{
    // If we can handle it, do nothing yet
    if (fIndex + extraNeeded < fCapacity)
        return;

    // Oops, not enough room. Calc new capacity and allocate new buffer
    const XMLSize_t newCap = ((fIndex + extraNeeded) * 2);
    XMLByte* newBuf = (XMLByte*) fMemoryManager->allocate
    (
        (newCap+4) * sizeof(XMLByte)
    );

    memset(newBuf, 0, (newCap+4) * sizeof(XMLByte));

    // Copy over the old stuff
    memcpy(newBuf, fDataBuf, fCapacity * sizeof(XMLByte) + 4);

    // Clean up old buffer and store new stuff
    fMemoryManager->deallocate(fDataBuf); 
    fDataBuf = newBuf;
    fCapacity = newCap;
}


XERCES_CPP_NAMESPACE_END

