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
 * $Id: WindowsFileMgr.cpp 556533 2007-07-16 07:36:41Z amassari $
 */

#include <windows.h>

#include <xercesc/util/FileManagers/WindowsFileMgr.hpp>
#include <xercesc/util/Janitor.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUniDefs.hpp>

#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER ((DWORD)-1)
#endif

XERCES_CPP_NAMESPACE_BEGIN

static bool isBackSlash(XMLCh c) {
    return c == chBackSlash ||
           c == chYenSign   ||
           c == chWonSign;
}

WindowsFileMgr::WindowsFileMgr()
{
    // Figure out if we are on NT and save that flag for later use
    OSVERSIONINFO   OSVer;
    OSVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    ::GetVersionEx(&OSVer);
    _onNT = (OSVer.dwPlatformId == VER_PLATFORM_WIN32_NT);
}


WindowsFileMgr::~WindowsFileMgr()
{
}


FileHandle
WindowsFileMgr::fileOpen(const XMLCh* fileName, bool toWrite, MemoryManager* const manager)
{
    // Watch for obvious wierdness
    if (!fileName)
        return 0;

    //
    //  We have to play a little trick here. If its /x:.....
    //  style fully qualified path, we have to toss the leading /
    //  character.
    //
    const XMLCh* nameToOpen = fileName;
    if (*fileName == chForwardSlash)
    {
        if (XMLString::stringLen(fileName) > 3)
        {
            if (*(fileName + 2) == chColon)
            {
                const XMLCh chDrive = *(fileName + 1);
                if (((chDrive >= chLatin_A) && (chDrive <= chLatin_Z))
                ||  ((chDrive >= chLatin_a) && (chDrive <= chLatin_z)))
                {
                    nameToOpen = fileName + 1;
                }
            }

            // Similarly for UNC paths
            if ( *(fileName + 1) == *(fileName + 2) &&
                 (*(fileName + 1) == chForwardSlash ||
                  *(fileName + 1) == chBackSlash) )
            {
                nameToOpen = fileName + 1;
            }
        }
    }

    //  Ok, this might look stupid but its a semi-expedient way to deal
    //  with a thorny problem. Shift-JIS and some other Asian encodings
    //  are fundamentally broken and map both the backslash and the Yen
    //  sign to the same code point. Transcoders have to pick one or the
    //  other to map '\' to Unicode and tend to choose the Yen sign.
    //
    //  Unicode Yen or Won signs as directory separators will fail.
    //
    //  So, we will check this path name for Yen or won signs and, if they are
    //  there, we'll replace them with slashes.
    //
    //  A further twist:  we replace Yen and Won with forward slashes rather
    //   than back slashes.  Either form of slash will work as a directory
    //   separator.  On Win 95 and 98, though, Unicode back-slashes may
    //   fail to transode back to 8-bit 0x5C with some Unicode converters
    //   to  some of the problematic code pages.  Forward slashes always
    //   transcode correctly back to 8 bit char * form.
    //
    XMLCh *tmpUName = 0;

    const XMLCh* srcPtr = nameToOpen;
    while (*srcPtr)
    {
        if (*srcPtr == chYenSign ||
            *srcPtr == chWonSign)
            break;
        srcPtr++;
    }

    //
    //  If we found a yen, then we have to create a temp file name. Else
    //  go with the file name as is and save the overhead.
    //
    if (*srcPtr)
    {
        tmpUName = XMLString::replicate(nameToOpen, manager);

        XMLCh* tmpPtr = tmpUName;
        while (*tmpPtr)
        {
            if (*tmpPtr == chYenSign ||
                *tmpPtr == chWonSign)
                *tmpPtr = chForwardSlash;
            tmpPtr++;
        }
        nameToOpen = tmpUName;
    }
    FileHandle retVal = 0;
    if (_onNT)
    {
        retVal = ::CreateFileW
            (
            (LPCWSTR) nameToOpen
            , toWrite?GENERIC_WRITE:GENERIC_READ
            , FILE_SHARE_READ
            , 0
            , toWrite?CREATE_ALWAYS:OPEN_EXISTING
            , toWrite?FILE_ATTRIBUTE_NORMAL:FILE_FLAG_SEQUENTIAL_SCAN
            , 0
            );
    }
    else
    {
        //
        //  We are Win 95 / 98.  Take the Unicode file name back to (char *)
        //    so that we can open it.
        //
        char* tmpName = XMLString::transcode(nameToOpen, manager);
        retVal = ::CreateFileA
            (
            tmpName
            , toWrite?GENERIC_WRITE:GENERIC_READ
            , FILE_SHARE_READ
            , 0
            , toWrite?CREATE_ALWAYS:OPEN_EXISTING
            , toWrite?FILE_ATTRIBUTE_NORMAL:FILE_FLAG_SEQUENTIAL_SCAN
            , 0
            );
        manager->deallocate(tmpName);//delete [] tmpName;
    }

    if (tmpUName)
        manager->deallocate(tmpUName);//delete [] tmpUName;

    if (retVal == INVALID_HANDLE_VALUE)
        return 0;

    return retVal;
}


