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
 * $Id: UnixHTTPURLInputStream.cpp 936316 2010-04-21 14:19:58Z borisk $
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include <string.h>

#if HAVE_UNISTD_H
#  include <unistd.h>
#endif
#if HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif
#if HAVE_SYS_SOCKET_H
#  include <sys/socket.h>
#endif
#if HAVE_NETINET_IN_H
#  include <netinet/in.h>
#endif
#if HAVE_ARPA_INET_H
#  include <arpa/inet.h>
#endif
#if HAVE_NETDB_H
#  include <netdb.h>
#endif
#include <errno.h>

#include <xercesc/util/NetAccessors/Socket/UnixHTTPURLInputStream.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLExceptMsgs.hpp>
#include <xercesc/util/Janitor.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class SocketJanitor
{
public:
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    SocketJanitor(int* toDelete) : fData(toDelete) {}
    ~SocketJanitor() { reset(); }

    int* get() const { return fData; }
    int* release() { int* p = fData; fData = 0; return p; }

    void reset(int* p = 0)
    {
        if(fData) {
            shutdown(*fData, 2);
            close(*fData);
        }
        fData = p;
    }
    bool isDataNull() { return (fData == 0); }

private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    SocketJanitor();
    SocketJanitor(const SocketJanitor&);
    SocketJanitor& operator=(const SocketJanitor&);

    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fData
    //      This is the pointer to the socket that must be closed when
    //      this object is destroyed.
    // -----------------------------------------------------------------------
    int*  fData;
};

