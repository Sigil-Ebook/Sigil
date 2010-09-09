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
 * $Id: XMLUri.cpp 881714 2009-11-18 10:39:06Z borisk $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/Janitor.hpp>
#include <xercesc/util/XMLURL.hpp>
#include <xercesc/util/XMLUri.hpp>
#include <xercesc/util/XMLChar.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  XMLUri: static data
// ---------------------------------------------------------------------------

//      Amended by RFC2732
//      reserved      = ";" | "/" | "?" | ":" | "@" | "&" | "=" | "+" |
//                      "$" | "," | "[" | "]"
//
const XMLCh XMLUri::RESERVED_CHARACTERS[] =
{
    chSemiColon, chForwardSlash, chQuestion, chColon, chAt,
    chAmpersand, chEqual, chPlus, chDollarSign, chComma, chOpenSquare,
    chCloseSquare, chNull
};

//
//      mark          = "-" | "_" | "." | "!" | "~" | "*" | "'" |
//                      "(" | ")"
//
const XMLCh XMLUri::MARK_CHARACTERS[] =
{
    chDash, chUnderscore, chPeriod, chBang, chTilde,
    chAsterisk, chSingleQuote, chOpenParen, chCloseParen, chNull
};

// combination of MARK and RESERVED
const XMLCh XMLUri::MARK_OR_RESERVED_CHARACTERS[] =
{
    chDash, chUnderscore, chPeriod, chBang, chTilde,
    chAsterisk, chSingleQuote, chOpenParen, chCloseParen,
    chSemiColon, chForwardSlash, chQuestion, chColon, chAt,
    chAmpersand, chEqual, chPlus, chDollarSign, chComma, chOpenSquare,
    chCloseSquare, chNull
};

//
//      scheme        = alpha *( alpha | digit | "+" | "-" | "." )
//
const XMLCh XMLUri::SCHEME_CHARACTERS[] =
{
    chPlus, chDash, chPeriod, chNull
};

//
//      userinfo      = *( unreserved | escaped |
//                         ";" | ":" | "&" | "=" | "+" | "$" | "," )
//
const XMLCh XMLUri::USERINFO_CHARACTERS[] =
{
    chSemiColon, chColon, chAmpersand, chEqual, chPlus,
    chDollarSign, chPeriod, chNull
};

//
//      reg_name     = 1*( unreserved | escaped | "$" | "," |
//                         ";" | ":" | "@" | "&" | "=" | "+" )
//
const XMLCh XMLUri::REG_NAME_CHARACTERS[] =
{
    chDollarSign, chComma, chSemiColon, chColon, chAt,
    chAmpersand, chEqual, chPlus, chNull
};

//      pchar plus ';' and '/'.
//      pchar         = unreserved | escaped |
//                      ":" | "@" | "&" | "=" | "+" | "$" | ","
const XMLCh XMLUri::PATH_CHARACTERS[] =
{
    chSemiColon, chForwardSlash, chColon, chAt, chAmpersand,
    chEqual, chPlus, chDollarSign, chComma, chNull
};


// ---------------------------------------------------------------------------
//  Local methods and data
// ---------------------------------------------------------------------------
static const int BUF_LEN = 64;

//
// "Scheme"
// "SchemeSpecificPart"
// "Parameters"
// "UserInfo"
// "Host"
// "Port"
// "RegName"
// "Path"
// "Query"
// "Fragment"
//
static const XMLCh errMsg_SCHEME[] =
{
    chLatin_s, chLatin_c, chLatin_h, chLatin_e,
    chLatin_m, chLatin_e, chNull
};

static const XMLCh errMsg_SCHEMESPART[] =
{
    chLatin_s, chLatin_c, chLatin_h, chLatin_e, chLatin_m, chLatin_e,
    chLatin_S, chLatin_p, chLatin_e, chLatin_c, chLatin_i, chLatin_f,
    chLatin_i, chLatin_c, chLatin_P, chLatin_a, chLatin_r, chLatin_t,
    chNull
};

static const XMLCh errMsg_PARAMS[] =
{
    chLatin_p, chLatin_a, chLatin_r, chLatin_a, chLatin_m,
    chLatin_e, chLatin_t, chLatin_e, chLatin_r, chLatin_s, chNull
};

static const XMLCh errMsg_USERINFO[] =
{
    chLatin_u, chLatin_s, chLatin_e, chLatin_r,
    chLatin_i, chLatin_n, chLatin_f, chLatin_o, chNull
};

static const XMLCh errMsg_HOST[] =
{
    chLatin_h, chLatin_o, chLatin_s, chLatin_t, chNull
};

static const XMLCh errMsg_PORT[] =
{
    chLatin_p, chLatin_o, chLatin_r, chLatin_t, chNull
};

static const XMLCh errMsg_REGNAME[] =
{
    chLatin_R, chLatin_e, chLatin_g,
    chLatin_N, chLatin_a, chLatin_m, chLatin_e, chNull
};

static const XMLCh errMsg_PATH[] =
{
    chLatin_p, chLatin_a, chLatin_t, chLatin_h, chNull
};

static const XMLCh errMsg_QUERY[] =
{
    chLatin_q, chLatin_u, chLatin_e, chLatin_r, chLatin_y, chNull
};

static const XMLCh errMsg_FRAGMENT[] =
{
    chLatin_f, chLatin_r, chLatin_a, chLatin_g,
    chLatin_m, chLatin_e, chLatin_n, chLatin_t, chNull
};

//
//  "//"
//  "/"
//  "./"
//  "/."
//  "/../"
//  "/.."
//
static const XMLCh DOUBLE_SLASH[] =
{
    chForwardSlash, chForwardSlash, chNull
};

static const XMLCh SINGLE_SLASH[] =
{
    chForwardSlash, chNull
};

static const XMLCh SLASH_DOT_SLASH[] =
{
    chForwardSlash, chPeriod, chForwardSlash, chNull
};

static const XMLCh SLASH_DOT[] =
{
    chForwardSlash, chPeriod, chNull
};

static const XMLCh SLASH_DOTDOT_SLASH[] =
{
    chForwardSlash, chPeriod, chPeriod, chForwardSlash, chNull
};

static const XMLCh SLASH_DOTDOT[] =
{
    chForwardSlash, chPeriod, chPeriod, chNull
};

//
//  ":/?#"
//
// REVISIT: why?
static const XMLCh SCHEME_SEPARATORS[] =
{
    chColon, chForwardSlash, chQuestion, chPound, chNull
};

//
//  "?#"
//
static const XMLCh PATH_SEPARATORS[] =
{
    chQuestion, chPound, chNull
};

// ---------------------------------------------------------------------------
//  XMLUri: Constructors and Helper methods
// ---------------------------------------------------------------------------
// ctor# 2

typedef JanitorMemFunCall<XMLUri>   CleanupType;

XMLUri::XMLUri(const XMLCh* const uriSpec,
               MemoryManager* const manager)
: fPort(-1)
, fScheme(0)
, fUserInfo(0)
, fHost(0)
, fRegAuth(0)
, fPath(0)
, fQueryString(0)
, fFragment(0)
, fURIText(0)
, fMemoryManager(manager)
{
    CleanupType cleanup(this, &XMLUri::cleanUp);

    try {
        initialize((XMLUri *)0, uriSpec);
    }
    catch(const OutOfMemoryException&)
    {
        cleanup.release();

        throw;
    }

    cleanup.release();
}

// ctor# 7 relative ctor
XMLUri::XMLUri(const XMLUri* const      baseURI
              , const XMLCh* const   uriSpec
              , MemoryManager* const manager)
: fPort(-1)
, fScheme(0)
, fUserInfo(0)
, fHost(0)
, fRegAuth(0)
, fPath(0)
, fQueryString(0)
, fFragment(0)
, fURIText(0)
, fMemoryManager(manager)
{
    CleanupType cleanup(this, &XMLUri::cleanUp);

    try {
        initialize(baseURI, uriSpec);
    }
    catch(const OutOfMemoryException&)
    {
        cleanup.release();

        throw;
    }

    cleanup.release();
}

//Copy constructor
XMLUri::XMLUri(const XMLUri& toCopy)
: XSerializable(toCopy)
, XMemory(toCopy)
, fPort(-1)
, fScheme(0)
, fUserInfo(0)
, fHost(0)
, fRegAuth(0)
, fPath(0)
, fQueryString(0)
, fFragment(0)
, fURIText(0)
, fMemoryManager(toCopy.fMemoryManager)
{
    CleanupType cleanup(this, &XMLUri::cleanUp);

    try {
        initialize(toCopy);
    }
    catch(const OutOfMemoryException&)
    {
        cleanup.release();

        throw;
    }

    cleanup.release();
}

XMLUri& XMLUri::operator=(const XMLUri& toAssign)
{
    cleanUp();

    CleanupType cleanup(this, &XMLUri::cleanUp);

    try {
        initialize(toAssign);
    }
    catch(const OutOfMemoryException&)
    {
        cleanup.release();

        throw;
    }

    cleanup.release();

    return *this;
}

XMLUri::~XMLUri()
{
    cleanUp();
}

void XMLUri::cleanUp()
{
    if (fScheme)
        XMLString::release(&fScheme, fMemoryManager);//delete[] fScheme;

    if (fUserInfo)
        XMLString::release(&fUserInfo, fMemoryManager);//delete[] fUserInfo;

    if (fHost)
        XMLString::release(&fHost, fMemoryManager);//delete[] fHost;

    if (fRegAuth)
        XMLString::release(&fRegAuth, fMemoryManager);//delete[] fRegAuth;

    if (fPath)
        XMLString::release(&fPath, fMemoryManager);//delete[] fPath;

    if (fQueryString)
        XMLString::release(&fQueryString, fMemoryManager);//delete[] fQueryString;

    if (fFragment)
        XMLString::release(&fFragment, fMemoryManager);//delete[] fFragment;

    XMLString::release(&fURIText, fMemoryManager);//delete[] fURIText;
}

