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
 * $Id: DOMImplementationImpl.cpp 671894 2008-06-26 13:29:21Z borisk $
 */

#include "DOMImplementationImpl.hpp"
#include "DOMDocumentImpl.hpp"
#include "DOMDocumentTypeImpl.hpp"
#include "DOMLSSerializerImpl.hpp"
#include "DOMLSInputImpl.hpp"
#include "DOMLSOutputImpl.hpp"
#include "DOMImplementationListImpl.hpp"

#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMDocumentType.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLInitializer.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/XMLChar.hpp>
#include <xercesc/util/XMLStringTokenizer.hpp>
#include <xercesc/util/XMLDOMMsg.hpp>
#include <xercesc/util/XMLMsgLoader.hpp>
#include <xercesc/parsers/DOMLSParserImpl.hpp>

XERCES_CPP_NAMESPACE_BEGIN


// ------------------------------------------------------------
//  Static constants
// ------------------------------------------------------------
static const XMLCh  g1_0[] =     // Points to "1.0"
        {chDigit_1, chPeriod, chDigit_0, chNull};
static const XMLCh  g2_0[] =      // Points to "2.0"
        {chDigit_2, chPeriod, chDigit_0, chNull};
static const XMLCh  g3_0[] =      // Points to "3.0"
        {chDigit_3, chPeriod, chDigit_0, chNull};
static const XMLCh  gTrav[] =     // Points to "Traversal"
        {chLatin_T, chLatin_r, chLatin_a, chLatin_v, chLatin_e, chLatin_r,
            chLatin_s, chLatin_a, chLatin_l, chNull};
static const XMLCh  gCore[] =     // Points to "Core"
        {chLatin_C, chLatin_o, chLatin_r, chLatin_e, chNull};
static const XMLCh  gRange[] =     // Points to "Range"
        {chLatin_R, chLatin_a, chLatin_n, chLatin_g, chLatin_e, chNull};
static const XMLCh  gLS[] =     // Points to "LS"
        {chLatin_L, chLatin_S, chNull};
static const XMLCh  gXPath[] =     // Points to "XPath"
        {chLatin_X, chLatin_P, chLatin_a, chLatin_t, chLatin_h, chNull};


static XMLMsgLoader *sMsgLoader = 0;
static DOMImplementationImpl *gDomimp = 0;

void XMLInitializer::initializeDOMImplementationImpl()
{
    sMsgLoader = XMLPlatformUtils::loadMsgSet(XMLUni::fgXMLDOMMsgDomain);

    if (!sMsgLoader)
      XMLPlatformUtils::panic(PanicHandler::Panic_CantLoadMsgDomain);

    gDomimp = new DOMImplementationImpl;
}

void XMLInitializer::terminateDOMImplementationImpl()
{
    delete gDomimp;
    gDomimp = 0;

    delete sMsgLoader;
    sMsgLoader = 0;
}

//
//
XMLMsgLoader* DOMImplementationImpl::getMsgLoader4DOM()
{
    return sMsgLoader;
}

DOMImplementationImpl *DOMImplementationImpl::getDOMImplementationImpl()
{
    return gDomimp;
}

// ------------------------------------------------------------
// DOMImplementation Virtual interface
// ------------------------------------------------------------
bool  DOMImplementationImpl::hasFeature(const  XMLCh * feature,  const  XMLCh * version) const
{
    if (!feature)
        return false;

    // ignore the + modifier
    if(*feature==chPlus)
        feature++;

    bool anyVersion = (version == 0 || !*version);
    bool version1_0 = XMLString::equals(version, g1_0);
    bool version2_0 = XMLString::equals(version, g2_0);
    bool version3_0 = XMLString::equals(version, g3_0);

    // Currently, we support only XML Level 1 version 1.0
    if (XMLString::compareIStringASCII(feature, XMLUni::fgXMLString) == 0
        && (anyVersion || version1_0 || version2_0))
        return true;

    if (XMLString::compareIStringASCII(feature, gCore) == 0
        && (anyVersion || version1_0 || version2_0 || version3_0))
        return true;

    if (XMLString::compareIStringASCII(feature, gTrav) == 0
        && (anyVersion || version2_0))
        return true;

    if (XMLString::compareIStringASCII(feature, gRange) == 0
        && (anyVersion || version2_0))
        return true;

    if (XMLString::compareIStringASCII(feature, gLS) == 0
        && (anyVersion || version3_0))
        return true;

    if (XMLString::compareIStringASCII(feature, gXPath) == 0
        && (anyVersion || version3_0))
        return true;

    return false;
}


//Introduced in DOM Level 2
DOMDocumentType *DOMImplementationImpl::createDocumentType(const XMLCh *qualifiedName,
	const XMLCh * publicId, const XMLCh *systemId)
{
    // assume XML 1.0 since we do not know its version yet.
    if(!XMLChar1_0::isValidName(qualifiedName))
        throw DOMException(DOMException::INVALID_CHARACTER_ERR, 0);

    //to do: do we need to create with user's memorymanager???
    DOMDocumentTypeImpl* docType = new DOMDocumentTypeImpl(0, qualifiedName, publicId, systemId, true);
    return docType;
}

