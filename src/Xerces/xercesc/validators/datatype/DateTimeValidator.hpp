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
 * $Id: DateTimeValidator.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DATETIME_VALIDATOR_HPP)
#define XERCESC_INCLUDE_GUARD_DATETIME_VALIDATOR_HPP

#include <xercesc/validators/datatype/AbstractNumericFacetValidator.hpp>
#include <xercesc/util/XMLDateTime.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class VALIDATORS_EXPORT DateTimeValidator : public AbstractNumericFacetValidator
{
public:

    // -----------------------------------------------------------------------
    //  Public dtor
    // -----------------------------------------------------------------------
	/** @name Constructor. */
    //@{

    virtual ~DateTimeValidator();

	//@}

	virtual void validate
                 (
                  const XMLCh*             const content
                ,       ValidationContext* const context = 0
                ,       MemoryManager*     const manager = XMLPlatformUtils::fgMemoryManager
                  );

    virtual int  compare(const XMLCh* const value1
                       , const XMLCh* const value2
                       , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
                       );

    /***
     * Support for Serialization/De-serialization
     ***/
    DECL_XSERIALIZABLE(DateTimeValidator)

protected:

    // -----------------------------------------------------------------------
    //  ctor used by derived class
    // -----------------------------------------------------------------------
    DateTimeValidator
    (
        DatatypeValidator* const baseValidator
        , RefHashTableOf<KVStringPair>* const facets
        , const int finalSet
        , const ValidatorType type
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    //
    // Abstract interface
    //

    virtual int  compareValues(const XMLNumber* const lValue
                             , const XMLNumber* const rValue);

    virtual void checkContent(const XMLCh*             const content
                            ,       ValidationContext* const context
                            , bool                           asBase
                            ,       MemoryManager*     const manager);

    virtual void  setMaxInclusive(const XMLCh* const);

    virtual void  setMaxExclusive(const XMLCh* const);

    virtual void  setMinInclusive(const XMLCh* const);

    virtual void  setMinExclusive(const XMLCh* const);

    virtual void  setEnumeration(MemoryManager* const manager);

protected:

    // -----------------------------------------------------------------------
    //  helper interface: to be implemented/overwritten by derived class
    // -----------------------------------------------------------------------
    virtual XMLDateTime*   parse(const XMLCh* const, MemoryManager* const manager) = 0;
    virtual void parse(XMLDateTime* const) = 0;

    // to be overwritten by duration
    virtual int            compareDates(const XMLDateTime* const lValue
                                      , const XMLDateTime* const rValue
                                      , bool strict);

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    DateTimeValidator(const DateTimeValidator&);
    DateTimeValidator& operator=(const DateTimeValidator&);
};

XERCES_CPP_NAMESPACE_END

#endif

/**
  * End of file DateTimeValidator.hpp
  */