void XMLUri::initialize(const XMLUri& toCopy)
{
    //
    // assuming that all fields from the toCopy are valid,
    // therefore need NOT to go through various setXXX() methods
    //
    fMemoryManager = toCopy.fMemoryManager;
    fScheme = XMLString::replicate(toCopy.fScheme, fMemoryManager);
    fUserInfo = XMLString::replicate(toCopy.fUserInfo, fMemoryManager);
    fHost = XMLString::replicate(toCopy.fHost, fMemoryManager);
    fPort = toCopy.fPort;
    fRegAuth = XMLString::replicate(toCopy.fRegAuth, fMemoryManager);
    fPath = XMLString::replicate(toCopy.fPath, fMemoryManager);
    fQueryString = XMLString::replicate(toCopy.fQueryString, fMemoryManager);
    fFragment = XMLString::replicate(toCopy.fFragment, fMemoryManager);
}

void XMLUri::initialize(const XMLUri* const baseURI
                      , const XMLCh*  const uriSpec)
{

    // get a trimmed version of uriSpec
    // uriSpec will NO LONGER be used in this function.
    //
    XMLCh* trimmedUriSpec = XMLString::replicate(uriSpec, fMemoryManager);
    XMLString::trim(trimmedUriSpec);
    ArrayJanitor<XMLCh> janName(trimmedUriSpec, fMemoryManager);
    XMLSize_t trimmedUriSpecLen = XMLString::stringLen(trimmedUriSpec);

    if ( !baseURI &&
        (!trimmedUriSpec || trimmedUriSpecLen == 0))
    {
        ThrowXMLwithMemMgr1(MalformedURLException
               , XMLExcepts::XMLNUM_URI_Component_Empty
               , errMsg_PARAMS
               , fMemoryManager);
    }

	// just make a copy of the base if spec is empty
	if (!trimmedUriSpec || trimmedUriSpecLen == 0)
    {
        initialize(*baseURI);
        return;
	}

	XMLSize_t index = 0;
	bool foundScheme = false;

	// Check for scheme, which must be before `/', '?' or '#'.
        int colonIdx = XMLString::indexOf(trimmedUriSpec, chColon);
        int slashIdx = XMLString::indexOf(trimmedUriSpec, chForwardSlash);
        int queryIdx = XMLString::indexOf(trimmedUriSpec, chQuestion);
        int fragmentIdx = XMLString::indexOf(trimmedUriSpec, chPound);

        if ((colonIdx <= 0) ||
            (colonIdx > slashIdx && slashIdx != -1) ||
            (colonIdx > queryIdx && queryIdx != -1) ||
            (colonIdx > fragmentIdx && fragmentIdx != -1))
        {
            // A standalone base is a valid URI according to spec
            if ( colonIdx == 0 || (!baseURI && fragmentIdx != 0) )
            {
                ThrowXMLwithMemMgr(MalformedURLException, XMLExcepts::XMLNUM_URI_No_Scheme, fMemoryManager);
            }
        }
        else
        {
            foundScheme = true;
            initializeScheme(trimmedUriSpec);
            index = XMLString::stringLen(fScheme)+1;
        }

    // It's an error if we stop here
    if (index == trimmedUriSpecLen || (foundScheme && (trimmedUriSpec[index] == chPound)))
    {
        ThrowXMLwithMemMgr1(MalformedURLException
                , XMLExcepts::XMLNUM_URI_Component_Empty
                , errMsg_PATH
                , fMemoryManager);
    }

	// two slashes means generic URI syntax, so we get the authority
    XMLCh* authUriSpec = (XMLCh*) fMemoryManager->allocate
    (
        (trimmedUriSpecLen+1) * sizeof(XMLCh)
    );//new XMLCh[trimmedUriSpecLen+1];
    ArrayJanitor<XMLCh> authName(authUriSpec, fMemoryManager);
    XMLString::subString(authUriSpec, trimmedUriSpec, index, trimmedUriSpecLen, fMemoryManager);

    if (((index+1) < trimmedUriSpecLen) &&
        XMLString::startsWith(authUriSpec, DOUBLE_SLASH))
    {
        index += 2;
        XMLSize_t startPos = index;

        // get authority - everything up to path, query or fragment
        XMLCh testChar;
        while (index < trimmedUriSpecLen)
        {
            testChar = trimmedUriSpec[index];
            if (testChar == chForwardSlash ||
                testChar == chQuestion     ||
                testChar == chPound         )
            {
                break;
            }

            index++;
        }

        // if we found authority, parse it out, otherwise we set the
        // host to empty string
        if (index > startPos)
        {
            XMLString::subString(authUriSpec, trimmedUriSpec, startPos, index, fMemoryManager);
            initializeAuthority(authUriSpec);
        }
        else
        {
            //fHost = 0;
            setHost(XMLUni::fgZeroLenString);
        }
    }

    // we need to check if index has exceed the lenght or not
    if (index >= trimmedUriSpecLen)
        return;

    XMLCh* pathUriSpec = (XMLCh*) fMemoryManager->allocate
    (
        (trimmedUriSpecLen+1) * sizeof(XMLCh)
    );//new XMLCh[trimmedUriSpecLen+1];
    ArrayJanitor<XMLCh> pathUriSpecName(pathUriSpec, fMemoryManager);
    XMLString::subString(pathUriSpec, trimmedUriSpec, index, trimmedUriSpecLen, fMemoryManager);

	initializePath(pathUriSpec);

	// Resolve relative URI to base URI - see RFC 2396 Section 5.2
	// In some cases, it might make more sense to throw an exception
	// (when scheme is specified is the string spec and the base URI
	// is also specified, for example), but we're just following the
	// RFC specifications
	if ( baseURI )
    {
        // check to see if this is the current doc - RFC 2396 5.2 #2
        // note that this is slightly different from the RFC spec in that
        // we don't include the check for query string being null
        // - this handles cases where the urispec is just a query
        // string or a fragment (e.g. "?y" or "#s") -
        // see <http://www.ics.uci.edu/~fielding/url/test1.html> which
        // identified this as a bug in the RFC
        if ((!fPath || !*fPath) &&
            fScheme == 0 &&
            fHost == 0 && fRegAuth == 0)
        {
            fScheme = XMLString::replicate(baseURI->getScheme(), fMemoryManager);
            fMemoryManager->deallocate(fUserInfo);//delete [] fUserInfo;
            fUserInfo = XMLString::replicate(baseURI->getUserInfo(), fMemoryManager);
            fHost = XMLString::replicate(baseURI->getHost(), fMemoryManager);
            fPort = baseURI->getPort();
            fRegAuth = XMLString::replicate(baseURI->getRegBasedAuthority(), fMemoryManager);
            fMemoryManager->deallocate(fPath);//delete [] fPath;
            fPath = XMLString::replicate(baseURI->getPath(), fMemoryManager);

            if ( !fQueryString )
            {
                fQueryString = XMLString::replicate(baseURI->getQueryString(), fMemoryManager);
            }
            return;
        }

        // check for scheme - RFC 2396 5.2 #3
        // if we found a scheme, it means absolute URI, so we're done
        if (fScheme == 0)
        {
            fScheme = XMLString::replicate(baseURI->getScheme(), fMemoryManager);
        }
        else
        {
            return;
        }

        // check for authority - RFC 2396 5.2 #4
        // if we found a host, then we've got a network path, so we're done
        if (fHost == 0 && fRegAuth == 0)
        {
            fMemoryManager->deallocate(fUserInfo);//delete [] fUserInfo;
            fUserInfo = XMLString::replicate(baseURI->getUserInfo(), fMemoryManager);
            fHost = XMLString::replicate(baseURI->getHost(), fMemoryManager);
            fPort = baseURI->getPort();
            fRegAuth = XMLString::replicate(baseURI->getRegBasedAuthority(), fMemoryManager);
        }
        else
        {
            return;
        }

        // check for absolute path - RFC 2396 5.2 #5
        if ((fPath && *fPath) &&
            XMLString::startsWith(fPath, SINGLE_SLASH))
        {
            return;
        }

        // if we get to this point, we need to resolve relative path
        // RFC 2396 5.2 #6

        XMLCh* basePath = XMLString::replicate(baseURI->getPath(), fMemoryManager);
        ArrayJanitor<XMLCh> basePathName(basePath, fMemoryManager);

        XMLSize_t bufLen = trimmedUriSpecLen+XMLString::stringLen(fPath)+XMLString::stringLen(basePath)+1;
        XMLCh* path = (XMLCh*) fMemoryManager->allocate(bufLen * sizeof(XMLCh));//new XMLCh[bufLen];
        ArrayJanitor<XMLCh> pathName(path, fMemoryManager);
        path[0] = 0;

        XMLCh* tmp1 = (XMLCh*) fMemoryManager->allocate(bufLen * sizeof(XMLCh));//new XMLCh[bufLen];
        ArrayJanitor<XMLCh> tmp1Name(tmp1, fMemoryManager);
        XMLCh* tmp2 = (XMLCh*) fMemoryManager->allocate(bufLen * sizeof(XMLCh));//new XMLCh[bufLen];
        ArrayJanitor<XMLCh> tmp2Name(tmp2, fMemoryManager);

        // 6a - get all but the last segment of the base URI path
        if (basePath)
        {
            int lastSlash = XMLString::lastIndexOf(basePath, chForwardSlash);
            if (lastSlash != -1)
            {
                XMLString::subString(path, basePath, 0, lastSlash+1, fMemoryManager);
            }
        }

        // 6b - append the relative URI path
        XMLString::catString(path, fPath);

        // 6c - remove all "./" where "." is a complete path segment
        int iIndex = -1;
        while ((iIndex = XMLString::patternMatch(path, SLASH_DOT_SLASH)) != -1)
        {
            XMLString::subString(tmp1, path, 0, iIndex, fMemoryManager);
            XMLString::subString(tmp2, path, iIndex+2, XMLString::stringLen(path), fMemoryManager);

            path[0] = 0;
            XMLString::catString(path, tmp1);
            XMLString::catString(path, tmp2);
        }

        // 6d - remove "." if path ends with "." as a complete path segment
        if (XMLString::endsWith(path, SLASH_DOT))
        {
            path[XMLString::stringLen(path) - 1] = chNull;
        }

        // 6e - remove all "<segment>/../" where "<segment>" is a complete
        // path segment not equal to ".."
        iIndex = -1;
        int segIndex = -1;
        int offset = 1;

        while ((iIndex = XMLString::patternMatch(&(path[offset]), SLASH_DOTDOT_SLASH)) != -1)
        {
			// Undo offset
			iIndex += offset;

			// Find start of <segment> within substring ending at found point.
			XMLString::subString(tmp1, path, 0, iIndex-1, fMemoryManager);
			segIndex = XMLString::lastIndexOf(tmp1, chForwardSlash);

			// Ensure <segment> exists and != ".."
            if (segIndex != -1                &&
                (path[segIndex+1] != chPeriod ||
                 path[segIndex+2] != chPeriod ||
				 segIndex + 3 != iIndex))
            {

                XMLString::subString(tmp1, path, 0, segIndex, fMemoryManager);
                XMLString::subString(tmp2, path, iIndex+3, XMLString::stringLen(path), fMemoryManager);

                path[0] = 0;
                XMLString::catString(path, tmp1);
                XMLString::catString(path, tmp2);

                offset = (segIndex == 0 ? 1 : segIndex);
            }
            else
            {
                offset += 4;
            }
        }// while

        // 6f - remove ending "<segment>/.." where "<segment>" is a
        // complete path segment
        if (XMLString::endsWith(path, SLASH_DOTDOT))
        {
			// Find start of <segment> within substring ending at found point.
            index = XMLString::stringLen(path) - 3;
			XMLString::subString(tmp1, path, 0, index-1, fMemoryManager);
			segIndex = XMLString::lastIndexOf(tmp1, chForwardSlash);

            if (segIndex != -1                &&
                (path[segIndex+1] != chPeriod ||
                 path[segIndex+2] != chPeriod ||
				 segIndex + 3 != (int)index))
            {
                path[segIndex+1] = chNull;
            }
        }

        if (getPath())
            fMemoryManager->deallocate(fPath);//delete [] fPath;

        fPath = XMLString::replicate(path, fMemoryManager);

    }
}

