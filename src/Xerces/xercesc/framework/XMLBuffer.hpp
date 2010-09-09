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
 * $Id: XMLBuffer.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XMLBUFFER_HPP)
#define XERCESC_INCLUDE_GUARD_XMLBUFFER_HPP

#include <xercesc/util/XMemory.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/framework/MemoryManager.hpp>
#include <string.h>

XERCES_CPP_NAMESPACE_BEGIN

class XMLBufferFullHandler;

/**
 *  XMLBuffer is a lightweight, expandable Unicode text buffer. Since XML is
 *  inherently theoretically unbounded in terms of the sizes of things, we
 *  very often need to have expandable buffers. The primary concern here is
 *  that appends of characters and other buffers or strings be very fast, so
 *  it always maintains the current buffer size.
 *
 *  The buffer is not null terminated until some asks to see the raw buffer
 *  contents. This also avoids overhead during append operations.
 */
class XMLPARSER_EXPORT XMLBuffer : public XMemory
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------

    /** @name Constructor */
    //@{
    XMLBuffer(const XMLSize_t capacity = 1023
              , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager) :

        fIndex(0)
        , fCapacity(capacity)
        , fFullSize(0)
        , fUsed(false)
        , fMemoryManager(manager)
        , fFullHandler(0)
        , fBuffer(0)
    {
        // Buffer is one larger than capacity, to allow for zero term
        fBuffer = (XMLCh*) manager->allocate((capacity+1) * sizeof(XMLCh)); //new XMLCh[fCapacity+1];

        // Keep it null terminated
        fBuffer[0] = XMLCh(0);
    }
    //@}

    /** @name Destructor */
    //@{
    ~XMLBuffer()
    {
        fMemoryManager->deallocate(fBuffer); //delete [] fBuffer;
    }
    //@}

    // -----------------------------------------------------------------------
    //  Buffer Full Handler Management
    // -----------------------------------------------------------------------
    void setFullHandler(XMLBufferFullHandler* handler, const XMLSize_t fullSize)
    {
        if (handler && fullSize) {
            fFullHandler = handler;
            fFullSize = fullSize;

            // Need to consider the case that the fullsize is less than the current capacity.
            // For example, say fullSize = 100 and fCapacity is 1023 (the default).
            // If the fIndex is less than the fullSize, then no problem.  We can just carry
            // on by resetting fCapacity to fullsize and proceed business as usual.
            // If the fIndex is already bigger than the fullSize then we call ensureCapacity
            // to see if it can handle emptying the current buffer (it will throw an
            // exception if it can't).
            if (fullSize < fCapacity) {
                fCapacity = fullSize;
                if (fIndex >= fullSize) {
                    ensureCapacity(0);
                }
            }
        }
        else {
            // reset fFullHandler to zero because setFullHandler had bad input
            fFullHandler = 0;
        }
    }

    // -----------------------------------------------------------------------
    //  Buffer Management
    // -----------------------------------------------------------------------
    void append(const XMLCh toAppend)
    {
        // Put in char and bump the index
        if (fIndex == fCapacity)
            ensureCapacity(1);
        fBuffer[fIndex++] = toAppend;
    }

    void append (const XMLCh* const chars, const XMLSize_t count)
    {
        if (count) {
            if (fIndex + count >= fCapacity) {
                ensureCapacity(count);
            }
            memcpy(&fBuffer[fIndex], chars, count * sizeof(XMLCh));
            fIndex += count;
        }
        else {
            append(chars);
        }
    }

    void append (const XMLCh* const chars)
    {
        if (chars != 0 && *chars != 0) {
            // get length of chars
            XMLSize_t count = 0;
            for (; *(chars+count); count++ ) /*noop*/;

            if (fIndex + count >= fCapacity) {
                ensureCapacity(count);
            }
            memcpy(&fBuffer[fIndex], chars, count * sizeof(XMLCh));
            fIndex += count;
        }
    }

    void set (const XMLCh* const chars, const XMLSize_t count)
    {
        fIndex = 0;
        append(chars, count);
    }

    void set (const XMLCh* const chars)
    {
        fIndex = 0;
        if (chars != 0 && *chars != 0)
            append(chars);
    }

    const XMLCh* getRawBuffer() const
    {
        fBuffer[fIndex] = 0;
        return fBuffer;
    }

    XMLCh* getRawBuffer()
    {
        fBuffer[fIndex] = 0;
        return fBuffer;
    }

    void reset()
    {
        fIndex = 0;
    }

    // -----------------------------------------------------------------------
    //  Getters
    // -----------------------------------------------------------------------
    bool getInUse() const
    {
        return fUsed;
    }

    XMLSize_t getLen() const
    {
        return fIndex;
    }

    bool isEmpty() const
    {
        return (fIndex == 0);
    }

    // -----------------------------------------------------------------------
    //  Setters
    // -----------------------------------------------------------------------
    void setInUse(const bool newValue)
    {
        fUsed = newValue;
    }

private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XMLBuffer(const XMLBuffer&);
    XMLBuffer& operator=(const XMLBuffer&);

    // -----------------------------------------------------------------------
    //  Declare our friends
    // -----------------------------------------------------------------------
    friend class XMLBufBid;

    // -----------------------------------------------------------------------
    //  Private helpers
    // -----------------------------------------------------------------------
    void ensureCapacity(const XMLSize_t extraNeeded);


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
    //  fUsed
    //      Indicates whether this buffer is in use or not.
    //
    //  fFullHandler, fFullSize
    //      If fFullHandler is non-null, the buffer has a maximum size
    //      indicated by fFullSize. If writing to the buffer would exceed the
    //      buffer's maximum size, fFullHandler's bufferFull callback is
    //      invoked, to empty the buffer.
    // -----------------------------------------------------------------------
    XMLSize_t                   fIndex;
    XMLSize_t                   fCapacity;
    XMLSize_t                   fFullSize;
    bool                        fUsed;
    MemoryManager* const        fMemoryManager;
    XMLBufferFullHandler*       fFullHandler;
    XMLCh*                      fBuffer;
};

/**
 *  XMLBufferFullHandler is a callback interface for clients of
 *  XMLBuffers that impose a size restriction (e.g. XMLScanner).
 *  Note that this is intended solely as a mix-in for internal
 *  use, and therefore does not derive from XMemory (to avoid
 *  the ambiguous base class problem).
 */
class XMLPARSER_EXPORT XMLBufferFullHandler
{
public :

    virtual ~XMLBufferFullHandler() {}

    /**
     * Callback method, intended to allow clients of an XMLBuffer which has
     * become full to empty it appropriately.
     * @return true if the handler was able to empty the buffer (either
     * partially or completely), otherwise false to indicate an error.
     */
    virtual bool bufferFull(XMLBuffer&) = 0;

};

XERCES_CPP_NAMESPACE_END

#endif
