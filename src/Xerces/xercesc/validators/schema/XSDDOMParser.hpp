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
 * $Id: XSDDOMParser.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XSDDOMPARSER_HPP)
#define XERCESC_INCLUDE_GUARD_XSDDOMPARSER_HPP


#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/validators/schema/XSDErrorReporter.hpp>
#include <xercesc/validators/schema/XSDLocator.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class DOMElement;
class XMLValidator;


/**
  * This class is used to parse schema documents into DOM trees
  */
class PARSERS_EXPORT XSDDOMParser : public XercesDOMParser
{
public :

    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------

    /** @name Constructors and Destructor */
    //@{
    /** Construct a XSDDOMParser, with an optional validator
      *
      * Constructor with an instance of validator class to use for
      * validation. If you don't provide a validator, a default one will
      * be created for you in the scanner.
      *
      * @param gramPool   Pointer to the grammar pool instance from
      *                   external application.
      *                   The parser does NOT own it.
      *
      * @param valToAdopt Pointer to the validator instance to use. The
      *                   parser is responsible for freeing the memory.
      */
    XSDDOMParser
    (
          XMLValidator* const   valToAdopt = 0
        , MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
        , XMLGrammarPool* const gramPool = 0
    );

    /**
      * Destructor
      */
    ~XSDDOMParser();

    //@}

    // -----------------------------------------------------------------------
    //  Implementation of the XMLDocumentHandler interface.
    // -----------------------------------------------------------------------

    /** @name Implementation of the XMLDocumentHandler interface. */
    //@{

    /** Handle a start element event
      *
      * This method is used to report the start of an element. It is
      * called at the end of the element, by which time all attributes
      * specified are also parsed. A new DOM Element node is created
      * along with as many attribute nodes as required. This new element
      * is added appended as a child of the current node in the tree, and
      * then replaces it as the current node (if the isEmpty flag is false.)
      *
      * @param elemDecl A const reference to the object containing element
      *                 declaration information.
      * @param urlId    An id referring to the namespace prefix, if
      *                 namespaces setting is switched on.
      * @param elemPrefix A const pointer to a Unicode string containing
      *                 the namespace prefix for this element. Applicable
      *                 only when namespace processing is enabled.
      * @param attrList A const reference to the object containing the
      *                 list of attributes just scanned for this element.
      * @param attrCount A count of number of attributes in the list
      *                 specified by the parameter 'attrList'.
      * @param isEmpty  A flag indicating whether this is an empty element
      *                 or not. If empty, then no endElement() call will
      *                 be made.
      * @param isRoot   A flag indicating whether this element was the
      *                 root element.
      * @see DocumentHandler#startElement
      */
    virtual void startElement
    (
        const   XMLElementDecl&         elemDecl
        , const unsigned int            urlId
        , const XMLCh* const            elemPrefix
        , const RefVectorOf<XMLAttr>&   attrList
        , const XMLSize_t               attrCount
        , const bool                    isEmpty
        , const bool                    isRoot
    );

    /** Handle and end of element event
      *
      * This method is used to indicate the end tag of an element. The
      * DOM parser pops the current element off the top of the element
      * stack, and make it the new current element.
      *
      * @param elemDecl A const reference to the object containing element
      *                 declaration information.
      * @param urlId    An id referring to the namespace prefix, if
      *                 namespaces setting is switched on.
      * @param isRoot   A flag indicating whether this element was the
      *                 root element.
      * @param elemPrefix A const pointer to a Unicode string containing
      *                 the namespace prefix for this element. Applicable
      *                 only when namespace processing is enabled.
      */
    virtual void endElement
    (
        const   XMLElementDecl& elemDecl
        , const unsigned int    urlId
        , const bool            isRoot
        , const XMLCh* const    elemPrefix
    );

    /** Handle document character events
      *
      * This method is used to report all the characters scanned by the
      * parser. This DOM implementation stores this data in the appropriate
      * DOM node, creating one if necessary.
      *
      * @param chars   A const pointer to a Unicode string representing the
      *                character data.
      * @param length  The length of the Unicode string returned in 'chars'.
      * @param cdataSection  A flag indicating if the characters represent
      *                      content from the CDATA section.
      */
    virtual void docCharacters
    (
        const   XMLCh* const    chars
        , const XMLSize_t       length
        , const bool            cdataSection
    );

