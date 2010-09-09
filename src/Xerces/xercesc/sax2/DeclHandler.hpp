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
 * $Id: DeclHandler.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DECLHANDLER_HPP)
#define XERCESC_INCLUDE_GUARD_DECLHANDLER_HPP

#include <xercesc/util/XercesDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/**
  * Receive notification of DTD declaration events.
  *
  * <p>This is an optional extension handler for SAX2 to provide more
  * complete information about DTD declarations in an XML document.
  * XML readers are not required to recognize this handler, and it is not
  * part of core-only SAX2 distributions.</p>
  *
  * <p>Note that data-related DTD declarations (unparsed entities and
  * notations) are already reported through the DTDHandler interface.</p>
  *
  * <p>If you are using the declaration handler together with a lexical
  * handler, all of the events will occur between the startDTD and the endDTD
  * events.</p>
  *
  * @see SAX2XMLReader#setLexicalHandler
  * @see SAX2XMLReader#setDeclarationHandler
  */

class SAX2_EXPORT DeclHandler
{
public:
    /** @name Constructors and Destructor */
    //@{
    /** Default constructor */
    DeclHandler()
    {
    }

    /** Destructor */
    virtual ~DeclHandler()
    {
    }
    //@}

    /** @name The virtual declaration handler interface */

    //@{
   /**
    * Report an element type declaration.
    *
    * <p>The content model will consist of the string "EMPTY", the string
    * "ANY", or a parenthesised group, optionally followed by an occurrence
    * indicator. The model will be normalized so that all parameter entities
    * are fully resolved and all whitespace is removed,and will include the
    * enclosing parentheses. Other normalization (such as removing redundant
    * parentheses or simplifying occurrence indicators) is at the discretion
    * of the parser.</p>
    *
    * @param name The element type name.
    * @param model The content model as a normalized string.
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    */
    virtual void elementDecl
    (
        const   XMLCh* const    name
        , const XMLCh* const    model
    ) = 0;

   /**
    * Report an attribute type declaration.
    *
    * <p>The Parser will call this method to report each occurrence of
    * a comment in the XML document.</p>
    *
    * <p>The application must not attempt to read from the array
    * outside of the specified range.</p>
    *
    * @param eName The name of the associated element.
    * @param aName The name of the attribute.
    * @param type A string representing the attribute type.
    * @param mode A string representing the attribute defaulting mode ("#IMPLIED", "#REQUIRED", or "#FIXED") or null if none of these applies.
    * @param value A string representing the attribute's default value, or null if there is none.
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    */
    virtual void attributeDecl
    (
        const   XMLCh* const    eName
        , const XMLCh* const    aName
        , const XMLCh* const    type
        , const XMLCh* const    mode
        , const XMLCh* const    value
    ) = 0;

   /**
    * Report an internal entity declaration.
    *
    * <p>Only the effective (first) declaration for each entity will be
    * reported. All parameter entities in the value will be expanded, but
    * general entities will not.</p>
    *
    * @param name The name of the entity. If it is a parameter entity, the name will begin with '%'.
    * @param value The replacement text of the entity.
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    */
    virtual void internalEntityDecl
    (
        const   XMLCh* const    name
        , const XMLCh* const    value
    ) = 0;

   /**
    * Report a parsed external entity declaration.
    *
    * <p>Only the effective (first) declaration for each entity will
    * be reported.</p>
    *
    * @param name The name of the entity. If it is a parameter entity, the name will begin with '%'.
    * @param publicId The The declared public identifier of the entity, or null if none was declared.
    * @param systemId The declared system identifier of the entity.
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    */
    virtual void externalEntityDecl
    (
        const   XMLCh* const    name
        , const XMLCh* const    publicId
        , const XMLCh* const    systemId
    ) = 0;

    //@}
private :
    /* Unimplemented Constructors and operators */
    /* Copy constructor */
    DeclHandler(const DeclHandler&);
    /** Assignment operator */
    DeclHandler& operator=(const DeclHandler&);
};

XERCES_CPP_NAMESPACE_END

#endif