// ---------------------------------------------------------------------------
//  Components initialization
// ---------------------------------------------------------------------------

//
// authority     = server | reg_name
// server        = [ [ userinfo "@" ] hostport ]
// hostport      = host [ ":" port ]
//
// reg_name      = 1*( unreserved | escaped | "$" | "," |
//                    ";" | ":" | "@" | "&" | "=" | "+" )
//
// userinfo      = *( unreserved | escaped |
//                 ";" | ":" | "&" | "=" | "+" | "$" | "," )
//

void XMLUri::initializeAuthority(const XMLCh* const uriSpec)
{

    int index = 0;
    XMLSize_t start = 0;
    const XMLSize_t end = XMLString::stringLen(uriSpec);

    //
    // server = [ [ userinfo "@" ] hostport ]
    // userinfo is everything up @,
    //
    XMLCh* userinfo = (XMLCh*) fMemoryManager->allocate
    (
        (end+1) * sizeof(XMLCh)
    );//new XMLCh[end+1];
    ArrayJanitor<XMLCh> userName(userinfo, fMemoryManager);
    index = XMLString::indexOf(&(uriSpec[start]), chAt);

    if ( index != -1)
    {
        XMLString::subString(userinfo, &(uriSpec[start]), 0, index, fMemoryManager);
        index++; // skip the @
        start += index;
    }
    else
    {
        userinfo = 0;
    }

    //
    // hostport = host [ ":" port ]
    // host is everything up to ':', or up to
    // and including ']' if followed by ':'.
    //
    XMLCh* host = (XMLCh*) fMemoryManager->allocate
    (
        (end+1) * sizeof(XMLCh)
    );//new XMLCh[end+1];
    ArrayJanitor<XMLCh> hostName(host, fMemoryManager);

    // Search for port boundary.
    if (start < end && uriSpec[start] == chOpenSquare)
    {
    	index = XMLString::indexOf(&(uriSpec[start]), chCloseSquare);
    	if (index != -1)
    	{
            // skip the ']'
            index = ((start + index + 1) < end
              && uriSpec[start + index + 1] == chColon) ? index+1 : -1;
    	}
    }
    else
    {
        index = XMLString::indexOf(&(uriSpec[start]), chColon);
    }

    if ( index != -1 )
    {
        XMLString::subString(host, &(uriSpec[start]), 0, index, fMemoryManager);
        index++;  // skip the :
        start +=index;
    }
    else
    {
        XMLString::subString(host, &(uriSpec[start]), 0, end-start, fMemoryManager);
        start = end;
    }

    // port is everything after ":"

    XMLCh* portStr = (XMLCh*) fMemoryManager->allocate
    (
        (end+1) * sizeof(XMLCh)
    );//new XMLCh[end+1];
    ArrayJanitor<XMLCh> portName(portStr, fMemoryManager);
    int port = -1;

    if ((host && *host) &&   // non empty host
        (index != -1)                    &&   // ":" found
        (start < end)                     )   // ":" is not the last
    {
        XMLString::subString(portStr, &(uriSpec[start]), 0, end-start, fMemoryManager);

        if (portStr && *portStr)
        {
            port = XMLString::parseInt(portStr, fMemoryManager);
        }
    } // if > 0

    // Check if we have server based authority.
    if (isValidServerBasedAuthority(host, port, userinfo, fMemoryManager))
    {
        if (fHost)
            fMemoryManager->deallocate(fHost);//delete [] fHost;

        if (fUserInfo)
            fMemoryManager->deallocate(fUserInfo);//delete[] fUserInfo;

        fHost = XMLString::replicate(host, fMemoryManager);
        fPort = port;
        fUserInfo = XMLString::replicate(userinfo, fMemoryManager);

        return;
    }
    // This must be registry based authority or the URI is malformed.
    setRegBasedAuthority(uriSpec);
}

// scheme = alpha *( alpha | digit | "+" | "-" | "." )
void XMLUri::initializeScheme(const XMLCh* const uriSpec)
{
    const XMLCh* tmpPtr = XMLString::findAny(uriSpec, SCHEME_SEPARATORS);

    if ( !tmpPtr )
    {
        ThrowXMLwithMemMgr(MalformedURLException, XMLExcepts::XMLNUM_URI_No_Scheme, fMemoryManager);
    }
	else
    {
        XMLCh* scheme = (XMLCh*) fMemoryManager->allocate
        (
            (XMLString::stringLen(uriSpec) + 1) * sizeof(XMLCh)
        );//new XMLCh[XMLString::stringLen(uriSpec)+1];
        ArrayJanitor<XMLCh> tmpName(scheme, fMemoryManager);
        XMLString::subString(scheme, uriSpec, 0, (tmpPtr - uriSpec), fMemoryManager);
        setScheme(scheme);
	}

}

