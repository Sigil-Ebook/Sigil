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
 * $Id: AbstractDOMParser.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_ABSTRACTDOMPARSER_HPP)
#define XERCESC_INCLUDE_GUARD_ABSTRACTDOMPARSER_HPP

#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/framework/XMLDocumentHandler.hpp>
#include <xercesc/framework/XMLErrorReporter.hpp>
#include <xercesc/framework/XMLEntityHandler.hpp>
#include <xercesc/util/SecurityManager.hpp>
#include <xercesc/util/ValueStackOf.hpp>
#include <xercesc/validators/DTD/DocTypeHandler.hpp>
#include <xercesc/dom/DOMDocumentType.hpp>
#include <xercesc/validators/DTD/DTDElementDecl.hpp>
#include <xercesc/framework/XMLBufferMgr.hpp>
#include <xercesc/framework/psvi/PSVIHandler.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XMLPScanToken;
class XMLScanner;
class XMLValidator;
class DOMDocumentImpl;
class DOMDocumentTypeImpl;
class DOMEntityImpl;
class DOMElement;
class GrammarResolver;
class XMLGrammarPool;
class PSVIHandler;

/**
  * This class implements the Document Object Model (DOM) interface.
  * It is used as a base for DOM parsers (i.e. XercesDOMParser, DOMLSParser).
  */
