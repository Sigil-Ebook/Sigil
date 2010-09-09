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
 * $Id: BinHTTPURLInputStream.cpp 936316 2010-04-21 14:19:58Z borisk $
 */


#include <xercesc/util/NetAccessors/WinSock/BinHTTPURLInputStream.hpp>
#include <windows.h>

#ifdef WITH_IPV6
#include <ws2tcpip.h>
#endif

#include <stdio.h>
#include <string.h>

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLNetAccessor.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLExceptMsgs.hpp>
#include <xercesc/util/Janitor.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/Mutexes.hpp>

XERCES_CPP_NAMESPACE_BEGIN

typedef u_short (WSAAPI * LPFN_HTONS)(u_short hostshort);
typedef SOCKET (WSAAPI * LPFN_SOCKET)(int af, int type, int protocol);
typedef int (WSAAPI * LPFN_CONNECT)(SOCKET s, const struct sockaddr* name, int namelen);
typedef int (WSAAPI * LPFN_SEND)(SOCKET s, const char* buf, int len, int flags);
typedef int (WSAAPI * LPFN_RECV)(SOCKET s, char* buf, int len, int flags);
typedef int (WSAAPI * LPFN_SHUTDOWN)(SOCKET s, int how);
typedef int (WSAAPI * LPFN_CLOSESOCKET)(SOCKET s);
typedef int (WSAAPI * LPFN_WSACLEANUP)();
typedef int (WSAAPI * LPFN_WSASTARTUP)(WORD wVersionRequested, LPWSADATA lpWSAData);
#ifdef WITH_IPV6
typedef int (WSAAPI * LPFN_GETADDRINFO)(const char* nodename, const char * servname, const struct addrinfo * hints, struct addrinfo ** res);
typedef void (WSAAPI * LPFN_FREEADDRINFO)(struct addrinfo * ai);
#else
typedef struct hostent *(WSAAPI * LPFN_GETHOSTBYNAME)(const char* name);
typedef struct hostent *(WSAAPI * LPFN_GETHOSTBYADDR)(const char* addr, int len, int type);
typedef unsigned long (WSAAPI * LPFN_INET_ADDR)(const char* cp);
#endif

static HMODULE gWinsockLib = NULL;
static LPFN_HTONS gWShtons = NULL;
static LPFN_SOCKET gWSsocket = NULL;
static LPFN_CONNECT gWSconnect = NULL;
static LPFN_SEND gWSsend = NULL;
static LPFN_RECV gWSrecv = NULL;
static LPFN_SHUTDOWN gWSshutdown = NULL;
static LPFN_CLOSESOCKET gWSclosesocket = NULL;
static LPFN_WSACLEANUP gWSACleanup = NULL;
#ifdef WITH_IPV6
static LPFN_GETADDRINFO gWSgetaddrinfo = NULL;
static LPFN_FREEADDRINFO gWSfreeaddrinfo = NULL;
#else
static LPFN_GETHOSTBYNAME gWSgethostbyname = NULL;
static LPFN_GETHOSTBYADDR gWSgethostbyaddr = NULL;
static LPFN_INET_ADDR gWSinet_addr = NULL;
#endif

static u_short wrap_htons(u_short hostshort)
{
    return (*gWShtons)(hostshort);
}

static SOCKET wrap_socket(int af,int type,int protocol)
{
    return (*gWSsocket)(af,type,protocol);
}

static int wrap_connect(SOCKET s,const struct sockaddr* name,int namelen)
{
    return (*gWSconnect)(s,name,namelen);
}

static int wrap_send(SOCKET s,const char* buf,int len,int flags)
{
    return (*gWSsend)(s,buf,len,flags);
}

static int wrap_recv(SOCKET s,char* buf,int len,int flags)
{
    return (*gWSrecv)(s,buf,len,flags);
}

static int wrap_shutdown(SOCKET s,int how)
{
    return (*gWSshutdown)(s,how);
}

static int wrap_closesocket(SOCKET socket)
{
    return (*gWSclosesocket)(socket);
}

#ifdef WITH_IPV6
static int wrap_getaddrinfo(const char* nodename,const char* servname,const struct addrinfo* hints,struct addrinfo** res)
{
   return (*gWSgetaddrinfo)(nodename,servname,hints,res);
}

static void wrap_freeaddrinfo(struct addrinfo* ai)
{
    (*gWSfreeaddrinfo)(ai);
}
#else
static struct hostent* wrap_gethostbyname(const char* name)
{
    return (*gWSgethostbyname)(name);
}

static struct hostent* wrap_gethostbyaddr(const char* addr,int len,int type)
{
    return (*gWSgethostbyaddr)(addr,len,type);
}

static unsigned long wrap_inet_addr(const char* cp)
{
    return (*gWSinet_addr)(cp);
}

#endif


class SocketJanitor
{
public:
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    SocketJanitor(SOCKET* toDelete) : fData(toDelete) {}
    ~SocketJanitor() { reset(); }

    SOCKET* get() const { return fData; }
    SOCKET* release() {    SOCKET* p = fData; fData = 0; return p; }

