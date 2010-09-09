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

/**
  * $Id: XSDDOMParser.cpp 729944 2008-12-29 17:03:32Z amassari $
  */



// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/schema/XSDDOMParser.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xercesc/internal/XMLScanner.hpp>
#include <xercesc/internal/ElemStack.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/impl/DOMElementImpl.hpp>
#include <xercesc/dom/impl/DOMAttrImpl.hpp>
#include <xercesc/dom/impl/DOMTextImpl.hpp>
#include <xercesc/framework/XMLValidityCodes.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  XSDDOMParser: Constructors and Destructor
// ---------------------------------------------------------------------------
XSDDOMParser::XSDDOMParser( XMLValidator* const   valToAdopt
                          , MemoryManager* const  manager
                          , XMLGrammarPool* const gramPool):
    XercesDOMParser(valToAdopt, manager, gramPool)
    , fSawFatal(false)
    , fAnnotationDepth(-1)
    , fInnerAnnotationDepth(-1)
    , fDepth(-1)
    , fUserErrorReporter(0)
    , fUserEntityHandler(0)
    , fURIs(0)
    , fAnnotationBuf(1023, manager)

{
    fURIs = new (manager) ValueVectorOf<unsigned int>(16, manager);
    fXSDErrorReporter.setErrorReporter(this);
    setValidationScheme(XercesDOMParser::Val_Never);
    setDoNamespaces(true);
}


XSDDOMParser::~XSDDOMParser()
{
    delete fURIs;
}


// ---------------------------------------------------------------------------
//  XSDDOMParser: Helper methods
// ---------------------------------------------------------------------------
DOMElement* XSDDOMParser::createElementNSNode(const XMLCh *namespaceURI,
                                              const XMLCh *qualifiedName)
{
    ReaderMgr::LastExtEntityInfo lastInfo;
    ((ReaderMgr*) fScanner->getLocator())->getLastExtEntityInfo(lastInfo);

    return getDocument()->createElementNS(namespaceURI, qualifiedName,
                                          lastInfo.lineNumber, lastInfo.colNumber);
}


void XSDDOMParser::startAnnotation( const XMLElementDecl&       elemDecl
                                  , const RefVectorOf<XMLAttr>& attrList
                                  , const XMLSize_t             attrCount)
{
    fAnnotationBuf.append(chOpenAngle);
	fAnnotationBuf.append(elemDecl.getFullName());
    fAnnotationBuf.append(chSpace);

    // attributes are a bit of a pain.  To get this right, we have to keep track
    // of the namespaces we've seen declared, then examine the namespace context
    // for other namespaces so that we can also include them.
    // optimized for simplicity and the case that not many
    // namespaces are declared on this annotation...
    fURIs->removeAllElements();
    for (XMLSize_t i=0; i < attrCount; i++) {

        const XMLAttr* oneAttrib = attrList.elementAt(i);
        const XMLCh* attrValue = oneAttrib->getValue();

        if (XMLString::equals(oneAttrib->getName(), XMLUni::fgXMLNSString))
            fURIs->addElement(fScanner->getPrefixId(XMLUni::fgZeroLenString));
        else  if (!XMLString::compareNString(oneAttrib->getQName(), XMLUni::fgXMLNSColonString, 6))
            fURIs->addElement(fScanner->getPrefixId(oneAttrib->getName()));

        fAnnotationBuf.append(oneAttrib->getQName());
        fAnnotationBuf.append(chEqual);
        fAnnotationBuf.append(chDoubleQuote);
        fAnnotationBuf.append(attrValue);
        fAnnotationBuf.append(chDoubleQuote);
        fAnnotationBuf.append(chSpace);
    }

    // now we have to look through currently in-scope namespaces to see what
    // wasn't declared here
    ValueVectorOf<PrefMapElem*>* namespaceContext = fScanner->getNamespaceContext();
    for (XMLSize_t j=0; j < namespaceContext->size(); j++)
    {
        unsigned int prefId = namespaceContext->elementAt(j)->fPrefId;

        if (!fURIs->containsElement(prefId)) {

            const XMLCh* prefix = fScanner->getPrefixForId(prefId);

            if (XMLString::equals(prefix, XMLUni::fgZeroLenString)) {
                fAnnotationBuf.append(XMLUni::fgXMLNSString);
            }
            else  {
                fAnnotationBuf.append(XMLUni::fgXMLNSColonString);
                fAnnotationBuf.append(prefix);
            }

            fAnnotationBuf.append(chEqual);
            fAnnotationBuf.append(chDoubleQuote);
            fAnnotationBuf.append(fScanner->getURIText(namespaceContext->elementAt(j)->fURIId));
            fAnnotationBuf.append(chDoubleQuote);
            fAnnotationBuf.append(chSpace);

            fURIs->addElement(prefId);
        }
    }

    fAnnotationBuf.append(chCloseAngle);
    fAnnotationBuf.append(chLF);
}

