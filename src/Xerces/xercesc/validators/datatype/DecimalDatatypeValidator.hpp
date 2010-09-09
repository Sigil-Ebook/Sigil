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
 * $Id: DecimalDatatypeValidator.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DECIMAL_DATATYPEVALIDATOR_HPP)
#define XERCESC_INCLUDE_GUARD_DECIMAL_DATATYPEVALIDATOR_HPP

#include <xercesc/validators/datatype/AbstractNumericValidator.hpp>
#include <xercesc/util/RefVectorOf.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XMLBigDecimal;

class VALIDATORS_EXPORT DecimalDatatypeValidator : public AbstractNumericValidator
{
public:

    // -----------------------------------------------------------------------
    //  Public ctor/dtor
    // -----------------------------------------------------------------------
	/** @name Constructors and Destructor */
    //@{

    DecimalDatatypeValidator
    (
        MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );
    DecimalDatatypeValidator
    (
        DatatypeValidator* const baseValidator
        , RefHashTableOf<KVStringPair>* const facets
        , RefArrayVectorOf<XMLCh>* const enums
        , const int finalSet
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );
    virtual ~DecimalDatatypeValidator();

	//@}

    // -----------------------------------------------------------------------
    // Compare methods
    // -----------------------------------------------------------------------
    /** @name Compare Function */
    //@{

    /**
     * Compare two boolean data types
     *
     * @param content1
     * @param content2
     * @return
     */
    virtual int compare(const XMLCh* const, const XMLCh* const
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
        );

    //@}

    /**
      * Returns an instance of the base datatype validator class
	  * Used by the DatatypeValidatorFactory.
      */
    virtual DatatypeValidator* newInstance
    (
        RefHashTableOf<KVStringPair>* const facets
        , RefArrayVectorOf<XMLCh>* const enums
        , const int finalSet
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    virtual const XMLCh* getCanonicalRepresentation
                        (
                          const XMLCh*         const rawData
                        ,       MemoryManager* const memMgr = 0
                        ,       bool                 toValidate = false
                        ) const;

    /***
     * Support for Serialization/De-serialization
     ***/
    DECL_XSERIALIZABLE(DecimalDatatypeValidator)

protected:

// -----------------------------------------------------------------------
// ctor provided to be used by derived classes
// -----------------------------------------------------------------------
    DecimalDatatypeValidator
    (
        DatatypeValidator* const baseValidator
        , RefHashTableOf<KVStringPair>* const facets
        , const int finalSet
        , const ValidatorType type
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

// -----------------------------------------------------------------------
// Abstract interface from AbstractNumericFacetValidator
// -----------------------------------------------------------------------

    virtual void assignAdditionalFacet(const XMLCh* const key
                                     , const XMLCh* const value
                                     , MemoryManager* const manager);

    virtual void inheritAdditionalFacet();

    virtual void checkAdditionalFacetConstraints(MemoryManager* const manager) const;

    virtual void checkAdditionalFacetConstraintsBase(MemoryManager* const manager) const;

    virtual int  compareValues(const XMLNumber* const lValue
                             , const XMLNumber* const rValue);

    virtual void  setMaxInclusive(const XMLCh* const);

    virtual void  setMaxExclusive(const XMLCh* const);

    virtual void  setMinInclusive(const XMLCh* const);

    virtual void  setMinExclusive(const XMLCh* const);

    virtual void  setEnumeration(MemoryManager* const manager);

// -----------------------------------------------------------------------
// Abstract interface from AbstractNumericValidator
// -----------------------------------------------------------------------

    virtual void checkContent(const XMLCh*             const content
                            ,       ValidationContext* const context
                            , bool                           asBase
                            ,       MemoryManager*     const manager);
public:

// -----------------------------------------------------------------------
// Getter methods
// -----------------------------------------------------------------------

    inline unsigned int                    getTotalDigits() const;

    inline unsigned int                    getFractionDigits() const;

private:
// -----------------------------------------------------------------------
// Setter methods
// -----------------------------------------------------------------------

    inline void  setTotalDigits(unsigned int);

    inline void  setFractionDigits(unsigned int);

    // -----------------------------------------------------------------------
    //  Private data members
    //
    // -----------------------------------------------------------------------
    unsigned int         fTotalDigits;
    unsigned int         fFractionDigits;


    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    DecimalDatatypeValidator(const DecimalDatatypeValidator&);
    DecimalDatatypeValidator& operator=(const DecimalDatatypeValidator&);
};

// -----------------------------------------------------------------------
// Getter methods
// -----------------------------------------------------------------------

inline unsigned int DecimalDatatypeValidator::getTotalDigits() const
{
    return fTotalDigits;
}

inline unsigned int DecimalDatatypeValidator::getFractionDigits() const
{
    return fFractionDigits;
}

// -----------------------------------------------------------------------
// Setter methods
// -----------------------------------------------------------------------

inline void DecimalDatatypeValidator::setTotalDigits(unsigned int newTotalDigits)
{
    fTotalDigits = newTotalDigits;
}

inline void DecimalDatatypeValidator::setFractionDigits(unsigned int newFractionDigits)
{
    fFractionDigits = newFractionDigits;
}

XERCES_CPP_NAMESPACE_END

#endif

/**
  * End of file DecimalDatatypeValidator.hpp
  */
