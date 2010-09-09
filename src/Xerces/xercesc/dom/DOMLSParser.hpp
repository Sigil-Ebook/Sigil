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
 * $Id: DOMLSParser.hpp 832686 2009-11-04 08:55:59Z borisk $
 *
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMLSPARSER_HPP)
#define XERCESC_INCLUDE_GUARD_DOMLSPARSER_HPP

#include <xercesc/dom/DOMConfiguration.hpp>
#include <xercesc/dom/DOMLSParserFilter.hpp>
#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/validators/common/Grammar.hpp>

XERCES_CPP_NAMESPACE_BEGIN


class DOMErrorHandler;
class DOMLSInput;
class DOMNode;
class DOMDocument;

/**
 * DOMLSParser provides an API for parsing XML documents and building the
 * corresponding DOM document tree. A DOMLSParser instance is obtained from
 * the DOMImplementationLS interface by invoking its createLSParser method.
 *
 * @since DOM Level 3
 *
 */
class CDOM_EXPORT DOMLSParser
{
protected :
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    /** @name Hidden constructors */
    //@{
    DOMLSParser() {};
    //@}

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    /** @name Unimplemented constructors and operators */
    //@{
    DOMLSParser(const DOMLSParser &);
    DOMLSParser & operator = (const DOMLSParser &);
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
    virtual ~DOMLSParser() {};
    //@}

    // -----------------------------------------------------------------------
    //  Class types
    // -----------------------------------------------------------------------
    /** @name Public Constants */
    //@{
    /**
     * A set of possible actions for the parseWithContext method.
     *
     * <p><code>ACTION_APPEND_AS_CHILDREN</code>:
     * Append the result of the parse operation as children of the context node.
     * For this action to work, the context node must be a <code>DOMElement</code>
     * or a <code>DOMDocumentFragment</code>. </p>
     *
     * <p><code>ACTION_INSERT_AFTER</code>:
     * Insert the result of the parse operation as the immediately following sibling
     * of the context node. For this action to work the context node's parent must
     * be a <code>DOMElement</code> or a <code>DOMDocumentFragment</code>. </p>
     *
     * <p><code>ACTION_INSERT_BEFORE</code>:
     * Insert the result of the parse operation as the immediately preceding sibling
     * of the context node. For this action to work the context node's parent must
     * be a <code>DOMElement</code> or a <code>DOMDocumentFragment</code>. </p>
     *
     * <p><code>ACTION_REPLACE</code>:
     * Replace the context node with the result of the parse operation. For this
     * action to work, the context node must have a parent, and the parent must be
     * a <code>DOMElement</code> or a <code>DOMDocumentFragment</code>. </p>
     *
     * <p><code>ACTION_REPLACE_CHILDREN</code>:
     * Replace all the children of the context node with the result of the parse
     * operation. For this action to work, the context node must be a <code>DOMElement</code>,
     * a <code>DOMDocument</code>, or a <code>DOMDocumentFragment</code>. </p>
     *
     * @see parseWithContext(...)
     * @since DOM Level 3
     */
    enum ActionType
    {
        ACTION_APPEND_AS_CHILDREN = 1,
        ACTION_REPLACE_CHILDREN   = 2,
        ACTION_INSERT_BEFORE      = 3,
        ACTION_INSERT_AFTER       = 4,
        ACTION_REPLACE            = 5
    };
    //@}

    // -----------------------------------------------------------------------
    //  Virtual DOMLSParser interface
    // -----------------------------------------------------------------------
    /** @name Functions introduced in DOM Level 3 */
    //@{

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------

