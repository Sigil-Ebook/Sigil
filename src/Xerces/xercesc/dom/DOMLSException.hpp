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
 * $Id: DOMLSException.hpp 671894 2008-06-26 13:29:21Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMLSEXCEPTION_HPP)
#define XERCESC_INCLUDE_GUARD_DOMLSEXCEPTION_HPP

#include <xercesc/dom/DOMException.hpp>

XERCES_CPP_NAMESPACE_BEGIN


/**
 * Parser or write operations may throw an LSException if the processing is stopped.
 * The processing can be stopped due to a <code>DOMError</code> with a severity of
 * DOMError::DOM_SEVERITY_FATAL_ERROR or a non recovered DOMError::DOM_SEVERITY_ERROR,
 * or if <code>DOMErrorHandler::handleError()</code> returned <code>false</code>.
 * <p><b>Note</b>: As suggested in the definition of the constants in the <code>DOMError</code>
 * interface, a DOM implementation may choose to continue after a fatal error, but the
 * resulting DOM tree is then implementation dependent.
 * <p>See also the <a href='http://www.w3.org/TR/2004/REC-DOM-Level-3-LS-20040407/DOM3-LS.html'>
 * Document Object Model (DOM) Level 3 Load and Save Specification</a>.
 * @since DOM Level 3
 */

class MemoryManager;

class CDOM_EXPORT DOMLSException : public DOMException {
public:
    // -----------------------------------------------------------------------
    //  Class Types
    // -----------------------------------------------------------------------
    /** @name Public Contants */
    //@{
    /**
     * ExceptionCode
     *
     * <p><code>PARSE_ERR:</code>
     * If an attempt was made to load a document, or an XML Fragment, using DOMLSParser
     * and the processing has been stopped.</p>
     *
     * <p><code>SERIALIZE_ERR:</code>
     * If an attempt was made to serialize a Node using LSSerializer and the processing
     * has been stopped.</p>
     *
     * @since DOM Level 3
     */
    enum LSExceptionCode {
         PARSE_ERR        = 81,
         SERIALIZE_ERR    = 82
        };
    //@}

    // -----------------------------------------------------------------------
    //  Constructors
    // -----------------------------------------------------------------------
    /** @name Constructors */
    //@{
    /**
      * Default constructor for DOMLSException.
      *
      */
    DOMLSException();

    /**
      * Constructor which takes an error code and a message.
      *
      * @param code           The error code which indicates the exception
      * @param messageCode    The string containing the error message
      * @param memoryManager  The memory manager used to (de)allocate memory
      */
    DOMLSException(short code,
                   short messageCode,
                   MemoryManager* const memoryManager);

    /**
      * Copy constructor.
      *
      * @param other The object to be copied.
      */
    DOMLSException(const DOMLSException &other);

    //@}

    // -----------------------------------------------------------------------
    //  Destructors
    // -----------------------------------------------------------------------
    /** @name Destructor. */
    //@{
	 /**
	  * Destructor for DOMLSException.
	  *
	  */
    virtual ~DOMLSException();
    //@}

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    DOMLSException & operator = (const DOMLSException &);
};

XERCES_CPP_NAMESPACE_END

#endif
