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
 * $Id: IC_Selector.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_IC_SELECTOR_HPP)
#define XERCESC_INCLUDE_GUARD_IC_SELECTOR_HPP


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/schema/identity/XPathMatcher.hpp>

#include <xercesc/internal/XSerializable.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Forward Declaration
// ---------------------------------------------------------------------------
class FieldActivator;


class VALIDATORS_EXPORT IC_Selector : public XSerializable, public XMemory
{
public:
    // -----------------------------------------------------------------------
    //  Constructors/Destructor
    // -----------------------------------------------------------------------
    IC_Selector(XercesXPath* const xpath,
                IdentityConstraint* const identityConstraint);
	~IC_Selector();

    // -----------------------------------------------------------------------
    //  operators
    // -----------------------------------------------------------------------
    bool operator== (const IC_Selector& other) const;
    bool operator!= (const IC_Selector& other) const;

	// -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    XercesXPath* getXPath() const { return fXPath; }
    IdentityConstraint* getIdentityConstraint() const { return fIdentityConstraint; }

	// -----------------------------------------------------------------------
    //  Factory methods
    // -----------------------------------------------------------------------
    XPathMatcher* createMatcher(FieldActivator* const fieldActivator,
                                const int initialDepth,
                                MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    /***
     * Support for Serialization/De-serialization
     ***/
    DECL_XSERIALIZABLE(IC_Selector)

    IC_Selector(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    IC_Selector(const IC_Selector& other);
    IC_Selector& operator= (const IC_Selector& other);

    // -----------------------------------------------------------------------
    //  Data members
    // -----------------------------------------------------------------------
    XercesXPath*        fXPath;
    IdentityConstraint* fIdentityConstraint;
};


class VALIDATORS_EXPORT SelectorMatcher : public XPathMatcher
{
public:
    // -----------------------------------------------------------------------
    //  Constructors/Destructor
    // -----------------------------------------------------------------------
    ~SelectorMatcher() {}

    int getInitialDepth() const { return fInitialDepth; }

    // -----------------------------------------------------------------------
    //  XMLDocumentHandler methods
    // -----------------------------------------------------------------------
    virtual void startDocumentFragment();
    virtual void startElement(const XMLElementDecl& elemDecl,
                              const unsigned int urlId,
                              const XMLCh* const elemPrefix,
		                      const RefVectorOf<XMLAttr>& attrList,
                              const XMLSize_t attrCount,
                              ValidationContext* validationContext = 0);
    virtual void endElement(const XMLElementDecl& elemDecl,
                            const XMLCh* const elemContent,
                            ValidationContext* validationContext = 0,
                            DatatypeValidator* actualValidator = 0);

private:
    // -----------------------------------------------------------------------
    //  Constructors/Destructor
    // -----------------------------------------------------------------------
    SelectorMatcher(XercesXPath* const anXPath,
                    IC_Selector* const selector,
                    FieldActivator* const fieldActivator,
                    const int initialDepth,
                    MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    SelectorMatcher(const SelectorMatcher& other);
    SelectorMatcher& operator= (const SelectorMatcher& other);

    // -----------------------------------------------------------------------
    //  Friends
    // -----------------------------------------------------------------------
    friend class IC_Selector;

    // -----------------------------------------------------------------------
    //  Data members
    // -----------------------------------------------------------------------
    int             fInitialDepth;
    int             fElementDepth;
    int             fMatchedDepth;
    IC_Selector*    fSelector;
    FieldActivator* fFieldActivator;
};

XERCES_CPP_NAMESPACE_END

#endif

/**
  * End of file IC_Selector.hpp
  */

