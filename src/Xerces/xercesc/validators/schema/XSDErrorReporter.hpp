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
 * $Id: XSDErrorReporter.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XSDERRORREPORTER_HPP)
#define XERCESC_INCLUDE_GUARD_XSDERRORREPORTER_HPP

#include <xercesc/util/XMemory.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class Locator;
class XMLErrorReporter;


/**
 *  This class reports schema errors
 */
class VALIDATORS_EXPORT XSDErrorReporter : public XMemory
{
public:
    // -----------------------------------------------------------------------
    //  Constructors are hidden, only the virtual destructor is exposed
    // -----------------------------------------------------------------------
    XSDErrorReporter(XMLErrorReporter* const errorReporter = 0);

    virtual ~XSDErrorReporter()
    {
    }

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    bool getExitOnFirstFatal() const;

    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------
    void setErrorReporter(XMLErrorReporter* const errorReporter);
    void setExitOnFirstFatal(const bool newValue);

    // -----------------------------------------------------------------------
    //  Report error methods
    // -----------------------------------------------------------------------
    void emitError(const unsigned int toEmit,
                   const XMLCh* const msgDomain,
                   const Locator* const aLocator);
    void emitError(const unsigned int toEmit,
                   const XMLCh* const msgDomain,
                   const Locator* const aLocator,
                   const XMLCh* const text1,
                   const XMLCh* const text2 = 0,
                   const XMLCh* const text3 = 0,
                   const XMLCh* const text4 = 0,
                   MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
                   );
    void emitError(const XMLException&  except,
                   const Locator* const aLocator);

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and destructor
    // -----------------------------------------------------------------------
    XSDErrorReporter(const XSDErrorReporter&);
    XSDErrorReporter& operator=(const XSDErrorReporter&);

    // -----------------------------------------------------------------------
    //  Private data members
    // -----------------------------------------------------------------------
    bool              fExitOnFirstFatal;
    XMLErrorReporter* fErrorReporter;
};


// ---------------------------------------------------------------------------
//  XSDErrorReporter: Getter methods
// ---------------------------------------------------------------------------
inline bool XSDErrorReporter::getExitOnFirstFatal() const
{
    return fExitOnFirstFatal;
}

// ---------------------------------------------------------------------------
//  XSDErrorReporter: Setter methods
// ---------------------------------------------------------------------------
inline void XSDErrorReporter::setExitOnFirstFatal(const bool newValue)
{
    fExitOnFirstFatal = newValue;
}

inline void XSDErrorReporter::setErrorReporter(XMLErrorReporter* const errorReporter)
{
    fErrorReporter = errorReporter;
}

XERCES_CPP_NAMESPACE_END

#endif
