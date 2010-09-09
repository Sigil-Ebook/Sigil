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
 * $Id: DOMException.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMEXCEPTION_HPP)
#define XERCESC_INCLUDE_GUARD_DOMEXCEPTION_HPP

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/util/PlatformUtils.hpp>

XERCES_CPP_NAMESPACE_BEGIN


/**
 * DOM operations only raise exceptions in "exceptional" circumstances, i.e.,
 * when an operation is impossible to perform (either for logical reasons,
 * because data is lost, or because the implementation has become unstable).
 * In general, DOM methods return specific error values in ordinary
 * processing situations, such as out-of-bound errors when using
 * <code>DOMNodeList</code>.
 * <p>Implementations should raise other exceptions under other circumstances.
 * For example, implementations should raise an implementation-dependent
 * exception if a <code>null</code> argument is passed.
 * <p>Some languages and object systems do not support the concept of
 * exceptions. For such systems, error conditions may be indicated using
 * native error reporting mechanisms. For some bindings, for example,
 * methods may return error codes similar to those listed in the
 * corresponding method descriptions.
 * <p>See also the <a href='http://www.w3.org/TR/2000/REC-DOM-Level-2-Core-20001113'>Document Object Model (DOM) Level 2 Core Specification</a>.
 * @since DOM Level 1
 */

class MemoryManager;

class CDOM_EXPORT DOMException  {
public:
    // -----------------------------------------------------------------------
    //  Class Types
    // -----------------------------------------------------------------------
    /** @name Public Constants */
    //@{
    /**
     * ExceptionCode
     *
     * <p><code>INDEX_SIZE_ERR:</code>
     * If index or size is negative, or greater than the allowed value.</p>
     *
     * <p><code>DOMSTRING_SIZE_ERR:</code>
     * If the specified range of text does not fit into a DOMString.</p>
     *
     * <p><code>HIERARCHY_REQUEST_ERR:</code>
     * If any node is inserted somewhere it doesn't belong.</p>
     *
     * <p><code>WRONG_DOCUMENT_ERR:</code>
     * If a node is used in a different document than the one that created it
     * (that doesn't support it).</p>
     *
     * <p><code>INVALID_CHARACTER_ERR:</code>
     * If an invalid or illegal character is specified, such as in a name. See
     * production 2 in the XML specification for the definition of a legal
     * character, and production 5 for the definition of a legal name
     * character.</p>
     *
     * <p><code>NO_DATA_ALLOWED_ERR:</code>
     * If data is specified for a node which does not support data.</p>
     *
     * <p><code>NO_MODIFICATION_ALLOWED_ERR:</code>
     * If an attempt is made to modify an object where modifications are not
     * allowed.</p>
     *
     * <p><code>NOT_FOUND_ERR:</code>
     * If an attempt is made to reference a node in a context where it does
     * not exist.</p>
     *
     * <p><code>NOT_SUPPORTED_ERR:</code>
     * If the implementation does not support the requested type of object or
     * operation.</p>
     *
     * <p><code>INUSE_ATTRIBUTE_ERR:</code>
     * If an attempt is made to add an attribute that is already in use
     * elsewhere.</p>
     *
     * The above are since DOM Level 1
     * @since DOM Level 1
     *
     * <p><code>INVALID_STATE_ERR:</code>
     * If an attempt is made to use an object that is not, or is no longer,
     * usable.</p>
     *
     * <p><code>SYNTAX_ERR:</code>
     * If an invalid or illegal string is specified.</p>
     *
     * <p><code>INVALID_MODIFICATION_ERR:</code>
     * If an attempt is made to modify the type of the underlying object.</p>
     *
     * <p><code>NAMESPACE_ERR:</code>
     * If an attempt is made to create or change an object in a way which is
     * incorrect with regard to namespaces.</p>
     *
     * <p><code>INVALID_ACCESS_ERR:</code>
     * If a parameter or an operation is not supported by the underlying
     * object.
     *
     * The above are since DOM Level 2
     * @since DOM Level 2
     *
     * <p><code>VALIDATION_ERR:</code>
     * If a call to a method such as <code>insertBefore</code> or
     * <code>removeChild</code> would make the <code>Node</code> invalid
     * with respect to "partial validity", this exception would be raised
     * and the operation would not be done.
     *
     * <p><code>TYPE_MISMATCH_ERR:</code>
     * If the type of an object is incompatible with the expected type of
     * the parameter associated to the object, this exception would be raised.
     *
     * The above is since DOM Level 3
     * @since DOM Level 3
     */
    enum ExceptionCode {
         INDEX_SIZE_ERR                 = 1,
         DOMSTRING_SIZE_ERR             = 2,
         HIERARCHY_REQUEST_ERR          = 3,
         WRONG_DOCUMENT_ERR             = 4,
         INVALID_CHARACTER_ERR          = 5,
         NO_DATA_ALLOWED_ERR            = 6,
         NO_MODIFICATION_ALLOWED_ERR    = 7,
         NOT_FOUND_ERR                  = 8,
         NOT_SUPPORTED_ERR              = 9,
         INUSE_ATTRIBUTE_ERR            = 10,
         INVALID_STATE_ERR              = 11,
         SYNTAX_ERR                     = 12,
         INVALID_MODIFICATION_ERR       = 13,
         NAMESPACE_ERR                  = 14,
         INVALID_ACCESS_ERR             = 15,
         VALIDATION_ERR                 = 16,
         TYPE_MISMATCH_ERR              = 17
        };
    //@}

public:
    // -----------------------------------------------------------------------
    //  Constructors
    // -----------------------------------------------------------------------
    /** @name Constructors */
    //@{
    /**
      * Default constructor for DOMException.
      *
      */
    DOMException();

    /**
      * Constructor which takes an error code and an optional message code.
      *
      * @param code           The error code which indicates the exception
      * @param messageCode    The string containing the error message
      * @param memoryManager  The memory manager used to (de)allocate memory
      */
    DOMException(short code,
                 short messageCode = 0,
                 MemoryManager* const  memoryManager = XMLPlatformUtils::fgMemoryManager);

    /**
      * Copy constructor.
      *
      * @param other The object to be copied.
      */
    DOMException(const DOMException &other);

    //@}

    // -----------------------------------------------------------------------
    //  Destructors
    // -----------------------------------------------------------------------
    /** @name Destructor. */
    //@{
	 /**
	  * Destructor for DOMException.
	  *
	  */
    virtual ~DOMException();
    //@}


public:
    // -----------------------------------------------------------------------
    //  Getter
    // -----------------------------------------------------------------------
    inline const XMLCh* getMessage()    const;

    // -----------------------------------------------------------------------
    //  Class Types
    // -----------------------------------------------------------------------
    /** @name Public variables */
    //@{
    /**
     * A code value, from the set defined by the ExceptionCode enum,
     * indicating the type of error that occured.
     */
    short   code;

    /**
     * A string value.  Applications may use this field to hold an error
     *  message.  The field value is not set by the DOM implementation,
     *  meaning that the string will be empty when an exception is first
     *  thrown.
     */
    const XMLCh *msg;
    //@}

protected:
    MemoryManager*  fMemoryManager;

private:

     /**
      * A boolean value.
      *   If the message is provided by the applications, it is not
      *   adopted.
      *   If the message is resolved by the DOM implementation, it is
      *   owned.
      */
    bool            fMsgOwned;

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    DOMException & operator = (const DOMException &);
};

inline const XMLCh* DOMException::getMessage() const
{
    return msg;
}

XERCES_CPP_NAMESPACE_END

#endif