DOMDocument *DOMImplementationImpl::createDocument(const XMLCh *namespaceURI,
	const XMLCh *qualifiedName, DOMDocumentType *doctype,
    MemoryManager* const manager)
{
    return new (manager) DOMDocumentImpl(namespaceURI, qualifiedName, doctype, this, manager);
}


//Introduced in DOM Level 3
void* DOMImplementationImpl::getFeature(const XMLCh*, const XMLCh*) const {
    return 0;
}

// Non-standard extension
DOMDocument *DOMImplementationImpl::createDocument(MemoryManager* const manager)
{
        return new (manager) DOMDocumentImpl(this, manager);
}

//
//  DOMImplementation::getImplementation.  DOMImplementation is supposed to
//                                              be a pure interface class.  This one static
//                                              function is the hook that lets things get started.
DOMImplementation *DOMImplementation::getImplementation()
{
    return (DOMImplementation*) DOMImplementationImpl::getDOMImplementationImpl();
}

bool DOMImplementation::loadDOMExceptionMsg
(
      const short                        msgToLoad
    ,       XMLCh* const                 toFill
    , const XMLSize_t                    maxChars
)
{
  // Figure out which exception range this code is and load the corresponding
  // message.
  //
  if (msgToLoad <= 50)
  {
    // DOMException
    return sMsgLoader->loadMsg(XMLDOMMsg::DOMEXCEPTION_ERRX+msgToLoad, toFill, maxChars);
  }
  else if (msgToLoad <= 80)
  {
    // DOMXPathException
    return sMsgLoader->loadMsg(XMLDOMMsg::DOMXPATHEXCEPTION_ERRX+msgToLoad-DOMXPathException::INVALID_EXPRESSION_ERR+1, toFill, maxChars);
  }
  else if (msgToLoad <= 110)
  {
    // DOMXLSException
    return sMsgLoader->loadMsg(XMLDOMMsg::DOMLSEXCEPTION_ERRX+msgToLoad-DOMLSException::PARSE_ERR+1, toFill, maxChars);
  }
  else
  {
    // DOMRangeException
    return sMsgLoader->loadMsg(XMLDOMMsg::DOMRANGEEXCEPTION_ERRX+msgToLoad-DOMRangeException::BAD_BOUNDARYPOINTS_ERR+1, toFill, maxChars);
  }
}

// ------------------------------------------------------------
// DOMImplementationLS Virtual interface
// ------------------------------------------------------------
//Introduced in DOM Level 3
DOMLSParser* DOMImplementationImpl::createLSParser( const DOMImplementationLSMode   mode,
                                                    const XMLCh* const     /*schemaType*/,
                                                    MemoryManager* const  manager,
                                                    XMLGrammarPool* const gramPool)
{
    if (mode == DOMImplementationLS::MODE_ASYNCHRONOUS)
        throw DOMException(DOMException::NOT_SUPPORTED_ERR, 0, manager);

    // TODO: schemaType
    return new (manager) DOMLSParserImpl(0, manager, gramPool);
}


DOMLSSerializer* DOMImplementationImpl::createLSSerializer(MemoryManager* const manager)
{
    return new (manager) DOMLSSerializerImpl(manager);
}

DOMLSInput* DOMImplementationImpl::createLSInput(MemoryManager* const manager)
{
    return new (manager) DOMLSInputImpl(manager);
}

DOMLSOutput* DOMImplementationImpl::createLSOutput(MemoryManager* const manager)
{
    return new (manager) DOMLSOutputImpl(manager);
}

// ------------------------------------------------------------
// DOMImplementationSource Virtual interface
// ------------------------------------------------------------
DOMImplementation* DOMImplementationImpl::getDOMImplementation(const XMLCh* features) const
{
    DOMImplementation* impl = DOMImplementation::getImplementation();

    XMLStringTokenizer tokenizer(features, XMLPlatformUtils::fgMemoryManager);
    const XMLCh* feature = 0;

    while (feature || tokenizer.hasMoreTokens()) {

        if (!feature)
            feature = tokenizer.nextToken();

        const XMLCh* version = 0;
        const XMLCh* token = tokenizer.nextToken();

        if (token && XMLString::isDigit(token[0]))
            version = token;

        if (!impl->hasFeature(feature, version))
            return 0;

        if (!version)
            feature = token;
    }
    return impl;
}

DOMImplementationList* DOMImplementationImpl::getDOMImplementationList(const XMLCh* features) const
{
    DOMImplementationListImpl* list = new DOMImplementationListImpl;
    DOMImplementation* myImpl=getDOMImplementation(features);
    if(myImpl)
        list->add(myImpl);
    return list;
}

XERCES_CPP_NAMESPACE_END
