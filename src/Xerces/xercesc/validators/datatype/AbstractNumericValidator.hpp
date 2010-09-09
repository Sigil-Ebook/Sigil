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
 * $Id: AbstractNumericValidator.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_ABSTRACT_NUMERIC_VALIDATOR_HPP)
#define XERCESC_INCLUDE_GUARD_ABSTRACT_NUMERIC_VALIDATOR_HPP

#include <xercesc/validators/datatype/AbstractNumericFacetValidator.hpp>
#include <xercesc/util/RefVectorOf.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class VALIDATORS_EXPORT AbstractNumericValidator : public AbstractNumericFacetValidator
{
public:

    // -----------------------------------------------------------------------
    //  Public ctor/dtor
    // -----------------------------------------------------------------------
	/** @name Constructor. */
    //@{

    virtual ~AbstractNumericValidator();

	//@}

	virtual void validate
                 (
                  const XMLCh*             const content
                ,       ValidationContext* const context = 0
                ,       MemoryManager*     const manager = XMLPlatformUtils::fgMemoryManager
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
    DECL_XSERIALIZABLE(AbstractNumericValidator)

protected:

    AbstractNumericValidator
    (
        DatatypeValidator* const baseValidator
        , RefHashTableOf<KVStringPair>* const facets
        , const int finalSet
        , const ValidatorType type
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    inline void init(RefArrayVectorOf<XMLCh>*  const enums
        , MemoryManager* const manager);

    //
    // Abstract interface
    //
    virtual void checkContent(const XMLCh*             const content
                            ,       ValidationContext* const context
                            , bool                           asBase
                            , MemoryManager* const manager) = 0;

    void boundsCheck(const XMLNumber* const
                    , MemoryManager* const manager);

private:

    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    AbstractNumericValidator(const AbstractNumericValidator&);
    AbstractNumericValidator& operator=(const AbstractNumericValidator&);
};

inline void AbstractNumericValidator::init(RefArrayVectorOf<XMLCh>*  const enums
                                           , MemoryManager* const manager)
{
    AbstractNumericFacetValidator::init(enums, manager);
}

XERCES_CPP_NAMESPACE_END

#endif

/**
  * End of file AbstractNumericValidator.hpp
  */
