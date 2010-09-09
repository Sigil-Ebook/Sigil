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
 * $Id: DOMLocatorImpl.cpp 676853 2008-07-15 09:58:05Z borisk $
 */

#include "DOMLocatorImpl.hpp"

XERCES_CPP_NAMESPACE_BEGIN


// ---------------------------------------------------------------------------
//  DOMLocatorImpl: Constructors and Destructor
// ---------------------------------------------------------------------------
DOMLocatorImpl::DOMLocatorImpl() :
fLineNum(0)
, fColumnNum(0)
, fByteOffset(~(XMLFilePos(0)))
, fUtf16Offset(~(XMLFilePos(0)))
, fRelatedNode(0)
, fURI(0)
{
}


DOMLocatorImpl::DOMLocatorImpl(const XMLFileLoc lineNum,
                               const XMLFileLoc columnNum,
                               DOMNode* const errorNode,
                               const XMLCh* const uri,
                               const XMLFilePos byteOffset,
                               const XMLFilePos utf16Offset) :
fLineNum(lineNum)
, fColumnNum(columnNum)
, fByteOffset(byteOffset)
, fUtf16Offset(utf16Offset)
, fRelatedNode(errorNode)
, fURI(uri)
{
}

DOMLocatorImpl::~DOMLocatorImpl()
{
}

XERCES_CPP_NAMESPACE_END
