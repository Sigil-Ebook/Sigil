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
 * $Id: QName.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_QNAME_HPP)
#define XERCESC_INCLUDE_GUARD_QNAME_HPP

#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/XMemory.hpp>
#include <xercesc/util/PlatformUtils.hpp>

#include <xercesc/internal/XSerializable.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XMLUTIL_EXPORT QName : public XSerializable, public XMemory
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    /** Default constructor. */
    QName(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    /** Constructs a specified qname using prefix, and localpart. */
    QName
    (
          const XMLCh* const   prefix
        , const XMLCh* const   localPart
	    , const unsigned int   uriId
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    /** Constructs a specified qname using rawName. */
    QName
    (
          const XMLCh* const   rawName
	    , const unsigned int   uriId
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    /** Copy constructor. */
    QName(const QName& qname);

    ~QName();

    // -----------------------------------------------------------------------
    //  Getters
    // -----------------------------------------------------------------------
    const XMLCh* getPrefix() const;
    XMLCh* getPrefix();

    const XMLCh* getLocalPart() const;
    XMLCh* getLocalPart();

    unsigned int getURI() const;

    const XMLCh* getRawName() const;
    XMLCh* getRawName();

    MemoryManager* getMemoryManager() const;

    // -----------------------------------------------------------------------
    //  Setters
    // -----------------------------------------------------------------------
    void setName
    (
        const XMLCh* const        prefix
      , const XMLCh* const        localPart
	   , const unsigned int        uriId
    );

    void setName
    (
        const XMLCh* const        rawName
	   , const unsigned int        uriId
    );

    void setPrefix(const XMLCh*) ;
    void setLocalPart(const XMLCh*) ;
    void setNPrefix(const XMLCh*, const XMLSize_t ) ;
    void setNLocalPart(const XMLCh*, const XMLSize_t ) ;
    void setURI(const unsigned int) ;

    void setValues(const QName& qname);

    // -----------------------------------------------------------------------
    //  comparison
    // -----------------------------------------------------------------------
    bool operator==(const QName&) const;

    // -----------------------------------------------------------------------
    //  Misc
    // -----------------------------------------------------------------------
    void cleanUp();

    /***
     * Support for Serialization/De-serialization
     ***/
    DECL_XSERIALIZABLE(QName)

private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------    
    QName& operator=(const QName&);

    // -----------------------------------------------------------------------
    //  Private instance variables
    //
    //  We copy the followings from XMLAttr.hpp, but stick to Java version's
    //  naming convention
    //
    //  fPrefix
    //  fPrefixBufSz
    //      The prefix that was applied to this attribute's name, and the
    //      current size of the buffer (minus one for the null.) Prefixes
    //      really don't matter technically but it might be required for
    //      practical reasons, to recreate the original document for instance.
    //
    //  fLocalPart
    //  fLocalPartBufSz
    //      The base part of the name of the attribute, and the current size
    //      of the buffer (minus one, where the null is.)
    //
    //  fRawName
    //  fRawNameBufSz
    //      This is the QName form of the name, which is faulted in (from the
    //      prefix and name) upon request. The size field indicates the
    //      current size of the buffer (minus one for the null.) It will be
    //      zero until filled in.
    //
    //  fURIId
    //      The id of the URI that this attribute belongs to.
    // -----------------------------------------------------------------------
    XMLSize_t           fPrefixBufSz;
    XMLSize_t           fLocalPartBufSz;
    XMLSize_t           fRawNameBufSz;
    unsigned int        fURIId;
    XMLCh*              fPrefix;
    XMLCh*              fLocalPart;
    XMLCh*              fRawName;
    MemoryManager*      fMemoryManager;
};

// ---------------------------------------------------------------------------
//  QName: Getter methods
// ---------------------------------------------------------------------------
inline const XMLCh* QName::getPrefix() const
{
	return fPrefix;
}

inline XMLCh* QName::getPrefix()
{
	return fPrefix;
}

inline const XMLCh* QName::getLocalPart() const
{
	return fLocalPart;
}

inline XMLCh* QName::getLocalPart()
{
	return fLocalPart;
}

inline unsigned int QName::getURI() const
{
	return fURIId;
}

inline MemoryManager* QName::getMemoryManager() const
{
    return fMemoryManager;
}

// ---------------------------------------------------------------------------
//  QName: Setter methods
// ---------------------------------------------------------------------------
inline void QName::setURI(const unsigned int uriId)
{
    fURIId = uriId;
}

inline void QName::setPrefix(const XMLCh* prefix)
{
    setNPrefix(prefix, XMLString::stringLen(prefix));
}

inline void QName::setLocalPart(const XMLCh* localPart)
{
    setNLocalPart(localPart, XMLString::stringLen(localPart));
}

XERCES_CPP_NAMESPACE_END

#endif
