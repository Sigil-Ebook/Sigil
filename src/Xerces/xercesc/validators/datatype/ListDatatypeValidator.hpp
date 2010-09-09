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
 * $Id: ListDatatypeValidator.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_LIST_DATATYPEVALIDATOR_HPP)
#define XERCESC_INCLUDE_GUARD_LIST_DATATYPEVALIDATOR_HPP

#include <xercesc/validators/datatype/AbstractStringValidator.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class VALIDATORS_EXPORT ListDatatypeValidator : public AbstractStringValidator
{
public:

    // -----------------------------------------------------------------------
    //  Public ctor/dtor
    // -----------------------------------------------------------------------
	/** @name Constructors and Destructor */
    //@{

    ListDatatypeValidator
    (
        MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );
    ListDatatypeValidator
    (
        DatatypeValidator* const baseValidator
        , RefHashTableOf<KVStringPair>* const facets
        , RefArrayVectorOf<XMLCh>* const enums
        , const int finalSet
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );
    virtual ~ListDatatypeValidator();

	//@}

    /** @name Getter Functions */
    //@{
    /**
      * Returns whether the type is atomic or not
      */
    virtual bool isAtomic() const;

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
    virtual DatatypeValidator* newInstance
    (
        RefHashTableOf<KVStringPair>* const facets
        , RefArrayVectorOf<XMLCh>* const enums
        , const int finalSet
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    DatatypeValidator* getItemTypeDTV() const;

    /***
     * Support for Serialization/De-serialization
     ***/
    DECL_XSERIALIZABLE(ListDatatypeValidator)

protected:

    //
    // ctor provided to be used by derived classes: No
    //

    virtual void checkValueSpace(const XMLCh* const content
                                , MemoryManager* const manager);

    virtual XMLSize_t getLength(const XMLCh* const content
            , MemoryManager* const manager) const;

    //
    // Overwrite AbstractStringValidator's
    //
    virtual void inspectFacetBase(MemoryManager* const manager);

    virtual void inheritFacet();

    virtual void checkContent(const XMLCh*             const content
                            ,       ValidationContext* const context
                            , bool                           asBase
                            ,       MemoryManager*     const manager);

private:

    void checkContent(      BaseRefVectorOf<XMLCh>*        tokenVector
                    , const XMLCh*                  const  content
                    ,       ValidationContext*      const  context
                    ,       bool                           asBase
                    ,       MemoryManager*          const  manager
                    );

    bool valueSpaceCheck(BaseRefVectorOf<XMLCh>* tokenVector
                       , const XMLCh*   const enumStr
                       , MemoryManager* const manager) const;

// -----------------------------------------------------------------------
// Getter methods
// -----------------------------------------------------------------------

    inline const XMLCh*         getContent() const;

// -----------------------------------------------------------------------
// Setter methods
// -----------------------------------------------------------------------

    inline void                 setContent(const XMLCh* const content);

    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    ListDatatypeValidator(const ListDatatypeValidator&);
    ListDatatypeValidator& operator=(const ListDatatypeValidator&);

    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fContent
    //      temporary var referencing the content to be validated,
    //      for error reporting purpose.
    //
    // -----------------------------------------------------------------------
     const XMLCh*         fContent;
};

// -----------------------------------------------------------------------
// Getter methods
// -----------------------------------------------------------------------
inline const XMLCh* ListDatatypeValidator::getContent() const
{
    return fContent;
}

inline bool ListDatatypeValidator::isAtomic() const
{
    return false;
}

// -----------------------------------------------------------------------
// Setter methods
// -----------------------------------------------------------------------
inline void ListDatatypeValidator::setContent(const XMLCh* const content)
{
    fContent = content;
}

XERCES_CPP_NAMESPACE_END

#endif

/**
  * End of file ListDatatypeValidator.hpp
  */