FileHandle
WindowsFileMgr::fileOpen(const char* path, bool toWrite, MemoryManager* const manager)
{
    XMLCh* tmpFileName = XMLString::transcode(path, manager);
    ArrayJanitor<XMLCh> janText(tmpFileName, manager);
    return fileOpen(tmpFileName, toWrite, manager);
}


FileHandle
WindowsFileMgr::openStdIn(MemoryManager* const manager)
{
    //
    //  Get the standard input handle. Duplicate it and return that copy
    //  since the outside world cannot tell the difference and will shut
    //  down this handle when its done with it. If we gave out the orignal,
    //  shutting it would prevent any further output.
    //
    HANDLE stdInOrg = ::GetStdHandle(STD_INPUT_HANDLE);
    if (stdInOrg == INVALID_HANDLE_VALUE) {
        XMLCh stdinStr[] = {chLatin_s, chLatin_t, chLatin_d, chLatin_i, chLatin_n, chNull};
        ThrowXMLwithMemMgr1(XMLPlatformUtilsException, XMLExcepts::File_CouldNotOpenFile, stdinStr, manager);
    }

    HANDLE retHandle;
    if (!::DuplicateHandle
    (
        ::GetCurrentProcess()
        , stdInOrg
        , ::GetCurrentProcess()
        , &retHandle
        , 0
        , FALSE
        , DUPLICATE_SAME_ACCESS))
    {
        ThrowXMLwithMemMgr(XMLPlatformUtilsException, XMLExcepts::File_CouldNotDupHandle, manager);
    }
    return retHandle;
}


void
WindowsFileMgr::fileClose(FileHandle f, MemoryManager* const manager)
{
    if (!f)
		ThrowXMLwithMemMgr(XMLPlatformUtilsException, XMLExcepts::CPtr_PointerIsZero, manager);

    if (!::CloseHandle(f))
        ThrowXMLwithMemMgr(XMLPlatformUtilsException, XMLExcepts::File_CouldNotCloseFile, manager);
}


