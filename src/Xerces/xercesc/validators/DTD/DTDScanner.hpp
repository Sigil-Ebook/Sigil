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
 * $Id: DTDScanner.hpp 676911 2008-07-15 13:27:32Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DTDSCANNER_HPP)
#define XERCESC_INCLUDE_GUARD_DTDSCANNER_HPP

#include <xercesc/validators/DTD/DTDGrammar.hpp>
#include <xercesc/validators/DTD/DTDEntityDecl.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XMLScanner;

/*
 * Default implementation of an XML DTD scanner.
 */
class DocTypeHandler;

class VALIDATORS_EXPORT DTDScanner : public XMemory
{
public:
    // -----------------------------------------------------------------------
    //  Class specific types
    //
    //  EntityExpRes
    //      Returned from scanEntityRef() to indicate how the expanded text
    //      was treated.
    //
    //  IDTypes
    //      Type of the ID
    // -----------------------------------------------------------------------
    enum EntityExpRes
    {
        EntityExp_Failed
        , EntityExp_Pushed
        , EntityExp_Returned
    };

    enum IDTypes
    {
        IDType_Public
        , IDType_External
        , IDType_Either
    };



    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    DTDScanner
    (
          DTDGrammar*           dtdGrammar
        , DocTypeHandler* const docTypeHandler
        , MemoryManager* const  grammarPoolMemoryManager
        , MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
    );
    virtual ~DTDScanner();

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    DocTypeHandler* getDocTypeHandler();
    const DocTypeHandler* getDocTypeHandler() const;

    // -----------------------------------------------------------------------
    //  Setter methods
    //
    //  setScannerInfo() is called by the scanner to tell the DTDScanner
    //  about the stuff it needs to have access to.
    // -----------------------------------------------------------------------
    void setScannerInfo
    (
        XMLScanner* const           owningScanner
        , ReaderMgr* const          readerMgr
        , XMLBufferMgr* const       bufMgr
    );

    void setDocTypeHandler
    (
            DocTypeHandler* const handlerToSet
    );

    void scanExtSubsetDecl(const bool inIncludeSect, const bool isDTD);
    bool scanInternalSubset();
    bool scanId
    (
                XMLBuffer&  pubIdToFill
        ,       XMLBuffer&  sysIdToFill
        , const IDTypes     whatKind
    );

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    DTDScanner(const DTDScanner &);
    DTDScanner& operator = (const  DTDScanner&);

