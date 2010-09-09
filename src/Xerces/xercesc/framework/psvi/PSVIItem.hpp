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
 * $Id: PSVIItem.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_PSVIITEM_HPP)
#define XERCESC_INCLUDE_GUARD_PSVIITEM_HPP

#include <xercesc/util/PlatformUtils.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/**
 * Represent the PSVI contributions for one element or one attribute information item.
 * This is *always* owned by the validator /parser object from which
 * it is obtained.  It is designed to be subclassed; subclasses will
 * specify under what conditions it may be relied upon to have meaningful contents.
 */

// forward declarations
class XSTypeDefinition; 
class XSSimpleTypeDefinition;
class XSValue;

class XMLPARSER_EXPORT PSVIItem : public XMemory
{
public:

    enum VALIDITY_STATE {
	    /** Validity value indicating that validation has either not 
	    been performed or that a strict assessment of validity could 
	    not be performed  
	    */
	    VALIDITY_NOTKNOWN               = 0,
	
	    /** Validity value indicating that validation has been strictly
	     assessed and the element in question is invalid according to the 
	     rules of schema validation.
	    */
	    VALIDITY_INVALID               = 1,
	
	    /** Validity value indicating that validation has been strictly 
	     assessed and the element in question is valid according to the rules 
	     of schema validation.
	     */
	    VALIDITY_VALID                 = 2
    };

    enum ASSESSMENT_TYPE {
	    /** Validation status indicating that schema validation has been 
	     performed and the element in question has specifically been skipped.   
	     */
	    VALIDATION_NONE                = 0,
	
	    /** Validation status indicating that schema validation has been 
	    performed on the element in question under the rules of lax validation.
	    */
	    VALIDATION_PARTIAL             = 1,
	
	    /**  Validation status indicating that full schema validation has been 
	    performed on the element.  */
	    VALIDATION_FULL                = 2
    };

    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    /** @name Constructors */
    //@{

