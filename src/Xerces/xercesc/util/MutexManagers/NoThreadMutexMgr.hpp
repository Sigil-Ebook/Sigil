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
 * $Id: NoThreadMutexMgr.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_NOTHREADMUTEXMGR_HPP)
#define XERCESC_INCLUDE_GUARD_NOTHREADMUTEXMGR_HPP

#include <xercesc/util/XMLMutexMgr.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/*
	The NoThread mutex manager is for use where no threading is used
	in an environment. Since no threading is used, mutexes are not
	needed, so the implementation does essentially nothing.
*/
class NoThreadMutexMgr : public XMLMutexMgr
{
    public:
        NoThreadMutexMgr();
        virtual ~NoThreadMutexMgr();

		// Mutex operations
		virtual XMLMutexHandle	create(MemoryManager* const manager);
		virtual void			destroy(XMLMutexHandle mtx, MemoryManager* const manager);
		virtual void			lock(XMLMutexHandle mtx);
		virtual void			unlock(XMLMutexHandle mtx);
};

XERCES_CPP_NAMESPACE_END


#endif

