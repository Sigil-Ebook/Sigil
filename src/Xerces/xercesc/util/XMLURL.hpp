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
 * $Id: XMLURL.hpp 536133 2007-05-08 09:05:14Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XMLURL_HPP)
#define XERCESC_INCLUDE_GUARD_XMLURL_HPP

#include <xercesc/util/PlatformUtils.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class BinInputStream;

//
//  This class supports file, http, and ftp style URLs. All others are
//  rejected
//
class XMLUTIL_EXPORT XMLURL : public XMemory
{
public:
    // -----------------------------------------------------------------------
    //  Class types
    //
    //  And they must remain in this order because they are indexes into an
    //  array internally!
    // -----------------------------------------------------------------------
    enum Protocols
    {
        File
        , HTTP
        , FTP
        , HTTPS

        , Protocols_Count
        , Unknown
    };


    // -----------------------------------------------------------------------
    //  Public static methods
    // -----------------------------------------------------------------------
    static Protocols lookupByName(const XMLCh* const protoName);
    static bool parse(const XMLCh* const urlText, XMLURL& xmlURL);

    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    XMLURL(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    XMLURL
    (
        const   XMLCh* const    baseURL
        , const XMLCh* const    relativeURL
        , MemoryManager* const manager  = XMLPlatformUtils::fgMemoryManager
    );
    XMLURL
    (
        const   XMLCh* const    baseURL
        , const char* const     relativeURL
        , MemoryManager* const manager  = XMLPlatformUtils::fgMemoryManager
    );
    XMLURL
    (
        const   XMLURL&         baseURL
        , const XMLCh* const    relativeURL
    );
    XMLURL
    (
        const   XMLURL&         baseURL
        , const char* const     relativeURL
    );
    XMLURL
    (
        const   XMLCh* const    urlText
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );
    XMLURL
    (
        const   char* const     urlText
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );
    XMLURL(const XMLURL& toCopy);
    virtual ~XMLURL();


    // -----------------------------------------------------------------------
    //  Operators
    // -----------------------------------------------------------------------
    XMLURL& operator=(const XMLURL& toAssign);
    bool operator==(const XMLURL& toCompare) const;
    bool operator!=(const XMLURL& toCompare) const;


    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    const XMLCh* getFragment() const;
    const XMLCh* getHost() const;
    const XMLCh* getPassword() const;
    const XMLCh* getPath() const;
    unsigned int getPortNum() const;
    Protocols getProtocol() const;
    const XMLCh* getProtocolName() const;
    const XMLCh* getQuery() const;
    const XMLCh* getURLText() const;
    const XMLCh* getUser() const;
    MemoryManager* getMemoryManager() const;


    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------
    void setURL(const XMLCh* const urlText);
    void setURL
    (
        const   XMLCh* const    baseURL
        , const XMLCh* const    relativeURL
    );
    void setURL
    (
        const   XMLURL&         baseURL
        , const XMLCh* const    relativeURL
    );
    // a version of setURL that doesn't throw malformed url exceptions
    bool setURL(
        const XMLCh* const    baseURL
        , const XMLCh* const    relativeURL
        , XMLURL& xmlURL);
    // -----------------------------------------------------------------------
    //  Miscellaneous methods
    // -----------------------------------------------------------------------
    bool isRelative() const;
    bool hasInvalidChar() const;
    BinInputStream* makeNewStream() const;
    void makeRelativeTo(const XMLCh* const baseURLText);
    void makeRelativeTo(const XMLURL& baseURL);


private:
    // -----------------------------------------------------------------------
    //  Private helper methods
    // -----------------------------------------------------------------------
    void buildFullText();
    void cleanUp();
    bool conglomerateWithBase(const XMLURL& baseURL, bool useExceptions=true);
    void parse
    (
        const   XMLCh* const    urlText
    );


    // -----------------------------------------------------------------------
    //  Data members
    //
    //  fFragment
    //      The fragment part of the URL, if any. If none, its a null.
    //
    //  fHost
    //      The host part of the URL that was parsed out. This one will often
    //      be null (or "localhost", which also means the current machine.)
    //
    //  fPassword
    //      The password found, if any. If none then its a null.
    //
    //  fPath
    //      The path part of the URL that was parsed out, if any. If none,
    //      then its a null.
    //
    //  fPortNum
    //      The port that was indicated in the URL. If no port was provided
    //      explicitly, then its left zero.
    //
    //  fProtocol
    //      Indicates the type of the URL's source. The text of the prefix
    //      can be gotten from this.
    //
    //  fQuery
    //      The query part of the URL, if any. If none, then its a null.
    //
    //  fUser
    //      The username found, if any. If none, then its a null.
    //
    //  fURLText
    //      This is a copy of the URL text, after it has been taken apart,
    //      made relative if needed, canonicalized, and then put back
    //      together. Its only created upon demand.
    //
    //  fHasInvalidChar
    //      This indicates if the URL Text contains invalid characters as per
    //      RFC 2396 standard.
    // -----------------------------------------------------------------------
    MemoryManager*  fMemoryManager;
    XMLCh*          fFragment;
    XMLCh*          fHost;
    XMLCh*          fPassword;
    XMLCh*          fPath;
    unsigned int    fPortNum;
    Protocols       fProtocol;
    XMLCh*          fQuery;
    XMLCh*          fUser;
    XMLCh*          fURLText;
    bool            fHasInvalidChar;
};


// ---------------------------------------------------------------------------
//  XMLURL: Public operators
// ---------------------------------------------------------------------------
inline bool XMLURL::operator!=(const XMLURL& toCompare) const
{
    return !operator==(toCompare);
}


// ---------------------------------------------------------------------------
//  XMLURL: Getter methods
// ---------------------------------------------------------------------------
inline const XMLCh* XMLURL::getFragment() const
{
    return fFragment;
}

inline const XMLCh* XMLURL::getHost() const
{
    return fHost;
}

inline const XMLCh* XMLURL::getPassword() const
{
    return fPassword;
}

inline const XMLCh* XMLURL::getPath() const
{
    return fPath;
}

inline XMLURL::Protocols XMLURL::getProtocol() const
{
    return fProtocol;
}

inline const XMLCh* XMLURL::getQuery() const
{
    return fQuery;
}

inline const XMLCh* XMLURL::getUser() const
{
    return fUser;
}

inline const XMLCh* XMLURL::getURLText() const
{
    //
    //  Fault it in if not already. Since this is a const method and we
    //  can't use mutable members due the compilers we have to support,
    //  we have to cast off the constness.
    //
    if (!fURLText)
        ((XMLURL*)this)->buildFullText();

    return fURLText;
}

inline MemoryManager* XMLURL::getMemoryManager() const
{
    return fMemoryManager;
}

MakeXMLException(MalformedURLException, XMLUTIL_EXPORT)

XERCES_CPP_NAMESPACE_END


#endif
