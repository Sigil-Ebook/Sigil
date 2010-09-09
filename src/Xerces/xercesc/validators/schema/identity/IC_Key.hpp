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
 * $Id: IC_Key.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_IC_KEY_HPP)
#define XERCESC_INCLUDE_GUARD_IC_KEY_HPP


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/schema/identity/IdentityConstraint.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class VALIDATORS_EXPORT IC_Key: public IdentityConstraint
{
public:
    // -----------------------------------------------------------------------
    //  Constructors/Destructor
    // -----------------------------------------------------------------------
    IC_Key(const XMLCh* const identityConstraintName,
           const XMLCh* const elemName,
           MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
	~IC_Key();

	// -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    short getType() const;

    /***
     * Support for Serialization/De-serialization
     ***/
    DECL_XSERIALIZABLE(IC_Key)

    IC_Key(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    IC_Key(const IC_Key& other);
    IC_Key& operator= (const IC_Key& other);
};


// ---------------------------------------------------------------------------
//  IC_Key: Getter methods
// ---------------------------------------------------------------------------
inline short IC_Key::getType() const {

    return IdentityConstraint::ICType_KEY;
}

XERCES_CPP_NAMESPACE_END

#endif

/**
  * End of file IC_Key.hpp
  */

