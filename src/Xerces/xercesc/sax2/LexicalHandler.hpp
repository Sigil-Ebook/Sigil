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
 * $Id: LexicalHandler.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_LEXICALHANDLER_HPP)
#define XERCESC_INCLUDE_GUARD_LEXICALHANDLER_HPP

#include <xercesc/util/XercesDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/**
  * Receive notification of lexical events.
  *
  * <p>This is an extension handler for that provides lexical information
  * about an XML document.  It does not provide information about document
  * content.  For those events, an application must register an instance of
  * a ContentHandler.</p>
  *
  * <p>The order of events in this interface is very important, and
  * mirrors the order of information in the document itself.  For
  * example, startDTD() and endDTD() events will occur before the
  * first element in the document.</p>
  *
  * @see SAX2XMLReader#setLexicalHandler
  * @see SAX2XMLReader#setContentHandler
  */

class SAX2_EXPORT LexicalHandler
{
public:
    /** @name Constructors and Destructor */
    //@{
    /** Default constructor */
    LexicalHandler()
    {
    }

    /** Destructor */
    virtual ~LexicalHandler()
    {
    }
    //@}

    /** @name The virtual document handler interface */

    //@{
   /**
    * Receive notification of comments.
    *
    * <p>The Parser will call this method to report each occurrence of
    * a comment in the XML document.</p>
    *
    * <p>The application must not attempt to read from the array
    * outside of the specified range.</p>
    *
    * @param chars The characters from the XML document.
    * @param length The number of characters to read from the array.
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    */
    virtual void comment
    (
        const   XMLCh* const    chars
        , const XMLSize_t       length
    ) = 0;

  /**
    * Receive notification of the end of a CDATA section.
    *
    * <p>The SAX parser will invoke this method at the end of
    * each CDATA parsed.</p>
    *
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    */
    virtual void endCDATA () = 0;

  /**
    * Receive notification of the end of the DTD declarations.
    *
    * <p>The SAX parser will invoke this method at the end of the
    * DTD</p>
    *
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    */
    virtual void endDTD () = 0;

  /**
    * Receive notification of the end of an entity.
    *
    * <p>The SAX parser will invoke this method at the end of an
    * entity</p>
    *
    * @param name The name of the entity that is ending.
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    */
    virtual void endEntity (const XMLCh* const name) = 0;

  /**
    * Receive notification of the start of a CDATA section.
    *
    * <p>The SAX parser will invoke this method at the start of
    * each CDATA parsed.</p>
    *
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    */
    virtual void startCDATA () = 0;

  /**
    * Receive notification of the start of the DTD declarations.
    *
    * <p>The SAX parser will invoke this method at the start of the
    * DTD</p>
    *
    * @param name The document type name.
    * @param publicId The declared public identifier for the external DTD subset, or null if none was declared.
    * @param systemId The declared system identifier for the external DTD subset, or null if none was declared.
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    */
    virtual void startDTD
    (
        const   XMLCh* const    name
        , const   XMLCh* const    publicId
        , const   XMLCh* const    systemId
    ) = 0;

  /**
    * Receive notification of the start of an entity.
    *
    * <p>The SAX parser will invoke this method at the start of an
    * entity</p>
    *
    * @param name The name of the entity that is starting.
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    */
    virtual void startEntity (const XMLCh* const name) = 0;

    //@}
private :
    /* Unimplemented Constructors and operators */
    /* Copy constructor */
    LexicalHandler(const LexicalHandler&);
    /** Assignment operator */
    LexicalHandler& operator=(const LexicalHandler&);
};

XERCES_CPP_NAMESPACE_END

#endif