UnixHTTPURLInputStream::UnixHTTPURLInputStream(const XMLURL& urlSource, const XMLNetHTTPInfo* httpInfo/*=0*/)
    : BinHTTPInputStreamCommon(urlSource.getMemoryManager()),
      fSocket(0)
{
    //
    //  Convert the hostName to the platform's code page for gethostbyname and
    //  inet_addr functions.
    //

    MemoryManager *memoryManager = urlSource.getMemoryManager();

    const XMLCh*        hostName = urlSource.getHost();

    if (hostName == 0)
      ThrowXMLwithMemMgr1(NetAccessorException,
                          XMLExcepts::File_CouldNotOpenFile,
                          urlSource.getURLText(),
                          memoryManager);

    char*               hostNameAsCharStar = XMLString::transcode(hostName, memoryManager);
    ArrayJanitor<char>  janHostNameAsCharStar(hostNameAsCharStar, memoryManager);

    XMLURL url(urlSource);
    int redirectCount = 0;
    SocketJanitor janSock(0);

    do {
        //
        // Set up a socket.
        //

#if HAVE_GETADDRINFO
        struct addrinfo hints, *res, *ai;

        CharBuffer portBuffer(10, memoryManager);
        portBuffer.appendDecimalNumber(url.getPortNum());

        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = PF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        int n = getaddrinfo(hostNameAsCharStar,portBuffer.getRawBuffer(),&hints, &res);
        if(n != 0)
        {
            hints.ai_flags = AI_NUMERICHOST;
            n = getaddrinfo(hostNameAsCharStar,portBuffer.getRawBuffer(),&hints, &res);
            if(n != 0)
                ThrowXMLwithMemMgr1(NetAccessorException, XMLExcepts::NetAcc_TargetResolution, hostName, memoryManager);
        }
        janSock.reset();
        for (ai = res; ai != NULL; ai = ai->ai_next) {
            // Open a socket with the correct address family for this address.
            fSocket = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
            if (fSocket < 0)
                continue;
            janSock.reset(&fSocket);
            if (connect(fSocket, ai->ai_addr, ai->ai_addrlen) < 0)
            {
                freeaddrinfo(res);
                ThrowXMLwithMemMgr1(NetAccessorException,
                         XMLExcepts::NetAcc_ConnSocket, url.getURLText(), memoryManager);
            }
            break;
        }
        freeaddrinfo(res);
        if (fSocket < 0)
        {
            ThrowXMLwithMemMgr1(NetAccessorException,
                     XMLExcepts::NetAcc_CreateSocket, url.getURLText(), memoryManager);
        }
#else
        struct hostent *hostEntPtr = 0;
        struct sockaddr_in sa;

        // Use the hostName in the local code page ....
        if((hostEntPtr = gethostbyname(hostNameAsCharStar)) == NULL)
        {
            unsigned long  numAddress = inet_addr(hostNameAsCharStar);
            if ((hostEntPtr =
                    gethostbyaddr((char *) &numAddress,
                        sizeof(unsigned long), AF_INET)) == NULL)
            {
                ThrowXMLwithMemMgr1(NetAccessorException,
                    XMLExcepts::NetAcc_TargetResolution, hostName, memoryManager);
            }
        }

        memset(&sa, '\0', sizeof(sockaddr_in));  // iSeries fix ??
        memcpy((void *) &sa.sin_addr,
            (const void *) hostEntPtr->h_addr, hostEntPtr->h_length);
        sa.sin_family = hostEntPtr->h_addrtype;
        sa.sin_port = htons((unsigned short)url.getPortNum());

        janSock.reset();
        fSocket = socket(hostEntPtr->h_addrtype, SOCK_STREAM, 0);
        if(fSocket < 0)
        {
            ThrowXMLwithMemMgr1(NetAccessorException,
                XMLExcepts::NetAcc_CreateSocket, url.getURLText(), memoryManager);
        }
        janSock.reset(&fSocket);

        if(connect(fSocket, (struct sockaddr *) &sa, sizeof(sa)) < 0)
        {
            ThrowXMLwithMemMgr1(NetAccessorException,
                XMLExcepts::NetAcc_ConnSocket, url.getURLText(), memoryManager);
        }
#endif

        int status = sendRequest(url, httpInfo);

        if(status == 200) {
            // HTTP 200 OK response means we're done.
            break;
        }
        // a 3xx response means there was an HTTP redirect
        else if(status >= 300 && status <= 307) {
            redirectCount++;

            XMLCh *newURLString = findHeader("Location");
            ArrayJanitor<XMLCh> janNewURLString(newURLString, memoryManager);

            if(newURLString == 0 || *newURLString == 0) {
                ThrowXMLwithMemMgr1(NetAccessorException, XMLExcepts::File_CouldNotOpenFile, url.getURLText(), memoryManager);
            }

            XMLURL newURL(memoryManager);
            newURL.setURL(url, newURLString);
            if(newURL.getProtocol() != XMLURL::HTTP) {
                ThrowXMLwithMemMgr1(NetAccessorException, XMLExcepts::File_CouldNotOpenFile, newURL.getURLText(), memoryManager);
            }

            url = newURL;
            hostName = newURL.getHost();

            if (hostName == 0)
              ThrowXMLwithMemMgr1(NetAccessorException,
                                  XMLExcepts::File_CouldNotOpenFile,
                                  newURL.getURLText(),
                                  memoryManager);

            janHostNameAsCharStar.release();
            hostNameAsCharStar = XMLString::transcode(hostName, memoryManager);
            janHostNameAsCharStar.reset(hostNameAsCharStar, memoryManager);
        }
        else {
            // Most likely a 404 Not Found error.
            ThrowXMLwithMemMgr1(NetAccessorException, XMLExcepts::File_CouldNotOpenFile, url.getURLText(), memoryManager);
        }
    } while(redirectCount < 6);

    janSock.release();
}


UnixHTTPURLInputStream::~UnixHTTPURLInputStream()
{
    shutdown(fSocket, 2);
    close(fSocket);
}

bool UnixHTTPURLInputStream::send(const char *buf, XMLSize_t len)
{
    XMLSize_t done = 0;
    int ret;

    while(done < len) {
        ret = ::send(fSocket, buf + done, len - done, 0);
        if(ret == -1) return false;
        done += ret;
    }

    return true;
}

int UnixHTTPURLInputStream::receive(char *buf, XMLSize_t len)
{
    return ::recv(fSocket, buf, len, 0);
}

XERCES_CPP_NAMESPACE_END