void XSDDOMParser::startAnnotationElement( const XMLElementDecl&       elemDecl
                                         , const RefVectorOf<XMLAttr>& attrList
                                         , const XMLSize_t             attrCount)
{
    fAnnotationBuf.append(chOpenAngle);
    fAnnotationBuf.append(elemDecl.getFullName());
    //fAnnotationBuf.append(chSpace);

    for(XMLSize_t i=0; i < attrCount; i++) {

        const XMLAttr* oneAttr = attrList.elementAt(i);
        fAnnotationBuf.append(chSpace);
        fAnnotationBuf.append(oneAttr ->getQName());
        fAnnotationBuf.append(chEqual);
        fAnnotationBuf.append(chDoubleQuote);
        fAnnotationBuf.append(oneAttr->getValue());
        fAnnotationBuf.append(chDoubleQuote);
    }

    fAnnotationBuf.append(chCloseAngle);
}

void XSDDOMParser::endAnnotationElement( const XMLElementDecl& elemDecl
                                       , bool complete)
{
    if (complete)
    {
        fAnnotationBuf.append(chLF);
        fAnnotationBuf.append(chOpenAngle);
        fAnnotationBuf.append(chForwardSlash);
        fAnnotationBuf.append(elemDecl.getFullName());
        fAnnotationBuf.append(chCloseAngle);

        // note that this is always called after endElement on <annotation>'s
        // child and before endElement on annotation.
        // hence, we must make this the child of the current
        // parent's only child.
        DOMTextImpl *node = (DOMTextImpl *)fDocument->createTextNode(fAnnotationBuf.getRawBuffer());
        fCurrentNode->appendChild(node);
        fAnnotationBuf.reset();
    }
    else      //capturing character calls
    {
        fAnnotationBuf.append(chOpenAngle);
        fAnnotationBuf.append(chForwardSlash);
        fAnnotationBuf.append(elemDecl.getFullName());
        fAnnotationBuf.append(chCloseAngle);
    }
}


// ---------------------------------------------------------------------------
//  XSDDOMParser: Setter methods
// ---------------------------------------------------------------------------
void XSDDOMParser::setUserErrorReporter(XMLErrorReporter* const errorReporter)
{
    fUserErrorReporter = errorReporter;
    fScanner->setErrorReporter(this);
}

void XSDDOMParser::setUserEntityHandler(XMLEntityHandler* const entityHandler)
{
    fUserEntityHandler = entityHandler;
    fScanner->setEntityHandler(this);
}


