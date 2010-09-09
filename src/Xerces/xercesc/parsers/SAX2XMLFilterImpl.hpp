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
 * $Id: SAX2XMLFilterImpl.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_SAX2XMLFILTERIMPL_HPP)
#define XERCESC_INCLUDE_GUARD_SAX2XMLFILTERIMPL_HPP

#include <xercesc/sax2/SAX2XMLFilter.hpp>
#include <xercesc/sax/EntityResolver.hpp>
#include <xercesc/sax/DTDHandler.hpp>
#include <xercesc/sax2/ContentHandler.hpp>
#include <xercesc/sax/ErrorHandler.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/**
  * This class implements the SAX2 'XMLFilterImpl' interface and should be
  * used by applications as the base class for their SAX2 filters.
  * This implementation simply forwards every call to the parent object.
  *
  */

class PARSERS_EXPORT SAX2XMLFilterImpl :
    public SAX2XMLFilter
    , public EntityResolver
    , public DTDHandler
    , public ContentHandler
    , public ErrorHandler
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    /** @name Constructors and Destructor */
    //@{
    /** The default constructor */
	SAX2XMLFilterImpl(SAX2XMLReader* parent);

    /** The destructor */
	~SAX2XMLFilterImpl() ;
   //@}

    //-----------------------------------------------------------------------
    // Implementation of SAX2XMLReader Interface
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    // The XMLReader interface
    //-----------------------------------------------------------------------
    /** @name Implementation of SAX 2.0 XMLReader interface's. */
    //@{

    /**
      * This method returns the installed content handler.
      *
      * @return A pointer to the installed content handler object.
      */
    virtual ContentHandler* getContentHandler() const ;

    /**
      * This method returns the installed DTD handler.
      *
      * @return A pointer to the installed DTD handler object.
      */
    virtual DTDHandler* getDTDHandler() const ;

    /**
      * This method returns the installed entity resolver.
      *
      * @return A pointer to the installed entity resolver object.
      */
    virtual EntityResolver* getEntityResolver() const ;

    /**
      * This method returns the installed error handler.
      *
      * @return A pointer to the installed error handler object.
      */
    virtual ErrorHandler* getErrorHandler() const ;

	/**
     * Query the current state of any feature in a SAX2 XMLReader.
	  *
	  * @param name The unique identifier (URI) of the feature being set.
	  * @return The current state of the feature.
     * @exception SAXNotRecognizedException If the requested feature is not known.
	  */
	virtual bool getFeature(const XMLCh* const name) const ;

	/**
     * Query the current value of a property in a SAX2 XMLReader.
     *
     * The parser owns the returned pointer.  The memory allocated for
     * the returned pointer will be destroyed when the parser is deleted.
     *
     * To ensure accessibility of the returned information after the parser
     * is deleted, callers need to copy and store the returned information
     * somewhere else; otherwise you may get unexpected result.  Since the returned
     * pointer is a generic void pointer, see the SAX2 Programming Guide to learn
     * exactly what type of property value each property returns for replication.
     *
     * @param name The unique identifier (URI) of the property being set.
     * @return     The current value of the property.  The pointer spans the same
     *             life-time as the parser.  A null pointer is returned if nothing
     *             was specified externally.
     * @exception  SAXNotRecognizedException If the requested property is not known.
     */
	virtual void* getProperty(const XMLCh* const name) const ;

  /**
    * Allow an application to register a document event handler.
    *
    * If the application does not register a document handler, all
    * document events reported by the SAX parser will be silently
    * ignored (this is the default behaviour implemented by
    * HandlerBase).
    *
    * Applications may register a new or different handler in the
    * middle of a parse, and the SAX parser must begin using the new
    * handler immediately.
    *
    * @param handler The document handler.
    * @see DocumentHandler#DocumentHandler
    * @see HandlerBase#HandlerBase
    */
    virtual void setContentHandler(ContentHandler* const handler) ;

  /**
    * Allow an application to register a DTD event handler.
    *
    * If the application does not register a DTD handler, all DTD
    * events reported by the SAX parser will be silently ignored (this
    * is the default behaviour implemented by HandlerBase).
    *
    * Applications may register a new or different handler in the middle
    * of a parse, and the SAX parser must begin using the new handler
    * immediately.
    *
    * @param handler The DTD handler.
    * @see DTDHandler#DTDHandler
    * @see HandlerBase#HandlerBase
    */
    virtual void setDTDHandler(DTDHandler* const handler) ;

  /**
    * Allow an application to register a custom entity resolver.
    *
    * If the application does not register an entity resolver, the
    * SAX parser will resolve system identifiers and open connections
    * to entities itself (this is the default behaviour implemented in
    * DefaultHandler).
    *
    * Applications may register a new or different entity resolver
    * in the middle of a parse, and the SAX parser must begin using
    * the new resolver immediately.
    *
    * @param resolver The object for resolving entities.
    * @see EntityResolver#EntityResolver
    * @see DefaultHandler#DefaultHandler
    */
    virtual void setEntityResolver(EntityResolver* const resolver) ;

  /**
    * Allow an application to register an error event handler.
    *
    * If the application does not register an error event handler,
    * all error events reported by the SAX parser will be silently
    * ignored, except for fatalError, which will throw a SAXException
    * (this is the default behaviour implemented by HandlerBase).
    *
    * Applications may register a new or different handler in the
    * middle of a parse, and the SAX parser must begin using the new
    * handler immediately.
    *
    * @param handler The error handler.
    * @see ErrorHandler#ErrorHandler
    * @see SAXException#SAXException
    * @see HandlerBase#HandlerBase
    */
    virtual void setErrorHandler(ErrorHandler* const handler) ;

  /**
    * Set the state of any feature in a SAX2 XMLReader.
    * Supported features in SAX2 for xerces-c are:
    * <br>(See the SAX2 Programming Guide for detail description).
    *
    * <br>http://xml.org/sax/features/validation (default: true)
    * <br>http://xml.org/sax/features/namespaces (default: true)
    * <br>http://xml.org/sax/features/namespace-prefixes (default: false)
    * <br>http://apache.org/xml/features/validation/dynamic (default: false)
    * <br>http://apache.org/xml/features/validation/reuse-grammar (default: false)
    * <br>http://apache.org/xml/features/validation/schema (default: true)
    * <br>http://apache.org/xml/features/validation/schema-full-checking (default: false)
    * <br>http://apache.org/xml/features/validating/load-schema (default: true)
    * <br>http://apache.org/xml/features/nonvalidating/load-external-dtd (default: true)
    * <br>http://apache.org/xml/features/continue-after-fatal-error (default: false)
    * <br>http://apache.org/xml/features/validation-error-as-fatal (default: false)
    *
    * @param name The unique identifier (URI) of the feature.
    * @param value The requested state of the feature (true or false).
    * @exception SAXNotRecognizedException If the requested feature is not known.
    * @exception SAXNotSupportedException Feature modification is not supported during parse
    *
    */
	virtual void setFeature(const XMLCh* const name, const bool value) ;

  /**
    * Set the value of any property in a SAX2 XMLReader.
    * Supported properties in SAX2 for xerces-c are:
    * <br>(See the SAX2 Programming Guide for detail description).
    *
    * <br>http://apache.org/xml/properties/schema/external-schemaLocation
    * <br>http://apache.org/xml/properties/schema/external-noNamespaceSchemaLocation.
    *
    * It takes a void pointer as the property value.  Application is required to initialize this void
    * pointer to a correct type.  See the SAX2 Programming Guide
    * to learn exactly what type of property value each property expects for processing.
    * Passing a void pointer that was initialized with a wrong type will lead to unexpected result.
    * If the same property is set more than once, the last one takes effect.
    *
    * @param name The unique identifier (URI) of the property being set.
    * @param value The requested value for the property.  See
    *            the SAX2 Programming Guide to learn
    *            exactly what type of property value each property expects for processing.
    *            Passing a void pointer that was initialized with a wrong type will lead
    *            to unexpected result.
    * @exception SAXNotRecognizedException If the requested property is not known.
    * @exception SAXNotSupportedException Property modification is not supported during parse
    */
	virtual void setProperty(const XMLCh* const name, void* value) ;

  /**
    * Parse an XML document.
    *
    * The application can use this method to instruct the SAX parser
    * to begin parsing an XML document from any valid input
    * source (a character stream, a byte stream, or a URI).
    *
    * Applications may not invoke this method while a parse is in
    * progress (they should create a new Parser instead for each
    * additional XML document).  Once a parse is complete, an
    * application may reuse the same Parser object, possibly with a
    * different input source.
    *
    * @param source The input source for the top-level of the
    *               XML document.
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    * @exception XMLException An exception from the parser or client
    *            handler code.
    * @see InputSource#InputSource
    * @see #setEntityResolver
    * @see #setDTDHandler
    * @see #setDocumentHandler
    * @see #setErrorHandler
    */
    virtual void parse
    (
        const   InputSource&    source
    ) ;

  /**
    * Parse an XML document from a system identifier (URI).
    *
    * This method is a shortcut for the common case of reading a
    * document from a system identifier.  It is the exact equivalent
    * of the following:
    *
    * parse(new URLInputSource(systemId));
    *
    * If the system identifier is a URL, it must be fully resolved
    * by the application before it is passed to the parser.
    *
    * @param systemId The system identifier (URI).
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    * @exception XMLException An exception from the parser or client
    *            handler code.
    * @see #parse(InputSource)
    */
    virtual void parse
    (
        const   XMLCh* const    systemId
    ) ;

  /**
    * Parse an XML document from a system identifier (URI).
    *
    * This method is a shortcut for the common case of reading a
    * document from a system identifier.  It is the exact equivalent
    * of the following:
    *
    * parse(new URLInputSource(systemId));
    *
    * If the system identifier is a URL, it must be fully resolved
    * by the application before it is passed to the parser.
    *
    * @param systemId The system identifier (URI).
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    * @exception XMLException An exception from the parser or client
    *            handler code.
    * @see #parse(InputSource)
    */
    virtual void parse
    (
        const   char* const     systemId
    ) ;

    //@}

    // -----------------------------------------------------------------------
    //  SAX 2.0-ext
    // -----------------------------------------------------------------------
    /** @name SAX 2.0-ext */
    //@{
    /**
      * This method returns the installed declaration handler.
      *
      * @return A pointer to the installed declaration handler object.
      */
    virtual DeclHandler* getDeclarationHandler() const ;

	/**
      * This method returns the installed lexical handler.
      *
      * @return A pointer to the installed lexical handler object.
      */
    virtual LexicalHandler* getLexicalHandler() const ;

   /**
    * Allow an application to register a declaration event handler.
    *
    * If the application does not register a declaration handler,
    * all events reported by the SAX parser will be silently
    * ignored. (this is the default behaviour implemented by DefaultHandler).
    *
    * Applications may register a new or different handler in the
    * middle of a parse, and the SAX parser must begin using the new
    * handler immediately.
    *
    * @param handler The DTD declaration handler.
    * @see DeclHandler#DeclHandler
    * @see SAXException#SAXException
    * @see DefaultHandler#DefaultHandler
    */
    virtual void setDeclarationHandler(DeclHandler* const handler) ;

   /**
    * Allow an application to register a lexical event handler.
    *
    * If the application does not register a lexical handler,
    * all events reported by the SAX parser will be silently
    * ignored. (this is the default behaviour implemented by HandlerBase).
    *
    * Applications may register a new or different handler in the
    * middle of a parse, and the SAX parser must begin using the new
    * handler immediately.
    *
    * @param handler The error handler.
    * @see LexicalHandler#LexicalHandler
    * @see SAXException#SAXException
    * @see HandlerBase#HandlerBase
    */
    virtual void setLexicalHandler(LexicalHandler* const handler) ;

    //@}

    // -----------------------------------------------------------------------
    //  Getter Methods
    // -----------------------------------------------------------------------
    /** @name Getter Methods (Xerces-C specific) */
    //@{
    /**
	  * This method is used to get the current validator.
	  *
	  * <b>SAX2XMLReader assumes responsibility for the validator.  It will be
	  * deleted when the XMLReader is destroyed.</b>
	  *
	  * @return A pointer to the validator.  An application should not deleted
	  * the object returned.
	  *
	  */
	virtual XMLValidator* getValidator() const ;

    /** Get error count from the last parse operation.
      *
      * This method returns the error count from the last parse
      * operation. Note that this count is actually stored in the
      * scanner, so this method simply returns what the
      * scanner reports.
      *
      * @return number of errors encountered during the latest
      *			parse operation.
      */
    virtual XMLSize_t getErrorCount() const ;

    /**
      * This method returns the state of the parser's
      * exit-on-First-Fatal-Error flag.
      *
      * <p>Or you can query the feature "http://apache.org/xml/features/continue-after-fatal-error"
      * which indicates the opposite state.</p>
      *
      * @return true, if the parser is currently configured to
      *         exit on the first fatal error, false otherwise.
      *
      * @see #setExitOnFirstFatalError
      * @see #getFeature
      */
    virtual bool getExitOnFirstFatalError() const ;

    /**
      * This method returns the state of the parser's
      * validation-constraint-fatal flag.
      *
      * <p>Or you can query the feature "http://apache.org/xml/features/validation-error-as-fatal"
      * which means the same thing.
      *
      * @return true, if the parser is currently configured to
      *         set validation constraint errors as fatal, false
      *         otherwise.
      *
      * @see #setValidationContraintFatal
      * @see #getFeature
      */
    virtual bool getValidationConstraintFatal() const ;

    /**
      * Retrieve the grammar that is associated with the specified namespace key
      *
      * @param  nameSpaceKey Namespace key
      * @return Grammar associated with the Namespace key.
      */
    virtual Grammar* getGrammar(const XMLCh* const nameSpaceKey);

    /**
      * Retrieve the grammar where the root element is declared.
      *
      * @return Grammar where root element declared
      */
    virtual Grammar* getRootGrammar();

    /**
      * Returns the string corresponding to a URI id from the URI string pool.
      *
      * @param uriId id of the string in the URI string pool.
      * @return URI string corresponding to the URI id.
      */
    virtual const XMLCh* getURIText(unsigned int uriId) const;

    /**
      * Returns the current src offset within the input source.
      * To be used only while parsing is in progress.
      *
      * @return offset within the input source
      */
    virtual XMLFilePos getSrcOffset() const;

    //@}

    // -----------------------------------------------------------------------
    //  Setter Methods
    // -----------------------------------------------------------------------
    /** @name Setter Methods (Xerces-C specific) */
    //@{
    /**
	  * This method is used to set a validator.
	  *
	  * <b>SAX2XMLReader assumes responsibility for the validator.  It will be
	  * deleted when the XMLReader is destroyed.</b>
	  *
	  * @param valueToAdopt A pointer to the validator that the reader should use.
	  *
	  */
	virtual void setValidator(XMLValidator* valueToAdopt) ;

    /**
      * This method allows users to set the parser's behaviour when it
      * encounters the first fatal error. If set to true, the parser
      * will exit at the first fatal error. If false, then it will
      * report the error and continue processing.
      *
      * <p>The default value is 'true' and the parser exits on the
      * first fatal error.</p>
      *
      * <p>Or you can set the feature "http://apache.org/xml/features/continue-after-fatal-error"
      * which has the opposite behaviour.</p>
      *
      * <p>If both the feature above and this function are used, the latter takes effect.</p>
      *
      * @param newState The value specifying whether the parser should
      *                 continue or exit when it encounters the first
      *                 fatal error.
      *
      * @see #getExitOnFirstFatalError
      * @see #setFeature
      */
    virtual void setExitOnFirstFatalError(const bool newState) ;

    /**
      * This method allows users to set the parser's behaviour when it
      * encounters a validation constraint error. If set to true, and the
      * the parser will treat validation error as fatal and will exit depends on the
      * state of "getExitOnFirstFatalError". If false, then it will
      * report the error and continue processing.
      *
      * Note: setting this true does not mean the validation error will be printed with
      * the word "Fatal Error".   It is still printed as "Error", but the parser
      * will exit if "setExitOnFirstFatalError" is set to true.
      *
      * <p>The default value is 'false'.</p>
      *
      * <p>Or you can set the feature "http://apache.org/xml/features/validation-error-as-fatal"
      * which means the same thing.</p>
      *
      * <p>If both the feature above and this function are used, the latter takes effect.</p>
      *
      * @param newState If true, the parser will exit if "setExitOnFirstFatalError"
      *                 is set to true.
      *
      * @see #getValidationConstraintFatal
      * @see #setExitOnFirstFatalError
      * @see #setFeature
      */
    virtual void setValidationConstraintFatal(const bool newState) ;
    //@}


    // -----------------------------------------------------------------------
    //  Progressive scan methods
    // -----------------------------------------------------------------------

    /** @name Progressive scan methods */
    //@{

    /** Begin a progressive parse operation
      *
      * This method is used to start a progressive parse on a XML file.
      * To continue parsing, subsequent calls must be to the parseNext
      * method.
      *
      * It scans through the prolog and returns a token to be used on
      * subsequent scanNext() calls. If the return value is true, then the
      * token is legal and ready for further use. If it returns false, then
      * the scan of the prolog failed and the token is not going to work on
      * subsequent scanNext() calls.
      *
      * @param systemId A pointer to a Unicode string representing the path
      *                 to the XML file to be parsed.
      * @param toFill   A token maintaing state information to maintain
      *                 internal consistency between invocation of 'parseNext'
      *                 calls.
      *
      * @return 'true', if successful in parsing the prolog. It indicates the
      *         user can go ahead with parsing the rest of the file. It
      *         returns 'false' to indicate that the parser could parse the
      *         prolog (which means the token will not be valid.)
      *
      * @see #parseNext
      * @see #parseFirst(char*,...)
      * @see #parseFirst(InputSource&,...)
      */
    virtual bool parseFirst
    (
        const   XMLCh* const    systemId
        ,       XMLPScanToken&  toFill
    ) ;

    /** Begin a progressive parse operation
      *
      * This method is used to start a progressive parse on a XML file.
      * To continue parsing, subsequent calls must be to the parseNext
      * method.
      *
      * It scans through the prolog and returns a token to be used on
      * subsequent scanNext() calls. If the return value is true, then the
      * token is legal and ready for further use. If it returns false, then
      * the scan of the prolog failed and the token is not going to work on
      * subsequent scanNext() calls.
      *
      * @param systemId A pointer to a regular native string representing
      *                 the path to the XML file to be parsed.
      * @param toFill   A token maintaing state information to maintain
      *                 internal consistency between invocation of 'parseNext'
      *                 calls.
      *
      * @return 'true', if successful in parsing the prolog. It indicates the
      *         user can go ahead with parsing the rest of the file. It
      *         returns 'false' to indicate that the parser could not parse
      *         the prolog.
      *
      * @see #parseNext
      * @see #parseFirst(XMLCh*,...)
      * @see #parseFirst(InputSource&,...)
      */
    virtual bool parseFirst
    (
        const   char* const     systemId
        ,       XMLPScanToken&  toFill
    ) ;

    /** Begin a progressive parse operation
      *
      * This method is used to start a progressive parse on a XML file.
      * To continue parsing, subsequent calls must be to the parseNext
      * method.
      *
      * It scans through the prolog and returns a token to be used on
      * subsequent scanNext() calls. If the return value is true, then the
      * token is legal and ready for further use. If it returns false, then
      * the scan of the prolog failed and the token is not going to work on
      * subsequent scanNext() calls.
      *
      * @param source   A const reference to the InputSource object which
      *                 points to the XML file to be parsed.
      * @param toFill   A token maintaing state information to maintain
      *                 internal consistency between invocation of 'parseNext'
      *                 calls.
      *
      * @return 'true', if successful in parsing the prolog. It indicates the
      *         user can go ahead with parsing the rest of the file. It
      *         returns 'false' to indicate that the parser could not parse
      *         the prolog.
      *
      * @see #parseNext
      * @see #parseFirst(XMLCh*,...)
      * @see #parseFirst(char*,...)
      */
    virtual bool parseFirst
    (
        const   InputSource&    source
        ,       XMLPScanToken&  toFill
    ) ;

    /** Continue a progressive parse operation
      *
      * This method is used to continue with progressive parsing of
      * XML files started by a call to 'parseFirst' method.
      *
      * It parses the XML file and stops as soon as it comes across
      * a XML token (as defined in the XML specification). Relevant
      * callback handlers are invoked as required by the SAX
      * specification.
      *
      * @param token A token maintaing state information to maintain
      *              internal consistency between invocation of 'parseNext'
      *              calls.
      *
      * @return 'true', if successful in parsing the next XML token.
      *         It indicates the user can go ahead with parsing the rest
      *         of the file. It returns 'false' to indicate that the parser
      *         could not find next token as per the XML specification
      *         production rule.
      *
      * @see #parseFirst(XMLCh*,...)
      * @see #parseFirst(char*,...)
      * @see #parseFirst(InputSource&,...)
      */
    virtual bool parseNext(XMLPScanToken& token) ;

    /** Reset the parser after a progressive parse
      *
      * If a progressive parse loop exits before the end of the document
      * is reached, the parser has no way of knowing this. So it will leave
      * open any files or sockets or memory buffers that were in use at
      * the time that the parse loop exited.
      *
      * The next parse operation will cause these open files and such to
      * be closed, but the next parse operation might occur at some unknown
      * future point. To avoid this problem, you should reset the parser if
      * you exit the loop early.
      *
      * If you exited because of an error, then this cleanup will be done
      * for you. Its only when you exit the file prematurely of your own
      * accord, because you've found what you wanted in the file most
      * likely.
      *
      * @param token A token maintaing state information to maintain
      *              internal consistency between invocation of 'parseNext'
      *              calls.
      */
    virtual void parseReset(XMLPScanToken& token) ;

    //@}

    // -----------------------------------------------------------------------
    //  Implementation of the grammar preparsing interface
    // -----------------------------------------------------------------------

    /** @name Implementation of Grammar preparsing interface's. */
    //@{
    /**
      * Preparse schema grammar (XML Schema, DTD, etc.) via an input source
      * object.
      *
      * This method invokes the preparsing process on a schema grammar XML
      * file specified by the SAX InputSource parameter. If the 'toCache' flag
      * is enabled, the parser will cache the grammars for re-use. If a grammar
      * key is found in the pool, no caching of any grammar will take place.
      *
      *
      * @param source A const reference to the SAX InputSource object which
      *               points to the schema grammar file to be preparsed.
      * @param grammarType The grammar type (Schema or DTD).
      * @param toCache If <code>true</code>, we cache the preparsed grammar,
      *                otherwise, no caching. Default is <code>false</code>.
      * @return The preparsed schema grammar object (SchemaGrammar or
      *         DTDGrammar). That grammar object is owned by the parser.
      *
      * @exception SAXException Any SAX exception, possibly
      *            wrapping another exception.
      * @exception XMLException An exception from the parser or client
      *            handler code.
      * @exception DOMException A DOM exception as per DOM spec.
      *
      * @see InputSource#InputSource
      */
    virtual Grammar* loadGrammar(const InputSource& source,
                                 const Grammar::GrammarType grammarType,
                                 const bool toCache = false);

    /**
      * Preparse schema grammar (XML Schema, DTD, etc.) via a file path or URL
      *
      * This method invokes the preparsing process on a schema grammar XML
      * file specified by the file path parameter. If the 'toCache' flag
      * is enabled, the parser will cache the grammars for re-use. If a grammar
      * key is found in the pool, no caching of any grammar will take place.
      *
      *
      * @param systemId A const XMLCh pointer to the Unicode string which
      *                 contains the path to the XML grammar file to be
      *                 preparsed.
      * @param grammarType The grammar type (Schema or DTD).
      * @param toCache If <code>true</code>, we cache the preparsed grammar,
      *                otherwise, no caching. Default is <code>false</code>.
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
                                 const bool toCache = false);

    /**
      * Preparse schema grammar (XML Schema, DTD, etc.) via a file path or URL
      *
      * This method invokes the preparsing process on a schema grammar XML
      * file specified by the file path parameter. If the 'toCache' flag
      * is enabled, the parser will cache the grammars for re-use. If a grammar
      * key is found in the pool, no caching of any grammar will take place.
      *
      *
      * @param systemId A const char pointer to a native string which contains
      *                 the path to the XML grammar file to be preparsed.
      * @param grammarType The grammar type (Schema or DTD).
      * @param toCache If <code>true</code>, we cache the preparsed grammar,
      *                otherwise, no caching. Default is <code>false</code>.
      * @return The preparsed schema grammar object (SchemaGrammar or
      *         DTDGrammar). That grammar object is owned by the parser.
      *
      * @exception SAXException Any SAX exception, possibly
      *            wrapping another exception.
      * @exception XMLException An exception from the parser or client
      *            handler code.
      * @exception DOMException A DOM exception as per DOM spec.
      */
    virtual Grammar* loadGrammar(const char* const systemId,
                                 const Grammar::GrammarType grammarType,
                                 const bool toCache = false);

    /**
      * Clear the cached grammar pool
      */
    virtual void resetCachedGrammarPool();

    /** Set maximum input buffer size
      *
      * This method allows users to limit the size of buffers used in parsing
      * XML character data. The effect of setting this size is to limit the
      * size of a ContentHandler::characters() call.
      *
      * The parser's default input buffer size is 1 megabyte.
      *
      * @param bufferSize The maximum input buffer size
      */
    void setInputBufferSize(const XMLSize_t bufferSize);

    //@}


    // -----------------------------------------------------------------------
    //  Advanced document handler list maintenance methods
    // -----------------------------------------------------------------------

    /** @name Advanced document handler list maintenance methods */
    //@{
    /**
      * This method installs the specified 'advanced' document callback
      * handler, thereby allowing the user to customize the processing,
      * if they choose to do so. Any number of advanced callback handlers
      * maybe installed.
      *
      * <p>The methods in the advanced callback interface represent
      * Xerces-C extensions. There is no specification for this interface.</p>
      *
      * @param toInstall A pointer to the users advanced callback handler.
      *
      * @see #removeAdvDocHandler
      */
    virtual void installAdvDocHandler(XMLDocumentHandler* const toInstall) ;

    /**
      * This method removes the 'advanced' document handler callback from
      * the underlying parser scanner. If no handler is installed, advanced
      * callbacks are not invoked by the scanner.
      * @param toRemove A pointer to the advanced callback handler which
      *                 should be removed.
      *
      * @see #installAdvDocHandler
      */
    virtual bool removeAdvDocHandler(XMLDocumentHandler* const toRemove) ;
    //@}


    // -----------------------------------------------------------------------
    //  The XMLFilter interface
    // -----------------------------------------------------------------------

    /** @name Implementation of SAX 2.0 XMLFilter interface's. */
    //@{
    /**
      * This method returns the parent XMLReader object.
      *
      * @return A pointer to the parent XMLReader object.
      */
    virtual SAX2XMLReader* getParent() const;

    /**
      * Sets the parent XMLReader object; parse requests will be forwarded to this
      * object, and callback notifications coming from it will be postprocessed
      *
      * @param parent The new XMLReader parent.
      * @see SAX2XMLReader#SAX2XMLReader
      */
    virtual void setParent(SAX2XMLReader* parent);
    //@}

    // -----------------------------------------------------------------------
    //  Implementation of the EntityResolver interface
    // -----------------------------------------------------------------------
    /** @name The EntityResolver interface */
    //@{

  /**
    * Allow the application to resolve external entities.
    *
    * <p>The Parser will call this method before opening any external
    * entity except the top-level document entity (including the
    * external DTD subset, external entities referenced within the
    * DTD, and external entities referenced within the document
    * element): the application may request that the parser resolve
    * the entity itself, that it use an alternative URI, or that it
    * use an entirely different input source.</p>
    *
    * <p>Application writers can use this method to redirect external
    * system identifiers to secure and/or local URIs, to look up
    * public identifiers in a catalogue, or to read an entity from a
    * database or other input source (including, for example, a dialog
    * box).</p>
    *
    * <p>If the system identifier is a URL, the SAX parser must
    * resolve it fully before reporting it to the application.</p>
    *
    * @param publicId The public identifier of the external entity
    *        being referenced, or null if none was supplied.
    * @param systemId The system identifier of the external entity
    *        being referenced.
    * @return An InputSource object describing the new input source,
    *         or null to request that the parser open a regular
    *         URI connection to the system identifier.
    *         The returned InputSource is owned by the parser which is
    *         responsible to clean up the memory.
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    * @exception IOException An IO exception,
    *            possibly the result of creating a new InputStream
    *            or Reader for the InputSource.
    * @see InputSource#InputSource
    */
    virtual InputSource* resolveEntity
    (
        const   XMLCh* const    publicId
        , const XMLCh* const    systemId
    );

    //@}

    // -----------------------------------------------------------------------
    //  Implementation of the DTDHandler interface
    // -----------------------------------------------------------------------
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
    );

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
    );

    /**
    * Reset the DocType object on its reuse
    *
    * <p>This method helps in reseting the DTD object implementation
    * defaults each time the DTD is begun.</p>
    *
    */
    virtual void resetDocType();

    //@}

    // -----------------------------------------------------------------------
    //  Implementation of the ContentHandler interface
    // -----------------------------------------------------------------------
    /** @name The virtual document handler interface */

    //@{
   /**
    * Receive notification of character data.
    *
    * <p>The Parser will call this method to report each chunk of
    * character data.  SAX parsers may return all contiguous character
    * data in a single chunk, or they may split it into several
    * chunks; however, all of the characters in any single event
    * must come from the same external entity, so that the Locator
    * provides useful information.</p>
    *
    * <p>The application must not attempt to read from the array
    * outside of the specified range.</p>
    *
    * <p>Note that some parsers will report whitespace using the
    * ignorableWhitespace() method rather than this one (validating
    * parsers must do so).</p>
    *
    * @param chars The characters from the XML document.
    * @param length The number of characters to read from the array.
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    * @see #ignorableWhitespace
    * @see Locator#Locator
    */
    virtual void characters
    (
        const   XMLCh* const    chars
        , const XMLSize_t       length
    );

  /**
    * Receive notification of the end of a document.
    *
    * <p>The SAX parser will invoke this method only once, and it will
    * be the last method invoked during the parse.  The parser shall
    * not invoke this method until it has either abandoned parsing
    * (because of an unrecoverable error) or reached the end of
    * input.</p>
    *
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    */
    virtual void endDocument ();

  /**
    * Receive notification of the end of an element.
    *
    * <p>The SAX parser will invoke this method at the end of every
    * element in the XML document; there will be a corresponding
    * startElement() event for every endElement() event (even when the
    * element is empty).</p>
    *
    * @param uri The URI of the associated namespace for this element
	* @param localname The local part of the element name
	* @param qname The QName of this element
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    */
    virtual void endElement
	(
		const XMLCh* const uri,
		const XMLCh* const localname,
		const XMLCh* const qname
	);

  /**
    * Receive notification of ignorable whitespace in element content.
    *
    * <p>Validating Parsers must use this method to report each chunk
    * of ignorable whitespace (see the W3C XML 1.0 recommendation,
    * section 2.10): non-validating parsers may also use this method
    * if they are capable of parsing and using content models.</p>
    *
    * <p>SAX parsers may return all contiguous whitespace in a single
    * chunk, or they may split it into several chunks; however, all of
    * the characters in any single event must come from the same
    * external entity, so that the Locator provides useful
    * information.</p>
    *
    * <p>The application must not attempt to read from the array
    * outside of the specified range.</p>
    *
    * @param chars The characters from the XML document.
    * @param length The number of characters to read from the array.
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    * @see #characters
    */
    virtual void ignorableWhitespace
    (
        const   XMLCh* const    chars
        , const XMLSize_t       length
    );

  /**
    * Receive notification of a processing instruction.
    *
    * <p>The Parser will invoke this method once for each processing
    * instruction found: note that processing instructions may occur
    * before or after the main document element.</p>
    *
    * <p>A SAX parser should never report an XML declaration (XML 1.0,
    * section 2.8) or a text declaration (XML 1.0, section 4.3.1)
    * using this method.</p>
    *
    * @param target The processing instruction target.
    * @param data The processing instruction data, or null if
    *        none was supplied.
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    */
    virtual void processingInstruction
    (
        const   XMLCh* const    target
        , const XMLCh* const    data
    );

  /**
    * Receive an object for locating the origin of SAX document events.
    *
    * SAX parsers are strongly encouraged (though not absolutely
    * required) to supply a locator: if it does so, it must supply
    * the locator to the application by invoking this method before
    * invoking any of the other methods in the DocumentHandler
    * interface.
    *
    * The locator allows the application to determine the end
    * position of any document-related event, even if the parser is
    * not reporting an error.  Typically, the application will
    * use this information for reporting its own errors (such as
    * character content that does not match an application's
    * business rules). The information returned by the locator
    * is probably not sufficient for use with a search engine.
    *
    * Note that the locator will return correct information only
    * during the invocation of the events in this interface. The
    * application should not attempt to use it at any other time.
    *
    * @param locator An object that can return the location of
    *                any SAX document event. The object is only
    *                'on loan' to the client code and they are not
    *                to attempt to delete or modify it in any way!
    *
    * @see Locator#Locator
    */
    virtual void setDocumentLocator(const Locator* const locator);

  /**
    * Receive notification of the beginning of a document.
    *
    * <p>The SAX parser will invoke this method only once, before any
    * other methods in this interface or in DTDHandler (except for
    * setDocumentLocator).</p>
    *
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    */
    virtual void startDocument();

  /**
    * Receive notification of the beginning of an element.
    *
    * <p>The Parser will invoke this method at the beginning of every
    * element in the XML document; there will be a corresponding
    * endElement() event for every startElement() event (even when the
    * element is empty). All of the element's content will be
    * reported, in order, before the corresponding endElement()
    * event.</p>
    *
    * <p>Note that the attribute list provided will
    * contain only attributes with explicit values (specified or
    * defaulted): #IMPLIED attributes will be omitted.</p>
    *
    * @param uri The URI of the associated namespace for this element
	* @param localname The local part of the element name
	* @param qname The QName of this element
    * @param attrs The attributes attached to the element, if any.
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    * @see #endElement
    * @see Attributes#Attributes
    */
    virtual void startElement
    (
        const   XMLCh* const    uri,
        const   XMLCh* const    localname,
        const   XMLCh* const    qname,
        const   Attributes&     attrs
    );

  /**
    * Receive notification of the start of an namespace prefix mapping.
    *
    * <p>By default, do nothing.  Application writers may override this
    * method in a subclass to take specific actions at the start of
    * each namespace prefix mapping.</p>
    *
    * @param prefix The namespace prefix used
    * @param uri The namespace URI used.
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    */
	virtual void startPrefixMapping
	(
		const	XMLCh* const	prefix,
		const	XMLCh* const	uri
	);

  /**
    * Receive notification of the end of an namespace prefix mapping.
    *
    * <p>By default, do nothing.  Application writers may override this
    * method in a subclass to take specific actions at the end of
    * each namespace prefix mapping.</p>
    *
    * @param prefix The namespace prefix used
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    */
	virtual void endPrefixMapping
	(
		const	XMLCh* const	prefix
	);

  /**
    * Receive notification of a skipped entity
    *
    * <p>The parser will invoke this method once for each entity
	* skipped.  All processors may skip external entities,
	* depending on the values of the features:<br>
	* http://xml.org/sax/features/external-general-entities<br>
	* http://xml.org/sax/features/external-parameter-entities</p>
	*
	* <p>Note: Xerces (specifically) never skips any entities, regardless
	* of the above features.  This function is never called in the
	* Xerces implementation of SAX2.</p>
    *
	* <p>Introduced with SAX2</p>
	*
    * @param name The name of the skipped entity.  If it is a parameter entity,
	* the name will begin with %, and if it is the external DTD subset,
	* it will be the string [dtd].
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    */
	virtual void skippedEntity
	(
		const	XMLCh* const	name
	);

    //@}

    // -----------------------------------------------------------------------
    //  Implementation of the ErrorHandler interface
    // -----------------------------------------------------------------------
    /** @name The error handler interface */
    //@{
   /**
    * Receive notification of a warning.
    *
    * <p>SAX parsers will use this method to report conditions that
    * are not errors or fatal errors as defined by the XML 1.0
    * recommendation.  The default behaviour is to take no action.</p>
    *
    * <p>The SAX parser must continue to provide normal parsing events
    * after invoking this method: it should still be possible for the
    * application to process the document through to the end.</p>
    *
    * @param exc The warning information encapsulated in a
    *            SAX parse exception.
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    * @see SAXParseException#SAXParseException
    */
    virtual void warning(const SAXParseException& exc);

  /**
    * Receive notification of a recoverable error.
    *
    * <p>This corresponds to the definition of "error" in section 1.2
    * of the W3C XML 1.0 Recommendation.  For example, a validating
    * parser would use this callback to report the violation of a
    * validity constraint.  The default behaviour is to take no
    * action.</p>
    *
    * <p>The SAX parser must continue to provide normal parsing events
    * after invoking this method: it should still be possible for the
    * application to process the document through to the end.  If the
    * application cannot do so, then the parser should report a fatal
    * error even if the XML 1.0 recommendation does not require it to
    * do so.</p>
    *
    * @param exc The error information encapsulated in a
    *            SAX parse exception.
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    * @see SAXParseException#SAXParseException
    */
    virtual void error(const SAXParseException& exc);

  /**
    * Receive notification of a non-recoverable error.
    *
    * <p>This corresponds to the definition of "fatal error" in
    * section 1.2 of the W3C XML 1.0 Recommendation.  For example, a
    * parser would use this callback to report the violation of a
    * well-formedness constraint.</p>
    *
    * <p>The application must assume that the document is unusable
    * after the parser has invoked this method, and should continue
    * (if at all) only for the sake of collecting addition error
    * messages: in fact, SAX parsers are free to stop reporting any
    * other events once this method has been invoked.</p>
    *
    * @param exc The error information encapsulated in a
    *            SAX parse exception.
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    * @see SAXParseException#SAXParseException
    */
    virtual void fatalError(const SAXParseException& exc);

    /**
    * Reset the Error handler object on its reuse
    *
    * <p>This method helps in reseting the Error handler object
    * implementation defaults each time the Error handler is begun.</p>
    *
    */
    virtual void resetErrors();

    //@}


