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
 * $Id: XMLInternalErrorHandler.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XMLINTERNALERRORHANDLER_HPP)
#define XERCESC_INCLUDE_GUARD_XMLINTERNALERRORHANDLER_HPP

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/sax/ErrorHandler.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XMLInternalErrorHandler : public ErrorHandler
{
public:
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    XMLInternalErrorHandler(ErrorHandler* userHandler = 0) :
       fSawWarning(false),
       fSawError(false),
       fSawFatal(false),
       fUserErrorHandler(userHandler)
    {
    }

    ~XMLInternalErrorHandler()
    {
    }

    // -----------------------------------------------------------------------
    //  Implementation of the error handler interface
    // -----------------------------------------------------------------------
    void warning(const SAXParseException& toCatch);
    void error(const SAXParseException& toCatch);
    void fatalError(const SAXParseException& toCatch);
    void resetErrors();

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    bool getSawWarning() const;
    bool getSawError() const;
    bool getSawFatal() const;

    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fSawWarning
    //      This is set if we get any warning, and is queryable via a getter
    //      method.
    //
    //  fSawError
    //      This is set if we get any errors, and is queryable via a getter
    //      method.
    //
    //  fSawFatal
    //      This is set if we get any fatal, and is queryable via a getter
    //      method.
    //
    //  fUserErrorHandler
    //      This is the error handler from user
    // -----------------------------------------------------------------------
    bool    fSawWarning;
    bool    fSawError;
    bool    fSawFatal;
    ErrorHandler* fUserErrorHandler;

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XMLInternalErrorHandler(const XMLInternalErrorHandler&);
    XMLInternalErrorHandler& operator=(const XMLInternalErrorHandler&);
};

inline bool XMLInternalErrorHandler::getSawWarning() const
{
    return fSawWarning;
}

inline bool XMLInternalErrorHandler::getSawError() const
{
    return fSawError;
}

inline bool XMLInternalErrorHandler::getSawFatal() const
{
    return fSawFatal;
}

inline void XMLInternalErrorHandler::warning(const SAXParseException& toCatch)
{
    fSawWarning = true;
    if (fUserErrorHandler)
        fUserErrorHandler->warning(toCatch);
}

inline void XMLInternalErrorHandler::error(const SAXParseException& toCatch)
{
    fSawError = true;
    if (fUserErrorHandler)
        fUserErrorHandler->error(toCatch);
}

inline void XMLInternalErrorHandler::fatalError(const SAXParseException& toCatch)
{
    fSawFatal = true;
    if (fUserErrorHandler)
        fUserErrorHandler->fatalError(toCatch);
}

inline void XMLInternalErrorHandler::resetErrors()
{
    fSawWarning = false;
    fSawError = false;
    fSawFatal = false;
}

XERCES_CPP_NAMESPACE_END

#endif
