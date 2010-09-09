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
 * $Id: XMLMutexMgr.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XMLMUTEXMGR_HPP)
#define XERCESC_INCLUDE_GUARD_XMLMUTEXMGR_HPP

#include <xercesc/util/XMemory.hpp>

XERCES_CPP_NAMESPACE_BEGIN

typedef void* XMLMutexHandle;

//	Abstract class for mutex implementation.
//  This is be used to allow multiple mutex handling implementations.
class XMLMutexMgr : public XMemory
{
    public:
        XMLMutexMgr() {}
        virtual ~XMLMutexMgr() {}

		// Mutex operations
		virtual XMLMutexHandle	create(MemoryManager* const manager) = 0;
		virtual void			destroy(XMLMutexHandle mtx, MemoryManager* const manager) = 0;
		virtual void			lock(XMLMutexHandle mtx) = 0;
		virtual void			unlock(XMLMutexHandle mtx) = 0;
};

XERCES_CPP_NAMESPACE_END


#endif

