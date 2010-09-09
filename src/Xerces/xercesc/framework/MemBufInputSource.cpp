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
 * $Id: MemBufInputSource.cpp 553941 2007-07-06 16:14:22Z amassari $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/BinMemInputStream.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  MemBufInputSource: Constructors and Destructor
// ---------------------------------------------------------------------------
MemBufInputSource::MemBufInputSource( const XMLByte* const  srcDocBytes
                                    , const XMLSize_t       byteCount
                                    , const XMLCh* const    bufId
                                    , const bool            adoptBuffer
                                    , MemoryManager* const  manager) :
    InputSource(bufId, manager)
    , fAdopted(adoptBuffer)
    , fByteCount(byteCount)
    , fCopyBufToStream(true)
    , fSrcBytes(srcDocBytes)
{
}

MemBufInputSource::MemBufInputSource( const XMLByte* const  srcDocBytes
                                    , const XMLSize_t       byteCount
                                    , const char* const     bufId
                                    , const bool            adoptBuffer
                                    , MemoryManager* const  manager) :
    InputSource(bufId, manager)
    , fAdopted(adoptBuffer)
    , fByteCount(byteCount)
    , fCopyBufToStream(true)
    , fSrcBytes(srcDocBytes)
{
}

MemBufInputSource::~MemBufInputSource()
{
    if (fAdopted)
        delete [] (XMLByte*)fSrcBytes;
}

void MemBufInputSource::resetMemBufInputSource(const XMLByte* const  srcDocBytes
                                             , const XMLSize_t       byteCount)
{
    fByteCount = byteCount;
    fSrcBytes  = srcDocBytes;
}

// ---------------------------------------------------------------------------
//  MemBufInputSource: InputSource interface implementation
// ---------------------------------------------------------------------------
BinInputStream* MemBufInputSource::makeStream() const
{
    //
    //  Create a memory input stream over our buffer. According to our
    //  fCopyBufToStream flag, we either tell it to copy the buffer or to
    //  just reference it.
    //
    return new (getMemoryManager()) BinMemInputStream
    (
        fSrcBytes
        , fByteCount
        , fCopyBufToStream ? BinMemInputStream::BufOpt_Copy
                           : BinMemInputStream::BufOpt_Reference
        , getMemoryManager()
    );
}

XERCES_CPP_NAMESPACE_END

