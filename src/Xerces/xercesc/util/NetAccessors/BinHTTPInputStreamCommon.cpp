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
 * $Id: BinFileInputStream.cpp 553903 2007-07-06 14:43:42Z amassari $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdlib.h>
#include <string.h>

#include <xercesc/util/NetAccessors/BinHTTPInputStreamCommon.hpp>

#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLExceptMsgs.hpp>
#include <xercesc/util/Janitor.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/Base64.hpp>

XERCES_CPP_NAMESPACE_BEGIN

BinHTTPInputStreamCommon::BinHTTPInputStreamCommon(MemoryManager *manager)
      : fBytesProcessed(0)
      , fBuffer(1023, manager)
      , fContentType(0)
      , fMemoryManager(manager)
{
}


BinHTTPInputStreamCommon::~BinHTTPInputStreamCommon()
{
    if(fContentType) fMemoryManager->deallocate(fContentType);
}

static const char *CRLF = "\r\n";

void BinHTTPInputStreamCommon::createHTTPRequest(const XMLURL &urlSource, const XMLNetHTTPInfo *httpInfo, CharBuffer &buffer)
{
    static const char *GET = "GET ";
    static const char *PUT = "PUT ";
    static const char *POST = "POST ";
    static const char *HTTP10 = " HTTP/1.0\r\n";
    static const char *HOST = "Host: ";
    static const char *AUTHORIZATION = "Authorization: Basic ";
    static const char *COLON = ":";

    XMLTransService::Codes failReason;
    const XMLSize_t blockSize = 2048;

    XMLTranscoder* trans = XMLPlatformUtils::fgTransService->makeNewTranscoderFor("ISO8859-1", failReason, blockSize, fMemoryManager);
    Janitor<XMLTranscoder> janTrans(trans);

    TranscodeToStr hostName(urlSource.getHost(), trans, fMemoryManager);
    TranscodeToStr path(urlSource.getPath(), trans, fMemoryManager);
    TranscodeToStr fragment(urlSource.getFragment(), trans, fMemoryManager);
    TranscodeToStr query(urlSource.getQuery(), trans, fMemoryManager);

    // Build up the http GET command to send to the server.
    // To do:  We should really support http 1.1.  This implementation
    //         is weak.
    if(httpInfo) {
        switch(httpInfo->fHTTPMethod) {
        case XMLNetHTTPInfo::GET:   buffer.append(GET); break;
        case XMLNetHTTPInfo::PUT:   buffer.append(PUT); break;
        case XMLNetHTTPInfo::POST:  buffer.append(POST); break;
        }
    }
    else {
        buffer.append(GET);
    }

    if(path.str() != 0) {
        buffer.append((char*)path.str());
    }
    else {
        buffer.append("/");
    }

    if(query.str() != 0) {
        buffer.append("?");
        buffer.append((char*)query.str());
    }

    if(fragment.str() != 0) {
        buffer.append((char*)fragment.str());
    }
    buffer.append(HTTP10);

    buffer.append(HOST);
    buffer.append((char*)hostName.str());
    if(urlSource.getPortNum() != 80)
    {
        buffer.append(COLON);
        buffer.appendDecimalNumber(urlSource.getPortNum());
    }
    buffer.append(CRLF);

    const XMLCh *username = urlSource.getUser();
    const XMLCh *password = urlSource.getPassword();
    if(username && password) {
        XMLBuffer userPassBuf(256, fMemoryManager);
        userPassBuf.append(username);
        userPassBuf.append(chColon);
        userPassBuf.append(password);

        TranscodeToStr userPass(userPassBuf.getRawBuffer(), trans, fMemoryManager);

        XMLSize_t len;
        XMLByte* encodedData = Base64::encode(userPass.str(), userPass.length(), &len, fMemoryManager);
        ArrayJanitor<XMLByte> janBuf2(encodedData, fMemoryManager);

        if(encodedData) {
            // HTTP doesn't want the 0x0A separating the data in chunks of 76 chars per line
            XMLByte* authData = (XMLByte*)fMemoryManager->allocate((len+1)*sizeof(XMLByte));
            ArrayJanitor<XMLByte> janBuf(authData, fMemoryManager);
            XMLByte *cursor = authData;
            for(XMLSize_t i = 0; i < len; ++i)
                if(encodedData[i] != chLF)
                    *cursor++ = encodedData[i];
            *cursor++ = 0;
            buffer.append(AUTHORIZATION);
            buffer.append((char*)authData);
            buffer.append(CRLF);
        }
    }

    if(httpInfo && httpInfo->fHeaders)
        buffer.append(httpInfo->fHeaders, httpInfo->fHeadersLen);

    buffer.append(CRLF);
}

