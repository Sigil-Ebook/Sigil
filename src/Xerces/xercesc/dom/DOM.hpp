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
 * $Id: DOM.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOM_HPP)
#define XERCESC_INCLUDE_GUARD_DOM_HPP

//
//  This is the primary header file for inclusion in application
//  programs using the C++ XML Document Object Model API.
//

#include <xercesc/dom/DOMAttr.hpp>
#include <xercesc/dom/DOMCDATASection.hpp>
#include <xercesc/dom/DOMCharacterData.hpp>
#include <xercesc/dom/DOMComment.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMDocumentFragment.hpp>
#include <xercesc/dom/DOMDocumentType.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMEntity.hpp>
#include <xercesc/dom/DOMEntityReference.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMNamedNodeMap.hpp>
#include <xercesc/dom/DOMNode.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/dom/DOMNotation.hpp>
#include <xercesc/dom/DOMProcessingInstruction.hpp>
#include <xercesc/dom/DOMText.hpp>

// Introduced in DOM Level 2
#include <xercesc/dom/DOMDocumentRange.hpp>
#include <xercesc/dom/DOMDocumentTraversal.hpp>
#include <xercesc/dom/DOMNodeFilter.hpp>
#include <xercesc/dom/DOMNodeIterator.hpp>
#include <xercesc/dom/DOMRange.hpp>
#include <xercesc/dom/DOMRangeException.hpp>
#include <xercesc/dom/DOMTreeWalker.hpp>

// Introduced in DOM Level 3
#include <xercesc/dom/DOMLSParser.hpp>
#include <xercesc/dom/DOMLSParserFilter.hpp>
#include <xercesc/dom/DOMConfiguration.hpp>
#include <xercesc/dom/DOMLSResourceResolver.hpp>
#include <xercesc/dom/DOMError.hpp>
#include <xercesc/dom/DOMErrorHandler.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMImplementationList.hpp>
#include <xercesc/dom/DOMImplementationRegistry.hpp>
#include <xercesc/dom/DOMImplementationSource.hpp>
#include <xercesc/dom/DOMLSInput.hpp>
#include <xercesc/dom/DOMLSOutput.hpp>
#include <xercesc/dom/DOMLocator.hpp>
#include <xercesc/dom/DOMPSVITypeInfo.hpp>
#include <xercesc/dom/DOMStringList.hpp>
#include <xercesc/dom/DOMTypeInfo.hpp>
#include <xercesc/dom/DOMUserDataHandler.hpp>
#include <xercesc/dom/DOMLSSerializer.hpp>
#include <xercesc/dom/DOMLSSerializerFilter.hpp>

#include <xercesc/dom/DOMXPathEvaluator.hpp>
#include <xercesc/dom/DOMXPathNSResolver.hpp>
#include <xercesc/dom/DOMXPathException.hpp>
#include <xercesc/dom/DOMXPathExpression.hpp>
#include <xercesc/dom/DOMXPathResult.hpp>
#include <xercesc/dom/DOMXPathNamespace.hpp>


#endif
