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
 * $Id: XMLScanner.hpp 882548 2009-11-20 13:44:14Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XMLSCANNER_HPP)
#define XERCESC_INCLUDE_GUARD_XMLSCANNER_HPP

#include <xercesc/framework/XMLBufferMgr.hpp>
#include <xercesc/framework/XMLErrorCodes.hpp>
#include <xercesc/framework/XMLRefInfo.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/NameIdPool.hpp>
#include <xercesc/util/RefHashTableOf.hpp>
#include <xercesc/util/SecurityManager.hpp>
#include <xercesc/internal/ReaderMgr.hpp>
#include <xercesc/internal/ElemStack.hpp>
#include <xercesc/validators/DTD/DTDEntityDecl.hpp>
#include <xercesc/framework/XMLAttr.hpp>
#include <xercesc/framework/ValidationContext.hpp>
#include <xercesc/validators/common/GrammarResolver.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class InputSource;
class XMLDocumentHandler;
class XMLEntityHandler;
class ErrorHandler;
class DocTypeHandler;
class XMLPScanToken;
class XMLStringPool;
class Grammar;
class XMLValidator;
class MemoryManager;
class PSVIHandler;


struct PSVIElemContext
{
    bool               fIsSpecified;
    bool               fErrorOccurred;
    int                fElemDepth;
    int                fFullValidationDepth;
    int                fNoneValidationDepth;
    DatatypeValidator* fCurrentDV;
    ComplexTypeInfo*   fCurrentTypeInfo;
    const XMLCh*       fNormalizedValue;
};

//  This is the mondo scanner class, which does the vast majority of the
//  work of parsing. It handles reading in input and spitting out events
//  to installed handlers.
class XMLPARSER_EXPORT XMLScanner : public XMemory, public XMLBufferFullHandler
{
public :
    // -----------------------------------------------------------------------
    //  Public class types
    //
    //  NOTE: These should really be private, but some of the compilers we
    //  have to deal with are too stupid to understand this.
    //
    //  DeclTypes
    //      Used by scanXMLDecl() to know what type of decl it should scan.
    //      Text decls have slightly different rules from XMLDecls.
    //
    //  EntityExpRes
    //      These are the values returned from the entity expansion method,
    //      to indicate how it went.
    //
    //  XMLTokens
    //      These represent the possible types of input we can get while
    //      scanning content.
    //
    //  ValScheme
    //      This indicates what the scanner should do in terms of validation.
    //      'Auto' means if there is any int/ext subset, then validate. Else,
    //      don't.
    // -----------------------------------------------------------------------
    enum DeclTypes
    {
        Decl_Text
        , Decl_XML
    };

    enum EntityExpRes
    {
        EntityExp_Pushed
        , EntityExp_Returned
        , EntityExp_Failed
    };

    enum XMLTokens
    {
        Token_CData
        , Token_CharData
        , Token_Comment
        , Token_EndTag
        , Token_EOF
        , Token_PI
        , Token_StartTag
        , Token_Unknown
    };

    enum ValSchemes
    {
        Val_Never
        , Val_Always
        , Val_Auto
    };


    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    XMLScanner
    (
        XMLValidator* const valToAdopt
        , GrammarResolver* const grammarResolver
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );
    XMLScanner
    (
        XMLDocumentHandler* const  docHandler
        , DocTypeHandler* const    docTypeHandler
        , XMLEntityHandler* const  entityHandler
        , XMLErrorReporter* const  errReporter
        , XMLValidator* const      valToAdopt
        , GrammarResolver* const grammarResolver
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );
    virtual ~XMLScanner();


    // -----------------------------------------------------------------------
    //  Error emitter methods
    // -----------------------------------------------------------------------
    bool emitErrorWillThrowException(const XMLErrs::Codes toEmit);
    void emitError(const XMLErrs::Codes toEmit);
    void emitError
    (
        const   XMLErrs::Codes    toEmit
        , const XMLCh* const        text1
        , const XMLCh* const        text2 = 0
        , const XMLCh* const        text3 = 0
        , const XMLCh* const        text4 = 0
    );
    void emitError
    (
        const   XMLErrs::Codes    toEmit
        , const char* const         text1
        , const char* const         text2 = 0
        , const char* const         text3 = 0
        , const char* const         text4 = 0
    );
    void emitError
    (
        const   XMLErrs::Codes    toEmit
        , const XMLExcepts::Codes   originalErrorCode
        , const XMLCh* const        text1 = 0
        , const XMLCh* const        text2 = 0
        , const XMLCh* const        text3 = 0
        , const XMLCh* const        text4 = 0

    );

    // -----------------------------------------------------------------------
    //  Implementation of XMLBufferFullHandler interface
    // -----------------------------------------------------------------------

    virtual bool bufferFull(XMLBuffer& toSend)
    {
        sendCharData(toSend);
        return true;
    }

    virtual Grammar::GrammarType getCurrentGrammarType() const;

    // -----------------------------------------------------------------------
    //  Public pure virtual methods
    // -----------------------------------------------------------------------
    virtual const XMLCh* getName() const = 0;
    virtual NameIdPool<DTDEntityDecl>* getEntityDeclPool() = 0;
    virtual const NameIdPool<DTDEntityDecl>* getEntityDeclPool() const = 0;
    virtual void scanDocument
    (
        const   InputSource&    src
    ) = 0;
    virtual bool scanNext(XMLPScanToken& toFill) = 0;
    virtual Grammar* loadGrammar
    (
        const   InputSource&    src
        , const short           grammarType
        , const bool            toCache = false
    ) = 0;

