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
 * $Id: SAXException.cpp 471747 2006-11-06 14:31:56Z amassari $
 */

#include "SAXException.hpp"

XERCES_CPP_NAMESPACE_BEGIN


// SAXNotSupportedException Constructors
SAXNotSupportedException::SAXNotSupportedException(MemoryManager* const manager)
	: SAXException(manager)
{
}

SAXNotSupportedException::SAXNotSupportedException(const XMLCh* const msg,
                                                   MemoryManager* const manager)
	: SAXException(msg, manager)
{
}

SAXNotSupportedException::SAXNotSupportedException(const char* const msg,
                                                   MemoryManager* const manager)
	: SAXException(msg, manager)
{
}

SAXNotSupportedException::SAXNotSupportedException(const SAXException& toCopy)
  : SAXException(toCopy)
{
}

// SAXNotRecognizedException Constructors
SAXNotRecognizedException::SAXNotRecognizedException(MemoryManager* const manager)
	: SAXException(manager)
{
}

SAXNotRecognizedException::SAXNotRecognizedException(const XMLCh* const msg,
                                                     MemoryManager* const manager)
	: SAXException(msg, manager)
{
}

SAXNotRecognizedException::SAXNotRecognizedException(const char* const msg,
                                                     MemoryManager* const manager)
	: SAXException(msg, manager)
{
}

SAXNotRecognizedException::SAXNotRecognizedException(const SAXException& toCopy)
  : SAXException(toCopy)
{
}

XERCES_CPP_NAMESPACE_END

