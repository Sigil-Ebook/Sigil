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
 * $Id: XMLFileMgr.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XMLFILEMGR_HPP)
#define XERCESC_INCLUDE_GUARD_XMLFILEMGR_HPP

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/util/XMemory.hpp>

XERCES_CPP_NAMESPACE_BEGIN

typedef void* FileHandle;
#define XERCES_Invalid_File_Handle 0

//	Abstract class for files. This is be used to allow multiple file handling implementations.
class XMLFileMgr : public XMemory
{
    public:
        XMLFileMgr() {}
        virtual ~XMLFileMgr() {}

		// File access
        virtual FileHandle	fileOpen(const XMLCh* path, bool toWrite, MemoryManager* const manager) = 0;
        virtual FileHandle	fileOpen(const char* path, bool toWrite, MemoryManager* const manager) = 0;
        virtual FileHandle	openStdIn(MemoryManager* const manager) = 0;
        
        virtual void		fileClose(FileHandle f, MemoryManager* const manager) = 0;
        virtual void		fileReset(FileHandle f, MemoryManager* const manager) = 0;

        virtual XMLFilePos	curPos(FileHandle f, MemoryManager* const manager) = 0;
        virtual XMLFilePos	fileSize(FileHandle f, MemoryManager* const manager) = 0;

        virtual XMLSize_t	fileRead(FileHandle f, XMLSize_t byteCount, XMLByte* buffer, MemoryManager* const manager) = 0;
        virtual void		fileWrite(FileHandle f, XMLSize_t byteCount, const XMLByte* buffer, MemoryManager* const manager) = 0;
        
        // Ancillary path handling routines
        virtual XMLCh*		getFullPath(const XMLCh* const srcPath, MemoryManager* const manager) = 0;
        virtual XMLCh*		getCurrentDirectory(MemoryManager* const manager) = 0;
        virtual bool		isRelative(const XMLCh* const toCheck, MemoryManager* const manager) = 0;
};

XERCES_CPP_NAMESPACE_END

#endif