    // -----------------------------------------------------------------------
    //  Private DTD scanning methods. These are all in XMLValidator2.cpp
    // -----------------------------------------------------------------------
    bool checkForPERef
    (
          const bool    inLiteral
        , const bool    inMarkup
    );
    bool expandPERef
    (
        const   bool    scanExternal
        , const bool    inLiteral
        , const bool    inMarkup
        , const bool    throwEndOfExt = false
    );
    bool getQuotedString(XMLBuffer& toFill);
    XMLAttDef* scanAttDef(DTDElementDecl& elemDecl, XMLBuffer& bufToUse);
    bool scanAttValue
    (
        const   XMLCh* const        attrName
        ,       XMLBuffer&          toFill
        , const XMLAttDef::AttTypes type
    );
    void scanAttListDecl();
    ContentSpecNode* scanChildren
    (
        const   DTDElementDecl&     elemDecl
        ,       XMLBuffer&          bufToUse
    );
    bool scanCharRef(XMLCh& toFill, XMLCh& second);
    void scanComment();
    bool scanContentSpec(DTDElementDecl& toFill);
    void scanDefaultDecl(DTDAttDef& toFill);
    void scanElementDecl();
    void scanEntityDecl();
    bool scanEntityDef();
    bool scanEntityLiteral(XMLBuffer& toFill);
    bool scanEntityDef(DTDEntityDecl& decl, const bool isPEDecl);
    EntityExpRes scanEntityRef(XMLCh& firstCh, XMLCh& secondCh, bool& escaped);
    bool scanEnumeration
    (
        const   DTDAttDef&  attDef
        ,       XMLBuffer&  toFill
        , const bool        notation
    );
    bool scanEq();
    void scanIgnoredSection();
    void scanMarkupDecl(const bool parseTextDecl);
    bool scanMixed(DTDElementDecl& toFill);
    void scanNotationDecl();
    void scanPI();
    bool scanPublicLiteral(XMLBuffer& toFill);
    bool scanSystemLiteral(XMLBuffer& toFill);
    void scanTextDecl();
    bool isReadingExternalEntity();


    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fDocTypeHandler
    //      This holds the optional doc type handler that can be installed
    //      and used to call back for all markup events. It is DTD specific.
    //
    //  fDumAttDef
    //  fDumElemDecl
    //  fDumEntityDecl
    //      These are dummy objects into which mark decls are parsed when
    //      they are just overrides of previously declared markup decls. In
    //      such situations, the first one wins but we need to have somewhere
    //      to parse them into. So these are lazily created and used as needed
    //      when such markup decls are seen.
    //
    //  fInternalSubset
    //      This is used to track whether we are in the internal subset or not,
    //      in which case we are in the external subset.
    //
    //  fNextAttrId
    //      Since att defs are per-element, we don't have a validator wide
    //      attribute def pool. So we use a simpler data structure in each
    //      element decl to store its att defs, and we use this simple counter
    //      to apply a unique id to each new attribute.
    //
    //  fDTDGrammar
    //      The DTD information we scanned like element decl, attribute decl
    //      are stored in this Grammar.
    //
    //  fBufMgr
    //      This is the buffer manager of the scanner. This is provided as a
    //      convenience so that the DTDScanner doesn't have to create its own
    //      buffer manager during the parse process.
    //
    //  fReaderMgr
    //      This is a pointer to the reader manager that is being used by the scanner.
    //
    //  fScanner
    //      The pointer to the scanner to which this DTDScanner belongs
    //
    //  fPEntityDeclPool
    //      This is a pool of EntityDecl objects, which contains all of the
    //      parameter entities that are declared in the DTD subsets.
    //
    //  fEmptyNamespaceId
    //      The uri for all DTD decls
    //
    //  fDocTypeReaderId
    //      The original reader in the fReaderMgr - to be compared against the
    //      current reader to decide whether we are processing an external/internal
    //      declaration
    // -----------------------------------------------------------------------
    MemoryManager*                  fMemoryManager;
    MemoryManager*                  fGrammarPoolMemoryManager;
    DocTypeHandler*                 fDocTypeHandler;
    DTDAttDef*                      fDumAttDef;
    DTDElementDecl*                 fDumElemDecl;
    DTDEntityDecl*                  fDumEntityDecl;
    bool                            fInternalSubset;
    unsigned int                    fNextAttrId;
    DTDGrammar*                     fDTDGrammar;
    XMLBufferMgr*                   fBufMgr;
    ReaderMgr*                      fReaderMgr;
    XMLScanner*                     fScanner;
    NameIdPool<DTDEntityDecl>*      fPEntityDeclPool;
    unsigned int                    fEmptyNamespaceId;
    XMLSize_t                       fDocTypeReaderId;
};


// ---------------------------------------------------------------------------
//  DTDScanner: Getter methods
// ---------------------------------------------------------------------------
inline DocTypeHandler* DTDScanner::getDocTypeHandler()
{
    return fDocTypeHandler;
}

inline const DocTypeHandler* DTDScanner::getDocTypeHandler() const
{
    return fDocTypeHandler;
}


// ---------------------------------------------------------------------------
//  DTDScanner: Setter methods
// ---------------------------------------------------------------------------
inline void DTDScanner::setDocTypeHandler(DocTypeHandler* const handlerToSet)
{
    fDocTypeHandler = handlerToSet;
}

// -----------------------------------------------------------------------
//  Helper methods
// -----------------------------------------------------------------------
inline bool DTDScanner::isReadingExternalEntity() {
    return (fDocTypeReaderId != fReaderMgr->getCurrentReaderNum());
}

XERCES_CPP_NAMESPACE_END

#endif
