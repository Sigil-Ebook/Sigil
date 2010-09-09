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
 * $Id: PosixFileMgr.cpp 673975 2008-07-04 09:23:56Z borisk $
 */

#include <stdio.h>
#include <unistd.h>
#include <limits.h>

#include <xercesc/util/FileManagers/PosixFileMgr.hpp>

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/Janitor.hpp>
#include <xercesc/util/RuntimeException.hpp>
#include <xercesc/util/PanicHandler.hpp>
#include <xercesc/util/XMLString.hpp>


XERCES_CPP_NAMESPACE_BEGIN


PosixFileMgr::PosixFileMgr()
{
}


PosixFileMgr::~PosixFileMgr()
{
}


FileHandle
PosixFileMgr::fileOpen(const XMLCh* path, bool toWrite, MemoryManager* const manager)
{
    const char* tmpFileName = XMLString::transcode(path, manager);
    ArrayJanitor<char> janText((char*)tmpFileName, manager);
    return fileOpen(tmpFileName, toWrite, manager);
}


FileHandle
PosixFileMgr::fileOpen(const char* path, bool toWrite, MemoryManager* const /*manager*/)
{
    const char* perms = (toWrite) ? "wb" : "rb";
    FileHandle result = (FileHandle)fopen(path , perms);
    return result;
}


FileHandle
PosixFileMgr::openStdIn(MemoryManager* const /*manager*/)
{
    return (FileHandle)fdopen(dup(0), "rb");
}


void
PosixFileMgr::fileClose(FileHandle f, MemoryManager* const manager)
{
    if (!f)
		ThrowXMLwithMemMgr(XMLPlatformUtilsException, XMLExcepts::CPtr_PointerIsZero, manager);

    if (fclose((FILE*)f))
        ThrowXMLwithMemMgr(XMLPlatformUtilsException,
                 XMLExcepts::File_CouldNotCloseFile, manager);
}


void
PosixFileMgr::fileReset(FileHandle f, MemoryManager* const manager)
{
    if (!f)
		ThrowXMLwithMemMgr(XMLPlatformUtilsException, XMLExcepts::CPtr_PointerIsZero, manager);

    // Seek to the start of the file
    if (fseek((FILE*)f, 0, SEEK_SET))
        ThrowXMLwithMemMgr(XMLPlatformUtilsException,
                 XMLExcepts::File_CouldNotResetFile, manager);
}


XMLFilePos
PosixFileMgr::curPos(FileHandle f, MemoryManager* const manager)
{
    if (!f)
		ThrowXMLwithMemMgr(XMLPlatformUtilsException, XMLExcepts::CPtr_PointerIsZero, manager);
		 
    long curPos = ftell((FILE*)f);
	
    if (curPos == -1)
        ThrowXMLwithMemMgr(XMLPlatformUtilsException, XMLExcepts::File_CouldNotGetSize, manager);

    return (XMLFilePos)curPos;
}


XMLFilePos
PosixFileMgr::fileSize(FileHandle f, MemoryManager* const manager)
{
    if (!f)
		ThrowXMLwithMemMgr(XMLPlatformUtilsException, XMLExcepts::CPtr_PointerIsZero, manager);
		
    // Get the current position
    long curPos = ftell((FILE*)f);
    if (curPos == -1)
        ThrowXMLwithMemMgr(XMLPlatformUtilsException, XMLExcepts::File_CouldNotGetCurPos, manager);

    // Seek to the end and save that value for return
    if (fseek((FILE*)f, 0, SEEK_END))
        ThrowXMLwithMemMgr(XMLPlatformUtilsException, XMLExcepts::File_CouldNotSeekToEnd, manager);

    long retVal = ftell((FILE*)f);
    if (retVal == -1)
        ThrowXMLwithMemMgr(XMLPlatformUtilsException, XMLExcepts::File_CouldNotSeekToEnd, manager);

    // And put the pointer back
    if (fseek((FILE*)f, curPos, SEEK_SET))
        ThrowXMLwithMemMgr(XMLPlatformUtilsException, XMLExcepts::File_CouldNotSeekToPos, manager);

    return (XMLFilePos)retVal;
}


XMLSize_t
PosixFileMgr::fileRead(FileHandle f, XMLSize_t byteCount, XMLByte* buffer, MemoryManager* const manager)
{
    if (!f || !buffer)
		ThrowXMLwithMemMgr(XMLPlatformUtilsException, XMLExcepts::CPtr_PointerIsZero, manager);
		
    XMLSize_t bytesRead = 0;
	if (byteCount > 0)
	{
    	bytesRead = fread((void*)buffer, 1, byteCount, (FILE*)f);

		if (ferror((FILE*)f))
			ThrowXMLwithMemMgr(XMLPlatformUtilsException, XMLExcepts::File_CouldNotReadFromFile, manager);
	}
	
    return bytesRead;
}


void
PosixFileMgr::fileWrite(FileHandle f, XMLSize_t byteCount, const XMLByte* buffer, MemoryManager* const manager)
{
    if (!f || !buffer)
		ThrowXMLwithMemMgr(XMLPlatformUtilsException, XMLExcepts::CPtr_PointerIsZero, manager);

    while (byteCount > 0)
    {
        XMLSize_t bytesWritten = fwrite(buffer, sizeof(XMLByte), byteCount, (FILE*)f);

        if (ferror((FILE*)f))
			ThrowXMLwithMemMgr(XMLPlatformUtilsException, XMLExcepts::File_CouldNotWriteToFile, manager);

		buffer		+= bytesWritten;
		byteCount	-= bytesWritten;
    }
}


XMLCh*
PosixFileMgr::getFullPath(const XMLCh* const srcPath, MemoryManager* const manager)
{
    //
    //  NOTE: The path provided has always already been opened successfully,
    //  so we know that its not some pathological freaky path. It comes in
    //  in native format, and goes out as Unicode always
    //
    char* newSrc = XMLString::transcode(srcPath, manager);
    ArrayJanitor<char> janText(newSrc, manager);

    // Use a local buffer that is big enough for the largest legal path
    char absPath[PATH_MAX + 1];
    
    // get the absolute path
    if (!realpath(newSrc, absPath))
   		ThrowXMLwithMemMgr(XMLPlatformUtilsException, XMLExcepts::File_CouldNotGetBasePathName, manager);

    return XMLString::transcode(absPath, manager);
}


XMLCh*
PosixFileMgr::getCurrentDirectory(MemoryManager* const manager)
{
    char dirBuf[PATH_MAX + 2];
    char *curDir = getcwd(&dirBuf[0], PATH_MAX + 1);

    if (!curDir)
        ThrowXMLwithMemMgr(XMLPlatformUtilsException,
                 XMLExcepts::File_CouldNotGetBasePathName, manager);

    return XMLString::transcode(curDir, manager);
}


bool
PosixFileMgr::isRelative(const XMLCh* const toCheck, MemoryManager* const /*manager*/)
{
    // Check for pathological case of empty path
    if (!toCheck || !toCheck[0])
        return false;

    //
    //  If it starts with a slash, then it cannot be relative.
    //
    return toCheck[0] != XMLCh('/');
}


XERCES_CPP_NAMESPACE_END

