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
 * $Id: XMLDocumentHandler.hpp 673679 2008-07-03 13:50:10Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XMLDOCUMENTHANDLER_HPP)
#define XERCESC_INCLUDE_GUARD_XMLDOCUMENTHANDLER_HPP

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/util/RefVectorOf.hpp>
#include <xercesc/framework/XMLAttr.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XMLElementDecl;
class XMLEntityDecl;

/**
  * This abstract class provides the interface for the scanner to return
  * XML document information up to the parser as it scans through the
  * document.
  *
  * The interface is very similar to org.sax.DocumentHandler, but
  * has some extra methods required to get all the data out.
  */
class XMLPARSER_EXPORT XMLDocumentHandler
{
public:
    // -----------------------------------------------------------------------
    //  Constructors are hidden, just the virtual destructor is exposed
    // -----------------------------------------------------------------------
    /** @name Destructor */
    //@{
    virtual ~XMLDocumentHandler()
    {
    }
    //@}

    /** @name The document handler interface */
    //@{
    /** Receive notification of character data.
      *
      * <p>The scanner will call this method to report each chunk of
      * character data. The scanner may return all contiguous character
      * data in a single chunk, or they may split it into several
      * chunks; however, all of the characters in any single event
      * will come from the same external entity, so that the Locator
      * provides useful information.</p>
      *
      * <p>The parser must not attempt to read from the array
      * outside of the specified range.</p>
      *
      * @param  chars           The content (characters) between markup from the XML
      *                         document.
      * @param  length          The number of characters to read from the array.
      * @param  cdataSection    Indicates that this data is inside a CDATA
      *                         section.
      * @see #ignorableWhitespace
      * @see Locator
      */
    virtual void docCharacters
    (
        const   XMLCh* const    chars
        , const XMLSize_t       length
        , const bool            cdataSection
    ) = 0;

    /** Receive notification of comments in the XML content being parsed.
      *
      * This scanner will call this method for any comments found in the
      * content of the document.
      *
      * @param comment The text of the comment.
      */
    virtual void docComment
    (
        const   XMLCh* const    comment
    ) = 0;

    /** Receive notification of PI's parsed in the XML content.
      *
      * The scanner will call this method for any PIs it finds within the
      * content of the document.
      *
      * @param  target  The name of the PI.
      * @param  data    The body of the PI. This may be an empty string since
      *                 the body is optional.
      */
    virtual void docPI
    (
        const   XMLCh* const    target
        , const XMLCh* const    data
    ) = 0;

    /** Receive notification after the scanner has parsed the end of the
      * document.
      *
      * The scanner will call this method when the current document has been
      * fully parsed. The handler may use this opportunity to do something with
      * the data, clean up temporary data, etc...
      */
    virtual void endDocument() = 0;

    /** Receive notification of the end of an element.
      *
      * This method is called when scanner encounters the end of element tag.
      * There will be a corresponding startElement() event for every
      * endElement() event, but not necessarily the other way around. For
      * empty tags, there is only a startElement() call.
      *
      * @param  elemDecl The name of the element whose end tag was just
      *                     parsed.
      * @param  uriId       The ID of the URI in the URI pool (only valid if
      *                     name spaces is enabled)
      * @param  isRoot      Indicates if this is the root element.
      * @param  prefixName  The string representing the prefix name
      */
    virtual void endElement
    (
        const   XMLElementDecl& elemDecl
        , const unsigned int    uriId
        , const bool            isRoot
        , const XMLCh* const    prefixName = 0
    ) = 0;

    /** Receive notification when a referenced entity's content ends
      *
      * This method is called when scanner encounters the end of an entity
      * reference.
      *
      * @param  entDecl  The name of the entity reference just scanned.
      */
    virtual void endEntityReference
    (
        const   XMLEntityDecl&  entDecl
    ) = 0;

