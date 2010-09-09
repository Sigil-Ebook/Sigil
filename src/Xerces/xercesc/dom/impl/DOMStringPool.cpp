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
 * $Id: DOMStringPool.cpp 678766 2008-07-22 14:00:16Z borisk $
 */

#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>

#include "DOMStringPool.hpp"
#include "DOMDocumentImpl.hpp"

XERCES_CPP_NAMESPACE_BEGIN

DOMBuffer::
DOMBuffer(DOMDocumentImpl *doc, XMLSize_t capacity) :
    fBuffer(0)
    , fIndex(0)
    , fCapacity(capacity)
    , fDoc(doc)
{
    // Buffer is one larger than capacity, to allow for zero term
    fBuffer = (XMLCh*) doc->allocate((fCapacity+1)*sizeof(XMLCh));

    // Keep it null terminated
    fBuffer[0] = XMLCh(0);
}

// ---------------------------------------------------------------------------
//  DOMBuffer: Private helper methods
// ---------------------------------------------------------------------------
void DOMBuffer::expandCapacity(const XMLSize_t extraNeeded)
{
    //not enough room. Calc new capacity and allocate new buffer
    const XMLSize_t newCap = (XMLSize_t)((fIndex + extraNeeded) * 1.25);
    XMLCh* newBuf = (XMLCh*) fDoc->allocate((newCap+1)*sizeof(XMLCh));

    // Copy over the old stuff
    memcpy(newBuf, fBuffer, fCapacity * sizeof(XMLCh));

    // revisit: Leave the old buffer in document heap, yes, this is a leak, but live with it!
    // store new stuff
    fBuffer = newBuf;
    fCapacity = newCap;
}

XERCES_CPP_NAMESPACE_END