    /**
      * Get a pointer to the <code>DOMConfiguration</code> object used when parsing
      * an input source.
      * This <code>DOMConfiguration</code> is specific to the parse operation.
      * No parameter values from this <code>DOMConfiguration</code> object are passed
      * automatically to the <code>DOMConfiguration</code> object on the
      * <code>DOMDocument</code> that is created, or used, by the parse operation.
      * The DOM application is responsible for passing any needed parameter values
      * from this <code>DOMConfiguration</code> object to the <code>DOMConfiguration</code>
      * object referenced by the <code>DOMDocument</code> object.
      *
      * In addition to the parameters recognized in on the <code>DOMConfiguration</code>
      * interface defined in [DOM Level 3 Core], the <code>DOMConfiguration</code> objects
      * for <code>DOMLSParser</code> add or modify the following parameters:
      *
      * "charset-overrides-xml-encoding"
      *     true [optional] (default)
      *         If a higher level protocol such as HTTP [IETF RFC 2616] provides an
      *         indication of the character encoding of the input stream being processed,
      *         that will override any encoding specified in the XML declaration or the
      *         Text declaration (see also section 4.3.3, "Character Encoding in Entities",
      *         in [XML 1.0]). Explicitly setting an encoding in the <code>DOMLSInput</code>
      *          overrides any encoding from the protocol.
      *     false [required]
      *         The parser ignores any character set encoding information from higher-level
      *         protocols.
      *
      * "disallow-doctype"
      *     true [optional]
      *         Throw a fatal "doctype-not-allowed" error if a doctype node is found while
      *         parsing the document. This is useful when dealing with things like SOAP
      *         envelopes where doctype nodes are not allowed.
      *     false [required] (default)
      *         Allow doctype nodes in the document.
      *
      * "ignore-unknown-character-denormalizations"
      *     true [required] (default)
      *         If, while verifying full normalization when [XML 1.1] is supported, a
      *         processor encounters characters for which it cannot determine the normalization
      *         properties, then the processor will ignore any possible denormalizations
      *         caused by these characters.
      *         This parameter is ignored for [XML 1.0].
      *     false [optional]
      *         Report an fatal "unknown-character-denormalization" error if a character
      *         is encountered for which the processor cannot determine the normalization
      *         properties.
      *
      * "infoset"
      *     See the definition of DOMConfiguration for a description of this parameter.
      *     Unlike in [DOM Level 3 Core], this parameter will default to true for DOMLSParser.
      *
      * "namespaces"
      *     true [required] (default)
      *         Perform the namespace processing as defined in [XML Namespaces] and
      *         [XML Namespaces 1.1].
      *     false [optional]
      *         Do not perform the namespace processing.
      *
      * "resource-resolver" [required]
      *     A pointer to a DOMLSResourceResolver object, or NULL. If the value of this parameter
      *     is not null when an external resource (such as an external XML entity or an XML schema
      *     location) is encountered, the implementation will request that the DOMLSResourceResolver
      *     referenced in this parameter resolves the resource.
      *
      * "supported-media-types-only"
      *     true [optional]
      *         Check that the media type of the parsed resource is a supported media type. If
      *         an unsupported media type is encountered, a fatal error of type "unsupported-media-type"
      *         will be raised. The media types defined in [IETF RFC 3023] must always be accepted.
      *     false [required] (default)
      *         Accept any media type.
      *
      * "validate"
      *     See the definition of <code>DOMConfiguration</code> for a description of this parameter.
      *     Unlike in [DOM Level 3 Core], the processing of the internal subset is always accomplished, even
      *     if this parameter is set to false.
      *
      * "validate-if-schema"
      *     See the definition of <code>DOMConfiguration</code> for a description of this parameter.
      *     Unlike in [DOM Level 3 Core], the processing of the internal subset is always accomplished, even
      *     if this parameter is set to false.
      *
      * "well-formed"
      *     See the definition of <code>DOMConfiguration</code> for a description of this parameter.
      *     Unlike in [DOM Level 3 Core], this parameter cannot be set to false.
      *
      * In addition to these, Xerces adds these non standard parameters:
      *
      * "http://apache.org/xml/properties/entity-resolver"
      *     A pointer to a XMLEntityResolver object, or NULL. If the value of this parameter
      *     is not null when an external resource (such as an external XML entity or an XML schema
      *     location) is encountered, the implementation will request that the XMLEntityResolver
      *     referenced in this parameter resolves the resource.
      *
      * "http://apache.org/xml/properties/schema/external-schemaLocation"
      *     A string holding a set of [namespaceUri schemaLocation] entries that will be treated as
      *     the content of the attribute xsi:schemaLocation of the root element
      *
      * "http://apache.org/xml/properties/schema/external-noNamespaceSchemaLocation"
      *     A string holding the schemaLocation for the empty namespace URI that will be treated as
      *     the content of the attribute xsi:noNamespaceSchemaLocation of the root element
      *
      * "http://apache.org/xml/properties/security-manager"
      *     A pointer to a SecurityManager object that will control how many entity references will be
      *     expanded during parsing
      *
      * "http://apache.org/xml/properties/scannerName"
      *     A string holding the type of scanner used while parsing. The valid names are:
      *      <ul>
      *       <li>IGXMLScanner: the default one, capable of both XMLSchema and DTD validation</li>
      *       <li>SGXMLScanner: a scanner that can only perform XMLSchema validation</li>
      *       <li>DGXMLScanner: a scanner that can only perform DTD validation</li>
      *       <li>WFXMLScanner: a scanner that cannot perform any type validation, only well-formedness</li>
      *      </ul>
      *
      * "http://apache.org/xml/properties/parser-use-DOMDocument-from-Implementation"
      *     A string holding the capabilities of the DOM implementation to be used to create the DOMDocument
      *     resulting from the parse operation. For instance, "LS" or "Core"
      *
      * "http://apache.org/xml/features/validation/schema"
      *     true
      *         Enable XMLSchema validation (note that also namespace processing should be enabled)
      *     false (default)
      *         Don't perform XMLSchema validation
      *
      * "http://apache.org/xml/features/validation/schema-full-checking"
      *     true
      *         Turn on full XMLSchema checking (e.g. Unique Particle Attribution)
      *     false (default)
      *         Don't perform full XMLSchema checking
      *
      * "http://apache.org/xml/features/validating/load-schema"
      *     true (default)
      *         Allow the parser to load schemas that are not in the grammar pool
      *     false
      *         Schemas that are not in the grammar pool are ignored
      *
      * "http://apache.org/xml/features/dom/user-adopts-DOMDocument"
      *     true
      *         The DOMDocument objects returned by <code>parse</code> will be owned by the caller
      *     false (default)
      *         The DOMDocument objects returned by <code>parse</code> will be owned by this <code>DOMLSParser</code>
      *         and deleted when released
      *
      * "http://apache.org/xml/features/nonvalidating/load-external-dtd"
      *     true (default)
      *         Allow the parser to load external DTDs
      *     false
      *         References to external DTDs will be ignored
      *
      * "http://apache.org/xml/features/continue-after-fatal-error"
      *     true
      *         Parsing should try to continue even if a fatal error has been triggered, trying to generate a DOM tree
      *         from a non well-formed XML
      *     false (default)
      *         Violation of XML rules will abort parsing
      *
      * "http://apache.org/xml/features/validation-error-as-fatal"
      *     true
      *         Validation errors are treated as fatal errors, and abort parsing (unless "continue-after-fatal-error"
      *         has been specified)
      *     false (default)
      *         Validation errors are normal errors
      *
      * "http://apache.org/xml/features/validation/cache-grammarFromParse"
      *     true
      *         XMLSchemas referenced by an XML file are cached in order to be reused by other parse operations
      *     false (default)
      *         XMLSchemas loaded during a parse operation will be discarded before the next one
      *
      * "http://apache.org/xml/features/validation/use-cachedGrammarInParse"
      *     true
      *         During this parse operation, reuse the XMLSchemas found in the cache
      *     false (default)
      *         Don't reuse the XMLSchemas found in the cache
      *
      * "http://apache.org/xml/features/calculate-src-ofs"
      *     true
      *         During parsing update the position in the source stream
      *     false (default)
      *         Don't waste time computing the position in the source stream
      *
      * "http://apache.org/xml/features/standard-uri-conformant"
      *     true
      *         Require that every URL being resolved is made of valid URL characters only
      *     false (default)
      *         Allow invalid URL characters in URL (e.g. spaces)
      *
      * "http://apache.org/xml/features/dom-has-psvi-info"
      *     true
      *         Add schema informations to DOMElement and DOMAttr nodes in the output DOM tree
      *     false (default)
      *         Don't store schema informations in the output DOM tree
      *
      * "http://apache.org/xml/features/generate-synthetic-annotations"
      *     true
      *         Create annotation objects in the representation of the loaded XMLSchemas
      *     false (default)
      *         Discard annotations found in the loaded XMLSchemas
      *
      * "http://apache.org/xml/features/validate-annotations"
      *     true
      *         Check that annotations are valid according to their XMLSchema definition
      *     false (default)
      *         Don't validate annotations
      *
      * "http://apache.org/xml/features/validation/identity-constraint-checking"
      *     true (default)
      *         Enforce identity constraints specified in the XMLSchema
      *     false
      *         Don't enforce identity constraints
      *
      * "http://apache.org/xml/features/validation/ignoreCachedDTD"
      *     true
      *         Don't reuse DTDs found in the cache, even if use-cachedGrammarInParse is <code>true</code>
      *     false (default)
      *         Reuse DTDs found in the cache, if use-cachedGrammarInParse is <code>true</code>
      *
      * "http://apache.org/xml/features/schema/ignore-annotations"
      *     true
      *         Don't process annotations found in an XMLSchema
      *     false (default)
      *         Process the annotations found in an XMLSchema
      *
      * "http://apache.org/xml/features/disable-default-entity-resolution"
      *     true
      *         Entities will be resolved only by a resolver installed by the user
      *     false (default)
      *         If the entity resolver has not been installed, or it refuses to resolve the given entity, the
      *         parser will try to locate it himself
      *
      * "http://apache.org/xml/features/validation/schema/skip-dtd-validation"
      *     true
      *         If XMLSchema validation is <code>true</code>, DTD validation will not be performed
      *     false (default)
      *         If a DTD is found, it will be used to validate the XML
      *
      * @return The pointer to the configuration object.
      * @since DOM Level 3
      */
    virtual DOMConfiguration* getDomConfig() = 0;

