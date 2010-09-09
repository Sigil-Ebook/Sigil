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
 * $Id: CurlURLInputStream.cpp 936316 2010-04-21 14:19:58Z borisk $
 */

#if HAVE_CONFIG_H
  #include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if HAVE_ERRNO_H
  #include <errno.h>
#endif
#if HAVE_UNISTD_H
  #include <unistd.h>
#endif
#if HAVE_SYS_TYPES_H
  #include <sys/types.h>
#endif
#if HAVE_SYS_TIME_H
  #include <sys/time.h>
#endif

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/util/XMLNetAccessor.hpp>
#include <xercesc/util/NetAccessors/Curl/CurlURLInputStream.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLExceptMsgs.hpp>
#include <xercesc/util/Janitor.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/util/TranscodingException.hpp>
#include <xercesc/util/PlatformUtils.hpp>

XERCES_CPP_NAMESPACE_BEGIN


CurlURLInputStream::CurlURLInputStream(const XMLURL& urlSource, const XMLNetHTTPInfo* httpInfo/*=0*/)
      : fMulti(0)
      , fEasy(0)
      , fMemoryManager(urlSource.getMemoryManager())
      , fURLSource(urlSource)
      , fTotalBytesRead(0)
      , fWritePtr(0)
      , fBytesRead(0)
      , fBytesToRead(0)
      , fDataAvailable(false)
      , fBufferHeadPtr(fBuffer)
      , fBufferTailPtr(fBuffer)
      , fPayload(0)
      , fPayloadLen(0)
      , fContentType(0)
{
	// Allocate the curl multi handle
	fMulti = curl_multi_init();

	// Allocate the curl easy handle
	fEasy = curl_easy_init();

	// Set URL option
    TranscodeToStr url(fURLSource.getURLText(), "ISO8859-1", fMemoryManager);
	curl_easy_setopt(fEasy, CURLOPT_URL, (char*)url.str());

    // Set up a way to recieve the data
	curl_easy_setopt(fEasy, CURLOPT_WRITEDATA, this);						// Pass this pointer to write function
	curl_easy_setopt(fEasy, CURLOPT_WRITEFUNCTION, staticWriteCallback);	// Our static write function

	// Do redirects
	curl_easy_setopt(fEasy, CURLOPT_FOLLOWLOCATION, (long)1);
	curl_easy_setopt(fEasy, CURLOPT_MAXREDIRS, (long)6);

    // Add username and password if authentication is required
    const XMLCh *username = urlSource.getUser();
    const XMLCh *password = urlSource.getPassword();
    if(username && password) {
        XMLBuffer userPassBuf(256, fMemoryManager);
        userPassBuf.append(username);
        userPassBuf.append(chColon);
        userPassBuf.append(password);

        TranscodeToStr userPass(userPassBuf.getRawBuffer(), "ISO8859-1", fMemoryManager);

        curl_easy_setopt(fEasy, CURLOPT_HTTPAUTH, (long)CURLAUTH_ANY);
        curl_easy_setopt(fEasy, CURLOPT_USERPWD, (char*)userPass.str());
    }

    if(httpInfo) {
        // Set the correct HTTP method
        switch(httpInfo->fHTTPMethod) {
        case XMLNetHTTPInfo::GET:
            break;
        case XMLNetHTTPInfo::PUT:
            curl_easy_setopt(fEasy, CURLOPT_UPLOAD, (long)1);
            break;
        case XMLNetHTTPInfo::POST:
            curl_easy_setopt(fEasy, CURLOPT_POST, (long)1);
            break;
        }

        // Add custom headers
        if(httpInfo->fHeaders) {
            struct curl_slist *headersList = 0;

            const char *headersBuf = httpInfo->fHeaders;
            const char *headersBufEnd = httpInfo->fHeaders + httpInfo->fHeadersLen;

            const char *headerStart = headersBuf;
            while(headersBuf < headersBufEnd) {
                if(*headersBuf == '\r' && (headersBuf + 1) < headersBufEnd &&
                   *(headersBuf + 1) == '\n') {

                    XMLSize_t length = headersBuf - headerStart;
                    ArrayJanitor<char> header((char*)fMemoryManager->allocate((length + 1) * sizeof(char)),
                                              fMemoryManager);
                    memcpy(header.get(), headerStart, length);
                    header.get()[length] = 0;

                    headersList = curl_slist_append(headersList, header.get());

                    headersBuf += 2;
                    headerStart = headersBuf;
                    continue;
                }
                ++headersBuf;
            }
            curl_easy_setopt(fEasy, CURLOPT_HTTPHEADER, headersList);
            curl_slist_free_all(headersList);
        }

        // Set up the payload
        if(httpInfo->fPayload) {
            fPayload = httpInfo->fPayload;
            fPayloadLen = httpInfo->fPayloadLen;
            curl_easy_setopt(fEasy, CURLOPT_READDATA, this);
            curl_easy_setopt(fEasy, CURLOPT_READFUNCTION, staticReadCallback);
            curl_easy_setopt(fEasy, CURLOPT_INFILESIZE_LARGE, (curl_off_t)fPayloadLen);
        }
    }

	// Add easy handle to the multi stack
	curl_multi_add_handle(fMulti, fEasy);

    // Start reading, to get the content type
	while(fBufferHeadPtr == fBuffer)
	{
		int runningHandles = 0;
        readMore(&runningHandles);
		if(runningHandles == 0) break;
	}

    // Find the content type
    char *contentType8 = 0;
    curl_easy_getinfo(fEasy, CURLINFO_CONTENT_TYPE, &contentType8);
    if(contentType8)
        fContentType = TranscodeFromStr((XMLByte*)contentType8, XMLString::stringLen(contentType8), "ISO8859-1", fMemoryManager).adopt();
}


