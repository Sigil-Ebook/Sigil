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
 * $Id: CurlNetAccessor.cpp 471747 2006-11-06 14:31:56Z amassari $
 */

#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLExceptMsgs.hpp>
#include <xercesc/util/NetAccessors/Curl/CurlURLInputStream.hpp>
#include <xercesc/util/NetAccessors/Curl/CurlNetAccessor.hpp>

XERCES_CPP_NAMESPACE_BEGIN

const XMLCh CurlNetAccessor::fgMyName[] =
{
    chLatin_C, chLatin_u, chLatin_r, chLatin_l, chLatin_N, chLatin_e,
    chLatin_t, chLatin_A, chLatin_c, chLatin_c, chLatin_e, chLatin_s,
    chLatin_s, chLatin_o, chLatin_r, chNull
};


CurlNetAccessor::CurlNetAccessor()
{
	initCurl();
}


CurlNetAccessor::~CurlNetAccessor()
{
	cleanupCurl();
}


//
// Global once-only init and cleanup of curl
//
// The init count used here is not thread protected; we assume
// that creation of the CurlNetAccessor will be serialized by
// the application. If the application is also using curl, then
// care must be taken that curl is initialized only once, by some
// other means, or by overloading these methods.
//
int CurlNetAccessor::fgCurlInitCount = 0;

void
CurlNetAccessor::initCurl()
{
	if (fgCurlInitCount++ == 0)
		curl_global_init(	0
						  | CURL_GLOBAL_ALL			// Initialize all curl modules
					//	  | CURL_GLOBAL_WIN32		// Initialize Windows sockets first
					//	  | CURL_GLOBAL_SSL			// Initialize SSL first
						  );
}


void
CurlNetAccessor::cleanupCurl()
{
	if (fgCurlInitCount > 0 && --fgCurlInitCount == 0)
		curl_global_cleanup();
}


BinInputStream*
CurlNetAccessor::makeNew(const XMLURL&  urlSource, const XMLNetHTTPInfo* httpInfo/*=0*/)
{
	// Just create a CurlURLInputStream
	// We defer any checking of the url type for curl in CurlURLInputStream
	CurlURLInputStream* retStrm =
		new (urlSource.getMemoryManager()) CurlURLInputStream(urlSource, httpInfo);
	return retStrm;            
}

XERCES_CPP_NAMESPACE_END

