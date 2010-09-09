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
 * $Id: SAXParser.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_SAXPARSER_HPP)
#define XERCESC_INCLUDE_GUARD_SAXPARSER_HPP

#include <xercesc/sax/Parser.hpp>
#include <xercesc/internal/VecAttrListImpl.hpp>
#include <xercesc/framework/XMLDocumentHandler.hpp>
#include <xercesc/framework/XMLElementDecl.hpp>
#include <xercesc/framework/XMLEntityHandler.hpp>
#include <xercesc/framework/XMLErrorReporter.hpp>
#include <xercesc/framework/XMLBuffer.hpp>
#include <xercesc/util/SecurityManager.hpp>
#include <xercesc/validators/common/Grammar.hpp>
#include <xercesc/validators/DTD/DocTypeHandler.hpp>


XERCES_CPP_NAMESPACE_BEGIN


class DocumentHandler;
class EntityResolver;
class XMLPScanToken;
class XMLScanner;
class XMLValidator;
class GrammarResolver;
class XMLGrammarPool;
class XMLEntityResolver;
class XMLResourceIdentifier;
class PSVIHandler;

/**
  * This class implements the SAX 'Parser' interface and should be
  * used by applications wishing to parse the XML files using SAX.
  * It allows the client program to install SAX handlers for event
  * callbacks.
  *
  * <p>It can be used to instantiate a validating or non-validating
  * parser, by setting a member flag.</p>
  *
  * @deprecated This interface has been replaced by the SAX2
  *             interface, which includes Namespace support.
  *             See SAX2XMLReader for more information.
  *
  * Note - XMLDocumentHandler calls, when used with SAXParser, will not provide correct namespace information. This is becaue the SAX parser does not support namespace aware processing.
  *
  *
  */

