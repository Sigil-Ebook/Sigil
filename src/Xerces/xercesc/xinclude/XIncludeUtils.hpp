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
 * $Id: XIncludeUtils.hpp 673949 2008-07-04 08:04:44Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XINCLUDEUTILS_HPP)
#define XERCESC_INCLUDE_GUARD_XINCLUDEUTILS_HPP

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/util/XMLException.hpp>
#include <xercesc/dom/DOMNode.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMErrorHandler.hpp>
#include <xercesc/framework/XMLElementDecl.hpp>
#include <xercesc/xinclude/XIncludeLocation.hpp>
#include <xercesc/framework/XMLErrorCodes.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XMLEntityHandler;

typedef struct XIncludeHistoryNode{
    XMLCh *URI;
    struct XIncludeHistoryNode *next;
}XIncludeHistoryNode;

/**
  * Class implementing all the utility functions required by an XInclude parser.
  *
  * This class is designed primarily for internal use. This class implements
  * utility methods to be called by an XInclude parser. It is intended to encapsulate
  * the actual processing and recognition of XInclude components.
  */
class XINCLUDE_EXPORT XIncludeUtils
{
private:

    /** Constructor
     *
     */
    XIncludeUtils(XMLErrorReporter *errorReporter);

    /** Destructor
     *
     */
    ~XIncludeUtils();

     /** Parse the supplied XInclude element performing relevant XInclude functionality
      *
      * @param xincludeNode The XInclude node to parse and action
      * @param parsedDocument The DOMDocument to which the results of the XInclude are to be added
      *
      * @return true if the XInclude processing was successful, false if not. Note that an
      * XInclude that fails resulting in a successful fallback action would return true.
      */
    bool doDOMNodeXInclude(DOMNode *xincludeNode, DOMDocument *parsedDocument, XMLEntityHandler* entityResolver);

     /** Parse an XInclude xml file into a DOMDocument node.
      *
      * @param href the location of the document to include
      * @param relativeHref
      * @param parsedDocument
      *
      * @return a newly created DOMDocument containing the parsed and actioned
      * href, or NULL if the document could not be loaded.
      */
    DOMDocument *doXIncludeXMLFileDOM(const XMLCh *href,
        const XMLCh *relativeHref,
        DOMNode *xincludeNode,
        DOMDocument *parsedDocument,
        XMLEntityHandler* entityResolver);

     /** Parse an XInclude text file into a DOMText node.
      *
      * @param href the location of the document to include
      * @param relativeHref
      * @param encoding
      * @param parsedDocument
      *
      * @return a newly created DOMText containing the parsed and actioned
      * href, or NULL if the document could not be loaded.
      */
    DOMText *doXIncludeTEXTFileDOM(const XMLCh *href,
        const XMLCh *relativeHref,
        const XMLCh *encoding,
        DOMNode *xincludeNode,
        DOMDocument *parsedDocument,
        XMLEntityHandler* entityResolver);

     /** Detect whether the supplied details are correct for an xi:include element
      *
      * @param name the element name
      * @param namespaceURI the element namespace
      *
      * @return true if details are valid for an xi:include element, false
      * if not.
      */
    static bool isXIIncludeElement(const XMLCh *name, const XMLCh *namespaceURI);

     /** Detect whether the supplied details are correct for an xi:fallback element
      *
      * @param name the element name
      * @param namespaceURI the element namespace
      *
      * @return true if details are valid for an xi:fallback element, false
      * if not.
      */
    static bool isXIFallbackElement(const XMLCh *name, const XMLCh *namespaceURI);

     /** Detect whether the supplied DOMNode is an xi:include element
      *
      * @param node The node to check
      *
      * @return true if node is an xi:include element, false
      * if not.
      */
    static bool isXIIncludeDOMNode(DOMNode *node);

     /** Detect whether the supplied DOMNode is an xi:fallback element
      *
      * @param node The DOMNode to check
      *
      * @return true if node is an xi:fallback element, false
      * if not.
      */
    static bool isXIFallbackDOMNode(DOMNode *node);

     /** Walk the content of the supplied source node, performing any xinclude actions
      * that are encountered.
      *
      * @param source A DOMNode to parse, this node may be modified by the method
      * @param parsedDocument the DOMDocument to which the parsed results are to be copied.
      *
      * @return true if XInclude behaviour was successfully performed on source, false if not.
      */
    bool parseDOMNodeDoingXInclude(DOMNode *source, DOMDocument *parsedDocument, XMLEntityHandler* entityResolver);