    virtual void resetCachedGrammar ();

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    const XMLDocumentHandler* getDocHandler() const;
    XMLDocumentHandler* getDocHandler();
    const DocTypeHandler* getDocTypeHandler() const;
    DocTypeHandler* getDocTypeHandler();
    bool getDoNamespaces() const;
    ValSchemes getValidationScheme() const;
    bool getDoSchema() const;
    bool getValidationSchemaFullChecking() const;
    bool getIdentityConstraintChecking() const;
    const XMLEntityHandler* getEntityHandler() const;
    XMLEntityHandler* getEntityHandler();
    const XMLErrorReporter* getErrorReporter() const;
    XMLErrorReporter* getErrorReporter();
    const ErrorHandler* getErrorHandler() const;
    ErrorHandler* getErrorHandler();
    const PSVIHandler* getPSVIHandler() const;
    PSVIHandler* getPSVIHandler();
    bool getExitOnFirstFatal() const;
    bool getValidationConstraintFatal() const;
    RefHashTableOf<XMLRefInfo>* getIDRefList();
    const RefHashTableOf<XMLRefInfo>* getIDRefList() const;

    ValidationContext*   getValidationContext();

    bool getInException() const;
    /*bool getLastExtLocation
    (
                XMLCh* const    sysIdToFill
        , const unsigned int    maxSysIdChars
        ,       XMLCh* const    pubIdToFill
        , const unsigned int    maxPubIdChars
        ,       XMLSSize_t&     lineToFill
        ,       XMLSSize_t&     colToFill
    ) const;*/
    const Locator* getLocator() const;
    const ReaderMgr* getReaderMgr() const;
    XMLFilePos getSrcOffset() const;
    bool getStandalone() const;
    const XMLValidator* getValidator() const;
    XMLValidator* getValidator();
    int getErrorCount();
    const XMLStringPool* getURIStringPool() const;
    XMLStringPool* getURIStringPool();
    bool getHasNoDTD() const;
    XMLCh* getExternalSchemaLocation() const;
    XMLCh* getExternalNoNamespaceSchemaLocation() const;
    SecurityManager* getSecurityManager() const;
    bool getLoadExternalDTD() const;
    bool getLoadSchema() const;
    bool getNormalizeData() const;
    bool isCachingGrammarFromParse() const;
    bool isUsingCachedGrammarInParse() const;
    bool getCalculateSrcOfs() const;
    Grammar* getRootGrammar() const;
    XMLReader::XMLVersion getXMLVersion() const;
    MemoryManager* getMemoryManager() const;
    ValueVectorOf<PrefMapElem*>* getNamespaceContext() const;
    unsigned int getPrefixId(const XMLCh* const prefix) const;
    const XMLCh* getPrefixForId(unsigned int prefId) const;

    // Return is a reference so that we can return it as void* from
    // getProperty.
    //
    const XMLSize_t& getLowWaterMark() const;

    bool getGenerateSyntheticAnnotations() const;
    bool getValidateAnnotations() const;
    bool getIgnoreCachedDTD() const;
    bool getIgnoreAnnotations() const;
    bool getDisableDefaultEntityResolution() const;
    bool getSkipDTDValidation() const;
    bool getHandleMultipleImports() const;

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    /**
      * When an attribute name has no prefix, unlike elements, it is not mapped
      * to the global namespace. So, in order to have something to map it to
      * for practical purposes, a id for an empty URL is created and used for
      * such names.
      *
      * @return The URL pool id of the URL for an empty URL "".
      */
    unsigned int getEmptyNamespaceId() const;

    /**
      * When a prefix is found that has not been mapped, an error is issued.
      * However, if the parser has been instructed not to stop on the first
      * fatal error, it needs to be able to continue. To do so, it will map
      * that prefix tot his magic unknown namespace id.
      *
      * @return The URL pool id of the URL for the unknown prefix
      *         namespace.
      */
    unsigned int getUnknownNamespaceId() const;

    /**
      * The prefix 'xml' is a magic prefix, defined by the XML spec and
      * requiring no prior definition. This method returns the id for the
      * intrinsically defined URL for this prefix.
      *
      * @return The URL pool id of the URL for the 'xml' prefix.
      */
    unsigned int getXMLNamespaceId() const;

    /**
      * The prefix 'xmlns' is a magic prefix, defined by the namespace spec
      * and requiring no prior definition. This method returns the id for the
      * intrinsically defined URL for this prefix.
      *
      * @return The URL pool id of the URL for the 'xmlns' prefix.
      */
    unsigned int getXMLNSNamespaceId() const;

    /**
      * This method find the passed URI id in its URI pool and
      * copy the text of that URI into the passed buffer.
      */
    bool getURIText
    (
        const   unsigned int    uriId
        ,       XMLBuffer&      uriBufToFill
    )   const;

    const XMLCh* getURIText(const   unsigned int    uriId) const;

    /* tell if the validator comes from user */
    bool isValidatorFromUser();

