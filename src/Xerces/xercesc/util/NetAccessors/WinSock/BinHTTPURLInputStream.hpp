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
 * $Id: BinHTTPURLInputStream.hpp 670359 2008-06-22 13:43:45Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_BINHTTPURLINPUTSTREAM_HPP)
#define XERCESC_INCLUDE_GUARD_BINHTTPURLINPUTSTREAM_HPP


#include <xercesc/util/NetAccessors/BinHTTPInputStreamCommon.hpp>

#include <winsock2.h>

XERCES_CPP_NAMESPACE_BEGIN

//
// This class implements the BinInputStream interface specified by the XML
// parser.
//
class XMLUTIL_EXPORT BinHTTPURLInputStream : public BinHTTPInputStreamCommon
{
public :
    BinHTTPURLInputStream(const XMLURL&  urlSource, const XMLNetHTTPInfo* httpInfo=0);
    ~BinHTTPURLInputStream();

    virtual bool send(const char *buf, XMLSize_t len);
    virtual int receive(char *buf, XMLSize_t len);

	static void Cleanup();

private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    BinHTTPURLInputStream(const BinHTTPURLInputStream&);
    BinHTTPURLInputStream& operator=(const BinHTTPURLInputStream&);

    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fSocketHandle
    //      The socket representing the connection to the remote file.
    //      We deliberately did not define the type to be SOCKET, so as to
    //      avoid bringing in any Windows header into this file.
    // -----------------------------------------------------------------------
    SOCKET              fSocketHandle;

    static bool         fInitialized;

    static void Initialize(MemoryManager* const manager  = XMLPlatformUtils::fgMemoryManager);
};

XERCES_CPP_NAMESPACE_END

#endif // BINHTTPURLINPUTSTREAM_HPP