XMLCh *BinHTTPInputStreamCommon::findHeader(const char *name)
{
    XMLSize_t len = strlen(name);

    char *p = strstr(fBuffer.getRawBuffer(), name);
    while(p != 0) {
        if(*(p - 1) == '\n' &&
            *(p + len) == ':' &&
            *(p + len + 1) == ' ') {

            p += len + 2;

            char *endP = strstr(p, CRLF);
            if(endP == 0) {
                for(endP = p; *endP != 0; ++endP) ;
            }

            // Transcode from iso-8859-1
            TranscodeFromStr value((XMLByte*)p, endP - p, "ISO8859-1", fMemoryManager);
            return value.adopt();
        }

        p = strstr(p + 1, name);
    }

    return 0;
}

int BinHTTPInputStreamCommon::sendRequest(const XMLURL &url, const XMLNetHTTPInfo *httpInfo)
{
    //
    //  Constants in ASCII to send/check in the HTTP request/response
    //

    static const char *CRLF2X = "\r\n\r\n";
    static const char *LF2X = "\n\n";

    // The port is open and ready to go.
    // Build up the http GET command to send to the server.
    CharBuffer requestBuffer(1023, fMemoryManager);
    createHTTPRequest(url, httpInfo, requestBuffer);

    // Send the http request
    if(!send(requestBuffer.getRawBuffer(), requestBuffer.getLen())) {
        ThrowXMLwithMemMgr1(NetAccessorException,
                            XMLExcepts::NetAcc_WriteSocket, url.getURLText(), fMemoryManager);
    }

    if(httpInfo && httpInfo->fPayload) {
        if(!send(httpInfo->fPayload, httpInfo->fPayloadLen)) {
            ThrowXMLwithMemMgr1(NetAccessorException,
                                XMLExcepts::NetAcc_WriteSocket, url.getURLText(), fMemoryManager);
        }
    }

    //
    // get the response, check the http header for errors from the server.
    //
    char tmpBuf[1024];
    int ret;

    fBuffer.reset();
    while(true) {
        ret = receive(tmpBuf, sizeof(tmpBuf));
        if(ret == -1) {
            ThrowXMLwithMemMgr1(NetAccessorException, XMLExcepts::NetAcc_ReadSocket, url.getURLText(), fMemoryManager);
        }

        fBuffer.append(tmpBuf, ret);

        fBufferPos = strstr(fBuffer.getRawBuffer(), CRLF2X);
        if(fBufferPos != 0) {
            fBufferPos += 4;
            *(fBufferPos - 2) = 0;
            break;
        }

        fBufferPos = strstr(fBuffer.getRawBuffer(), LF2X);
        if(fBufferPos != 0) {
            fBufferPos += 2;
            *(fBufferPos - 1) = 0;
            break;
        }
    }

    // Parse the response status
    char *p = strstr(fBuffer.getRawBuffer(), "HTTP");
    if(p == 0) {
        ThrowXMLwithMemMgr1(NetAccessorException, XMLExcepts::NetAcc_ReadSocket, url.getURLText(), fMemoryManager);
    }

    p = strchr(p, chSpace);
    if(p == 0) {
        ThrowXMLwithMemMgr1(NetAccessorException, XMLExcepts::NetAcc_ReadSocket, url.getURLText(), fMemoryManager);
    }

    return atoi(p);
}

const XMLCh *BinHTTPInputStreamCommon::getContentType() const
{
    if(fContentType == 0) {
        // mutable
        const_cast<BinHTTPInputStreamCommon*>(this)->fContentType =
        const_cast<BinHTTPInputStreamCommon*>(this)->findHeader("Content-Type");
    }
    return fContentType;
}

XMLSize_t BinHTTPInputStreamCommon::readBytes(XMLByte* const    toFill,
                                              const XMLSize_t    maxToRead)
{
    XMLSize_t len = fBuffer.getRawBuffer() + fBuffer.getLen() - fBufferPos;
    if(len > 0)
    {
        // If there's any data left over in the buffer into which we first
        //   read from the server (to get the http header), return that.
        if (len > maxToRead)
            len = maxToRead;
        memcpy(toFill, fBufferPos, len);
        fBufferPos += len;
    }
    else
    {
        // There was no data in the local buffer.
        // Read some from the socket, straight into our caller's buffer.
        //
        int cbRead = receive((char *)toFill, maxToRead);
        if (cbRead == -1)
        {
            ThrowXMLwithMemMgr(NetAccessorException, XMLExcepts::NetAcc_ReadSocket, fMemoryManager);
        }
        len = cbRead;
    }

    fBytesProcessed += len;
    return len;
}

XERCES_CPP_NAMESPACE_END
