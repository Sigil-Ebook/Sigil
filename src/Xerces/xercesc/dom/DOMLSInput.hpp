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
 * $Id: DOMLSInput.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMLSINPUT_HPP)
#define XERCESC_INCLUDE_GUARD_DOMLSINPUT_HPP

#include <xercesc/util/XercesDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN


class InputSource;


/**
  * This interface represents a single input source for an XML entity.
  *
  * <p>This interface allows an application to encapsulate information about
  * an input source in a single object, which may include a public identifier,
  * a system identifier, a byte stream (possibly with a specified encoding),
  * and/or a character stream.</p>
  *
  * <p>There are two places that the application will deliver this input source
  * to the parser: as the argument to the parse method, or as the return value
  * of the DOMLSResourceResolver.resolveResource method.</p>
  *
  * <p>The DOMLSParser will use the DOMLSInput object to determine how to
  * read XML input. If there is a character stream available, the parser will
  * read that stream directly; if not, the parser will use a byte stream, if
  * available; if neither a character stream nor a byte stream is available,
  * the parser will attempt to open a URI connection to the resource identified
  * by the system identifier.</p>
  *
  * <p>A DOMLSInput object belongs to the application: the parser shall
  * never modify it in any way (it may modify a copy if necessary).</p>
  *
  * @see DOMLSParser#parse
  * @see DOMLSResourceResolver#resolveResource
  * @since DOM Level 3
  */
class CDOM_EXPORT DOMLSInput
{
protected:
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    /** @name Hidden constructors */
    //@{    
    DOMLSInput() {};
    //@}

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    /** @name Unimplemented constructors and operators */
    //@{
    DOMLSInput(const DOMLSInput &);
    DOMLSInput & operator = (const DOMLSInput &);
    //@}

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
    virtual ~DOMLSInput() {};
    //@}

    // -----------------------------------------------------------------------
    //  Virtual DOMLSInput interface
    // -----------------------------------------------------------------------
    /** @name Functions introduced in DOM Level 3 */
    //@{
    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    /**
     * String data to parse. If provided, this will always be treated as a sequence of 16-bit units (UTF-16 encoded characters). 
     * It is not a requirement to have an XML declaration when using stringData. If an XML declaration is present, the value of 
     * the encoding attribute will be ignored.
     *
     */
    virtual const XMLCh* getStringData() const = 0;

    /**
     * Returns the byte stream for this input source.
     *
     * @see InputSource
     */
    virtual InputSource* getByteStream() const = 0;

    /**
     * An input source can be set to force the parser to assume a particular
     * encoding for the data that input source reprsents, via the setEncoding()
     * method. This method returns name of the encoding that is to be forced.
     * If the encoding has never been forced, it returns a null pointer.
     *
     * @return The forced encoding, or null if none was supplied.
     * @see #setEncoding
     * @since DOM Level 3
     */
    virtual const XMLCh* getEncoding() const = 0;


    /**
     * Get the public identifier for this input source.
     *
     * @return The public identifier, or null if none was supplied.
     * @see #setPublicId
     * @since DOM Level 3
     */
    virtual const XMLCh* getPublicId() const = 0;


    /**
     * Get the system identifier for this input source.
     *
     * <p>If the system ID is a URL, it will be fully resolved.</p>
     *
     * @return The system identifier.
     * @see #setSystemId
     * @since DOM Level 3
     */
    virtual const XMLCh* getSystemId() const = 0;


    /**
     * Get the base URI to be used for resolving relative URIs to absolute
     * URIs. If the baseURI is itself a relative URI, the behavior is
     * implementation dependent.
     *
     * @return The base URI.
     * @see #setBaseURI
     * @since DOM Level 3
     */
    virtual const XMLCh* getBaseURI() const = 0;

    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------
    // -----------------------------------------------------------------------
    /**
     * Sets the UTF-16 string for this input source.
     *
     */
    virtual void setStringData(const XMLCh* data) = 0;

    /**
     * Sets the byte stream for this input source.
     *
     * @see BinInputStream
     */
    virtual void setByteStream(InputSource* stream) = 0;

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
     * @since DOM Level 3
     */
    virtual void setEncoding(const XMLCh* const encodingStr) = 0;


    /**
     * Set the public identifier for this input source.
     *
     * <p>The public identifier is always optional: if the application writer
     * includes one, it will be provided as part of the location information.</p>
     *
     * @param publicId The public identifier as a string.
     * @see #getPublicId
     * @since DOM Level 3
     */
    virtual void setPublicId(const XMLCh* const publicId) = 0;

    /**
     * Set the system identifier for this input source.
     *
     * <p>The system id is always required. The public id may be used to map
     * to another system id, but the system id must always be present as a fall
     * back.</p>
     *
     * <p>If the system ID is a URL, it must be fully resolved.</p>
     *
     * @param systemId The system identifier as a string.
     * @see #getSystemId
     * @since DOM Level 3
     */
    virtual void setSystemId(const XMLCh* const systemId) = 0;

    /**
     * Set the base URI to be used for resolving relative URIs to absolute
     * URIs. If the baseURI is itself a relative URI, the behavior is
     * implementation dependent.
     *
     * @param baseURI The base URI.
     * @see #getBaseURI
     * @since DOM Level 3
     */
    virtual void setBaseURI(const XMLCh* const baseURI) = 0;
    //@}

    // -----------------------------------------------------------------------
    //  Non-standard Extension
    // -----------------------------------------------------------------------
    /** @name Non-standard Extension */
    //@{

    /**
     * Indicates if the parser should issue fatal error if this input source
     * is not found.  If set to false, the parser issue warning message instead.
     *
     * @param  flag True if the parser should issue fatal error if this input source is not found.
     *               If set to false, the parser issue warning message instead.  (Default: true)
     *
     * @see #getIssueFatalErrorIfNotFound
     */
    virtual void setIssueFatalErrorIfNotFound(bool flag) = 0;


    /**
     * Get the flag that indicates if the parser should issue fatal error if this input source
     * is not found.
     *
     * @return True if the parser should issue fatal error if this input source is not found.
     *         False if the parser issue warning message instead.
     * @see #setIssueFatalErrorIfNotFound
     */
    virtual bool getIssueFatalErrorIfNotFound() const = 0;

    /**
     * Called to indicate that this DOMLSInput is no longer in use
     * and that the implementation may relinquish any resources associated with it.
     *
     * Access to a released object will lead to unexpected result.
     */
    virtual void release() = 0;
    //@}
};


XERCES_CPP_NAMESPACE_END

#endif