    /**
      * Get a const pointer to the application filter
      *
      * This method returns the installed application filter. If no filter
      * has been installed, then it will be a zero pointer.
      *
      * @return A const pointer to the installed application filter
      * @since DOM Level 3
      */
    virtual const DOMLSParserFilter* getFilter() const = 0;

    /**
      * Return whether the parser is asynchronous
      *
      * @return <code>true</code> if the <code>DOMLSParser</code> is asynchronous,
      *         <code>false</code> if it is synchronous
      * @since DOM Level 3
      */
    virtual bool getAsync() const = 0;

    /**
      * Return whether the parser is busy parsing
      *
      * @return <code>true</code> if the <code>DOMLSParser</code> is currently busy
      *         loading a document, otherwise <code>false</code>.
      * @since DOM Level 3
      */
    virtual bool getBusy() const = 0;

    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------
    /**
      * Set the application filter
      *
      * When the application provides a filter, the parser will call out to
      * the filter at the completion of the construction of each <code>DOMElement</code>
      * node. The filter implementation can choose to remove the element from the
      * document being constructed or to terminate the parse early.
      * The filter is invoked after the operations requested by the DOMConfiguration
      * parameters have been applied. For example, if "validate" is set to true,
      * the validation is done before invoking the filter.
      *
      * <i>Any previously set filter is merely dropped, since the parser
      * does not own them.</i>
      *
      * @param filter  A const pointer to the user supplied application
      *                filter.
      *
      * @see #getFilter
      * @since DOM Level 3
      */
    virtual void setFilter(DOMLSParserFilter* const filter) = 0;

