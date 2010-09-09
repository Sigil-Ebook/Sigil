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
 * $Id: URLAccessCFBinInputStream.hpp 670359 2008-06-22 13:43:45Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_URLACCESSCFBININPUTSTREAM_HPP)
#define XERCESC_INCLUDE_GUARD_URLACCESSCFBININPUTSTREAM_HPP


#include <xercesc/util/XMLURL.hpp>
#include <xercesc/util/XMLExceptMsgs.hpp>
#include <xercesc/util/BinInputStream.hpp>

#if defined(__APPLE__)
    //	Framework includes from ProjectBuilder
	#include <CoreServices/CoreServices.h>
#else
    //	Classic includes otherwise
	#include <CFURL.h>
	#include <CFURLAccess.h>
#endif

XERCES_CPP_NAMESPACE_BEGIN

//
// This class implements the BinInputStream interface specified by the XML
// parser.
//

class XMLUTIL_EXPORT URLAccessCFBinInputStream : public BinInputStream
{
public :
    URLAccessCFBinInputStream(const XMLURL&  urlSource);
    ~URLAccessCFBinInputStream();

    virtual XMLFilePos curPos() const;
    virtual XMLSize_t readBytes
    (
                XMLByte* const  toFill
        , const XMLSize_t       maxToRead
    );

    virtual const XMLCh* getContentType() const;

private :
    CFDataRef			mDataRef;
    CFIndex				mBytesProcessed;
};


inline XMLFilePos
URLAccessCFBinInputStream::curPos() const
{
    return mBytesProcessed;
}

XERCES_CPP_NAMESPACE_END

#endif // URLACCESSCFBININPUTSTREAM_HPP