class PARSERS_EXPORT SAXParser :

    public XMemory
    , public Parser
    , public XMLDocumentHandler
    , public XMLErrorReporter
    , public XMLEntityHandler
    , public DocTypeHandler
{
public :
    // -----------------------------------------------------------------------
    //  Class types
    // -----------------------------------------------------------------------
    /** ValScheme enum used in setValidationScheme
      *    Val_Never:  Do not report validation errors.
      *    Val_Always: The parser will always report validation errors.
      *    Val_Auto:   The parser will report validation errors only if a grammar is specified.
      *
      * @see #setValidationScheme
      */

    enum ValSchemes
    {
        Val_Never
        , Val_Always
        , Val_Auto
    };


    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    /** @name Constructors and Destructor */
    //@{
    /** Constructor with an instance of validator class to use for
      * validation.
      * @param valToAdopt Pointer to the validator instance to use. The
      *                   parser is responsible for freeing the memory.
      * @param manager    Pointer to the memory manager to be used to
      *                   allocate objects.
      * @param gramPool   The collection of cached grammars.
      */
    SAXParser
    (
          XMLValidator*   const valToAdopt = 0
        , MemoryManager*  const manager = XMLPlatformUtils::fgMemoryManager
        , XMLGrammarPool* const gramPool = 0
    );

    /**
      * Destructor
      */
    ~SAXParser();
    //@}


    // -----------------------------------------------------------------------
    //  Getter Methods
    // -----------------------------------------------------------------------
    /** @name Getter methods */
    //@{
    /**
      * This method returns the installed document handler. Suitable
      * for 'lvalue' usages.
      *
      * @return The pointer to the installed document handler object.
      */
    DocumentHandler* getDocumentHandler();

    /**
      * This method returns the installed document handler. Suitable
      * only for 'rvalue' usages.
      *
      * @return A const pointer to the installed document handler object.
      */
    const DocumentHandler* getDocumentHandler() const;

    /**
      * This method returns the installed entity resolver. Suitable
      * for 'lvalue' usages.
      *
      * @return The pointer to the installed entity resolver object.
      */
    EntityResolver* getEntityResolver();

    /**
      * This method returns the installed entity resolver. Suitable
      * for 'rvalue' usages.
      *
      * @return A const pointer to the installed entity resolver object.
      */
    const EntityResolver* getEntityResolver() const;

    /**
      * This method returns the installed entity resolver. Suitable
      * for 'lvalue' usages.
      *
      * @return The pointer to the installed entity resolver object.
      */
    XMLEntityResolver* getXMLEntityResolver();

    /**
      * This method returns the installed entity resolver. Suitable
      * for 'rvalue' usages.
      *
      * @return A const pointer to the installed entity resolver object.
      */
    const XMLEntityResolver* getXMLEntityResolver() const;

    /**
      * This method returns the installed error handler. Suitable
      * for 'lvalue' usages.
      *
      * @return The pointer to the installed error handler object.
      */
    ErrorHandler* getErrorHandler();

    /**
      * This method returns the installed error handler. Suitable
      * for 'rvalue' usages.
      *
      * @return A const pointer to the installed error handler object.
      */
    const ErrorHandler* getErrorHandler() const;

    /**
      * This method returns the installed PSVI handler. Suitable
      * for 'lvalue' usages.
      *
      * @return The pointer to the installed PSVI handler object.
      */
    PSVIHandler* getPSVIHandler();

    /**
      * This method returns the installed PSVI handler. Suitable
      * for 'rvalue' usages.
      *
      * @return A const pointer to the installed PSVI handler object.
      */
    const PSVIHandler* getPSVIHandler() const;

    /**
      * This method returns a reference to the parser's installed
      * validator.
      *
      * @return A const reference to the installed validator object.
      */
    const XMLValidator& getValidator() const;

    /**
      * This method returns an enumerated value that indicates the current
      * validation scheme set on this parser.
      *
      * @return The ValSchemes value current set on this parser.
      * @see #setValidationScheme
      */
    ValSchemes getValidationScheme() const;

    /** Get the 'do schema' flag
      *
      * This method returns the state of the parser's schema processing
      * flag.
      *
      * @return true, if the parser is currently configured to
      *         understand schema, false otherwise.
      *
      * @see #setDoSchema
      */
    bool getDoSchema() const;

    /** Get the 'full schema constraint checking' flag
      *
      * This method returns the state of the parser's full schema constraint
      * checking flag.
      *
      * @return true, if the parser is currently configured to
      *         have full schema constraint checking, false otherwise.
      *
      * @see #setValidationSchemaFullChecking
      */
    bool getValidationSchemaFullChecking() const;

    /** Get the 'identity constraint checking' flag
      *
      * This method returns the state of the parser's identity constraint
      * checking flag.
      *
      * @return true, if the parser is currently configured to
      *         have identity constraint checking, false otherwise.
      *
      * @see #setIdentityConstraintChecking
      */
    bool getIdentityConstraintChecking() const;

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
    int getErrorCount() const;

    /**
      * This method returns the state of the parser's namespace
      * handling capability.
      *
      * @return true, if the parser is currently configured to
      *         understand namespaces, false otherwise.
      *
      * @see #setDoNamespaces
      */
    bool getDoNamespaces() const;

    /**
      * This method returns the state of the parser's
      * exit-on-First-Fatal-Error flag.
      *
      * @return true, if the parser is currently configured to
      *         exit on the first fatal error, false otherwise.
      *
      * @see #setExitOnFirstFatalError
      */
    bool getExitOnFirstFatalError() const;

    /**
      * This method returns the state of the parser's
      * validation-constraint-fatal flag.
      *
      * @return true, if the parser is currently configured to
      *         set validation constraint errors as fatal, false
      *         otherwise.
      *
      * @see #setValidationConstraintFatal
      */
    bool getValidationConstraintFatal() const;

   /** Get the set of Namespace/SchemaLocation that is specified externally.
      *
      * This method returns the list of Namespace/SchemaLocation that was
      * specified using setExternalSchemaLocation.
      *
      * The parser owns the returned string, and the memory allocated for
      * the returned string will be destroyed when the parser is deleted.
      *
      * To ensure accessibility of the returned information after the parser
      * is deleted, callers need to copy and store the returned information
      * somewhere else.
      *
      * @return a pointer to the list of Namespace/SchemaLocation that was
      *         specified externally.  The pointer spans the same life-time as
      *         the parser.  A null pointer is returned if nothing
      *         was specified externally.
      *
      * @see #setExternalSchemaLocation(const XMLCh* const)
      */
    XMLCh* getExternalSchemaLocation() const;

   /** Get the noNamespace SchemaLocation that is specified externally.
      *
      * This method returns the no target namespace XML Schema Location
      * that was specified using setExternalNoNamespaceSchemaLocation.
      *
      * The parser owns the returned string, and the memory allocated for
      * the returned string will be destroyed when the parser is deleted.
      *
      * To ensure accessibility of the returned information after the parser
      * is deleted, callers need to copy and store the returned information
      * somewhere else.
      *
      * @return a pointer to the no target namespace Schema Location that was
      *         specified externally.  The pointer spans the same life-time as
      *         the parser.  A null pointer is returned if nothing
      *         was specified externally.
      *
      * @see #setExternalNoNamespaceSchemaLocation(const XMLCh* const)
      */
    XMLCh* getExternalNoNamespaceSchemaLocation() const;

   /** Get the SecurityManager instance attached to this parser.
      *
      * This method returns the security manager
      * that was specified using setSecurityManager.
      *
      * The SecurityManager instance must have been specified by the application;
      * this should not be deleted until after the parser has been deleted (or
      * a new SecurityManager instance has been supplied to the parser).
      *
      * @return a pointer to the SecurityManager instance
      *         specified externally.  A null pointer is returned if nothing
      *         was specified externally.
      *
      * @see #setSecurityManager(SecurityManager* const)
      */
    SecurityManager* getSecurityManager() const;

    /** Get the raw buffer low water mark for this parser.
      *
      * If the number of available bytes in the raw buffer is less than
      * the low water mark the parser will attempt to read more data before
      * continuing parsing. By default the value for this parameter is 100
      * bytes. You may want to set this parameter to 0 if you would like
      * the parser to parse the available data immediately without
      * potentially blocking while waiting for more date.
      *
      * @return current low water mark
      *
      * @see #setSecurityManager
      */
    XMLSize_t getLowWaterMark() const;

    /** Get the 'Loading External DTD' flag
      *
      * This method returns the state of the parser's loading external DTD
      * flag.
      *
      * @return false, if the parser is currently configured to
      *         ignore external DTD completely, true otherwise.
      *
      * @see #setLoadExternalDTD
      * @see #getValidationScheme
      */
    bool getLoadExternalDTD() const;

    /** Get the 'Loading Schema' flag
      *
      * This method returns the state of the parser's loading schema
      * flag.
      *
      * @return true, if the parser is currently configured to
      *         automatically load schemas that are not in the
      *         grammar pool, false otherwise.
      *
      * @see #setLoadSchema
      */
    bool getLoadSchema() const;

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
      * Get the 'calculate src offset flag'
      *
      * This method returns the state of the parser's src offset calculation
      * when parsing an XML document.
      *
      * @return true, if the parser is currently configured to
      *         calculate src offsets, false otherwise.
      *
      * @see #setCalculateSrcOfs
      */
    bool getCalculateSrcOfs() const;

    /**
      * Get the 'force standard uri flag'
      *
      * This method returns the state if the parser forces standard uri
      *
      * @return true, if the parser is currently configured to
      *         force standard uri, i.e. malformed uri will be rejected.
      *
      * @see #setStandardUriConformant
      */
    bool getStandardUriConformant() const;

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

    /** Get the 'generate synthetic annotations' flag
      *
      * @return true, if the parser is currently configured to
      *         generate synthetic annotations, false otherwise.
      *         A synthetic XSAnnotation is created when a schema
      *         component has non-schema attributes but has no
      *         child annotations so that the non-schema attributes
      *         can be recovered under PSVI.
      *
      * @see #setGenerateSyntheticAnnotations
      */
    bool getGenerateSyntheticAnnotations() const;

    /** Get the 'validate annotations' flag
      *
      * @return true, if the parser is currently configured to
      *         validate annotations, false otherwise.
      *
      * @see #setValidateAnnotations
      */
    bool getValidateAnnotations() const;

    /** Get the 'ignore cached DTD grammar' flag
      *
      * @return true, if the parser is currently configured to
      *         ignore cached DTD, false otherwise.
      *
      * @see #setIgnoreCachedDTD
      */
    bool getIgnoreCachedDTD() const;

    /** Get the 'ignore annotations' flag
      *
      * @return true, if the parser is currently configured to
      *         ignore annotations, false otherwise.
      *
      * @see #setIgnoreAnnotations
      */
    bool getIgnoreAnnotations() const;

    /** Get the 'disable default entity resolution' flag
      *
      * @return true, if the parser is currently configured to
      *         not perform default entity resolution, false otherwise.
      *
      * @see #setDisableDefaultEntityResolution
      */
    bool getDisableDefaultEntityResolution() const;

    /** Get the 'skip DTD validation' flag
      *
      * @return true, if the parser is currently configured to
      *         skip DTD validation, false otherwise.
      *
      * @see #setSkipDTDValidation
      */
    bool getSkipDTDValidation() const;

    /** Get the 'handle multiple schema imports' flag
      *
      * @return true, if the parser is currently configured to
      *         import multiple schemas with the same namespace, false otherwise.
      *
      * @see #setHandleMultipleImports
      */
    bool getHandleMultipleImports() const;
    //@}


    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------

    /** @name Setter methods */
    //@{
    /** set the 'generate synthetic annotations' flag
      *
      * @param newValue The value for specifying whether Synthetic Annotations
      *        should be generated or not.
      *        A synthetic XSAnnotation is created when a schema
      *        component has non-schema attributes but has no
      *        child annotations.
      *
      * @see #getGenerateSyntheticAnnotations
      */
    void setGenerateSyntheticAnnotations(const bool newValue);

    /** set the 'validate annotations' flag
      *
      * @param newValue The value for specifying whether annotations
      *        should be validate or not.
      *
      * @see #getValidateAnnotations
      */
    void setValidateAnnotations(const bool newValue);

    /**
      * This method allows users to enable or disable the parser's
      * namespace processing. When set to true, parser starts enforcing
      * all the constraints / rules specified by the NameSpace
      * specification.
      *
      * <p>The parser's default state is: false.</p>
      *
      * @param newState The value specifying whether NameSpace rules should
      *                 be enforced or not.
      *
      * @see #getDoNamespaces
      */
    void setDoNamespaces(const bool newState);

    /**
      * This method allows users to set the validation scheme to be used
      * by this parser. The value is one of the ValSchemes enumerated values
      * defined by this class:
      *
      * <br>  Val_Never  - turn off validation
      * <br>  Val_Always - turn on validation
      * <br>  Val_Auto   - turn on validation if any internal/external
      *                  DTD subset have been seen
      *
      * <p>The parser's default state is: Val_Never.</p>
      *
      * @param newScheme The new validation scheme to use.
      *
      * @see #getValidationScheme
      */
    void setValidationScheme(const ValSchemes newScheme);

    /** Set the 'schema support' flag
      *
      * This method allows users to enable or disable the parser's
      * schema processing. When set to false, parser will not process
      * any schema found.
      *
      * The parser's default state is: false.
      *
      * Note: If set to true, namespace processing must also be turned on.
      *
      * @param newState The value specifying whether schema support should
      *                 be enforced or not.
      *
      * @see #getDoSchema
      */
    void setDoSchema(const bool newState);

    /**
      * This method allows the user to turn full Schema constraint checking on/off.
      * Only takes effect if Schema validation is enabled.
      * If turned off, partial constraint checking is done.
      *
      * Full schema constraint checking includes those checking that may
      * be time-consuming or memory intensive. Currently, particle unique
      * attribution constraint checking and particle derivation restriction checking
      * are controlled by this option.
      *
      * The parser's default state is: false.
      *
      * @param schemaFullChecking True to turn on full schema constraint checking.
      *
      * @see #getValidationSchemaFullChecking
      */
    void setValidationSchemaFullChecking(const bool schemaFullChecking);

    /**
      * This method allows the user to turn identity constraint checking on/off.
      * Only takes effect if Schema validation is enabled.
      * If turned off, identity constraint checking is not done.
      *
      * The parser's default state is: true.
      *
      * @param identityConstraintChecking True to turn on identity constraint checking.
      *
      * @see #getIdentityConstraintChecking
      */
    void setIdentityConstraintChecking(const bool identityConstraintChecking);

    /**
      * This method allows users to set the parser's behaviour when it
      * encounters the first fatal error. If set to true, the parser
      * will exit at the first fatal error. If false, then it will
      * report the error and continue processing.
      *
      * <p>The default value is 'true' and the parser exits on the
      * first fatal error.</p>
      *
      * @param newState The value specifying whether the parser should
      *                 continue or exit when it encounters the first
      *                 fatal error.
      *
      * @see #getExitOnFirstFatalError
      */
    void setExitOnFirstFatalError(const bool newState);

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
      * @param newState If true, the parser will exit if "setExitOnFirstFatalError"
      *                 is set to true.
      *
      * @see #getValidationConstraintFatal
      * @see #setExitOnFirstFatalError
      */
    void setValidationConstraintFatal(const bool newState);

    /**
      * This method allows the user to specify a list of schemas to use.
      * If the targetNamespace of a schema specified using this method matches
      * the targetNamespace of a schema occurring in the instance document in
      * the schemaLocation attribute, or if the targetNamespace matches the
      * namespace attribute of the "import" element, the schema specified by the
      * user using this method will be used (i.e., the schemaLocation attribute
      * in the instance document or on the "import" element will be effectively ignored).
      *
      * If this method is called more than once, only the last one takes effect.
      *
      * The syntax is the same as for schemaLocation attributes in instance
      * documents: e.g, "http://www.example.com file_name.xsd". The user can
      * specify more than one XML Schema in the list.
      *
      * @param schemaLocation the list of schemas to use
      *
      * @see #getExternalSchemaLocation
      */

    void setExternalSchemaLocation(const XMLCh* const schemaLocation);

    /**
      * This method is same as setExternalSchemaLocation(const XMLCh* const).
      * It takes native char string as parameter
      *
      * @param schemaLocation the list of schemas to use
      *
      * @see #setExternalSchemaLocation(const XMLCh* const)
      */
    void setExternalSchemaLocation(const char* const schemaLocation);

    /**
      * This method allows the user to specify the no target namespace XML
      * Schema Location externally.  If specified, the instance document's
      * noNamespaceSchemaLocation attribute will be effectively ignored.
      *
      * If this method is called more than once, only the last one takes effect.
      *
      * The syntax is the same as for the noNamespaceSchemaLocation attribute
      * that may occur in an instance document: e.g."file_name.xsd".
      *
      * @param noNamespaceSchemaLocation the XML Schema Location with no target namespace
      *
      * @see #getExternalNoNamespaceSchemaLocation
      */
    void setExternalNoNamespaceSchemaLocation(const XMLCh* const noNamespaceSchemaLocation);

    /**
      * This method is same as setExternalNoNamespaceSchemaLocation(const XMLCh* const).
      * It takes native char string as parameter
      *
      * @param noNamespaceSchemaLocation the XML Schema Location with no target namespace
      *
      * @see #setExternalNoNamespaceSchemaLocation(const XMLCh* const)
      */
    void setExternalNoNamespaceSchemaLocation(const char* const noNamespaceSchemaLocation);

    /**
      * This allows an application to set a SecurityManager on
      * the parser; this object stores information that various
      * components use to limit their consumption of system
      * resources while processing documents.
      *
      * If this method is called more than once, only the last one takes effect.
      * It may not be reset during a parse.
      *
      *
      * @param securityManager  the SecurityManager instance to
      * be used by this parser
      *
      * @see #getSecurityManager
      */
    void setSecurityManager(SecurityManager* const securityManager);

    /** Set the raw buffer low water mark for this parser.
      *
      * If the number of available bytes in the raw buffer is less than
      * the low water mark the parser will attempt to read more data before
      * continuing parsing. By default the value for this parameter is 100
      * bytes. You may want to set this parameter to 0 if you would like
      * the parser to parse the available data immediately without
      * potentially blocking while waiting for more date.
      *
      * @param lwm new low water mark
      *
      * @see #getSecurityManager
      */
    void setLowWaterMark(XMLSize_t lwm);

    /** Set the 'Loading External DTD' flag
      *
      * This method allows users to enable or disable the loading of external DTD.
      * When set to false, the parser will ignore any external DTD completely
      * if the validationScheme is set to Val_Never.
      *
      * The parser's default state is: true.
      *
      * This flag is ignored if the validationScheme is set to Val_Always or Val_Auto.
      *
      * @param newState The value specifying whether external DTD should
      *                 be loaded or not.
      *
      * @see #getLoadExternalDTD
      * @see #setValidationScheme
      */
    void setLoadExternalDTD(const bool newState);

    /** Set the 'Loading Schema' flag
      *
      * This method allows users to enable or disable the loading of schemas.
      * When set to false, the parser not attempt to load schemas beyond
      * querying the grammar pool for them.
      *
      * The parser's default state is: true.
      *
      * @param newState The value specifying whether schemas should
      *                 be loaded if they're not found in the grammar
      *                 pool.
      *
      * @see #getLoadSchema
      * @see #setDoSchema
      */
    void setLoadSchema(const bool newState);

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
      * If the 'Grammar caching' flag is set to true, this method ignores the
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

    /** Enable/disable src offset calculation
      *
      * This method allows users to enable/disable src offset calculation.
      * Disabling the calculation will improve performance.
      *
      * The parser's default state is: false.
      *
      * @param newState The value specifying whether we should enable or
      *                 disable src offset calculation
      *
      * @see #getCalculateSrcOfs
      */
    void setCalculateSrcOfs(const bool newState);

    /** Force standard uri
      *
      * This method allows users to tell the parser to force standard uri conformance.
      *
      * The parser's default state is: false.
      *
      * @param newState The value specifying whether the parser should reject malformed URI.
      *
      * @see #getStandardUriConformant
      */
    void setStandardUriConformant(const bool newState);

    /** Set the scanner to use when scanning the XML document
      *
      * This method allows users to set the scanner to use
      * when scanning a given XML document.
      *
      * @param scannerName The name of the desired scanner
      */
    void useScanner(const XMLCh* const scannerName);

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

    /** Set the 'ignore annotation' flag
      *
      * This method gives users the option to not generate XSAnnotations
      * when "traversing" a schema.
      *
      * The parser's default state is false
      *
      * @param newValue The state to set
      */
    void setIgnoreAnnotations(const bool newValue);

    /** Set the 'disable default entity resolution' flag
      *
      * This method gives users the option to not perform default entity
      * resolution.  If the user's resolveEntity method returns NULL the
      * parser will try to resolve the entity on its own.  When this option
      * is set to true, the parser will not attempt to resolve the entity
      * when the resolveEntity method returns NULL.
      *
      * The parser's default state is false
      *
      * @param newValue The state to set
      *
      * @see #EntityResolver
      */
    void setDisableDefaultEntityResolution(const bool newValue);

    /** Set the 'skip DTD validation' flag
      *
      * This method gives users the option to skip DTD validation only when
      * schema validation is on (i.e. when performing validation,  we will
      * ignore the DTD, except for entities, when schema validation is enabled).
      *
      * NOTE: This option is ignored if schema validation is disabled.
      *
      * The parser's default state is false
      *
      * @param newValue The state to set
      */
    void setSkipDTDValidation(const bool newValue);

    /** Set the 'handle multiple schema imports' flag
      *
      * This method gives users the ability to import multiple schemas that
      * have the same namespace.
      *
      * NOTE: This option is ignored if schema validation is disabled.
      *
      * The parser's default state is false
      *
      * @param newValue The state to set
      */
    void setHandleMultipleImports(const bool newValue);
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
      * Note - XMLDocumentHandler calls, when used with SAXParser, will not provide correct namespace information. This is becaue the SAX parser does not support namespace aware processing.
      *
      * @param toInstall A pointer to the users advanced callback handler.
      *
      * @see #removeAdvDocHandler
      */
    void installAdvDocHandler(XMLDocumentHandler* const toInstall);

    /**
      * This method removes the 'advanced' document handler callback from
      * the underlying parser scanner. If no handler is installed, advanced
      * callbacks are not invoked by the scanner.
      * @param toRemove A pointer to the advanced callback handler which
      *                 should be removed.
      *
      * Note - XMLDocumentHandler calls, when used with SAXParser, will not provide correct namespace information. This is becaue the SAX parser does not support namespace aware processing.
      *
      * @see #installAdvDocHandler
      */
    bool removeAdvDocHandler(XMLDocumentHandler* const toRemove);
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
    bool parseFirst
    (
        const   XMLCh* const    systemId
        ,       XMLPScanToken&  toFill
    );

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
    bool parseFirst
    (
        const   char* const     systemId
        ,       XMLPScanToken&  toFill
    );

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
    bool parseFirst
    (
        const   InputSource&    source
        ,       XMLPScanToken&  toFill
    );

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
    bool parseNext(XMLPScanToken& token);

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
    void parseReset(XMLPScanToken& token);

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


    // -----------------------------------------------------------------------
    //  Implementation of the SAX Parser interface
    // -----------------------------------------------------------------------

    /** @name Implementation of SAX 1.0 Parser interface's. */
    //@{
    /**
      * This method invokes the parsing process on the XML file specified
      * by the InputSource parameter.
      *
      * @param source A const reference to the InputSource object which
      *               points to the XML file to be parsed.
      *
      * @see Parser#parse(InputSource)
      */
    virtual void parse(const InputSource& source);

    /**
      * This method invokes the parsing process on the XML file specified by
      * the Unicode string parameter 'systemId'.
      *
      * @param systemId A const XMLCh pointer to the Unicode string which
      *                 contains the path to the XML file to be parsed.
      *
      * @see Parser#parse(XMLCh*)
      */
    virtual void parse(const XMLCh* const systemId);

    /**
      * This method invokes the parsing process on the XML file specified by
      * the native char* string parameter 'systemId'.
      *
      * @param systemId A const char pointer to a native string which
      *                 contains the path to the XML file to be parsed.
      */
    virtual void parse(const char* const systemId);

    /**
      * This method installs the user specified SAX Document Handler
      * callback function on parser.
      *
      * @param handler A pointer to the document handler to be called
      *                when the parser comes across 'document' events
      *                as per the SAX specification.
      *
      * @see Parser#parse(char*)
      */
    virtual void setDocumentHandler(DocumentHandler* const handler);

    /**
      * This method installs the user specified DTD handler on the parser.
      *
      * @param handler A pointer to the DTD handler to be called
      *                when the parser comes across 'DTD' events
      *                as per the SAX specification.
      *
      * @see Parser#setDTDHandler
      */
    virtual void setDTDHandler(DTDHandler* const handler);

    /**
      * This method installs the user specified error handler on
      * the parser.
      *
      * @param handler A pointer to the error handler to be called
      *                when the parser comes across 'error' events
      *                as per the SAX specification.
      *
      * @see Parser#setErrorHandler
      */
    virtual void setErrorHandler(ErrorHandler* const handler);

    /**
      * This method installs the user specified PSVI handler on
      * the parser.
      *
      * @param handler A pointer to the PSVI handler to be called
      *                when the parser comes across 'PSVI' events
      *                as per the schema specification.
      *
      * @see Parser#setPSVIHandler
      */
    virtual void setPSVIHandler(PSVIHandler* const handler);

    /**
      * This method installs the user specified entity resolver on the
      * parser. It allows applications to trap and redirect calls to
      * external entities.
      *
      * <i>Any previously set entity resolver is merely dropped, since the parser
      * does not own them.  If both setEntityResolver and setXMLEntityResolver
      * are called, then the last one is used.</i>
      *
      * @param resolver A pointer to the entity resolver to be called
      *                 when the parser comes across references to
      *                 entities in the XML file.
      *
      * @see Parser#setEntityResolver
      */
    virtual void setEntityResolver(EntityResolver* const resolver);

    /**
      * This method installs the user specified entity resolver on the
      * parser. It allows applications to trap and redirect calls to
      * external entities.
      *
      * <i>Any previously set entity resolver is merely dropped, since the parser
      * does not own them.  If both setEntityResolver and setXMLEntityResolver
      * are called, then the last one is used.</i>
      *
      * @param resolver A pointer to the entity resolver to be called
      *                 when the parser comes across references to
      *                 entities in the XML file.
      *
      * @see Parser#setXMLEntityResolver
      */
    virtual void setXMLEntityResolver(XMLEntityResolver* const resolver);

    //@}


    // -----------------------------------------------------------------------
    //  Implementation of the XMLDocumentHandler interface
    // -----------------------------------------------------------------------

    /** @name Implementation of the XMLDocumentHandler Interface. */
    //@{
    /**
      * This method is used to report all the characters scanned
      * by the parser. The driver will invoke the 'characters'
      * method of the user installed SAX Document Handler.
      *
      * <p>If any advanced callback handlers are installed, the
      * corresponding 'docCharacters' method will also be invoked.</p>
      *
      * @param chars   A const pointer to a Unicode string representing the
      *                character data.
      * @param length  The length of the Unicode string returned in 'chars'.
      * @param cdataSection  A flag indicating if the characters represent
      *                      content from the CDATA section.
      * @see DocumentHandler#characters
      */
    virtual void docCharacters
    (
        const   XMLCh* const    chars
        , const XMLSize_t       length
        , const bool            cdataSection
    );

    /**
      * This method is used to report any comments scanned by the parser.
      * This method is a no-op unless, unless an advanced callback handler
      * is installed, in which case the corresponding 'docComment' method
      * is invoked.
      *
      * @param comment A const pointer to a null terminated Unicode
      *                string representing the comment text.
      */
    virtual void docComment
    (
        const   XMLCh* const    comment
    );

    /**
      * This method is used to report any PI scanned by the parser.
      *
      * <p>Any PI's occurring before any 'content' are not reported
      * to any SAX handler as per the specification. However, all
      * PI's within content are reported via the SAX Document Handler's
      * 'processingInstruction' method.
      *
      * <p>If any advanced callback handlers are installed, the
      * corresponding 'docPI' method will be invoked.</p>
      *
      * @param target A const pointer to a Unicode string representing the
      *               target of the PI declaration.
      * @param data   A const pointer to a Unicode string representing the
      *               data of the PI declaration. See the PI production rule
      *               in the XML specification for details.
      *
      * @see DocumentHandler#processingInstruction
      */
    virtual void docPI
    (
        const   XMLCh* const    target
        , const XMLCh* const    data
    );

    /**
      * This method is used to indicate the end of root element
      * was just scanned by the parser. Corresponding 'endDocument'
      * method of the user installed SAX Document Handler will also
      * be invoked.
      *
      * <p>In addition, if any advanced callback handlers are installed,
      * the corresponding 'endDocument' method is invoked.</p>
      *
      * @see DocumentHandler#endDocument
      */
    virtual void endDocument();

    /**
      * This method is used to indicate the end tag of an element.
      * The driver will invoke the corresponding 'endElement' method of
      * the SAX Document Handler interface.
      *
      * <p>If any advanced callback handlers are installed, the
      * corresponding 'endElement' method is also invoked.</p>
      *
      * @param elemDecl A const reference to the object containing element
      *                 declaration information.
      * @param urlId    An id referring to the namespace prefix, if
      *                 namespaces setting is switched on.
      * @param isRoot   A flag indicating whether this element was the
      *                 root element.
      * @param elemPrefix A const pointer to a Unicode string containing
      *                   the namespace prefix for this element. Applicable
      *                   only when namespace processing is enabled.
      * @see DocumentHandler#endElement
      */
    virtual void endElement
    (
        const   XMLElementDecl& elemDecl
        , const unsigned int    urlId
        , const bool            isRoot
        , const XMLCh* const    elemPrefix
    );

    /**
      * This method is used to indicate that an end of an entity reference
      * was just scanned.
      *
      * <p>If any advanced callback handlers are installed, the
      * corresponding 'endEntityReference' method is invoked.</p>
      *
      * @param entDecl A const reference to the object containing the
      *                entity declaration information.
      */
    virtual void endEntityReference
    (
        const   XMLEntityDecl&  entDecl
    );

    /**
      * This method is used to report all the whitespace characters,
      * which are determined to be 'ignorable'. This distinction
      * between characters is only made, if validation is enabled.
      * Corresponding 'ignorableWhitespace' method of the user installed
      * SAX Document Handler interface is called.
      *
      * <p>Any whitespace before content is not reported to the SAX
      * Document Handler method, as per the SAX specification.
      * However, if any advanced callback handlers are installed, the
      * corresponding 'ignorableWhitespace' method is invoked.</p>
      *
      * @param chars   A const pointer to a Unicode string representing the
      *                ignorable whitespace character data.
      * @param length  The length of the Unicode string 'chars'.
      * @param cdataSection  A flag indicating if the characters represent
      *                      content from the CDATA section.
      * @see DocumentHandler#ignorableWhitespace
      */
    virtual void ignorableWhitespace
    (
        const   XMLCh* const    chars
        , const XMLSize_t       length
        , const bool            cdataSection
    );

    /**
      * This method allows the user installed Document Handler and
      * any advanced callback handlers to 'reset' themselves.
      */
    virtual void resetDocument();

    /**
      * This method is used to report the start of the parsing process.
      * The corresponding user installed SAX Document Handler's method
      * 'startDocument' is invoked.
      *
      * <p>If any advanced callback handlers are installed, then the
      * corresponding 'startDocument' method is also called.</p>
      *
      * @see DocumentHandler#startDocument
      */
    virtual void startDocument();

    /**
      * This method is used to report the start of an element. It is
      * called at the end of the element, by which time all attributes
      * specified are also parsed. The corresponding user installed
      * SAX Document Handler's method 'startElement' is invoked.
      *
      * <p>If any advanced callback handlers are installed, then the
      * corresponding 'startElement' method is also called.</p>
      *
      * @param elemDecl A const reference to the object containing element
      *                 declaration information.
      * @param urlId    An id referring to the namespace prefix, if
      *                 namespaces setting is switched on.
      * @param elemPrefix A const pointer to a Unicode string containing
      *                   the namespace prefix for this element. Applicable
      *                   only when namespace processing is enabled.
      * @param attrList  A const reference to the object containing the
      *                  list of attributes just scanned for this element.
      * @param attrCount A count of number of attributes in the list
      *                  specified by the parameter 'attrList'.
      * @param isEmpty  A flag indicating whether this is an empty element
      *                 or not.
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

    /**
      * This method is used to indicate the start of an entity reference.
      *
      * <p>If any advanced callback handlers are installed, the
      * corresponding 'endEntityReference' method is invoked.</p>
      *
      * @param entDecl A const reference to the object containing the
      *                entity declaration information.
      */
    virtual void startEntityReference
    (
        const   XMLEntityDecl&  entDecl
    );

    /**
      * This method is used to report the XML decl scanned by the parser.
      * Refer to the XML specification to see the meaning of parameters.
      *
      * <b>This method is a no-op for this SAX driver
      * implementation.</b>
      *
      * @param versionStr A const pointer to a Unicode string representing
      *                   version string value.
      * @param encodingStr A const pointer to a Unicode string representing
      *                    the encoding string value.
      * @param standaloneStr A const pointer to a Unicode string
      *                      representing the standalone string value.
      * @param actualEncodingStr A const pointer to a Unicode string
      *                          representing the actual encoding string
      *                          value.
      */
    virtual void XMLDecl
    (
        const   XMLCh* const    versionStr
        , const XMLCh* const    encodingStr
        , const XMLCh* const    standaloneStr
        , const XMLCh* const    actualEncodingStr
    );
    //@}


    // -----------------------------------------------------------------------
    //  Implementation of the XMLErrorReporter interface
    // -----------------------------------------------------------------------

    /** @name Implementation of the XMLErrorReporter Interface. */
    //@{
    /**
      * This method is used to report back errors found while parsing the
      * XML file. The driver will call the corresponding user installed
      * SAX Error Handler methods: 'fatal', 'error', 'warning' depending
      * on the severity of the error. This classification is defined by
      * the XML specification.
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

    /**
      * This method allows the user installed Error Handler
      * callback to 'reset' itself.
      *
      * <b>This method is a no-op for this SAX driver
      * implementation.</b>
      *
      */
    virtual void resetErrors();
    //@}


    // -----------------------------------------------------------------------
    //  Implementation of the XMLEntityHandler interface
    // -----------------------------------------------------------------------

    /** @name Implementation of the XMLEntityHandler Interface. */
    //@{
    /**
      * This method is used to indicate the end of parsing of an external
      * entity file.
      *
      * <b>This method is a no-op for this SAX driver
      * implementation.</b>
      *
      * @param inputSource A const reference to the InputSource object
      *                    which points to the XML file being parsed.
      * @see InputSource
      */
    virtual void endInputSource(const InputSource& inputSource);

    /**
      * This method allows an installed XMLEntityHandler to further
      * process any system id's of external entities encountered in
      * the XML file being parsed, such as redirection etc.
      *
      * <b>This method always returns 'false'
      * for this SAX driver implementation.</b>
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

    /**
      * This method allows the installed XMLEntityHandler to reset
      * itself.
      *
      * <b>This method is a no-op for this SAX driver
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

    /**
      * This method is used to indicate the start of parsing an
      * external entity file.
      *
      * <b>This method is a no-op for this SAX driver
      * implementation.</b>
      *
      * @param inputSource A const reference to the InputSource object
      *                    which points to the external entity
      *                    being parsed.
      */
    virtual void startInputSource(const InputSource& inputSource);
    //@}


    // -----------------------------------------------------------------------
    //  Implementation of the Deprecated DocTypeHandler Interface
    // -----------------------------------------------------------------------
    /** @name Implementation of the deprecated DocTypeHandler Interface */
    //@{
    /**
      * This method is used to report an attribute definition.
      *
      * <b>This method is a no-op for this SAX
      * driver implementation.</b>
      *
      * @param elemDecl A const reference to the object containing information
      *                 about the element whose attribute definition was just
      *                 parsed.
      * @param attDef   A const reference to the object containing information
      *                 attribute definition.
      * @param ignore   The flag indicating whether this attribute definition
      *                 was ignored by the parser or not.
      */
    virtual void attDef
    (
        const   DTDElementDecl& elemDecl
        , const DTDAttDef&      attDef
        , const bool            ignore
    );

    /**
      * This method is used to report a comment occurring within the DTD.
      *
      * <b>This method is a no-op for this SAX driver
      * implementation.</b>
      *
      * @param comment  A const pointer to a Unicode string representing the
      *                 text of the comment just parsed.
      */
    virtual void doctypeComment
    (
        const   XMLCh* const    comment
    );

    /**
      * This method is used to report the DOCTYPE declaration.
      *
      * <b>This method is a no-op for this SAX driver
      * implementation.</b>
      *
      * @param elemDecl A const reference to the object containing information
      *                 about the root element definition declaration of the
      *                 XML document being parsed.
      * @param publicId A const pointer to a Unicode string representing the
      *                 public id of the DTD file.
      * @param systemId A const pointer to a Unicode string representing the
      *                 system id of the DTD file.
      * @param hasIntSubset A flag indicating if this XML file contains any
      *                     internal subset.
      * @param hasExtSubset A flag indicating if this XML file contains any
      *                     external subset. Default is false.
      */
    virtual void doctypeDecl
    (
        const   DTDElementDecl& elemDecl
        , const XMLCh* const    publicId
        , const XMLCh* const    systemId
        , const bool            hasIntSubset
        , const bool            hasExtSubset = false
    );

    /**
      * This method is used to report any PI declarations
      * occurring inside the DTD definition block.
      *
      * <b>This method is a no-op for this SAX driver
      * implementation.</b>
      *
      * @param target A const pointer to a Unicode string representing the
      *               target of the PI declaration.
      * @param data   A const pointer to a Unicode string representing the
      *               data of the PI declaration. See the PI production rule
      *               in the XML specification for details.
      */
    virtual void doctypePI
    (
        const   XMLCh* const    target
        , const XMLCh* const    data
    );

    /**
      * This method is used to report any whitespaces
      * occurring inside the DTD definition block.
      *
      * <b>This method is a no-op for this SAX driver
      * implementation.</b>
      *
      * @param chars  A const pointer to a Unicode string representing the
      *               whitespace characters.
      * @param length The length of the whitespace Unicode string.
      */
    virtual void doctypeWhitespace
    (
        const   XMLCh* const    chars
        , const XMLSize_t       length
    );

    /**
      * This method is used to report an element declarations
      * successfully scanned by the parser.
      *
      * <b>This method is a no-op for this SAX driver
      * implementation.</b>
      *
      * @param decl   A const reference to the object containing element
      *               declaration information.
      * @param isIgnored The flag indicating whether this definition was
      *                  ignored by the parser or not.
      */
    virtual void elementDecl
    (
        const   DTDElementDecl& decl
        , const bool            isIgnored
    );

    /**
      * This method is used to report the end of an attribute
      * list declaration for an element.
      *
      * <b>This method is a no-op for this SAX driver
      * implementation.</b>
      *
      * @param elemDecl A const reference to the object containing element
      *                 declaration information.
      */
    virtual void endAttList
    (
        const   DTDElementDecl& elemDecl
    );

    /**
      * This method is used to report the end of the internal subset.
      *
      * <b>This method is a no-op for this SAX driver
      * implementation.</b>
      */
    virtual void endIntSubset();

    /**
      * This method is used to report the end of the external subset.
      *
      * <b>This method is a no-op for this SAX driver
      * implementation.</b>
      */
    virtual void endExtSubset();

    /**
      * This method is used to report any entity declarations.
      * For unparsed entities, this driver will invoke the
      * SAX DTDHandler::unparsedEntityDecl callback.
      *
      * @param entityDecl A const reference to the object containing
      *                   the entity declaration information.
      * @param isPEDecl  The flag indicating whether this was a
      *                  parameter entity declaration or not.
      * @param isIgnored The flag indicating whether this definition
      *                  was ignored by the parser or not.
      *
      * @see DTDHandler#unparsedEntityDecl
      */
    virtual void entityDecl
    (
        const   DTDEntityDecl&  entityDecl
        , const bool            isPEDecl
        , const bool            isIgnored
    );

    /**
      * This method allows the user installed DTD handler to
      * reset itself.
      */
    virtual void resetDocType();

    /**
      * This method is used to report any notation declarations.
      * If there is a user installed DTDHandler, then the driver will
      * invoke the SAX DTDHandler::notationDecl callback.
      *
      * @param notDecl A const reference to the object containing the notation
      *                declaration information.
      * @param isIgnored The flag indicating whether this definition was ignored
      *                  by the parser or not.
      *
      * @see DTDHandler#notationDecl
      */
    virtual void notationDecl
    (
        const   XMLNotationDecl&    notDecl
        , const bool                isIgnored
    );

    /**
      * This method is used to indicate the start of an element's attribute
      * list declaration.
      *
      * <b>This method is a no-op for this SAX driver
      * implementation.</b>
      *
      * @param elemDecl A const reference to the object containing element
      *                 declaration information.
      */
    virtual void startAttList
    (
        const   DTDElementDecl& elemDecl
    );

    /**
      * This method is used indicate the start of the internal subset.
      *
      * <b>This method is a no-op for this SAX driver
      * implementation.</b>
      */
    virtual void startIntSubset();

    /**
      * This method is used indicate the start of the external subset.
      *
      * <b>This method is a no-op for this SAX driver
      * implementation.</b>
      */
    virtual void startExtSubset();

    /**
      * This method is used to report the TextDecl. Refer to the XML
      * specification for the syntax of a TextDecl.
      *
      * <b>This method is a no-op for this SAX driver
      * implementation.</b>
      *
      * @param versionStr A const pointer to a Unicode string representing
      *                   the version number of the 'version' clause.
      * @param encodingStr A const pointer to a Unicode string representing
      *                    the encoding name of the 'encoding' clause.
      */
    virtual void TextDecl
    (
        const   XMLCh* const    versionStr
        , const XMLCh* const    encodingStr
    );
    //@}

protected :
    // -----------------------------------------------------------------------
    //  Protected Methods
    // -----------------------------------------------------------------------
    /**
      * This method returns a reference to the underlying scanner object.
      * It allows read only access to data maintained in the scanner.
      *
      * @return A const reference to the underlying scanner object.
      */
    const XMLScanner& getScanner() const;

    /** Get the Grammar resolver
      *
      * This provides derived classes with access to the grammar resolver.
      */
    GrammarResolver* getGrammarResolver() const;


private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    SAXParser(const SAXParser&);
    SAXParser& operator=(const SAXParser&);

    // -----------------------------------------------------------------------
    //  Initialize/Cleanup methods
    // -----------------------------------------------------------------------
    void initialize();
    void cleanUp();
    void resetInProgress();

    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fAttrList
    //      A temporary implementation of the basic SAX attribute list
    //      interface. We use this one over and over on each startElement
    //      event to allow SAX-like access to the element attributes.
    //
    //  fDocHandler
    //      The installed SAX doc handler, if any. Null if none.
    //
    //  fDTDHandler
    //      The installed SAX DTD handler, if any. Null if none.
    //
    //  fElemDepth
    //      This is used to track the element nesting depth, so that we can
    //      know when we are inside content. This is so we can ignore char
    //      data outside of content.
    //
    //  fEntityResolver
    //      The installed SAX entity handler, if any. Null if none.
    //
    //  fErrorHandler
    //      The installed SAX error handler, if any. Null if none.
    //
    //  fPSVIHandler
    //      The installed PSVI handler, if any. Null if none.
    //
    //  fAdvDHCount
    //  fAdvDHList
    //  fAdvDHListSize
    //      This is an array of pointers to XMLDocumentHandlers, which is
    //      how we see installed advanced document handlers. There will
    //      usually not be very many at all, so a simple array is used
    //      instead of a collection, for performance. It will grow if needed,
    //      but that is unlikely.
    //
    //      The count is how many handlers are currently installed. The size
    //      is how big the array itself is (for expansion purposes.) When
    //      count == size, is time to expand.
    //
    //  fParseInProgress
    //      This flag is set once a parse starts. It is used to prevent
    //      multiple entrance or reentrance of the parser.
    //
    //  fScanner
    //      The scanner being used by this parser. It is created internally
    //      during construction.
    //
    //   fGrammarPool
    //      The grammar pool passed from external application (through derivatives).
    //      which could be 0, not owned.
    //
    // -----------------------------------------------------------------------
    bool                 fParseInProgress;
    XMLSize_t            fElemDepth;
    XMLSize_t            fAdvDHCount;
    XMLSize_t            fAdvDHListSize;
    VecAttrListImpl      fAttrList;
    DocumentHandler*     fDocHandler;
    DTDHandler*          fDTDHandler;
    EntityResolver*      fEntityResolver;
    XMLEntityResolver*   fXMLEntityResolver;
    ErrorHandler*        fErrorHandler;
    PSVIHandler*         fPSVIHandler;
    XMLDocumentHandler** fAdvDHList;
    XMLScanner*          fScanner;
    GrammarResolver*     fGrammarResolver;
    XMLStringPool*       fURIStringPool;
    XMLValidator*        fValidator;
    MemoryManager*       fMemoryManager;
    XMLGrammarPool*      fGrammarPool;
    XMLBuffer            fElemQNameBuf;
};