void XMLUri::initializePath(const XMLCh* const uriSpec)
{
    if ( !uriSpec )
    {
        ThrowXMLwithMemMgr1(MalformedURLException
                , XMLExcepts::XMLNUM_URI_Component_Empty
                , errMsg_PATH
                , fMemoryManager);
    }

    XMLSize_t index = 0;
    XMLSize_t start = 0;
    XMLSize_t end = XMLString::stringLen(uriSpec);
    XMLCh testChar = 0;

    // path - everything up to query string or fragment
    if (start < end)
    {
        // RFC 2732 only allows '[' and ']' to appear in the opaque part.
        if (!getScheme() || uriSpec[start] == chForwardSlash)
        {
            // Scan path.
            // abs_path = "/"  path_segments
            // rel_path = rel_segment [ abs_path ]
            while (index < end)
            {
                testChar = uriSpec[index];
                if (testChar == chQuestion || testChar == chPound)
                {
                    break;
                }

                // check for valid escape sequence
                if (testChar == chPercent)
                {
                    if (index+2 >= end ||
                        !XMLString::isHex(uriSpec[index+1]) ||
                        !XMLString::isHex(uriSpec[index+2]))
                    {
                        XMLCh value1[BUF_LEN+1];
                        XMLString::moveChars(value1, &(uriSpec[index]), 3);
                        value1[3] = chNull;
                        ThrowXMLwithMemMgr2(MalformedURLException
                                , XMLExcepts::XMLNUM_URI_Component_Invalid_EscapeSequence
                                , errMsg_PATH
                                , value1
                                , fMemoryManager);
                    }
                }
                else if (!isUnreservedCharacter(testChar) &&
                         !isPathCharacter(testChar))
                {
                    XMLCh value1[BUF_LEN+1];
                    value1[0] = testChar;
                    value1[1] = chNull;
                    ThrowXMLwithMemMgr2(MalformedURLException
                            , XMLExcepts::XMLNUM_URI_Component_Invalid_Char
                            , errMsg_PATH
                            , value1
                            , fMemoryManager);
                }

                index++;
            }//while (index < end)
        }
        else
        {
            // Scan opaque part.
            // opaque_part = uric_no_slash *uric
            while (index < end)
            {
                testChar = uriSpec[index];
                if (testChar == chQuestion || testChar == chPound)
                {
                    break;
                }

                // check for valid escape sequence
                if (testChar == chPercent)
                {
                    if (index+2 >= end ||
                        !XMLString::isHex(uriSpec[index+1]) ||
                        !XMLString::isHex(uriSpec[index+2]))
                    {
                        XMLCh value1[BUF_LEN+1];
                        XMLString::moveChars(value1, &(uriSpec[index]), 3);
                        value1[3] = chNull;
                        ThrowXMLwithMemMgr2(MalformedURLException
                                , XMLExcepts::XMLNUM_URI_Component_Invalid_EscapeSequence
                                , errMsg_PATH
                                , value1
                                , fMemoryManager);
                    }
                }
                // If the scheme specific part is opaque, it can contain '['
                // and ']'. uric_no_slash wasn't modified by RFC 2732, which
                // I've interpreted as an error in the spec, since the
                // production should be equivalent to (uric - '/'), and uric
                // contains '[' and ']'.
                else if (!isReservedOrUnreservedCharacter(testChar))
                {
                    XMLCh value1[BUF_LEN+1];
                    value1[0] = testChar;
                    value1[1] = chNull;
                    ThrowXMLwithMemMgr2(MalformedURLException
                            , XMLExcepts::XMLNUM_URI_Component_Invalid_Char
                            , errMsg_PATH
                            , value1
                            , fMemoryManager);
                }

                index++;
            }//while (index < end)
        }
    } //if (start < end)

    if (getPath())
    {
        fMemoryManager->deallocate(fPath);//delete [] fPath;
    }

    fPath = (XMLCh*) fMemoryManager->allocate((index+1) * sizeof(XMLCh));//new XMLCh[index+1];
    XMLString::subString(fPath, uriSpec, start, index, fMemoryManager);

    // query - starts with ? and up to fragment or end
    if (testChar == chQuestion)
    {
        index++;
        start = index;
        while (index < end)
        {
            testChar = uriSpec[index];
            if (testChar == chPound)
            {
                break;
            }

            if (testChar == chPercent)
            {
                if (index+2 >= end ||
                    !XMLString::isHex(uriSpec[index+1]) ||
                    !XMLString::isHex(uriSpec[index+2]))
                {
                    XMLCh value1[BUF_LEN+1];
                    XMLString::moveChars(value1, &(uriSpec[index]), 3);
                    value1[3] = chNull;
                    ThrowXMLwithMemMgr2(MalformedURLException
                            , XMLExcepts::XMLNUM_URI_Component_Invalid_EscapeSequence
                            , errMsg_QUERY
                            , value1
                            , fMemoryManager);
                }
            }
            else if (!isReservedOrUnreservedCharacter(testChar))
            {
                XMLCh value1[BUF_LEN+1];
                value1[0] = testChar;
                value1[1] = chNull;
                ThrowXMLwithMemMgr2(MalformedURLException
                        , XMLExcepts::XMLNUM_URI_Component_Invalid_Char
                        , errMsg_QUERY
                        , value1
                        , fMemoryManager);
            }
            index++;
        }

        if (getQueryString())
        {
            fMemoryManager->deallocate(fQueryString);//delete [] fQueryString;
        }

        fQueryString = (XMLCh*) fMemoryManager->allocate
        (
            (index - start + 1) * sizeof(XMLCh)
        );//new XMLCh[index - start + 1];
        XMLString::subString(fQueryString, uriSpec, start, index, fMemoryManager);
    }

    // fragment - starts with #
    if (testChar == chPound)
    {
        index++;
        start = index;
        while (index < end)
        {
            testChar = uriSpec[index];

            if (testChar == chPercent)
            {
                if (index+2 >= end ||
                    !XMLString::isHex(uriSpec[index+1]) ||
                    !XMLString::isHex(uriSpec[index+2]))
                {
                    XMLCh value1[BUF_LEN+1];
                    XMLString::moveChars(value1, &(uriSpec[index]), 3);
                    value1[3] = chNull;
                    ThrowXMLwithMemMgr2(MalformedURLException
                            , XMLExcepts::XMLNUM_URI_Component_Invalid_EscapeSequence
                            , errMsg_FRAGMENT
                            , value1
                            , fMemoryManager);
                }
            }
            else if (!isReservedOrUnreservedCharacter(testChar))
            {
                XMLCh value1[BUF_LEN+1];
                value1[0] = testChar;
                value1[1] = chNull;
                ThrowXMLwithMemMgr2(MalformedURLException
                        , XMLExcepts::XMLNUM_URI_Component_Invalid_Char
                        , errMsg_FRAGMENT
                        , value1
                        , fMemoryManager);
            }

            index++;

        }

        if (getFragment())
            fMemoryManager->deallocate(fFragment);//delete [] fFragment;

        //make sure that there is something following the '#'
        if (index > start)
        {
            fFragment = (XMLCh*) fMemoryManager->allocate
            (
                (index - start + 1) * sizeof(XMLCh)
            );//new XMLCh[index - start + 1];
            XMLString::subString(fFragment, uriSpec, start, index, fMemoryManager);
        }
        else
        {
            // RFC 2396, 4.0. URI Reference
            // URI-reference = [absoulteURI | relativeURI] [# fragment]
            //
            // RFC 2396, 4.1. Fragment Identifier
            // fragment = *uric
            //
            // empty fragment is valid
            fFragment = 0;
        }
    }

}

// ---------------------------------------------------------------------------
//  Setter
// ---------------------------------------------------------------------------
void XMLUri::setScheme(const XMLCh* const newScheme)
{
    if ( !newScheme )
    {
        ThrowXMLwithMemMgr1(MalformedURLException
                , XMLExcepts::XMLNUM_URI_Component_Set_Null
                , errMsg_SCHEME
                , fMemoryManager);
    }

    if (!isConformantSchemeName(newScheme))
    {
        ThrowXMLwithMemMgr2(MalformedURLException
                , XMLExcepts::XMLNUM_URI_Component_Not_Conformant
                , errMsg_SCHEME
                , newScheme
                , fMemoryManager);
    }

    if (getScheme())
    {
        fMemoryManager->deallocate(fScheme);//delete [] fScheme;
    }

    fScheme = XMLString::replicate(newScheme, fMemoryManager);
    XMLString::lowerCase(fScheme);
}

//
// server = [ [ userinfo "@" ] hostport ]
// hostport = host [":" port]
//
// setUserInfo(), setHost() and setPort() are closely related
// three methods, in a word, userinfo and port has dependency
// on host.
//
// if host is not present, userinfo must be null and port = -1
//
void XMLUri::setUserInfo(const XMLCh* const newUserInfo)
{
    if ( newUserInfo &&
         !getHost()    )
    {
        ThrowXMLwithMemMgr2(MalformedURLException
                , XMLExcepts::XMLNUM_URI_NullHost
                , errMsg_USERINFO
                , newUserInfo
                , fMemoryManager);
    }

    isConformantUserInfo(newUserInfo, fMemoryManager);

    if (getUserInfo())
    {
        fMemoryManager->deallocate(fUserInfo);//delete [] fUserInfo;
    }

    //sometimes we get passed a empty string rather than a null.
    //Other procedures rely on it being null
    if(newUserInfo && *newUserInfo) {
        fUserInfo = XMLString::replicate(newUserInfo, fMemoryManager);
    }
    else
        fUserInfo = 0;

}

void XMLUri::setHost(const XMLCh* const newHost)
{
    if ( !newHost )
    {
        if (getHost())
            fMemoryManager->deallocate(fHost);//delete [] fHost;

        fHost = 0;
        setUserInfo(0);
        setPort(-1);

        return;
    }

    if ( *newHost && !isWellFormedAddress(newHost, fMemoryManager))
    {
        ThrowXMLwithMemMgr2(MalformedURLException
                , XMLExcepts::XMLNUM_URI_Component_Not_Conformant
                , errMsg_HOST
                , newHost
                , fMemoryManager);
    }

    if (getHost())
    {
        fMemoryManager->deallocate(fHost);//delete [] fHost;
    }

    fHost = XMLString::replicate(newHost, fMemoryManager);
    setRegBasedAuthority(0);
}

void XMLUri::setPort(int newPort)
{
    if (newPort >= 0 && newPort <= 65535)
    {
        if (!getHost())
        {
            XMLCh value1[BUF_LEN+1];
            XMLString::binToText(newPort, value1, BUF_LEN, 10, fMemoryManager);
            ThrowXMLwithMemMgr2(MalformedURLException
                    , XMLExcepts::XMLNUM_URI_NullHost
                    , errMsg_PORT
                    , value1
                    , fMemoryManager);
        }
    }
    else if (newPort != -1)
    {
        XMLCh value1[BUF_LEN+1];
        XMLString::binToText(newPort, value1, BUF_LEN, 10, fMemoryManager);
        ThrowXMLwithMemMgr1(MalformedURLException
                , XMLExcepts::XMLNUM_URI_PortNo_Invalid
                , value1
                , fMemoryManager);
    }

    fPort = newPort;
}

