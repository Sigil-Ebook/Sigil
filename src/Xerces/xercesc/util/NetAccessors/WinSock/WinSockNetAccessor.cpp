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
 * $Id: WinSockNetAccessor.cpp 635984 2008-03-11 15:54:37Z borisk $
 */


#define _WINSOCKAPI_

#include <windows.h>

#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/util/XMLExceptMsgs.hpp>
#include <xercesc/util/NetAccessors/WinSock/BinHTTPURLInputStream.hpp>
#include <xercesc/util/NetAccessors/WinSock/WinSockNetAccessor.hpp>

XERCES_CPP_NAMESPACE_BEGIN

const XMLCh WinSockNetAccessor::fgMyName[] =
{
    chLatin_W, chLatin_i, chLatin_n, chLatin_S, chLatin_o, chLatin_c,
    chLatin_k, chLatin_N, chLatin_e, chLatin_t, chLatin_A, chLatin_c,
    chLatin_c, chLatin_e, chLatin_s, chLatin_s, chLatin_o, chLatin_r,
    chNull
};

WinSockNetAccessor::WinSockNetAccessor()
{
}


WinSockNetAccessor::~WinSockNetAccessor()
{
    // Cleanup code for the WinSock library here.
    BinHTTPURLInputStream::Cleanup();
}


BinInputStream* WinSockNetAccessor::makeNew(const XMLURL&  urlSource, const XMLNetHTTPInfo* httpInfo /*=0*/)
{
    XMLURL::Protocols  protocol = urlSource.getProtocol();
    switch(protocol)
    {
        case XMLURL::HTTP:
        {
            BinHTTPURLInputStream* retStrm =
                new (urlSource.getMemoryManager()) BinHTTPURLInputStream(urlSource, httpInfo);
            return retStrm;
            break;
        }

        //
        // These are the only protocols we support now. So throw and
        // unsupported protocol exception for the others.
        //
        default :
            ThrowXMLwithMemMgr(MalformedURLException, XMLExcepts::URL_UnsupportedProto, urlSource.getMemoryManager());
            break;
    }
    return 0;
}

XERCES_CPP_NAMESPACE_END

