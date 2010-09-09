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
 * $Id: Mutexes.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_MUTEXES_HPP)
#define XERCESC_INCLUDE_GUARD_MUTEXES_HPP

#include <xercesc/util/XMemory.hpp>
#include <xercesc/util/PlatformUtils.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XMLUTIL_EXPORT XMLMutex : public XMemory
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    XMLMutex(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    ~XMLMutex();


    // -----------------------------------------------------------------------
    //  Lock control methods
    // -----------------------------------------------------------------------
    void lock();
    void unlock();


private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XMLMutex(const XMLMutex&);
    XMLMutex& operator=(const XMLMutex&);


    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fHandle
    //      The raw mutex handle. Its just a void pointer so we do not
    //      pass judgement on its value at all. We just pass it into the
    //      platform utilities methods which knows what's really in it.
    // fManager
    //      The MemoryManager that this XMLMutex was initialized with.
    // -----------------------------------------------------------------------
    void*          fHandle;
    MemoryManager* fManager;


    // -----------------------------------------------------------------------
    //  Sun PlatformUtils needs access to fHandle to initialize the
    //  atomicOpsMutex at startup.
    // -----------------------------------------------------------------------
    friend class XMLPlatformUtils;
};


class XMLUTIL_EXPORT XMLMutexLock : public XMemory
{
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
public:
    XMLMutexLock(XMLMutex* const toLock);
    ~XMLMutexLock();


private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XMLMutexLock();
    XMLMutexLock(const XMLMutexLock&);
    XMLMutexLock& operator=(const XMLMutexLock&);


    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fToLock
    //      The mutex object that we are locking
    // -----------------------------------------------------------------------
    XMLMutex*   fToLock;
};

XERCES_CPP_NAMESPACE_END

#endif
