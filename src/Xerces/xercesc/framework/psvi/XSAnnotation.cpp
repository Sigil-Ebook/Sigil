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
 * $Id: XSAnnotation.cpp 679296 2008-07-24 08:13:42Z borisk $
 */

#include <xercesc/framework/psvi/XSAnnotation.hpp>
#include <xercesc/util/XMLString.hpp>

#include <xercesc/framework/MemBufInputSource.hpp>

#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOMElement.hpp>

XERCES_CPP_NAMESPACE_BEGIN

XSAnnotation::XSAnnotation(const XMLCh*          const content,
                                 MemoryManager * const manager)
:XSObject(XSConstants::ANNOTATION, 0, manager)
,fContents(XMLString::replicate(content, manager))
,fNext(0)
,fSystemId(0)
,fLine(0)
,fCol(0)
{
}

XSAnnotation::XSAnnotation(MemoryManager * const manager)
:XSObject(XSConstants::ANNOTATION, 0, manager)
,fContents(0)
,fNext(0)
,fSystemId(0)
,fLine(0)
,fCol(0)
{
}

XSAnnotation::~XSAnnotation()
{
    fMemoryManager->deallocate(fContents);

    if (fNext)
        delete fNext;

    fMemoryManager->deallocate(fSystemId);
}

// XSAnnotation methods
void XSAnnotation::writeAnnotation(DOMNode* node, ANNOTATION_TARGET targetType)
{
    XercesDOMParser *parser = new (fMemoryManager) XercesDOMParser(0, fMemoryManager);
    parser->setDoNamespaces(true);
    parser->setValidationScheme(XercesDOMParser::Val_Never);

    DOMDocument* futureOwner = (targetType == W3C_DOM_ELEMENT) ?
        ((DOMElement*)node)->getOwnerDocument() :
        (DOMDocument*)node;

    MemBufInputSource* memBufIS = new (fMemoryManager) MemBufInputSource
    (
        (const XMLByte*)fContents
        , XMLString::stringLen(fContents)*sizeof(XMLCh)
        , ""
        , false
        , fMemoryManager
    );
    memBufIS->setEncoding(XMLUni::fgXMLChEncodingString);
    memBufIS->setCopyBufToStream(false);

    try
    {
        parser->parse(*memBufIS);
    }
    catch (const XMLException&)
    {
        // REVISIT:  should we really eat this?
    }

    DOMNode* newElem = futureOwner->importNode((parser->getDocument())->getDocumentElement(), true);
    node->insertBefore(newElem, node->getFirstChild());

    delete parser;
    delete memBufIS;
}


void XSAnnotation::writeAnnotation(ContentHandler* handler)
{
    SAX2XMLReader* parser = XMLReaderFactory::createXMLReader(fMemoryManager);
    parser->setFeature(XMLUni::fgSAX2CoreNameSpaces, true);
    parser->setFeature(XMLUni::fgSAX2CoreValidation, false);
    parser->setContentHandler(handler);

    MemBufInputSource* memBufIS = new (fMemoryManager) MemBufInputSource
    (
        (const XMLByte*)fContents
        , XMLString::stringLen(fContents)*sizeof(XMLCh)
        , ""
        , false
        , fMemoryManager
    );
    memBufIS->setEncoding(XMLUni::fgXMLChEncodingString);
    memBufIS->setCopyBufToStream(false);

    try
    {
        parser->parse(*memBufIS);
    }
    catch (const XMLException&)
    {
    }

    delete parser;
    delete memBufIS;
}


void XSAnnotation::setNext(XSAnnotation* const nextAnnotation)
{
    if (fNext)
        fNext->setNext(nextAnnotation);
    else
        fNext = nextAnnotation;
}

XSAnnotation* XSAnnotation::getNext()
{
    return fNext;
}

void XSAnnotation::setSystemId(const XMLCh* const systemId)
{
    if (fSystemId)
    {
        fMemoryManager->deallocate(fSystemId);
        fSystemId = 0;
    }

    if (systemId)
        fSystemId = XMLString::replicate(systemId, fMemoryManager);

}

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_TOCREATE(XSAnnotation)

void XSAnnotation::serialize(XSerializeEngine& serEng)
{

    if (serEng.isStoring())
    {
        serEng.writeString(fContents);
        serEng<<fNext;
        serEng.writeString(fSystemId);

        serEng.writeUInt64 (fLine);
        serEng.writeUInt64 (fCol);
    }
    else
    {
        serEng.readString(fContents);
        serEng>>fNext;
        serEng.readString(fSystemId);

        serEng.readUInt64 (fLine);
        serEng.readUInt64 (fCol);
    }
}

XERCES_CPP_NAMESPACE_END
