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
 * $Id: Hashers.hpp 679382 2008-07-24 12:09:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_HASHERS_HPP)
#define XERCESC_INCLUDE_GUARD_HASHERS_HPP

#include <xercesc/util/XMLString.hpp>

XERCES_CPP_NAMESPACE_BEGIN

//
// Common hashers. Only widely-used hashers should be placed here.
//

/**
 * Hasher for keys that are const XMLCh*.
 */
struct StringHasher
{
  /**
   * Returns a hash value based on the key
   *
   * @param key the key to be hashed
   * @param mod the modulus the hasher should use
   */
  XMLSize_t getHashVal(const void* key, XMLSize_t mod) const
  {
    return XMLString::hash ((const XMLCh*)key, mod);
  }

  /**
   * Compares two keys and determines if they are semantically equal
   *
   * @param key1 the first key to be compared
   * @param key2 the second key to be compared
   *
   * @return true if they are equal
   */
  bool equals(const void *const key1, const void *const key2) const
  {
    return XMLString::equals ((const XMLCh*)key1, (const XMLCh*)key2);
  }
};

/**
 * Hasher for keys that are pointers.
 */
struct PtrHasher
{
  /**
   * Returns a hash value based on the key
   *
   * @param key the key to be hashed
   * @param mod the modulus the hasher should use
   */
  XMLSize_t getHashVal(const void* key, XMLSize_t mod) const
  {
    return ((XMLSize_t)key) % mod;
  }

  /**
   * Compares two keys and determines if they are semantically equal
   *
   * @param key1 the first key to be compared
   * @param key2 the second key to be compared
   *
   * @return true if they are equal
   */
  bool equals(const void *const key1, const void *const key2) const
  {
    return key1 == key2;
  }
};

XERCES_CPP_NAMESPACE_END

#endif
