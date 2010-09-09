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
 * $Id: XMLNetAccessor.hpp 673960 2008-07-04 08:50:12Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XMLNETACCESSOR_HPP)
#define XERCESC_INCLUDE_GUARD_XMLNETACCESSOR_HPP

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/util/XMLURL.hpp>
#include <xercesc/util/XMLException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class BinInputStream;

//  This class holds advanced informations about the HTTP connection
class XMLUTIL_EXPORT XMLNetHTTPInfo
{
public:
    XMLNetHTTPInfo();

    typedef enum {
        GET,
        PUT,
        POST
    } HTTPMethod;

    // -----------------------------------------------------------------------
    //  Data members
    //
    //  fHTTPMethod
    //      The type of the HTTP request
    //
    //  fHeaders
    //      The extra headers that will be sent as part of the request; the format is
    //      Header1: Value\r\nHeader2: Value\r\n
    //
    //  fHeadersLen
    //      The length of the string pointed by fHeaders, in bytes
    //
    //  fPayload
    //      The extra data that will be sent after the headers; in the case of a PUT
    //      operation, this is the content of the resource being posted. It can be binary data
    //
    //  fPayloadLen
    //      The length of the binary buffer pointed by fPayload, in bytes
    //
    HTTPMethod      fHTTPMethod;
    const char*     fHeaders;
    XMLSize_t       fHeadersLen;
    const char*     fPayload;
    XMLSize_t       fPayloadLen;
};

inline XMLNetHTTPInfo::XMLNetHTTPInfo()
:fHTTPMethod(XMLNetHTTPInfo::GET),
 fHeaders(0),
 fHeadersLen(0),
 fPayload(0),
 fPayloadLen(0)
{
}


//
//  This class is an abstract interface via which the URL class accesses
//  net access services. When any source URL is not in effect a local file
//  path, then the URL class is used to look at it. Then the URL class can
//  be asked to make a binary input stream via which the referenced resource
//  can be read in.
//
//  The URL class will use an object derived from this class to create a
//  binary stream for the URL to return. The object it uses is provided by
//  the platform utils, and is actually provided by the per-platform init
//  code so each platform can decide what actual implementation it wants to
//  use.
//
class XMLUTIL_EXPORT XMLNetAccessor : public XMemory
{
public :
    // -----------------------------------------------------------------------
    //  Virtual destructor
    // -----------------------------------------------------------------------
    virtual ~XMLNetAccessor()
    {
    }


    // -----------------------------------------------------------------------
    //  The virtual net accessor interface
    // -----------------------------------------------------------------------
    virtual const XMLCh* getId() const = 0;

    virtual BinInputStream* makeNew
    (
        const   XMLURL&                 urlSrc,
        const   XMLNetHTTPInfo*         httpInfo=0
    ) = 0;


protected :
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    XMLNetAccessor()
    {
    }


private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XMLNetAccessor(const XMLNetAccessor&);
    XMLNetAccessor& operator=(const XMLNetAccessor&);
};

MakeXMLException(NetAccessorException, XMLUTIL_EXPORT)

XERCES_CPP_NAMESPACE_END

#endif