    // -----------------------------------------------------------------------
    //  Parsing methods
    // -----------------------------------------------------------------------
    /**
      * Parse an XML document from a resource identified by a <code>DOMLSInput</code>.
      *
      * The parser owns the returned DOMDocument.  It will be deleted
      * when the parser is released.
      *
      * @param source The <code>DOMLSInput</code> from which the source of the document
      *               is to be read.
      * @return If the <code>DOMLSParser</code> is a synchronous <code>DOMLSParser</code>
      *         the newly created and populated <code>DOMDocument</code> is returned.
      *         If the <code>DOMLSParser</code> is asynchronous then <code>NULL</code>
      *         is returned since the document object may not yet be constructed when
      *         this method returns.
      * @exception DOMException INVALID_STATE_ERR: Raised if the <code>DOMLSParser::busy</code>
      *                                            attribute is true.
      * @exception DOMLSException PARSE_ERR: Starting from Xerces-C++ 4.0.0 this exception is
      *                                      raised if the <code>DOMLSParser</code> was unable
      *                                      to load the XML document. DOM applications should
      *                                      attach a <code>DOMErrorHandler</code> using the
      *                                      parameter "error-handler" if they wish to get details
      *                                      on the error.
      *
      * @see DOMLSInput#DOMLSInput
      * @see DOMConfiguration
      * @see resetDocumentPool
      * @since DOM Level 3
      */
    virtual DOMDocument* parse(const DOMLSInput* source) = 0;

