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
 * $Id: XIncludeDOMDocumentProcessor.cpp 655706 2008-05-13 01:08:39Z amassari $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/xinclude/XIncludeDOMDocumentProcessor.hpp>
#include <xercesc/xinclude/XIncludeUtils.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/framework/XMLErrorReporter.hpp>

XERCES_CPP_NAMESPACE_BEGIN

DOMDocument *
XIncludeDOMDocumentProcessor::doXIncludeDOMProcess(const DOMDocument * const source, XMLErrorReporter *errorHandler, XMLEntityHandler* entityResolver /*=NULL*/){
    XIncludeUtils xiu(errorHandler);

    DOMImplementation* impl = source->getImplementation();
    DOMDocument *xincludedDocument = impl->createDocument();
    
    try
    {
        /* set up the declaration etc of the output document to match the source */
        xincludedDocument->setDocumentURI( source->getDocumentURI());
        xincludedDocument->setXmlStandalone( source->getXmlStandalone());
        xincludedDocument->setXmlVersion( source->getXmlVersion());

        /* copy entire source document into the xincluded document. Xincluded document can
           then be modified in place */
        DOMNode *child = source->getFirstChild();
        for (; child != NULL; child = child->getNextSibling()){
            if (child->getNodeType() == DOMNode::DOCUMENT_TYPE_NODE){
                /* I am simply ignoring these at the moment */
                continue;
            }
            DOMNode *newNode = xincludedDocument->importNode(child, true);
            xincludedDocument->appendChild(newNode);
        }

        DOMNode *docNode = xincludedDocument->getDocumentElement();
        /* parse and include the document node */
        xiu.parseDOMNodeDoingXInclude(docNode, xincludedDocument, entityResolver);

        xincludedDocument->normalizeDocument();
    }
    catch(const XMLErrs::Codes)
    {
        xincludedDocument->release();
        return NULL;
    }
    catch(...)
    {
        xincludedDocument->release();
        throw;
    }

    return xincludedDocument;
}

XERCES_CPP_NAMESPACE_END