CurlURLInputStream::~CurlURLInputStream()
{
	// Remove the easy handle from the multi stack
	curl_multi_remove_handle(fMulti, fEasy);

	// Cleanup the easy handle
	curl_easy_cleanup(fEasy);

	// Cleanup the multi handle
	curl_multi_cleanup(fMulti);

    if(fContentType) fMemoryManager->deallocate(fContentType);
}


size_t
CurlURLInputStream::staticWriteCallback(char *buffer,
                                        size_t size,
                                        size_t nitems,
                                        void *outstream)
{
	return ((CurlURLInputStream*)outstream)->writeCallback(buffer, size, nitems);
}

size_t
CurlURLInputStream::staticReadCallback(char *buffer,
                                       size_t size,
                                       size_t nitems,
                                       void *stream)
{
    return ((CurlURLInputStream*)stream)->readCallback(buffer, size, nitems);
}

size_t
CurlURLInputStream::writeCallback(char *buffer,
                                  size_t size,
                                  size_t nitems)
{
	XMLSize_t cnt = size * nitems;
	XMLSize_t totalConsumed = 0;

	// Consume as many bytes as possible immediately into the buffer
	XMLSize_t consume = (cnt > fBytesToRead) ? fBytesToRead : cnt;
	memcpy(fWritePtr, buffer, consume);
	fWritePtr		+= consume;
	fBytesRead		+= consume;
	fTotalBytesRead	+= consume;
	fBytesToRead	-= consume;

	//printf("write callback consuming %d bytes\n", consume);

	// If bytes remain, rebuffer as many as possible into our holding buffer
	buffer			+= consume;
	totalConsumed	+= consume;
	cnt				-= consume;
	if (cnt > 0)
	{
		XMLSize_t bufAvail = sizeof(fBuffer) - (fBufferHeadPtr - fBuffer);
		consume = (cnt > bufAvail) ? bufAvail : cnt;
		memcpy(fBufferHeadPtr, buffer, consume);
		fBufferHeadPtr	+= consume;
		buffer			+= consume;
		totalConsumed	+= consume;
		//printf("write callback rebuffering %d bytes\n", consume);
	}

	// Return the total amount we've consumed. If we don't consume all the bytes
	// then an error will be generated. Since our buffer size is equal to the
	// maximum size that curl will write, this should never happen unless there
	// is a logic error somewhere here.
	return totalConsumed;
}

size_t
CurlURLInputStream::readCallback(char *buffer,
                                 size_t size,
                                 size_t nitems)
{
    XMLSize_t len = size * nitems;
    if(len > fPayloadLen) len = fPayloadLen;

    memcpy(buffer, fPayload, len);

    fPayload += len;
    fPayloadLen -= len;

    return len;
}

