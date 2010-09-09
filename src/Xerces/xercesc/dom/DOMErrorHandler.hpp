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
 * $Id: DOMErrorHandler.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMERRORHANDLER_HPP)
#define XERCESC_INCLUDE_GUARD_DOMERRORHANDLER_HPP

#include <xercesc/util/XercesDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN


class DOMError;

/**
  * Basic interface for DOM error handlers.
  *
  * <p>DOMErrorHandler is a callback interface that the DOM implementation
  * can call when reporting errors that happens while processing XML data, or
  * when doing some other processing (e.g. validating a document).</p>
  *
  * <p>The application that is using the DOM implementation is expected to
  * implement this interface.</p>
  *
  * @see DOMLSParser#getDomConfig
  * @since DOM Level 3
  */

class CDOM_EXPORT DOMErrorHandler
{
protected:
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    /** @name Hidden constructors */
    //@{    
    DOMErrorHandler() {};
    //@}

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    /** @name Unimplemented constructors and operators */
    //@{
    DOMErrorHandler(const DOMErrorHandler &);
    DOMErrorHandler & operator = (const DOMErrorHandler &);
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
    virtual ~DOMErrorHandler() {};
    //@}

    // -----------------------------------------------------------------------
    //  Virtual DOMErrorHandler interface
    // -----------------------------------------------------------------------
    /** @name Functions introduced in DOM Level 3 */
    //@{
    /**
     * This method is called on the error handler when an error occurs.
     * If an exception is thrown from this method, it is considered to be equivalent of returning <code>true</code>.
     *
     * @param domError The error object that describes the error, this object
     *                 may be reused by the DOM implementation across multiple
     *                 calls to the handleError method.
     * @return If the handleError method returns <code>true</code> the DOM
     *         implementation should continue as if the error didn't happen
     *         when possible, if the method returns <code>false</code> then the
     *         DOM implementation should stop the current processing when
     *         possible.
     *
     * @since DOM Level 3
     */
    virtual bool handleError(const DOMError& domError) = 0;
    //@}

};

XERCES_CPP_NAMESPACE_END

#endif
