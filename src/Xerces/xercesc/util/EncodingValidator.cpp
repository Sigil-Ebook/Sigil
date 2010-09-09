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
 * $Id: EncodingValidator.cpp 635560 2008-03-10 14:10:09Z borisk $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/EncodingValidator.hpp>
#include <xercesc/internal/IANAEncodings.hpp>
#include <xercesc/util/XMLInitializer.hpp>

XERCES_CPP_NAMESPACE_BEGIN

EncodingValidator* EncodingValidator::fInstance = 0;

void XMLInitializer::initializeEncodingValidator()
{
    EncodingValidator::fInstance = new EncodingValidator();
}

void XMLInitializer::terminateEncodingValidator()
{
    delete EncodingValidator::fInstance;
    EncodingValidator::fInstance = 0;
}

// ---------------------------------------------------------------------------
//  EncodingValidator: Constructors and Destructor
// ---------------------------------------------------------------------------
EncodingValidator::EncodingValidator() :
    fEncodingRegistry(0)
{
    initializeRegistry();
}

EncodingValidator::~EncodingValidator() {

    delete fEncodingRegistry;
    fEncodingRegistry = 0;
}

// ---------------------------------------------------------------------------
//  EncodingValidator: Validation methods
// ---------------------------------------------------------------------------
bool EncodingValidator::isValidEncoding(const XMLCh* const encName) {

    if (fEncodingRegistry->containsKey(encName))
		return true;

	return false;
}


// ---------------------------------------------------------------------------
//  EncodingValidator: Initialization methods
// ---------------------------------------------------------------------------
void EncodingValidator::initializeRegistry() {

    fEncodingRegistry = new ValueHashTableOf<bool>(109);

    for (unsigned int i=0; i < gEncodingArraySize; i++) {
        fEncodingRegistry->put((void*) gEncodingArray[i], true);
    }
}


// ---------------------------------------------------------------------------
//  EncodingValidator: Instance methods
// ---------------------------------------------------------------------------
EncodingValidator* EncodingValidator::instance()
{
    return fInstance;
}

XERCES_CPP_NAMESPACE_END

/**
  * End of file EncodingValidator.cpp
  */
