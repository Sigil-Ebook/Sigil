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
 * $Id: IC_Unique.cpp 471747 2006-11-06 14:31:56Z amassari $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/schema/identity/IC_Unique.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  IC_Unique: Constructors and Destructor
// ---------------------------------------------------------------------------
IC_Unique::IC_Unique(const XMLCh* const identityConstraintName,
                     const XMLCh* const elemName,
                     MemoryManager* const manager)
    : IdentityConstraint(identityConstraintName, elemName, manager)
{
}


IC_Unique::~IC_Unique()
{
}

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_TOCREATE(IC_Unique)

void IC_Unique::serialize(XSerializeEngine& serEng)
{
    IdentityConstraint::serialize(serEng);

    //no data
}

IC_Unique::IC_Unique(MemoryManager* const manager)
:IdentityConstraint(0, 0, manager)
{
}

XERCES_CPP_NAMESPACE_END

/**
  * End of file IC_Unique.cpp
  */


