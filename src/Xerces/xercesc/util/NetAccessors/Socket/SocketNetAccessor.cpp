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
 * $Id: SocketNetAccessor.cpp 471747 2006-11-06 14:31:56Z amassari $
 */

#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLExceptMsgs.hpp>
#include <xercesc/util/NetAccessors/Socket/UnixHTTPURLInputStream.hpp>
#include <xercesc/util/NetAccessors/Socket/SocketNetAccessor.hpp>

XERCES_CPP_NAMESPACE_BEGIN

const XMLCh SocketNetAccessor::fgMyName[] =
{
    chLatin_S, chLatin_o, chLatin_c, chLatin_k, chLatin_e, chLatin_t,
    chLatin_N, chLatin_e, chLatin_t, chLatin_A, chLatin_c, chLatin_c,
    chLatin_e, chLatin_s, chLatin_s, chLatin_o, chLatin_r, chNull
};


SocketNetAccessor::SocketNetAccessor()
{
    // Do any one time initialization here.
    // Nothing to do, in this case.
}


SocketNetAccessor::~SocketNetAccessor()
{
    // Again, nothing to do here.
}


BinInputStream* SocketNetAccessor::makeNew(const XMLURL&  urlSource, const XMLNetHTTPInfo* httpInfo/*=0*/)
{
    XMLURL::Protocols  protocol = urlSource.getProtocol();
    switch(protocol)
    {
        case XMLURL::HTTP:
        {
            UnixHTTPURLInputStream* retStrm =
                new (urlSource.getMemoryManager()) UnixHTTPURLInputStream(urlSource, httpInfo);
            return retStrm;            
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