    /* tell if standard URI are forced */
    bool getStandardUriConformant() const;

    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------
    void addGlobalPrefix(const XMLCh* const prefix, const unsigned int uriId);
    void setDocHandler(XMLDocumentHandler* const docHandler);
    void setDocTypeHandler(DocTypeHandler* const docTypeHandler);
    void setDoNamespaces(const bool doNamespaces);
    void setEntityHandler(XMLEntityHandler* const docTypeHandler);
    void setErrorReporter(XMLErrorReporter* const errHandler);
    void setErrorHandler(ErrorHandler* const handler);
    void setPSVIHandler(PSVIHandler* const handler);
    void setURIStringPool(XMLStringPool* const stringPool);
    void setExitOnFirstFatal(const bool newValue);
    void setValidationConstraintFatal(const bool newValue);
    void setValidationScheme(const ValSchemes newScheme);
    void setValidator(XMLValidator* const valToAdopt);
    void setDoSchema(const bool doSchema);
    void setValidationSchemaFullChecking(const bool schemaFullChecking);
    void setIdentityConstraintChecking(const bool identityConstraintChecking);
    void setHasNoDTD(const bool hasNoDTD);
    void cacheGrammarFromParse(const bool newValue);
    void useCachedGrammarInParse(const bool newValue);
    void setRootElemName(XMLCh* rootElemName);
    void setExternalSchemaLocation(const XMLCh* const schemaLocation);
    void setExternalNoNamespaceSchemaLocation(const XMLCh* const noNamespaceSchemaLocation);
    void setExternalSchemaLocation(const char* const schemaLocation);
    void setExternalNoNamespaceSchemaLocation(const char* const noNamespaceSchemaLocation);
    void setSecurityManager(SecurityManager* const securityManager);
    void setLoadExternalDTD(const bool loadDTD);
    void setLoadSchema(const bool loadSchema);
    void setNormalizeData(const bool normalizeData);
    void setCalculateSrcOfs(const bool newValue);
    void setParseSettings(XMLScanner* const refScanner);
    void setStandardUriConformant(const bool newValue);
    void setInputBufferSize(const XMLSize_t bufferSize);
    void setLowWaterMark(XMLSize_t newValue);

    void setGenerateSyntheticAnnotations(const bool newValue);
    void setValidateAnnotations(const bool newValue);
    void setIgnoredCachedDTD(const bool newValue);
    void setIgnoreAnnotations(const bool newValue);
    void setDisableDefaultEntityResolution(const bool newValue);
    void setSkipDTDValidation(const bool newValue);
    void setHandleMultipleImports(const bool newValue);

    // -----------------------------------------------------------------------
    //  Mutator methods
    // -----------------------------------------------------------------------
    void incrementErrorCount(void);			// For use by XMLValidator

    // -----------------------------------------------------------------------
    //  Document scanning methods
    //
    //  scanDocument() does the entire source document. scanFirst(),
    //  scanNext(), and scanReset() support a progressive parse.
    // -----------------------------------------------------------------------
    void scanDocument
    (
        const   XMLCh* const    systemId
    );
    void scanDocument
    (
        const   char* const     systemId
    );

    bool scanFirst
    (
        const   InputSource&    src
        ,       XMLPScanToken&  toFill
    );
    bool scanFirst
    (
        const   XMLCh* const    systemId
        ,       XMLPScanToken&  toFill
    );
    bool scanFirst
    (
        const   char* const     systemId
        ,       XMLPScanToken&  toFill
    );

    void scanReset(XMLPScanToken& toFill);

    bool checkXMLDecl(bool startWithAngle);

    // -----------------------------------------------------------------------
    //  Grammar preparsing methods
    // -----------------------------------------------------------------------
    Grammar* loadGrammar
    (
        const   XMLCh* const    systemId
        , const short           grammarType
        , const bool            toCache = false
    );
    Grammar* loadGrammar
    (
        const   char* const     systemId
        , const short           grammarType
        , const bool            toCache = false
    );

    // -----------------------------------------------------------------------
    //  Helper methods
    // -----------------------------------------------------------------------
    unsigned int resolveQName
    (
        const   XMLCh* const        qName
        ,       XMLBuffer&          prefixBufToFill
        , const ElemStack::MapModes mode
        ,       int&                prefixColonPos
    );

protected:
    // -----------------------------------------------------------------------
    //  Protected pure virtual methods
    // -----------------------------------------------------------------------
    virtual void scanCDSection() = 0;
    virtual void scanCharData(XMLBuffer& toToUse) = 0;
    virtual EntityExpRes scanEntityRef
    (
        const   bool    inAttVal
        ,       XMLCh&  firstCh
        ,       XMLCh&  secondCh
        ,       bool&   escaped
    ) = 0;
    virtual void scanDocTypeDecl() = 0;
    virtual void scanReset(const InputSource& src) = 0;
    virtual void sendCharData(XMLBuffer& toSend) = 0;

    //return owned by the caller
    virtual InputSource* resolveSystemId(const XMLCh* const /*sysId*/
                                        ,const XMLCh* const /*pubId*/) {return 0;};

    // -----------------------------------------------------------------------
    //  Protected scanning methods
    // -----------------------------------------------------------------------
    bool scanCharRef(XMLCh& toFill, XMLCh& second);
    void scanComment();
    bool scanEq(bool inDecl = false);
    void scanMiscellaneous();
    void scanPI();
    void scanProlog();
    void scanXMLDecl(const DeclTypes type);

    // -----------------------------------------------------------------------
    //  Private helper methods
    // -----------------------------------------------------------------------
    void checkInternalDTD(bool hasExtSubset, const XMLCh* const sysId, const XMLCh* const pubId);
    void checkIDRefs();
    bool isLegalToken(const XMLPScanToken& toCheck);
    XMLTokens senseNextToken(XMLSize_t& orgReader);
    void initValidator(XMLValidator* theValidator);
    inline void resetValidationContext();
    unsigned int *getNewUIntPtr();
    void resetUIntPool();
    void recreateUIntPool();
    unsigned int resolvePrefix
    (
        const   XMLCh* const        prefix
        , const ElemStack::MapModes mode
    );
    unsigned int resolveQNameWithColon
    (
        const   XMLCh* const        qName
        ,       XMLBuffer&          prefixBufToFill
        , const ElemStack::MapModes mode
        , const int                 prefixColonPos
    );

