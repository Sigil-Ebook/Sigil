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
 * $Id: DTDHandler.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DTDHANDLER_HPP)
#define XERCESC_INCLUDE_GUARD_DTDHANDLER_HPP

#include <xercesc/util/XercesDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/**
  * Receive notification of basic DTD-related events.
  *
  * <p>If a SAX application needs information about notations and
  * unparsed entities, then the application implements this
  * interface and registers an instance with the SAX parser using
  * the parser's setDTDHandler method.  The parser uses the
  * instance to report notation and unparsed entity declarations to
  * the application.</p>
  *
  * <p>The SAX parser may report these events in any order, regardless
  * of the order in which the notations and unparsed entities were
  * declared; however, all DTD events must be reported after the
  * document handler's startDocument event, and before the first
  * startElement event.</p>
  *
  * <p>It is up to the application to store the information for
  * future use (perhaps in a hash table or object tree).
  * If the application encounters attributes of type "NOTATION",
  * "ENTITY", or "ENTITIES", it can use the information that it
  * obtained through this interface to find the entity and/or
  * notation corresponding with the attribute value.</p>
  *
  * <p>The HandlerBase class provides a default implementation
  * of this interface, which simply ignores the events.</p>
  *
  * @see Parser#setDTDHandler
  * @see HandlerBase#HandlerBase
  */

class SAX_EXPORT DTDHandler
{
public:
    /** @name Constructors and Destructor */
    //@{
    /** Default Constructor */
    DTDHandler()
    {
    }

    /** Destructor */
    virtual ~DTDHandler()
    {
    }

    //@}

    /** @name The DTD handler interface */
    //@{
  /**
    * Receive notification of a notation declaration event.
    *
    * <p>It is up to the application to record the notation for later
    * reference, if necessary.</p>
    *
    * <p>If a system identifier is present, and it is a URL, the SAX
    * parser must resolve it fully before passing it to the
    * application.</p>
    *
    * @param name The notation name.
    * @param publicId The notation's public identifier, or null if
    *        none was given.
    * @param systemId The notation's system identifier, or null if
    *        none was given.
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    * @see #unparsedEntityDecl
    * @see AttributeList#AttributeList
    */
	virtual void notationDecl
    (
        const   XMLCh* const    name
        , const XMLCh* const    publicId
        , const XMLCh* const    systemId
    ) = 0;

  /**
    * Receive notification of an unparsed entity declaration event.
    *
    * <p>Note that the notation name corresponds to a notation
    * reported by the notationDecl() event.  It is up to the
    * application to record the entity for later reference, if
    * necessary.</p>
    *
    * <p>If the system identifier is a URL, the parser must resolve it
    * fully before passing it to the application.</p>
    *
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    * @param name The unparsed entity's name.
    * @param publicId The entity's public identifier, or null if none
    *        was given.
    * @param systemId The entity's system identifier (it must always
    *        have one).
    * @param notationName The name of the associated notation.
    * @see #notationDecl
    * @see AttributeList#AttributeList
    */
    virtual void unparsedEntityDecl
    (
        const   XMLCh* const    name
        , const XMLCh* const    publicId
        , const XMLCh* const    systemId
        , const XMLCh* const    notationName
    ) = 0;

    /**
    * Reset the DocType object on its reuse
    *
    * <p>This method helps in reseting the DTD object implementation
    * defaults each time the DTD is begun.</p>
    *
    */
    virtual void resetDocType() = 0;

    //@}

private :
    /* Unimplemented constructors and operators */

    /* Copy constructor */
    DTDHandler(const DTDHandler&);

    /* Assignment operator */
    DTDHandler& operator=(const DTDHandler&);

};

XERCES_CPP_NAMESPACE_END

#endif