    /**
      * Parse an XML document from a location identified by a URI reference [IETF RFC 2396].
      * If the URI contains a fragment identifier (see section 4.1 in [IETF RFC 2396]),
      * the behavior is not defined by this specification, future versions of this
      * specification may define the behavior.
      *
      * The parser owns the returned DOMDocument.  It will be deleted
      * when the parser is released.
      *
      * @param uri The location of the XML document to be read (in Unicode)
      * @return If the <code>DOMLSParser</code> is a synchronous <code>DOMLSParser</code>
      *         the newly created and populated <code>DOMDocument</code> is returned.
      *         If the <code>DOMLSParser</code> is asynchronous then <code>NULL</code>
      *         is returned since the document object is not yet parsed when this method returns.
      * @exception DOMException INVALID_STATE_ERR: Raised if the <code>DOMLSParser::busy</code>
      *                                            attribute is true.
      * @exception DOMLSException PARSE_ERR: Starting from Xerces-C++ 4.0.0 this exception is
      *                                      raised if the <code>DOMLSParser</code> was unable
      *                                      to load the XML document. DOM applications should
      *                                      attach a <code>DOMErrorHandler</code> using the
      *                                      parameter "error-handler" if they wish to get details
      *                                      on the error.
      *
      * @see #parse(DOMLSInput,...)
      * @see resetDocumentPool
      * @since DOM Level 3
      */
    virtual DOMDocument* parseURI(const XMLCh* const uri) = 0;

    /**
      * Parse an XML document from a location identified by a URI reference [IETF RFC 2396].
      * If the URI contains a fragment identifier (see section 4.1 in [IETF RFC 2396]),
      * the behavior is not defined by this specification, future versions of this
      * specification may define the behavior.
      *
      * The parser owns the returned DOMDocument.  It will be deleted
      * when the parser is released.
      *
      * @param uri The location of the XML document to be read (in the local code page)
      * @return If the <code>DOMLSParser</code> is a synchronous <code>DOMLSParser</code>
      *         the newly created and populated <code>DOMDocument</code> is returned.
      *         If the <code>DOMLSParser</code> is asynchronous then <code>NULL</code>
      *         is returned since the document object is not yet parsed when this method returns.
      * @exception DOMException INVALID_STATE_ERR: Raised if the <code>DOMLSParser::busy</code>
      *                                            attribute is true.
      * @exception DOMLSException PARSE_ERR: Starting from Xerces-C++ 4.0.0 this exception is
      *                                      raised if the <code>DOMLSParser</code> was unable
      *                                      to load the XML document. DOM applications should
      *                                      attach a <code>DOMErrorHandler</code> using the
      *                                      parameter "error-handler" if they wish to get details
      *                                      on the error.
      *
      * @see #parse(DOMLSInput,...)
      * @see resetDocumentPool
      * @since DOM Level 3
      */
    virtual DOMDocument* parseURI(const char* const uri) = 0;

