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
 * $Id: SchemaAttDef.hpp 676911 2008-07-15 13:27:32Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_SCHEMAATTDEF_HPP)
#define XERCESC_INCLUDE_GUARD_SCHEMAATTDEF_HPP

#include <xercesc/util/XMLString.hpp>
#include <xercesc/framework/XMLAttDef.hpp>
#include <xercesc/util/ValueVectorOf.hpp>
#include <xercesc/validators/datatype/DatatypeValidator.hpp>
#include <xercesc/validators/datatype/UnionDatatypeValidator.hpp>
#include <xercesc/validators/schema/PSVIDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class DatatypeValidator;
class QName;
class ComplexTypeInfo;
//
//  This class is a derivative of the core XMLAttDef class. This class adds
//  any Schema specific data members and provides Schema specific implementations
//  of any underlying attribute def virtual methods.
//
class VALIDATORS_EXPORT SchemaAttDef : public XMLAttDef
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructors
    // -----------------------------------------------------------------------
    SchemaAttDef(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    SchemaAttDef
    (
          const XMLCh* const           prefix
        , const XMLCh* const           localPart
        , const int                    uriId
        , const XMLAttDef::AttTypes    type = CData
        , const XMLAttDef::DefAttTypes defType = Implied
        , MemoryManager* const         manager = XMLPlatformUtils::fgMemoryManager
    );
    SchemaAttDef
    (
          const XMLCh* const           prefix
        , const XMLCh* const           localPart
        , const int                    uriId
        , const XMLCh* const           attValue
        , const XMLAttDef::AttTypes    type
        , const XMLAttDef::DefAttTypes defType
        , const XMLCh* const           enumValues = 0
        , MemoryManager* const         manager = XMLPlatformUtils::fgMemoryManager
    );
    SchemaAttDef
    (
          const SchemaAttDef*                   other
    );
    virtual ~SchemaAttDef();

    // -----------------------------------------------------------------------
    //  Implementation of the XMLAttDef interface
    // -----------------------------------------------------------------------
    virtual const XMLCh* getFullName() const;
    virtual void reset();

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    XMLSize_t getElemId() const;
    QName* getAttName() const;
    DatatypeValidator* getDatatypeValidator() const;
    ValueVectorOf<unsigned int>* getNamespaceList() const;
    const SchemaAttDef* getBaseAttDecl() const;
    SchemaAttDef* getBaseAttDecl();
    PSVIDefs::PSVIScope getPSVIScope() const;

    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------
    void setElemId(const XMLSize_t newId);
    void setAttName
    (
        const XMLCh* const        prefix
       ,const XMLCh* const        localPart
       ,const int                 uriId = -1
    );
    void setDatatypeValidator(DatatypeValidator* newDatatypeValidator);    
    void setBaseAttDecl(SchemaAttDef* const attDef);
    void setPSVIScope(const PSVIDefs::PSVIScope toSet);
    
    void setNamespaceList(const ValueVectorOf<unsigned int>* const toSet);
    void resetNamespaceList();
    void setEnclosingCT(ComplexTypeInfo* complexTypeInfo);
    /***
     * Support for Serialization/De-serialization
     ***/
    DECL_XSERIALIZABLE(SchemaAttDef)

private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    SchemaAttDef(const SchemaAttDef&);
    SchemaAttDef& operator=(const SchemaAttDef&);

    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fElemId
    //      This is the id of the element (the id is into the element decl
    //      pool) of the element this attribute def said it belonged to.
    //      This is used later to link back to the element, mostly for
    //      validation purposes.
    //
    //  fAttName
    //      This is the name of the attribute.
    //
    //  fDatatypeValidator
    //      The DatatypeValidator used to validate this attribute type.        
    //    
    //  fNamespaceList
    //      The list of namespace values for a wildcard attribute
    //    
    //  fBaseAttDecl
    //      The base attribute declaration that this attribute is based on
    //      NOTE: we do not have a notion of attribute use, so in the case
    //      of ref'd attributes and inherited attributes, we make a copy
    //      of the actual attribute declaration. The fBaseAttDecl stores that
    //      declaration, and will be helpful when we build the XSModel (i.e
    //      easy access the XSAnnotation object).
    // -----------------------------------------------------------------------
    XMLSize_t                    fElemId;

    PSVIDefs::PSVIScope          fPSVIScope;

    QName*                       fAttName;
    DatatypeValidator*           fDatatypeValidator;    
    ValueVectorOf<unsigned int>* fNamespaceList;
    SchemaAttDef*                fBaseAttDecl;
};


// ---------------------------------------------------------------------------
//  SchemaAttDef: Getter methods
// ---------------------------------------------------------------------------
inline XMLSize_t SchemaAttDef::getElemId() const
{
    return fElemId;
}


inline QName* SchemaAttDef::getAttName() const
{
    return fAttName;
}

inline DatatypeValidator* SchemaAttDef::getDatatypeValidator() const
{
    return fDatatypeValidator;
}

inline ValueVectorOf<unsigned int>*
SchemaAttDef::getNamespaceList() const {
    return fNamespaceList;
}

inline SchemaAttDef* SchemaAttDef::getBaseAttDecl()
{
    return fBaseAttDecl;
}

inline const SchemaAttDef* SchemaAttDef::getBaseAttDecl() const
{
    return fBaseAttDecl;
}

inline PSVIDefs::PSVIScope SchemaAttDef::getPSVIScope() const
{
    return fPSVIScope;
}

// ---------------------------------------------------------------------------
//  SchemaAttDef: Setter methods
// ---------------------------------------------------------------------------
inline void SchemaAttDef::setElemId(const XMLSize_t newId)
{
    fElemId = newId;
}

inline void SchemaAttDef::setDatatypeValidator(DatatypeValidator* newDatatypeValidator)
{
    fDatatypeValidator = newDatatypeValidator;
}

inline void SchemaAttDef::resetNamespaceList() {

    if (fNamespaceList && fNamespaceList->size()) {
        fNamespaceList->removeAllElements();
    }
}

inline void SchemaAttDef::setNamespaceList(const ValueVectorOf<unsigned int>* const toSet) {

    if (toSet && toSet->size()) {

        if (fNamespaceList) {
            *fNamespaceList = *toSet;
        }
        else {
            fNamespaceList = new (getMemoryManager()) ValueVectorOf<unsigned int>(*toSet);
        }
    }
    else  {
        resetNamespaceList();
    }
}

inline void SchemaAttDef::reset() {    
}

inline void SchemaAttDef::setEnclosingCT(ComplexTypeInfo*)
{
}

inline void SchemaAttDef::setBaseAttDecl(SchemaAttDef* const attDef)
{
    fBaseAttDecl = attDef;
}

inline void SchemaAttDef::setPSVIScope(const PSVIDefs::PSVIScope toSet)
{
    fPSVIScope = toSet;
}

XERCES_CPP_NAMESPACE_END

#endif
