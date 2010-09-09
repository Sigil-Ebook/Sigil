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
 * $Id: DOMLocator.hpp 676853 2008-07-15 09:58:05Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMLOCATOR_HPP)
#define XERCESC_INCLUDE_GUARD_DOMLOCATOR_HPP

#include <xercesc/util/XercesDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN


class DOMNode;


/**
  * DOMLocator is an interface that describes a location. (e.g. where an error
  * occured).
  *
  * @see DOMError#DOMError
  * @since DOM Level 3
  */

class CDOM_EXPORT DOMLocator
{
protected:
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    /** @name Hidden constructors */
    //@{
    DOMLocator() {};
    //@}

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    /** @name Unimplemented constructors and operators */
    //@{
    DOMLocator(const DOMLocator &);
    DOMLocator & operator = (const DOMLocator &);
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
    virtual ~DOMLocator() {};
    //@}

    // -----------------------------------------------------------------------
    //  Virtual DOMLocator interface
    // -----------------------------------------------------------------------
    /** @name Functions introduced in DOM Level 3 */
    //@{
    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    /**
     * Get the line number where the error occured, or 0 if there is
     * no line number available.
     *
     * @since DOM Level 3
     */
    virtual XMLFileLoc getLineNumber() const = 0;

    /**
     * Get the column number where the error occured, or 0 if there
     * is no column number available.
     *
     * @since DOM Level 3
     */
    virtual XMLFileLoc getColumnNumber() const = 0;

    /**
     * Get the byte offset into the input source, or ~(XMLFilePos(0)) if
     * there is no byte offset available.
     *
     * @since DOM Level 3
     */
    virtual XMLFilePos getByteOffset() const = 0;

    /**
     * Get the UTF-16 offset into the input source, or ~(XMLFilePos(0)) if
     * there is no UTF-16 offset available.
     *
     * @since DOM Level 3
     */
    virtual XMLFilePos getUtf16Offset() const = 0;

    /**
     * Get the DOMNode where the error occured, or <code>null</code> if there
     * is no node available.
     *
     * @since DOM Level 3
     */
    virtual DOMNode* getRelatedNode() const = 0;

    /**
     * Get the URI where the error occured, or <code>null</code> if there is no
     * URI available.
     *
     * @since DOM Level 3
     */
    virtual const XMLCh* getURI() const = 0;
    //@}
};

XERCES_CPP_NAMESPACE_END

#endif