    inline
    void setAttrDupChkRegistry
         (
            const XMLSize_t    &attrNumber
          ,       bool         &toUseHashTable
         );

    // -----------------------------------------------------------------------
    //  Data members
    //
    //  fBufferSize
    //      Maximum input buffer size
    //
    //  fLowWaterMark
    //      The low water mark for the raw byte buffer.
    //
    //  fAttrList
    //      Every time we get a new element start tag, we have to pass to
    //      the document handler the attributes found. To make it more
    //      efficient we keep this ref vector of XMLAttr objects around. We
    //      just reuse it over and over, allowing it to grow to meet the
    //      peak need.
    //
    //  fBufMgr
    //      This is a manager for temporary buffers used during scanning.
    //      For efficiency we must use a set of static buffers, but we have
    //      to insure that they are not incorrectly reused. So this manager
    //      provides the smarts to hand out buffers as required.
    //
    //  fDocHandler
    //      The client code's document handler. If zero, then no document
    //      handler callouts are done. We don't adopt it.
    //
    //  fDocTypeHandler
    //      The client code's document type handler (used by DTD Validator).
    //
    //  fDoNamespaces
    //      This flag indicates whether the client code wants us to do
    //      namespaces or not. If the installed validator indicates that it
    //      has to do namespaces, then this is ignored.
    //
    //  fEntityHandler
    //      The client code's entity handler. If zero, then no entity handler
    //      callouts are done. We don't adopt it.
    //
    //  fErrorReporter
    //      The client code's error reporter. If zero, then no error reporter
    //      callouts are done. We don't adopt it.
    //
    //  fErrorHandler
    //      The client code's error handler.  Need to store this info for
    //      Schema parse error handling.
    //
    //  fPSVIHandler
    //      The client code's PSVI handler.
    //
    //  fExitOnFirstFatal
    //      This indicates whether we bail out on the first fatal XML error
    //      or not. It defaults to true, which is the strict XML way, but it
    //      can be changed.
    //
    //  fValidationConstraintFatal
    //      This indicates whether we treat validation constraint errors as
    //      fatal errors or not. It defaults to false, but it can be changed.
    //
    //  fIDRefList
    //      This is a list of XMLRefInfo objects. This member lets us do all
    //      needed ID-IDREF balancing checks.
    //
    //  fInException
    //      To avoid a circular freakout when we catch an exception and emit
    //      it, which would normally throw again if the 'fail on first error'
    //      flag is one.
    //
    //  fReaderMgr
    //      This is the reader manager, from which we get characters. It
    //      manages the reader stack for us, and provides a lot of convenience
    //      methods to do specialized checking for chars, sequences of chars,
    //      skipping chars, etc...
    //
    //  fScannerId
    //  fSequenceId
    //      These are used for progressive parsing, to make sure that the
    //      client code does the right thing at the right time.
    //
    //  fStandalone
    //      Indicates whether the document is standalone or not. Defaults to
    //      no, but can be overridden in the XMLDecl.
    //
    //  fHasNoDTD
    //      Indicates the document has no DTD or has only an internal DTD subset
    //      which contains no parameter entity references.
    //
    //  fValidate
    //      Indicates whether any validation should be done. This is defined
    //      by the existence of a Grammar together with fValScheme.
    //
    //  fValidator
    //      The installed validator. We look at them via the abstract
    //      validator interface, and don't know what it actual is.
    //      Either point to user's installed validator, or fDTDValidator
    //      or fSchemaValidator.
    //
    //  fValidatorFromUser
    //      This flag indicates whether the validator was installed from
    //      user.  If false, then the validator was created by the Scanner.
    //
    //  fValScheme
    //      This is the currently set validation scheme. It defaults to
    //      'never', but can be set by the client.
    //
    //  fErrorCount
    //		The number of errors we've encountered.
    //
    //  fDoSchema
    //      This flag indicates whether the client code wants Schema to
    //      be processed or not.
    //
    //  fSchemaFullChecking
    //      This flag indicates whether the client code wants full Schema
    //      constraint checking.
    //
    //  fIdentityConstraintChecking
    //      This flag indicates whether the client code wants Identity
    //      Constraint checking, defaulted to true to maintain backward
    //      compatibility (to minimize supprise)
    //
    //  fAttName
    //  fAttValue
    //  fCDataBuf
    //  fNameBuf
    //  fQNameBuf
    //  fPrefixBuf
    //      For the most part, buffers are obtained from the fBufMgr object
    //      on the fly. However, for the start tag scan, we have a set of
    //      fixed buffers for performance reasons. These are used a lot and
    //      there are a number of them, so asking the buffer manager each
    //      time for new buffers is a bit too much overhead.
    //
    //  fEmptyNamespaceId
    //      This is the id of the empty namespace URI. This is a special one
    //      because of the xmlns="" type of deal. We have to quickly sense
    //      that its the empty namespace.
    //
    //  fUnknownNamespaceId
    //      This is the id of the namespace URI which is assigned to the
    //      global namespace. Its for debug purposes only, since there is no
    //      real global namespace URI. Its set by the derived class.
    //
    //  fXMLNamespaceId
    //  fXMLNSNamespaceId
    //      These are the ids of the namespace URIs which are assigned to the
    //      'xml' and 'xmlns' special prefixes. The former is officially
    //      defined but the latter is not, so we just provide one for debug
    //      purposes.
    //
    //  fSchemaNamespaceId
    //      This is the id of the schema namespace URI.
    //
    //  fGrammarResolver
    //      Grammar Pool that stores all the grammars. Key is namespace for
    //      schema and system id for external DTD. When caching a grammar, if
    //      a grammar is already in the pool, it will be replaced with the
    //      new parsed one.
    //
    //  fGrammar
    //      Current Grammar used by the Scanner and Validator
    //
    //  fRootGrammar
    //      The grammar where the root element is declared.
    //
    //  fGrammarType
    //      Current Grammar Type.  Store this value instead of calling getGrammarType
    //      all the time for faster performance.
    //
    //  fURIStringPool
    //      This is a pool for URIs with unique ids assigned. We use a standard
    //      string pool class.  This pool is going to be shared by all Grammar.
    //      Use only if namespace is turned on.
    //
    //  fRootElemName
    //      No matter we are using DTD or Schema Grammar, if a DOCTYPE exists,
    //      we need to verify the root element name.  So store the rootElement
    //      that is used in the DOCTYPE in the Scanner instead of in the DTDGrammar
    //      where it used to
    //
    //  fExternalSchemaLocation
    //      The list of Namespace/SchemaLocation that was specified externally
    //      using setExternalSchemaLocation.
    //
    //  fExternalNoNamespaceSchemaLocation
    //      The no target namespace XML Schema Location that was specified
    //      externally using setExternalNoNamespaceSchemaLocation.
    //
    //  fSecurityManager
    //      The SecurityManager instance; as and when set by the application.
    //
    //  fEntityExpansionLimit
    //      The number of entity expansions to be permitted while processing this document
    //      Only meaningful when fSecurityManager != 0
    //
    //  fEntityExpansionCount
    //      The number of general entities expanded so far in this document.
    //      Only meaningful when fSecurityManager != null
    //
    //  fLoadExternalDTD
    //      This flag indicates whether the external DTD be loaded or not
    //
    //  fLoadSchema
    //      This flag indicates whether the parser should attempt to load
    //      schemas if they cannot be found in the grammar pool.
    //
    //  fNormalizeData
    //      This flag indicates whether the parser should perform datatype
    //      normalization that is defined in the schema.
    //
    //  fCalculateSrcOfs
    //      This flag indicates the parser should calculate the source offset.
    //      Turning this on may impact performance.
    //
    //  fStandardUriConformant
    //      This flag controls whether we force conformant URI
    //
    //  fXMLVersion
    //      Enum to indicate if the main doc is XML 1.1 or XML 1.0 conformant
    //  fUIntPool
    //      pool of unsigned integers to help with duplicate attribute
    //      detection and filling in default/fixed attributes
    //  fUIntPoolRow
    //      current row in fUIntPool
    //  fUIntPoolCol
    //      current column in row
    //  fUIntPoolRowTotal
    //      total number of rows in table
    //
    //  fMemoryManager
    //      Pluggable memory manager for dynamic allocation/deallocation.
    //
    // -----------------------------------------------------------------------
    XMLSize_t                   fBufferSize;
    XMLSize_t                   fLowWaterMark;
    bool                        fStandardUriConformant;
    bool                        fCalculateSrcOfs;
    bool                        fDoNamespaces;
    bool                        fExitOnFirstFatal;
    bool                        fValidationConstraintFatal;
    bool                        fInException;
    bool                        fStandalone;
    bool                        fHasNoDTD;
    bool                        fValidate;
    bool                        fValidatorFromUser;
    bool                        fDoSchema;
    bool                        fSchemaFullChecking;
    bool                        fIdentityConstraintChecking;
    bool                        fToCacheGrammar;
    bool                        fUseCachedGrammar;
    bool                        fLoadExternalDTD;
    bool                        fLoadSchema;
    bool                        fNormalizeData;
    bool                        fGenerateSyntheticAnnotations;
    bool                        fValidateAnnotations;
    bool                        fIgnoreCachedDTD;
    bool                        fIgnoreAnnotations;
    bool                        fDisableDefaultEntityResolution;
    bool                        fSkipDTDValidation;
    bool                        fHandleMultipleImports;
    int                         fErrorCount;
    XMLSize_t                   fEntityExpansionLimit;
    XMLSize_t                   fEntityExpansionCount;
    unsigned int                fEmptyNamespaceId;
    unsigned int                fUnknownNamespaceId;
    unsigned int                fXMLNamespaceId;
    unsigned int                fXMLNSNamespaceId;
    unsigned int                fSchemaNamespaceId;
    unsigned int **             fUIntPool;
    unsigned int                fUIntPoolRow;
    unsigned int                fUIntPoolCol;
    unsigned int                fUIntPoolRowTotal;
    XMLUInt32                   fScannerId;
    XMLUInt32                   fSequenceId;
    RefVectorOf<XMLAttr>*       fAttrList;
    RefHash2KeysTableOf<XMLAttr>*  fAttrDupChkRegistry;
    XMLDocumentHandler*         fDocHandler;
    DocTypeHandler*             fDocTypeHandler;
    XMLEntityHandler*           fEntityHandler;
    XMLErrorReporter*           fErrorReporter;
    ErrorHandler*               fErrorHandler;
    PSVIHandler*                fPSVIHandler;
    ValidationContext           *fValidationContext;
    bool                        fEntityDeclPoolRetrieved;
    ReaderMgr                   fReaderMgr;
    XMLValidator*               fValidator;
    ValSchemes                  fValScheme;
    GrammarResolver* const      fGrammarResolver;
    MemoryManager* const        fGrammarPoolMemoryManager;
    Grammar*                    fGrammar;
    Grammar*                    fRootGrammar;
    XMLStringPool*              fURIStringPool;
    XMLCh*                      fRootElemName;
    XMLCh*                      fExternalSchemaLocation;
    XMLCh*                      fExternalNoNamespaceSchemaLocation;
    SecurityManager*            fSecurityManager;
    XMLReader::XMLVersion       fXMLVersion;
    MemoryManager*              fMemoryManager;
    XMLBufferMgr                fBufMgr;
    XMLBuffer                   fAttNameBuf;
    XMLBuffer                   fAttValueBuf;
    XMLBuffer                   fCDataBuf;
    XMLBuffer                   fQNameBuf;
    XMLBuffer                   fPrefixBuf;
    XMLBuffer                   fURIBuf;
    XMLBuffer                   fWSNormalizeBuf;
    ElemStack                   fElemStack;


private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XMLScanner();
    XMLScanner(const XMLScanner&);
    XMLScanner& operator=(const XMLScanner&);

