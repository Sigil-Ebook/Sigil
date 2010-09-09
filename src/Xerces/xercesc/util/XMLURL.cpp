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
 * $Id: XMLURL.cpp 901107 2010-01-20 08:45:02Z borisk $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/BinFileInputStream.hpp>
#include <xercesc/util/Janitor.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/RuntimeException.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/util/XMLURL.hpp>
#include <xercesc/util/XMLNetAccessor.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/util/XMLUri.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/util/XMLChar.hpp>

XERCES_CPP_NAMESPACE_BEGIN



// ---------------------------------------------------------------------------
//  Local types
//
//  TypeEntry
//      This structure defines a single entry in the list of URL types. Each
//      entry indicates the prefix for that type of URL, and the SourceTypes
//      value it maps to.
// ---------------------------------------------------------------------------
struct ProtoEntry
{
    XMLURL::Protocols   protocol;
    const XMLCh*        prefix;
    unsigned int        defPort;
};


// ---------------------------------------------------------------------------
//  Local data
//
//  gXXXString
//      These are the strings for our prefix types. They all have to be
//      Unicode strings all the time, so we can't just do regular strings.
//
//  gProtoList
//      The list of URL types that we support and some info related to each
//      one.
//
//  gMaxProtoLen
//      The length of the longest protocol string
//
//      NOTE:!!! Be sure to keep this up to date if new protocols are added!
// ---------------------------------------------------------------------------
static const XMLCh  gFileString[] =
{
        chLatin_f, chLatin_i, chLatin_l, chLatin_e, chNull
};

static const XMLCh gFTPString[]  =
{
        chLatin_f, chLatin_t, chLatin_p, chNull
};

static const XMLCh gHTTPString[] =
{
        chLatin_h, chLatin_t, chLatin_t, chLatin_p, chNull
};

static const XMLCh gHTTPSString[] =
{
        chLatin_h, chLatin_t, chLatin_t, chLatin_p, chLatin_s, chNull
};

static ProtoEntry gProtoList[XMLURL::Protocols_Count] =
{
        { XMLURL::File     , gFileString    , 0  }
    ,   { XMLURL::HTTP     , gHTTPString    , 80 }
    ,   { XMLURL::FTP      , gFTPString     , 21 }
    ,   { XMLURL::HTTPS    , gHTTPSString   , 443 }
};

// !!! Keep these up to date with list above!
static const unsigned int gMaxProtoLen = 5;

static const XMLCh gListOne[]    = { chColon, chForwardSlash, chNull };
static const XMLCh gListTwo[]    = { chAt, chNull };
static const XMLCh gListThree[]  = { chColon, chNull };
static const XMLCh gListFour[]   = { chForwardSlash, chNull };
static const XMLCh gListFive[]   = { chPound, chQuestion, chNull };
static const XMLCh gListSix[]    = { chPound, chNull };

// ---------------------------------------------------------------------------
//  Local methods
// ---------------------------------------------------------------------------
static bool isHexDigit(const XMLCh toCheck)
{
    if (((toCheck >= chDigit_0) && (toCheck <= chDigit_9))
    ||  ((toCheck >= chLatin_A) && (toCheck <= chLatin_Z))
    ||  ((toCheck >= chLatin_a) && (toCheck <= chLatin_z)))
    {
        return true;
    }
    return false;
}

static unsigned int xlatHexDigit(const XMLCh toXlat)
{
    if ((toXlat >= chDigit_0) && (toXlat <= chDigit_9))
        return (unsigned int)(toXlat - chDigit_0);

    if ((toXlat >= chLatin_A) && (toXlat <= chLatin_Z))
        return (unsigned int)(toXlat - chLatin_A) + 10;

    return (unsigned int)(toXlat - chLatin_a) + 10;
}



// ---------------------------------------------------------------------------
//  XMLURL: Public, static methods
// ---------------------------------------------------------------------------
XMLURL::Protocols XMLURL::lookupByName(const XMLCh* const protoName)
{
    for (unsigned int index = 0; index < XMLURL::Protocols_Count; index++)
    {
        if (!XMLString::compareIStringASCII(protoName, gProtoList[index].prefix))
            return gProtoList[index].protocol;
    }
    return XMLURL::Unknown;
}



// ---------------------------------------------------------------------------
//  XMLURL: Constructors and Destructor
// ---------------------------------------------------------------------------
XMLURL::XMLURL(MemoryManager* const manager) :

    fMemoryManager(manager)
    , fFragment(0)
    , fHost(0)
    , fPassword(0)
    , fPath(0)
    , fPortNum(0)
    , fProtocol(XMLURL::Unknown)
    , fQuery(0)
    , fUser(0)
    , fURLText(0)
    , fHasInvalidChar(false)
{
}

typedef JanitorMemFunCall<XMLURL>   CleanupType;

XMLURL::XMLURL(const XMLCh* const    baseURL
             , const XMLCh* const    relativeURL
             , MemoryManager* const manager) :

    fMemoryManager(manager)
    , fFragment(0)
    , fHost(0)
    , fPassword(0)
    , fPath(0)
    , fPortNum(0)
    , fProtocol(XMLURL::Unknown)
    , fQuery(0)
    , fUser(0)
    , fURLText(0)
    , fHasInvalidChar(false)
{
    CleanupType cleanup(this, &XMLURL::cleanUp);

	try
	{
        setURL(baseURL, relativeURL);
	}
    catch(const OutOfMemoryException&)
    {
        cleanup.release();

        throw;
    }

    cleanup.release();
}