private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    SAX2XMLFilterImpl(const SAX2XMLFilterImpl&);
    SAX2XMLFilterImpl& operator=(const SAX2XMLFilterImpl&);

    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fParentReader
    //      The object that we are filtering
    //
    //  fDocHandler
    //      The installed SAX content handler, if any. Null if none.
    //
    //  fDTDHandler
    //      The installed SAX DTD handler, if any. Null if none.
    //
    //  fEntityResolver
    //      The installed SAX entity handler, if any. Null if none.
    //
    //  fErrorHandler
    //      The installed SAX error handler, if any. Null if none.
    //
    // -----------------------------------------------------------------------
    SAX2XMLReader*              fParentReader;
    ContentHandler*		        fDocHandler;
    DTDHandler*                 fDTDHandler;
    EntityResolver*             fEntityResolver;
    ErrorHandler*               fErrorHandler;
};


// ---------------------------------------------------------------------------
//  SAX2XMLReader: Getter methods
// ---------------------------------------------------------------------------
inline SAX2XMLReader* SAX2XMLFilterImpl::getParent() const
{
    return fParentReader;
}

inline ContentHandler* SAX2XMLFilterImpl::getContentHandler() const
{
    return fDocHandler;
}

inline DTDHandler* SAX2XMLFilterImpl::getDTDHandler() const
{
	return fDTDHandler;
}