bool CurlURLInputStream::readMore(int *runningHandles)
{
    // Ask the curl to do some work
    CURLMcode curlResult = curl_multi_perform(fMulti, runningHandles);

    // Process messages from curl
    int msgsInQueue = 0;
    for (CURLMsg* msg = NULL; (msg = curl_multi_info_read(fMulti, &msgsInQueue)) != NULL; )
    {
        //printf("msg %d, %d from curl\n", msg->msg, msg->data.result);

        if (msg->msg != CURLMSG_DONE)
            return true;

        switch (msg->data.result)
        {
        case CURLE_OK:
            // We completed successfully. runningHandles should have dropped to zero, so we'll bail out below...
            break;

        case CURLE_UNSUPPORTED_PROTOCOL:
            ThrowXMLwithMemMgr(MalformedURLException, XMLExcepts::URL_UnsupportedProto, fMemoryManager);
            break;

        case CURLE_COULDNT_RESOLVE_HOST:
        case CURLE_COULDNT_RESOLVE_PROXY:
          {
            if (fURLSource.getHost())
              ThrowXMLwithMemMgr1(NetAccessorException, XMLExcepts::NetAcc_TargetResolution, fURLSource.getHost(), fMemoryManager);
            else
              ThrowXMLwithMemMgr1(NetAccessorException, XMLExcepts::File_CouldNotOpenFile, fURLSource.getURLText(), fMemoryManager);
            break;
          }

        case CURLE_COULDNT_CONNECT:
            ThrowXMLwithMemMgr1(NetAccessorException, XMLExcepts::NetAcc_ConnSocket, fURLSource.getURLText(), fMemoryManager);
            break;

        case CURLE_RECV_ERROR:
            ThrowXMLwithMemMgr1(NetAccessorException, XMLExcepts::NetAcc_ReadSocket, fURLSource.getURLText(), fMemoryManager);
            break;

        default:
            ThrowXMLwithMemMgr1(NetAccessorException, XMLExcepts::NetAcc_InternalError, fURLSource.getURLText(), fMemoryManager);
            break;
        }
    }

    // If nothing is running any longer, bail out
    if(*runningHandles == 0)
        return false;

    // If there is no further data to read, and we haven't
    // read any yet on this invocation, call select to wait for data
    if (curlResult != CURLM_CALL_MULTI_PERFORM && fBytesRead == 0)
    {
        fd_set readSet;
        fd_set writeSet;
        fd_set exceptSet;
        int fdcnt=0;

        FD_ZERO(&readSet);
        FD_ZERO(&writeSet);
        FD_ZERO(&exceptSet);

        // Ask curl for the file descriptors to wait on
        curl_multi_fdset(fMulti, &readSet, &writeSet, &exceptSet, &fdcnt);

        // Wait on the file descriptors
        timeval tv;
        tv.tv_sec  = 2;
        tv.tv_usec = 0;
        select(fdcnt+1, &readSet, &writeSet, &exceptSet, &tv);
    }

    return curlResult == CURLM_CALL_MULTI_PERFORM;
}

XMLSize_t
CurlURLInputStream::readBytes(XMLByte* const          toFill
                                     , const XMLSize_t maxToRead)
{
	fBytesRead = 0;
	fBytesToRead = maxToRead;
	fWritePtr = toFill;

	for (bool tryAgain = true; fBytesToRead > 0 && (tryAgain || fBytesRead == 0); )
	{
		// First, any buffered data we have available
		XMLSize_t bufCnt = fBufferHeadPtr - fBufferTailPtr;
		bufCnt = (bufCnt > fBytesToRead) ? fBytesToRead : bufCnt;
		if (bufCnt > 0)
		{
			memcpy(fWritePtr, fBufferTailPtr, bufCnt);
			fWritePtr		+= bufCnt;
			fBytesRead		+= bufCnt;
			fTotalBytesRead	+= bufCnt;
			fBytesToRead	-= bufCnt;

			fBufferTailPtr	+= bufCnt;
			if (fBufferTailPtr == fBufferHeadPtr)
				fBufferHeadPtr = fBufferTailPtr = fBuffer;

			//printf("consuming %d buffered bytes\n", bufCnt);

			tryAgain = true;
			continue;
		}

		// Ask the curl to do some work
		int runningHandles = 0;
        tryAgain = readMore(&runningHandles);

		// If nothing is running any longer, bail out
		if (runningHandles == 0)
			break;
	}

	return fBytesRead;
}

const XMLCh *CurlURLInputStream::getContentType() const
{
    return fContentType;
}

XERCES_CPP_NAMESPACE_END
