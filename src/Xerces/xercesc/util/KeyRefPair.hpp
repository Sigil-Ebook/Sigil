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
 * $Id: KeyRefPair.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_KEYREFPAIR_HPP)
#define XERCESC_INCLUDE_GUARD_KEYREFPAIR_HPP


#include <xercesc/util/XMemory.hpp>

XERCES_CPP_NAMESPACE_BEGIN

template <class TKey, class TValue> class KeyRefPair : public XMemory
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        KeyRefPair();
        KeyRefPair(TKey* key, TValue* value);
        KeyRefPair(const KeyRefPair<TKey,TValue>* toCopy);
        KeyRefPair(const KeyRefPair<TKey,TValue>& toCopy);
        ~KeyRefPair();


        // -------------------------------------------------------------------
        //  Getters
        // -------------------------------------------------------------------
        const TKey* getKey() const;
        TKey* getKey();
        const TValue* getValue() const;
        TValue* getValue();


        // -------------------------------------------------------------------
        //  Setters
        // -------------------------------------------------------------------
        TKey* setKey(TKey* newKey);
        TValue* setValue(TValue* newValue);


    private :
        // unimplemented:        
        KeyRefPair<TKey,TValue>& operator=(const KeyRefPair<TKey,TValue>&);
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  fKey
        //      The object that represents the key of the pair
        //
        //  fValue
        //      The object that represents the value of the pair
        // -------------------------------------------------------------------
        TKey*    fKey;
        TValue*  fValue;
};

XERCES_CPP_NAMESPACE_END

#if !defined(XERCES_TMPLSINC)
#include <xercesc/util/KeyRefPair.c>
#endif

#endif