    /**
      * The default constructor 
      *
      * @param  manager     The configurable memory manager
      */
    PSVIItem(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    //@};

    /** @name Destructor */
    //@{
    virtual ~PSVIItem();
    //@}

    //---------------------
    /** @name PSVIItem methods */

    //@{

    /**
     * [validation context]
     * 
     * @return A string identifying the nearest ancestor element 
     *          information item with a [schema information] property
     *         (or this element item itself if it has such a property)
     *          (form to be determined)
     * @see <a href="http://www.w3.org/TR/xmlschema-1/#e-validation_context">XML Schema Part 1: Structures [validation context]</a>
     */
    const XMLCh *getValidationContext();

    /**
     * Determine the validity of the node with respect 
     * to the validation being attempted
     * 
     * @return return the [validity] property. Possible values are: 
     *         VALIDITY_UNKNOWN, VALIDITY_INVALID, VALIDITY_VALID
     */
    VALIDITY_STATE getValidity() const;

    /**
     * Determines the extent to which the item has been validated
     * 
     * @return return the [validation attempted] property. The possible values are 
     *         VALIDATION_NONE, VALIDATION_ORDERED_PARTIAL and VALIDATION_FULL
     */
    ASSESSMENT_TYPE getValidationAttempted() const;

    /**
     * A list of error codes generated from validation attempts. 
     * Need to find all the possible sub-clause reports that need reporting
     * 
     * @return list of error codes
     */
    /***
    const XMLCh ** getErrorCodes();
    ****/
    
    /**
     * [schema normalized value] 
     * 
     * @see <a href="http://www.w3.org/TR/xmlschema-1/#e-schema_normalized_value">XML Schema Part 1: Structures [schema normalized value]</a>
     * @return the normalized value of this item after validation
     */
    const XMLCh *getSchemaNormalizedValue();

    /**
     * An item isomorphic to the type definition used to validate this element.
     * 
     * @return  a type declaration
     */
    virtual XSTypeDefinition *getTypeDefinition() = 0;
    
    /**
     * If and only if that type definition is a simple type definition
     * with {variety} union, or a complex type definition whose {content type}
     * is a simple thype definition with {variety} union, then an item isomorphic
     * to that member of the union's {member type definitions} which actually
     * validated the element item's normalized value.
     * 
     * @return  a simple type declaration
     */
    virtual XSSimpleTypeDefinition *getMemberTypeDefinition() = 0;
    
    /**
     * [schema default]
     * 
     * @return The canonical lexical representation of the declaration's {value constraint} value.
     * @see <a href="http://www.w3.org/TR/xmlschema-1/#e-schema_default">XML Schema Part 1: Structures [schema default]</a>
     */
    const XMLCh *getSchemaDefault();

    /**
     * [schema specified] 
     * @see <a href="http://www.w3.org/TR/xmlschema-1/#e-schema_specified">XML Schema Part 1: Structures [schema specified]</a>
     * @return true - value was specified in schema, false - value comes from the infoset
     */
    bool getIsSchemaSpecified() const;

    /**
     * Return the canonical representation of this value.
     * Note that, formally, this is not a PSVI property.
     * @return string representing the canonical representation, if this item
     * was validated by a simple type definition for which canonical
     * representations of values are defined.
     */
    const XMLCh *getCanonicalRepresentation() const;

    //@}

    /**
     * 
     * Get actual value in the form of XSValue,
     * caller needs to delete the object returned.
     *
     * @return  an XSValue 
     */
    virtual XSValue *getActualValue() const;

    //----------------------------------
    /** methods needed by implementation */

    //@{

    void setValidationAttempted(PSVIItem::ASSESSMENT_TYPE attemptType);
    void setValidity(PSVIItem::VALIDITY_STATE validity);

    /** reset the object
     * @param validationContext  corresponds to schema validation context property
     * @param normalizedValue  corresponds to schema normalized value property
     * @param validityState  state of item's validity
     * @param assessmentType  type of assessment carried out on item
     */
    void reset(
            const XMLCh* const validationContext
            , const XMLCh* const normalizedValue
            , const VALIDITY_STATE validityState
            , const ASSESSMENT_TYPE assessmentType
        );
    //@}
private:

    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    PSVIItem(const PSVIItem&);
    PSVIItem & operator=(const PSVIItem &);


protected:
    // -----------------------------------------------------------------------
    //  data members
    // -----------------------------------------------------------------------
    // fMemoryManager:
    //  used for any memory allocations
    // fValidationContext
    //  corresponds to the schema [validation context] property
    // fNormalizedValue
    //  The schema normalized value (when present)
    // fDefaultValue
    //  default value specified in the schema, if any
    // fCanonicalValue
    //  canonicalized version of normalizedValue
    // fValidityState
    //  Whether this item is valid or not
    // fAssessmentType
    //  The kind of assessment that produced the given validity outcome
    // fIsSpecified
    //  Whether this item exists because a default was specified in the schema
    // fType
    //  type responsible for validating this item
    // fMemberType
    //  If fType is a union type, the member type that validated this item
    MemoryManager* const        fMemoryManager;
    const XMLCh*                fValidationContext;
    const XMLCh*                fNormalizedValue;
    const XMLCh*                fDefaultValue;
    XMLCh*                      fCanonicalValue;
    VALIDITY_STATE              fValidityState;
    ASSESSMENT_TYPE             fAssessmentType;
    bool                        fIsSpecified;
    XSTypeDefinition *          fType;
    XSSimpleTypeDefinition*     fMemberType;
};

inline PSVIItem::~PSVIItem() {}

inline const XMLCh *PSVIItem::getValidationContext() 
{
    return fValidationContext;
}

inline const XMLCh* PSVIItem::getSchemaNormalizedValue() 
{
    return fNormalizedValue;
}

inline const XMLCh* PSVIItem::getSchemaDefault() 
{
    return fDefaultValue;
}

inline const XMLCh* PSVIItem::getCanonicalRepresentation() const
{
    return fCanonicalValue;
}

inline PSVIItem::VALIDITY_STATE PSVIItem::getValidity() const
{
    return fValidityState;
}

inline bool PSVIItem::getIsSchemaSpecified() const
{
    return fIsSpecified;
}

inline PSVIItem::ASSESSMENT_TYPE PSVIItem::getValidationAttempted() const
{
    return fAssessmentType;
}

XERCES_CPP_NAMESPACE_END

#endif
