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
 * $Id: BinMemInputStream.hpp 670359 2008-06-22 13:43:45Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_BINMEMINPUTSTREAM_HPP)
#define XERCESC_INCLUDE_GUARD_BINMEMINPUTSTREAM_HPP

#include <xercesc/util/BinInputStream.hpp>
#include <xercesc/util/PlatformUtils.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XMLUTIL_EXPORT BinMemInputStream : public BinInputStream
{
public :
    // -----------------------------------------------------------------------
    //  Class specific types
    // -----------------------------------------------------------------------
    enum BufOpts
    {
        BufOpt_Adopt
        , BufOpt_Copy
        , BufOpt_Reference
    };


    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    BinMemInputStream
    (
        const   XMLByte* const  initData
        , const XMLSize_t       capacity
        , const BufOpts         bufOpt = BufOpt_Copy
        , MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
    );
    virtual ~BinMemInputStream();


    // -----------------------------------------------------------------------
    //  Stream management methods
    // -----------------------------------------------------------------------
    void reset();


    // -----------------------------------------------------------------------
    //  Implementation of the input stream interface
    // -----------------------------------------------------------------------
    virtual XMLFilePos curPos() const;

    virtual XMLSize_t readBytes
    (
                XMLByte* const  toFill
        , const XMLSize_t       maxToRead
    );

    virtual const XMLCh* getContentType() const;

    inline XMLSize_t getSize() const;

private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    BinMemInputStream(const BinMemInputStream&);
    BinMemInputStream& operator=(const BinMemInputStream&);
    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fBuffer
    //      The buffer of bytes that we are streaming.
    //
    //  fBufOpt
    //      Indicates the ownership status of the buffer. The caller can have
    //      us adopt it (we delete it), reference it, or just make our own
    //      copy of it.
    //
    //  fCapacity
    //      The size of the buffer being streamed.
    //
    //  fCurIndex
    //      The current index where the next byte will be read from. When it
    //      hits fCapacity, we are done.
    // -----------------------------------------------------------------------
    const XMLByte*  fBuffer;
    BufOpts         fBufOpt;
    XMLSize_t       fCapacity;
    XMLSize_t       fCurIndex;
    MemoryManager*  fMemoryManager;
};


// ---------------------------------------------------------------------------
//  BinMemInputStream: Stream management methods
// ---------------------------------------------------------------------------
inline void BinMemInputStream::reset()
{
    fCurIndex = 0;
}


// ---------------------------------------------------------------------------
//  BinMemInputStream: Implementation of the input stream interface
// ---------------------------------------------------------------------------
inline XMLFilePos BinMemInputStream::curPos() const
{
    return fCurIndex;
}

inline XMLSize_t BinMemInputStream::getSize() const
{
    return fCapacity;
}

XERCES_CPP_NAMESPACE_END

#endif
