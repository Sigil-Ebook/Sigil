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
 * $Id: GrammarResolver.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_GRAMMARRESOLVER_HPP)
#define XERCESC_INCLUDE_GUARD_GRAMMARRESOLVER_HPP

#include <xercesc/framework/XMLGrammarPool.hpp>
#include <xercesc/util/RefHashTableOf.hpp>
#include <xercesc/util/StringPool.hpp>
#include <xercesc/validators/common/Grammar.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class DatatypeValidator;
class DatatypeValidatorFactory;
class XMLGrammarDescription;

/**
 * This class embodies the representation of a Grammar pool Resolver.
 * This class is called from the validator.
 *
 */

class VALIDATORS_EXPORT GrammarResolver : public XMemory
{
public:

    /** @name Constructor and Destructor */
    //@{
    /**
     *
     * Default Constructor
     */
    GrammarResolver(
                    XMLGrammarPool* const gramPool
                  , MemoryManager*  const manager = XMLPlatformUtils::fgMemoryManager
                    );
    /**
      * Destructor
      */
    ~GrammarResolver();

    //@}

    /** @name Getter methods */
    //@{
    /**
     * Retrieve the DatatypeValidator
     *
     * @param uriStr the namespace URI
     * @param typeName the type name
     * @return the DatatypeValidator associated with namespace & type name
     */
    DatatypeValidator* getDatatypeValidator(const XMLCh* const uriStr,
                                            const XMLCh* const typeName);

    /**
     * Retrieve the DatatypeValidatorFactory used for built-in schema types
     *
     * @return the DatatypeValidator associated with namespace for XMLSchema
     */
    DatatypeValidatorFactory* getBuiltinDatatypeValidatorFactory();

    /**
     * Retrieve the grammar that is associated with the specified namespace key
     *
     * @param  gramDesc   grammar description for the grammar
     * @return Grammar abstraction associated with the grammar description
     */
    Grammar* getGrammar( XMLGrammarDescription* const gramDesc ) ;

    /**
     * Retrieve the grammar that is associated with the specified namespace key
     *
     * @param  namespaceKey   Namespace key into Grammar pool
     * @return Grammar abstraction associated with the NameSpace key.
     */
    Grammar* getGrammar( const XMLCh* const namespaceKey ) ;

    /**
     * Get an enumeration of Grammar in the Grammar pool
     *
     * @return enumeration of Grammar in Grammar pool
     */
    RefHashTableOfEnumerator<Grammar> getGrammarEnumerator() const;

    /**
     * Get an enumeration of the referenced Grammars 
     *
     * @return enumeration of referenced Grammars
     */
    RefHashTableOfEnumerator<Grammar> getReferencedGrammarEnumerator() const;

    /**
     * Get an enumeration of the cached Grammars in the Grammar pool
     *
     * @return enumeration of the cached Grammars in Grammar pool
     */
    RefHashTableOfEnumerator<Grammar> getCachedGrammarEnumerator() const;

    /**
     * Get a string pool of schema grammar element/attribute names/prefixes
     * (used by TraverseSchema)
     *
     * @return a string pool of schema grammar element/attribute names/prefixes
     */
    XMLStringPool* getStringPool();

    /**
     * Is the specified Namespace key in Grammar pool?
     *
     * @param  nameSpaceKey    Namespace key
     * @return True if Namespace key association is in the Grammar pool.
     */
    bool containsNameSpace( const XMLCh* const nameSpaceKey );

    inline XMLGrammarPool* getGrammarPool() const;

    inline MemoryManager* getGrammarPoolMemoryManager() const;

    //@}

    /** @name Setter methods */
    //@{

    /**
      * Set the 'Grammar caching' flag
      */
    void cacheGrammarFromParse(const bool newState);

    /**
      * Set the 'Use cached grammar' flag
      */
    void useCachedGrammarInParse(const bool newState);

    //@}


    /** @name GrammarResolver methods */
    //@{
    /**
     * Add the Grammar with Namespace Key associated to the Grammar Pool.
     * The Grammar will be owned by the Grammar Pool.
     *
     * @param  grammarToAdopt  Grammar abstraction used by validator.
     */
    void putGrammar(Grammar* const               grammarToAdopt );

    /**
     * Returns the Grammar with Namespace Key associated from the Grammar Pool
     * The Key entry is removed from the table (grammar is not deleted if
     * adopted - now owned by caller).
     *
     * @param  nameSpaceKey    Key to associate with Grammar abstraction
     */
    Grammar* orphanGrammar(const XMLCh* const nameSpaceKey);

    /**
     * Cache the grammars in fGrammarBucket to fCachedGrammarRegistry.
     * If a grammar with the same key is already cached, an exception is
     * thrown and none of the grammars will be cached.
     */
    void cacheGrammars();

    /**
     * Reset internal Namespace/Grammar registry.
     */
    void reset();
    void resetCachedGrammar();

    /**
     * Returns an XSModel, either from the GrammarPool or by creating one
     */
    XSModel*    getXSModel();


    ValueVectorOf<SchemaGrammar*>* getGrammarsToAddToXSModel();

    //@}

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    GrammarResolver(const GrammarResolver&);
    GrammarResolver& operator=(const GrammarResolver&);

    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fStringPool            The string pool used by TraverseSchema to store
    //                         element/attribute names and prefixes.
    //                         Always owned by Grammar pool implementation
    //
    //  fGrammarBucket         The parsed Grammar Pool, if no caching option.
    //
    //  fGrammarFromPool       Referenced Grammar Set, not owned
    //
    //  fGrammarPool           The Grammar Set either plugged or created. 
    //
    //  fDataTypeReg           DatatypeValidatorFactory registry
    //
    //  fMemoryManager         Pluggable memory manager for dynamic memory
    //                         allocation/deallocation
    // -----------------------------------------------------------------------
    bool                            fCacheGrammar;
    bool                            fUseCachedGrammar;
    bool                            fGrammarPoolFromExternalApplication;
    XMLStringPool*                  fStringPool;
    RefHashTableOf<Grammar>*        fGrammarBucket;
    RefHashTableOf<Grammar>*        fGrammarFromPool;
    DatatypeValidatorFactory*       fDataTypeReg;
    MemoryManager*                  fMemoryManager;
    XMLGrammarPool*                 fGrammarPool;
    XSModel*                        fXSModel;
    XSModel*                        fGrammarPoolXSModel;
    ValueVectorOf<SchemaGrammar*>*  fGrammarsToAddToXSModel;
};

inline XMLStringPool* GrammarResolver::getStringPool() {

    return fStringPool;
}


inline void GrammarResolver::useCachedGrammarInParse(const bool aValue)
{
    fUseCachedGrammar = aValue;
}

inline XMLGrammarPool* GrammarResolver::getGrammarPool() const
{
    return fGrammarPool;
}

inline MemoryManager* GrammarResolver::getGrammarPoolMemoryManager() const
{
    return fGrammarPool->getMemoryManager();
}

inline ValueVectorOf<SchemaGrammar*>* GrammarResolver::getGrammarsToAddToXSModel()
{
    return fGrammarsToAddToXSModel;
}

inline DatatypeValidatorFactory* GrammarResolver::getBuiltinDatatypeValidatorFactory()
{
    return fDataTypeReg;
}

XERCES_CPP_NAMESPACE_END

#endif
