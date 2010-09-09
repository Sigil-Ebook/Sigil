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
 * $Id: BooleanDatatypeValidator.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_BOOLEAN_DATATYPEVALIDATOR_HPP)
#define XERCESC_INCLUDE_GUARD_BOOLEAN_DATATYPEVALIDATOR_HPP

#include <xercesc/validators/datatype/DatatypeValidator.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class VALIDATORS_EXPORT BooleanDatatypeValidator : public DatatypeValidator
{
public:

    // -----------------------------------------------------------------------
    //  Public ctor/dtor
    // -----------------------------------------------------------------------
	/** @name Constructor. */
    //@{

    BooleanDatatypeValidator
    (
        MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );
    BooleanDatatypeValidator
    (
        DatatypeValidator* const baseValidator
        , RefHashTableOf<KVStringPair>* const facets
        , RefArrayVectorOf<XMLCh>* const enums
        , const int finalSet
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    virtual ~BooleanDatatypeValidator();

	//@}

	virtual const RefArrayVectorOf<XMLCh>* getEnumString() const;

    // -----------------------------------------------------------------------
    // Getter methods
    // -----------------------------------------------------------------------
    /** @name Getter Functions */
    //@{

    virtual const XMLCh* getCanonicalRepresentation
                        (
                          const XMLCh*         const rawData
                        ,       MemoryManager* const memMgr = 0
                        ,       bool                 toValidate = false
                        ) const;
    //@}

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

    /**
     * Compare two boolean data types
     *
     * @param content1
     * @param content2
     * @return
     */
    int compare(const XMLCh* const, const XMLCh* const
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
        );

    //@}

    /**
      * Returns an instance of the base datatype validator class
	  * Used by the DatatypeValidatorFactory.
      */
    DatatypeValidator* newInstance
    (
        RefHashTableOf<KVStringPair>* const facets
        , RefArrayVectorOf<XMLCh>* const enums
        , const int finalSet
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    /***
     * Support for Serialization/De-serialization
     ***/
    DECL_XSERIALIZABLE(BooleanDatatypeValidator)

private:

    virtual void checkContent(const XMLCh*             const content
                            ,       ValidationContext* const context
                            , bool                           asBase
                            , MemoryManager* const manager);

    // -----------------------------------------------------------------------
    //  Unimplemented methods.
    // -----------------------------------------------------------------------
    BooleanDatatypeValidator(const BooleanDatatypeValidator&);
    BooleanDatatypeValidator& operator=(const BooleanDatatypeValidator&);

};

// ---------------------------------------------------------------------------
//  Constructors and Destructor
// ---------------------------------------------------------------------------
inline BooleanDatatypeValidator::BooleanDatatypeValidator(MemoryManager* const manager)
:DatatypeValidator(0, 0, 0, DatatypeValidator::Boolean, manager)
{
    setFinite(true);
}

inline BooleanDatatypeValidator::~BooleanDatatypeValidator()
{
}

// -----------------------------------------------------------------------
// Getter methods
// -----------------------------------------------------------------------


// -----------------------------------------------------------------------
// Compare methods
// -----------------------------------------------------------------------

inline DatatypeValidator* BooleanDatatypeValidator::newInstance
(
      RefHashTableOf<KVStringPair>* const facets
    , RefArrayVectorOf<XMLCh>* const      enums
    , const int                           finalSet
    , MemoryManager* const                manager
)
{
    return (DatatypeValidator*) new (manager) BooleanDatatypeValidator(this, facets, enums, finalSet, manager);
}

inline void BooleanDatatypeValidator::validate( const XMLCh*             const content
                                              ,       ValidationContext* const context
                                              ,       MemoryManager*     const manager)
{
    checkContent(content, context, false, manager);
}

XERCES_CPP_NAMESPACE_END

#endif

/**
  * End of file BooleanDatatypeValidator.hpp
  */