class PARSERS_EXPORT AbstractDOMParser :

    public XMemory
    , public XMLDocumentHandler
    , public XMLErrorReporter
    , public XMLEntityHandler
    , public DocTypeHandler
    , public PSVIHandler
{
public :
    // -----------------------------------------------------------------------
    //  Class types
    // -----------------------------------------------------------------------
    /** @name Public constants */
    //@{

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

    //@}


    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    /** @name Destructor */
    //@{

    /**
      * Destructor
      */
    virtual ~AbstractDOMParser();

    //@}

    // -----------------------------------------------------------------------
    //  Utility methods
    // -----------------------------------------------------------------------

    /** @name Utility methods */
    //@{
    /** Reset the parser
      *
      * This method resets the state of the DOM driver and makes
      * it ready for a fresh parse run.
      */
    void reset();

    /** Adopt the DOM document
      *
      * This method returns the DOMDocument object representing the
      * root of the document tree.
      *
      * The caller will adopt the DOMDocument and thus is responsible to
      * call DOMDocument::release() to release the associated memory.
      * The parser will not delete it.   The ownership is transferred
      * from the parser to the caller.
      *
      * @return The adopted DOMDocument object which represents the entire
      *         XML document.
      */
    DOMDocument* adoptDocument();

    //@}


    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------

    /** @name Getter methods */
    //@{

    /** Get the DOM document
      *
      * This method returns the DOMDocument object representing the
      * root of the document tree. This object provides the primary
      * access to the document's data.
      *
      * The returned DOMDocument object is owned by the parser.
      *
      * @return The DOMDocument object which represents the entire
      *         XML document.
      */
    DOMDocument* getDocument();

    /** Get a const reference to the validator
      *
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

    /** Get the identity constraint checking' flag
      *
      * This method returns the state of the parser's identity constraint
      * checking flag.
      *
      * @return true, if the parser is currently configured to
      *         have identity constraint checking, false otherwise.
      *
      * @see setIdentityConstraintChecking
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
      *
      */
    XMLSize_t getErrorCount() const;

    /** Get the 'do namespaces' flag
      *
      * This method returns the state of the parser's namespace processing
      * flag.
      *
      * @return true, if the parser is currently configured to
      *         understand namespaces, false otherwise.
      *
      * @see #setDoNamespaces
      */
    bool getDoNamespaces() const;

    /** Get the 'exit on first error' flag
      *
      * This method returns the state of the parser's
      * exit-on-First-Fatal-Error flag. If this flag is true, then the
      * parse will exit the first time it sees any non-wellformed XML or
      * any validity error. The default state is true.
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

    /** Get the 'include entity references' flag
      *
      * This method returns the flag that specifies whether the parser is
      * creating entity reference nodes in the DOM tree being produced.
      *
      * @return  The state of the create entity reference node
      *               flag.
      * @see #setCreateEntityReferenceNodes
      */
    bool  getCreateEntityReferenceNodes()const;

   /** Get the 'include ignorable whitespace' flag.
      *
      * This method returns the state of the parser's include ignorable
      * whitespace flag.
      *
      * @return 'true' if the include ignorable whitespace flag is set on
      *         the parser, 'false' otherwise.
      *
      * @see #setIncludeIgnorableWhitespace
      */
    bool getIncludeIgnorableWhitespace() const;

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
      * @see #setSecurityManager
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
    const XMLSize_t& getLowWaterMark() const;

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

    /** Get the 'create comment node' flag
      *
      * This method returns the flag that specifies whether the parser is
      * creating comment nodes in the DOM tree being produced.
      *
      * @return  The state of the create comment node flag.
      * @see #setCreateCommentNodes
      */
    bool  getCreateCommentNodes()const;

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

    /** Get the 'associate schema info' flag
      *
      * This method returns the flag that specifies whether
      * the parser is storing schema informations in the element
      * and attribute nodes in the DOM tree being produced.
      *
      * @return  The state of the associate schema info flag.
      * @see #setCreateSchemaInfo
      */
    bool getCreateSchemaInfo() const;

    /** Get the 'do XInclude' flag
      *
      * This method returns the flag that specifies whether
      * the parser will process XInclude nodes
      * in the DOM tree being produced.
      *
      * @return  The state of the 'do XInclude' flag.
      * @see #setDoXInclude
      */
    bool getDoXInclude() const;

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
      *         A synthetic XSAnnotation is created when a schema
      *         component has non-schema attributes but has no
      *         child annotations so that the non-schema attributes
      *         can be recovered under PSVI.
      *
      * @see #getGenerateSyntheticAnnotations
      */
    void setGenerateSyntheticAnnotations(const bool newValue);

    /** set the 'validlate annotations' flag
      *
      * @param newValue The value for specifying whether Annotations
      *        should be validated or not.
      *
      * @see #getValidateAnnotations
      */
    void setValidateAnnotations(const bool newValue);

    /** Set the 'do namespaces' flag
      *
      * This method allows users to enable or disable the parser's
      * namespace processing. When set to true, parser starts enforcing
      * all the constraints and rules specified by the NameSpace
      * specification.
      *
      * The parser's default state is: false.
      *
      * @param newState The value specifying whether NameSpace rules should
      *                 be enforced or not.
      *
      * @see #getDoNamespaces
      */
    void setDoNamespaces(const bool newState);

    /** Set the 'exit on first error' flag
      *
      * This method allows users to set the parser's behaviour when it
      * encounters the first fatal error. If set to true, the parser
      * will exit at the first fatal error. If false, then it will
      * report the error and continue processing.
      *
      * The default value is 'true' and the parser exits on the
      * first fatal error.
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

     /** Set the 'include entity references' flag
      *
      * This method allows the user to specify whether the parser should
      * create entity reference nodes in the DOM tree being produced.
      * When the 'create' flag is
      * true, the parser will create EntityReference nodes in the DOM tree.
      * The EntityReference nodes and their child nodes will be read-only.
      * When the 'create' flag is false, no EntityReference nodes will be created.
      * <p>The replacement text
      * of the entity is included in either case, either as a
      * child of the Entity Reference node or in place at the location
      * of the reference.
      * <p>The default value is 'true'.
      *
      * @param create The new state of the create entity reference nodes
      *               flag.
      * @see #getCreateEntityReferenceNodes
      */
    void setCreateEntityReferenceNodes(const bool create);

   /** Set the 'include ignorable whitespace' flag
      *
      * This method allows the user to specify whether a validating parser
      * should include ignorable whitespaces as text nodes.  It has no effect
      * on non-validating parsers which always include non-markup text.
      * <p>When set to true (also the default), ignorable whitespaces will be
      * added to the DOM tree as text nodes.  The method
      * DOMText::isIgnorableWhitespace() will return true for those text
      * nodes only.
      * <p>When set to false, all ignorable whitespace will be discarded and
      * no text node is added to the DOM tree.  Note: applications intended
      * to process the "xml:space" attribute should not set this flag to false.
      * And this flag also overrides any schema datateye whitespace facets,
      * that is, all ignorable whitespace will be discarded even though
      * 'preserve' is set in schema datatype whitespace facets.
      *
      * @param include The new state of the include ignorable whitespace
      *                flag.
      *
      * @see #getIncludeIgnorableWhitespace
      */
    void setIncludeIgnorableWhitespace(const bool include);

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

    /** Set the 'do schema' flag
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
      * This method allows users to enable or disable the parser's identity
      * constraint checks.
      *
      * <p>By default, the parser does identity constraint checks.
      *    The default value is true.</p>
      *
      * @param newState The value specifying whether the parser should
      *                 do identity constraint checks or not in the
      *                 input XML document.
      *
      * @see #getIdentityConstraintChecking
      */
    void setIdentityConstraintChecking(const bool newState);

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

     /** Set the 'create comment nodes' flag
      *
      * This method allows the user to specify whether the parser should
      * create comment nodes in the DOM tree being produced.
      * <p>The default value is 'true'.
      *
      * @param create The new state of the create comment nodes
      *               flag.
      * @see #getCreateCommentNodes
      */
    void setCreateCommentNodes(const bool create);

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
      * This method allows users to set the  scanner to use
      * when scanning a given XML document.
      *
      * @param scannerName The name of the desired scanner
      */
    void useScanner(const XMLCh* const scannerName);

    /** Set the implementation to use when creating the  document
      *
      * This method allows users to set the implementation to use
      * to create the document when parseing.
      *
      * @param implementationFeatures The names of the desired features the implementation should have.
      */
    void useImplementation(const XMLCh* const implementationFeatures);

    /**
      * This method installs the user specified PSVI handler on
      * the parser.
      *
      * @param handler A pointer to the PSVI handler to be called
      *                when the parser comes across 'PSVI' events
      *                as per the schema specification.
      */
    virtual void setPSVIHandler(PSVIHandler* const handler);

    /** Set the 'associate schema info' flag
      *
      * This method allows users to specify whether
      * the parser should store schema informations in the element
      * and attribute nodes in the DOM tree being produced.
      *
      * @param newState The state to set
      * @see #getCreateSchemaInfo
      */
    void  setCreateSchemaInfo(const bool newState);

    /** Set the 'do XInclude' flag
      *
      * This method allows users to specify whether
      * the parser should process XInclude nodes
      * in the DOM tree being produced.
      *
      * @param newState The state to set
      * @see #getDoXInclude
      */
    void  setDoXInclude(const bool newState);

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
    //  Parsing methods
    // -----------------------------------------------------------------------

    /** @name Parsing methods */
    //@{

    /** Parse via an input source object
      *
      * This method invokes the parsing process on the XML file specified
      * by the InputSource parameter. This API is borrowed from the
      * SAX Parser interface.
      *
      * @param source A const reference to the InputSource object which
      *               points to the XML file to be parsed.
      * @exception SAXException Any SAX exception, possibly
      *            wrapping another exception.
      * @exception XMLException An exception from the parser or client
      *            handler code.
      * @exception DOMException A DOM exception as per DOM spec.
      * @see InputSource#InputSource
      */
    void parse(const InputSource& source);

    /** Parse via a file path or URL
      *
      * This method invokes the parsing process on the XML file specified by
      * the Unicode string parameter 'systemId'. This method is borrowed
      * from the SAX Parser interface.
      *
      * @param systemId A const XMLCh pointer to the Unicode string which
      *                 contains the path to the XML file to be parsed.
      *
      * @exception SAXException Any SAX exception, possibly
      *            wrapping another exception.
      * @exception XMLException An exception from the parser or client
      *            handler code.
      * @exception DOMException A DOM exception as per DOM spec.
      * @see #parse(InputSource,...)
      */
    void parse(const XMLCh* const systemId);

    /** Parse via a file path or URL (in the local code page)
      *
      * This method invokes the parsing process on the XML file specified by
      * the native char* string parameter 'systemId'.
      *
      * @param systemId A const char pointer to a native string which
      *                 contains the path to the XML file to be parsed.
      *
      * @exception SAXException Any SAX exception, possibly
      *            wrapping another exception.
      * @exception XMLException An exception from the parser or client
      *            handler code.
      * @exception DOMException A DOM exception as per DOM spec.
      * @see #parse(InputSource,...)
      */
    void parse(const char* const systemId);

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
      * @return 'true', if successful in parsing the prolog. It indicates the
      *         user can go ahead with parsing the rest of the file. It
      *         returns 'false' to indicate that the parser could not parse
      *         the prolog.
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
      * a XML token (as defined in the XML specification).
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
      *
      * @see #parseFirst(XMLCh*,...)
      * @see #parseFirst(char*,...)
      * @see #parseFirst(InputSource&,...)
      */
    void parseReset(XMLPScanToken& token);

    //@}

    // -----------------------------------------------------------------------
    //  Implementation of the PSVIHandler interface.
    // -----------------------------------------------------------------------

    /** @name Implementation of the PSVIHandler interface. */
    //@{

    /** Receive notification of the PSVI properties of an element.
      * The scanner will issue this call after the XMLDocumentHandler
      * endElement call.  Since the scanner will issue the psviAttributes
      * call immediately after reading the start tag of an element, all element
      * content will be effectively bracketed by these two calls.
      * @param  localName The name of the element whose end tag was just
      *                     parsed.
      * @param  uri       The namespace to which the element is bound
      * @param  elementInfo    Object containing the element's PSVI properties
      */
    virtual void handleElementPSVI
    (
        const   XMLCh* const            localName
        , const XMLCh* const            uri
        ,       PSVIElement *           elementInfo
    );

    virtual void handlePartialElementPSVI
    (
        const   XMLCh* const            localName
        , const XMLCh* const            uri
        ,       PSVIElement *           elementInfo
    );
    /**
      * Enables PSVI information about attributes to be passed back to the
      * application.  This callback will be made on *all*
      * elements; on elements with no attributes, the final parameter will
      * be null.
      * @param  localName The name of the element upon which start tag
      *          these attributes were encountered.
      * @param  uri       The namespace to which the element is bound
      * @param  psviAttributes   Object containing the attributes' PSVI properties
      *          with information to identify them.
      */
    virtual void handleAttributesPSVI
    (
        const   XMLCh* const            localName
        , const XMLCh* const            uri
        ,       PSVIAttributeList *     psviAttributes
    );
    //@}

    // -----------------------------------------------------------------------
    //  Implementation of the XMLDocumentHandler interface.
    // -----------------------------------------------------------------------

    /** @name Implementation of the XMLDocumentHandler interface. */
    //@{

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

    /** Handle a document PI event
      *
      * This method is used to report any PI scanned by the parser. A new
      * PI node is created and appended as a child of the current node in
      * the tree.
      *
      * @param target A const pointer to a Unicode string representing the
      *               target of the PI declaration.
      * @param data   A const pointer to a Unicode string representing the
      *               data of the PI declaration. See the PI production rule
      *               in the XML specification for details.
      */
    virtual void docPI
    (
        const   XMLCh* const    target
        , const XMLCh* const    data
    );

    /** Handle the end of document event
      *
      * This method is used to indicate the end of the current document.
      */
    virtual void endDocument();

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

    /** Handle a document reset event
      *
      * This method allows the user installed Document Handler to 'reset'
      * itself, freeing all the memory resources. The scanner calls this
      * method before starting a new parse event.
      */
    virtual void resetDocument();

    /** Handle a start document event
      *
      * This method is used to report the start of the parsing process.
      */
    virtual void startDocument();

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

    /** Handle an XMLDecl event
      *
      * This method is used to report the XML decl scanned by the parser.
      * Refer to the XML specification to see the meaning of parameters.
      *
      * <b>This method is a no-op for this DOM
      * implementation.</b>
      *
      * @param versionStr A const pointer to a Unicode string representing
      *                   version string value.
      * @param encodingStr A const pointer to a Unicode string representing
      *                    the encoding string value.
      * @param standaloneStr A const pointer to a Unicode string
      *                      representing the standalone string value.
      * @param actualEncStr A const pointer to a Unicode string
      *                     representing the actual encoding string
      *                     value.
      */
    virtual void XMLDecl
    (
        const   XMLCh* const    versionStr
        , const XMLCh* const    encodingStr
        , const XMLCh* const    standaloneStr
        , const XMLCh* const    actualEncStr
    );

    //@}


    // -----------------------------------------------------------------------
    //  Implementation of the deprecated DocTypeHandler interface.
    // -----------------------------------------------------------------------
    /** @name Deprecated DocTypeHandler Interfaces */
    //@{
    virtual void attDef
    (
        const   DTDElementDecl&     elemDecl
        , const DTDAttDef&          attDef
        , const bool                ignoring
    );

    virtual void doctypeComment
    (
        const   XMLCh* const    comment
    );

    virtual void doctypeDecl
    (
        const   DTDElementDecl& elemDecl
        , const XMLCh* const    publicId
        , const XMLCh* const    systemId
        , const bool            hasIntSubset
        , const bool            hasExtSubset = false
    );

    virtual void doctypePI
    (
        const   XMLCh* const    target
        , const XMLCh* const    data
    );

    virtual void doctypeWhitespace
    (
        const   XMLCh* const    chars
        , const XMLSize_t       length
    );

    virtual void elementDecl
    (
        const   DTDElementDecl& decl
        , const bool            isIgnored
    );

    virtual void endAttList
    (
        const   DTDElementDecl& elemDecl
    );

    virtual void endIntSubset();

    virtual void endExtSubset();

    virtual void entityDecl
    (
        const   DTDEntityDecl&  entityDecl
        , const bool            isPEDecl
        , const bool            isIgnored
    );

    virtual void resetDocType();

    virtual void notationDecl
    (
        const   XMLNotationDecl&    notDecl
        , const bool                isIgnored
    );

    virtual void startAttList
    (
        const   DTDElementDecl& elemDecl
    );

    virtual void startIntSubset();

    virtual void startExtSubset();

    virtual void TextDecl
    (
        const   XMLCh* const    versionStr
        , const XMLCh* const    encodingStr
    );

    //@}

protected:
    // DOM node creation hooks. Override them if you are using your own
    // DOM node types.
    //
    virtual DOMCDATASection* createCDATASection (const XMLCh*, XMLSize_t);
    virtual DOMText* createText (const XMLCh*, XMLSize_t);

    virtual DOMElement* createElement (const XMLCh* name);
    virtual DOMElement* createElementNS (const XMLCh* namespaceURI,
                                         const XMLCh* elemPrefix,
                                         const XMLCh* localName,
                                         const XMLCh* qName);

    virtual DOMAttr* createAttr (const XMLCh* name);
    virtual DOMAttr* createAttrNS (const XMLCh* namespaceURI,
                                   const XMLCh* elemPrefix,
                                   const XMLCh* localName,
                                   const XMLCh* qName);




protected :
    // -----------------------------------------------------------------------
    //  Protected Constructor Methods
    // -----------------------------------------------------------------------
    /** @name Constructors */
    //@{
    /** Construct a AbstractDOMParser, with an optional validator
      *
      * Constructor with an instance of validator class to use for
      * validation. If you don't provide a validator, a default one will
      * be created for you in the scanner.
      *
      * @param valToAdopt Pointer to the validator instance to use. The
      *                   parser is responsible for freeing the memory.
      *
      * @param gramPool   Pointer to the grammar pool instance from
      *                   external application (through derivatives).
      *                   The parser does NOT own it.
      *
      * @param manager    Pointer to the memory manager to be used to
      *                   allocate objects.
      */
    AbstractDOMParser
    (
          XMLValidator* const   valToAdopt = 0
        , MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
        , XMLGrammarPool* const gramPool = 0
    );

    //@}

    // -----------------------------------------------------------------------
    //  Protected getter methods
    // -----------------------------------------------------------------------
    /** @name Protected getter methods */
    //@{
    /** Get the current DOM node
      *
      * This provides derived classes with access to the current node, i.e.
      * the node to which new nodes are being added.
      */
    DOMNode* getCurrentNode();

    /** Get the XML scanner
      *
      * This provides derived classes with access to the XML scanner.
      */
    XMLScanner* getScanner() const;

    /** Get the Grammar resolver
      *
      * This provides derived classes with access to the grammar resolver.
      */
    GrammarResolver* getGrammarResolver() const;

    /** Get the parse in progress flag
      *
      * This provides derived classes with access to the parse in progress
      * flag.
      */
    bool getParseInProgress() const;

    MemoryManager* getMemoryManager() const;

    //@}


    // -----------------------------------------------------------------------
    //  Protected setter methods
    // -----------------------------------------------------------------------

    /** @name Protected setter methods */
    //@{

    /** Set the current DOM node
      *
      * This method sets the current node maintained inside the parser to
      * the one specified.
      *
      * @param toSet The DOM node which will be the current node.
      */
    void setCurrentNode(DOMNode* toSet);

    /** Set the document node
      *
      * This method sets the DOM Document node to the one specified.
      *
      * @param toSet The new DOM Document node for this XML document.
      */
    void setDocument(DOMDocument* toSet);

    /** Set the parse in progress flag
      *
      * This method sets the parse in progress flag to true or false.
      *
      * @param toSet The value of the flag to be set.
      */
    void setParseInProgress(const bool toSet);
    //@}

    // -----------------------------------------------------------------------
    //  Protected Helper methods
    // -----------------------------------------------------------------------
    /** @name Protected helper methods */
    //@{
    void resetPool();

    /**
     * Returns true if the user has adopted the document
     */
    bool isDocumentAdopted() const;

    //@}


private :
    // -----------------------------------------------------------------------
    //  Initialize/Cleanup methods
    // -----------------------------------------------------------------------
    void initialize();
    void cleanUp();
    void resetInProgress();

    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    AbstractDOMParser(const AbstractDOMParser&);
    AbstractDOMParser& operator=(const AbstractDOMParser&);

protected:
    // -----------------------------------------------------------------------
    //  Protected data members
    //
    //  fCurrentNode
    //  fCurrentParent
    //      Used to track the current node during nested element events. Since
    //      the tree must be built from a set of disjoint callbacks, we need
    //      these to keep up with where we currently are.
    //
    //  fCurrentEntity
    //      Used to track the current entity decl.  If a text decl is seen later on,
    //      it is used to update the encoding and version information.
    //
    //  fDocument
    //      The root document object, filled with the document contents.
    //
    //  fCreateEntityReferenceNodes
    //      Indicates whether entity reference nodes should be created.
    //
    //  fIncludeIgnorableWhitespace
    //      Indicates whether ignorable whitespace should be added to
    //      the DOM tree for validating parsers.
    //
    //  fScanner
    //      The scanner used for this parser. This is created during the
    //      constructor.
    //
    //  fImplementationFeatures
    //      The implementation features that we use to get an implementation
    //      for use in creating the DOMDocument used during parse. If this is
    //      null then the default DOMImplementation is used
    //
    //  fParseInProgress
    //      Used to prevent multiple entrance to the parser while its doing
    //      a parse.
    //
    //  fWithinElement
    //      A flag to indicate that the parser is within at least one level
    //      of element processing.
    //
    //  fDocumentType
    //      Used to store and update the documentType variable information
    //      in fDocument
    //
    //  fDocumentVector
    //      Store all the previous fDocument(s) (thus not the current fDocument)
    //      created in this parser.  It is destroyed when the parser is destructed.
    //
    //  fCreateCommentNodes
    //      Indicates whether comment nodes should be created.
    //
    //  fDocumentAdoptedByUser
    //      The DOMDocument ownership has been transferred to application
    //      If set to true, the parser does not own the document anymore
    //      and thus will not release its memory.
    //
    //  fInternalSubset
    //      Buffer for storing the internal subset information.
    //      Once complete (after DOCTYPE is finished scanning), send
    //      it to DocumentType Node
    //
    //   fGrammarPool
    //      The grammar pool passed from external application (through derivatives).
    //      which could be 0, not owned.
    //
    //  fCreateSchemaInfo
    //      Indicates whether element and attributes will have schema info associated
    //
	//   fDoXinclude
	//      A bool used to request that XInlcude processing occur on the
	//      Document the parser parses.
    // -----------------------------------------------------------------------
    bool                          fCreateEntityReferenceNodes;
    bool                          fIncludeIgnorableWhitespace;
    bool                          fWithinElement;
    bool                          fParseInProgress;
    bool                          fCreateCommentNodes;
    bool                          fDocumentAdoptedByUser;
    bool                          fCreateSchemaInfo;
    bool                          fDoXInclude;
    XMLScanner*                   fScanner;
    XMLCh*                        fImplementationFeatures;
    DOMNode*                      fCurrentParent;
    DOMNode*                      fCurrentNode;
    DOMEntityImpl*                fCurrentEntity;
    DOMDocumentImpl*              fDocument;
    DOMDocumentTypeImpl*          fDocumentType;
    RefVectorOf<DOMDocumentImpl>* fDocumentVector;
    GrammarResolver*              fGrammarResolver;
    XMLStringPool*                fURIStringPool;
    XMLValidator*                 fValidator;
    MemoryManager*                fMemoryManager;
    XMLGrammarPool*               fGrammarPool;
    XMLBufferMgr                  fBufMgr;
    XMLBuffer&                    fInternalSubset;
    PSVIHandler*                  fPSVIHandler;
};



