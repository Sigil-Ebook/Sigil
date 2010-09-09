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
 * $Id: DOMLSParserImpl.hpp 830538 2009-10-28 13:41:11Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMBUILDERIMPL_HPP)
#define XERCESC_INCLUDE_GUARD_DOMBUILDERIMPL_HPP


#include <xercesc/parsers/AbstractDOMParser.hpp>
#include <xercesc/dom/DOMLSParser.hpp>
#include <xercesc/dom/DOMLSInput.hpp>
#include <xercesc/dom/DOMConfiguration.hpp>
#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/util/RefVectorOf.hpp>
#include <xercesc/util/ValueHashTableOf.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XMLEntityResolver;
class XMLResourceIdentifier;
class DOMStringListImpl;
class DOMLSResourceResolver;

 /**
  * Introduced in DOM Level 3
  *
  * DOMLSParserImpl provides an implementation of a DOMLSParser interface.
  * A DOMLSParser instance is obtained from the DOMImplementationLS interface
  * by invoking its createDOMLSParser method.
  */
class PARSERS_EXPORT DOMLSParserImpl : public AbstractDOMParser,
                                       public DOMLSParser,
                                       public DOMConfiguration
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Detructor
    // -----------------------------------------------------------------------

    /** @name Constructors and Destructor */
    //@{
    /** Construct a DOMLSParserImpl, with an optional validator
      *
      * Constructor with an instance of validator class to use for
      * validation. If you don't provide a validator, a default one will
      * be created for you in the scanner.
      *
      * @param valToAdopt Pointer to the validator instance to use. The
      *                   parser is responsible for freeing the memory.
      * @param manager    The memory manager to be used for memory allocations
      * @param gramPool   Pointer to the grammar pool instance from
      *                   external application.
      *                   The parser does NOT own it.
      *
      */
    DOMLSParserImpl
    (
          XMLValidator* const   valToAdopt = 0
        , MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
        , XMLGrammarPool* const gramPool = 0
    );

    /**
      * Destructor
      */
    virtual ~DOMLSParserImpl();

    //@}

    // -----------------------------------------------------------------------
    //  Implementation of DOMLSParser interface
    // -----------------------------------------------------------------------
    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------

    /** @name Getter methods */
    //@{

    /**
      * @see DOMLSParser#getDomConfig
      */
    virtual DOMConfiguration* getDomConfig();

    /**
      * @see DOMLSParser#getFilter
      */
    virtual const DOMLSParserFilter* getFilter() const;

    /**
      * @see DOMLSParser#getAsync
      */
    virtual bool getAsync() const;

    /**
      * @see DOMLSParser#getBusy
      */
    virtual bool getBusy() const;
    //@}


    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------

    /** @name Setter methods */
    //@{
    /**
      * @see DOMLSParser#setFilter
      */
    virtual void setFilter(DOMLSParserFilter* const filter);

    //@}

    // -----------------------------------------------------------------------
    //  Parsing methods
    // -----------------------------------------------------------------------
    /** @name Parsing methods */
    //@{

    // -----------------------------------------------------------------------
    //  Parsing methods
    // -----------------------------------------------------------------------
    /**
      * @see DOMLSParser#parse
      */
    virtual DOMDocument* parse(const DOMLSInput* source);

    /**
      * @see DOMLSParser#parseURI
      */
    virtual DOMDocument* parseURI(const XMLCh* const uri);

    /**
      * @see DOMLSParser#parseURI
      */
    virtual DOMDocument* parseURI(const char* const uri);

    /**
      * @see DOMLSParser#parseWithContext
      */
    virtual DOMNode* parseWithContext
    (
        const   DOMLSInput*     source
        ,       DOMNode*        contextNode
        , const ActionType      action
    );

    /**
      * @see DOMLSParser#abort
      */
    virtual void abort();


    // -----------------------------------------------------------------------
    //  Non-standard Extension
    // -----------------------------------------------------------------------
    /** @name Non-standard Extension */
    //@{

    /**
     * Called to indicate that this DOMLSParser is no longer in use
     * and that the implementation may relinquish any resources associated with it.
     *
     */
    virtual void              release();

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
    virtual void resetDocumentPool();

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
                             const bool toCache = false);

    /**
      * Preparse schema grammar (XML Schema, DTD, etc.) via a file path or URL
      *
      * This method invokes the preparsing process on a schema grammar XML
      * file specified by the file path parameter. If the 'toCache' flag
      * is enabled, the parser will cache the grammars for re-use. If a grammar
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
                             const bool toCache = false);

    /**
      * Preparse schema grammar (XML Schema, DTD, etc.) via a file path or URL
      *
      * This method invokes the preparsing process on a schema grammar XML
      * file specified by the file path parameter. If the 'toCache' flag
      * is enabled, the parser will cache the grammars for re-use. If a grammar
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
     * Retrieve the grammar that is associated with the specified namespace key
     *
     * @param  nameSpaceKey Namespace key
     * @return Grammar associated with the Namespace key.
     */
    virtual Grammar* getGrammar(const XMLCh* const nameSpaceKey) const;

    /**
     * Retrieve the grammar where the root element is declared.
     *
     * @return Grammar where root element declared
     */
    virtual Grammar* getRootGrammar() const;

    /**
     * Returns the string corresponding to a URI id from the URI string pool.
     *
     * @param uriId id of the string in the URI string pool.
     * @return URI string corresponding to the URI id.
     */
    virtual const XMLCh* getURIText(unsigned int uriId) const;

    /**
      * Clear the cached grammar pool
      */
    virtual void resetCachedGrammarPool();

    /**
      * Returns the current src offset within the input source.
      * To be used only while parsing is in progress.
      *
      * @return offset within the input source
      */
    virtual XMLFilePos getSrcOffset() const;

    //@}

    // -----------------------------------------------------------------------
    //  Implementation of the DOMConfiguration interface.
    // -----------------------------------------------------------------------
    /** @name Implementation of the DOMConfiguration interface. */
    //@{
    /**
     * Set the value of a parameter.
     *
     * @param name The name of the parameter to set.
     * @param value The new value or null if the user wishes to unset the
     * parameter. While the type of the value parameter is defined as
     * <code>DOMUserData</code>, the object type must match the type defined
     * by the definition of the parameter. For example, if the parameter is
     * "error-handler", the value must be of type <code>DOMErrorHandler</code>
     *
     * @exception DOMException (NOT_SUPPORTED_ERR) Raised when the
     * parameter name is recognized but the requested value cannot be set.
     * @exception DOMException (NOT_FOUND_ERR) Raised when the
     * parameter name is not recognized.
     *
     * @since DOM level 3
     **/
    virtual void setParameter(const XMLCh* name, const void* value);
    virtual void setParameter(const XMLCh* name, bool value);

    /**
     * Return the value of a parameter if known.
     *
     * @param name The name of the parameter.
     * @return The current object associated with the specified parameter or
     * null if no object has been associated or if the parameter is not
     * supported.
     *
     * @exception DOMException (NOT_FOUND_ERR) Raised when the i
     * boolean parameter
     * name is not recognized.
     *
     * @since DOM level 3
     **/
    virtual const void* getParameter(const XMLCh* name) const;

    /**
     * Check if setting a parameter to a specific value is supported.
     *
     * @param name The name of the parameter to check.
     * @param value An object. if null, the returned value is true.
     * @return true if the parameter could be successfully set to the specified
     * value, or false if the parameter is not recognized or the requested value
     * is not supported. This does not change the current value of the parameter
     * itself.
     *
     * @since DOM level 3
     **/
    virtual bool canSetParameter(const XMLCh* name, const void* value) const;
    virtual bool canSetParameter(const XMLCh* name, bool value) const;

    /**
     * The list of the parameters supported by this DOMConfiguration object and
     * for which at least one value can be set by the application.
     * Note that this list can also contain parameter names defined outside this specification.
     *
     * @return The list of parameters that can be used with setParameter/getParameter
     * @since DOM level 3
     **/
    virtual const DOMStringList* getParameterNames() const;
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
      * @see DOMErrorHandler
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
      * <b><font color="#FF0000">This method is a no-op for this DOM
      * implementation.</font></b>
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
      * <b><font color="#FF0000">This method is a no-op for this DOM
      * implementation.</font></b>
      *
      * @param inputSource A const reference to the InputSource object
      *                    which points to the XML file being parsed.
      * @see InputSource
      */
    virtual void endInputSource(const InputSource& inputSource);

    /** Expand a system id
      *
      * This method allows an installed XMLEntityHandler to further
      * process any system id's of enternal entities encountered in
      * the XML file being parsed, such as redirection etc.
      *
      * <b><font color="#FF0000">This method always returns 'false'
      * for this DOM implementation.</font></b>
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
      * <b><font color="#FF0000">This method is a no-op for this DOM
      * implementation.</font></b>
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
      * <b><font color="#FF0000">This method is a no-op for this DOM parse
      * implementation.</font></b>
      *
      * @param inputSource A const reference to the InputSource object
      *                    which points to the external entity
      *                    being parsed.
      */
    virtual void startInputSource(const InputSource& inputSource);

    //@}

    // -----------------------------------------------------------------------
    //  Implementation of the XMLDocumentHandler interface.
    // -----------------------------------------------------------------------
    virtual void docCharacters
    (
        const   XMLCh* const    chars
        , const XMLSize_t       length
        , const bool            cdataSection
    );
    virtual void docComment
    (
        const   XMLCh* const    comment
    );
    virtual void docPI
    (
        const   XMLCh* const    target
        , const XMLCh* const    data
    );
    virtual void startEntityReference
    (
        const   XMLEntityDecl&  entDecl
    );
    virtual void endElement
    (
        const   XMLElementDecl& elemDecl
        , const unsigned int    urlId
        , const bool            isRoot
        , const XMLCh* const    elemPrefix
    );
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

    // overriden callbacks to implement parseWithContext behavior
    virtual void startDocument();
    virtual void XMLDecl
    (
        const   XMLCh* const    versionStr
        , const XMLCh* const    encodingStr
        , const XMLCh* const    standaloneStr
        , const XMLCh* const    actualEncStr
    );