    /**
      * Parse an XML fragment from a resource identified by a <code>DOMLSInput</code>
      * and insert the content into an existing document at the position specified
      * with the context and action arguments. When parsing the input stream, the
      * context node (or its parent, depending on where the result will be inserted)
      * is used for resolving unbound namespace prefixes. The context node's
      * <code>ownerDocument</code> node (or the node itself if the node of type
      * <code>DOCUMENT_NODE</code>) is used to resolve default attributes and entity
      * references.
      * As the new data is inserted into the document, at least one mutation event
      * is fired per new immediate child or sibling of the context node.
      * If the context node is a <code>DOMDocument</code> node and the action is
      * <code>ACTION_REPLACE_CHILDREN</code>, then the document that is passed as
      * the context node will be changed such that its <code>xmlEncoding</code>,
      * <code>documentURI</code>, <code>xmlVersion</code>, <code>inputEncoding</code>,
      * <code>xmlStandalone</code>, and all other such attributes are set to what they
      * would be set to if the input source was parsed using <code>DOMLSParser::parse()</code>.
      * This method is always synchronous, even if the <code>DOMLSParser</code> is
      * asynchronous (<code>DOMLSParser::getAsync()</code> returns true).
      * If an error occurs while parsing, the caller is notified through the <code>ErrorHandler</code>
      * instance associated with the "error-handler" parameter of the <code>DOMConfiguration</code>.
      * When calling <code>parseWithContext</code>, the values of the following configuration
      * parameters will be ignored and their default values will always be used instead:
      *   "validate",
      *   "validate-if-schema"
      *   "element-content-whitespace".
      * Other parameters will be treated normally, and the parser is expected to call
      * the <code>DOMLSParserFilter</code> just as if a whole document was parsed.
      *
      * @param source The <code>DOMLSInput</code> from which the source document is
      *               to be read. The source document must be an XML fragment, i.e.
      *               anything except a complete XML document (except in the case where
      *               the context node of type <code>DOCUMENT_NODE</code>, and the action is
      *               <code>ACTION_REPLACE_CHILDREN</code>), a <code>DOCTYPE</code>
      *               (internal subset), entity declaration(s), notation declaration(s),
      *               or XML or text declaration(s).
      * @param contextNode The node that is used as the context for the data that is being
      *                    parsed. This node must be a <code>DOMDocument</code> node, a
      *                    <code>DOMDocumentFragment</code> node, or a node of a type that
      *                    is allowed as a child of an <code>DOMElement</code> node, e.g.
      *                    it cannot be an <code>DOMAttribute</code> node.
      * @param action This parameter describes which action should be taken between the new
      *               set of nodes being inserted and the existing children of the context node.
      *               The set of possible actions is defined in <code>ACTION_TYPES</code> above.
      * @return Return the node that is the result of the parse operation. If the result is more
      *         than one top-level node, the first one is returned.
      *
      * @exception DOMException
      *     HIERARCHY_REQUEST_ERR: Raised if the content cannot replace, be inserted before, after,
      *                            or as a child of the context node (see also <code>DOMNode::insertBefore</code>
      *                            or <code>DOMNode::replaceChild</code> in [DOM Level 3 Core]).
      *     NOT_SUPPORTED_ERR: Raised if the <code>DOMLSParser</code> doesn't support this method,
      *                        or if the context node is of type <code>DOMDocument</code> and the DOM
      *                        implementation doesn't support the replacement of the <code>DOMDocumentType</code>
      *                        child or <code>DOMElement</code> child.
      *     NO_MODIFICATION_ALLOWED_ERR: Raised if the context node is a read only node and the content
      *                                  is being appended to its child list, or if the parent node of
      *                                  the context node is read only node and the content is being
      *                                  inserted in its child list.
      *     INVALID_STATE_ERR: Raised if the <code>DOMLSParser::getBusy()</code> returns true.
      *
      * @exception DOMLSException PARSE_ERR: Raised if the <code>DOMLSParser</code> was unable to load
      *                                      the XML fragment. DOM applications should attach a
      *                                      <code>DOMErrorHandler</code> using the parameter "error-handler"
      *                                      if they wish to get details on the error.
      * @since DOM Level 3
      */
    virtual DOMNode* parseWithContext(const DOMLSInput* source, DOMNode* contextNode, const ActionType action) = 0;

    /**
      * Abort the loading of the document that is currently being loaded by the <code>DOMLSParser</code>.
      * If the <code>DOMLSParser</code> is currently not busy, a call to this method does nothing.
      *
      * Note: invoking this method will remove the installed <code>DOMLSParserFilter</code> filter
      *
      * @since DOM Level 3
      */
    virtual void abort() = 0;
    //@}

    // -----------------------------------------------------------------------
    //  Non-standard Extension
    // -----------------------------------------------------------------------
    /** @name Non-standard Extension */
    //@{
    /**
     * Called to indicate that this DOMLSParser is no longer in use
     * and that the implementation may relinquish any resources associated with it.
     *
     * Access to a released object will lead to unexpected result.
     */
    virtual void              release() = 0;

    /** Reset the documents vector pool and release all the associated memory
      * back to the system.
      *
      * When parsing a document using a DOM parser, all memory allocated
      * for a DOM tree is associated to the DOM document.
      *
      * If you do multiple parse using the same DOM parser instance, then
      * multiple DOM documents will be generated and saved in a vector pool.
      * All these documents (and thus all the allocated memory)
      * won't be deleted until the parser instance is destroyed.
      *
      * If you don't need these DOM documents anymore and don't want to
      * destroy the DOM parser instance at this moment, then you can call this method
      * to reset the document vector pool and release all the allocated memory
      * back to the system.
      *
      * It is an error to call this method if you are in the middle of a
      * parse (e.g. in the mid of a progressive parse).
      *
      * @exception IOException An exception from the parser if this function
      *            is called when a parse is in progress.
      *
      */
    virtual void              resetDocumentPool() = 0;