// ---------------------------------------------------------------------------
//  AbstractDOMParser: Getter methods
// ---------------------------------------------------------------------------
inline bool AbstractDOMParser::getCreateEntityReferenceNodes() const
{
    return fCreateEntityReferenceNodes;
}

inline bool AbstractDOMParser::getIncludeIgnorableWhitespace() const
{
    return fIncludeIgnorableWhitespace;
}

inline bool AbstractDOMParser::getParseInProgress() const
{
    return fParseInProgress;
}

inline XMLScanner* AbstractDOMParser::getScanner() const
{
    return fScanner;
}

inline GrammarResolver* AbstractDOMParser::getGrammarResolver() const
{
    return fGrammarResolver;
}

inline bool AbstractDOMParser::getCreateCommentNodes() const
{
    return fCreateCommentNodes;
}

inline PSVIHandler* AbstractDOMParser::getPSVIHandler()
{
    return fPSVIHandler;
}

inline const PSVIHandler* AbstractDOMParser::getPSVIHandler() const
{
    return fPSVIHandler;
}

inline bool AbstractDOMParser::getCreateSchemaInfo() const
{
    return fCreateSchemaInfo;
}

inline bool AbstractDOMParser::getDoXInclude() const
{
    return fDoXInclude;
}
// ---------------------------------------------------------------------------
//  AbstractDOMParser: Setter methods
// ---------------------------------------------------------------------------
inline void AbstractDOMParser::setCreateEntityReferenceNodes(const bool create)
{
    fCreateEntityReferenceNodes = create;
}

