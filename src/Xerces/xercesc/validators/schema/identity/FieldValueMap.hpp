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
 * $Id: FieldValueMap.hpp 708224 2008-10-27 16:02:26Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_FIELDVALUEMAP_HPP)
#define XERCESC_INCLUDE_GUARD_FIELDVALUEMAP_HPP

/**
  * This class maps values associated with fields of an identity constraint
  * that have successfully matched some string in an instance document.
  */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/ValueVectorOf.hpp>
#include <xercesc/util/RefArrayVectorOf.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Forward Declaration
// ---------------------------------------------------------------------------
class IC_Field;
class DatatypeValidator;


class VALIDATORS_EXPORT FieldValueMap : public XMemory
{
public:
    // -----------------------------------------------------------------------
    //  Constructors/Destructor
    // -----------------------------------------------------------------------
    FieldValueMap(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    FieldValueMap(const FieldValueMap& other);
	~FieldValueMap();

	// -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    DatatypeValidator* getDatatypeValidatorAt(const XMLSize_t index) const;
    DatatypeValidator* getDatatypeValidatorFor(const IC_Field* const key) const;
    XMLCh* getValueAt(const XMLSize_t index) const;
    XMLCh* getValueFor(const IC_Field* const key) const;
    IC_Field* keyAt(const XMLSize_t index) const;

	// -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------
    void put(IC_Field* const key, DatatypeValidator* const dv,
             const XMLCh* const value);

	// -----------------------------------------------------------------------
    //  Helper methods
    // -----------------------------------------------------------------------
    XMLSize_t size() const;
    bool indexOf(const IC_Field* const key, XMLSize_t& location) const;
    void clear();

private:
    // -----------------------------------------------------------------------
    //  Private helper methods
    // -----------------------------------------------------------------------
    void cleanUp();

    // -----------------------------------------------------------------------
    //  Unimplemented operators
    // -----------------------------------------------------------------------
    FieldValueMap& operator= (const FieldValueMap& other);

    // -----------------------------------------------------------------------
    //  Data
    // -----------------------------------------------------------------------
    ValueVectorOf<IC_Field*>*          fFields;
    ValueVectorOf<DatatypeValidator*>* fValidators;
    RefArrayVectorOf<XMLCh>*           fValues;
    MemoryManager*                     fMemoryManager;
};


// ---------------------------------------------------------------------------
//  FieldValueMap: Getter methods
// ---------------------------------------------------------------------------
inline DatatypeValidator*
FieldValueMap::getDatatypeValidatorAt(const XMLSize_t index) const {

    if (fValidators) {
        return fValidators->elementAt(index);
    }

    return 0;
}

inline DatatypeValidator*
FieldValueMap::getDatatypeValidatorFor(const IC_Field* const key) const {

    XMLSize_t location;
    if (fValidators && indexOf(key, location)) {
        return fValidators->elementAt(location);
    }

    return 0;
}

inline XMLCh* FieldValueMap::getValueAt(const XMLSize_t index) const {

    if (fValues) {
        return fValues->elementAt(index);
    }

    return 0;
}

inline XMLCh* FieldValueMap::getValueFor(const IC_Field* const key) const {

    XMLSize_t location;
    if (fValues && indexOf(key, location)) {
        return fValues->elementAt(location);
    }

    return 0;
}

inline IC_Field* FieldValueMap::keyAt(const XMLSize_t index) const {

    if (fFields) {
        return fFields->elementAt(index);
    }

    return 0;
}

// ---------------------------------------------------------------------------
//  FieldValueMap: Helper methods
// ---------------------------------------------------------------------------
inline XMLSize_t FieldValueMap::size() const {

    if (fFields) {
        return fFields->size();
    }

    return 0;
}

// ---------------------------------------------------------------------------
//  FieldValueMap: Setter methods
// ---------------------------------------------------------------------------
inline void FieldValueMap::put(IC_Field* const key,
                               DatatypeValidator* const dv,
                               const XMLCh* const value) {

    if (!fFields) {
        fFields = new (fMemoryManager) ValueVectorOf<IC_Field*>(4, fMemoryManager);
        fValidators = new (fMemoryManager) ValueVectorOf<DatatypeValidator*>(4, fMemoryManager);
        fValues = new (fMemoryManager) RefArrayVectorOf<XMLCh>(4, true, fMemoryManager);
    }

    XMLSize_t keyIndex;
    bool bFound=indexOf(key, keyIndex);

    if (!bFound) {

        fFields->addElement(key);
        fValidators->addElement(dv);
        fValues->addElement(XMLString::replicate(value, fMemoryManager));
    }
    else {
        fValidators->setElementAt(dv, keyIndex);
        fValues->setElementAt(XMLString::replicate(value, fMemoryManager), keyIndex);
    }
}

XERCES_CPP_NAMESPACE_END

#endif

/**
  * End of file FieldValueMap.hpp
  */