void
WindowsFileMgr::fileReset(FileHandle f, MemoryManager* const manager)
{
    if (!f)
		ThrowXMLwithMemMgr(XMLPlatformUtilsException, XMLExcepts::CPtr_PointerIsZero, manager);

    // Seek to the start of the file
    if (::SetFilePointer(f, 0, 0, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
        ThrowXMLwithMemMgr(XMLPlatformUtilsException, XMLExcepts::File_CouldNotResetFile, manager);
}


XMLFilePos
WindowsFileMgr::curPos(FileHandle f, MemoryManager* const manager)
{
    if (!f)
		ThrowXMLwithMemMgr(XMLPlatformUtilsException, XMLExcepts::CPtr_PointerIsZero, manager);
		 
    // Get the current position
    LONG high=0;
    DWORD low = ::SetFilePointer(f, 0, &high, FILE_CURRENT);
    if (low == INVALID_SET_FILE_POINTER && GetLastError()!=NO_ERROR)
        ThrowXMLwithMemMgr(XMLPlatformUtilsException, XMLExcepts::File_CouldNotGetCurPos, manager);

    return (((XMLFilePos)high) << 32) | low;
}


XMLFilePos
WindowsFileMgr::fileSize(FileHandle f, MemoryManager* const manager)
{
    if (!f)
		ThrowXMLwithMemMgr(XMLPlatformUtilsException, XMLExcepts::CPtr_PointerIsZero, manager);
	
    DWORD high=0;
    DWORD low=::GetFileSize(f, &high);
    if(low==INVALID_FILE_SIZE && GetLastError()!=NO_ERROR)
        // TODO: find a better exception
		ThrowXMLwithMemMgr(XMLPlatformUtilsException, XMLExcepts::File_CouldNotGetCurPos, manager);

    return (((XMLFilePos)high) << 32) | low;
}


XMLSize_t
WindowsFileMgr::fileRead(FileHandle f, XMLSize_t byteCount, XMLByte* buffer, MemoryManager* const manager)
{
    if (!f || !buffer)
		ThrowXMLwithMemMgr(XMLPlatformUtilsException, XMLExcepts::CPtr_PointerIsZero, manager);
		
    DWORD bytesRead = 0;
    if (!::ReadFile(f, buffer, (DWORD)byteCount, &bytesRead, 0))
    {
        //
        //  Check specially for a broken pipe error. If we get this, it just
        //  means no more data from the pipe, so return zero.
        //
        if (::GetLastError() != ERROR_BROKEN_PIPE)
            ThrowXMLwithMemMgr(XMLPlatformUtilsException, XMLExcepts::File_CouldNotReadFromFile, manager);
    }
    return (unsigned int)bytesRead;
}


void
WindowsFileMgr::fileWrite(FileHandle f, XMLSize_t byteCount, const XMLByte* buffer, MemoryManager* const manager)
{
    if (!f || !buffer)
		ThrowXMLwithMemMgr(XMLPlatformUtilsException, XMLExcepts::CPtr_PointerIsZero, manager);

    const XMLByte* tmpFlush = buffer;

    while (byteCount > 0)
    {
        DWORD bytesWritten = 0;
        if (!::WriteFile(f, tmpFlush, (DWORD)byteCount, &bytesWritten, 0))
            ThrowXMLwithMemMgr(XMLPlatformUtilsException, XMLExcepts::File_CouldNotWriteToFile, manager);

        tmpFlush+=bytesWritten;
        byteCount-=bytesWritten;
    }
}


XMLCh*
WindowsFileMgr::getFullPath(const XMLCh* const srcPath, MemoryManager* const manager)
{
    //
    //  If we are on NT, then use wide character APIs, else use ASCII APIs.
    //  We have to do it manually since we are only built in ASCII mode from
    //  the standpoint of the APIs.
    //
    if (_onNT)
    {
        // Use a local buffer that is big enough for the largest legal path
        const unsigned int bufSize = 1024;
        XMLCh tmpPath[bufSize + 1];

        XMLCh* namePart = 0;
        if (!::GetFullPathNameW((LPCWSTR)srcPath, bufSize, (LPWSTR)tmpPath, (LPWSTR*)&namePart))
            return 0;

        // Return a copy of the path
        return XMLString::replicate(tmpPath, manager);
    }
     else
    {
        // Transcode the incoming string
        char* tmpSrcPath = XMLString::transcode(srcPath, manager);
        ArrayJanitor<char> janSrcPath(tmpSrcPath, manager);

        // Use a local buffer that is big enough for the largest legal path
        const unsigned int bufSize = 511;
        char tmpPath[511 + 1];

        char* namePart = 0;
        if (!::GetFullPathNameA(tmpSrcPath, bufSize, tmpPath, &namePart))
            return 0;

        // Return a transcoded copy of the path
        return XMLString::transcode(tmpPath, manager);
    }
}


XMLCh*
WindowsFileMgr::getCurrentDirectory(MemoryManager* const manager)
{
    //
    //  If we are on NT, then use wide character APIs, else use ASCII APIs.
    //  We have to do it manually since we are only built in ASCII mode from
    //  the standpoint of the APIs.
    //
    if (_onNT)
    {
        // Use a local buffer that is big enough for the largest legal path
        const unsigned int bufSize = 1024;
        XMLCh tmpPath[bufSize + 1];

        if (!::GetCurrentDirectoryW(bufSize, (LPWSTR)tmpPath))
            return 0;

        // Return a copy of the path
        return XMLString::replicate(tmpPath, manager);
    }
     else
    {
        // Use a local buffer that is big enough for the largest legal path
        const unsigned int bufSize = 511;
        char tmpPath[511 + 1];

        if (!::GetCurrentDirectoryA(bufSize, tmpPath))
            return 0;

        // Return a transcoded copy of the path
        return XMLString::transcode(tmpPath, manager);
    }
}


bool
WindowsFileMgr::isRelative(const XMLCh* const toCheck, MemoryManager* const /*manager*/)
{
    // Check for pathological case of empty path
    if (!toCheck || !toCheck[0])
        return false;

    //
    //  If its starts with a drive, then it cannot be relative. Note that
    //  we checked the drive not being empty above, so worst case its one
    //  char long and the check of the 1st char will fail because its really
    //  a null character.
    //
    if (toCheck[1] == chColon)
    {
        if (((toCheck[0] >= chLatin_A) && (toCheck[0] <= chLatin_Z))
        ||  ((toCheck[0] >= chLatin_a) && (toCheck[0] <= chLatin_z)))
        {
            return false;
        }
    }

    //
    //  If it starts with a double slash, then it cannot be relative since
    //  it's a remote file.
    //
    if (isBackSlash(toCheck[0]))
        return false;

    // Else assume its a relative path
    return true;
}


XERCES_CPP_NAMESPACE_END

