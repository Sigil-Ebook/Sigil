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


#include "DOMXPathException.hpp"
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLMsgLoader.hpp>
#include <xercesc/util/XMLDOMMsg.hpp>
#include "impl/DOMImplementationImpl.hpp"

XERCES_CPP_NAMESPACE_BEGIN


DOMXPathException::DOMXPathException()
: DOMException()
{
}


DOMXPathException::DOMXPathException(short exCode,
                                     short messageCode,
                                     MemoryManager* const  memoryManager)
: DOMException(exCode, messageCode?messageCode:XMLDOMMsg::DOMXPATHEXCEPTION_ERRX+exCode-DOMXPathException::INVALID_EXPRESSION_ERR+1, memoryManager)
{
}


DOMXPathException::DOMXPathException(const DOMXPathException &other)
: DOMException(other)
{
}


DOMXPathException::~DOMXPathException()
{
}

XERCES_CPP_NAMESPACE_END