// ---------------------------------------------------------------------------
//  XSDDOMParser: Implementation of the XMLDocumentHandler interface
// ---------------------------------------------------------------------------
void XSDDOMParser::startElement( const XMLElementDecl&       elemDecl
                               , const unsigned int          urlId
                               , const XMLCh* const          elemPrefix
                               , const RefVectorOf<XMLAttr>& attrList
                               , const XMLSize_t             attrCount
                               , const bool                  isEmpty
                               , const bool                  isRoot)
{
    fDepth++;

    // while it is true that non-whitespace character data
    // may only occur in appInfo or documentation
    // elements, it's certainly legal for comments and PI's to
    // occur as children of annotation; we need
    // to account for these here.
    if (fAnnotationDepth == -1)
    {
        if (XMLString::equals(elemDecl.getBaseName(), SchemaSymbols::fgELT_ANNOTATION) &&
            XMLString::equals(getURIText(urlId), SchemaSymbols::fgURI_SCHEMAFORSCHEMA))
        {

            fAnnotationDepth = fDepth;
            startAnnotation(elemDecl, attrList, attrCount);
        }
    }
    else if (fDepth == fAnnotationDepth+1)
    {
        fInnerAnnotationDepth = fDepth;
        startAnnotationElement(elemDecl, attrList, attrCount);
    }
    else
    {
        startAnnotationElement(elemDecl, attrList, attrCount);
        if(isEmpty)
            endElement(elemDecl, urlId, isRoot, elemPrefix);
        // avoid falling through; don't call startElement in this case
        return;
    }

    DOMElement *elem;
    if (urlId != fScanner->getEmptyNamespaceId())  //TagName has a prefix
    {
        if (elemPrefix && *elemPrefix)
        {
            XMLBufBid elemQName(&fBufMgr);
            elemQName.set(elemPrefix);
            elemQName.append(chColon);
            elemQName.append(elemDecl.getBaseName());
            elem = createElementNSNode(
                fScanner->getURIText(urlId), elemQName.getRawBuffer());
        }
        else {
            elem = createElementNSNode(
                fScanner->getURIText(urlId), elemDecl.getBaseName());
        }
    }
    else {
        elem = createElementNSNode(0, elemDecl.getBaseName());
    }

    DOMElementImpl *elemImpl = (DOMElementImpl *) elem;
    for (XMLSize_t index = 0; index < attrCount; ++index)
    {
        const XMLAttr* oneAttrib = attrList.elementAt(index);
        unsigned int attrURIId = oneAttrib->getURIId();
        const XMLCh* namespaceURI = 0;

        //for xmlns=...
        if (XMLString::equals(oneAttrib->getName(), XMLUni::fgXMLNSString))
            attrURIId = fScanner->getXMLNSNamespaceId();

        //TagName has a prefix
        if (attrURIId != fScanner->getEmptyNamespaceId())
            namespaceURI = fScanner->getURIText(attrURIId); //get namespaceURI

        //  revisit.  Optimize to init the named node map to the
        //            right size up front.
        DOMAttrImpl *attr = (DOMAttrImpl *)
            fDocument->createAttributeNS(namespaceURI, oneAttrib->getQName());
        attr->setValue(oneAttrib -> getValue());
        DOMNode* remAttr = elemImpl->setAttributeNodeNS(attr);
        if (remAttr)
            remAttr->release();

        // Attributes of type ID.  If this is one, add it to the hashtable of IDs
        //   that is constructed for use by GetElementByID().
        if (oneAttrib->getType()==XMLAttDef::ID)
        {
            if (fDocument->fNodeIDMap == 0)
                fDocument->fNodeIDMap = new (fDocument) DOMNodeIDMap(500, fDocument);
            fDocument->fNodeIDMap->add(attr);
            attr->fNode.isIdAttr(true);
        }

        attr->setSpecified(oneAttrib->getSpecified());
    }

    // set up the default attributes
    if (elemDecl.hasAttDefs())
	{
        XMLAttDefList* defAttrs = &elemDecl.getAttDefList();
        XMLAttDef* attr = 0;
        DOMAttrImpl * insertAttr = 0;

        for (XMLSize_t i=0; i<defAttrs->getAttDefCount(); i++)
        {
            attr = &defAttrs->getAttDef(i);

            const XMLAttDef::DefAttTypes defType = attr->getDefaultType();
            if ((defType == XMLAttDef::Default)
            ||  (defType == XMLAttDef::Fixed))
            {
                // DOM Level 2 wants all namespace declaration attributes
                // to be bound to "http://www.w3.org/2000/xmlns/"
                // So as long as the XML parser doesn't do it, it needs to
                // done here.
                const XMLCh* qualifiedName = attr->getFullName();
                XMLBufBid bbPrefixQName(&fBufMgr);
                XMLBuffer& prefixBuf = bbPrefixQName.getBuffer();
                int colonPos = -1;
                unsigned int uriId = fScanner->resolveQName(qualifiedName, prefixBuf, ElemStack::Mode_Attribute, colonPos);

                const XMLCh* namespaceURI = 0;
                if (XMLString::equals(qualifiedName, XMLUni::fgXMLNSString))
                    uriId = fScanner->getXMLNSNamespaceId();

                //TagName has a prefix
                if (uriId != fScanner->getEmptyNamespaceId())
                    namespaceURI = fScanner->getURIText(uriId);

                insertAttr = (DOMAttrImpl *) fDocument->createAttributeNS(
                    namespaceURI, qualifiedName);

                DOMAttr* remAttr = elemImpl->setDefaultAttributeNodeNS(insertAttr);
                if (remAttr)
                    remAttr->release();

                if (attr->getValue() != 0)
                {
                    insertAttr->setValue(attr->getValue());
                    insertAttr->setSpecified(false);
                }
            }

            insertAttr = 0;
            attr->reset();
        }
    }

    fCurrentParent->appendChild(elem);
    fCurrentParent = elem;
    fCurrentNode = elem;
    fWithinElement = true;

    // If an empty element, do end right now (no endElement() will be called)
    if (isEmpty)
        endElement(elemDecl, urlId, isRoot, elemPrefix);
}



