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
 * $Id: AbstractStringValidator.hpp 695949 2008-09-16 15:57:44Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_ABSTRACT_STRING_VALIDATOR_HPP)
#define XERCESC_INCLUDE_GUARD_ABSTRACT_STRING_VALIDATOR_HPP

#include <xercesc/validators/datatype/DatatypeValidator.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class VALIDATORS_EXPORT AbstractStringValidator : public DatatypeValidator
{
public:

    // -----------------------------------------------------------------------
    //  Public ctor/dtor
    // -----------------------------------------------------------------------
	/** @name Constructor. */
    //@{

    virtual ~AbstractStringValidator();

	//@}

	virtual const RefArrayVectorOf<XMLCh>* getEnumString() const;

    // -----------------------------------------------------------------------
    // Validation methods
    // -----------------------------------------------------------------------
    /** @name Validation Function */
    //@{

    /**
     * validate that a string matches the boolean datatype
     * @param content A string containing the content to be validated
     *
     * @exception throws InvalidDatatypeException if the content is
     * is not valid.
     */

	virtual void validate
                 (
                  const XMLCh*             const content
                ,       ValidationContext* const context = 0
                ,       MemoryManager*     const manager = XMLPlatformUtils::fgMemoryManager
                  );

    //@}

    // -----------------------------------------------------------------------
    // Compare methods
    // -----------------------------------------------------------------------
    /** @name Compare Function */
    //@{

    virtual int compare(const XMLCh* const, const XMLCh* const
        ,       MemoryManager*     const manager = XMLPlatformUtils::fgMemoryManager
        );

    //@}

    /***
     * Support for Serialization/De-serialization
     ***/
    DECL_XSERIALIZABLE(AbstractStringValidator)

protected:

    AbstractStringValidator
    (
        DatatypeValidator* const baseValidator
        , RefHashTableOf<KVStringPair>* const facets
        , const int finalSet
        , const ValidatorType type
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    void init(RefArrayVectorOf<XMLCh>*           const enums
        , MemoryManager* const manager);

    //
    // Abstract interface
    //
    virtual void assignAdditionalFacet(const XMLCh* const key
                                     , const XMLCh* const value
                                     , MemoryManager* const manager);

    virtual void inheritAdditionalFacet();

    virtual void checkAdditionalFacetConstraints(MemoryManager* const manager) const;

    virtual void checkAdditionalFacet(const XMLCh* const content
                                    , MemoryManager* const manager) const;

    virtual XMLSize_t getLength(const XMLCh* const content
        , MemoryManager* const manager) const;
    
    virtual void checkValueSpace(const XMLCh* const content
        , MemoryManager* const manager) = 0;

    //
    //   to Allow ListDTV to overwrite
    //
    virtual void inspectFacetBase(MemoryManager* const manager);

    virtual void inheritFacet();

    virtual void checkContent(const XMLCh*             const content
                            ,       ValidationContext* const context
                            , bool                           asBase
                            , MemoryManager* const manager);

    /*
     **  Base64BinaryDatatypeValidator to overwrite
     */
    virtual void normalizeEnumeration(MemoryManager* const manager);

    virtual void normalizeContent(XMLCh* const, MemoryManager* const manager) const;

public:
// -----------------------------------------------------------------------
// Getter methods
// -----------------------------------------------------------------------

    inline XMLSize_t            getLength() const;

    inline XMLSize_t            getMaxLength() const;

    inline XMLSize_t            getMinLength() const;

    inline RefArrayVectorOf<XMLCh>*  getEnumeration() const;

protected:
// -----------------------------------------------------------------------
// Setter methods
// -----------------------------------------------------------------------

    inline void                 setLength(XMLSize_t);

    inline void                 setMaxLength(XMLSize_t);

    inline void                 setMinLength(XMLSize_t);

    inline void                 setEnumeration(RefArrayVectorOf<XMLCh>*, bool);

private:

    void assignFacet(MemoryManager* const manager);

    void inspectFacet(MemoryManager* const manager);

    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    AbstractStringValidator(const AbstractStringValidator&);
    AbstractStringValidator& operator=(const AbstractStringValidator&);

    // -----------------------------------------------------------------------
    //  Private data members
    //
    // -----------------------------------------------------------------------
     XMLSize_t            fLength;
     XMLSize_t            fMaxLength;
     XMLSize_t            fMinLength;
     bool                 fEnumerationInherited;
     RefArrayVectorOf<XMLCh>*  fEnumeration;
};

// -----------------------------------------------------------------------
// Getter methods
// -----------------------------------------------------------------------

inline XMLSize_t AbstractStringValidator::getLength() const
{
    return fLength;
}

inline XMLSize_t AbstractStringValidator::getMaxLength() const
{
    return fMaxLength;
}

inline XMLSize_t AbstractStringValidator::getMinLength() const
{
    return fMinLength;
}

inline RefArrayVectorOf<XMLCh>* AbstractStringValidator:: getEnumeration() const
{
    return fEnumeration;
}

// -----------------------------------------------------------------------
// Setter methods
// -----------------------------------------------------------------------

inline void AbstractStringValidator::setLength(XMLSize_t newLength)
{
    fLength = newLength;
}

inline void AbstractStringValidator::setMaxLength(XMLSize_t newMaxLength)
{
    fMaxLength = newMaxLength;
}

inline void AbstractStringValidator::setMinLength(XMLSize_t newMinLength)
{
    fMinLength = newMinLength;
}

inline void AbstractStringValidator::setEnumeration(RefArrayVectorOf<XMLCh>* enums
                                           , bool                inherited)
{
    if (enums)
    {
        if ( !fEnumerationInherited && fEnumeration)
            delete fEnumeration;

        fEnumeration = enums;
        fEnumerationInherited = inherited;
        setFacetsDefined(DatatypeValidator::FACET_ENUMERATION);
    }
}

XERCES_CPP_NAMESPACE_END

#endif

/**
  * End of file AbstractStringValidator.hpp
  */
