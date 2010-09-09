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
 * $Id: XMLInitializer.cpp 635560 2008-03-10 14:10:09Z borisk $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/XMLInitializer.hpp>
#include <xercesc/util/PlatformUtils.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  XMLInitializer: Initialization methods
// ---------------------------------------------------------------------------
void XMLInitializer::initializeStaticData()
{
    try
    {
        // Note that in some cases the order of initialization can be
        // important.
        //

        // Core
        //
        initializeEncodingValidator();
        initializeXMLException();
        initializeXMLScanner();
        initializeXMLValidator();

        // Regex
        //
        initializeRangeTokenMap();
        initializeRegularExpression();

        // DTD
        //
        initializeDTDGrammar();

        // Schema
        //
        initializeXSDErrorReporter();
        initializeDatatypeValidatorFactory();
        initializeGeneralAttributeCheck();
        initializeXSValue();
        initializeComplexTypeInfo();

        // DOM
        //
        initializeDOMImplementationRegistry();
        initializeDOMImplementationImpl();
        initializeDOMDocumentTypeImpl();
        initializeDOMNodeListImpl();
        initializeDOMNormalizer();
    }
    catch(...) {
        XMLPlatformUtils::panic(PanicHandler::Panic_AllStaticInitErr);
    }
}


void XMLInitializer::terminateStaticData()
{
    // Terminate in the reverse order of initialization. There shouldn't
    // be any exceptions and if there are, we can't do anything about them
    // since we are no longer initialized (think of it as throwing from
    // a destructor).
    //

    // DOM
    //
    terminateDOMNormalizer();
    terminateDOMNodeListImpl();
    terminateDOMDocumentTypeImpl();
    terminateDOMImplementationImpl();
    terminateDOMImplementationRegistry();

    // Schema
    //
    terminateComplexTypeInfo();
    terminateXSValue();
    terminateGeneralAttributeCheck();
    terminateDatatypeValidatorFactory();
    terminateXSDErrorReporter();

    // DTD
    //
    terminateDTDGrammar();

    // Regex
    //
    terminateRegularExpression();
    terminateRangeTokenMap();

    // Core
    //
    terminateXMLValidator();
    terminateXMLScanner();
    terminateXMLException();
    terminateEncodingValidator();
}


XERCES_CPP_NAMESPACE_END
