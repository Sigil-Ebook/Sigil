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
 * $Id: Wrapper4DOMLSInput.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_WRAPPER4DOMLSINPUT_HPP)
#define XERCESC_INCLUDE_GUARD_WRAPPER4DOMLSINPUT_HPP

#include <xercesc/sax/InputSource.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class DOMLSInput;
class DOMLSResourceResolver;

/**
  * Wrap a DOMLSInput object and make it behave like a SAX InputSource.
  */
class XMLPARSER_EXPORT Wrapper4DOMLSInput: public InputSource
{
public:
    /** @name Constructors and Destructor */
    //@{

  /**
    * Constructor
    *
    * Wrap a DOMLSInput and make it behave like a SAX InputSource.
    * By default, the wrapper will adopt the DOMLSInput that is wrapped.
    *
    * @param  inputSource  The DOMLSInput to be wrapped
    * @param  entityResolver  The DOMLSResourceResolver to be used when resolving publicID entries
    * @param  adoptFlag    Indicates if the wrapper should adopt the wrapped
    *                      DOMLSInput. Default is true.
    * @param  manager      Pointer to the memory manager to be used to
    *                      allocate objects.
    */
    Wrapper4DOMLSInput
    (
        DOMLSInput* const inputSource
        , DOMLSResourceResolver* entityResolver = 0
        , const bool adoptFlag = true
        , MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
    );

  /**
    * Destructor
    *
    */
    virtual ~Wrapper4DOMLSInput();
    //@}


    // -----------------------------------------------------------------------
    /** @name Virtual input source interface */
    //@{
  /**
    *
    * Makes the byte stream for this input source.
    *
    * <p>The function will call the makeStream of the wrapped input source.
    * The returned stream becomes the parser's property.</p>
    *
    * @see BinInputStream
    */
    BinInputStream* makeStream() const;

    //@}

    // -----------------------------------------------------------------------
    /** @name Getter methods */
    //@{
  /**
    *
    * An input source can be set to force the parser to assume a particular
    * encoding for the data that input source reprsents, via the setEncoding()
    * method. This method will delegate to the wrapped input source to return
    * name of the encoding that is to be forced. If the encoding has never
    * been forced, it returns a null pointer.
    *
    * @return The forced encoding, or null if none was supplied.
    * @see #setEncoding
    */
    const XMLCh* getEncoding() const;


  /**
    *
    * Get the public identifier for this input source. Delegated to the
    * wrapped input source object.
    *
    * @return The public identifier, or null if none was supplied.
    * @see #setPublicId
    */
    const XMLCh* getPublicId() const;


  /**
    *
    * Get the system identifier for this input source. Delegated to the
    * wrapped input source object.
    *
    * <p>If the system ID is a URL, it will be fully resolved.</p>
    *
    * @return The system identifier.
    * @see #setSystemId
    */
    const XMLCh* getSystemId() const;

  /**
    *
    * Get the flag that indicates if the parser should issue fatal error if
    * this input source is not found. Delegated to the wrapped input source
    * object.
    *
    * @return True  if the parser should issue fatal error if this input source
    *               is not found.
    *         False if the parser issue warning message instead.
    * @see #setIssueFatalErrorIfNotFound
    */
    bool getIssueFatalErrorIfNotFound() const;

    //@}


    // -----------------------------------------------------------------------
    /** @name Setter methods */
    //@{

  /**
    *
    * Set the encoding which will be required for use with the XML text read
    * via a stream opened by this input source. This will update the wrapped
    * input source object.
    *
    * <p>This is usually not set, allowing the encoding to be sensed in the
    * usual XML way. However, in some cases, the encoding in the file is known
    * to be incorrect because of intermediate transcoding, for instance
    * encapsulation within a MIME document.
    *
    * @param encodingStr The name of the encoding to force.
    */
    void setEncoding(const XMLCh* const encodingStr);


  /**
    *
    * Set the public identifier for this input source. This will update the
    * wrapped input source object.
    *
    * <p>The public identifier is always optional: if the application writer
    * includes one, it will be provided as part of the location information.</p>
    *
    * @param publicId The public identifier as a string.
    * @see Locator#getPublicId
    * @see SAXParseException#getPublicId
    * @see #getPublicId
    */
    void setPublicId(const XMLCh* const publicId);

  /**
    *
    * Set the system identifier for this input source. This will update the
    * wrapped input source object.
    *
    * <p>The system id is always required. The public id may be used to map
    * to another system id, but the system id must always be present as a fall
    * back.</p>
    *
    * <p>If the system ID is a URL, it must be fully resolved.</p>
    *
    * @param systemId The system identifier as a string.
    * @see #getSystemId
    * @see Locator#getSystemId
    * @see SAXParseException#getSystemId
    */
    void setSystemId(const XMLCh* const systemId);

  /**
    *
    * Indicates if the parser should issue fatal error if this input source
    * is not found.  If set to false, the parser issue warning message instead.
    * This will update the wrapped input source object.
    *
    * @param  flag True if the parser should issue fatal error if this input source is not found.
    *               If set to false, the parser issue warning message instead.  (Default: true)
    *
    * @see #getIssueFatalErrorIfNotFound
    */
    void setIssueFatalErrorIfNotFound(const bool flag);

    //@}


private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    Wrapper4DOMLSInput(const Wrapper4DOMLSInput&);
    Wrapper4DOMLSInput& operator=(const Wrapper4DOMLSInput&);

    // -----------------------------------------------------------------------
    //  Private data members
    // -----------------------------------------------------------------------
    bool                    fAdoptInputSource,
                            fForceXMLChEncoding;
    DOMLSInput*             fInputSource;
    DOMLSResourceResolver*  fEntityResolver;
};

XERCES_CPP_NAMESPACE_END


#endif