    // -----------------------------------------------------------------------
    //  Private helper methods
    // -----------------------------------------------------------------------
    void commonInit();
    void cleanUp();

    // -----------------------------------------------------------------------
    //  Private scanning methods
    // -----------------------------------------------------------------------
    bool getQuotedString(XMLBuffer& toFill);
    XMLSize_t scanUpToWSOr
    (
                XMLBuffer&  toFill
        , const XMLCh       chEndChar
    );
};

// ---------------------------------------------------------------------------
//  XMLScanner: Getter methods
// ---------------------------------------------------------------------------
inline const XMLDocumentHandler* XMLScanner::getDocHandler() const
{
    return fDocHandler;
}

inline XMLDocumentHandler* XMLScanner::getDocHandler()
{
    return fDocHandler;
}

inline const DocTypeHandler* XMLScanner::getDocTypeHandler() const
{
    return fDocTypeHandler;
}

inline DocTypeHandler* XMLScanner::getDocTypeHandler()
{
    return fDocTypeHandler;
}

inline bool XMLScanner::getDoNamespaces() const
{
    return fDoNamespaces;
}

inline const XMLEntityHandler* XMLScanner::getEntityHandler() const
{
    return fEntityHandler;
}

inline XMLEntityHandler* XMLScanner::getEntityHandler()
{
    return fEntityHandler;
}

