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
 * $Id: KeyValuePair.c 471747 2006-11-06 14:31:56Z amassari $
 */


// ---------------------------------------------------------------------------
//  Include
// ---------------------------------------------------------------------------
#if defined(XERCES_TMPLSINC)
#include <xercesc/util/KeyValuePair.hpp>
#endif

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  KeyValuePair: Constructors and Destructor
// ---------------------------------------------------------------------------
template <class TKey, class TValue> KeyValuePair<TKey,TValue>::KeyValuePair()
{
}

template <class TKey, class TValue> KeyValuePair<TKey,TValue>::
KeyValuePair(const TKey& key, const TValue& value) :

    fKey(key)
    , fValue(value)
{
}

template <class TKey, class TValue> KeyValuePair<TKey,TValue>::
KeyValuePair(const KeyValuePair<TKey,TValue>& toCopy) :

    fKey(toCopy.fKey)
    , fValue(toCopy.fValue)
{
}

template <class TKey, class TValue> KeyValuePair<TKey,TValue>::~KeyValuePair()
{
}


// ---------------------------------------------------------------------------
//  KeyValuePair: Getters
// ---------------------------------------------------------------------------
template <class TKey, class TValue> const TKey&
KeyValuePair<TKey,TValue>::getKey() const
{
    return fKey;

}

template <class TKey, class TValue> TKey& KeyValuePair<TKey,TValue>::getKey()
{
    return fKey;
}

template <class TKey, class TValue> const TValue&
KeyValuePair<TKey,TValue>::getValue() const
{
    return fValue;
}

template <class TKey, class TValue> TValue& KeyValuePair<TKey,TValue>::getValue()
{
    return fValue;
}


// ---------------------------------------------------------------------------
//  KeyValuePair: Setters
// ---------------------------------------------------------------------------
template <class TKey, class TValue> TKey&
KeyValuePair<TKey,TValue>::setKey(const TKey& newKey)
{
    fKey = newKey;
    return fKey;
}

template <class TKey, class TValue> TValue&
KeyValuePair<TKey,TValue>::setValue(const TValue& newValue)
{
    fValue = newValue;
    return fValue;
}

XERCES_CPP_NAMESPACE_END
