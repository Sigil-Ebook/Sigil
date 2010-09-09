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
 * $Id: DOMError.hpp 671894 2008-06-26 13:29:21Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMERROR_HPP)
#define XERCESC_INCLUDE_GUARD_DOMERROR_HPP

#include <xercesc/util/XercesDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class DOMLocator;


/**
  * DOMError is an interface that describes an error.
  *
  * @see DOMErrorHandler#handleError
  * @since DOM Level 3
  */

class CDOM_EXPORT DOMError
{
protected:
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    /** @name Hidden constructors */
    //@{
    DOMError() {};
    //@}

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    /** @name Unimplemented constructors and operators */
    //@{
    DOMError(const DOMError &);
    DOMError & operator = (const DOMError &);
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
    virtual ~DOMError() {};
    //@}

    // -----------------------------------------------------------------------
    //  Class types
    // -----------------------------------------------------------------------
    /** @name Public constants */
    //@{
    /**
     * The severity of the error described by the <code>DOMError</code>.
     *
     * <p><code>DOM_SEVERITY_ERROR:</code>
     * The severity of the error described by the <code>DOMError</code> is error.
     * A DOM_SEVERITY_ERROR may not cause the processing to stop if the error can
     * be recovered, unless <code>DOMErrorHandler::handleError()</code> returns false.</p>
     *
     * <p><code>DOM_SEVERITY_FATAL_ERROR</code>
     * The severity of the error described by the <code>DOMError</code> is fatal error.
     * A DOM_SEVERITY_FATAL_ERROR will cause the normal processing to stop. The return
     * value of <code>DOMErrorHandler::handleError()</code> is ignored unless the
     * implementation chooses to continue, in which case the behavior becomes undefined.</p>
     *
     * <p><code>DOM_SEVERITY_WARNING</code>
     * The severity of the error described by the <code>DOMError</code> is warning.
     * A DOM_SEVERITY_WARNING will not cause the processing to stop, unless
     * <code>DOMErrorHandler::handleError()</code> returns false.</p>
     *
     * @since DOM Level 3
     */
    enum ErrorSeverity
    {
        DOM_SEVERITY_WARNING     = 1,
        DOM_SEVERITY_ERROR       = 2,
        DOM_SEVERITY_FATAL_ERROR = 3
    };
    //@}


    // -----------------------------------------------------------------------
    //  Virtual DOMError interface
    // -----------------------------------------------------------------------
    /** @name Functions introduced in DOM Level 3 */
    //@{
    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    /**
     * Get the severity of the error
     *
     * @see   setSeverity
     * @since DOM Level 3
     */
    virtual ErrorSeverity getSeverity() const = 0;

    /**
     * Get the message describing the error that occured.
     *
     * @since DOM Level 3
     */
    virtual const XMLCh* getMessage() const = 0;

    /**
     * Get the location of the error
     *
     * @since DOM Level 3
     */
    virtual DOMLocator* getLocation() const = 0;

    /**
     * The related platform dependent exception if any.
     *
     * @since DOM Level 3
     */
    virtual void* getRelatedException() const = 0;

    /**
     * A <code>XMLCh*</code> indicating which related data is expected in
     * relatedData. Users should refer to the specification of the error
     * in order to find its <code>XMLCh*</code> type and relatedData
     * definitions if any.
     *
     * Note: As an example, <code>DOMDocument::normalizeDocument()</code> does generate
     * warnings when the "split-cdata-sections" parameter is in use. Therefore, the
     * method generates a DOM_SEVERITY_WARNING with type "cdata-sections-splitted"
     * and the first <code>DOMCDATASection</code> node in document order resulting from the split
     * is returned by the relatedData attribute.
     *
     * @since DOM Level 3
     */
    virtual const XMLCh* getType() const = 0;

    /**
     * The related DOMError::getType dependent data if any.
     *
     * @since DOM Level 3
     */
    virtual void* getRelatedData() const = 0;
    //@}

};

XERCES_CPP_NAMESPACE_END

#endif
