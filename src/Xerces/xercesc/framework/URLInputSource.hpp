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
 * $Id: URLInputSource.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_URLINPUTSOURCE_HPP)
#define XERCESC_INCLUDE_GUARD_URLINPUTSOURCE_HPP

#include <xercesc/util/XMLURL.hpp>
#include <xercesc/sax/InputSource.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class BinInputStream;

/**
 *  This class is a derivative of the standard InputSource class. It provides
 *  for the parser access to data which is referenced via a URL, as apposed to
 *  a local file name. The URL can be provided via an XMLURL class, as a fully
 *  qualified system id, or a base system id and a system id which may be
 *  fully qualified or may be relative to the base.
 *
 *  As with all InputSource derivatives. The primary objective of an input
 *  source is to create an input stream via which the parser can spool in
 *  data from the referenced source.
 *
 *  Note that the parse system does not necessarily support URL based XML
 *  entities out of the box. Support for socket based access is optional and
 *  controlled by the per-platform support.
 */
class XMLPARSER_EXPORT URLInputSource : public InputSource
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------

    /** @name Constructors */
    //@{

    /**
      * This constructor accepts an already built URL. It is assumed that
      * it is correct and it will be used as is. In this case, no public id
      * accepted, but it can still be set via the parent class' setPublicId()
      * method.
      *
      * @param  urlId   The URL which holds the system id of the entity
      *                 to parse.
      * @param  manager Pointer to the memory manager to be used to
      *                 allocate objects.
      */
    URLInputSource
    (
        const XMLURL& urlId
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );


    /**
     *  This constructor takes a base system id URL and a possibly relative
     *  system id. The relative part is parsed and, if it is indeed relative,
     *  it will be made relative to the passed base id. Otherwise, it will be
     *  taken as is.
     *
     *  @param  baseId      The base system id URL which provides the base
     *                      for any relative id part.
     *
     *  @param  systemId    The possibly relative system id URL. If its relative
     *                      its based on baseId, else its taken as is.
     *  @param  manager     Pointer to the memory manager to be used to
     *                      allocate objects.
     */
    URLInputSource
    (
        const   XMLCh* const    baseId
        , const XMLCh* const    systemId
        , MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
    );

    /**
     *  This constructor is identical to the previous one, except that it also
     *  allows you to set a public id if you want to.
     *
     *  @param  baseId      The base system id URL which provides the base
     *                      for any relative id part.
     *
     *  @param  systemId    The possibly relative system id URL. If its relative
     *                      its based on baseId, else its taken as is.
     *
     *  @param  publicId    The optional public id to set. This is just passed
     *                      on to the parent class for storage.
     *
     * @param  manager      Pointer to the memory manager to be used to
     *                      allocate objects.
     */
    URLInputSource
    (
        const   XMLCh* const    baseId
        , const XMLCh* const    systemId
        , const XMLCh* const    publicId
        , MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
    );


    /**
     *  This constructor is identical to the second constructor above, except that
     *  it accepts the relative system id part as a local code page string and
     *  just transcodes it internally, as a convenience.
     *
     *  @param  baseId      The base system id URL which provides the base
     *                      for any relative id part.
     *
     *  @param  systemId    The possibly relative system id URL. If its relative
     *                      its based on baseId, else its taken as is.
     *
     *  @param  manager     Pointer to the memory manager to be used to
     *                      allocate objects.
     */
    URLInputSource
    (
        const   XMLCh* const    baseId
        , const char* const     systemId
        , MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
    );

    /**
     *  This constructor is identical to the third constructor above, except that
     *  it accepts the relative and public ids as local code page strings and just
     *  transcodes them internally, as a convenience.
     *
     *  @param  baseId      The base system id URL which provides the base
     *                      for any relative id part.
     *
     *  @param  systemId    The possibly relative system id URL. If its relative
     *                      its based on baseId, else its taken as is.
     *
     *  @param  publicId    The optional public id to set. This is just passed
     *                      on to the parent class for storage.
     *                      on to the parent class for storage.
     *
     *  @param  manager     Pointer to the memory manager to be used to
     *                      allocate objects.
     */
    URLInputSource
    (
        const   XMLCh* const    baseId
        , const char* const     systemId
        , const char* const     publicId
        , MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
    );

    //@}

    /** @name Destructor */
    //@{
    ~URLInputSource();
    //@}


    // -----------------------------------------------------------------------
    //  Virtual input source interface
    // -----------------------------------------------------------------------

    /** @name Virtual methods */
    //@{

    /**
     * This method will return a binary input stream derivative that will
     * parse from the source referred to by the URL system id.
     */
    BinInputStream* makeStream() const;

    //@}


    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------

    /** @name Getter methods */
    //@{

    /**
      * This method will return a const reference to the URL member which
      * contains the system id in pre-parsed URL form. If you just want the
      * string format, call getSystemId() on the parent class.
      *
      * @return A const reference to a URL object that contains the current
      *         system id set for this input source.
      */
    const XMLURL& urlSrc() const;

    //@}


private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------    
    URLInputSource(const URLInputSource&);
    URLInputSource& operator=(const URLInputSource&);

    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fURL
    //      This is the URL created from the passed ids.
    // -----------------------------------------------------------------------
    XMLURL fURL;
};


inline const XMLURL& URLInputSource::urlSrc() const
{
    return fURL;
}

XERCES_CPP_NAMESPACE_END

#endif
