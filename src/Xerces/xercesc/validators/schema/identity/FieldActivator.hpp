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
 * $Id: FieldActivator.hpp 679340 2008-07-24 10:28:29Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_FIELDACTIVATOR_HPP)
#define XERCESC_INCLUDE_GUARD_FIELDACTIVATOR_HPP

/**
  * This class is responsible for activating fields within a specific scope;
  * the caller merely requests the fields to be activated.
  */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/ValueHashTableOf.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Forward Declaration
// ---------------------------------------------------------------------------
class IdentityConstraint;
class XPathMatcher;
class ValueStoreCache;
class IC_Field;
class XPathMatcherStack;


class VALIDATORS_EXPORT FieldActivator : public XMemory
{
public:
    // -----------------------------------------------------------------------
    //  Constructors/Destructor
    // -----------------------------------------------------------------------
    FieldActivator(ValueStoreCache* const valueStoreCache,
                   XPathMatcherStack* const matcherStack,
                   MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
	FieldActivator(const FieldActivator& other);
	~FieldActivator();

    // -----------------------------------------------------------------------
    //  Operator methods
    // -----------------------------------------------------------------------
    FieldActivator& operator =(const FieldActivator& other);

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    bool getMayMatch(IC_Field* const field);

    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------
    void setValueStoreCache(ValueStoreCache* const other);
    void setMatcherStack(XPathMatcherStack* const matcherStack);
    void setMayMatch(IC_Field* const field, bool value);

	// -----------------------------------------------------------------------
    //  Activation methods
    // -----------------------------------------------------------------------
    /**
      * Start the value scope for the specified identity constraint. This
      * method is called when the selector matches in order to initialize
      * the value store.
      */
    void startValueScopeFor(const IdentityConstraint* const ic, const int initialDepth);

    /**
      * Request to activate the specified field. This method returns the
      * matcher for the field.
      */
    XPathMatcher* activateField(IC_Field* const field, const int initialDepth);

    /**
      * Ends the value scope for the specified identity constraint.
      */
    void endValueScopeFor(const IdentityConstraint* const ic, const int initialDepth);

private:
    // -----------------------------------------------------------------------
    //  Data
    // -----------------------------------------------------------------------
    ValueStoreCache*                   fValueStoreCache;
    XPathMatcherStack*                 fMatcherStack;
    ValueHashTableOf<bool, PtrHasher>* fMayMatch;
    MemoryManager*                     fMemoryManager;
};


// ---------------------------------------------------------------------------
//  FieldActivator: Getter methods
// ---------------------------------------------------------------------------
inline bool FieldActivator::getMayMatch(IC_Field* const field) {

    return fMayMatch->get(field);
}

// ---------------------------------------------------------------------------
//  FieldActivator: Setter methods
// ---------------------------------------------------------------------------
inline void FieldActivator::setValueStoreCache(ValueStoreCache* const other) {

    fValueStoreCache = other;
}

inline void
FieldActivator::setMatcherStack(XPathMatcherStack* const matcherStack) {

    fMatcherStack = matcherStack;
}

inline void FieldActivator::setMayMatch(IC_Field* const field, bool value) {

    fMayMatch->put(field, value);
}

XERCES_CPP_NAMESPACE_END

#endif

/**
  * End of file FieldActivator.hpp
  */
