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
 * $Id: InputSource.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_INPUTSOURCE_HPP)
#define XERCESC_INCLUDE_GUARD_INPUTSOURCE_HPP

#include <xercesc/util/PlatformUtils.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class BinInputStream;


/**
  * A single input source for an XML entity.
  *
  * <p>This class encapsulates information about an input source in a
  * single object, which may include a public identifier or a system
  * identifier</p>
  *
  * <p>There are two places that the application will deliver this input
  * source to the parser: as the argument to the Parser::parse method, or as
  * the return value of the EntityResolver::resolveEntity method.</p>
  *
  * <p>InputSource is never used directly, but is the base class for a number
  * of derived classes for particular types of input sources. Derivatives are
  * provided (in the framework/ directory) for URL input sources, memory buffer
  * input sources, and so on.</p>
  *
  * <p>When it is time to parse the input described by an input source, it
  * will be asked to create a binary stream for that source. That stream will
  * be used to input the data of the source. The derived class provides the
  * implementation of the makeStream() method, and provides a type of stream
  * of the correct type for the input source it represents.
  *
  * <p>An InputSource object belongs to the application: the parser never
  * modifies them in any way. They are always passed by const reference so
  * the parser will make a copy of any input sources that it must keep
  * around beyond the call.</p>
  *
  * @see Parser#parse
  * @see EntityResolver#resolveEntity
  */
class SAX_EXPORT InputSource : public XMemory
{
public:
    // -----------------------------------------------------------------------
    //  All constructors are hidden, just the destructor is available
    // -----------------------------------------------------------------------
    /** @name Destructor */
    //@{
  /**
    * Destructor
    *
    */
    virtual ~InputSource();
    //@}


    // -----------------------------------------------------------------------
    /** @name Virtual input source interface */
    //@{
  /**
    * Makes the byte stream for this input source.
    *
    * <p>The derived class must create and return a binary input stream of an
    * appropriate type for its kind of data source. The returned stream must
    * be dynamically allocated and becomes the parser's property.
    * </p>
    *
    * @see BinInputStream
    */
    virtual BinInputStream* makeStream() const = 0;

    //@}


    // -----------------------------------------------------------------------
    /** @name Getter methods */
    //@{
  /**
    * An input source can be set to force the parser to assume a particular
    * encoding for the data that input source represents, via the setEncoding()
    * method. This method returns name of the encoding that is to be forced.
    * If the encoding has never been forced, it returns a null pointer.
    *
    * @return The forced encoding, or null if none was supplied.
    * @see #setEncoding
    */
    virtual const XMLCh* getEncoding() const;


  /**
    * Get the public identifier for this input source.
    *
    * @return The public identifier, or null if none was supplied.
    * @see #setPublicId
    */
    virtual const XMLCh* getPublicId() const;


  /**
    * Get the system identifier for this input source.
    *
    * <p>If the system ID is a URL, it will be fully resolved.</p>
    *
    * @return The system identifier.
    * @see #setSystemId
    */
    virtual const XMLCh* getSystemId() const;

  /**
    * Get the flag that indicates if the parser should issue fatal error if this input source
    * is not found.
    *
    * @return True if the parser should issue fatal error if this input source is not found.
    *         False if the parser issue warning message instead.
    * @see #setIssueFatalErrorIfNotFound
    */
    virtual bool getIssueFatalErrorIfNotFound() const;

    MemoryManager* getMemoryManager() const;

    //@}


    // -----------------------------------------------------------------------
    /** @name Setter methods */
    //@{

  /**
    * Set the encoding which will be required for use with the XML text read
    * via a stream opened by this input source.
    *
    * <p>This is usually not set, allowing the encoding to be sensed in the
    * usual XML way. However, in some cases, the encoding in the file is known
    * to be incorrect because of intermediate transcoding, for instance
    * encapsulation within a MIME document.
    *
    * @param encodingStr The name of the encoding to force.
    */
    virtual void setEncoding(const XMLCh* const encodingStr);


