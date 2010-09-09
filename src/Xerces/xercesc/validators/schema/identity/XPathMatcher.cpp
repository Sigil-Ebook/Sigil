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
 * $Id: XPathMatcher.cpp 804234 2009-08-14 14:20:16Z amassari $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/schema/identity/XPathMatcher.hpp>
#include <xercesc/validators/schema/identity/XercesXPath.hpp>
#include <xercesc/validators/schema/SchemaElementDecl.hpp>
#include <xercesc/validators/schema/SchemaAttDef.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xercesc/util/RuntimeException.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/framework/ValidationContext.hpp>

XERCES_CPP_NAMESPACE_BEGIN

typedef JanitorMemFunCall<XPathMatcher>     CleanupType;

// ---------------------------------------------------------------------------
//  XPathMatcher: Constructors and Destructor
// ---------------------------------------------------------------------------
XPathMatcher::XPathMatcher( XercesXPath* const xpath
                          , MemoryManager* const manager)
    : fLocationPathSize(0)
    , fMatched(0)
    , fNoMatchDepth(0)
    , fCurrentStep(0)
    , fStepIndexes(0)
    , fLocationPaths(0)
    , fIdentityConstraint(0)
    , fMemoryManager(manager)
{
    CleanupType cleanup(this, &XPathMatcher::cleanUp);

    try {
        init(xpath);
    }
    catch(const OutOfMemoryException&)
    {
        cleanup.release();

        throw;
    }

    cleanup.release();
}


XPathMatcher::XPathMatcher(XercesXPath* const xpath,
                           IdentityConstraint* const ic,
						   MemoryManager* const manager)
    : fLocationPathSize(0)
    , fMatched(0)
    , fNoMatchDepth(0)
    , fCurrentStep(0)
    , fStepIndexes(0)
    , fLocationPaths(0)
    , fIdentityConstraint(ic)
    , fMemoryManager(manager)
{
    CleanupType cleanup(this, &XPathMatcher::cleanUp);

    try {
        init(xpath);
    }
    catch(const OutOfMemoryException&)
    {
        cleanup.release();

        throw;
    }

    cleanup.release();
}


XPathMatcher::~XPathMatcher()
{
    cleanUp();
}

// ---------------------------------------------------------------------------
//  XPathMatcher: Helper methods
// ---------------------------------------------------------------------------
void XPathMatcher::init(XercesXPath* const xpath) {

    if (xpath) {

        fLocationPaths = xpath->getLocationPaths();
        fLocationPathSize = (fLocationPaths ? fLocationPaths->size() : 0);

        if (fLocationPathSize) {

            fStepIndexes = new (fMemoryManager) RefVectorOf<ValueStackOf<XMLSize_t> >(fLocationPathSize, true, fMemoryManager);
            fCurrentStep = (XMLSize_t*) fMemoryManager->allocate
            (
                fLocationPathSize * sizeof(XMLSize_t)
            );//new int[fLocationPathSize];
            fNoMatchDepth = (XMLSize_t*) fMemoryManager->allocate
            (
                fLocationPathSize * sizeof(XMLSize_t)
            );//new int[fLocationPathSize];
            fMatched = (unsigned char*) fMemoryManager->allocate
            (
                fLocationPathSize * sizeof(unsigned char)
            );//new int[fLocationPathSize];

            for(XMLSize_t i=0; i < fLocationPathSize; i++) {
                fStepIndexes->addElement(new (fMemoryManager) ValueStackOf<XMLSize_t>(8, fMemoryManager));
            }
        }
    }
}


// ---------------------------------------------------------------------------
//  XPathMatcher: XMLDocumentHandler methods
// ---------------------------------------------------------------------------
void XPathMatcher::startDocumentFragment() {

    for(XMLSize_t i = 0; i < fLocationPathSize; i++) {

        fStepIndexes->elementAt(i)->removeAllElements();
        fCurrentStep[i] = 0;
        fNoMatchDepth[i] = 0;
        fMatched[i] = 0;
    }
}