inline const XMLErrorReporter* XMLScanner::getErrorReporter() const
{
    return fErrorReporter;
}

inline XMLErrorReporter* XMLScanner::getErrorReporter()
{
    return fErrorReporter;
}

inline const ErrorHandler* XMLScanner::getErrorHandler() const
{
    return fErrorHandler;
}

inline ErrorHandler* XMLScanner::getErrorHandler()
{
    return fErrorHandler;
}

inline const PSVIHandler* XMLScanner::getPSVIHandler() const
{
    return fPSVIHandler;
}

inline PSVIHandler* XMLScanner::getPSVIHandler()
{
    return fPSVIHandler;
}

inline bool XMLScanner::getExitOnFirstFatal() const
{
    return fExitOnFirstFatal;
}

inline bool XMLScanner::getValidationConstraintFatal() const
{
    return fValidationConstraintFatal;
}

inline bool XMLScanner::getInException() const
{
    return fInException;
}

inline RefHashTableOf<XMLRefInfo>* XMLScanner::getIDRefList()
{
    return fValidationContext->getIdRefList();
}

inline const RefHashTableOf<XMLRefInfo>* XMLScanner::getIDRefList() const
{
    return fValidationContext->getIdRefList();
}

inline ValidationContext*  XMLScanner::getValidationContext()
{
    if (!fEntityDeclPoolRetrieved)
    {
        fValidationContext->setEntityDeclPool(getEntityDeclPool());
        fEntityDeclPoolRetrieved = true;
    }

    return fValidationContext;
}

inline const Locator* XMLScanner::getLocator() const
{
    return &fReaderMgr;
}

inline const ReaderMgr* XMLScanner::getReaderMgr() const
{
    return &fReaderMgr;
}

inline XMLFilePos XMLScanner::getSrcOffset() const
{
    return fReaderMgr.getSrcOffset();
}

inline bool XMLScanner::getStandalone() const
{
    return fStandalone;
}

inline XMLScanner::ValSchemes XMLScanner::getValidationScheme() const
{
    return fValScheme;
}

inline const XMLValidator* XMLScanner::getValidator() const
{
    return fValidator;
}

inline XMLValidator* XMLScanner::getValidator()
{
    return fValidator;
}

inline bool XMLScanner::getDoSchema() const
{
    return fDoSchema;
}

inline bool XMLScanner::getValidationSchemaFullChecking() const
{
    return fSchemaFullChecking;
}

inline bool XMLScanner::getIdentityConstraintChecking() const
{
    return fIdentityConstraintChecking;
}

inline int XMLScanner::getErrorCount()
{
    return fErrorCount;
}

inline bool XMLScanner::isValidatorFromUser()
{
    return fValidatorFromUser;
}

inline unsigned int XMLScanner::getEmptyNamespaceId() const
{
    return fEmptyNamespaceId;
}

inline unsigned int XMLScanner::getUnknownNamespaceId() const
{
    return fUnknownNamespaceId;
}

inline unsigned int XMLScanner::getXMLNamespaceId() const
{
    return fXMLNamespaceId;
}