XMLURL::XMLURL(const XMLCh* const  baseURL
             , const char* const   relativeURL
             , MemoryManager* const manager) :

    fMemoryManager(manager)
    , fFragment(0)
    , fHost(0)
    , fPassword(0)
    , fPath(0)
    , fPortNum(0)
    , fProtocol(XMLURL::Unknown)
    , fQuery(0)
    , fUser(0)
    , fURLText(0)
    , fHasInvalidChar(false)
{
    CleanupType cleanup(this, &XMLURL::cleanUp);

    XMLCh* tmpRel = XMLString::transcode(relativeURL, fMemoryManager);
    ArrayJanitor<XMLCh> janRel(tmpRel, fMemoryManager);
	try
	{
		setURL(baseURL, tmpRel);
	}
    catch(const OutOfMemoryException&)
    {
        cleanup.release();

        throw;
    }

    cleanup.release();
}

XMLURL::XMLURL(const XMLURL&         baseURL
             , const XMLCh* const    relativeURL) :

    fMemoryManager(baseURL.fMemoryManager)
    , fFragment(0)
    , fHost(0)
    , fPassword(0)
    , fPath(0)
    , fPortNum(0)
    , fProtocol(XMLURL::Unknown)
    , fQuery(0)
    , fUser(0)
    , fURLText(0)
    , fHasInvalidChar(false)
{
    CleanupType cleanup(this, &XMLURL::cleanUp);

	try
	{
		setURL(baseURL, relativeURL);
	}
    catch(const OutOfMemoryException&)
    {
        cleanup.release();

        throw;
    }

    cleanup.release();
}

XMLURL::XMLURL(const  XMLURL&        baseURL
             , const char* const     relativeURL) :

    fMemoryManager(baseURL.fMemoryManager)
    , fFragment(0)
    , fHost(0)
    , fPassword(0)
    , fPath(0)
    , fPortNum(0)
    , fProtocol(XMLURL::Unknown)
    , fQuery(0)
    , fUser(0)
    , fURLText(0)
    , fHasInvalidChar(false)
{
    CleanupType cleanup(this, &XMLURL::cleanUp);

    XMLCh* tmpRel = XMLString::transcode(relativeURL, fMemoryManager);
    ArrayJanitor<XMLCh> janRel(tmpRel, fMemoryManager);
	try
	{
		setURL(baseURL, tmpRel);
	}
    catch(const OutOfMemoryException&)
    {
        cleanup.release();

        throw;
    }

    cleanup.release();
}

XMLURL::XMLURL(const XMLCh* const urlText,
               MemoryManager* const manager) :

    fMemoryManager(manager)
    , fFragment(0)
    , fHost(0)
    , fPassword(0)
    , fPath(0)
    , fPortNum(0)
    , fProtocol(XMLURL::Unknown)
    , fQuery(0)
    , fUser(0)
    , fURLText(0)
    , fHasInvalidChar(false)
{
    CleanupType cleanup(this, &XMLURL::cleanUp);

	try
	{
	    setURL(urlText);
	}
    catch(const OutOfMemoryException&)
    {
        cleanup.release();

        throw;
    }

    cleanup.release();
}

XMLURL::XMLURL(const char* const urlText,
               MemoryManager* const manager) :

    fMemoryManager(manager)
    , fFragment(0)
    , fHost(0)
    , fPassword(0)
    , fPath(0)
    , fPortNum(0)
    , fProtocol(XMLURL::Unknown)
    , fQuery(0)
    , fUser(0)
    , fURLText(0)
    , fHasInvalidChar(false)
{
    CleanupType cleanup(this, &XMLURL::cleanUp);

    XMLCh* tmpText = XMLString::transcode(urlText, fMemoryManager);
    ArrayJanitor<XMLCh> janRel(tmpText, fMemoryManager);
	try
	{
	    setURL(tmpText);
	}
    catch(const OutOfMemoryException&)
    {
        cleanup.release();

        throw;
    }

    cleanup.release();
}

XMLURL::XMLURL(const XMLURL& toCopy) :
    XMemory(toCopy)
    , fMemoryManager(toCopy.fMemoryManager)
    , fFragment(0)
    , fHost(0)
    , fPassword(0)
    , fPath(0)
    , fPortNum(toCopy.fPortNum)
    , fProtocol(toCopy.fProtocol)
    , fQuery(0)
    , fUser(0)
    , fURLText(0)
    , fHasInvalidChar(toCopy.fHasInvalidChar)
{
    CleanupType cleanup(this, &XMLURL::cleanUp);

    try
    {
        fFragment = XMLString::replicate(toCopy.fFragment, fMemoryManager);
        fHost = XMLString::replicate(toCopy.fHost, fMemoryManager);
        fPassword = XMLString::replicate(toCopy.fPassword, fMemoryManager);
        fPath = XMLString::replicate(toCopy.fPath, fMemoryManager);
        fQuery = XMLString::replicate(toCopy.fQuery, fMemoryManager);
        fUser = XMLString::replicate(toCopy.fUser, fMemoryManager);
        fURLText = XMLString::replicate(toCopy.fURLText, fMemoryManager);
    }
    catch(const OutOfMemoryException&)
    {
        cleanup.release();

        throw;
    }

    cleanup.release();
}

XMLURL::~XMLURL()
{
    cleanUp();
}


