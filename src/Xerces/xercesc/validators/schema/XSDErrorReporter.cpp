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

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/framework/XMLErrorCodes.hpp>
#include <xercesc/framework/XMLValidityCodes.hpp>
#include <xercesc/framework/XMLErrorReporter.hpp>
#include <xercesc/util/XMLMsgLoader.hpp>
#include <xercesc/util/XMLInitializer.hpp>
#include <xercesc/validators/schema/XSDErrorReporter.hpp>
#include <xercesc/validators/schema/XSDLocator.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Local static data
// ---------------------------------------------------------------------------
static XMLMsgLoader*  gErrMsgLoader = 0;
static XMLMsgLoader*  gValidMsgLoader = 0;

void XMLInitializer::initializeXSDErrorReporter()
{
    gErrMsgLoader = XMLPlatformUtils::loadMsgSet(XMLUni::fgXMLErrDomain);

    if (!gErrMsgLoader)
      XMLPlatformUtils::panic(PanicHandler::Panic_CantLoadMsgDomain);

    gValidMsgLoader = XMLPlatformUtils::loadMsgSet(XMLUni::fgValidityDomain);

    if (!gValidMsgLoader)
      XMLPlatformUtils::panic(PanicHandler::Panic_CantLoadMsgDomain);
}

void XMLInitializer::terminateXSDErrorReporter()
{
    delete gErrMsgLoader;
    gErrMsgLoader = 0;

    delete gValidMsgLoader;
    gValidMsgLoader = 0;
}

// ---------------------------------------------------------------------------
//  XSDErrorReporter: Constructors and Destructor
// ---------------------------------------------------------------------------
XSDErrorReporter::XSDErrorReporter(XMLErrorReporter* const errorReporter) :
    fExitOnFirstFatal(false)
    , fErrorReporter(errorReporter)
{

}


// ---------------------------------------------------------------------------
//  XSDErrorReporter: Error reporting
// ---------------------------------------------------------------------------
void XSDErrorReporter::emitError(const unsigned int toEmit,
                                 const XMLCh* const msgDomain,
                                 const Locator* const aLocator)
{
    // Bump the error count if it is not a warning
//    if (XMLErrs::errorType(toEmit) != XMLErrorReporter::ErrType_Warning)
//        incrementErrorCount();

    //
    //  Load the message into alocal and replace any tokens found in
    //  the text.
    //
    const XMLSize_t msgSize = 1023;
    XMLCh errText[msgSize + 1];
    XMLMsgLoader* msgLoader = gErrMsgLoader;
    XMLErrorReporter::ErrTypes errType = XMLErrs::errorType((XMLErrs::Codes) toEmit);

    if (XMLString::equals(msgDomain, XMLUni::fgValidityDomain)) {

        errType = XMLValid::errorType((XMLValid::Codes) toEmit);
        msgLoader = gValidMsgLoader;
    }

    if (!msgLoader->loadMsg(toEmit, errText, msgSize))
    {
                // <TBD> Should probably load a default message here
    }

    if (fErrorReporter)
        fErrorReporter->error(toEmit, msgDomain, errType, errText, aLocator->getSystemId(),
                              aLocator->getPublicId(), aLocator->getLineNumber(),
                              aLocator->getColumnNumber());

    // Bail out if its fatal an we are to give up on the first fatal error
    if (errType == XMLErrorReporter::ErrType_Fatal && fExitOnFirstFatal)
        throw (XMLErrs::Codes) toEmit;
}

void XSDErrorReporter::emitError(const unsigned int toEmit,
                                 const XMLCh* const msgDomain,
                                 const Locator* const aLocator,
                                 const XMLCh* const text1,
                                 const XMLCh* const text2,
                                 const XMLCh* const text3,
                                 const XMLCh* const text4,
                                 MemoryManager* const manager)
{
    // Bump the error count if it is not a warning
//    if (XMLErrs::errorType(toEmit) != XMLErrorReporter::ErrType_Warning)
//        incrementErrorCount();

    //
    //  Load the message into alocal and replace any tokens found in
    //  the text.
    //
    const XMLSize_t maxChars = 2047;
    XMLCh errText[maxChars + 1];
    XMLMsgLoader* msgLoader = gErrMsgLoader;
    XMLErrorReporter::ErrTypes errType = XMLErrs::errorType((XMLErrs::Codes) toEmit);

    if (XMLString::equals(msgDomain, XMLUni::fgValidityDomain)) {

        errType = XMLValid::errorType((XMLValid::Codes) toEmit);
        msgLoader = gValidMsgLoader;
    }

    if (!msgLoader->loadMsg(toEmit, errText, maxChars, text1, text2, text3, text4, manager))
    {
                // <TBD> Should probably load a default message here
    }

    if (fErrorReporter)
        fErrorReporter->error(toEmit, msgDomain, errType, errText, aLocator->getSystemId(),
                              aLocator->getPublicId(), aLocator->getLineNumber(),
                              aLocator->getColumnNumber());

    // Bail out if its fatal an we are to give up on the first fatal error
    if (errType == XMLErrorReporter::ErrType_Fatal && fExitOnFirstFatal)
        throw (XMLErrs::Codes) toEmit;
}

void XSDErrorReporter::emitError(const XMLException&  except,
                                 const Locator* const aLocator)
{
    const XMLCh* const  errText = except.getMessage();
    const unsigned int  toEmit = except.getCode();
    //Before the code was modified to call this routine it used to use
    //the XMLErrs::DisplayErrorMessage error message, which is just {'0'}
    //and that error message has errType of Error.  So to be consistent
    //with previous behaviour set the errType to be Error instead of
    //getting the error type off of the exception.
    //XMLErrorReporter::ErrTypes errType = XMLErrs::errorType((XMLErrs::Codes) toEmit);
    XMLErrorReporter::ErrTypes errType = XMLErrorReporter::ErrType_Error;

    if (fErrorReporter)
        fErrorReporter->error(toEmit, XMLUni::fgExceptDomain, errType, errText, aLocator->getSystemId(),
                              aLocator->getPublicId(), aLocator->getLineNumber(),
                              aLocator->getColumnNumber());

    // Bail out if its fatal an we are to give up on the first fatal error
    //if (errType == XMLErrorReporter::ErrType_Fatal && fExitOnFirstFatal)
    //    throw (XMLErrs::Codes) toEmit;
}

XERCES_CPP_NAMESPACE_END