    /** Handle a document comment event
      *
      * This method is used to report any comments scanned by the parser.
      * A new comment node is created which stores this data.
      *
      * @param comment A const pointer to a null terminated Unicode
      *                string representing the comment text.
      */
    virtual void docComment
    (
        const   XMLCh* const    comment
    );

    /** Handle a start entity reference event
      *
      * This method is used to indicate the start of an entity reference.
      * If the expand entity reference flag is true, then a new
      * DOM Entity reference node is created.
      *
      * @param entDecl A const reference to the object containing the
      *                entity declaration information.
      */
    virtual void startEntityReference
    (
        const   XMLEntityDecl&  entDecl
    );

    /** Handle and end of entity reference event
      *
      * This method is used to indicate that an end of an entity reference
      * was just scanned.
      *
      * @param entDecl A const reference to the object containing the
      *                entity declaration information.
      */
    virtual void endEntityReference
    (
        const   XMLEntityDecl&  entDecl
    );

    /** Handle an ignorable whitespace vent
      *
      * This method is used to report all the whitespace characters, which
      * are determined to be 'ignorable'. This distinction between characters
      * is only made, if validation is enabled.
      *
      * Any whitespace before content is ignored. If the current node is
      * already of type DOMNode::TEXT_NODE, then these whitespaces are
      * appended, otherwise a new Text node is created which stores this
      * data. Essentially all contiguous ignorable characters are collected
      * in one node.
      *
      * @param chars   A const pointer to a Unicode string representing the
      *                ignorable whitespace character data.
      * @param length  The length of the Unicode string 'chars'.
      * @param cdataSection  A flag indicating if the characters represent
      *                      content from the CDATA section.
      */
    virtual void ignorableWhitespace
    (
        const   XMLCh* const    chars
        , const XMLSize_t       length
        , const bool            cdataSection
    );

    //@}

    // -----------------------------------------------------------------------
    //  Get methods
    // -----------------------------------------------------------------------
    bool getSawFatal() const;


    // -----------------------------------------------------------------------
    //  Set methods
    // -----------------------------------------------------------------------
    void setUserErrorReporter(XMLErrorReporter* const errorReporter);
    void setUserEntityHandler(XMLEntityHandler* const entityHandler);


    // -----------------------------------------------------------------------
    //  XMLErrorReporter interface
    // -----------------------------------------------------------------------
    virtual void error
    (
        const   unsigned int        errCode
        , const XMLCh* const        errDomain
        , const ErrTypes            type
        , const XMLCh* const        errorText
        , const XMLCh* const        systemId
        , const XMLCh* const        publicId
        , const XMLFileLoc          lineNum
        , const XMLFileLoc          colNum
    );

    // -----------------------------------------------------------------------
    //  XMLEntityHandler interface
    // -----------------------------------------------------------------------
    virtual InputSource* resolveEntity(XMLResourceIdentifier* resourceIdentifier);

protected :
    // -----------------------------------------------------------------------
    //  Protected Helper methods
    // -----------------------------------------------------------------------
    virtual DOMElement* createElementNSNode(const XMLCh *fNamespaceURI,
                                            const XMLCh *qualifiedName);

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XSDDOMParser(const XSDDOMParser&);
    XSDDOMParser& operator=(const XSDDOMParser&);

    // -----------------------------------------------------------------------
    //  Private Helper methods
    // -----------------------------------------------------------------------
    void startAnnotation
    (
        const   XMLElementDecl&         elemDecl
        , const RefVectorOf<XMLAttr>&   attrList
        , const XMLSize_t               attrCount
    );
    void startAnnotationElement
    (
        const   XMLElementDecl&         elemDecl
        , const RefVectorOf<XMLAttr>&   attrList
        , const XMLSize_t               attrCount
    );
    void endAnnotationElement
    (
        const XMLElementDecl& elemDecl
        ,     bool            complete
    );

    // -----------------------------------------------------------------------
    //  Private data members
    // -----------------------------------------------------------------------
    bool                         fSawFatal;
    int                          fAnnotationDepth;
    int                          fInnerAnnotationDepth;
    int                          fDepth;
    XMLErrorReporter*            fUserErrorReporter;
    XMLEntityHandler*            fUserEntityHandler;
    ValueVectorOf<unsigned int>* fURIs;
    XMLBuffer                    fAnnotationBuf;
    XSDErrorReporter             fXSDErrorReporter;
    XSDLocator                   fXSLocator;
};


inline bool XSDDOMParser::getSawFatal() const
{
    return fSawFatal;
}

XERCES_CPP_NAMESPACE_END

#endif