// ---------------------------------------------------------------------------
//  XMLURL: Public operators
// ---------------------------------------------------------------------------
XMLURL& XMLURL::operator=(const XMLURL& toAssign)
{
    if (this == &toAssign)
        return *this;

    // Clean up our stuff
    cleanUp();

    // And copy his stuff
    fMemoryManager = toAssign.fMemoryManager;
    fFragment = XMLString::replicate(toAssign.fFragment, fMemoryManager);
    fHost = XMLString::replicate(toAssign.fHost, fMemoryManager);
    fPassword = XMLString::replicate(toAssign.fPassword, fMemoryManager);
    fPath = XMLString::replicate(toAssign.fPath, fMemoryManager);
    fPortNum = toAssign.fPortNum;
    fProtocol = toAssign.fProtocol;
    fQuery = XMLString::replicate(toAssign.fQuery, fMemoryManager);
    fUser = XMLString::replicate(toAssign.fUser, fMemoryManager);
    fURLText = XMLString::replicate(toAssign.fURLText, fMemoryManager);
    fHasInvalidChar = toAssign.fHasInvalidChar;

    return *this;
}

bool XMLURL::operator==(const XMLURL& toCompare) const
{
    //
    //  Compare the two complete URLs (which have been processed the same
    //  way so they should now be the same even if they came in via different
    //  relative parts.
    //
    if (!XMLString::equals(getURLText(), toCompare.getURLText()))
        return false;

    return true;
}



// ---------------------------------------------------------------------------
//  XMLURL: Getter methods
// ---------------------------------------------------------------------------
unsigned int XMLURL::getPortNum() const
{
    //
    //  If it was not provided explicitly, then lets return the default one
    //  for the protocol.
    //
    if (!fPortNum)
    {
        if (fProtocol == Unknown)
            return 0;
        return gProtoList[fProtocol].defPort;
    }
    return fPortNum;
}


const XMLCh* XMLURL::getProtocolName() const
{
    // Check to see if its ever been set
    if (fProtocol == Unknown)
        ThrowXMLwithMemMgr(MalformedURLException, XMLExcepts::URL_NoProtocolPresent, fMemoryManager);

    return gProtoList[fProtocol].prefix;
}


// ---------------------------------------------------------------------------
//  XMLURL: Setter methods
// ---------------------------------------------------------------------------
void XMLURL::setURL(const XMLCh* const urlText)
{
    //
    //  Try to parse the URL.
    //
    cleanUp();
    parse(urlText);
}

void XMLURL::setURL(const XMLCh* const    baseURL
                  , const XMLCh* const    relativeURL)
{
    cleanUp();

    // Parse our URL string
    parse(relativeURL);

	//
	//  If its relative and the base is non-null and non-empty, then
	//  parse the base URL string and conglomerate them.
	//
	if (isRelative() && baseURL)
	{
		if (*baseURL)
		{
			XMLURL basePart(baseURL, fMemoryManager);
			if (!conglomerateWithBase(basePart, false))
			{
				cleanUp();
				ThrowXMLwithMemMgr(MalformedURLException, XMLExcepts::URL_RelativeBaseURL, fMemoryManager);
			}
		}
	}
}

// this version of setURL doesn't throw a malformedurl exception
// instead it returns false when it failed (or when it would of
// thrown a malformedurl exception)
bool XMLURL::setURL(const XMLCh* const    baseURL
                  , const XMLCh* const    relativeURL
                  , XMLURL& xmlURL)
{
    cleanUp();

    // Parse our URL string
    if (parse(relativeURL, xmlURL))
    {
	    //  If its relative and the base is non-null and non-empty, then
	    //  parse the base URL string and conglomerate them.
	    //
	    if (isRelative() && baseURL && *baseURL)
	    {
	        XMLURL basePart(fMemoryManager);
            if (parse(baseURL, basePart)  && conglomerateWithBase(basePart, false))
            {
		        return true;
		    }
	    }
        else
            return true;
    }
    return false;
}

void XMLURL::setURL(const XMLURL&         baseURL
                  , const XMLCh* const    relativeURL)
{
    cleanUp();

	// Parse our URL string
    parse(relativeURL);

    // If its relative, then conglomerate with the base URL
    if (isRelative())
		conglomerateWithBase(baseURL);
}


// ---------------------------------------------------------------------------
//  XMLURL: Miscellaneous methods
// ---------------------------------------------------------------------------
bool XMLURL::isRelative() const
{
    // If no protocol then relative
    if (fProtocol == Unknown)
        return true;

    // If no path, or the path is not absolute, then relative
    if (!fPath)
        return true;

    if (*fPath != chForwardSlash)
        return true;

    return false;
}


bool XMLURL::hasInvalidChar() const {
    return fHasInvalidChar;
}