void XMLUri::setRegBasedAuthority(const XMLCh* const newRegAuth)
{
    if ( !newRegAuth )
    {
        if (getRegBasedAuthority())
            fMemoryManager->deallocate(fRegAuth);//delete [] fRegAuth;

        fRegAuth = 0;
        return;
    }
    // reg_name = 1*( unreserved | escaped | "$" | "," |
    //            ";" | ":" | "@" | "&" | "=" | "+" )
    else if ( !*newRegAuth || !isValidRegistryBasedAuthority(newRegAuth) )
    {
        ThrowXMLwithMemMgr2(MalformedURLException
                , XMLExcepts::XMLNUM_URI_Component_Not_Conformant
                , errMsg_REGNAME
                , newRegAuth
                , fMemoryManager);
    }

    if (getRegBasedAuthority())
        fMemoryManager->deallocate(fRegAuth);//delete [] fRegAuth;

    fRegAuth = XMLString::replicate(newRegAuth, fMemoryManager);
    setHost(0);
}

//
// setPath(), setQueryString() and setFragment() are closely
// related three methods as well.
//
void XMLUri::setPath(const XMLCh* const newPath)
{
    if (!newPath)
    {
        if (getPath())
            fMemoryManager->deallocate(fPath);//delete [] fPath;

        fPath = 0;
        setQueryString(0);
        setFragment(0);
    }
    else
    {
        initializePath(newPath);
    }
}

//
// fragment = *uric
//
void XMLUri::setFragment(const XMLCh* const newFragment)
{
	if ( !newFragment )
    {
        if (getFragment())
            fMemoryManager->deallocate(fFragment);//delete [] fFragment;

        fFragment = 0;
	}
	else if (!isGenericURI())
    {
        ThrowXMLwithMemMgr2(MalformedURLException
                , XMLExcepts::XMLNUM_URI_Component_for_GenURI_Only
                , errMsg_FRAGMENT
                , newFragment
                , fMemoryManager);
	}
	else if ( !getPath() )
    {
        ThrowXMLwithMemMgr2(MalformedURLException
               , XMLExcepts::XMLNUM_URI_NullPath
               , errMsg_FRAGMENT
               , newFragment
               , fMemoryManager);
	}
	else if (!isURIString(newFragment))
    {
        ThrowXMLwithMemMgr1(MalformedURLException
                , XMLExcepts::XMLNUM_URI_Component_Invalid_Char
                , errMsg_FRAGMENT
                , fMemoryManager);
	}
	else
    {
        if (getFragment())
        {
            fMemoryManager->deallocate(fFragment);//delete [] fFragment;
        }

        fFragment = XMLString::replicate(newFragment, fMemoryManager);
	}
}

//
// query = *uric
//
void XMLUri::setQueryString(const XMLCh* const newQueryString)
{
	if ( !newQueryString )
    {
        if (getQueryString())
            fMemoryManager->deallocate(fQueryString);//delete [] fQueryString;

        fQueryString = 0;
	}
	else if (!isGenericURI())
    {
        ThrowXMLwithMemMgr2(MalformedURLException
                , XMLExcepts::XMLNUM_URI_Component_for_GenURI_Only
                , errMsg_QUERY
                , newQueryString
                , fMemoryManager);
	}
	else if ( !getPath() )
    {
        ThrowXMLwithMemMgr2(MalformedURLException
                , XMLExcepts::XMLNUM_URI_NullPath
                , errMsg_QUERY
                , newQueryString
                , fMemoryManager);
	}
	else if (!isURIString(newQueryString))
    {
        ThrowXMLwithMemMgr2(MalformedURLException
               , XMLExcepts::XMLNUM_URI_Component_Invalid_Char
               , errMsg_QUERY
               , newQueryString
               , fMemoryManager);
	}
	else
    {
        if (getQueryString())
        {
            fMemoryManager->deallocate(fQueryString);//delete [] fQueryString;
        }

        fQueryString = XMLString::replicate(newQueryString, fMemoryManager);
	}
}

// ---------------------------------------------------------------------------
//  XMLUri: Public, static methods
// ---------------------------------------------------------------------------

//
//  scheme = alpha *( alpha | digit | "+" | "-" | "." )
//  alphanum = alpha | digit
//
bool XMLUri::isConformantSchemeName(const XMLCh* const scheme)
{
	if ( !scheme )
        return false;

    const XMLCh* tmpStr = scheme;
    if (!XMLString::isAlpha(*tmpStr))     // first: alpha
        return false;

    // second onwards: ( alpha | digit | "+" | "-" | "." )
    tmpStr++;
    while (*tmpStr)
    {
        if ( !XMLString::isAlphaNum(*tmpStr) &&
             (XMLString::indexOf(SCHEME_CHARACTERS, *tmpStr) == -1))
            return false;

        tmpStr++;
    }

    return true;
}

//
// userinfo = *( unreserved | escaped |
//              ";" | ":" | "&" | "=" | "+" | "$" | "," )
//
void XMLUri::isConformantUserInfo(const XMLCh* const userInfo
                                  , MemoryManager* const manager)
{
	if ( !userInfo )
        return;

    const XMLCh* tmpStr = userInfo;
    while (*tmpStr)
    {
        if ( isUnreservedCharacter(*tmpStr) ||
            (XMLString::indexOf(USERINFO_CHARACTERS, *tmpStr) != -1))
        {
            tmpStr++;
        }
        else if (*tmpStr == chPercent)               // '%'
        {
            if (XMLString::isHex(*(tmpStr+1)) &&     // 1st hex
                XMLString::isHex(*(tmpStr+2))  )     // 2nd hex
            {
                tmpStr+=3;
            }
            else
            {
                XMLCh value1[BUF_LEN+1];
                value1[0] = chPercent;
                value1[1] = *(tmpStr+1);
                value1[2] = *(tmpStr+2);
                value1[3] = chNull;

                ThrowXMLwithMemMgr2(MalformedURLException
                        , XMLExcepts::XMLNUM_URI_Component_Invalid_EscapeSequence
                        , errMsg_USERINFO
                        , value1
                        , manager);
            }
        }
        else
        {
            ThrowXMLwithMemMgr2(MalformedURLException
                    , XMLExcepts::XMLNUM_URI_Component_Invalid_Char
                    , errMsg_USERINFO
                    , userInfo
                    , manager);
        }
    } //while

    return;
}

bool XMLUri::isValidServerBasedAuthority(const XMLCh* const host,
                                         const XMLSize_t hostLen,
                                         const int port,
                                         const XMLCh* const userinfo,
                                         const XMLSize_t userLen)
{
    // The order is important, do not change
    if (!isWellFormedAddress(host, hostLen))
        return false;

    // check port number
    if ((port > 65535) || (port < 0 && port != -1))
        return false;

    // check userinfo
    XMLSize_t index = 0;
    while (index < userLen)
    {
        if (isUnreservedCharacter(userinfo[index]) ||
            (XMLString::indexOf(USERINFO_CHARACTERS, userinfo[index]) != -1))
        {
            index++;
        }
        else if (userinfo[index] == chPercent)               // '%'
        {
            if (XMLString::isHex(userinfo[index+1]) &&     // 1st hex
                XMLString::isHex(userinfo[index+2])  )     // 2nd hex
                index +=3;
            else
                return false;
        }
        else
            return false;
    } //while

    return true;
}

bool XMLUri::isValidServerBasedAuthority(const XMLCh* const host
                                         , const int port
                                         , const XMLCh* const userinfo
                                         , MemoryManager* const manager)
{
    // The order is important, do not change
    if (!isWellFormedAddress(host, manager))
        return false;

    // check port number
    if ((port > 65535) || (port < 0 && port != -1))
        return false;

    // check userinfo
    if (!userinfo)
        return true;

    const XMLCh* tmpStr = userinfo;
    while (*tmpStr)
    {
        if ( isUnreservedCharacter(*tmpStr) ||
            (XMLString::indexOf(USERINFO_CHARACTERS, *tmpStr) != -1))
        {
            tmpStr++;
        }
        else if (*tmpStr == chPercent)               // '%'
        {
            if (XMLString::isHex(*(tmpStr+1)) &&     // 1st hex
                XMLString::isHex(*(tmpStr+2))  )     // 2nd hex
            {
                tmpStr+=3;
            }
            else
                return false;
        }
        else
            return false;
    } //while

    return true;
}

bool XMLUri::isValidRegistryBasedAuthority(const XMLCh* const authority,
                                           const XMLSize_t authLen)
{
    // check authority
    XMLSize_t index = 0;
    while (index < authLen)
    {
        if (isUnreservedCharacter(authority[index]) ||
            (XMLString::indexOf(REG_NAME_CHARACTERS, authority[index]) != -1))
        {
            index++;
        }
        else if (authority[index] == chPercent)               // '%'
        {
            if (XMLString::isHex(authority[index+1]) &&     // 1st hex
                XMLString::isHex(authority[index+2])  )     // 2nd hex
                index +=3;
            else
                return false;
        }
        else
            return false;
    } //while

    return true;
}

bool XMLUri::isValidRegistryBasedAuthority(const XMLCh* const authority)
{
    // check authority
    if (!authority)
        return false;

    const XMLCh* tmpStr = authority;
    while (*tmpStr)
    {
        if (isUnreservedCharacter(*tmpStr) ||
            (XMLString::indexOf(REG_NAME_CHARACTERS, *tmpStr) != -1))
        {
            tmpStr++;
        }
        else if (*tmpStr == chPercent)               // '%'
        {
            if (XMLString::isHex(*(tmpStr+1)) &&     // 1st hex
                XMLString::isHex(*(tmpStr+2))  )     // 2nd hex
            {
                tmpStr+=3;
            }
            else
                return false;
        }
        else
            return false;
    } //while

    return true;
}

