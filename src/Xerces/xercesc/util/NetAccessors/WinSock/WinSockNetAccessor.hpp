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
 * $Id: WinSockNetAccessor.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_WINSOCKNETACCESSOR_HPP)
#define XERCESC_INCLUDE_GUARD_WINSOCKNETACCESSOR_HPP


#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/util/XMLURL.hpp>
#include <xercesc/util/BinInputStream.hpp>
#include <xercesc/util/XMLNetAccessor.hpp>


XERCES_CPP_NAMESPACE_BEGIN

//
// This class is the wrapper for the WinSock library which provides
// support for sockets. Its being used here to add the ability to
// use HTTP URL's as the system id's in the XML decl clauses.
//

class XMLUTIL_EXPORT WinSockNetAccessor : public XMLNetAccessor
{
public :
    WinSockNetAccessor();
    ~WinSockNetAccessor();

    virtual BinInputStream* makeNew(const XMLURL&  urlSource, const XMLNetHTTPInfo* httpInfo=0);
    virtual const XMLCh* getId() const;

private :
    static const XMLCh fgMyName[];

    WinSockNetAccessor(const WinSockNetAccessor&);
    WinSockNetAccessor& operator=(const WinSockNetAccessor&);

}; // WinSockNetAccessor

inline const XMLCh* WinSockNetAccessor::getId() const
{
    return fgMyName;
}

XERCES_CPP_NAMESPACE_END


#endif // WINSOCKNETACCESSOR_HPP