inline unsigned int XMLScanner::getXMLNSNamespaceId() const
{
    return fXMLNSNamespaceId;
}

inline const XMLStringPool* XMLScanner::getURIStringPool() const
{
    return fURIStringPool;
}

inline XMLStringPool* XMLScanner::getURIStringPool()
{
    return fURIStringPool;
}

inline bool XMLScanner::getHasNoDTD() const
{
    return fHasNoDTD;
}

inline XMLCh* XMLScanner::getExternalSchemaLocation() const
{
    return fExternalSchemaLocation;
}

inline XMLCh* XMLScanner::getExternalNoNamespaceSchemaLocation() const
{
    return fExternalNoNamespaceSchemaLocation;
}

inline SecurityManager* XMLScanner::getSecurityManager() const
{
    return fSecurityManager;
}

inline bool XMLScanner::getLoadExternalDTD() const
{
    return fLoadExternalDTD;
}

inline bool XMLScanner::getLoadSchema() const
{
    return fLoadSchema;
}

inline bool XMLScanner::getNormalizeData() const
{
    return fNormalizeData;
}

inline bool XMLScanner::isCachingGrammarFromParse() const
{
    return fToCacheGrammar;
}

inline bool XMLScanner::isUsingCachedGrammarInParse() const
{
    return fUseCachedGrammar;
}

inline bool XMLScanner::getCalculateSrcOfs() const
{
    return fCalculateSrcOfs;
}

inline Grammar* XMLScanner::getRootGrammar() const
{
    return fRootGrammar;
}

inline bool XMLScanner::getStandardUriConformant() const
{
    return fStandardUriConformant;
}

inline XMLReader::XMLVersion XMLScanner::getXMLVersion() const
{
	return fXMLVersion;
}

inline MemoryManager* XMLScanner::getMemoryManager() const
{
    return fMemoryManager;
}

inline ValueVectorOf<PrefMapElem*>* XMLScanner::getNamespaceContext() const
{
    return fElemStack.getNamespaceMap();
}

inline unsigned int XMLScanner::getPrefixId(const XMLCh* const prefix) const
{
    return fElemStack.getPrefixId(prefix);
}

inline const XMLCh* XMLScanner::getPrefixForId(unsigned int prefId) const
{
    return fElemStack.getPrefixForId(prefId);
}

inline bool XMLScanner::getGenerateSyntheticAnnotations() const
{
    return fGenerateSyntheticAnnotations;
}

inline bool XMLScanner::getValidateAnnotations() const
{
    return fValidateAnnotations;
}

inline const XMLSize_t& XMLScanner::getLowWaterMark() const
{
    return fLowWaterMark;
}

inline bool XMLScanner::getIgnoreCachedDTD() const
{
    return fIgnoreCachedDTD;
}

inline bool XMLScanner::getIgnoreAnnotations() const
{
    return fIgnoreAnnotations;
}

inline bool XMLScanner::getDisableDefaultEntityResolution() const
{
    return fDisableDefaultEntityResolution;
}

inline bool XMLScanner::getSkipDTDValidation() const
{
    return fSkipDTDValidation;
}

inline bool XMLScanner::getHandleMultipleImports() const
{
    return fHandleMultipleImports;
}

// ---------------------------------------------------------------------------
//  XMLScanner: Setter methods
// ---------------------------------------------------------------------------
inline void XMLScanner::addGlobalPrefix(const XMLCh* const prefix, const unsigned int uriId)
{
    fElemStack.addGlobalPrefix(prefix, uriId);
}

inline void XMLScanner::setDocHandler(XMLDocumentHandler* const docHandler)
{
    fDocHandler = docHandler;
}

inline void XMLScanner::setDocTypeHandler(DocTypeHandler* const docTypeHandler)
{
    fDocTypeHandler = docTypeHandler;
}

inline void XMLScanner::setErrorHandler(ErrorHandler* const handler)
{
    fErrorHandler = handler;
}

inline void XMLScanner::setPSVIHandler(PSVIHandler* const handler)
{
    fPSVIHandler = handler;
}

inline void XMLScanner::setEntityHandler(XMLEntityHandler* const entityHandler)
{
    fEntityHandler = entityHandler;
    fReaderMgr.setEntityHandler(entityHandler);
}

inline void XMLScanner::setErrorReporter(XMLErrorReporter* const errHandler)
{
    fErrorReporter = errHandler;
}

inline void XMLScanner::setExitOnFirstFatal(const bool newValue)
{
    fExitOnFirstFatal = newValue;
}


inline void XMLScanner::setValidationConstraintFatal(const bool newValue)
{
    fValidationConstraintFatal = newValue;
}

inline void XMLScanner::setValidationScheme(const ValSchemes newScheme)
{
    fValScheme = newScheme;

    // validation flag for Val_Auto is set to false by default,
    //   and will be turned to true if a grammar is seen
    if (fValScheme == Val_Always)
        fValidate = true;
    else
        fValidate = false;
}

inline void XMLScanner::setDoSchema(const bool doSchema)
{
    fDoSchema = doSchema;
}

inline void XMLScanner::setDoNamespaces(const bool doNamespaces)
{
    fDoNamespaces = doNamespaces;
}

inline void XMLScanner::setValidationSchemaFullChecking(const bool schemaFullChecking)
{
    fSchemaFullChecking = schemaFullChecking;
}

inline void XMLScanner::setIdentityConstraintChecking(const bool identityConstraintChecking)
{
    fIdentityConstraintChecking = identityConstraintChecking;
}