void XPathMatcher::startElement(const XMLElementDecl& elemDecl,
                                const unsigned int urlId,
                                const XMLCh* const elemPrefix,
								const RefVectorOf<XMLAttr>& attrList,
                                const XMLSize_t attrCount,
                                ValidationContext* validationContext /*=0*/) {

    for (XMLSize_t i = 0; i < fLocationPathSize; i++) {

        // push context
        XMLSize_t startStep = fCurrentStep[i];
        fStepIndexes->elementAt(i)->push(startStep);

        // try next xpath, if not matching
        if ((fMatched[i] & XP_MATCHED_D) == XP_MATCHED || fNoMatchDepth[i] > 0) {
            fNoMatchDepth[i]++;
            continue;
        }

        if((fMatched[i] & XP_MATCHED_D) == XP_MATCHED_D) {
            fMatched[i] = XP_MATCHED_DP;
        }

        // consume self::node() steps
        XercesLocationPath* locPath = fLocationPaths->elementAt(i);
        XMLSize_t stepSize = locPath->getStepSize();

        while (fCurrentStep[i] < stepSize &&
               locPath->getStep(fCurrentStep[i])->getAxisType() == XercesStep::AxisType_SELF) {
            fCurrentStep[i]++;
        }

        if (fCurrentStep[i] == stepSize) {

            fMatched[i] = XP_MATCHED;
            continue;
        }

        // now if the current step is a descendant step, we let the next
        // step do its thing; if it fails, we reset ourselves
        // to look at this step for next time we're called.
        // so first consume all descendants:
        XMLSize_t descendantStep = fCurrentStep[i];

        while (fCurrentStep[i] < stepSize &&
               locPath->getStep(fCurrentStep[i])->getAxisType() == XercesStep::AxisType_DESCENDANT) {
            fCurrentStep[i]++;
        }

        bool sawDescendant = fCurrentStep[i] > descendantStep;
        if (fCurrentStep[i] == stepSize) {

            fNoMatchDepth[i]++;
            continue;
        }

        // match child::... step, if haven't consumed any self::node()
        if ((fCurrentStep[i] == startStep || fCurrentStep[i] > descendantStep) &&
            locPath->getStep(fCurrentStep[i])->getAxisType() == XercesStep::AxisType_CHILD) {

            XercesStep* step = locPath->getStep(fCurrentStep[i]);
            XercesNodeTest* nodeTest = step->getNodeTest();

            QName elemQName(elemPrefix, elemDecl.getElementName()->getLocalPart(), urlId, fMemoryManager);
            if (!matches(nodeTest, &elemQName)) {

                if(fCurrentStep[i] > descendantStep) {
                    fCurrentStep[i] = descendantStep;
                    continue;
                }

                fNoMatchDepth[i]++;
                continue;
            }

            fCurrentStep[i]++;
        }

        if (fCurrentStep[i] == stepSize) {

            if (sawDescendant) {

                fCurrentStep[i] = descendantStep;
                fMatched[i] = XP_MATCHED_D;
            }
            else {
                fMatched[i] = XP_MATCHED;
            }

            continue;
        }

        // match attribute::... step
        if (fCurrentStep[i] < stepSize &&
            locPath->getStep(fCurrentStep[i])->getAxisType() == XercesStep::AxisType_ATTRIBUTE) {

            if (attrCount) {

                XercesNodeTest* nodeTest = locPath->getStep(fCurrentStep[i])->getNodeTest();

                for (XMLSize_t attrIndex = 0; attrIndex < attrCount; attrIndex++) {

                    const XMLAttr* curDef = attrList.elementAt(attrIndex);

                    if (matches(nodeTest, curDef->getAttName())) {

                        fCurrentStep[i]++;

                        if (fCurrentStep[i] == stepSize) {

                            fMatched[i] = XP_MATCHED_A;

                            SchemaAttDef* attDef = ((SchemaElementDecl&) elemDecl).getAttDef(curDef->getName(), curDef->getURIId());
                            DatatypeValidator* dv = (attDef) ? attDef->getDatatypeValidator() : 0;
                            const XMLCh* value = curDef->getValue();
                            // store QName using their Clark name
                            if(dv && dv->getType()==DatatypeValidator::QName)
                            {
                                int index=XMLString::indexOf(value, chColon);
                                if(index==-1)
                                    matched(value, dv, false);
                                else
                                {
                                    XMLBuffer buff(1023, fMemoryManager);
                                    buff.append(chOpenCurly);
                                    if(validationContext)
                                    {
                                        XMLCh* prefix=(XMLCh*)fMemoryManager->allocate((index+1)*sizeof(XMLCh));
                                        ArrayJanitor<XMLCh> janPrefix(prefix, fMemoryManager);
                                        XMLString::subString(prefix, value, 0, (XMLSize_t)index, fMemoryManager);
                                        buff.append(validationContext->getURIForPrefix(prefix));
                                    }
                                    buff.append(chCloseCurly);
                                    buff.append(value+index+1);
                                    matched(buff.getRawBuffer(), dv, false);
                                }
                            }
                            else
                                matched(value, dv, false);
                        }
                        break;
                    }
                }
            }

            if ((fMatched[i] & XP_MATCHED) != XP_MATCHED) {

                if(fCurrentStep[i] > descendantStep) {

                    fCurrentStep[i] = descendantStep;
                    continue;
                }

                fNoMatchDepth[i]++;
            }
        }
    }
}

