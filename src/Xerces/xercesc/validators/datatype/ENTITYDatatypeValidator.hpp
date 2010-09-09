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
 * $Id: ENTITYDatatypeValidator.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_ENTITY_DATATYPEVALIDATOR_HPP)
#define XERCESC_INCLUDE_GUARD_ENTITY_DATATYPEVALIDATOR_HPP

#include <xercesc/validators/datatype/StringDatatypeValidator.hpp>
#include <xercesc/util/NameIdPool.hpp>
#include <xercesc/validators/DTD/DTDEntityDecl.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class VALIDATORS_EXPORT ENTITYDatatypeValidator : public StringDatatypeValidator
{
public:

    // -----------------------------------------------------------------------
    //  Public ctor/dtor
    // -----------------------------------------------------------------------
	/** @name Constructors and Destructor. */
    //@{

    ENTITYDatatypeValidator
    (
        MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );
    ENTITYDatatypeValidator
    (
        DatatypeValidator* const baseValidator
        , RefHashTableOf<KVStringPair>* const facets
        , RefArrayVectorOf<XMLCh>* const enums
        , const int finalSet
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    virtual ~ENTITYDatatypeValidator();

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
    virtual int compare(const XMLCh* const, const XMLCh* const
        ,       MemoryManager*     const manager = XMLPlatformUtils::fgMemoryManager);

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
    DECL_XSERIALIZABLE(ENTITYDatatypeValidator)

protected:

    //
    // ctor provided to be used by derived classes
    //
    ENTITYDatatypeValidator(DatatypeValidator*            const baseValidator
                          , RefHashTableOf<KVStringPair>* const facets
                          , const int                           finalSet
                          , const ValidatorType                 type);

    virtual void checkValueSpace(const XMLCh* const content
                                , MemoryManager* const manager);

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    ENTITYDatatypeValidator(const ENTITYDatatypeValidator&);
    ENTITYDatatypeValidator& operator = (const  ENTITYDatatypeValidator&);

};

XERCES_CPP_NAMESPACE_END

#endif

/**
  * End of file ENTITYDatatypeValidator.hpp
  */