BinInputStream* XMLURL::makeNewStream() const
{
    //
    //  If its a local host, then we short circuit it and use our own file
    //  stream support. Otherwise, we just let it fall through and let the
    //  installed network access object provide a stream.
    //
    if (fProtocol == XMLURL::File)
    {
        if (!fHost || !XMLString::compareIStringASCII(fHost, XMLUni::fgLocalHostString))
        {

            XMLCh* realPath = XMLString::replicate(fPath, fMemoryManager);
            ArrayJanitor<XMLCh> basePathName(realPath, fMemoryManager);

            //
            // Need to manually replace any character reference %xx first
            // HTTP protocol will be done automatically by the netaccessor
            //
            XMLSize_t end = XMLString::stringLen(realPath);
            int percentIndex = XMLString::indexOf(realPath, chPercent, 0, fMemoryManager);

            while (percentIndex != -1) {

                if (percentIndex+2 >= (int)end ||
                    !isHexDigit(realPath[percentIndex+1]) ||
                    !isHexDigit(realPath[percentIndex+2]))
                {
                    XMLCh value1[4];
                    XMLString::moveChars(value1, &(realPath[percentIndex]), 3);
                    value1[3] = chNull;
                    ThrowXMLwithMemMgr2(MalformedURLException
                            , XMLExcepts::XMLNUM_URI_Component_Invalid_EscapeSequence
                            , realPath
                            , value1
                            , fMemoryManager);
                }

                unsigned int value = (xlatHexDigit(realPath[percentIndex+1]) * 16) + xlatHexDigit(realPath[percentIndex+2]);

                realPath[percentIndex] = XMLCh(value);

                XMLSize_t i =0;
                for (i = percentIndex + 1; i < end - 2 ; i++)
                    realPath[i] = realPath[i+2];
                realPath[i] = chNull;
                end = i;

                if (((XMLSize_t)(percentIndex + 1)) < end)
                  percentIndex = XMLString::indexOf(realPath, chPercent, percentIndex + 1, fMemoryManager);
                else
                  percentIndex = -1;
            }


            BinFileInputStream* retStrm = new (fMemoryManager) BinFileInputStream(realPath, fMemoryManager);
            if (!retStrm->getIsOpen())
            {
                delete retStrm;
                return 0;
            }
            return retStrm;
        }
    }

    //
    //  If we don't have have an installed net accessor object, then we
    //  have to just throw here.
    //
    if (!XMLPlatformUtils::fgNetAccessor)
        ThrowXMLwithMemMgr(MalformedURLException, XMLExcepts::URL_UnsupportedProto, fMemoryManager);

    // Else ask the net accessor to create the stream
    return XMLPlatformUtils::fgNetAccessor->makeNew(*this);
}

void XMLURL::makeRelativeTo(const XMLCh* const baseURLText)
{
    // If this one is not relative, don't bother
    if (!isRelative())
        return;

    XMLURL baseURL(baseURLText, fMemoryManager);
    conglomerateWithBase(baseURL);
}

void XMLURL::makeRelativeTo(const XMLURL& baseURL)
{
    // If this one is not relative, don't bother
    if (!isRelative())
        return;
    conglomerateWithBase(baseURL);
}




// ---------------------------------------------------------------------------
//  XMLURL: Private helper methods
// ---------------------------------------------------------------------------

//
//  This method will take the broken out parts of the URL and build up the
//  full text. We don't do this unless someone asks us to, since its often
//  never required.
//
void XMLURL::buildFullText()
{
    // Calculate the worst case size of the buffer required
    XMLSize_t bufSize = gMaxProtoLen + 1
                           + XMLString::stringLen(fFragment) + 1
                           + XMLString::stringLen(fHost) + 2
                           + XMLString::stringLen(fPassword) + 1
                           + XMLString::stringLen(fPath)
                           + XMLString::stringLen(fQuery) + 1
                           + XMLString::stringLen(fUser) + 1
                           + 32;

    // Clean up the existing buffer and allocate another
    fMemoryManager->deallocate(fURLText);//delete [] fURLText;
    fURLText = (XMLCh*) fMemoryManager->allocate((bufSize) * sizeof(XMLCh));//new XMLCh[bufSize];
    *fURLText = 0;

    XMLCh* outPtr = fURLText;
    if (fProtocol != Unknown)
    {
        XMLString::catString(fURLText, getProtocolName());
        outPtr += XMLString::stringLen(fURLText);
        *outPtr++ = chColon;
        *outPtr++ = chForwardSlash;
        *outPtr++ = chForwardSlash;
    }

    if (fUser)
    {
        XMLString::copyString(outPtr, fUser);
        outPtr += XMLString::stringLen(fUser);

        if (fPassword)
        {
            *outPtr++ = chColon;
            XMLString::copyString(outPtr, fPassword);
            outPtr += XMLString::stringLen(fPassword);
        }

        *outPtr++ = chAt;
    }

    if (fHost)
    {
        XMLString::copyString(outPtr, fHost);
        outPtr += XMLString::stringLen(fHost);

        //
        //  If the port is zero, then we don't put it in. Else we need
        //  to because it was explicitly provided.
        //
        if (fPortNum)
        {
            *outPtr++ = chColon;

            XMLCh tmpBuf[17];
            XMLString::binToText(fPortNum, tmpBuf, 16, 10, fMemoryManager);
            XMLString::copyString(outPtr, tmpBuf);
            outPtr += XMLString::stringLen(tmpBuf);
        }
    }

    if (fPath)
    {
        XMLString::copyString(outPtr, fPath);
        outPtr += XMLString::stringLen(fPath);
    }

    if (fQuery)
    {
        *outPtr++ = chQuestion;
        XMLString::copyString(outPtr, fQuery);
        outPtr += XMLString::stringLen(fQuery);
    }

    if (fFragment)
    {
        *outPtr++ = chPound;
        XMLString::copyString(outPtr, fFragment);
        outPtr += XMLString::stringLen(fFragment);
    }

    // Cap it off in case the last op was not a string copy
    *outPtr = 0;
}


