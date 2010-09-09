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
 * $Id: URLAccessCFBinInputStream.cpp 936316 2010-04-21 14:19:58Z borisk $
 */

#include <cstdlib>
#include <cstring>

#include <xercesc/util/XMLNetAccessor.hpp>
#include <xercesc/util/NetAccessors/MacOSURLAccessCF/URLAccessCFBinInputStream.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLExceptMsgs.hpp>
#include <xercesc/util/Janitor.hpp>

XERCES_CPP_NAMESPACE_BEGIN

URLAccessCFBinInputStream::URLAccessCFBinInputStream(const XMLURL& urlSource)
      : mBytesProcessed(0),
        mDataRef(NULL)
{
    //	Figure out what we're dealing with
    const XMLCh* urlText = urlSource.getURLText();
    unsigned int urlLength = XMLString::stringLen(urlText);

    //	Create a CFString from the path
    CFStringRef stringRef = NULL;
    if (urlText)
    {
        stringRef = CFStringCreateWithCharacters(
            kCFAllocatorDefault,
            urlText,
            urlLength
            );
    }

    //	Create a URLRef from the CFString
    CFURLRef urlRef = NULL;
    if (stringRef)
    {
        urlRef = CFURLCreateWithString(
            kCFAllocatorDefault,
            stringRef,
            NULL				// CFURLRef baseURL
            );
    }

	//	Fetch the data
    mDataRef = NULL;
    SInt32 errorCode = 0;
    Boolean success = false;
    if (stringRef)
    {
        success = CFURLCreateDataAndPropertiesFromResource(
            kCFAllocatorDefault,
            urlRef,
            &mDataRef,
            NULL,				// CFDictionaryRef *properties,
            NULL,				// CFArrayRef desiredProperties,
            &errorCode
            );
    }

    //	Cleanup temporary stuff
    if (stringRef)
        CFRelease(stringRef);
    if (urlRef)
        CFRelease(urlRef);

    //	Check for an error in fetching the data
    if (!success || errorCode)
    {
        //	Dispose any potential dataRef
        if (mDataRef)
        {
            CFRelease(mDataRef);
            mDataRef = NULL;
        }

        //	Do a best attempt at mapping some errors
        switch (errorCode)
        {
            case kCFURLUnknownSchemeError:
                ThrowXML(MalformedURLException, XMLExcepts::URL_UnsupportedProto);
                break;

            case kCFURLRemoteHostUnavailableError:
              {
                if (urlSource.getHost())
                  ThrowXML1(NetAccessorException, XMLExcepts::NetAcc_TargetResolution, urlSource.getHost());
                else
                  ThrowXML1(NetAccessorException, XMLExcepts::File_CouldNotOpenFile, urlText);
                break;
              }

            case kCFURLUnknownError:
                ThrowXML1(NetAccessorException, XMLExcepts::NetAcc_ReadSocket, urlText);
                break;

            case kCFURLResourceNotFoundError:
            case kCFURLResourceAccessViolationError:
            case kCFURLTimeoutError:
                ThrowXML1(NetAccessorException, XMLExcepts::File_CouldNotOpenFile, urlText);
                break;

            case kCFURLImproperArgumentsError:
            case kCFURLUnknownPropertyKeyError:
            case kCFURLPropertyKeyUnavailableError:
            default:
                ThrowXML1(NetAccessorException, XMLExcepts::NetAcc_InternalError, urlText);
                break;
        }
    }
}


URLAccessCFBinInputStream::~URLAccessCFBinInputStream()
{
    //	Release any dataRef
    if (mDataRef)
        CFRelease(mDataRef);
}


//
//	We've already read the data into a dataRef.
//	Just spoon it out to the caller as they ask for it.
//
XMLSize_t
URLAccessCFBinInputStream::readBytes(XMLByte* const    toFill
                                    , const XMLSize_t  maxToRead)
{
    //	If we don't have a dataRef, we can't return any data
    if (!mDataRef)
        return 0;

    //	Get the length of the data we've fetched
    CFIndex dataLength = CFDataGetLength(mDataRef);

    //	Calculate how much to return based on how much
    //	we've already returned, and how much the user wants
    CFIndex n = dataLength - mBytesProcessed;			// Amount remaining
    CFIndex desired = maxToRead & 0x7fffffff;			// CFIndex is signed
    if (n > desired)									// Amount desired
        n = desired;

    //	Read the appropriate bytes into the user buffer
    CFRange range = CFRangeMake(mBytesProcessed, n);
    CFDataGetBytes(mDataRef, range, reinterpret_cast<UInt8*>(toFill));

    //	Update bytes processed
    mBytesProcessed += n;

    //	Return the number of bytes delivered
    return n;
}

const XMLCh* URLAccessCFBinInputStream::getContentType() const
{
    // TODO
    //
    return 0;
}

XERCES_CPP_NAMESPACE_END
