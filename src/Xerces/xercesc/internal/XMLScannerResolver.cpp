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
 * $Id: XMLScannerResolver.cpp 471747 2006-11-06 14:31:56Z amassari $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/internal/XMLScannerResolver.hpp>
#include <xercesc/internal/WFXMLScanner.hpp>
#include <xercesc/internal/DGXMLScanner.hpp>
#include <xercesc/internal/SGXMLScanner.hpp>
#include <xercesc/internal/IGXMLScanner.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  XMLScannerResolver: Public static methods
// ---------------------------------------------------------------------------
XMLScanner*
XMLScannerResolver::getDefaultScanner( XMLValidator* const  valToAdopt
                                     , GrammarResolver* const grammarResolver
                                     , MemoryManager* const manager)
{
    return new (manager) IGXMLScanner(valToAdopt, grammarResolver, manager);
}

XMLScanner*
XMLScannerResolver::resolveScanner( const XMLCh* const   scannerName
                                  , XMLValidator* const  valToAdopt
                                  , GrammarResolver* const grammarResolver
                                  , MemoryManager* const manager)
{
    if (XMLString::equals(scannerName, XMLUni::fgWFXMLScanner))
        return new (manager) WFXMLScanner(valToAdopt, grammarResolver, manager);
    else if (XMLString::equals(scannerName, XMLUni::fgIGXMLScanner))
        return new (manager) IGXMLScanner(valToAdopt, grammarResolver, manager);
    else if (XMLString::equals(scannerName, XMLUni::fgSGXMLScanner))
        return new (manager) SGXMLScanner(valToAdopt, grammarResolver, manager);
    else if (XMLString::equals(scannerName, XMLUni::fgDGXMLScanner))
        return new (manager) DGXMLScanner(valToAdopt, grammarResolver, manager);

    // REVISIT: throw an exception or return a default one?
    return 0;
}

XMLScanner*
XMLScannerResolver::resolveScanner( const XMLCh* const        scannerName
                                  , XMLDocumentHandler* const docHandler
                                  , DocTypeHandler* const     docTypeHandler
                                  , XMLEntityHandler* const   entityHandler
                                  , XMLErrorReporter* const   errReporter
                                  , XMLValidator* const       valToAdopt
                                  , GrammarResolver* const    grammarResolver
                                  , MemoryManager* const      manager)
{
    if (XMLString::equals(scannerName, XMLUni::fgWFXMLScanner))
        return new (manager) WFXMLScanner(docHandler, docTypeHandler, entityHandler, errReporter, valToAdopt, grammarResolver, manager);
    else if (XMLString::equals(scannerName, XMLUni::fgIGXMLScanner))
        return new (manager) IGXMLScanner(docHandler, docTypeHandler, entityHandler, errReporter, valToAdopt, grammarResolver, manager);
    else if (XMLString::equals(scannerName, XMLUni::fgSGXMLScanner))
        return new (manager) SGXMLScanner(docHandler, docTypeHandler, entityHandler, errReporter, valToAdopt, grammarResolver, manager);
    else if (XMLString::equals(scannerName, XMLUni::fgDGXMLScanner))
        return new (manager) DGXMLScanner(docHandler, docTypeHandler, entityHandler, errReporter, valToAdopt, grammarResolver, manager);

    // REVISIT: throw an exception or return a default one?
    return 0;
}


XERCES_CPP_NAMESPACE_END
