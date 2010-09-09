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
 * $Id: DOMStringPool.hpp 678766 2008-07-22 14:00:16Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMSTRINGPOOL_HPP)
#define XERCESC_INCLUDE_GUARD_DOMSTRINGPOOL_HPP

//
//  This file is part of the internal implementation of the C++ XML DOM.
//  It should NOT be included or used directly by application programs.
//
//  Applications should include the file <xercesc/dom/DOM.hpp> for the entire
//  DOM API, or xercesc/dom/DOM*.hpp for individual DOM classes, where the class
//  name is substituded for the *.
//

#include <xercesc/util/XercesDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class   DOMDocumentImpl;

//
//  DStringPoolEntry - one of these structs is allocated for each
//                      XMLCh String in the pool.  Each slot in the
//                      hash table array itself is a pointer to the head
//                      of a singly-linked list of these structs.
//
//                      Although this struct is delcared with a string length of one,
//                      the factory method allocates enough storage to hold the full
//                      string length.
//
struct DOMStringPoolEntry
{
    DOMStringPoolEntry    *fNext;
    XMLCh                 fString[1];
};

//
// DOMBuffer is a lightweight text buffer
// The buffer is not nul terminated until some asks to see the raw buffer
// contents. This also avoids overhead during append operations.
class DOMBuffer
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    DOMBuffer(DOMDocumentImpl *doc, XMLSize_t capacity = 31);

    ~DOMBuffer()
    {
    }

    // -----------------------------------------------------------------------
    //  Buffer Management
    // -----------------------------------------------------------------------
    void append (const XMLCh* const chars);
    void append (const XMLCh* const chars, const XMLSize_t count);

    void set (const XMLCh* const chars);
    void set (const XMLCh* const chars, const XMLSize_t count);

    const XMLCh* getRawBuffer() const
    {
        fBuffer[fIndex] = 0;
        return fBuffer;
    }

    void reset()
    {
        fIndex = 0;
        fBuffer[0] = 0;
    }

    void chop
    (
        const XMLSize_t    count
    )
    {
        fBuffer[count] = 0;
        fIndex = count;
    }


    // -----------------------------------------------------------------------
    //  Getters
    // -----------------------------------------------------------------------
    XMLSize_t getLen() const
    {
        return fIndex;
    }

    XMLSize_t getCapacity() const
    {
        return fCapacity;
    }

    // -----------------------------------------------------------------------
    //  Private helpers
    // -----------------------------------------------------------------------
    void expandCapacity(const XMLSize_t extraNeeded);


private :
    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fBuffer
    //      The pointer to the buffer data. Its grown as needed. Its always
    //      one larger than fCapacity, to leave room for the null terminator.
    //
    //  fIndex
    //      The current index into the buffer, as characters are appended
    //      to it. If its zero, then the buffer is empty.
    //
    //  fCapacity
    //      The current capacity of the buffer. Its actually always one
    //      larger, to leave room for the null terminator.
    //
    //  fDoc
    //      For allocating memory
    // -----------------------------------------------------------------------
    XMLCh*           fBuffer;
    XMLSize_t        fIndex;
    XMLSize_t        fCapacity;
    DOMDocumentImpl* fDoc;

    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    DOMBuffer(const DOMBuffer &);
    DOMBuffer & operator = (const DOMBuffer &);
};

inline void DOMBuffer::
append (const XMLCh* const chars)
{
  XMLSize_t count = XMLString::stringLen(chars);
  if (fIndex + count >= fCapacity)
    expandCapacity(count);

  memcpy(&fBuffer[fIndex], chars, count * sizeof(XMLCh));
  fIndex += count;

  // Keep it null terminated
  fBuffer[fIndex] = 0;
}

inline void DOMBuffer::
append (const XMLCh* const chars, const XMLSize_t count)
{
  if (fIndex + count >= fCapacity)
    expandCapacity(count);

  memcpy(&fBuffer[fIndex], chars, count * sizeof(XMLCh));
  fIndex += count;

  // Keep it null terminated
  fBuffer[fIndex] = 0;
}

inline void DOMBuffer::
set (const XMLCh* const chars)
{
  XMLSize_t count = XMLString::stringLen(chars);
  fIndex = 0;
  if (count >= fCapacity)
    expandCapacity(count);

  memcpy(fBuffer, chars, count * sizeof(XMLCh));
  fIndex = count;

  // Keep it null terminated
  fBuffer[fIndex] = 0;
}

inline void DOMBuffer::
set (const XMLCh* const chars, const XMLSize_t count)
{
  fIndex = 0;
  if (count >= fCapacity)
    expandCapacity(count);

  memcpy(fBuffer, chars, count * sizeof(XMLCh));
  fIndex = count;

  // Keep it null terminated
  fBuffer[fIndex] = 0;
}

XERCES_CPP_NAMESPACE_END

#endif
