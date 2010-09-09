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
 * $Id: XMemory.cpp 635226 2008-03-09 12:04:39Z borisk $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/XMemory.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/framework/MemoryManager.hpp>
#include <assert.h>

XERCES_CPP_NAMESPACE_BEGIN

void* XMemory::operator new(size_t size)
{
	size_t headerSize = XMLPlatformUtils::alignPointerForNewBlockAllocation(
										sizeof(MemoryManager*));
	
    void* const block = XMLPlatformUtils::fgMemoryManager->allocate
        (
	        headerSize + size
        );
    *(MemoryManager**)block = XMLPlatformUtils::fgMemoryManager;

    return (char*)block + headerSize;
}

#if defined(XERCES_MFC_SUPPORT)

void* XMemory::operator new(size_t size, const char* /*file*/, int /*line*/)
{ 
		 return operator new(size); 
}
 
void XMemory::operator delete(void* p, const char* /*file*/, int /*line*/)
{ 
		 operator delete(p); 
}
 
#endif

void* XMemory::operator new(size_t size, MemoryManager* manager)
{
    assert(manager != 0);
	
    size_t headerSize = XMLPlatformUtils::alignPointerForNewBlockAllocation(
       sizeof(MemoryManager*));
       
    void* const block = manager->allocate(headerSize + size);
    *(MemoryManager**)block = manager;

    return (char*)block + headerSize;
}

void* XMemory::operator new(size_t /*size*/, void* ptr)
{
    return ptr;
}

void XMemory::operator delete(void* p)
{
    if (p != 0)
    {
        size_t headerSize = XMLPlatformUtils::alignPointerForNewBlockAllocation(
          sizeof(MemoryManager*));
        void* const block = (char*)p - headerSize;

        MemoryManager* const manager = *(MemoryManager**)block;
        assert(manager != 0);
        manager->deallocate(block);
    }
}

//The Borland compiler is complaining about duplicate overloading of delete
#if !defined(XERCES_NO_MATCHING_DELETE_OPERATOR)

void XMemory::operator delete(void* p, MemoryManager* manager)
{
    assert(manager != 0);
	
    if (p != 0)
    {
        size_t headerSize = XMLPlatformUtils::alignPointerForNewBlockAllocation(sizeof(MemoryManager*));
        void* const block = (char*)p - headerSize;

        /***
         * assert(*(MemoryManager**)block == manager);                 
         *
         * NOTE: for compiler which can't properly trace the memory manager used in the 
         *       placement new, we use the memory manager embedded in the memory rather 
         *       than the one passed in
         */ 
        MemoryManager* pM = *(MemoryManager**)block;
        pM->deallocate(block);
    }
}

void XMemory::operator delete(void* /*p*/, void* /*ptr*/)
{
}

#endif

XERCES_CPP_NAMESPACE_END

