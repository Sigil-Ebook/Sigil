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
 * $Id: SAXParseException.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_SAXPARSEEXCEPTION_HPP)
#define XERCESC_INCLUDE_GUARD_SAXPARSEEXCEPTION_HPP

#include <xercesc/sax/SAXException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class Locator;

/**
  * Encapsulate an XML parse error or warning.
  *
  * <p>This exception will include information for locating the error
  * in the original XML document.  Note that although the application
  * will receive a SAXParseException as the argument to the handlers
  * in the ErrorHandler interface, the application is not actually
  * required to throw the exception; instead, it can simply read the
  * information in it and take a different action.</p>
  *
  * <p>Since this exception is a subclass of SAXException, it
  * inherits the ability to wrap another exception.</p>
  *
  * @see SAXException#SAXException
  * @see Locator#Locator
  * @see ErrorHandler#ErrorHandler
  */
class SAX_EXPORT SAXParseException : public SAXException
{
public:
    /** @name Constructors and Destructor */
    //@{
  /**
    * Create a new SAXParseException from a message and a Locator.
    *
    * <p>This constructor is especially useful when an application is
    * creating its own exception from within a DocumentHandler
    * callback.</p>
    *
    * @param message The error or warning message.
    * @param locator The locator object for the error or warning.
    * @param manager    Pointer to the memory manager to be used to
    *                   allocate objects.
    * @see Locator#Locator
    * @see Parser#setLocale
    */
    SAXParseException(const XMLCh* const message, const Locator& locator,
                      MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);


  /**
    * Create a new SAXParseException.
    *
    * <p>This constructor is most useful for parser writers.</p>
    *
    * <p>If the system identifier is a URL, the parser must resolve it
    * fully before creating the exception.</p>
    *
    * @param message The error or warning message.
    * @param publicId The public identifier of the entity that generated
    *                 the error or warning.
    * @param systemId The system identifier of the entity that generated
    *                 the error or warning.
    * @param lineNumber The line number of the end of the text that
    *                   caused the error or warning.
    * @param columnNumber The column number of the end of the text that
    *                     caused the error or warning.
    * @param manager    Pointer to the memory manager to be used to
    *                   allocate objects.
    * @see Parser#setLocale
    */
    SAXParseException
    (
        const   XMLCh* const    message
        , const XMLCh* const    publicId
        , const XMLCh* const    systemId
        , const XMLFileLoc   lineNumber
        , const XMLFileLoc   columnNumber
        , MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
    );

  /**
    * Copy constructor
    *
    * @param toCopy The object to be copied
    */
    SAXParseException(const SAXParseException& toCopy);

    /**
      * Destructor
      */
    ~SAXParseException();

    //@}

    /** @name Assignment operator */
    //@{
   /**
    * Assignment operator
    *
    * @param toAssign The object to be copied through assignment
    *
    */
    SAXParseException& operator=(const SAXParseException& toAssign);
    //@}

    /** @name Getter methods */
    //@{
   /**
    * The column number of the end of the text where the exception occurred.
    *
    * <p>The first column in a line is position 1.</p>
    *
    * @return An integer representing the column number, or 0
    *         if none is available.
    * @see Locator#getColumnNumber
    */
    XMLFileLoc getColumnNumber() const;
  /**
    * The line number of the end of the text where the exception occurred.
    *
    * @return An integer representing the line number, or 0
    *         if none is available.
    * @see Locator#getLineNumber
    */
    XMLFileLoc getLineNumber() const;
  /**
    * Get the public identifier of the entity where the exception occurred.
    *
    * @return A string containing the public identifier, or null
    *         if none is available.
    * @see Locator#getPublicId
    */
    const XMLCh* getPublicId() const;
  /**
    * Get the system identifier of the entity where the exception occurred.
    *
    * <p>If the system identifier is a URL, it will be resolved
    * fully.</p>
    *
    * @return A string containing the system identifier, or null
    *         if none is available.
    * @see Locator#getSystemId
    */
    const XMLCh* getSystemId() const;
    //@}

private:
    /* Data Members */

    /* The column in the source text where the error occured. */
    XMLFileLoc   fColumnNumber;
    /* The line in the source text where the error occured. */
    XMLFileLoc   fLineNumber;
    /* The public id of the file where the error occured. */
    XMLCh*          fPublicId;
    /* The system id of the file where the error occured. */
    XMLCh*          fSystemId;


};

XERCES_CPP_NAMESPACE_END

#endif
