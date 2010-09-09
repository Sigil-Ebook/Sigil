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
 * $Id: XMLBufferMgr.hpp 673679 2008-07-03 13:50:10Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XMLBUFFERMGR_HPP)
#define XERCESC_INCLUDE_GUARD_XMLBUFFERMGR_HPP

#include <xercesc/framework/XMLBuffer.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XMLBufBid;

/**
 *  There are many places where XMLBuffer objects are needed. In order to
 *  avoid either constantly creating and destroying them or maintaining a
 *  fixed set and worrying about accidental reuse, a buffer manager can
 *  provide a pool of buffers which can be temporarily used and then put
 *  back into the pool. This provides a good compromise between performance
 *  and easier maintenance.
 */
class XMLPARSER_EXPORT XMLBufferMgr : public XMemory
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------

    /** @name Constructor */
    //@{
    XMLBufferMgr(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    //@}

    /** @name Destructor */
    //@{
    ~XMLBufferMgr();
    //@}


    // -----------------------------------------------------------------------
    //  Buffer management
    // -----------------------------------------------------------------------
    XMLBuffer& bidOnBuffer();
    void releaseBuffer(XMLBuffer& toRelease);

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    XMLSize_t getBufferCount() const;
    XMLSize_t getAvailableBufferCount() const;

private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XMLBufferMgr(const XMLBufferMgr&);
    XMLBufferMgr& operator=(const XMLBufferMgr&);

    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fBufCount
    //      The count of buffers that have been allocated so far.
    //
    //  fBufList;
    //      The list of pointers to buffers that are loaned out. There will
    //      never be a lot of them, so a flat list is good enough.
    // -----------------------------------------------------------------------
    XMLSize_t       fBufCount;
    MemoryManager*  fMemoryManager;
    XMLBuffer**     fBufList;
};

inline XMLSize_t XMLBufferMgr::getBufferCount() const
{
    return fBufCount;
}

inline XMLSize_t XMLBufferMgr::getAvailableBufferCount() const
{
    XMLSize_t available = fBufCount;
    for (XMLSize_t index = 0; index < fBufCount && fBufList[index]; index++)
    {
        if (fBufList[index]->getInUse())
            --available;
    }
    return available;
}


/**
 *  XMLBufBid is a scoped based janitor that allows the scanner code to ask
 *  for a buffer on a scoped basis and then insure that it gets freed back
 *  into the pool no matter how the scope is exited (exception or normal exit.)
 */
class XMLBufBid : public XMemory
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    XMLBufBid(XMLBufferMgr* const srcMgr) :

        fBuffer(srcMgr->bidOnBuffer())
        , fMgr(srcMgr)
    {
    }

    ~XMLBufBid()
    {
        fMgr->releaseBuffer(fBuffer);
    }



    // -----------------------------------------------------------------------
    //  Buffer access
    // -----------------------------------------------------------------------
    void append(const XMLCh toAppend)
    {
        fBuffer.append(toAppend);
    }

    void append(const XMLCh* const toAppend, const XMLSize_t count = 0)
    {
        fBuffer.append(toAppend, count);
    }

    const XMLBuffer& getBuffer() const
    {
        return fBuffer;
    }

    XMLBuffer& getBuffer()
    {
        return fBuffer;
    }

    const XMLCh* getRawBuffer() const
    {
        fBuffer.fBuffer[fBuffer.fIndex] = 0;
        return fBuffer.fBuffer;
    }

    XMLCh* getRawBuffer()
    {
        fBuffer.fBuffer[fBuffer.fIndex] = 0;
        return fBuffer.fBuffer;
    }

    XMLSize_t getLen() const
    {
        return fBuffer.fIndex;
    }

    bool isEmpty() const
    {
        return (fBuffer.fIndex == 0);
    }

    void reset()
    {
        fBuffer.reset();
    }

    void set(const XMLCh* const chars, const XMLSize_t count = 0)
    {
        fBuffer.set(chars, count);
    }


private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XMLBufBid(const XMLBufBid&);
    XMLBufBid& operator=(const XMLBufBid&);

    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fBuffer
    //      This is the buffer we got, and which we will release.
    //
    //  fMgr
    //      This is the buffer manager we got the buffer from. This is needed
    //      to release the buffer later.
    // -----------------------------------------------------------------------
    XMLBuffer&          fBuffer;
    XMLBufferMgr* const fMgr;
};

XERCES_CPP_NAMESPACE_END

#endif