//
//  Just a central place to handle cleanup, since its done from a number
//  of different spots.
//
void XMLURL::cleanUp()
{
    fMemoryManager->deallocate(fFragment);//delete [] fFragment;
    fMemoryManager->deallocate(fHost);//delete [] fHost;
    fMemoryManager->deallocate(fPassword);//delete [] fPassword;
    fMemoryManager->deallocate(fPath);//delete [] fPath;
    fMemoryManager->deallocate(fQuery);//delete [] fQuery;
    fMemoryManager->deallocate(fUser);//delete [] fUser;
    fMemoryManager->deallocate(fURLText);//delete [] fURLText;

    fFragment = 0;
    fHost = 0;
    fPassword = 0;
    fPath = 0;
    fQuery = 0;
    fUser = 0;
    fURLText = 0;

    fProtocol = Unknown;
    fPortNum = 0;
    fHasInvalidChar = false;
}


//This function  has been modified to take a bool parameter and the
//functionality inside looks irrational but is only to make
//solaris 2.7 CC 5.0 optimized build happy.

bool XMLURL::conglomerateWithBase(const XMLURL& baseURL, bool useExceptions)
{
    // The base URL cannot be relative
    if (baseURL.isRelative())
    {
        if (useExceptions)
			ThrowXMLwithMemMgr(MalformedURLException, XMLExcepts::URL_RelativeBaseURL, fMemoryManager);
        else
            return false;
    }

    //
    //  Check a special case. If all we have is a fragment, then we want
    //  to just take the base host and path, plus our fragment.
    //
    if ((fProtocol == Unknown)
    &&  !fHost
    &&  !fPath
    &&  fFragment)
    {
        // Just in case, make sure we don't leak the user or password values
        fMemoryManager->deallocate(fUser);//delete [] fUser;
        fUser = 0;
        fMemoryManager->deallocate(fPassword);//delete [] fPassword;
        fPassword = 0;

        // Copy over the protocol and port number as is
        fProtocol = baseURL.fProtocol;
        fPortNum = baseURL.fPortNum;

        // Replicate the base fields that are provided
        fHost = XMLString::replicate(baseURL.fHost, fMemoryManager);
        fUser = XMLString::replicate(baseURL.fUser, fMemoryManager);
        fPassword = XMLString::replicate(baseURL.fPassword, fMemoryManager);
        fPath = XMLString::replicate(baseURL.fPath, fMemoryManager);
        return true;
    }

    //
    //  All we have to do is run up through our fields and, for each one
    //  that we don't have, use the based URL's. Once we hit one field
    //  that we have, we stop.
    //
    if (fProtocol != Unknown)
        return true;
    fProtocol = baseURL.fProtocol;

    //
    //  If the protocol is not file, and we either already have our own
    //  host, or the base does not have one, then we are done.
    //
    if (fProtocol != File)
    {
        if (fHost || !baseURL.fHost)
            return true;
    }

    // Replicate all of the hosty stuff if the base has one
    if (baseURL.fHost)
    {
        // Just in case, make sure we don't leak a user or password field
        fMemoryManager->deallocate(fUser);//delete [] fUser;
        fUser = 0;
        fMemoryManager->deallocate(fPassword);//delete [] fPassword;
        fPassword = 0;
        fMemoryManager->deallocate(fHost);//delete [] fHost;
        fHost = 0;

        fHost = XMLString::replicate(baseURL.fHost, fMemoryManager);
        fUser = XMLString::replicate(baseURL.fUser, fMemoryManager);
        fPassword = XMLString::replicate(baseURL.fPassword, fMemoryManager);

        fPortNum = baseURL.fPortNum;
    }

    // If we have a path and its absolute, then we are done
    const bool hadPath = (fPath != 0);
    if (hadPath)
    {
        if (*fPath == chForwardSlash)
            return true;
    }

    // Its a relative path, so weave them together.
    if (baseURL.fPath) {
        XMLCh* temp = XMLPlatformUtils::weavePaths(baseURL.fPath, fPath ,fMemoryManager);
        fMemoryManager->deallocate(fPath);//delete [] fPath;
        fPath = temp;
    }

    // If we had any original path, then we are done
    if (hadPath)
        return true;

    // We had no original path, so go on to deal with the query/fragment parts
    if (fQuery || !baseURL.fQuery)
        return true;
    fQuery = XMLString::replicate(baseURL.fQuery, fMemoryManager);

    if (fFragment || !baseURL.fFragment)
        return true;
    fFragment = XMLString::replicate(baseURL.fFragment, fMemoryManager);
	return true;
}