private :
    // -----------------------------------------------------------------------
    //  Initialize/Cleanup methods
    // -----------------------------------------------------------------------
    void resetParse();

    // -----------------------------------------------------------------------
    //  Helper methods
    // -----------------------------------------------------------------------
    void applyFilter(DOMNode* node);

    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fEntityResolver
    //      The installed DOM entity resolver, if any. Null if none.
    //
    //  fXMLEntityResolver
    //      The installed Xerces entity resolver, if any. Null if none.
    //
    //  fErrorHandler
    //      The installed DOM error handler, if any. Null if none.
    //
    //  fFilter
    //      The installed application filter, if any. Null if none.
    //
    //  fCharsetOverridesXMLEncoding
    //      Indicates if the "charset-overrides-xml-encoding" is set or not
    //
    //  fUserAdoptsDocument
    //      The DOMDocument ownership has been transferred to application
    //      If set to true, the parser does not own the document anymore
    //      and thus will not release its memory.
    //
    //  fSupportedParameters
    //      A list of the parameters that can be set, including the ones
    //      specific of Xerces
	//
    //  fFilterAction
    //      A map of elements rejected by the DOMLSParserFilter::startElement
    //      callback, used to avoid invoking DOMLSParserFilter::acceptNode
    //      on its children
	//
    //  fFilterDelayedTextNodes
    //      As text nodes are filled incrementally, store them in a map
    //      so that we ask DOMLSParserFilter::acceptNode only once, when it
    //      is completely created
	//
    //  fWrapNodesInDocumentFragment
    //  fWrapNodesContext
    //  fWrapNodesAction
    //      Variables used to keep the state for parseWithContext API 
    //
    //-----------------------------------------------------------------------
    DOMLSResourceResolver*      fEntityResolver;
    XMLEntityResolver*          fXMLEntityResolver;
    DOMErrorHandler*            fErrorHandler;
    DOMLSParserFilter*          fFilter;
    bool                        fCharsetOverridesXMLEncoding;
    bool                        fUserAdoptsDocument;
    DOMStringListImpl*          fSupportedParameters;
    ValueHashTableOf<DOMLSParserFilter::FilterAction, PtrHasher>*   fFilterAction;
    ValueHashTableOf<bool, PtrHasher>*                              fFilterDelayedTextNodes;
    DOMDocumentFragment*        fWrapNodesInDocumentFragment;
    DOMNode*                    fWrapNodesContext;
    ActionType                  fWrapNodesAction;

    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    DOMLSParserImpl(const DOMLSParserImpl &);
    DOMLSParserImpl & operator = (const DOMLSParserImpl &);
};



// ---------------------------------------------------------------------------
//  DOMLSParserImpl: Handlers for the XMLEntityHandler interface
// ---------------------------------------------------------------------------
inline void DOMLSParserImpl::endInputSource(const InputSource&)
{
    // The DOM entity resolver doesn't handle this
}

inline bool DOMLSParserImpl::expandSystemId(const XMLCh* const, XMLBuffer&)
{
    // The DOM entity resolver doesn't handle this
    return false;
}

inline void DOMLSParserImpl::resetEntities()
{
    // Nothing to do on this one
}

inline void DOMLSParserImpl::startInputSource(const InputSource&)
{
    // The DOM entity resolver doesn't handle this
}


// ---------------------------------------------------------------------------
//  DOMLSParserImpl: Getter methods
// ---------------------------------------------------------------------------
inline DOMConfiguration* DOMLSParserImpl::getDomConfig()
{
    return this;
}

inline bool DOMLSParserImpl::getAsync() const
{
    // We are a synchronous parser
    return false;
}

inline const DOMLSParserFilter* DOMLSParserImpl::getFilter() const
{
    return fFilter;
}


XERCES_CPP_NAMESPACE_END

#endif