// ---------------------------------------------------------------------------
//  SAXParser: Getter methods
// ---------------------------------------------------------------------------
inline DocumentHandler* SAXParser::getDocumentHandler()
{
    return fDocHandler;
}

inline const DocumentHandler* SAXParser::getDocumentHandler() const
{
    return fDocHandler;
}

inline EntityResolver* SAXParser::getEntityResolver()
{
    return fEntityResolver;
}

inline XMLEntityResolver* SAXParser::getXMLEntityResolver()
{
    return fXMLEntityResolver;
}

inline const XMLEntityResolver* SAXParser::getXMLEntityResolver() const
{
    return fXMLEntityResolver;
}

inline const EntityResolver* SAXParser::getEntityResolver() const
{
    return fEntityResolver;
}

inline ErrorHandler* SAXParser::getErrorHandler()
{
    return fErrorHandler;
}

inline const ErrorHandler* SAXParser::getErrorHandler() const
{
    return fErrorHandler;
}

inline PSVIHandler* SAXParser::getPSVIHandler()
{
    return fPSVIHandler;
}

inline const PSVIHandler* SAXParser::getPSVIHandler() const
{
    return fPSVIHandler;
}

inline const XMLScanner& SAXParser::getScanner() const
{
    return *fScanner;
}

inline GrammarResolver* SAXParser::getGrammarResolver() const
{
    return fGrammarResolver;
}

XERCES_CPP_NAMESPACE_END

#endif
