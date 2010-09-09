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
 * $Id: DatatypeValidatorFactory.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DATATYPEVALIDATORFACTORY_HPP)
#define XERCESC_INCLUDE_GUARD_DATATYPEVALIDATORFACTORY_HPP

/**
 * This class implements a factory of Datatype Validators. Internally the
 * DatatypeValidators are kept in a registry.
 * There is one instance of DatatypeValidatorFactory per Parser.
 * There is one datatype Registry per instance of DatatypeValidatorFactory,
 * such registry is first allocated with the number DatatypeValidators needed.
 * e.g.
 * If Parser finds an XML document with a DTD, a registry of DTD validators (only
 * 9 validators) get initialized in the registry.
 * The initialization process consist of instantiating the Datatype and
 * facets and registering the Datatype into registry table.
 * This implementation uses a Hashtable as a registry. The datatype validators created
 * by the factory will be deleted by the registry.
 *
 * As the Parser parses an instance document it knows if validation needs
 * to be checked. If no validation is necessary we should not instantiate a
 * DatatypeValidatorFactory.
 * If validation is needed, we need to instantiate a DatatypeValidatorFactory.
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/datatype/DatatypeValidator.hpp>
#include <xercesc/validators/datatype/XMLCanRepGroup.hpp>
#include <xercesc/util/RefVectorOf.hpp>

#include <xercesc/internal/XSerializable.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  DatatypeValidatorFactory: Local declaration
// ---------------------------------------------------------------------------
typedef RefHashTableOf<KVStringPair> KVStringPairHashTable;
typedef RefHashTableOf<DatatypeValidator> DVHashTable;
typedef RefArrayVectorOf<XMLCh> XMLChRefVector;


class VALIDATORS_EXPORT DatatypeValidatorFactory : public XSerializable, public XMemory
{
public:

    // -----------------------------------------------------------------------
    //  Public Constructors and Destructor
    // -----------------------------------------------------------------------
    /** @name Constructors */
    //@{

    DatatypeValidatorFactory
    (
        MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    //@}

    /** @name Destructor. */
    //@{

    ~DatatypeValidatorFactory();

    //@}

    // -----------------------------------------------------------------------
    // Getter methods
    // -----------------------------------------------------------------------
    /** @name Getter Functions */
    //@{

    /**
     * Returns the datatype validator
     *
     * @param  dvType   Datatype validator name/type
     */
    DatatypeValidator* getDatatypeValidator(const XMLCh* const dvType) const;

    /**
     * Returns the user defined registry of types
     **/
    DVHashTable* getUserDefinedRegistry() const;


    /**
     * Returns the built in  registry of types
     **/
    static DVHashTable* getBuiltInRegistry();

    //@}

  private:
    /**
     * Initializes registry with primitive and derived Simple types.
     *
     * This method does not clear the registry to clear the registry you
     * have to call resetRegistry.
     *
     * The net effect of this method is to start with the smallest set of
     * datatypes needed by the validator.
     *
     * If we start with Schema's then we initialize to full set of
     * validators.
     */
    void expandRegistryToFullSchemaSet();

  public:
    // -----------------------------------------------------------------------
    // Canonical Representation Group
    // -----------------------------------------------------------------------
           void                        initCanRepRegistory();

    static XMLCanRepGroup::CanRepGroup getCanRepGroup(const DatatypeValidator* const);

    static DatatypeValidator* getBuiltInBaseValidator(const DatatypeValidator* const);

    // -----------------------------------------------------------------------
    // Validator Factory methods
    // -----------------------------------------------------------------------
    /** @name Validator Factory Functions */
    //@{

    /**
     * Creates a new datatype validator of type baseValidator's class and
     * adds it to the registry
     *
     * @param  typeName        Datatype validator name
     *
     * @param  baseValidator   Base datatype validator
     *
     * @param  facets          datatype facets if any
     *
     * @param  enums           vector of values for enum facet
     *
     * @param  isDerivedByList Indicates whether the datatype is derived by
     *                         list or not
     *
     * @param  finalSet       'final' values of the simpleType
     *
     * @param  isUserDefined  Indicates whether the datatype is built-in or
     *                        user defined
     */
    DatatypeValidator* createDatatypeValidator
    (
        const XMLCh* const                    typeName
        , DatatypeValidator* const            baseValidator
        , RefHashTableOf<KVStringPair>* const facets
        , RefArrayVectorOf<XMLCh>* const      enums
        , const bool                          isDerivedByList
        , const int                           finalSet = 0
        , const bool                          isUserDefined = true
        , MemoryManager* const                manager = XMLPlatformUtils::fgMemoryManager
    );

    /**
     * Creates a new datatype validator of type UnionDatatypeValidator and
     * adds it to the registry
     *
     * @param  typeName       Datatype validator name
     *
     * @param  validators     Vector of datatype validators
     *
     * @param  finalSet       'final' values of the simpleType
     *
     * @param  isUserDefined  Indicates whether the datatype is built-in or
     *                        user defined
     */
    DatatypeValidator* createDatatypeValidator
    (
          const XMLCh* const                    typeName
        , RefVectorOf<DatatypeValidator>* const validators
        , const int                             finalSet
        , const bool                            isUserDefined = true
        , MemoryManager* const                  manager = XMLPlatformUtils::fgMemoryManager
    );

    //@}

    /**
      * Reset datatype validator registry
      */
    void resetRegistry();

    /***
     * Support for Serialization/De-serialization
     ***/
    DECL_XSERIALIZABLE(DatatypeValidatorFactory)

private:

    // -----------------------------------------------------------------------
    //  CleanUp methods
    // -----------------------------------------------------------------------
    void cleanUp();

    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    DatatypeValidatorFactory(const DatatypeValidatorFactory&);
    DatatypeValidatorFactory& operator=(const DatatypeValidatorFactory&);

    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fUserDefinedRegistry
    //      This is a hashtable of user defined dataype validators.
    //
    //  fBuiltInRegistry
    //      This is a hashtable of built-in primitive datatype validators.
    // -----------------------------------------------------------------------
    XERCES_CPP_NAMESPACE_QUALIFIER RefHashTableOf<XERCES_CPP_NAMESPACE_QUALIFIER DatatypeValidator>*        fUserDefinedRegistry;
    static XERCES_CPP_NAMESPACE_QUALIFIER RefHashTableOf<DatatypeValidator>* fBuiltInRegistry;
    static XERCES_CPP_NAMESPACE_QUALIFIER RefHashTableOf<XMLCanRepGroup, PtrHasher>*    fCanRepRegistry;
    XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* const fMemoryManager;

    friend class XPath2ContextImpl;
    friend class XMLInitializer;
};

inline DatatypeValidator*
DatatypeValidatorFactory::getDatatypeValidator(const XMLCh* const dvType) const
{
	if (dvType) {
        if (fBuiltInRegistry && fBuiltInRegistry->containsKey(dvType)) {
		    return fBuiltInRegistry->get(dvType);
        }

        if (fUserDefinedRegistry && fUserDefinedRegistry->containsKey(dvType)) {
		    return fUserDefinedRegistry->get(dvType);

        }
    }
	return 0;
}

inline DVHashTable*
DatatypeValidatorFactory::getUserDefinedRegistry() const {
    return fUserDefinedRegistry;
}

inline DVHashTable*
DatatypeValidatorFactory::getBuiltInRegistry() {
    return fBuiltInRegistry;
}
// ---------------------------------------------------------------------------
//  DatatypeValidator: CleanUp methods
// ---------------------------------------------------------------------------
inline void DatatypeValidatorFactory::cleanUp() {

    if (fUserDefinedRegistry)
    {
	    delete fUserDefinedRegistry;
	    fUserDefinedRegistry = 0;
    }
}

XERCES_CPP_NAMESPACE_END

#endif

/**
  * End of file DatatypeValidatorFactory.hpp
  */
