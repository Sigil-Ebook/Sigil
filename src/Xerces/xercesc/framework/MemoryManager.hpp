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
 * $Id: MemoryManager.hpp 673975 2008-07-04 09:23:56Z borisk $
 */


#if !defined(XERCESC_INCLUDE_GUARD_MEMORYMANAGER_HPP)
#define XERCESC_INCLUDE_GUARD_MEMORYMANAGER_HPP

#include <xercesc/util/XercesDefs.hpp>
#include <stdlib.h>


XERCES_CPP_NAMESPACE_BEGIN


/**
 *  Configurable memory manager
 *
 *  <p>This interface allows outside applications to plug in their own memory
 *  manager to be used by Xerces for memory allocation/deallocation.</p>
 */
class XMLPARSER_EXPORT MemoryManager
{
public:
    // -----------------------------------------------------------------------
    //  Constructors are hidden, only the virtual destructor is exposed
    // -----------------------------------------------------------------------

    /** @name Destructor */
    //@{

    /**
      * Default destructor
      */
    virtual ~MemoryManager()
    {
    }
    //@}


    /**
      * This method is called to obtain the memory manager that should be
      * used to allocate memory used in exceptions. If the same memory
      * manager can be used, simply return 'this' from this function.
      * Note, however, that if there is a possibility that an exception
      * thrown can outlive the memory manager (for example, because the
      * memory manager object is allocated on the stack or is managed by
      * a stack-bound object), it is recommended that you return
      * XMLPlatformUtils::fgMemoryManager.
      *
      * @return A pointer to the memory manager
      */
    virtual MemoryManager* getExceptionMemoryManager() = 0;


    // -----------------------------------------------------------------------
    //  The virtual memory manager interface
    // -----------------------------------------------------------------------
    /** @name The pure virtual methods in this interface. */
    //@{

    /**
      * This method allocates requested memory.
      *
      * @param size The requested memory size
      *
      * @return A pointer to the allocated memory
      */
    virtual void* allocate(XMLSize_t size) = 0;

    /**
      * This method deallocates memory
      *
      * @param p The pointer to the allocated memory to be deleted
      */
    virtual void deallocate(void* p) = 0;

    //@}


protected :
    // -----------------------------------------------------------------------
    //  Hidden Constructors
    // -----------------------------------------------------------------------
    /** @name Constructor */
    //@{

    /**
      * Protected default constructor
      */
    MemoryManager()
    {
    }
    //@}



private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    MemoryManager(const MemoryManager&);
    MemoryManager& operator=(const MemoryManager&);
};

XERCES_CPP_NAMESPACE_END

#endif