void XSDDOMParser::endElement( const XMLElementDecl& elemDecl
                             , const unsigned int
                             , const bool
                             , const XMLCh* const)
{
    if(fAnnotationDepth > -1)
    {
        if (fInnerAnnotationDepth == fDepth)
        {
            fInnerAnnotationDepth = -1;
            endAnnotationElement(elemDecl, false);
	    }
        else if (fAnnotationDepth == fDepth)
        {
            fAnnotationDepth = -1;
            endAnnotationElement(elemDecl, true);
        }
        else
        {   // inside a child of annotation
            endAnnotationElement(elemDecl, false);
            fDepth--;
            return;
        }
    }

    fDepth--;
    fCurrentNode   = fCurrentParent;
    fCurrentParent = fCurrentNode->getParentNode ();

    // If we've hit the end of content, clear the flag.
    //
    if (fCurrentParent == fDocument)
        fWithinElement = false;
}

void XSDDOMParser::docCharacters(  const   XMLCh* const    chars
                              , const XMLSize_t       length
                              , const bool            cdataSection)
{
    // Ignore chars outside of content
    if (!fWithinElement)
        return;

    if (fInnerAnnotationDepth == -1)
    {
        if (!((ReaderMgr*) fScanner->getReaderMgr())->getCurrentReader()->isAllSpaces(chars, length))
        {
            ReaderMgr::LastExtEntityInfo lastInfo;
            fScanner->getReaderMgr()->getLastExtEntityInfo(lastInfo);
            fXSLocator.setValues(lastInfo.systemId, lastInfo.publicId, lastInfo.lineNumber, lastInfo.colNumber);
            fXSDErrorReporter.emitError(XMLValid::NonWSContent, XMLUni::fgValidityDomain, &fXSLocator);
        }
    }
    // when it's within either of the 2 annotation subelements, characters are
    // allowed and we need to store them.
    else if (cdataSection == true)
    {
        fAnnotationBuf.append(XMLUni::fgCDataStart);
        fAnnotationBuf.append(chars, length);
        fAnnotationBuf.append(XMLUni::fgCDataEnd);
    }
    else
    {
        for(unsigned int i = 0; i < length; i++ )
        {
            if(chars[i] == chAmpersand)
            {
                fAnnotationBuf.append(chAmpersand);
                fAnnotationBuf.append(XMLUni::fgAmp);
                fAnnotationBuf.append(chSemiColon);
            }
            else if (chars[i] == chOpenAngle)
            {
                fAnnotationBuf.append(chAmpersand);
                fAnnotationBuf.append(XMLUni::fgLT);
                fAnnotationBuf.append(chSemiColon);
            }
            else {
                fAnnotationBuf.append(chars[i]);
            }
        }
    }
}

void XSDDOMParser::docComment(const XMLCh* const comment)
{
    if (fAnnotationDepth > -1)
    {
        fAnnotationBuf.append(XMLUni::fgCommentString);
        fAnnotationBuf.append(comment);
        fAnnotationBuf.append(chDash);
        fAnnotationBuf.append(chDash);
        fAnnotationBuf.append(chCloseAngle);
    }
}

void XSDDOMParser::startEntityReference(const XMLEntityDecl&)
{
}

void XSDDOMParser::endEntityReference(const XMLEntityDecl&)
{
}

void XSDDOMParser::ignorableWhitespace( const XMLCh* const chars
                                      , const XMLSize_t    length
                                      , const bool)
{
    // Ignore chars before the root element
    if (!fWithinElement || !fIncludeIgnorableWhitespace)
        return;

    if (fAnnotationDepth > -1)
        fAnnotationBuf.append(chars, length);
}

// ---------------------------------------------------------------------------
//  XSDDOMParser: Implementation of the XMLErrorReporter interface
// ---------------------------------------------------------------------------
void XSDDOMParser::error(const   unsigned int                code
                         , const XMLCh* const                msgDomain
                         , const XMLErrorReporter::ErrTypes  errType
                         , const XMLCh* const                errorText
                         , const XMLCh* const                systemId
                         , const XMLCh* const                publicId
                         , const XMLFileLoc                  lineNum
                         , const XMLFileLoc                  colNum)
{
    if (errType >= XMLErrorReporter::ErrType_Fatal)
        fSawFatal = true;

    if (fUserErrorReporter)
        fUserErrorReporter->error(code, msgDomain, errType, errorText,
                                  systemId, publicId, lineNum, colNum);
}

InputSource*
XSDDOMParser::resolveEntity(XMLResourceIdentifier* resourceIdentifier)
{
    if (fUserEntityHandler)
        return fUserEntityHandler->resolveEntity(resourceIdentifier);

    return 0;
}

XERCES_CPP_NAMESPACE_END