inline EntityResolver* SAX2XMLFilterImpl::getEntityResolver() const
{
	return fEntityResolver;
}

inline ErrorHandler* SAX2XMLFilterImpl::getErrorHandler() const
{
	return fErrorHandler;
}

inline LexicalHandler* SAX2XMLFilterImpl::getLexicalHandler() const
{
   return 0;
}

inline DeclHandler* SAX2XMLFilterImpl::getDeclarationHandler() const
{
   return 0;
}

inline void SAX2XMLFilterImpl::setContentHandler(ContentHandler* const handler)
{
	fDocHandler = handler;
}

inline void SAX2XMLFilterImpl::setDTDHandler(DTDHandler* const handler)
{
    fDTDHandler = handler;
}

inline void SAX2XMLFilterImpl::setErrorHandler(ErrorHandler* const handler)
{
    fErrorHandler = handler;
}

inline void SAX2XMLFilterImpl::setEntityResolver(EntityResolver* const resolver)
{
    fEntityResolver = resolver;
}

inline void SAX2XMLFilterImpl::setLexicalHandler(LexicalHandler* const /*handler*/)
{
}

inline void SAX2XMLFilterImpl::setDeclarationHandler(DeclHandler* const /*handler*/)
{
}

inline void SAX2XMLFilterImpl::installAdvDocHandler(XMLDocumentHandler* const /*toInstall*/)
{
}

inline bool SAX2XMLFilterImpl::removeAdvDocHandler(XMLDocumentHandler* const /*toRemove*/)
{
    return false;
}

XERCES_CPP_NAMESPACE_END

#endif
