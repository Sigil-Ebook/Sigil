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
 * $Id: DTDAttDef.hpp 676911 2008-07-15 13:27:32Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DTDATTDEF_HPP)
#define XERCESC_INCLUDE_GUARD_DTDATTDEF_HPP

#include <xercesc/framework/XMLAttDef.hpp>

XERCES_CPP_NAMESPACE_BEGIN

//
//  This class is a derivative of the core XMLAttDef class. This class adds
//  any DTD specific data members and provides DTD specific implementations
//  of any underlying attribute def virtual methods.
//
//  In the DTD we don't do namespaces, so the attribute names are just the
//  QName literally from the DTD. This is what we return as the full name,
//  which is what is used to key these in any name keyed collections.
//
class VALIDATORS_EXPORT DTDAttDef : public XMLAttDef
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructors
    // -----------------------------------------------------------------------
    DTDAttDef(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    DTDAttDef
    (
        const   XMLCh* const           attName
        , const XMLAttDef::AttTypes    type = CData
        , const XMLAttDef::DefAttTypes defType = Implied
        , MemoryManager* const         manager = XMLPlatformUtils::fgMemoryManager
    );
    DTDAttDef
    (
        const   XMLCh* const           attName
        , const XMLCh* const           attValue
        , const XMLAttDef::AttTypes    type
        , const XMLAttDef::DefAttTypes defType
        , const XMLCh* const           enumValues = 0
        , MemoryManager* const         manager = XMLPlatformUtils::fgMemoryManager
    );
    ~DTDAttDef();


    // -----------------------------------------------------------------------
    //  Implementation of the XMLAttDef interface
    // -----------------------------------------------------------------------
    virtual const XMLCh* getFullName() const;

    //does nothing currently
    virtual void reset() {};

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    XMLSize_t getElemId() const;

    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------
    void setElemId(const XMLSize_t newId);
    void setName(const XMLCh* const newName);

    /***
     * Support for Serialization/De-serialization
     ***/
    DECL_XSERIALIZABLE(DTDAttDef)

private :
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    DTDAttDef(const DTDAttDef &);
    DTDAttDef& operator = (const  DTDAttDef&);

    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fElemId
    //      This is the id of the element (the id is into the element decl
    //      pool) of the element this attribute def said it belonged to.
    //      This is used later to link back to the element, mostly for
    //      validation purposes.
    //
    //  fName
    //      This is the name of the attribute. Since we don't do namespaces
    //      in the DTD, its just the fully qualified name.
    // -----------------------------------------------------------------------
    XMLSize_t       fElemId;
    XMLCh*          fName;
};


// ---------------------------------------------------------------------------
//  DTDAttDef: Implementation of the XMLAttDef interface
// ---------------------------------------------------------------------------
inline const XMLCh* DTDAttDef::getFullName() const
{
    return fName;
}


// ---------------------------------------------------------------------------
//  DTDAttDef: Getter methods
// ---------------------------------------------------------------------------
inline XMLSize_t DTDAttDef::getElemId() const
{
    return fElemId;
}

// ---------------------------------------------------------------------------
//  DTDAttDef: Setter methods
// ---------------------------------------------------------------------------
inline void DTDAttDef::setElemId(const XMLSize_t newId)
{
    fElemId = newId;
}


XERCES_CPP_NAMESPACE_END

#endif