    void reset(SOCKET* p = 0)
    {
        if(fData) {
            wrap_shutdown(*fData, SD_BOTH);
            wrap_closesocket(*fData);
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
    SOCKET*  fData;
};

bool BinHTTPURLInputStream::fInitialized = false;

void BinHTTPURLInputStream::Initialize(MemoryManager* const manager)
{
    //
    // Initialize the WinSock library here.
    //
    WORD        wVersionRequested;
    WSADATA     wsaData;

    LPFN_WSASTARTUP startup = NULL;
    if(gWinsockLib == NULL)
    {
#ifdef WITH_IPV6
      gWinsockLib = LoadLibraryA("WS2_32");
#else
      gWinsockLib = LoadLibraryA("WSOCK32");
#endif
      if(gWinsockLib == NULL)
      {
          ThrowXMLwithMemMgr(NetAccessorException, XMLExcepts::NetAcc_InitFailed, manager);
      }
      else
      {
          startup = (LPFN_WSASTARTUP) GetProcAddress(gWinsockLib,"WSAStartup");
          gWSACleanup = (LPFN_WSACLEANUP) GetProcAddress(gWinsockLib,"WSACleanup");
          gWShtons = (LPFN_HTONS) GetProcAddress(gWinsockLib,"htons");
          gWSsocket = (LPFN_SOCKET) GetProcAddress(gWinsockLib,"socket");
          gWSconnect = (LPFN_CONNECT) GetProcAddress(gWinsockLib,"connect");
          gWSsend = (LPFN_SEND) GetProcAddress(gWinsockLib,"send");
          gWSrecv = (LPFN_RECV) GetProcAddress(gWinsockLib,"recv");
          gWSshutdown = (LPFN_SHUTDOWN) GetProcAddress(gWinsockLib,"shutdown");
          gWSclosesocket = (LPFN_CLOSESOCKET) GetProcAddress(gWinsockLib,"closesocket");
#ifdef WITH_IPV6
          gWSgetaddrinfo = (LPFN_GETADDRINFO) GetProcAddress(gWinsockLib,"getaddrinfo");
          gWSfreeaddrinfo = (LPFN_FREEADDRINFO) GetProcAddress(gWinsockLib,"freeaddrinfo");
#else
          gWSgethostbyname = (LPFN_GETHOSTBYNAME) GetProcAddress(gWinsockLib,"gethostbyname");
          gWSgethostbyaddr = (LPFN_GETHOSTBYADDR) GetProcAddress(gWinsockLib,"gethostbyaddr");
          gWSinet_addr = (LPFN_INET_ADDR) GetProcAddress(gWinsockLib,"inet_addr");
#endif

          if(startup == NULL
             || gWSACleanup == NULL
             || gWShtons == NULL
             || gWSsocket == NULL
             || gWSconnect == NULL
             || gWSsend == NULL
             || gWSrecv == NULL
             || gWSshutdown == NULL
             || gWSclosesocket == NULL
#ifdef WITH_IPV6
             || gWSgetaddrinfo == NULL
             || gWSfreeaddrinfo == NULL
#else
             || gWSgethostbyname == NULL
             || gWSgethostbyaddr == NULL
             || gWSinet_addr == NULL
#endif
          )
          {
              gWSACleanup = NULL;
              Cleanup();
              ThrowXMLwithMemMgr(NetAccessorException, XMLExcepts::NetAcc_InitFailed, manager);
          }
      }
    }

    wVersionRequested = MAKEWORD( 2, 2 );

    int err = (*startup)(wVersionRequested, &wsaData);
    if (err != 0)
    {
        // Call WSAGetLastError() to get the last error.
        ThrowXMLwithMemMgr(NetAccessorException, XMLExcepts::NetAcc_InitFailed, manager);
    }
    fInitialized = true;
}

void BinHTTPURLInputStream::Cleanup()
{
    XMLMutexLock lock(XMLPlatformUtils::fgAtomicMutex);

    if(fInitialized)
    {
        if(gWSACleanup) (*gWSACleanup)();
        gWSACleanup = NULL;
        FreeLibrary(gWinsockLib);
        gWinsockLib = NULL;
        gWShtons = NULL;
        gWSsocket = NULL;
        gWSconnect = NULL;
        gWSsend = NULL;
        gWSrecv = NULL;
        gWSshutdown = NULL;
        gWSclosesocket = NULL;
#ifdef WITH_IPV6
        gWSgetaddrinfo = NULL;
        gWSfreeaddrinfo = NULL;
#else
        gWSgethostbyname = NULL;
        gWSgethostbyaddr = NULL;
        gWSinet_addr = NULL;
#endif

        fInitialized = false;
    }
}

BinHTTPURLInputStream::BinHTTPURLInputStream(const XMLURL& urlSource, const XMLNetHTTPInfo* httpInfo /*=0*/)
      : BinHTTPInputStreamCommon(urlSource.getMemoryManager())
      , fSocketHandle(0)
{
    MemoryManager *memoryManager = urlSource.getMemoryManager();

    // Check if we need to load the winsock library. While locking the
    // mutex every time may be somewhat slow, we don't care in this
    // particular case since the next operation will most likely be
    // the network access which is a lot slower.
    //
    {
        XMLMutexLock lock(XMLPlatformUtils::fgAtomicMutex);

        if (!fInitialized)
          Initialize(memoryManager);
    }

    //
    // Pull all of the parts of the URL out of th urlSource object, and transcode them
    //   and transcode them back to ASCII.
    //
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

#ifdef WITH_IPV6
        struct addrinfo hints, *res, *ai;

        CharBuffer portBuffer(10, memoryManager);
        portBuffer.appendDecimalNumber(url.getPortNum());

        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = PF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        int n = wrap_getaddrinfo(hostNameAsCharStar,portBuffer.getRawBuffer(),&hints, &res);
        if(n != 0)
        {
            hints.ai_flags = AI_NUMERICHOST;
            n = wrap_getaddrinfo(hostNameAsCharStar,(const char*)tempbuf,&hints, &res);
            if(n != 0)
                ThrowXMLwithMemMgr1(NetAccessorException, XMLExcepts::NetAcc_TargetResolution, hostName, memoryManager);
        }
        janSock.reset();
        for (ai = res; ai != NULL; ai = ai->ai_next) {
            // Open a socket with the correct address family for this address.
            fSocketHandle = wrap_socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
            if (fSocketHandle == INVALID_SOCKET)
                continue;
            janSock.reset(&fSocketHandle);
            if (wrap_connect(fSocketHandle, ai->ai_addr, (int)ai->ai_addrlen) == SOCKET_ERROR)
            {
                wrap_freeaddrinfo(res);
                // Call WSAGetLastError() to get the error number.
                ThrowXMLwithMemMgr1(NetAccessorException,
                         XMLExcepts::NetAcc_ConnSocket, url.getURLText(), memoryManager);
            }
            break;
        }
        wrap_freeaddrinfo(res);
        if (fSocketHandle == INVALID_SOCKET)
        {
            // Call WSAGetLastError() to get the error number.
            ThrowXMLwithMemMgr1(NetAccessorException,
                     XMLExcepts::NetAcc_CreateSocket, url.getURLText(), memoryManager);
        }
#else
        struct hostent*     hostEntPtr = 0;
        struct sockaddr_in  sa;


        if ((hostEntPtr = wrap_gethostbyname(hostNameAsCharStar)) == NULL)
        {
            unsigned long  numAddress = wrap_inet_addr(hostNameAsCharStar);
            if (numAddress == INADDR_NONE)
            {
                // Call WSAGetLastError() to get the error number.
                ThrowXMLwithMemMgr1(NetAccessorException,
                    XMLExcepts::NetAcc_TargetResolution, hostName, memoryManager);
            }
            if ((hostEntPtr =
                    wrap_gethostbyaddr((const char *) &numAddress,
                        sizeof(unsigned long), AF_INET)) == NULL)
            {
                // Call WSAGetLastError() to get the error number.
                ThrowXMLwithMemMgr1(NetAccessorException,
                    XMLExcepts::NetAcc_TargetResolution, hostName, memoryManager);
            }
        }

        memcpy((void *) &sa.sin_addr,
            (const void *) hostEntPtr->h_addr, hostEntPtr->h_length);
        sa.sin_family = hostEntPtr->h_addrtype;
        sa.sin_port = wrap_htons((unsigned short)url.getPortNum());

        janSock.reset();
        fSocketHandle = wrap_socket(hostEntPtr->h_addrtype, SOCK_STREAM, 0);
        if (fSocketHandle == INVALID_SOCKET)
        {
            // Call WSAGetLastError() to get the error number.
            ThrowXMLwithMemMgr1(NetAccessorException,
                XMLExcepts::NetAcc_CreateSocket, url.getURLText(), memoryManager);
        }
        janSock.reset(&fSocketHandle);

        if (wrap_connect(fSocketHandle, (struct sockaddr *) &sa, sizeof(sa)) == SOCKET_ERROR)
        {
            // Call WSAGetLastError() to get the error number.
            ThrowXMLwithMemMgr1(NetAccessorException,
                XMLExcepts::NetAcc_ConnSocket, url.getURLText(), memoryManager);
        }

#endif

        int status = sendRequest(url, httpInfo);

        if(status == 200) {
            // HTTP 200 OK response means we're done.
            // We're done
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

BinHTTPURLInputStream::~BinHTTPURLInputStream()
{
    wrap_shutdown(fSocketHandle, SD_BOTH);
    wrap_closesocket(fSocketHandle);
}

bool BinHTTPURLInputStream::send(const char *buf, XMLSize_t len)
{
    XMLSize_t done = 0;
    int ret;

    while(done < len) {
        ret = wrap_send(fSocketHandle, buf + done, (int)(len - done), 0);
        if(ret == SOCKET_ERROR) return false;
        done += ret;
    }

    return true;
}

int BinHTTPURLInputStream::receive(char *buf, XMLSize_t len)
{
    int iLen = wrap_recv(fSocketHandle, buf, (int)len, 0);
    if (iLen == SOCKET_ERROR) return -1;
    return iLen;
}

XERCES_CPP_NAMESPACE_END