inline void AbstractDOMParser::setIncludeIgnorableWhitespace(const bool include)
{
    fIncludeIgnorableWhitespace = include;
}

inline void AbstractDOMParser::setCreateCommentNodes(const bool create)
{
    fCreateCommentNodes = create;
}

inline void AbstractDOMParser::useImplementation(const XMLCh* const implementationFeatures)
{
    fMemoryManager->deallocate(fImplementationFeatures);
    fImplementationFeatures = XMLString::replicate(implementationFeatures, fMemoryManager);
}

inline void AbstractDOMParser::setDoXInclude(const bool newState)
{
    fDoXInclude = newState;
}

// ---------------------------------------------------------------------------
//  AbstractDOMParser: Protected getter methods
// ---------------------------------------------------------------------------
inline DOMNode* AbstractDOMParser::getCurrentNode()
{
    return fCurrentNode;
}

inline MemoryManager* AbstractDOMParser::getMemoryManager() const
{
    return fMemoryManager;
}

// ---------------------------------------------------------------------------
//  AbstractDOMParser: Protected setter methods
// ---------------------------------------------------------------------------
inline void AbstractDOMParser::setCurrentNode(DOMNode* toSet)
{
    fCurrentNode = toSet;
}

inline void AbstractDOMParser::setParseInProgress(const bool toSet)
{
    fParseInProgress = toSet;
}

XERCES_CPP_NAMESPACE_END

#endif
