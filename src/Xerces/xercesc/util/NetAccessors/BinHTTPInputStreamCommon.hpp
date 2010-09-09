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
 * $Id: BinFileInputStream.hpp 553903 2007-07-06 14:43:42Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_BINHTTPINPUTSTREAMCOMMON_HPP)
#define XERCESC_INCLUDE_GUARD_BINHTTPINPUTSTREAMCOMMON_HPP

#include <xercesc/util/XMLURL.hpp>
#include <xercesc/util/BinInputStream.hpp>
#include <xercesc/util/XMLNetAccessor.hpp>
#include <xercesc/framework/MemoryManager.hpp>
#include <string.h>

XERCES_CPP_NAMESPACE_BEGIN

//
// This class implements a simple expanding character buffer
//
class XMLUTIL_EXPORT CharBuffer
{
public:
    CharBuffer(XMLSize_t capacity = 1023,
               MemoryManager *manager = XMLPlatformUtils::fgMemoryManager)
        : fCapacity(capacity),
          fIndex(0),
          fMemoryManager(manager)
    {
        fBuffer = (char*)fMemoryManager->allocate((fCapacity + 1) * sizeof(char));
    }

    ~CharBuffer()
    {
        fMemoryManager->deallocate(fBuffer);
    }

    const char* getRawBuffer() const
    {
        fBuffer[fIndex] = 0;
        return fBuffer;
    }

    char* getRawBuffer()
    {
        fBuffer[fIndex] = 0;
        return fBuffer;
    }

    XMLSize_t getLen() const
    {
        return fIndex;
    }

    void reset()
    {
        fIndex = 0;
    }

    void append(const char *chars)
    {
        if(chars != 0 && *chars != 0) {
            // get length of chars
            XMLSize_t count = 0;
            for(; *(chars+count); ++count) ;

            if(fIndex + count >= fCapacity) {
                ensureCapacity(count);
            }
            memcpy(&fBuffer[fIndex], chars, count * sizeof(char));
            fIndex += count;
        }
    }

    void append(const char *chars, XMLSize_t len)
    {
        if(chars != 0 && len != 0) {
            if(fIndex + len >= fCapacity) {
                ensureCapacity(len);
            }
            memcpy(&fBuffer[fIndex], chars, len * sizeof(char));
            fIndex += len;
        }
    }

    void appendDecimalNumber(unsigned int n)
    {
        if(n >= 10) {
		appendDecimalNumber(n / 10);
		n = n % 10;
	}

        if(fIndex + 1 >= fCapacity)
            ensureCapacity(1);

        fBuffer[fIndex] = '0' + n;
        ++fIndex;
    }

    void set(const char *chars)
    {
        reset();
        append(chars);
    }

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    CharBuffer(const CharBuffer &);
    CharBuffer &operator=(const CharBuffer &);

    void ensureCapacity(XMLSize_t extraNeeded)
    {
        // If we can't handle it, try doubling the buffer size.
        XMLSize_t newCap = (fIndex + extraNeeded) * 2;

        if(newCap > fCapacity)
        {
            // Allocate new buffer
            char* newBuf = (char*)fMemoryManager->allocate((newCap + 1) * sizeof(char));

            // Copy over the old stuff
            memcpy(newBuf, fBuffer, fIndex * sizeof(char));

            // Clean up old buffer and store new stuff
            fMemoryManager->deallocate(fBuffer);
            fBuffer = newBuf;
            fCapacity = newCap;
        }
    }

    // -----------------------------------------------------------------------
    //  Private data members
    // -----------------------------------------------------------------------
    char *fBuffer;
    XMLSize_t fCapacity;
    XMLSize_t fIndex;
    MemoryManager *fMemoryManager;
};

//
// Common base implementation of HTTP BinInputStream that is used by some
// platform-specific implementations.
//
class XMLUTIL_EXPORT BinHTTPInputStreamCommon : public BinInputStream
{
public :
    virtual XMLFilePos curPos() const;
    virtual XMLSize_t readBytes
    (
                XMLByte* const  toFill
        , const XMLSize_t    maxToRead
    );

    virtual const XMLCh *getContentType() const;

protected :
    BinHTTPInputStreamCommon(MemoryManager *manager);
    virtual ~BinHTTPInputStreamCommon();

    /**
     * \return The HTTP status code
     */
    int sendRequest(const XMLURL &url, const XMLNetHTTPInfo *httpInfo);
    XMLCh *findHeader(const char *name);

    virtual bool send(const char *buf, XMLSize_t len) = 0;
    /**
     * \return The length of the data received, or -1 if an error occured
     */
    virtual int receive(char *buf, XMLSize_t len) = 0;

private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    BinHTTPInputStreamCommon(const BinHTTPInputStreamCommon&);
    BinHTTPInputStreamCommon& operator=(const BinHTTPInputStreamCommon&);

    void createHTTPRequest(const XMLURL &urlSource, const XMLNetHTTPInfo *httpInfo, CharBuffer &buffer);

    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fBytesProcessed
    //      Its a rolling count of the number of bytes processed off this
    //      input stream.
    //  fBuffer
    //      Holds the http header, plus the first part of the actual
    //      data.  Filled at the time the stream is opened, data goes
    //      out to user in response to readBytes().
    //  fBufferPos
    //      Pointers into fBuffer, showing start and end+1 of content
    //      that readBytes must return.
    // -----------------------------------------------------------------------

    XMLSize_t           fBytesProcessed;
    CharBuffer          fBuffer;
    char *              fBufferPos;
    XMLCh *             fContentType;
    MemoryManager*      fMemoryManager;
};


inline XMLFilePos BinHTTPInputStreamCommon::curPos() const
{
    return fBytesProcessed;
}

XERCES_CPP_NAMESPACE_END

#endif