void XMLURL::parse(const XMLCh* const urlText)
{
    // Simplify things by checking for the psycho scenarios first
    if (!*urlText)
        ThrowXMLwithMemMgr(MalformedURLException, XMLExcepts::URL_NoProtocolPresent, fMemoryManager);

    // Before we start, check if this urlText contains valid uri characters
    if (!XMLUri::isURIString(urlText))
        fHasInvalidChar = true;
    else
        fHasInvalidChar = false;

    //
    //  The first thing we will do is to check for a file name, so that
    //  we don't waste time thinking its a URL. If its in the form x:\ or x:/
    //  and x is an ASCII letter, then assume that's the deal.
    //
    if (((*urlText >= chLatin_A) && (*urlText <= chLatin_Z))
    ||  ((*urlText >= chLatin_a) && (*urlText <= chLatin_z)))
    {
        if (*(urlText + 1) == chColon)
        {
            if ((*(urlText + 2) == chForwardSlash)
            ||  (*(urlText + 2) == chBackSlash))
            {
                ThrowXMLwithMemMgr(MalformedURLException, XMLExcepts::URL_NoProtocolPresent, fMemoryManager);
            }
        }
    }

    // Get a copy of the URL that we can modify
    XMLCh* srcCpy = XMLString::replicate(urlText, fMemoryManager);
    ArrayJanitor<XMLCh> janSrcCopy(srcCpy, fMemoryManager);

    //
    //  Get a pointer now that we can run up thrown the source as we parse
    //  bits and pieces out of it.
    //
    XMLCh* srcPtr = srcCpy;

    // Run up past any spaces
    while (*srcPtr)
    {
        if (!XMLChar1_0::isWhitespace(*srcPtr))
            break;
        srcPtr++;
    }

    // Make sure it wasn't all space
    if (!*srcPtr)
        ThrowXMLwithMemMgr(MalformedURLException, XMLExcepts::URL_NoProtocolPresent, fMemoryManager);

    //
    //  Ok, the next thing we have to do is to find either a / or : character.
    //  If the : is first, we assume we have a protocol. If the / is first,
    //  then we skip to the host processing.
    //
    XMLCh* ptr1 = XMLString::findAny(srcPtr, gListOne);
    XMLCh* ptr2;

    // If we found a protocol, then deal with it
    if (ptr1)
    {
        if (*ptr1 == chColon)
        {
            // Cap the string at the colon
            *ptr1 = 0;

            // And try to find it in our list of protocols
            fProtocol = lookupByName(srcPtr);

            if (fProtocol == Unknown)
            {
                ThrowXMLwithMemMgr1
                (
                    MalformedURLException
                    , XMLExcepts::URL_UnsupportedProto1
                    , srcPtr
                    , fMemoryManager
                );
            }

            // And move our source pointer up past what we've processed
            srcPtr = (ptr1 + 1);
        }
    }

    //
    //  Ok, next we need to see if we have any host part. If the next
    //  two characters are //, then we need to check, else move on.
    //
    if ((*srcPtr == chForwardSlash) && (*(srcPtr + 1) == chForwardSlash))
    {
        // Move up past the slashes
        srcPtr += 2;

        //
        //  If we aren't at the end of the string, then there has to be a
        //  host part at this point. we will just look for the next / char
        //  or end of string and make all of that the host for now.
        //
        if (*srcPtr)
        {
            // Search from here for a / character
            ptr1 = XMLString::findAny(srcPtr, gListFour);

            //
            //  If we found something, then the host is between where
            //  we are and what we found. Else the host is the rest of
            //  the content and we are done. If its empty, leave it null.
            //
            if (ptr1)
            {
                if (ptr1 != srcPtr)
                {
                    fMemoryManager->deallocate(fHost);//delete [] fHost;
                    fHost = (XMLCh*) fMemoryManager->allocate
                    (
                        ((ptr1 - srcPtr) + 1) * sizeof(XMLCh)
                    );//new XMLCh[(ptr1 - srcPtr) + 1];
                    ptr2 = fHost;
                    while (srcPtr < ptr1)
                        *ptr2++ = *srcPtr++;
                    *ptr2 = 0;
                }
            }
             else
            {
                fMemoryManager->deallocate(fHost);//delete [] fHost;
                fHost = XMLString::replicate(srcPtr, fMemoryManager);

                // Update source pointer to the end
                srcPtr += XMLString::stringLen(fHost);
            }
        }
    }
    else
    {
        //
        // http protocol requires two forward slashes
        // we didn't get them, so throw an exception
        //
        if (fProtocol == HTTP) {
            ThrowXMLwithMemMgr
                (
                    MalformedURLException
                    , XMLExcepts::URL_ExpectingTwoSlashes
                    , fMemoryManager
                );
        }
    }

    //
    //  If there was a host part, then we have to grovel through it for
    //  all the bits and pieces it can hold.
    //
    if (fHost)
    {
        //
        //  Look for a '@' character, which indicates a user name. If we
        //  find one, then everything between the start of the host data
        //  and the character is the user name.
        //
        ptr1 = XMLString::findAny(fHost, gListTwo);
        if (ptr1)
        {
            // Get this info out as the user name
            *ptr1 = 0;
            fMemoryManager->deallocate(fUser);//delete [] fUser;
            fUser = XMLString::replicate(fHost, fMemoryManager);
            ptr1++;

            // And now cut these chars from the host string
            XMLString::cut(fHost, ptr1 - fHost);

            // Is there a password inside the user string?
            ptr2 = XMLString::findAny(fUser, gListThree);
            if (ptr2)
            {
                // Remove it from the user name string
                *ptr2 = 0;

                // And copy out the remainder to the password field
                ptr2++;
                fMemoryManager->deallocate(fPassword);//delete [] fPassword;
                fPassword = XMLString::replicate(ptr2, fMemoryManager);
            }
        }

        //
        //  Ok, so now we are at the actual host name, if any. If we are
        //  not at the end of the host data, then lets see if we have a
        //  port trailing the
        //
        ptr1 = XMLString::findAny(fHost, gListThree);
        if (ptr1)
        {
            // Remove it from the host name
            *ptr1 = 0;

            // Try to convert it to a numeric port value and store it
            ptr1++;
            if (!XMLString::textToBin(ptr1, fPortNum, fMemoryManager))
                ThrowXMLwithMemMgr(MalformedURLException, XMLExcepts::URL_BadPortField, fMemoryManager);
        }

        // If the host ended up empty, then toss is
        if (!*fHost)
        {
            fMemoryManager->deallocate(fHost);//delete[] fHost;
            fHost = 0;
        }
    }

    // If we are at the end, then we are done now
    if (!*srcPtr) {
        if(fHost) {
            static const XMLCh slash[] = { chForwardSlash, chNull };
            fPath = XMLString::replicate(slash, fMemoryManager);
        }
        return;
    }

    //
    //  Next is the path part. It can be absolute, i.e. starting with a
    //  forward slash character, or relative. Its basically everything up
    //  to the end of the string or to any trailing query or fragment.
    //
    ptr1 = XMLString::findAny(srcPtr, gListFive);
    if (!ptr1)
    {
        fMemoryManager->deallocate(fPath);//delete [] fPath;
        fPath = XMLString::replicate(srcPtr, fMemoryManager);
        return;
    }

    // Everything from where we are to what we found is the path
    if (ptr1 > srcPtr)
    {
        fMemoryManager->deallocate(fPath);//delete [] fPath;
        fPath = (XMLCh*) fMemoryManager->allocate
        (
            ((ptr1 - srcPtr) + 1) * sizeof(XMLCh)
        );//new XMLCh[(ptr1 - srcPtr) + 1];
        ptr2 = fPath;
        while (srcPtr < ptr1)
            *ptr2++ = *srcPtr++;
        *ptr2 = 0;
    }

    //
    //  If we found a fragment, then it is the rest of the string and we
    //  are done.
    //
    if (*srcPtr == chPound)
    {
        srcPtr++;
        fMemoryManager->deallocate(fFragment);//delete [] fFragment;
        fFragment = XMLString::replicate(srcPtr, fMemoryManager);
        return;
    }

    //
    //  The query is either the rest of the string, or up to the fragment
    //  separator.
    //
    srcPtr++;
    ptr1 = XMLString::findAny(srcPtr, gListSix);
    fMemoryManager->deallocate(fQuery);//delete [] fQuery;
    if (!ptr1)
    {
        fQuery = XMLString::replicate(srcPtr, fMemoryManager);
        return;
    }
     else
    {
        fQuery = (XMLCh*) fMemoryManager->allocate
        (
            ((ptr1 - srcPtr) + 1) * sizeof(XMLCh)
        );//new XMLCh[(ptr1 - srcPtr) + 1];
        ptr2 = fQuery;
        while (srcPtr < ptr1)
            *ptr2++ = *srcPtr++;
        *ptr2 = 0;
    }

    // If we are not at the end now, then everything else is the fragment
    if (*srcPtr == chPound)
    {
        srcPtr++;
        fMemoryManager->deallocate(fFragment);//delete [] fFragment;
        fFragment = XMLString::replicate(srcPtr, fMemoryManager);
    }
}

