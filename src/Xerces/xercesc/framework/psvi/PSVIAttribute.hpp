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
 * $Id: PSVIAttribute.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_PSVIATTRIBUTE_HPP)
#define XERCESC_INCLUDE_GUARD_PSVIATTRIBUTE_HPP

#include <xercesc/framework/psvi/PSVIItem.hpp>
#include <xercesc/framework/psvi/XSSimpleTypeDefinition.hpp>
#include <xercesc/validators/datatype/DatatypeValidator.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/**
 * Represent the PSVI contributions for one attribute information item.
 * This is *always* owned by the scanner/parser object from which
 * it is obtained.  The validator will specify 
 * under what conditions it may be relied upon to have meaningful contents.
 */

// forward declarations
class XSAttributeDeclaration;

class XMLPARSER_EXPORT PSVIAttribute : public PSVIItem  
{
public:

    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    /** @name Constructors */
    //@{

    /**
      * The default constructor 
      *
      * @param  manager     The configurable memory manager
      */
    PSVIAttribute( MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    //@};

    /** @name Destructor */
    //@{
    ~PSVIAttribute();
    //@}

    //---------------------
    /** @name PSVIAttribute methods */

    //@{

    /**
     * An item isomorphic to the attribute declaration used to validate
     * this attribute.
     * 
     * @return  an attribute declaration
     */
    XSAttributeDeclaration *getAttributeDeclaration();
    
    /**
     * An item isomorphic to the type definition used to validate this element.
     * 
     * @return  a type declaration
     */
    XSTypeDefinition *getTypeDefinition();
    
    /**
     * If and only if that type definition is a simple type definition
     * with {variety} union, or a complex type definition whose {content type}
     * is a simple thype definition with {variety} union, then an item isomorphic
     * to that member of the union's {member type definitions} which actually
     * validated the element item's normalized value.
     * 
     * @return  a simple type declaration
     */
    XSSimpleTypeDefinition *getMemberTypeDefinition();
    
    //@}

    //----------------------------------
    /** methods needed by implementation */

    //@{

    /**
      * reset this object.  Intended to be called by
      * the implementation.
      */
    void reset(
            const XMLCh * const         valContext
            , PSVIItem::VALIDITY_STATE  state
            , PSVIItem::ASSESSMENT_TYPE assessmentType
            , XSSimpleTypeDefinition *  validatingType
            , XSSimpleTypeDefinition *  memberType
            , const XMLCh * const       defaultValue
            , const bool                isSpecified
            , XSAttributeDeclaration *  attrDecl
            , DatatypeValidator *       dv
        );

    /**
      * set the schema normalized value (and
      * implicitly the canonical value) of this object; intended to be used
      * by the implementation.
      */
    void setValue(const XMLCh * const       normalizedValue);

    /**
      * set VALIDITY_STATE to specified value; intended to be
      * called by implementation.
      */
    void updateValidity(VALIDITY_STATE newValue);

    //@}

private:

    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    PSVIAttribute(const PSVIAttribute&);
    PSVIAttribute & operator=(const PSVIAttribute &);


    // -----------------------------------------------------------------------
    //  data members
    // -----------------------------------------------------------------------
    // fAttributeDecl
    //      attribute declaration component that validated this attribute 
    // fDV
    //      implementation-specific datatype validator used to validate this attribute
    XSAttributeDeclaration *    fAttributeDecl;
    DatatypeValidator *         fDV;
};
inline PSVIAttribute::~PSVIAttribute() 
{
    fMemoryManager->deallocate((void *)fCanonicalValue);
}

inline XSAttributeDeclaration *PSVIAttribute::getAttributeDeclaration() 
{
    return fAttributeDecl;
}

inline XSTypeDefinition* PSVIAttribute::getTypeDefinition()
{
    return fType;
}

inline XSSimpleTypeDefinition* PSVIAttribute::getMemberTypeDefinition() 
{
    return fMemberType;
}

inline void PSVIAttribute::updateValidity(VALIDITY_STATE newValue)
{
    fValidityState = newValue;
}

XERCES_CPP_NAMESPACE_END

#endif