  /**
    * Set the public identifier for this input source.
    *
    * <p>The public identifier is always optional: if the application writer
    * includes one, it will be provided as part of the location information.</p>
    *
    * @param publicId The public identifier as a string.
    * @see Locator#getPublicId
    * @see SAXParseException#getPublicId
    * @see #getPublicId
    */
    virtual void setPublicId(const XMLCh* const publicId);

  /**
    * Set the system identifier for this input source.
    *
    * <p>Set the system identifier for this input source.
    *
    * </p>The system id is always required. The public id may be used to map
    * to another system id, but the system id must always be present as a fall
    * back.
    *
    * <p>If the system ID is a URL, it must be fully resolved.</p>
    *
    * @param systemId The system identifier as a string.
    * @see #getSystemId
    * @see Locator#getSystemId
    * @see SAXParseException#getSystemId
    */
    virtual void setSystemId(const XMLCh* const systemId);

  /**
    * Indicates if the parser should issue fatal error if this input source
    * is not found.  If set to false, the parser issue warning message instead.
    *
    * @param  flag True if the parser should issue fatal error if this input source is not found.
    *               If set to false, the parser issue warning message instead.  (Default: true)
    *
    * @see #getIssueFatalErrorIfNotFound
    */
    virtual void setIssueFatalErrorIfNotFound(const bool flag);

    //@}


protected :
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    /** @name Constructors and Destructor */
    //@{
    /** Default constructor */
    InputSource(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    /** Constructor with a system identifier as XMLCh type.
      * @param systemId The system identifier (URI).
      * @param manager    Pointer to the memory manager to be used to
      *                   allocate objects.
      */
    InputSource(const XMLCh* const systemId,
                MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    /** Constructor with a system and public identifiers
      * @param systemId The system identifier (URI).
      * @param publicId The public identifier as in the entity definition.
      * @param manager    Pointer to the memory manager to be used to
      *                   allocate objects.
      */
    InputSource
    (
        const   XMLCh* const   systemId
        , const XMLCh* const   publicId
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    /** Constructor witha system identifier as string
      * @param systemId The system identifier (URI).
      * @param manager    Pointer to the memory manager to be used to
      *                   allocate objects.
      */
    InputSource(const char* const systemId,
                MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    /** Constructor witha system and public identifiers. Both as string
      * @param systemId The system identifier (URI).
      * @param publicId The public identifier as in the entity definition.
      * @param manager    Pointer to the memory manager to be used to
      *                   allocate objects.
      */
    InputSource
    (
        const   char* const systemId
        , const char* const publicId
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    //@}





private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    InputSource(const InputSource&);
    InputSource& operator=(const InputSource&);


    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fEncoding
    //      This is the encoding to use. Usually this is null, which means
    //      to use the information found in the file itself. But, if set,
    //      this encoding will be used without question.
    //
    //  fPublicId
    //      This is the optional public id for the input source. It can be
    //      null if none is desired.
    //
    //  fSystemId
    //      This is the system id for the input source. This is what is
    //      actually used to open the source.
    //
    //  fFatalErrorIfNotFound
    // -----------------------------------------------------------------------
    MemoryManager* const fMemoryManager;
    XMLCh*         fEncoding;
    XMLCh*         fPublicId;
    XMLCh*         fSystemId;
    bool           fFatalErrorIfNotFound;
};


// ---------------------------------------------------------------------------
//  InputSource: Getter methods
// ---------------------------------------------------------------------------
inline const XMLCh* InputSource::getEncoding() const
{
    return fEncoding;
}

inline const XMLCh* InputSource::getPublicId() const
{
    return fPublicId;
}

inline const XMLCh* InputSource::getSystemId() const
{
    return fSystemId;
}

inline bool InputSource::getIssueFatalErrorIfNotFound() const
{
    return fFatalErrorIfNotFound;
}

inline MemoryManager* InputSource::getMemoryManager() const
{
    return fMemoryManager;
}

// ---------------------------------------------------------------------------
//  InputSource: Setter methods
// ---------------------------------------------------------------------------
inline void InputSource::setIssueFatalErrorIfNotFound(const bool flag)
{
    fFatalErrorIfNotFound = flag;
}

XERCES_CPP_NAMESPACE_END

#endif
