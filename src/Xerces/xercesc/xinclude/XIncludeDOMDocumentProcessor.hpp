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
 * $Id: XIncludeDOMDocumentProcessor.hpp 655706 2008-05-13 01:08:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XINCLUDEDOMDOCUMENTPROCESSOR_HPP)
#define XERCESC_INCLUDE_GUARD_XINCLUDEDOMDOCUMENTPROCESSOR_HPP

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/XMLErrorReporter.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XMLEntityHandler;

/**
  * Class for representing and manipulating the XMLCh * href's used
  * by an xi:include element.
  *
  * This class is designed primarily for internal use. This class implements
  * the functionality required to calculate relative hrefs and the base URI
  * fixups required for performing XInclude functionality.
  */
class XINCLUDE_EXPORT XIncludeDOMDocumentProcessor
{
public:
     /** Walk the supplied DOMDocument performing all XInclude's as encountered.
      *
      * @param source A DOMDocument to parse, this document is not modified.
      * @param errorHandled An errorHandler to call back in case of problems
      *
      * @return a newly created DOMDocument containing the parsed and actioned
      * xinclude elements.
      */
    DOMDocument *doXIncludeDOMProcess(const DOMDocument * const source, XMLErrorReporter *errorHandler, XMLEntityHandler* entityResolver=NULL);
};

XERCES_CPP_NAMESPACE_END

#endif /* XINCLUDEDOMDOCUMENTPROCESSOR_HPP */

