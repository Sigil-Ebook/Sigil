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
 * $Id: XercesDOMParser.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XERCESDOMPARSER_HPP)
#define XERCESC_INCLUDE_GUARD_XERCESDOMPARSER_HPP


#include <xercesc/parsers/AbstractDOMParser.hpp>
#include <xercesc/validators/common/Grammar.hpp>

XERCES_CPP_NAMESPACE_BEGIN


class EntityResolver;
class ErrorHandler;
class XMLEntityResolver;
class XMLResourceIdentifier;

 /**
  * This class implements the Document Object Model (DOM) interface.
  * It should be used by applications which choose to parse and
  * process the XML document using the DOM api's. This implementation
  * also allows the applications to install an error and an entity
  * handler (useful extensions to the DOM specification).
  *
  * <p>It can be used to instantiate a validating or non-validating
  * parser, by setting a member flag.</p>
  */
class PARSERS_EXPORT XercesDOMParser : public AbstractDOMParser
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------

    /** @name Constructors and Destructor */
    //@{
    /** Construct a XercesDOMParser, with an optional validator
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
      * @param  manager   Pointer to the memory manager to be used to
      *                   allocate objects.
      */
    XercesDOMParser
    (
          XMLValidator* const   valToAdopt = 0
        , MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
        , XMLGrammarPool* const gramPool = 0
    );

    /**
      * Destructor
      */
    virtual ~XercesDOMParser();

    //@}


    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------

    /** @name Getter methods */
    //@{

    /** Get a pointer to the error handler
      *
      * This method returns the installed error handler. If no handler
      * has been installed, then it will be a zero pointer.
      *
      * @return The pointer to the installed error handler object.
      */
    ErrorHandler* getErrorHandler();

    /** Get a const pointer to the error handler
      *
      * This method returns the installed error handler.  If no handler
      * has been installed, then it will be a zero pointer.
      *
      * @return A const pointer to the installed error handler object.
      */
    const ErrorHandler* getErrorHandler() const;

    /** Get a pointer to the entity resolver
      *
      * This method returns the installed entity resolver.  If no resolver
      * has been installed, then it will be a zero pointer.
      *
      * @return The pointer to the installed entity resolver object.
      */
    EntityResolver* getEntityResolver();

    /** Get a const pointer to the entity resolver
      *
      * This method returns the installed entity resolver. If no resolver
      * has been installed, then it will be a zero pointer.
      *
      * @return A const pointer to the installed entity resolver object.
      */
    const EntityResolver* getEntityResolver() const;

    /**
      * Get a pointer to the entity resolver
      *
      * This method returns the installed entity resolver.  If no resolver
      * has been installed, then it will be a zero pointer.
      *
      * @return The pointer to the installed entity resolver object.
      */
    XMLEntityResolver* getXMLEntityResolver();

    /**
      * Get a const pointer to the entity resolver
      *
      * This method returns the installed entity resolver. If no resolver
      * has been installed, then it will be a zero pointer.
      *
      * @return A const pointer to the installed entity resolver object.
      */
    const XMLEntityResolver* getXMLEntityResolver() const;

    /** Get the 'Grammar caching' flag
      *
      * This method returns the state of the parser's grammar caching when
      * parsing an XML document.
      *
      * @return true, if the parser is currently configured to
      *         cache grammars, false otherwise.
      *
      * @see #cacheGrammarFromParse
      */
    bool isCachingGrammarFromParse() const;

    /** Get the 'Use cached grammar' flag
      *
      * This method returns the state of the parser's use of cached grammar
      * when parsing an XML document.
      *
      * @return true, if the parser is currently configured to
      *         use cached grammars, false otherwise.
      *
      * @see #useCachedGrammarInParse
      */
    bool isUsingCachedGrammarInParse() const;

    /**
     * Retrieve the grammar that is associated with the specified namespace key
     *
     * @param  nameSpaceKey Namespace key
     * @return Grammar associated with the Namespace key.
     */
    Grammar* getGrammar(const XMLCh* const nameSpaceKey);

    /**
     * Retrieve the grammar where the root element is declared.
     *
     * @return Grammar where root element declared
     */
    Grammar* getRootGrammar();

    /**
     * Returns the string corresponding to a URI id from the URI string pool.
     *
     * @param uriId id of the string in the URI string pool.
     * @return URI string corresponding to the URI id.
     */
    const XMLCh* getURIText(unsigned int uriId) const;

    /**
     * Returns the current src offset within the input source.
     * To be used only while parsing is in progress.
     *
     * @return offset within the input source
     */
    XMLFilePos getSrcOffset() const;

    /** Get the 'ignore cached DTD grammar' flag
      *
      * @return true, if the parser is currently configured to
      *         ignore cached DTD, false otherwise.
      *
      * @see #setIgnoreCachedDTD
      */
    bool getIgnoreCachedDTD() const;

    //@}


    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------

    /** @name Setter methods */
    //@{

    /** Set the error handler
      *
      * This method allows applications to install their own error handler
      * to trap error and warning messages.
      *
      * <i>Any previously set handler is merely dropped, since the parser
      * does not own them.</i>
      *
      * @param handler  A const pointer to the user supplied error
      *                 handler.
      *
      * @see #getErrorHandler
      */
    void setErrorHandler(ErrorHandler* const handler);

    /** Set the entity resolver
      *
      * This method allows applications to install their own entity
      * resolver. By installing an entity resolver, the applications
      * can trap and potentially redirect references to external
      * entities.
      *
      * <i>Any previously set entity resolver is merely dropped, since the parser
      * does not own them.  If both setEntityResolver and setXMLEntityResolver
      * are called, then the last one is used.</i>
      *
      * @param handler  A const pointer to the user supplied entity
      *                 resolver.
      *
      * @see #getEntityResolver
      */
    void setEntityResolver(EntityResolver* const handler);

    /**
      * Set the entity resolver
      *
      * This method allows applications to install their own entity
      * resolver. By installing an entity resolver, the applications
      * can trap and potentially redirect references to external
      * entities.
      *
      * <i>Any previously set entity resolver is merely dropped, since the parser
      * does not own them.  If both setEntityResolver and setXMLEntityResolver
      * are called, then the last one set is used.</i>
      *
      * @param handler  A const pointer to the user supplied entity
      *                 resolver.
      *
      * @see #getXMLEntityResolver
      */
    void setXMLEntityResolver(XMLEntityResolver* const handler);

    /** Set the 'Grammar caching' flag
      *
      * This method allows users to enable or disable caching of grammar when
      * parsing XML documents. When set to true, the parser will cache the
      * resulting grammar for use in subsequent parses.
      *
      * If the flag is set to true, the 'Use cached grammar' flag will also be
      * set to true.
      *
      * The parser's default state is: false.
      *
      * @param newState The value specifying whether we should cache grammars
      *                 or not.
      *
      * @see #isCachingGrammarFromParse
      * @see #useCachedGrammarInParse
      */
    void cacheGrammarFromParse(const bool newState);

    /** Set the 'Use cached grammar' flag
      *
      * This method allows users to enable or disable the use of cached
      * grammars.  When set to true, the parser will use the cached grammar,
      * instead of building the grammar from scratch, to validate XML
      * documents.
      *
      * If the 'Grammar caching' flag is set to true, this method ignore the
      * value passed in.
      *
      * The parser's default state is: false.
      *
      * @param newState The value specifying whether we should use the cached
      *                 grammar or not.
      *
      * @see #isUsingCachedGrammarInParse
      * @see #cacheGrammarFromParse
      */
    void useCachedGrammarInParse(const bool newState);

    /** Set the 'ignore cached DTD grammar' flag
      *
      * This method gives users the option to ignore a cached DTD grammar, when
      * an XML document contains both an internal and external DTD, and the use
      * cached grammar from parse option is enabled. Currently, we do not allow
      * using cached DTD grammar when an internal subset is present in the
      * document. This option will only affect the behavior of the parser when
      * an internal and external DTD both exist in a document (i.e. no effect
      * if document has no internal subset).
      *
      * The parser's default state is false
      *
      * @param newValue The state to set
      */
    void setIgnoreCachedDTD(const bool newValue);

    //@}

    // -----------------------------------------------------------------------
    //  Utility methods
    // -----------------------------------------------------------------------

    /** @name Utility methods */
    //@{
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
    void resetDocumentPool();

    //@}

    // -----------------------------------------------------------------------
    //  Implementation of the XMLErrorReporter interface.
    // -----------------------------------------------------------------------

    /** @name Implementation of the XMLErrorReporter interface. */
    //@{

    /** Handle errors reported from the parser
      *
      * This method is used to report back errors found while parsing the
      * XML file. This method is also borrowed from the SAX specification.
      * It calls the corresponding user installed Error Handler method:
      * 'fatal', 'error', 'warning' depending on the severity of the error.
      * This classification is defined by the XML specification.
      *
      * @param errCode An integer code for the error.
      * @param msgDomain A const pointer to an Unicode string representing
      *                  the message domain to use.
      * @param errType An enumeration classifying the severity of the error.
      * @param errorText A const pointer to an Unicode string representing
      *                  the text of the error message.
      * @param systemId  A const pointer to an Unicode string representing
      *                  the system id of the XML file where this error
      *                  was discovered.
      * @param publicId  A const pointer to an Unicode string representing
      *                  the public id of the XML file where this error
      *                  was discovered.
      * @param lineNum   The line number where the error occurred.
      * @param colNum    The column number where the error occurred.
      * @see ErrorHandler
      */
    virtual void error
    (
        const   unsigned int                errCode
        , const XMLCh* const                msgDomain
        , const XMLErrorReporter::ErrTypes  errType
        , const XMLCh* const                errorText
        , const XMLCh* const                systemId
        , const XMLCh* const                publicId
        , const XMLFileLoc                  lineNum
        , const XMLFileLoc                  colNum
    );

    /** Reset any error data before a new parse
     *
      * This method allows the user installed Error Handler callback to
      * 'reset' itself.
      *
      * <b>This method is a no-op for this DOM
      * implementation.</b>
      */
    virtual void resetErrors();
    //@}


    // -----------------------------------------------------------------------
    //  Implementation of the XMLEntityHandler interface.
    // -----------------------------------------------------------------------

    /** @name Implementation of the XMLEntityHandler interface. */
    //@{

    /** Handle an end of input source event
      *
      * This method is used to indicate the end of parsing of an external
      * entity file.
      *
      * <b>This method is a no-op for this DOM
      * implementation.</b>
      *
      * @param inputSource A const reference to the InputSource object
      *                    which points to the XML file being parsed.
      * @see InputSource
      */
    virtual void endInputSource(const InputSource& inputSource);

    /** Expand a system id
      *
      * This method allows an installed XMLEntityHandler to further
      * process any system id's of external entities encountered in
      * the XML file being parsed, such as redirection etc.
      *
      * <b>This method always returns 'false'
      * for this DOM implementation.</b>
      *
      * @param systemId  A const pointer to an Unicode string representing
      *                  the system id scanned by the parser.
      * @param toFill    A pointer to a buffer in which the application
      *                  processed system id is stored.
      * @return 'true', if any processing is done, 'false' otherwise.
      */
    virtual bool expandSystemId
    (
        const   XMLCh* const    systemId
        ,       XMLBuffer&      toFill
    );

    /** Reset any entity handler information
      *
      * This method allows the installed XMLEntityHandler to reset
      * itself.
      *
      * <b>This method is a no-op for this DOM
      * implementation.</b>
      */
    virtual void resetEntities();

    /** Resolve a public/system id
      *
      * This method allows a user installed entity handler to further
      * process any pointers to external entities. The applications can
      * implement 'redirection' via this callback.
      *
      * @param resourceIdentifier An object containing the type of
      *        resource to be resolved and the associated data members
      *        corresponding to this type.
      * @return The value returned by the user installed resolveEntity
      *         method or NULL otherwise to indicate no processing was done.
      *         The returned InputSource is owned by the parser which is
      *         responsible to clean up the memory.
      * @see XMLEntityHandler
      * @see XMLEntityResolver
      */
    virtual InputSource* resolveEntity
    (
        XMLResourceIdentifier* resourceIdentifier
    );

    /** Handle a 'start input source' event
      *
      * This method is used to indicate the start of parsing an external
      * entity file.
      *
      * <b>This method is a no-op for this DOM parse
      * implementation.</b>
      *
      * @param inputSource A const reference to the InputSource object
      *                    which points to the external entity
      *                    being parsed.
      */
    virtual void startInputSource(const InputSource& inputSource);

    //@}

    // -----------------------------------------------------------------------
    //  Grammar preparsing interface
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
    Grammar* loadGrammar(const InputSource& source,
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
    Grammar* loadGrammar(const XMLCh* const systemId,
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
    Grammar* loadGrammar(const char* const systemId,
                         const Grammar::GrammarType grammarType,
                         const bool toCache = false);

    /**
      * This method allows the user to reset the pool of cached grammars.
      */
    void resetCachedGrammarPool();

    //@}


private :
    // -----------------------------------------------------------------------
    //  Initialize/Cleanup methods
    // -----------------------------------------------------------------------
    void resetParse();

    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XercesDOMParser(const XercesDOMParser&);
    XercesDOMParser& operator=(const XercesDOMParser&);

    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fEntityResolver
    //      The installed SAX entity resolver, if any. Null if none.
    //
    //  fErrorHandler
    //      The installed SAX error handler, if any. Null if none.
    //-----------------------------------------------------------------------
    EntityResolver*          fEntityResolver;
    XMLEntityResolver*       fXMLEntityResolver;
    ErrorHandler*            fErrorHandler;
};



// ---------------------------------------------------------------------------
//  XercesDOMParser: Handlers for the XMLEntityHandler interface
// ---------------------------------------------------------------------------
inline void XercesDOMParser::endInputSource(const InputSource&)
{
    // The DOM entity resolver doesn't handle this
}

inline bool XercesDOMParser::expandSystemId(const XMLCh* const, XMLBuffer&)
{
    // The DOM entity resolver doesn't handle this
    return false;
}

inline void XercesDOMParser::resetEntities()
{
    // Nothing to do on this one
}

inline void XercesDOMParser::startInputSource(const InputSource&)
{
    // The DOM entity resolver doesn't handle this
}


// ---------------------------------------------------------------------------
//  XercesDOMParser: Getter methods
// ---------------------------------------------------------------------------
inline ErrorHandler* XercesDOMParser::getErrorHandler()
{
    return fErrorHandler;
}

inline const ErrorHandler* XercesDOMParser::getErrorHandler() const
{
    return fErrorHandler;
}

inline EntityResolver* XercesDOMParser::getEntityResolver()
{
    return fEntityResolver;
}

inline const EntityResolver* XercesDOMParser::getEntityResolver() const
{
    return fEntityResolver;
}

inline XMLEntityResolver* XercesDOMParser::getXMLEntityResolver()
{
    return fXMLEntityResolver;
}

inline const XMLEntityResolver* XercesDOMParser::getXMLEntityResolver() const
{
    return fXMLEntityResolver;
}

XERCES_CPP_NAMESPACE_END

#endif