    /**
      * Preparse schema grammar (XML Schema, DTD, etc.) via an input source
      * object.
      *
      * This method invokes the preparsing process on a schema grammar XML
      * file specified by the DOMLSInput parameter. If the 'toCache' flag
      * is enabled, the parser will cache the grammars for re-use. If a grammar
      * key is found in the pool, no caching of any grammar will take place.
      *
      * @param source A const reference to the DOMLSInput object which
      *               points to the schema grammar file to be preparsed.
      * @param grammarType The grammar type (Schema or DTD).
      * @param toCache If <code>true</code>, we cache the preparsed grammar,
      *                otherwise, no chaching. Default is <code>false</code>.
      * @return The preparsed schema grammar object (SchemaGrammar or
      *         DTDGrammar). That grammar object is owned by the parser.
      *
      * @exception SAXException Any SAX exception, possibly
      *            wrapping another exception.
      * @exception XMLException An exception from the parser or client
      *            handler code.
      * @exception DOMException A DOM exception as per DOM spec.
      *
      * @see DOMLSInput#DOMLSInput
      */
    virtual Grammar* loadGrammar(const DOMLSInput* source,
                                 const Grammar::GrammarType grammarType,
                                 const bool toCache = false) = 0;

    /**
      * Preparse schema grammar (XML Schema, DTD, etc.) via a file path or URL
      *
      * This method invokes the preparsing process on a schema grammar XML
      * file specified by the file path parameter. If the 'toCache' flag is
      * enabled, the parser will cache the grammars for re-use. If a grammar
      * key is found in the pool, no caching of any grammar will take place.
      *
      * @param systemId A const XMLCh pointer to the Unicode string which
      *                 contains the path to the XML grammar file to be
      *                 preparsed.
      * @param grammarType The grammar type (Schema or DTD).
      * @param toCache If <code>true</code>, we cache the preparsed grammar,
      *                otherwise, no chaching. Default is <code>false</code>.
      * @return The preparsed schema grammar object (SchemaGrammar or
      *         DTDGrammar). That grammar object is owned by the parser.
      *
      * @exception SAXException Any SAX exception, possibly
      *            wrapping another exception.
      * @exception XMLException An exception from the parser or client
      *            handler code.
      * @exception DOMException A DOM exception as per DOM spec.
      */
    virtual Grammar* loadGrammar(const XMLCh* const systemId,
                                 const Grammar::GrammarType grammarType,
                                 const bool toCache = false) = 0;

    /**
      * Preparse schema grammar (XML Schema, DTD, etc.) via a file path or URL
      *
      * This method invokes the preparsing process on a schema grammar XML
      * file specified by the file path parameter. If the 'toCache' flag is
      * enabled, the parser will cache the grammars for re-use. If a grammar
      * key is found in the pool, no caching of any grammar will take place.
      *
      * @param systemId A const char pointer to a native string which contains
      *                 the path to the XML grammar file to be preparsed.
      * @param grammarType The grammar type (Schema or DTD).
      * @param toCache If <code>true</code>, we cache the preparsed grammar,
      *                otherwise, no chaching. Default is <code>false</code>.
      * @return The preparsed schema grammar object (SchemaGrammar or
      *         DTDGrammar). That grammar object is owned by the parser.
      *
      *
      * @exception SAXException Any SAX exception, possibly
      *            wrapping another exception.
      * @exception XMLException An exception from the parser or client
      *            handler code.
      * @exception DOMException A DOM exception as per DOM spec.
      */
    virtual Grammar* loadGrammar(const char* const systemId,
                                 const Grammar::GrammarType grammarType,
                                 const bool toCache = false) = 0;

    /**
     * Retrieve the grammar that is associated with the specified namespace key
     *
     * @param  nameSpaceKey Namespace key
     * @return Grammar associated with the Namespace key.
     */
    virtual Grammar* getGrammar(const XMLCh* const nameSpaceKey) const = 0;

    /**
     * Retrieve the grammar where the root element is declared.
     *
     * @return Grammar where root element declared
     */
    virtual Grammar* getRootGrammar() const = 0;

    /**
     * Returns the string corresponding to a URI id from the URI string pool.
     *
     * @param uriId id of the string in the URI string pool.
     * @return URI string corresponding to the URI id.
     */
    virtual const XMLCh* getURIText(unsigned int uriId) const = 0;

    /**
      * Clear the cached grammar pool
      */
    virtual void resetCachedGrammarPool() = 0;

    /**
      * Returns the current src offset within the input source.
      *
      * @return offset within the input source
      */
    virtual XMLFilePos getSrcOffset() const = 0;

    //@}

};


XERCES_CPP_NAMESPACE_END

#endif
