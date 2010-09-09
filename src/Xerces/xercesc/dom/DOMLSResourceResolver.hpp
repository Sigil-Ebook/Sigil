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
 * $Id: DOMLSResourceResolver.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMLSRESOURCERESOLVER_HPP)
#define XERCESC_INCLUDE_GUARD_DOMLSRESOURCERESOLVER_HPP

#include <xercesc/util/XercesDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN


class DOMLSInput;

/**
  * DOMLSResourceResolver provides a way for applications to redirect references
  * to external entities.
  *
  * <p>Applications needing to implement customized handling for external
  * entities must implement this interface and register their implementation
  * by setting the entityResolver attribute of the DOMLSParser.</p>
  *
  * <p>The DOMLSParser will then allow the application to intercept any
  * external entities (including the external DTD subset and external parameter
  * entities) before including them.</p>
  *
  * <p>Many DOM applications will not need to implement this interface, but it
  * will be especially useful for applications that build XML documents from
  * databases or other specialized input sources, or for applications that use
  * URNs.</p>
  *
  * @see DOMLSParser#getDomConfig
  * @see DOMLSInput#DOMLSInput
  * @since DOM Level 3
  */
class CDOM_EXPORT DOMLSResourceResolver
{
protected:
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    /** @name Hidden constructors */
    //@{    
    DOMLSResourceResolver() {};
    //@}

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    /** @name Unimplemented constructors and operators */
    //@{
    DOMLSResourceResolver(const DOMLSResourceResolver &);
    DOMLSResourceResolver & operator = (const DOMLSResourceResolver &);
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
    virtual ~DOMLSResourceResolver() {};
    //@}

    // -----------------------------------------------------------------------
    //  Virtual DOMLSResourceResolver interface
    // -----------------------------------------------------------------------
    /** @name Functions introduced in DOM Level 3 */
    //@{
    /**
     * Allow the application to resolve external resources.
     *
     * The <code>DOMLSParser</code> will call this method before opening any external resource, 
     * including the external DTD subset, external entities referenced within the DTD, and 
     * external entities referenced within the document element (however, the top-level 
     * document entity is not passed to this method). The application may then request that 
     * the <code>DOMLSParser</code> resolve the external resource itself, that it use an 
     * alternative URI, or that it use an entirely different input source.
     *
     * Application writers can use this method to redirect external system identifiers to 
     * secure and/or local URI, to look up public identifiers in a catalogue, or to read 
     * an entity from a database or other input source (including, for example, a dialog box).
     *
     * The returned DOMLSInput is owned by the DOMLSParser which is
     * responsible to clean up the memory.
     *
     * @param resourceType The type of the resource being resolved. For XML [XML 1.0] resources 
     *                     (i.e. entities), applications must use the value "http://www.w3.org/TR/REC-xml". 
     *                     For XML Schema [XML Schema Part 1], applications must use the value 
     *                     "http://www.w3.org/2001/XMLSchema". Other types of resources are outside 
     *                     the scope of this specification and therefore should recommend an absolute 
     *                     URI in order to use this method.
     * @param namespaceUri The namespace of the resource being resolved, e.g. the target namespace 
     *                     of the XML Schema [XML Schema Part 1] when resolving XML Schema resources.
     * @param publicId     The public identifier of the external entity being referenced, or <code>null</code> 
     *                     if no public identifier was supplied or if the resource is not an entity.
     * @param systemId     The system identifier, a URI reference [IETF RFC 2396], of the external 
     *                     resource being referenced, or <code>null</code> if no system identifier was supplied.
     * @param baseURI      The absolute base URI of the resource being parsed, or <code>null</code> if 
     *                     there is no base URI.
     * @return A DOMLSInput object describing the new input source,
     *         or <code>null</code> to request that the parser open a regular
     *         URI connection to the resource.
     *         The returned DOMLSInput is owned by the DOMLSParser which is
     *         responsible to clean up the memory.
     * @see DOMLSInput#DOMLSInput
     * @since DOM Level 3
     */
    virtual DOMLSInput* resolveResource(  const XMLCh* const    resourceType
                                        , const XMLCh* const    namespaceUri
                                        , const XMLCh* const    publicId
                                        , const XMLCh* const    systemId
                                        , const XMLCh* const    baseURI) = 0;

    //@}

};

XERCES_CPP_NAMESPACE_END

#endif
