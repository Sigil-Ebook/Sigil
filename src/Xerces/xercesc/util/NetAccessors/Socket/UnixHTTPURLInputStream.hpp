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
 * $Id: UnixHTTPURLInputStream.hpp 670359 2008-06-22 13:43:45Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_UNIXHTTPURLINPUTSTREAM_HPP)
#define XERCESC_INCLUDE_GUARD_UNIXHTTPURLINPUTSTREAM_HPP

#include <xercesc/util/NetAccessors/BinHTTPInputStreamCommon.hpp>

XERCES_CPP_NAMESPACE_BEGIN

//
// This class implements the BinInputStream interface specified by the XML
// parser.
//
class XMLUTIL_EXPORT UnixHTTPURLInputStream : public BinHTTPInputStreamCommon
{
public :
    UnixHTTPURLInputStream(const XMLURL&  urlSource, const XMLNetHTTPInfo* httpInfo=0);
    ~UnixHTTPURLInputStream();

    virtual bool send(const char *buf, XMLSize_t len);
    virtual int receive(char *buf, XMLSize_t len);

private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    UnixHTTPURLInputStream(const UnixHTTPURLInputStream&);
    UnixHTTPURLInputStream& operator=(const UnixHTTPURLInputStream&);

    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fSocket
    //      The socket representing the connection to the remote file.
    // -----------------------------------------------------------------------

    int                 fSocket;
};

XERCES_CPP_NAMESPACE_END

#endif // UNIXHTTPURLINPUTSTREAM_HPP