    /** Receive notification of ignorable whitespace in element content.
      *
      * <p>Validating Parsers must use this method to report each chunk
      * of ignorable whitespace (see the W3C XML 1.0 recommendation,
      * section 2.10): non-validating parsers may also use this method
      * if they are capable of parsing and using content models.</p>
      *
      * <p>The scanner may return all contiguous whitespace in a single
      * chunk, or it may split it into several chunks; however, all of
      * the characters in any single event will come from the same
      * external entity, so that the Locator provides useful
      * information.</p>
      *
      * <p>The parser must not attempt to read from the array
      * outside of the specified range.</p>
      *
      * @param  chars       The whitespace characters from the XML document.
      * @param  length      The number of characters to read from the array.
      * @param  cdataSection Indicates that this data is inside a CDATA
      *                     section.
      * @see #docCharacters
      */
    virtual void ignorableWhitespace
    (
        const   XMLCh* const    chars
        , const XMLSize_t       length
        , const bool            cdataSection
    ) = 0;

    /** Reset the document handler's state, if required
      *
      * This method is used to give the registered document handler a
      * chance to reset itself. Its called by the scanner at the start of
      * every parse.
      */
    virtual void resetDocument() = 0;

    /** Receive notification of the start of a new document
      *
      * This method is the first callback called the scanner at the
      * start of every parse. This is before any content is parsed.
      */
    virtual void startDocument() = 0;

    /** Receive notification of a new start tag
      *
      * This method is called when scanner encounters the start of an element tag.
      * All elements must always have a startElement() tag. Empty tags will
      * only have the startElement() tag and no endElement() tag.
      *
      * @param  elemDecl The name of the element whose start tag was just
      *                     parsed.
      * @param  uriId       The ID of the URI in the URI pool (only valid if
      *                     name spaces is enabled)
      * @param  prefixName  The string representing the prefix name
      * @param  attrList    List of attributes in the element
      * @param  attrCount   Count of the attributes in the element
      * @param  isEmpty     Indicates if the element is empty, in which case
      *                     you should not expect an endElement() event.
      * @param  isRoot      Indicates if this is the root element.
      */
    virtual void startElement
    (
        const   XMLElementDecl&         elemDecl
        , const unsigned int            uriId
        , const XMLCh* const            prefixName
        , const RefVectorOf<XMLAttr>&   attrList
        , const XMLSize_t               attrCount
        , const bool                    isEmpty
        , const bool                    isRoot
    ) = 0;

    /** Receive notification when the scanner hits an entity reference.
      *
      * This is currently useful only to DOM parser configurations as SAX
      * does not provide any api to return this information.
      *
      * @param  entDecl  The name of the entity that was referenced.
      */
    virtual void startEntityReference(const XMLEntityDecl& entDecl) = 0;

    /** Receive notification of an XML declaration
      *
      * Currently neither DOM nor SAX provide API's to return back this
      * information.
      *
      * @param  versionStr      The value of the <code>version</code> pseudoattribute
      *                         of the XML decl.
      * @param  encodingStr     The value of the <code>encoding</code> pseudoattribute
      *                         of the XML decl.
      * @param  standaloneStr   The value of the <code>standalone</code>
      *                         pseudoattribute of the XML decl.
      * @param  autoEncodingStr The encoding string auto-detected by the
      *                         scanner. In absence of any 'encoding' attribute in the
      *                         XML decl, the XML standard specifies how a parser can
      *                         auto-detect. If there is no <code>encodingStr</code>
      *                         this is what will be used to try to decode the file.
      */
    virtual void XMLDecl
    (
        const   XMLCh* const    versionStr
        , const XMLCh* const    encodingStr
        , const XMLCh* const    standaloneStr
        , const XMLCh* const    autoEncodingStr
    ) = 0;

    //@}



protected :
    // -----------------------------------------------------------------------
    //  Hidden Constructors
    // -----------------------------------------------------------------------
    XMLDocumentHandler()
    {
    }


private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XMLDocumentHandler(const XMLDocumentHandler&);
    XMLDocumentHandler& operator=(const XMLDocumentHandler&);
};

XERCES_CPP_NAMESPACE_END

#endif
