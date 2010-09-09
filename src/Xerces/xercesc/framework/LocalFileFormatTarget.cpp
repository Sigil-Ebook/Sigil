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
 * $Id: LocalFileFormatTarget.cpp 932887 2010-04-11 13:04:59Z borisk $
 */

#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <xercesc/framework/MemoryManager.hpp>
#include <xercesc/util/IOException.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <assert.h>
#include <string.h>

XERCES_CPP_NAMESPACE_BEGIN

const XMLSize_t MAX_BUFFER_SIZE = 65536;

LocalFileFormatTarget::LocalFileFormatTarget( const XMLCh* const   fileName
                                            , MemoryManager* const manager)
: fSource(0)
, fDataBuf(0)
, fIndex(0)
, fCapacity(1024)
, fMemoryManager(manager)
{
    fSource = XMLPlatformUtils::openFileToWrite(fileName, manager);

    if (fSource == (FileHandle) XERCES_Invalid_File_Handle)
        ThrowXMLwithMemMgr1(IOException, XMLExcepts::File_CouldNotOpenFile, fileName, fMemoryManager);

    fDataBuf = (XMLByte*) fMemoryManager->allocate (
      fCapacity * sizeof(XMLByte));
}

LocalFileFormatTarget::LocalFileFormatTarget( const char* const    fileName
                                            , MemoryManager* const manager)
: fSource(0)
, fDataBuf(0)
, fIndex(0)
, fCapacity(1024)
, fMemoryManager(manager)
{
    fSource = XMLPlatformUtils::openFileToWrite(fileName, manager);

    if (fSource == (FileHandle) XERCES_Invalid_File_Handle)
        ThrowXMLwithMemMgr1(IOException, XMLExcepts::File_CouldNotOpenFile, fileName, fMemoryManager);

    fDataBuf = (XMLByte*) fMemoryManager->allocate (
      fCapacity * sizeof(XMLByte));
}

LocalFileFormatTarget::~LocalFileFormatTarget()
{
    try
    {
        // flush remaining buffer before destroy
        XMLPlatformUtils::writeBufferToFile(fSource, fIndex, fDataBuf, fMemoryManager);

        if (fSource)
          XMLPlatformUtils::closeFile(fSource, fMemoryManager);
    }
    catch (...)
    {
      // There is nothing we can do about it here.
    }

    fMemoryManager->deallocate(fDataBuf);//delete [] fDataBuf;
}

void LocalFileFormatTarget::flush()
{
  XMLPlatformUtils::writeBufferToFile(fSource, fIndex, fDataBuf, fMemoryManager);
  fIndex = 0;
}

void LocalFileFormatTarget::writeChars(const XMLByte* const toWrite
                                     , const XMLSize_t count
                                     , XMLFormatter * const)
{
    if (count)
    {
      if (count < MAX_BUFFER_SIZE)
      {
        // If we don't have enough space, see if we can grow the buffer.
        //
        if (fIndex + count > fCapacity && fCapacity < MAX_BUFFER_SIZE)
          ensureCapacity (count);

        // If still not enough space, flush the buffer.
        //
        if (fIndex + count > fCapacity)
        {
          XMLPlatformUtils::writeBufferToFile(fSource, fIndex, fDataBuf, fMemoryManager);
          fIndex = 0;
        }

        memcpy(&fDataBuf[fIndex], toWrite, count * sizeof(XMLByte));
        fIndex += count;
      }
      else
      {
        if (fIndex)
        {
          XMLPlatformUtils::writeBufferToFile(fSource, fIndex, fDataBuf, fMemoryManager);
          fIndex = 0;
        }

        XMLPlatformUtils::writeBufferToFile(fSource, count, toWrite, fMemoryManager);
      }
    }

    return;
}

void LocalFileFormatTarget::ensureCapacity(const XMLSize_t extraNeeded)
{
    XMLSize_t newCap = fCapacity * 2;

    while (fIndex + extraNeeded > newCap)
      newCap *= 2;

    XMLByte* newBuf  = (XMLByte*) fMemoryManager->allocate (
      newCap * sizeof(XMLByte));

    // Copy over the old stuff
    memcpy(newBuf, fDataBuf, fIndex * sizeof(XMLByte));

    // Clean up old buffer and store new stuff
    fMemoryManager->deallocate(fDataBuf);
    fDataBuf = newBuf;
    fCapacity = newCap;
}

XERCES_CPP_NAMESPACE_END