//
// uric     = reserved | unreserved | escaped
// escaped  = "%" hex hex
// hex      = digit | "A" | "B" | "C" | "D" | "E" | "F" |
//                    "a" | "b" | "c" | "d" | "e" | "f"
//
bool XMLUri::isURIString(const XMLCh* const uricString)
{
	if (!uricString || !*uricString)
        return false;

    const XMLCh* tmpStr = uricString;

    while (*tmpStr)
    {
        if (isReservedOrUnreservedCharacter(*tmpStr))
        {
            tmpStr++;
        }
        else if (*tmpStr == chPercent)               // '%'
        {
            if (XMLString::isHex(*(tmpStr+1)) &&     // 1st hex
                XMLString::isHex(*(tmpStr+2))  )     // 2nd hex
            {
                tmpStr+=3;
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }

    return true;
}

//
//  host          = hostname | IPv4address
//
//  hostname      = *( domainlabel "." ) toplabel [ "." ]
//  domainlabel   = alphanum | alphanum *( alphanum | "-" ) alphanum
//  toplabel      = alpha | alpha *( alphanum | "-" ) alphanum
//
//  IPv4address   = 1*3DIGIT "." 1*3DIGIT "." 1*3DIGIT "." 1*3DIGIT
//
bool XMLUri::isWellFormedAddress(const XMLCh* const addrString
                                 , MemoryManager* const manager)
{
    // Check that we have a non-zero length string.
    if (!addrString || !*addrString)
        return false;

    // Get address length.
    XMLSize_t addrStrLen = XMLString::stringLen(addrString);

    // Check if the host is a valid IPv6reference.
    if (*addrString == chOpenSquare)
    {
        return isWellFormedIPv6Reference(addrString, addrStrLen);
    }

    //
    // Cannot start with a '.', '-', or end with a '-'.
    //
    if (*addrString == chPeriod ||
        *addrString == chDash ||
        addrString[addrStrLen-1] == chDash)
        return false;

    // rightmost domain label starting with digit indicates IP address
    // since top level domain label can only start with an alpha
    // see RFC 2396 Section 3.2.2

    int lastPeriodPos = XMLString::lastIndexOf(addrString, chPeriod);

    // if the string ends with "."
    // get the second last "."
    if (XMLSize_t(lastPeriodPos + 1) == addrStrLen)
    {
        XMLCh* tmp2 = (XMLCh*) manager->allocate
        (
            addrStrLen * sizeof(XMLCh)
        );//new XMLCh[addrStrLen];
        XMLString::subString(tmp2, addrString, 0, lastPeriodPos, manager);
        lastPeriodPos = XMLString::lastIndexOf(tmp2, chPeriod);
        manager->deallocate(tmp2);//delete [] tmp2;

        if ( XMLString::isDigit(addrString[lastPeriodPos + 1]))
			return false;
    }

    if (XMLString::isDigit(addrString[lastPeriodPos + 1]))
    {
        return isWellFormedIPv4Address(addrString, addrStrLen);
    } // end of IPv4address
    else
    {
        //
        //  hostname      = *( domainlabel "." ) toplabel [ "." ]
        //  domainlabel   = alphanum | alphanum *( alphanum | "-" ) alphanum
        //  toplabel      = alpha | alpha *( alphanum | "-" ) alphanum

        // RFC 2396 states that hostnames take the form described in
        // RFC 1034 (Section 3) and RFC 1123 (Section 2.1). According
        // to RFC 1034, hostnames are limited to 255 characters.
        if (addrStrLen > 255) {
            return false;
        }

        unsigned int labelCharCount = 0;

        // domain labels can contain alphanumerics and '-"
        // but must start and end with an alphanumeric
        for (XMLSize_t i = 0; i < addrStrLen; i++)
        {
            if (addrString[i] == chPeriod)
            {
              if (((i > 0)  &&
                   (!XMLString::isAlphaNum(addrString[i-1]))) ||
                  ((i + 1 < addrStrLen) &&
                   (!XMLString::isAlphaNum(addrString[i+1])))  )
                {
                    return false;
                }
                labelCharCount = 0;
            }
            else if (!XMLString::isAlphaNum(addrString[i]) &&
                      addrString[i] != chDash)
            {
                return false;
            }
            // RFC 1034: Labels must be 63 characters or less.
            else if (++labelCharCount > 63) {
                return false;
            }
        } //for
    }

    return true;
}

//
//  RFC 2732 amended RFC 2396 by replacing the definition
//  of IPv4address with the one defined by RFC 2373.
//
//  IPv4address   = 1*3DIGIT "." 1*3DIGIT "." 1*3DIGIT "." 1*3DIGIT
//
bool XMLUri::isWellFormedIPv4Address(const XMLCh* const addr, const XMLSize_t length)
{
    int numDots = 0;
    int numDigits = 0;

    // IPv4address = 1*3DIGIT "." 1*3DIGIT "." 1*3DIGIT "." 1*3DIGIT
    //
    // make sure that
    // 1) we see only digits and dot separators,
    // 2) that any dot separator is preceded and followed by a digit
    // 3) that we find 3 dots
    // 4) that each segment contains 1 to 3 digits.
    // 5) that each segment is not greater than 255.
    for (XMLSize_t i = 0; i < length; ++i)
    {
        if (addr[i] == chPeriod)
        {
            if ((i == 0) ||
                (i+1 == length) ||
                !XMLString::isDigit(addr[i+1]))
            {
               return false;
            }
            numDigits = 0;
            if (++numDots > 3)
                return false;
        }
        else if (!XMLString::isDigit(addr[i]))
        {
            return false;
        }
        // Check that that there are no more than three digits
        // in this segment.
        else if (++numDigits > 3)
        {
            return false;
        }
        // Check that this segment is not greater than 255.
        else if (numDigits == 3)
        {
            XMLCh first = addr[i-2];
            XMLCh second = addr[i-1];
            XMLCh last = addr[i];
            if (!(first < chDigit_2 ||
                 (first == chDigit_2 &&
                 (second < chDigit_5 ||
                 (second == chDigit_5 && last <= chDigit_5)))))
            {
                return false;
            }
        }
    } //for
    return (numDots == 3);
}

//
//  IPv6reference = "[" IPv6address "]"
//
bool XMLUri::isWellFormedIPv6Reference(const XMLCh* const addr, const XMLSize_t length)
{
    XMLSize_t end = length-1;

    // Check if string is a potential match for IPv6reference.
    if (!(length > 2 && addr[0] == chOpenSquare && addr[end] == chCloseSquare))
    {
        return false;
    }

    // Counter for the number of 16-bit sections read in the address.
    int counter = 0;

    // Scan hex sequence before possible '::' or IPv4 address.
    int iIndex = scanHexSequence(addr, 1, end, counter);
    if (iIndex == -1)
        return false;

    XMLSize_t index=(XMLSize_t)iIndex;
    // Address must contain 128-bits of information.
    if (index == end)
    {
       return (counter == 8);
    }

    if (index+1 < end && addr[index] == chColon)
    {
        if (addr[index+1] == chColon)
        {
            // '::' represents at least one 16-bit group of zeros.
            if (++counter > 8)
            {
                return false;
            }
            index += 2;
            // Trailing zeros will fill out the rest of the address.
            if (index == end)
            {
                return true;
            }
        }
        // If the second character wasn't ':', in order to be valid,
        // the remainder of the string must match IPv4Address,
        // and we must have read exactly 6 16-bit groups.
        else
        {
            if (counter == 6)
                return isWellFormedIPv4Address(addr+index+1, end-index-1);
            else
                return false;
        }
    }
    else
    {
       return false;
    }

    // 3. Scan hex sequence after '::'.
    int prevCount = counter;
    iIndex = scanHexSequence(addr, index, end, counter);
    if (iIndex == -1)
        return false;

    index=(XMLSize_t)iIndex;
    // If this is the end of the address then
    // we've got 128-bits of information.
    if (index == end)
    {
        return true;
    }

    // The address ends in an IPv4 address, or it is invalid.
    // scanHexSequence has already made sure that we have the right number of bits.
    XMLSize_t shiftCount = (counter > prevCount) ? index+1 : index;
    return isWellFormedIPv4Address(addr + shiftCount, end - shiftCount);
}

//
//  For use with isWellFormedIPv6Reference only.
//
int XMLUri::scanHexSequence (const XMLCh* const addr, XMLSize_t index, XMLSize_t end, int& counter)
{
    XMLCh testChar = chNull;
    int numDigits = 0;
    XMLSize_t start = index;

    // Trying to match the following productions:
    // hexseq = hex4 *( ":" hex4)
    // hex4   = 1*4HEXDIG
    for (; index < end; ++index)
    {
      	testChar = addr[index];
      	if (testChar == chColon)
      	{
      	    // IPv6 addresses are 128-bit, so there can be at most eight sections.
      	    if (numDigits > 0 && ++counter > 8)
      	    {
      	        return -1;
      	    }
      	    // This could be '::'.
      	    if (numDigits == 0 || ((index+1 < end) && addr[index+1] == chColon))
      	    {
      	        return (int)index;
      	    }
      	    numDigits = 0;
        }
        // This might be invalid or an IPv4address. If it's potentially an IPv4address,
        // backup to just after the last valid character that matches hexseq.
        else if (!XMLString::isHex(testChar))
        {
            if (testChar == chPeriod && numDigits < 4 && numDigits > 0 && counter <= 6)
            {
                int back = (int)index - numDigits - 1;
                return (back >= (int)start) ? back : (int)start;
            }
            return -1;
        }
        // There can be at most 4 hex digits per group.
        else if (++numDigits > 4)
        {
            return -1;
        }
    }
    return (numDigits > 0 && ++counter <= 8) ? (int)end : -1;
}

bool XMLUri::isGenericURI()
{
    return (getHost() != 0);
}


//
//  This method will take the broken out parts of the URI and build up the
//  full text. We don't do this unless someone asks us to, since its often
//  never required.
//
void XMLUri::buildFullText()
{
    // Calculate the worst case size of the buffer required
    XMLSize_t bufSize = XMLString::stringLen(fScheme) + 1
                           + XMLString::stringLen(fFragment) + 1
                           + XMLString::stringLen(fHost ? fHost : fRegAuth) + 2
                           + XMLString::stringLen(fPath)
                           + XMLString::stringLen(fQueryString) + 1
                           + XMLString::stringLen(fUserInfo) + 1
                           + 32;

    // Clean up the existing buffer and allocate another
    fMemoryManager->deallocate(fURIText);//delete [] fURIText;
    fURIText = (XMLCh*) fMemoryManager->allocate(bufSize * sizeof(XMLCh));//new XMLCh[bufSize];
    *fURIText = 0;

    XMLCh* outPtr = fURIText;
    if (fScheme != 0)
    {
        XMLString::catString(fURIText, getScheme());
        outPtr += XMLString::stringLen(fURIText);
        *outPtr++ = chColon;
    }

    // Authority
    if (fHost || fRegAuth)
    {
        *outPtr++ = chForwardSlash;
        *outPtr++ = chForwardSlash;

        // Server based authority.
        if (fHost)
        {
            if (fUserInfo)
            {
                XMLString::copyString(outPtr, fUserInfo);
                outPtr += XMLString::stringLen(fUserInfo);
                *outPtr++ = chAt;
            }

            XMLString::copyString(outPtr, fHost);
            outPtr += XMLString::stringLen(fHost);

            //
            //  If the port is -1, then we don't put it in. Else we need
            //  to because it was explicitly provided.
            //
            if (fPort != -1)
            {
                *outPtr++ = chColon;

                XMLCh tmpBuf[17];
                XMLString::binToText(fPort, tmpBuf, 16, 10, fMemoryManager);
                XMLString::copyString(outPtr, tmpBuf);
                outPtr += XMLString::stringLen(tmpBuf);
            }
        }
        // Registry based authority.
        else {
            XMLString::copyString(outPtr, fRegAuth);
            outPtr += XMLString::stringLen(fRegAuth);
        }
    }

    if (fPath)
    {
        XMLString::copyString(outPtr, fPath);
        outPtr += XMLString::stringLen(fPath);
    }

    if (fQueryString)
    {
        *outPtr++ = chQuestion;
        XMLString::copyString(outPtr, fQueryString);
        outPtr += XMLString::stringLen(fQueryString);
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

// NOTE: no check for NULL value of uriStr (caller responsiblilty)
bool XMLUri::isValidURI(const XMLUri* const baseURI
                       , const XMLCh* const uriStr
                       , bool bAllowSpaces/*=false*/)
{
    // get a trimmed version of uriStr
    // uriStr will NO LONGER be used in this function.
    const XMLCh* trimmedUriSpec = uriStr;

    while (XMLChar1_0::isWhitespace(*trimmedUriSpec))
        trimmedUriSpec++;

    XMLSize_t trimmedUriSpecLen = XMLString::stringLen(trimmedUriSpec);

    while (trimmedUriSpecLen) {
        if (XMLChar1_0::isWhitespace(trimmedUriSpec[trimmedUriSpecLen-1]))
            trimmedUriSpecLen--;
        else
            break;
    }

    if (trimmedUriSpecLen == 0)
    {
        if (!baseURI)
            return false;
        else
            return true;
    }

    XMLSize_t index = 0;
    bool foundScheme = false;

    // Check for scheme, which must be before `/', '?' or '#'.
    int colonIdx = XMLString::indexOf(trimmedUriSpec, chColon);
    int slashIdx = XMLString::indexOf(trimmedUriSpec, chForwardSlash);
    int queryIdx = XMLString::indexOf(trimmedUriSpec, chQuestion);
    int fragmentIdx = XMLString::indexOf(trimmedUriSpec, chPound);

    if ((colonIdx <= 0) ||
        (colonIdx > slashIdx && slashIdx != -1) ||
        (colonIdx > queryIdx && queryIdx != -1) ||
        (colonIdx > fragmentIdx && fragmentIdx != -1))
    {
        // A standalone base is a valid URI according to spec
        if (colonIdx == 0 || (!baseURI && fragmentIdx != 0))
            return false;
    }
    else
    {
        if (!processScheme(trimmedUriSpec, index))
            return false;
        foundScheme = true;
        ++index;
    }

    // It's an error if we stop here
    if (index == trimmedUriSpecLen || (foundScheme && (trimmedUriSpec[index] == chPound)))
        return false;

	// two slashes means generic URI syntax, so we get the authority
    const XMLCh* authUriSpec = trimmedUriSpec +  index;
    if (((index+1) < trimmedUriSpecLen) &&
        XMLString::startsWith(authUriSpec, DOUBLE_SLASH))
    {
        index += 2;
        XMLSize_t startPos = index;

        // get authority - everything up to path, query or fragment
        XMLCh testChar;
        while (index < trimmedUriSpecLen)
        {
            testChar = trimmedUriSpec[index];
            if (testChar == chForwardSlash ||
                testChar == chQuestion     ||
                testChar == chPound         )
            {
                break;
            }

            index++;
        }

        // if we found authority, parse it out, otherwise we set the
        // host to empty string
        if (index > startPos)
        {
            if (!processAuthority(trimmedUriSpec + startPos, index - startPos))
                return false;
        }
    }

    // we need to check if index has exceed the lenght or not
    if (index < trimmedUriSpecLen)
    {
	    if (!processPath(trimmedUriSpec + index, trimmedUriSpecLen - index, foundScheme, bAllowSpaces))
            return false;
    }

    return true;
}

// NOTE: no check for NULL value of uriStr (caller responsiblilty)
// NOTE: this routine is the same as above, but it uses a flag to
//       indicate the existance of a baseURI rather than an XMLuri.
bool XMLUri::isValidURI(bool haveBaseURI, const XMLCh* const uriStr, bool bAllowSpaces/*=false*/)
{
    // get a trimmed version of uriStr
    // uriStr will NO LONGER be used in this function.
    const XMLCh* trimmedUriSpec = uriStr;

    while (XMLChar1_0::isWhitespace(*trimmedUriSpec))
        trimmedUriSpec++;

    XMLSize_t trimmedUriSpecLen = XMLString::stringLen(trimmedUriSpec);

    while (trimmedUriSpecLen) {
        if (XMLChar1_0::isWhitespace(trimmedUriSpec[trimmedUriSpecLen-1]))
            trimmedUriSpecLen--;
        else
            break;
    }

    if (trimmedUriSpecLen == 0)
    {
        if (!haveBaseURI)
            return false;
        return true;
    }

    XMLSize_t index = 0;
    bool foundScheme = false;

    // Check for scheme, which must be before `/', '?' or '#'.
    int colonIdx = XMLString::indexOf(trimmedUriSpec, chColon);
    int slashIdx = XMLString::indexOf(trimmedUriSpec, chForwardSlash);
    int queryIdx = XMLString::indexOf(trimmedUriSpec, chQuestion);
    int fragmentIdx = XMLString::indexOf(trimmedUriSpec, chPound);

    if ((colonIdx <= 0) ||
        (colonIdx > slashIdx && slashIdx != -1) ||
        (colonIdx > queryIdx && queryIdx != -1) ||
        (colonIdx > fragmentIdx && fragmentIdx != -1))
    {
        // A standalone base is a valid URI according to spec
        if (colonIdx == 0 || (!haveBaseURI && fragmentIdx != 0))
            return false;
    }
    else
    {
        if (!processScheme(trimmedUriSpec, index))
            return false;
        foundScheme = true;
        ++index;
    }

    // It's an error if we stop here
    if (index == trimmedUriSpecLen || (foundScheme && (trimmedUriSpec[index] == chPound)))
        return false;

	// two slashes means generic URI syntax, so we get the authority
    const XMLCh* authUriSpec = trimmedUriSpec +  index;
    if (((index+1) < trimmedUriSpecLen) &&
        XMLString::startsWith(authUriSpec, DOUBLE_SLASH))
    {
        index += 2;
        XMLSize_t startPos = index;

        // get authority - everything up to path, query or fragment
        XMLCh testChar;
        while (index < trimmedUriSpecLen)
        {
            testChar = trimmedUriSpec[index];
            if (testChar == chForwardSlash ||
                testChar == chQuestion     ||
                testChar == chPound         )
            {
                break;
            }

            index++;
        }

        // if we found authority, parse it out, otherwise we set the
        // host to empty string
        if (index > startPos)
        {
            if (!processAuthority(trimmedUriSpec + startPos, index - startPos))
                return false;
        }
    }

    // we need to check if index has exceed the length or not
    if (index < trimmedUriSpecLen)
    {
        if (!processPath(trimmedUriSpec + index, trimmedUriSpecLen - index, foundScheme, bAllowSpaces))
            return false;
    }

    return true;
}

bool XMLUri::isWellFormedAddress(const XMLCh* const addrString,
                                 const XMLSize_t addrStrLen)
{
    // Check that we have a non-zero length string.
    if (addrStrLen == 0)
        return false;

    // Check if the host is a valid IPv6reference.
    if (*addrString == chOpenSquare)
    {
        return isWellFormedIPv6Reference(addrString, addrStrLen);
    }

    //
    // Cannot start with a '.', '-', or end with a '-'.
    //
    if (*addrString == chPeriod ||
        *addrString == chDash ||
        addrString[addrStrLen-1] == chDash)
        return false;

    // rightmost domain label starting with digit indicates IP address
    // since top level domain label can only start with an alpha
    // see RFC 2396 Section 3.2.2

    int lastPeriodPos = XMLString::lastIndexOf(chPeriod, addrString, addrStrLen);

    // if the string ends with "."
    // get the second last "."
    if (XMLSize_t(lastPeriodPos + 1) == addrStrLen)
    {
        lastPeriodPos = XMLString::lastIndexOf(chPeriod, addrString, lastPeriodPos);

        if ( XMLString::isDigit(addrString[lastPeriodPos + 1]))
			return false;
    }

    if (XMLString::isDigit(addrString[lastPeriodPos + 1]))
    {
        return isWellFormedIPv4Address(addrString, addrStrLen);
    } // end of IPv4address
    else
    {
        //
        //  hostname      = *( domainlabel "." ) toplabel [ "." ]
        //  domainlabel   = alphanum | alphanum *( alphanum | "-" ) alphanum
        //  toplabel      = alpha | alpha *( alphanum | "-" ) alphanum

        // RFC 2396 states that hostnames take the form described in
        // RFC 1034 (Section 3) and RFC 1123 (Section 2.1). According
        // to RFC 1034, hostnames are limited to 255 characters.
        if (addrStrLen > 255) {
            return false;
        }

        unsigned int labelCharCount = 0;

        // domain labels can contain alphanumerics and '-"
        // but must start and end with an alphanumeric
        for (XMLSize_t i = 0; i < addrStrLen; i++)
        {
            if (addrString[i] == chPeriod)
            {
              if (((i > 0)  &&
                   (!XMLString::isAlphaNum(addrString[i-1]))) ||
                  ((i + 1 < addrStrLen) &&
                   (!XMLString::isAlphaNum(addrString[i+1])))  )
                {
                    return false;
                }
                labelCharCount = 0;
            }
            else if (!XMLString::isAlphaNum(addrString[i]) &&
                      addrString[i] != chDash)
            {
                return false;
            }
            // RFC 1034: Labels must be 63 characters or less.
            else if (++labelCharCount > 63) {
                return false;
            }
        } //for
    }

    return true;
}

bool XMLUri::processScheme(const XMLCh* const schemeStr, XMLSize_t& index)
{
    const XMLCh* tmpPtr = XMLString::findAny(schemeStr, SCHEME_SEPARATORS);

    if (tmpPtr) {
        index = tmpPtr - schemeStr;
        return isConformantSchemeName(schemeStr, index);
    }
    else {
        return false;
    }
}


bool XMLUri::isConformantSchemeName( const XMLCh* const scheme
                                   , const XMLSize_t schemeLen)
{
    if (!XMLString::isAlpha(*scheme))     // first: alpha
        return false;

    // second onwards: ( alpha | digit | "+" | "-" | "." )
    for (XMLSize_t i=1; i<schemeLen; i++)
    {
        if ( !XMLString::isAlphaNum(scheme[i]) &&
             (XMLString::indexOf(SCHEME_CHARACTERS, scheme[i]) == -1))
            return false;
    }

    return true;
}

bool XMLUri::processAuthority( const XMLCh* const authSpec
                             , const XMLSize_t authLen)
{
    int index = XMLString::indexOf(authSpec, chAt);
    XMLSize_t start = 0;

    // server = [ [ userinfo "@" ] hostport ]
    // userinfo is everything up @,
    const XMLCh* userinfo;
    int userInfoLen = 0;
    if ((index != -1) && (XMLSize_t(index) < authLen))
    {
        userinfo = authSpec;
        userInfoLen = index;
        start = index + 1;
    }
    else
    {
        userinfo = XMLUni::fgZeroLenString;
    }

    // hostport = host [ ":" port ]
    // host is everything up to ':', or up to
    // and including ']' if followed by ':'.
    //
    // Search for port boundary.
    const XMLCh* host;
    XMLSize_t hostLen = 0;
    if ((start < authLen) && (authSpec[start] == chOpenSquare))
    {
    	index = XMLString::indexOf(&(authSpec[start]), chCloseSquare);
    	if ((index != -1) && (XMLSize_t(index) < authLen))
    	{
            // skip the ']'
            index = ((start + index + 1) < authLen
              && authSpec[start + index + 1] == chColon) ? index+1 : -1;
    	}
    }
    else
    {
        index = XMLString::indexOf(&(authSpec[start]), chColon);
        if (index!=-1 && XMLSize_t(index) >= authLen)
            index = -1;
    }

    host = &(authSpec[start]);
    if (index != -1)
    {
        hostLen = index;
        start += index + 1;  // skip the :
    }
    else
    {
        hostLen = authLen - start;
        start = authLen;
    }

    // port is everything after ":"
    int port = -1;
    if ((hostLen) &&   // non empty host
        (index != -1)                    &&   // ":" found
        (start < authLen)                     )   // ":" is not the last
    {
        const XMLCh* portStr = &(authSpec[start]);
        if (*portStr)
        {
            port = 0;
            for (XMLSize_t i=0; i<(authLen - start); i++)
            {
                if (portStr[i] < chDigit_0 || portStr[i] > chDigit_9)
                {
                  // Assume this is a registry-based authority.
                  //
                  port = -1;
                  hostLen = 0;
                  host = XMLUni::fgZeroLenString;
                  userInfoLen = 0;
                  userinfo = XMLUni::fgZeroLenString;
                  break;
                }

                port = (port * 10) + (int) (portStr[i] - chDigit_0);
            }
        }
    }

    return isValidServerBasedAuthority(host, hostLen, port, userinfo, userInfoLen)
      || isValidRegistryBasedAuthority(authSpec, authLen);
}

bool XMLUri::processPath(const XMLCh* const pathStr,
                         const XMLSize_t pathStrLen,
                         const bool isSchemePresent,
                         const bool bAllowSpaces/*=false*/)
{
    if (pathStrLen != 0)
    {
        XMLSize_t index = 0;
        XMLCh testChar = chNull;
        bool isOpaque = (!isSchemePresent || *pathStr == chForwardSlash);

        // path - everything up to query string or fragment
        //
        // RFC 2732 only allows '[' and ']' to appear in the opaque part.
        while (index < pathStrLen)
        {
            testChar = pathStr[index];
            if (testChar == chQuestion || testChar == chPound)
                break;

            if (testChar == chPercent)
            {
                if (index+2 >= pathStrLen ||
                    !XMLString::isHex(pathStr[index+1]) ||
                    !XMLString::isHex(pathStr[index+2]))
                        return false;
            }
            else if (testChar==chSpace)
            {
                if(!bAllowSpaces)
                    return false;
            }
            else if (!isUnreservedCharacter(testChar) &&
                     ((isOpaque && !isPathCharacter(testChar)) ||
                      (!isOpaque && !isReservedCharacter(testChar))))
            {
                return false;
            }

            index++;
        }

        // query - starts with ? and up to fragment or end
        // fragment - starts with #
        bool isQuery = (testChar == chQuestion);
        if (isQuery || testChar == chPound)
        {
            index++;
            while (index < pathStrLen)
            {
                testChar = pathStr[index];
                if (testChar == chPound && isQuery) {
                    isQuery = false;
                    index++;
                    continue;
                }

                if (testChar == chPercent)
                {
                    if (index+2 >= pathStrLen ||
                        !XMLString::isHex(pathStr[index+1]) ||
                        !XMLString::isHex(pathStr[index+2]))
                        return false;
                }
                else if (testChar==chSpace)
                {
                    if(!bAllowSpaces)
                        return false;
                }
                else if (!isReservedOrUnreservedCharacter(testChar))
                {
                    return false;
                }
                index++;
            }
        }
    } //if (pathStrLen...)

    return true;
}

/***
 * [Bug7698]: filenames with embedded spaces in schemaLocation strings not handled properly
 *
 * This method is called when Scanner/TraverseSchema knows that the URI reference is
 * for local file.
 *
 ***/
void XMLUri::normalizeURI(const XMLCh*     const systemURI,
                                XMLBuffer&       normalizedURI)
{
    const XMLCh* pszSrc = systemURI;

    normalizedURI.reset();

    while (*pszSrc) {

        if ((*(pszSrc) == chPercent)
        &&  (*(pszSrc+1) == chDigit_2)
        &&  (*(pszSrc+2) == chDigit_0))
        {
            pszSrc += 3;
            normalizedURI.append(chSpace);
        }
        else
        {
            normalizedURI.append(*pszSrc);
            pszSrc++;
        }
    }
}

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_TOCREATE(XMLUri)

void XMLUri::serialize(XSerializeEngine& serEng)
{

    if (serEng.isStoring())
    {
        serEng<<fPort;
        serEng.writeString(fScheme);
        serEng.writeString(fUserInfo);
        serEng.writeString(fHost);
        serEng.writeString(fRegAuth);
        serEng.writeString(fPath);
        serEng.writeString(fQueryString);
        serEng.writeString(fFragment);
        serEng.writeString(fURIText);
    }
    else
    {
        serEng>>fPort;
        serEng.readString(fScheme);
        serEng.readString(fUserInfo);
        serEng.readString(fHost);
        serEng.readString(fRegAuth);
        serEng.readString(fPath);
        serEng.readString(fQueryString);
        serEng.readString(fFragment);
        serEng.readString(fURIText);
    }

}

XMLUri::XMLUri(MemoryManager* const manager)
: fPort(-1)
, fScheme(0)
, fUserInfo(0)
, fHost(0)
, fRegAuth(0)
, fPath(0)
, fQueryString(0)
, fFragment(0)
, fURIText(0)
, fMemoryManager(manager)
{
}

XERCES_CPP_NAMESPACE_END