void XPathMatcher::endElement(const XMLElementDecl& elemDecl,
                              const XMLCh* const elemContent,
                              ValidationContext* validationContext /*=0*/,
                              DatatypeValidator* actualValidator /*=0*/) {

    for(XMLSize_t i = 0; i < fLocationPathSize; i++) {

        // go back a step
        fCurrentStep[i] = fStepIndexes->elementAt(i)->pop();

        // don't do anything, if not matching
        if (fNoMatchDepth[i] > 0) {
            fNoMatchDepth[i]--;
        }
        // signal match, if appropriate
        else {

            if (fMatched[i] == 0)
                continue;
            
            if ((fMatched[i] & XP_MATCHED_A) == XP_MATCHED_A) {
                fMatched[i] = 0;
                continue;
            }

            DatatypeValidator* dv = actualValidator?actualValidator:((SchemaElementDecl*) &elemDecl)->getDatatypeValidator();
            bool isNillable = (((SchemaElementDecl *) &elemDecl)->getMiscFlags() & SchemaSymbols::XSD_NILLABLE) != 0;

            // store QName using their Clark name
            if(dv && dv->getType()==DatatypeValidator::QName)
            {
                int index=XMLString::indexOf(elemContent, chColon);
                if(index==-1)
                    matched(elemContent, dv, isNillable);
                else
                {
                    XMLBuffer buff(1023, fMemoryManager);
                    buff.append(chOpenCurly);
                    if(validationContext)
                    {
                        XMLCh* prefix=(XMLCh*)fMemoryManager->allocate((index+1)*sizeof(XMLCh));
                        ArrayJanitor<XMLCh> janPrefix(prefix, fMemoryManager);
                        XMLString::subString(prefix, elemContent, 0, (XMLSize_t)index, fMemoryManager);
                        buff.append(validationContext->getURIForPrefix(prefix));
                    }
                    buff.append(chCloseCurly);
                    buff.append(elemContent+index+1);
                    matched(buff.getRawBuffer(), dv, isNillable);
                }
            }
            else
                matched(elemContent, dv, isNillable);
            fMatched[i] = 0;
        }
    }
}


// ---------------------------------------------------------------------------
//  XPathMatcher: Match methods
// ---------------------------------------------------------------------------
unsigned char XPathMatcher::isMatched() {

    // xpath has been matched if any one of the members of the union have matched.
    for (XMLSize_t i=0; i < fLocationPathSize; i++) {
        if (((fMatched[i] & XP_MATCHED) == XP_MATCHED)
            && ((fMatched[i] & XP_MATCHED_DP) != XP_MATCHED_DP))
            return fMatched[i];
    }

    return 0;
}

void XPathMatcher::matched(const XMLCh* const,
                           DatatypeValidator* const,
                           const bool) {
    return;
}

bool XPathMatcher::matches(const XercesNodeTest* nodeTest, const QName* qName)
{
    if (nodeTest->getType() == XercesNodeTest::NodeType_QNAME) {
        return (*nodeTest->getName())==(*qName);
    }
    if (nodeTest->getType() == XercesNodeTest::NodeType_NAMESPACE) {
        return nodeTest->getName()->getURI() == qName->getURI();
    }
    // NodeType_WILDCARD
    return true;
}

// ---------------------------------------------------------------------------
//  XPathMatcher: Match methods
// ---------------------------------------------------------------------------
int XPathMatcher::getInitialDepth() const
{
    ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Regex_NotSupported, fMemoryManager);
    return 0; // to make some compilers happy
}

XERCES_CPP_NAMESPACE_END

/**
  * End of file XPathMatcher.cpp
  */