inline void XMLScanner::setHasNoDTD(const bool hasNoDTD)
{
    fHasNoDTD = hasNoDTD;
}

inline void XMLScanner::setRootElemName(XMLCh* rootElemName)
{
    fMemoryManager->deallocate(fRootElemName);//delete [] fRootElemName;
    fRootElemName = XMLString::replicate(rootElemName, fMemoryManager);
}

inline void XMLScanner::setExternalSchemaLocation(const XMLCh* const schemaLocation)
{
    fMemoryManager->deallocate(fExternalSchemaLocation);//delete [] fExternalSchemaLocation;
    fExternalSchemaLocation = XMLString::replicate(schemaLocation, fMemoryManager);
}

inline void XMLScanner::setExternalNoNamespaceSchemaLocation(const XMLCh* const noNamespaceSchemaLocation)
{
    fMemoryManager->deallocate(fExternalNoNamespaceSchemaLocation);//delete [] fExternalNoNamespaceSchemaLocation;
    fExternalNoNamespaceSchemaLocation = XMLString::replicate(noNamespaceSchemaLocation, fMemoryManager);
}

inline void XMLScanner::setExternalSchemaLocation(const char* const schemaLocation)
{
    fMemoryManager->deallocate(fExternalSchemaLocation);//delete [] fExternalSchemaLocation;
    fExternalSchemaLocation = XMLString::transcode(schemaLocation, fMemoryManager);
}

inline void XMLScanner::setExternalNoNamespaceSchemaLocation(const char* const noNamespaceSchemaLocation)
{
    fMemoryManager->deallocate(fExternalNoNamespaceSchemaLocation);//delete [] fExternalNoNamespaceSchemaLocation;
    fExternalNoNamespaceSchemaLocation = XMLString::transcode(noNamespaceSchemaLocation, fMemoryManager);
}

inline void XMLScanner::setSecurityManager(SecurityManager* const securityManager)
{
    fSecurityManager = securityManager;
    if(securityManager != 0)
    {
        fEntityExpansionLimit = securityManager->getEntityExpansionLimit();
        fEntityExpansionCount = 0;
    }
}

inline void XMLScanner::setLoadExternalDTD(const bool loadDTD)
{
    fLoadExternalDTD = loadDTD;
}

inline void XMLScanner::setLoadSchema(const bool loadSchema)
{
    fLoadSchema = loadSchema;
}

inline void XMLScanner::setNormalizeData(const bool normalizeData)
{
    fNormalizeData = normalizeData;
}

inline void XMLScanner::cacheGrammarFromParse(const bool newValue)
{
    fToCacheGrammar = newValue;
}

inline void XMLScanner::useCachedGrammarInParse(const bool newValue)
{
    fUseCachedGrammar = newValue;
}

inline void XMLScanner::setCalculateSrcOfs(const bool newValue)
{
    fCalculateSrcOfs = newValue;
}

inline void XMLScanner::setStandardUriConformant(const bool newValue)
{
    fStandardUriConformant = newValue;
    fReaderMgr.setStandardUriConformant(newValue);
}

inline void XMLScanner::setGenerateSyntheticAnnotations(const bool newValue)
{
    fGenerateSyntheticAnnotations = newValue;
}

inline void XMLScanner::setValidateAnnotations(const bool newValue)
{
    fValidateAnnotations = newValue;
}

inline void XMLScanner::setInputBufferSize(const XMLSize_t bufferSize)
{
    fBufferSize = bufferSize;
    fCDataBuf.setFullHandler(this, fBufferSize);
}

inline void XMLScanner::setLowWaterMark(XMLSize_t newValue)
{
    fLowWaterMark = newValue;
}

inline void XMLScanner::setIgnoredCachedDTD(const bool newValue)
{
    fIgnoreCachedDTD = newValue;
}

inline void XMLScanner::setIgnoreAnnotations(const bool newValue)
{
    fIgnoreAnnotations = newValue;
}

inline void XMLScanner::setDisableDefaultEntityResolution(const bool newValue)
{
    fDisableDefaultEntityResolution = newValue;
}

inline void XMLScanner::setSkipDTDValidation(const bool newValue)
{
    fSkipDTDValidation = newValue;
}

inline void XMLScanner::setHandleMultipleImports(const bool newValue)
{
    fHandleMultipleImports = newValue;
}

// ---------------------------------------------------------------------------
//  XMLScanner: Mutator methods
// ---------------------------------------------------------------------------
inline void XMLScanner::incrementErrorCount()
{
    ++fErrorCount;
}

inline void XMLScanner::resetValidationContext()
{
    fValidationContext->clearIdRefList();
    fValidationContext->setEntityDeclPool(0);
    fEntityDeclPoolRetrieved = false;
}

inline void XMLScanner::setAttrDupChkRegistry(const XMLSize_t    &attrNumber
                                            ,       bool         &toUseHashTable)
{
   // once the attribute exceed 100, we use hash table to check duplication
    if (attrNumber > 100)
   {
        toUseHashTable = true;

        if (!fAttrDupChkRegistry)
        {
            fAttrDupChkRegistry = new (fMemoryManager) RefHash2KeysTableOf<XMLAttr>
            (
              2*attrNumber+1, false, fMemoryManager
            );
        }
        else
        {
            fAttrDupChkRegistry->removeAll();
        }
    }

}

inline Grammar::GrammarType XMLScanner::getCurrentGrammarType() const
{
    return Grammar::UnKnown;
}

XERCES_CPP_NAMESPACE_END

#endif
