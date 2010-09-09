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
 * $Id: StringDatatypeValidator.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_STRING_DATATYPEVALIDATOR_HPP)
#define XERCESC_INCLUDE_GUARD_STRING_DATATYPEVALIDATOR_HPP

#include <xercesc/validators/datatype/AbstractStringValidator.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class VALIDATORS_EXPORT StringDatatypeValidator : public AbstractStringValidator
{
public:

    // -----------------------------------------------------------------------
    //  Public ctor/dtor
    // -----------------------------------------------------------------------
	/** @name Constructors and Destructor */
    //@{

    StringDatatypeValidator
    (
        MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );
    StringDatatypeValidator
    (
        DatatypeValidator* const baseValidator
        , RefHashTableOf<KVStringPair>* const facets
        , RefArrayVectorOf<XMLCh>* const enums
        , const int finalSet
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );
    virtual ~StringDatatypeValidator();

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

    /***
     * Support for Serialization/De-serialization
     ***/
    DECL_XSERIALIZABLE(StringDatatypeValidator)

protected:

    //
    // ctor provided to be used by derived classes
    //
    StringDatatypeValidator
    (
        DatatypeValidator* const baseValidator
        , RefHashTableOf<KVStringPair>* const facets
        , const int finalSet
        , const ValidatorType type
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    virtual void assignAdditionalFacet(const XMLCh* const key
                                     , const XMLCh* const value
                                     , MemoryManager* const manager);

    virtual void inheritAdditionalFacet();

    virtual void checkAdditionalFacetConstraints(MemoryManager* const manager) const;

    virtual void checkAdditionalFacet(const XMLCh* const content
                        , MemoryManager* const manager) const;

    virtual void checkValueSpace(const XMLCh* const content
                        , MemoryManager* const manager);

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    StringDatatypeValidator(const StringDatatypeValidator&);
    StringDatatypeValidator& operator=(const StringDatatypeValidator&);
};

XERCES_CPP_NAMESPACE_END

#endif

/**
  * End of file StringDatatypeValidator.hpp
  */
