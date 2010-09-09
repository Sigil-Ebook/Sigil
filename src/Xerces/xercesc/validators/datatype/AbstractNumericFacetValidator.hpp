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
 * $Id: AbstractNumericFacetValidator.hpp 673155 2008-07-01 17:55:39Z dbertoni $
 */

#if !defined(XERCESC_INCLUDE_GUARD_ABSTRACT_NUMERIC_FACET_VALIDATOR_HPP)
#define XERCESC_INCLUDE_GUARD_ABSTRACT_NUMERIC_FACET_VALIDATOR_HPP

#include <xercesc/validators/datatype/DatatypeValidator.hpp>
#include <xercesc/util/RefArrayVectorOf.hpp>
#include <xercesc/util/XMLNumber.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class VALIDATORS_EXPORT AbstractNumericFacetValidator : public DatatypeValidator
{
public:

    // -----------------------------------------------------------------------
    //  Public ctor/dtor
    // -----------------------------------------------------------------------
	/** @name Constructor. */
    //@{

    virtual ~AbstractNumericFacetValidator();

	//@}

	virtual const RefArrayVectorOf<XMLCh>* getEnumString() const;

    /***
     * Support for Serialization/De-serialization
     ***/
    DECL_XSERIALIZABLE(AbstractNumericFacetValidator)

protected:

    AbstractNumericFacetValidator
    (
        DatatypeValidator* const baseValidator
        , RefHashTableOf<KVStringPair>* const facets
        , const int finalSet
        , const ValidatorType type
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    void init(RefArrayVectorOf<XMLCh>*  const enums
        , MemoryManager* const manager);

    //
    // Abstract interface
    //
    virtual void assignAdditionalFacet(const XMLCh* const key
                                     , const XMLCh* const value
                                     , MemoryManager* const manager);

    virtual void inheritAdditionalFacet();

    virtual void checkAdditionalFacetConstraints(MemoryManager* const manager) const;

    virtual void checkAdditionalFacetConstraintsBase(MemoryManager* const manager) const;

    virtual int  compareValues(const XMLNumber* const lValue
                             , const XMLNumber* const rValue) = 0;

    virtual void checkContent(const XMLCh*             const content
                            ,       ValidationContext* const context
                            , bool                           asBase
                            ,       MemoryManager*     const manager) = 0;

// -----------------------------------------------------------------------
// Setter methods
// -----------------------------------------------------------------------

    virtual void  setMaxInclusive(const XMLCh* const) = 0;

    virtual void  setMaxExclusive(const XMLCh* const) = 0;

    virtual void  setMinInclusive(const XMLCh* const) = 0;

    virtual void  setMinExclusive(const XMLCh* const) = 0;

    virtual void  setEnumeration(MemoryManager* const manager) = 0;

    static const int INDETERMINATE;

public:
// -----------------------------------------------------------------------
// Getter methods
// -----------------------------------------------------------------------

    inline XMLNumber*                  getMaxInclusive() const;

    inline XMLNumber*                  getMaxExclusive() const;

    inline XMLNumber*                  getMinInclusive() const;

    inline XMLNumber*                  getMinExclusive() const;

    inline RefVectorOf<XMLNumber>*     getEnumeration() const;

protected:
    // -----------------------------------------------------------------------
    //  Protected data members
    //
    //      Allow access to derived class
    //
    // -----------------------------------------------------------------------
    bool                     fMaxInclusiveInherited;
    bool                     fMaxExclusiveInherited;
    bool                     fMinInclusiveInherited;
    bool                     fMinExclusiveInherited;
    bool                     fEnumerationInherited;

    XMLNumber*               fMaxInclusive;
    XMLNumber*               fMaxExclusive;
    XMLNumber*               fMinInclusive;
    XMLNumber*               fMinExclusive;

    RefVectorOf<XMLNumber>*  fEnumeration;    // save the actual value
    RefArrayVectorOf<XMLCh>*      fStrEnumeration;

private:

    void assignFacet(MemoryManager* const manager);

    void inspectFacet(MemoryManager* const manager);

    void inspectFacetBase(MemoryManager* const manager);

    void inheritFacet();

    void storeClusive(XSerializeEngine&
                    , bool
                    , XMLNumber*);

    void loadClusive(XSerializeEngine&
                   , bool&
                   , XMLNumber*&
                   , XMLNumber::NumberType
                   , int );

    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    AbstractNumericFacetValidator(const AbstractNumericFacetValidator&);
    AbstractNumericFacetValidator& operator=(const AbstractNumericFacetValidator&);
};

// -----------------------------------------------------------------------
// Getter methods
// -----------------------------------------------------------------------

inline XMLNumber* AbstractNumericFacetValidator::getMaxInclusive() const
{
    return fMaxInclusive;
}

inline XMLNumber* AbstractNumericFacetValidator::getMaxExclusive() const
{
    return fMaxExclusive;
}

inline XMLNumber* AbstractNumericFacetValidator::getMinInclusive() const
{
    return fMinInclusive;
}

inline XMLNumber* AbstractNumericFacetValidator::getMinExclusive() const
{
    return fMinExclusive;
}

inline RefVectorOf<XMLNumber>* AbstractNumericFacetValidator::getEnumeration() const
{
    return fEnumeration;
}

XERCES_CPP_NAMESPACE_END

#endif

/**
  * End of file AbstractNumericFacetValidator.hpp
  */
