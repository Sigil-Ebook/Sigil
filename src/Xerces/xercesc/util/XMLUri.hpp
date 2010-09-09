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
 * $Id: XMLUri.hpp 557254 2007-07-18 13:28:54Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XMLURI_HPP)
#define XERCESC_INCLUDE_GUARD_XMLURI_HPP

#include <xercesc/util/XMemory.hpp>
#include <xercesc/util/XMLString.hpp>

#include <xercesc/internal/XSerializable.hpp>
#include <xercesc/framework/XMLBuffer.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/*
 * This class is a direct port of Java's URI class, to distinguish
 * itself from the XMLURL, we use the name XMLUri instead of
 * XMLURI.
 *
 * TODO: how to relate XMLUri and XMLURL since URL is part of URI.
 *
 */

class XMLUTIL_EXPORT XMLUri : public XSerializable, public XMemory
{
public:

    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------

    /**
     * Construct a new URI from a URI specification string.
     *
     * If the specification follows the "generic URI" syntax, (two slashes
     * following the first colon), the specification will be parsed
     * accordingly - setting the
     *                           scheme,
     *                           userinfo,
     *                           host,
     *                           port,
     *                           path,
     *                           querystring and
     *                           fragment
     * fields as necessary.
     *
     * If the specification does not follow the "generic URI" syntax,
     * the specification is parsed into a
     *                           scheme and
     *                           scheme-specific part (stored as the path) only.
     *
     * @param uriSpec the URI specification string (cannot be null or empty)
     *
     * @param manager Pointer to the memory manager to be used to
     *                allocate objects.
     *
     * ctor# 2
     *
     */
    XMLUri(const XMLCh* const    uriSpec,
           MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    /**
     * Construct a new URI from a base URI and a URI specification string.
     * The URI specification string may be a relative URI.
     *
     * @param baseURI the base URI (cannot be null if uriSpec is null or
     *                empty)
     *
     * @param uriSpec the URI specification string (cannot be null or
     *                empty if base is null)
     *
     * @param manager Pointer to the memory manager to be used to
     *                allocate objects.
     *
     * ctor# 7 relative ctor
     *
     */
    XMLUri(const XMLUri* const  baseURI
         , const XMLCh* const   uriSpec
         , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    /**
     * Copy constructor
     */
    XMLUri(const XMLUri& toCopy);
    XMLUri& operator=(const XMLUri& toAssign);

    virtual ~XMLUri();

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    /**
     * Get the URI as a string specification. See RFC 2396 Section 5.2.
     *
     * @return the URI string specification
     */
    const XMLCh* getUriText() const;

    /**
     * Get the scheme for this URI.
     *
     * @return the scheme for this URI
     */
     const XMLCh* getScheme() const;

    /**
     * Get the userinfo for this URI.
     *
     * @return the userinfo for this URI (null if not specified).
     */
     const XMLCh* getUserInfo() const;


    /**
     * Get the host for this URI.
     *
     * @return the host for this URI (null if not specified).
     */
     const XMLCh* getHost() const;

    /**
     * Get the port for this URI.
     *
     * @return the port for this URI (-1 if not specified).
     */
     int getPort() const;
     
    /**
     * Get the registry based authority for this URI.
     * 
     * @return the registry based authority (null if not specified).
     */
     const XMLCh* getRegBasedAuthority() const;

    /**
     * Get the path for this URI. Note that the value returned is the path
     * only and does not include the query string or fragment.
     *
     * @return the path for this URI.
     */
     const XMLCh* getPath() const;

    /**
     * Get the query string for this URI.
     *
     * @return the query string for this URI. Null is returned if there
     *         was no "?" in the URI spec, empty string if there was a
     *         "?" but no query string following it.
     */
     const XMLCh* getQueryString() const;

    /**
     * Get the fragment for this URI.
     *
     * @return the fragment for this URI. Null is returned if there
     *         was no "#" in the URI spec, empty string if there was a
     *         "#" but no fragment following it.
     */
     const XMLCh* getFragment() const;

    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------

    /**
     * Set the scheme for this URI. The scheme is converted to lowercase
     * before it is set.
     *
     * @param newScheme the scheme for this URI (cannot be null)
     *
     */
     void setScheme(const XMLCh* const newScheme);

    /**
     * Set the userinfo for this URI. If a non-null value is passed in and
     * the host value is null, then an exception is thrown.
     *
     * @param newUserInfo the userinfo for this URI
     *
     */
     void setUserInfo(const XMLCh* const newUserInfo);

    /**
     * Set the host for this URI. If null is passed in, the userinfo
     * field is also set to null and the port is set to -1.
     *
     * Note: This method overwrites registry based authority if it
     * previously existed in this URI.
     *
     * @param newHost the host for this URI
     *
     */
     void setHost(const XMLCh* const newHost);

    /**
     * Set the port for this URI. -1 is used to indicate that the port is
     * not specified, otherwise valid port numbers are  between 0 and 65535.
     * If a valid port number is passed in and the host field is null,
     * an exception is thrown.
     *
     * @param newPort the port number for this URI
     *
     */
     void setPort(int newPort);
     
    /**
     * Sets the registry based authority for this URI.
     * 
     * Note: This method overwrites server based authority
     * if it previously existed in this URI.
     * 
     * @param newRegAuth the registry based authority for this URI
     */
     void setRegBasedAuthority(const XMLCh* const newRegAuth);

    /**
     * Set the path for this URI.
     *
     * If the supplied path is null, then the
     * query string and fragment are set to null as well.
     *
     * If the supplied path includes a query string and/or fragment,
     * these fields will be parsed and set as well.
     *
     * Note:
     *
     * For URIs following the "generic URI" syntax, the path
     * specified should start with a slash.
     *
     * For URIs that do not follow the generic URI syntax, this method
     * sets the scheme-specific part.
     *
     * @param newPath the path for this URI (may be null)
     *
     */
     void setPath(const XMLCh* const newPath);

    /**
     * Set the query string for this URI. A non-null value is valid only
     * if this is an URI conforming to the generic URI syntax and
     * the path value is not null.
     *
     * @param newQueryString the query string for this URI
     *
     */
     void setQueryString(const XMLCh* const newQueryString);

    /**
     * Set the fragment for this URI. A non-null value is valid only
     * if this is a URI conforming to the generic URI syntax and
     * the path value is not null.
     *
     * @param newFragment the fragment for this URI
     *
     */
     void setFragment(const XMLCh* const newFragment);

     // -----------------------------------------------------------------------
    //  Miscellaneous methods
    // -----------------------------------------------------------------------

    /**
     * Determine whether a given string contains only URI characters (also
     * called "uric" in RFC 2396). uric consist of all reserved
     * characters, unreserved characters and escaped characters.
     *
     * @return true if the string is comprised of uric, false otherwise
     */
    static bool isURIString(const XMLCh* const uric);

    /**
     * Determine whether a given string is a valid URI
     */
    static bool isValidURI( const XMLUri* const baseURI
                          , const XMLCh* const uriStr
                          , bool bAllowSpaces=false);
    /**
     * Determine whether a given string is a valid URI
     */
    static bool isValidURI( bool haveBaseURI
                          , const XMLCh* const uriStr
                          , bool bAllowSpaces=false);


    static void normalizeURI(const XMLCh*     const systemURI,
                                   XMLBuffer&       normalizedURI);

    /***
     * Support for Serialization/De-serialization
     ***/
    DECL_XSERIALIZABLE(XMLUri)

    XMLUri(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

private:

    static const XMLCh MARK_OR_RESERVED_CHARACTERS[];
    static const XMLCh RESERVED_CHARACTERS[];
    static const XMLCh MARK_CHARACTERS[];
    static const XMLCh SCHEME_CHARACTERS[];
    static const XMLCh USERINFO_CHARACTERS[];
    static const XMLCh REG_NAME_CHARACTERS[];
    static const XMLCh PATH_CHARACTERS[];

    //helper method for getUriText
    void buildFullText();

    // -----------------------------------------------------------------------
    //  Private helper methods
    // -----------------------------------------------------------------------

    /**
     * Determine whether a character is a reserved character:
     *
     * @return true if the string contains any reserved characters
     */
    static bool isReservedCharacter(const XMLCh theChar);
    
    /**
     * Determine whether a character is a path character:
     *
     * @return true if the character is path character
     */
    static bool isPathCharacter(const XMLCh theChar);

    /**
     * Determine whether a char is an unreserved character.
     *
     * @return true if the char is unreserved, false otherwise
     */
    static bool isUnreservedCharacter(const XMLCh theChar);

    /**
     * Determine whether a char is an reserved or unreserved character.
     *
     * @return true if the char is reserved or unreserved, false otherwise
     */                
    static bool isReservedOrUnreservedCharacter(const XMLCh theChar);

    /**
     * Determine whether a scheme conforms to the rules for a scheme name.
     * A scheme is conformant if it starts with an alphanumeric, and
     * contains only alphanumerics, '+','-' and '.'.
     *
     * @return true if the scheme is conformant, false otherwise
     */
    static bool isConformantSchemeName(const XMLCh* const scheme);

    /**
     * Determine whether a userInfo conforms to the rules for a userinfo.
     *
     * @return true if the scheme is conformant, false otherwise
     */
    static void isConformantUserInfo(const XMLCh* const userInfo
        , MemoryManager* const manager);
    
    /**
     * Determines whether the components host, port, and user info
     * are valid as a server authority.
     *
     * @return true if the given host, port, and userinfo compose
     * a valid server authority
     */
    static bool isValidServerBasedAuthority(const XMLCh* const host
                                           , const XMLSize_t hostLen
                                           , const int port
                                           , const XMLCh* const userinfo
                                           , const XMLSize_t userLen);
                                           
    /**
     * Determines whether the components host, port, and user info
     * are valid as a server authority.
     *
     * @return true if the given host, port, and userinfo compose
     * a valid server authority
     */
    static bool isValidServerBasedAuthority(const XMLCh* const host
                                           , const int port
                                           , const XMLCh* const userinfo
                                           , MemoryManager* const manager);
      
   /**
    * Determines whether the given string is a registry based authority.
    * 
    * @param authority the authority component of a URI
    * 
    * @return true if the given string is a registry based authority
    */
    static bool isValidRegistryBasedAuthority(const XMLCh* const authority
                                             , const XMLSize_t authLen);

   /**
    * Determines whether the given string is a registry based authority.
    * 
    * @param authority the authority component of a URI
    * 
    * @return true if the given string is a registry based authority
    */
    static bool isValidRegistryBasedAuthority(const XMLCh* const authority);

    /**
     * Determine whether a string is syntactically capable of representing
     * a valid IPv4 address, IPv6 reference or the domain name of a network host.
     *
     * A valid IPv4 address consists of four decimal digit groups
     * separated by a '.'.
     *
     * See RFC 2732 Section 3, and RFC 2373 Section 2.2, for the 
     * definition of IPv6 references.
     *
     * A hostname consists of domain labels (each of which must begin and
     * end with an alphanumeric but may contain '-') separated by a '.'.
     * See RFC 2396 Section 3.2.2.
     *
     * @return true if the string is a syntactically valid IPv4 address
     *              or hostname
     */
     static bool isWellFormedAddress(const XMLCh* const addr
         , MemoryManager* const manager);
     
    /**
     * Determines whether a string is an IPv4 address as defined by 
     * RFC 2373, and under the further constraint that it must be a 32-bit
     * address. Though not expressed in the grammar, in order to satisfy 
     * the 32-bit address constraint, each segment of the address cannot 
     * be greater than 255 (8 bits of information).
     *
     * @return true if the string is a syntactically valid IPv4 address
     */
     static bool isWellFormedIPv4Address(const XMLCh* const addr, const XMLSize_t length);
     
    /**
     * Determines whether a string is an IPv6 reference as defined
     * by RFC 2732, where IPv6address is defined in RFC 2373. The 
     * IPv6 address is parsed according to Section 2.2 of RFC 2373,
     * with the additional constraint that the address be composed of
     * 128 bits of information.
     *
     * Note: The BNF expressed in RFC 2373 Appendix B does not 
     * accurately describe section 2.2, and was in fact removed from
     * RFC 3513, the successor of RFC 2373.
     *
     * @return true if the string is a syntactically valid IPv6 reference
     */
     static bool isWellFormedIPv6Reference(const XMLCh* const addr, const XMLSize_t length);
     
    /**
     * Helper function for isWellFormedIPv6Reference which scans the 
     * hex sequences of an IPv6 address. It returns the index of the 
     * next character to scan in the address, or -1 if the string 
     * cannot match a valid IPv6 address. 
     *
     * @param address the string to be scanned
     * @param index the beginning index (inclusive)
     * @param end the ending index (exclusive)
     * @param counter a counter for the number of 16-bit sections read
     * in the address
     *
     * @return the index of the next character to scan, or -1 if the
     * string cannot match a valid IPv6 address
     */
     static int scanHexSequence (const XMLCh* const addr, XMLSize_t index, XMLSize_t end, int& counter);

    /**
     * Get the indicator as to whether this URI uses the "generic URI"
     * syntax.
     *
     * @return true if this URI uses the "generic URI" syntax, false
     *         otherwise
     */
     bool isGenericURI();

    // -----------------------------------------------------------------------
    //  Miscellaneous methods
    // -----------------------------------------------------------------------

    /**
     * Initialize all fields of this URI from another URI.
     *
     * @param toCopy the URI to copy (cannot be null)
     */
     void initialize(const XMLUri& toCopy);

    /**
     * Initializes this URI from a base URI and a URI specification string.
     * See RFC 2396 Section 4 and Appendix B for specifications on parsing
     * the URI and Section 5 for specifications on resolving relative URIs
     * and relative paths.
     *
     * @param baseURI the base URI (may be null if uriSpec is an absolute
     *               URI)
     *
     * @param uriSpec the URI spec string which may be an absolute or
     *                  relative URI (can only be null/empty if base
     *                  is not null)
     *
     */
     void initialize(const XMLUri* const baseURI
                   , const XMLCh*  const uriSpec);

    /**
     * Initialize the scheme for this URI from a URI string spec.
     *
     * @param uriSpec the URI specification (cannot be null)
     *
     */
     void initializeScheme(const XMLCh* const uriSpec);

    /**
     * Initialize the authority (userinfo, host and port) for this
     * URI from a URI string spec.
     *
     * @param uriSpec the URI specification (cannot be null)
     *
     */
     void initializeAuthority(const XMLCh* const uriSpec);

    /**
     * Initialize the path for this URI from a URI string spec.
     *
     * @param uriSpec the URI specification (cannot be null)
     *
     */
     void initializePath(const XMLCh* const uriSpec);

     /**
      * cleanup the data variables
      *
      */
     void cleanUp();

    static bool isConformantSchemeName(const XMLCh* const scheme,
                                       const XMLSize_t schemeLen);
    static bool processScheme(const XMLCh* const uriStr, XMLSize_t& index);
    static bool processAuthority(const XMLCh* const uriStr, const XMLSize_t authLen);
    static bool isWellFormedAddress(const XMLCh* const addr, const XMLSize_t addrLen);
    static bool processPath(const XMLCh* const pathStr, const XMLSize_t pathStrLen,
                            const bool isSchemePresent, const bool bAllowSpaces=false);

    // -----------------------------------------------------------------------
    //  Data members
    //
    //  for all the data member, we own it,
    //  responsible for the creation and/or deletion for
    //  the memory allocated.
    //
    // -----------------------------------------------------------------------
    int             fPort;
    XMLCh*          fScheme;
    XMLCh*          fUserInfo;
    XMLCh*          fHost;
    XMLCh*          fRegAuth;
    XMLCh*          fPath;
    XMLCh*          fQueryString;
    XMLCh*          fFragment;
    XMLCh*          fURIText;
    MemoryManager*  fMemoryManager;
};

// ---------------------------------------------------------------------------
//  XMLUri: Getter methods
// ---------------------------------------------------------------------------
inline const XMLCh* XMLUri::getScheme() const
{
    return fScheme;
}

inline const XMLCh* XMLUri::getUserInfo() const
{
	return fUserInfo;
}

inline const XMLCh* XMLUri::getHost() const
{
	return fHost;
}

inline int XMLUri::getPort() const
{
	return fPort;
}

inline const XMLCh* XMLUri::getRegBasedAuthority() const
{
	return fRegAuth;
}

inline const XMLCh* XMLUri::getPath() const
{
	return fPath;
}

inline const XMLCh* XMLUri::getQueryString() const
{
	return fQueryString;
}

inline const XMLCh* XMLUri::getFragment() const
{
	return fFragment;
}

inline const XMLCh* XMLUri::getUriText() const
{
    //
    //  Fault it in if not already. Since this is a const method and we
    //  can't use mutable members due the compilers we have to support,
    //  we have to cast off the constness.
    //
    if (!fURIText)
        ((XMLUri*)this)->buildFullText();

    return fURIText;
}

// ---------------------------------------------------------------------------
//  XMLUri: Helper methods
// ---------------------------------------------------------------------------
inline bool XMLUri::isReservedOrUnreservedCharacter(const XMLCh theChar)
{
   return (XMLString::isAlphaNum(theChar) ||
           XMLString::indexOf(MARK_OR_RESERVED_CHARACTERS, theChar) != -1);
}

inline bool XMLUri::isReservedCharacter(const XMLCh theChar)
{
    return (XMLString::indexOf(RESERVED_CHARACTERS, theChar) != -1);
}

inline bool XMLUri::isPathCharacter(const XMLCh theChar)
{
    return (XMLString::indexOf(PATH_CHARACTERS, theChar) != -1);
}

inline bool XMLUri::isUnreservedCharacter(const XMLCh theChar)
{
    return (XMLString::isAlphaNum(theChar) ||
            XMLString::indexOf(MARK_CHARACTERS, theChar) != -1);
}

XERCES_CPP_NAMESPACE_END

#endif