bool XMLURL::parse(const XMLCh* const urlText, XMLURL& xmlURL)
{
    // Simplify things by checking for the psycho scenarios first
    if (!*urlText)
        return false;

    // Before we start, check if this urlText contains valid uri characters
    if (!XMLUri::isURIString(urlText))
        xmlURL.fHasInvalidChar = true;
    else
        xmlURL.fHasInvalidChar = false;

    //
    //  The first thing we will do is to check for a file name, so that
    //  we don't waste time thinking its a URL. If its in the form x:\ or x:/
    //  and x is an ASCII letter, then assume that's the deal.
    //
    if (((*urlText >= chLatin_A) && (*urlText <= chLatin_Z))
    ||  ((*urlText >= chLatin_a) && (*urlText <= chLatin_z)))
    {
        if (*(urlText + 1) == chColon)
        {
            if ((*(urlText + 2) == chForwardSlash)
            ||  (*(urlText + 2) == chBackSlash))
            {
                return false;
            }
        }
    }

    // Get a copy of the URL that we can modify
    XMLCh* srcCpy = XMLString::replicate(urlText, xmlURL.fMemoryManager);
    ArrayJanitor<XMLCh> janSrcCopy(srcCpy, xmlURL.fMemoryManager);

    //
    //  Get a pointer now that we can run up thrown the source as we parse
    //  bits and pieces out of it.
    //
    XMLCh* srcPtr = srcCpy;

    // Run up past any spaces
    while (*srcPtr)
    {
        if (!XMLChar1_0::isWhitespace(*srcPtr))
            break;
        srcPtr++;
    }

    // Make sure it wasn't all space
    if (!*srcPtr)
        return false;

    //
    //  Ok, the next thing we have to do is to find either a / or : character.
    //  If the : is first, we assume we have a protocol. If the / is first,
    //  then we skip to the host processing.
    //
    XMLCh* ptr1 = XMLString::findAny(srcPtr, gListOne);
    XMLCh* ptr2;

    // If we found a protocol, then deal with it
    if (ptr1)
    {
        if (*ptr1 == chColon)
        {
            // Cap the string at the colon
            *ptr1 = 0;

            // And try to find it in our list of protocols
            xmlURL.fProtocol = lookupByName(srcPtr);

            if (xmlURL.fProtocol == Unknown)
                return false;

            // And move our source pointer up past what we've processed
            srcPtr = (ptr1 + 1);
        }
    }

    //
    //  Ok, next we need to see if we have any host part. If the next
    //  two characters are //, then we need to check, else move on.
    //
    if ((*srcPtr == chForwardSlash) && (*(srcPtr + 1) == chForwardSlash))
    {
        // Move up past the slashes
        srcPtr += 2;

        //
        //  If we aren't at the end of the string, then there has to be a
        //  host part at this point. we will just look for the next / char
        //  or end of string and make all of that the host for now.
        //
        if (*srcPtr)
        {
            // Search from here for a / character
            ptr1 = XMLString::findAny(srcPtr, gListFour);

            //
            //  If we found something, then the host is between where
            //  we are and what we found. Else the host is the rest of
            //  the content and we are done. If its empty, leave it null.
            //
            if (ptr1)
            {
                if (ptr1 != srcPtr)
                {
                    xmlURL.fHost = (XMLCh*) xmlURL.fMemoryManager->allocate
                    (
                        ((ptr1 - srcPtr) + 1) * sizeof(XMLCh)
                    );//new XMLCh[(ptr1 - srcPtr) + 1];
                    ptr2 = xmlURL.fHost;
                    while (srcPtr < ptr1)
                        *ptr2++ = *srcPtr++;
                    *ptr2 = 0;
                }
            }
             else
            {
                xmlURL.fHost = XMLString::replicate(srcPtr, xmlURL.fMemoryManager);

                // Update source pointer to the end
                srcPtr += XMLString::stringLen(xmlURL.fHost);
            }
        }
    }
    else
    {
        //
        // http protocol requires two forward slashes
        // we didn't get them, so throw an exception
        //
        if (xmlURL.fProtocol == HTTP)
            return false;
    }

    //
    //  If there was a host part, then we have to grovel through it for
    //  all the bits and pieces it can hold.
    //
    if (xmlURL.fHost)
    {
        //
        //  Look for a '@' character, which indicates a user name. If we
        //  find one, then everything between the start of the host data
        //  and the character is the user name.
        //
        ptr1 = XMLString::findAny(xmlURL.fHost, gListTwo);
        if (ptr1)
        {
            // Get this info out as the user name
            *ptr1 = 0;
            xmlURL.fUser = XMLString::replicate(xmlURL.fHost, xmlURL.fMemoryManager);
            ptr1++;

            // And now cut these chars from the host string
            XMLString::cut(xmlURL.fHost, ptr1 - xmlURL.fHost);

            // Is there a password inside the user string?
            ptr2 = XMLString::findAny(xmlURL.fUser, gListThree);
            if (ptr2)
            {
                // Remove it from the user name string
                *ptr2 = 0;

                // And copy out the remainder to the password field
                ptr2++;
                xmlURL.fPassword = XMLString::replicate(ptr2, xmlURL.fMemoryManager);
            }
        }

        //
        //  Ok, so now we are at the actual host name, if any. If we are
        //  not at the end of the host data, then lets see if we have a
        //  port trailing the
        //
        ptr1 = XMLString::findAny(xmlURL.fHost, gListThree);
        if (ptr1)
        {
            // Remove it from the host name
            *ptr1 = 0;

            // Try to convert it to a numeric port value and store it
            ptr1++;
            if (!XMLString::textToBin(ptr1, xmlURL.fPortNum, xmlURL.fMemoryManager))
                return false;
        }

        // If the host ended up empty, then toss is
        if (!*(xmlURL.fHost))
        {
            xmlURL.fMemoryManager->deallocate(xmlURL.fHost);//delete[] fHost;
            xmlURL.fHost = 0;
        }
    }

    // If we are at the end, then we are done now
    if (!*srcPtr) {
        if(xmlURL.fHost) {
            static const XMLCh slash[] = { chForwardSlash, chNull };
            xmlURL.fPath = XMLString::replicate(slash, xmlURL.fMemoryManager);
        }
        return true;
    }

    //
    //  Next is the path part. It can be absolute, i.e. starting with a
    //  forward slash character, or relative. Its basically everything up
    //  to the end of the string or to any trailing query or fragment.
    //
    ptr1 = XMLString::findAny(srcPtr, gListFive);
    if (!ptr1)
    {
        xmlURL.fPath = XMLString::replicate(srcPtr, xmlURL.fMemoryManager);
        return true;
    }

    // Everything from where we are to what we found is the path
    if (ptr1 > srcPtr)
    {
        xmlURL.fPath = (XMLCh*) xmlURL.fMemoryManager->allocate
        (
            ((ptr1 - srcPtr) + 1) * sizeof(XMLCh)
        );//new XMLCh[(ptr1 - srcPtr) + 1];
        ptr2 = xmlURL.fPath;
        while (srcPtr < ptr1)
            *ptr2++ = *srcPtr++;
        *ptr2 = 0;
    }

    //
    //  If we found a fragment, then it is the rest of the string and we
    //  are done.
    //
    if (*srcPtr == chPound)
    {
        srcPtr++;
        xmlURL.fFragment = XMLString::replicate(srcPtr, xmlURL.fMemoryManager);
        return true;
    }

    //
    //  The query is either the rest of the string, or up to the fragment
    //  separator.
    //
    srcPtr++;
    ptr1 = XMLString::findAny(srcPtr, gListSix);
    if (!ptr1)
    {
        xmlURL.fQuery = XMLString::replicate(srcPtr, xmlURL.fMemoryManager);
        return true;
    }
     else
    {
        xmlURL.fQuery = (XMLCh*) xmlURL.fMemoryManager->allocate
        (
            ((ptr1 - srcPtr) + 1) * sizeof(XMLCh)
        );//new XMLCh[(ptr1 - srcPtr) + 1];
        ptr2 = xmlURL.fQuery;
        while (srcPtr < ptr1)
            *ptr2++ = *srcPtr++;
        *ptr2 = 0;
    }

    // If we are not at the end now, then everything else is the fragment
    if (*srcPtr == chPound)
    {
        srcPtr++;
        xmlURL.fFragment = XMLString::replicate(srcPtr, xmlURL.fMemoryManager);
    }

    return true;
}

XERCES_CPP_NAMESPACE_END