     /** Parse the supplied URI and escape all characters as specified by
      * the XINclusions specification.
      *
      * @param hrefAttrValue the href to parse and escape.
      * @param needsDeallocating set to true if the return value needs deallocating
      * by the caller after use, false if the value returned is the same as the
      * hrefAttrValue passed in.
      *
      * @return an escaped version of hrefAttrValue or hrefAttrValue itself if
      * hrefAttrValue contains only valid characters.
      */
    /* 4.1.1 */
    const XMLCh *getEscapedHRefAttrValue(const XMLCh *hrefAttrValue, bool &needsDeallocating);

     /** Set the accept and accept-lang parameters on HTTP requests generated while
      * XIncluding.
      *
      * @param acceptAttrValue
      * @param acceptLangAttrValue
      *
      * @return true if the values were successfully added to the HTTP request, false
      * if not.
      */
    /* 4.1.2 */
    bool setContentNegotiation(const XMLCh *acceptAttrValue, const XMLCh *acceptLangAttrValue);

     /** Check the characters passed in are all valid characters for XInclusion
      * as specified at http://www.w3.org/TR/xinclude/#text-included-items
      *
      * @param includeChars the characters to parse for validity
      *
      * @return true if the includeChars parameter contains only valid characters
      * for inclusion, false if there are invalid characters in includeChars.
      */
    bool checkTextIsValidForInclude(XMLCh *includeChars);

     /** Add the supplied parameter to the InclusionHistoryStack
      *
      * @param URItoAdd the URI to add to the InclusionHistoryStack/
      *
      * @return true if the URI was added, false if a problem prevented
      * the URI being added.
      */
    bool addDocumentURIToCurrentInclusionHistoryStack(const XMLCh *URItoAdd);

     /** Check the XInclude InclusionHistoryStack to see if the supplied URI
      * has already been included. This is used to ensure that circular inclusion
      * chains are detected and that the inclusion mechanism does not get stuck in
      * a loop.
      *
      * @param toFind the URI to look up.
      *
      * @return true if the toFind parameter is found in the InclusionHistortStack,
      * false if the parameter is not in the stack or the stack is empty.
      */
    bool isInCurrentInclusionHistoryStack(const XMLCh *toFind);

     /** Pop (i.e. remove and return) the top value from the InclusionHistoryStack
      *
      * @param toPop the value that is expected to be at the top of the stack, or
      * NULL if no checking is required.
      *
      * @return the element at the top of the stack
      */
    XIncludeHistoryNode * popFromCurrentInclusionHistoryStack(const XMLCh *toPop);

     /** Free the internal inclusion history list.
      *
      * @return nothing
      */
    void freeInclusionHistory();

     /** Construct and pass on an error description
      *
      * @param errorNode The DOMNode that was being parsed when the error occurred
      * @param errorType The severity of the error
      * @param errorMsg An optional message to include in the error report
      * @param href The URI of the document being parsed.
      *
      * @return true if the errorHandler requests continuation of parsing despite error
      * false if the errorHandler requests parsing end on encountering error, or it
      * there is no error handler.
      */
    bool reportError(const DOMNode* const errorNode
                     , XMLErrs::Codes errorType
                     , const XMLCh* const errorMsg
                     , const XMLCh*    const href);

public:
    /* temporarily public to facilitate helper func getBaseAttrValue */
    static const XMLCh fgXIBaseAttrName[];
private:
    XIncludeHistoryNode *fIncludeHistoryHead;
    XMLSize_t fErrorCount;
    XMLErrorReporter *fErrorReporter;
    static const XMLCh fgXIIncludeQName[];
    static const XMLCh fgXIFallbackQName[];
    static const XMLCh fgXIIncludeHREFAttrName[];
    static const XMLCh fgXIIncludeParseAttrName[];
    static const XMLCh fgXIIncludeParseAttrXMLValue[];
    static const XMLCh fgXIIncludeParseAttrTextValue[];
    static const XMLCh fgXIIncludeXPointerAttrName[];
    static const XMLCh fgXIIncludeEncodingAttrName[];
    static const XMLCh fgXIIncludeAcceptAttrName[];
    static const XMLCh fgXIIncludeAcceptLanguageAttrName[];
    static const XMLCh fgXIIIncludeNamespaceURI[];

    friend class XIncludeDOMDocumentProcessor;
    friend class AbstractDOMParser;
};

XERCES_CPP_NAMESPACE_END

#endif
