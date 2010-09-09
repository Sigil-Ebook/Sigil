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
 * $Id: DGXMLScanner.hpp 810580 2009-09-02 15:52:22Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DGXMLSCANNER_HPP)
#define XERCESC_INCLUDE_GUARD_DGXMLSCANNER_HPP

#include <xercesc/internal/XMLScanner.hpp>
#include <xercesc/util/ValueVectorOf.hpp>
#include <xercesc/util/NameIdPool.hpp>
#include <xercesc/util/Hash2KeysSetOf.hpp>
#include <xercesc/validators/common/Grammar.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class DTDElementDecl;
class DTDGrammar;
class DTDValidator;

//  This is an integrated scanner class, which does DTD/XML Schema grammar
//  processing.
class XMLPARSER_EXPORT DGXMLScanner : public XMLScanner
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    DGXMLScanner
    (
          XMLValidator* const  valToAdopt
        , GrammarResolver* const grammarResolver
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );
    DGXMLScanner
    (
          XMLDocumentHandler* const docHandler
        , DocTypeHandler* const     docTypeHandler
        , XMLEntityHandler* const   entityHandler
        , XMLErrorReporter* const   errReporter
        , XMLValidator* const       valToAdopt
        , GrammarResolver* const    grammarResolver
        , MemoryManager* const      manager = XMLPlatformUtils::fgMemoryManager
    );
    virtual ~DGXMLScanner();

    // -----------------------------------------------------------------------
    //  XMLScanner public virtual methods
    // -----------------------------------------------------------------------
    virtual const XMLCh* getName() const;
    virtual NameIdPool<DTDEntityDecl>* getEntityDeclPool();
    virtual const NameIdPool<DTDEntityDecl>* getEntityDeclPool() const;
    virtual void scanDocument
    (
        const   InputSource&    src
    );
    virtual bool scanNext(XMLPScanToken& toFill);
    virtual Grammar* loadGrammar
    (
        const   InputSource&    src
        , const short           grammarType
        , const bool            toCache = false
    );

    virtual Grammar::GrammarType getCurrentGrammarType() const;

private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    DGXMLScanner();
    DGXMLScanner(const DGXMLScanner&);
    DGXMLScanner& operator=(const DGXMLScanner&);

    // -----------------------------------------------------------------------
    //  XMLScanner virtual methods
    // -----------------------------------------------------------------------
    virtual void scanCDSection();
    virtual void scanCharData(XMLBuffer& toToUse);
    virtual EntityExpRes scanEntityRef
    (
        const   bool    inAttVal
        ,       XMLCh&  firstCh
        ,       XMLCh&  secondCh
        ,       bool&   escaped
    );
    virtual void scanDocTypeDecl();
    virtual void scanReset(const InputSource& src);
    virtual void sendCharData(XMLBuffer& toSend);
    virtual InputSource* resolveSystemId(const XMLCh* const sysId
                                        ,const XMLCh* const pubId);

    // -----------------------------------------------------------------------
    //  Private helper methods
    // -----------------------------------------------------------------------
    void commonInit();
    void cleanUp();

    XMLSize_t buildAttList
    (
        const XMLSize_t                     attCount
        ,       XMLElementDecl*             elemDecl
        ,       RefVectorOf<XMLAttr>&       toFill
    );
    void updateNSMap
    (
        const   XMLCh* const attrPrefix
        , const XMLCh* const attrLocalName
        , const XMLCh* const attrValue
    );
    void scanAttrListforNameSpaces(RefVectorOf<XMLAttr>* theAttrList, XMLSize_t attCount, XMLElementDecl* elemDecl);

    // -----------------------------------------------------------------------
    //  Private scanning methods
    // -----------------------------------------------------------------------
    bool scanAttValue
    (
        const   XMLAttDef* const    attDef
        , const XMLCh *const        attrName
        ,       XMLBuffer&          toFill
    );
    bool scanContent();
    void scanEndTag(bool& gotData);
    bool scanStartTag(bool& gotData);
    bool scanStartTagNS(bool& gotData);

    // -----------------------------------------------------------------------
    //  Grammar preparsing methods
    // -----------------------------------------------------------------------
    Grammar* loadDTDGrammar(const InputSource& src, const bool toCache = false);

    // -----------------------------------------------------------------------
    //  Data members
    //
    //  fRawAttrList
    //      During the initial scan of the attributes we can only do a raw
    //      scan for key/value pairs. So this vector is used to store them
    //      until they can be processed (and put into fAttrList.)
    //
    //  fDTDValidator
    //      The DTD validator instance.
    //
    // fDTDElemNonDeclPool
    //     registry of "faulted-in" DTD element decls
    // fElemCount
    //      count of the number of start tags seen so far (starts at 1).
    //      Used for duplicate attribute detection/processing of required/defaulted attributes
    // fAttDefRegistry
    //      mapping from XMLAttDef instances to the count of the last
    //      start tag where they were utilized.
    // fUndeclaredAttrRegistry
    //      mapping of attr QNames to detect duplicates
    //
    // -----------------------------------------------------------------------
    ValueVectorOf<XMLAttr*>*    fAttrNSList;
    DTDValidator*               fDTDValidator;
    DTDGrammar*                 fDTDGrammar;
    NameIdPool<DTDElementDecl>* fDTDElemNonDeclPool;
    unsigned int                fElemCount;
    RefHashTableOf<unsigned int, PtrHasher>* fAttDefRegistry;
    Hash2KeysSetOf<StringHasher>*            fUndeclaredAttrRegistry;
};

inline const XMLCh* DGXMLScanner::getName() const
{
    return XMLUni::fgDGXMLScanner;
}

inline Grammar::GrammarType DGXMLScanner::getCurrentGrammarType() const
{
    return Grammar::DTDGrammarType;
}

XERCES_CPP_NAMESPACE_END

#endif
