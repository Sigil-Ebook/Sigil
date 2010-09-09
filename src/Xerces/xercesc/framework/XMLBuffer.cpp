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

/**
 * $Id: XMLBuffer.cpp 932887 2010-04-11 13:04:59Z borisk $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/framework/XMLBuffer.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/RuntimeException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  XMLBuffer: Buffer management
// ---------------------------------------------------------------------------

void XMLBuffer::ensureCapacity(const XMLSize_t extraNeeded)
{    
    // If we can't handle it, try doubling the buffer size.
    XMLSize_t newCap = (fIndex + extraNeeded) * 2;

    // If a maximum size is set, and double the current buffer size exceeds that
    // maximum, first check if the maximum size will accomodate the extra needed.
    if (fFullHandler && (newCap > fFullSize))
    {
        // If the maximum buffer size accomodates the extra needed, resize to
        // the maximum
        if (fIndex + extraNeeded <= fFullSize) 
        {
            newCap = fFullSize;
        }

        // Otherwise, allow the registered full-handler to try to empty the buffer.
        // If it claims success, and we can accommodate the extra needed in the buffer
        // to be expanded, resize to the maximum
        // Note the order of evaluation: bufferFull() has the intentional side-effect
        // of modifying fIndex.
        else if (fFullHandler->bufferFull(*this) && (fIndex + extraNeeded <= fFullSize))
        {
            newCap = fFullSize;
        }

        // Finally, if the full-handler failed, or the buffer (of maximum size) 
        // still can't accomodate the extra needed, we must fail.
        else
            ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Array_BadNewSize, fMemoryManager);
    }

    // Note the previous if block can modify newCap, so we may not need to allocate
    // at all.
    if (newCap > fCapacity)
    {
        // Allocate new buffer
        XMLCh* newBuf = (XMLCh*) fMemoryManager->allocate((newCap+1) * sizeof(XMLCh)); //new XMLCh[newCap+1];
 
        // Copy over the old stuff
        memcpy(newBuf, fBuffer, fIndex * sizeof(XMLCh));

        // Clean up old buffer and store new stuff
        fMemoryManager->deallocate(fBuffer); //delete [] fBuffer;
        fBuffer = newBuf;
        fCapacity = newCap;
    }
}

XERCES_CPP_NAMESPACE_END

