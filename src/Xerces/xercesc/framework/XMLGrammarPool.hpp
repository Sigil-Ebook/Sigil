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
 * $Id: XMLGrammarPool.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XMLGRAMMARPOOL_HPP)
#define XERCESC_INCLUDE_GUARD_XMLGRAMMARPOOL_HPP

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/RefHashTableOf.hpp>
#include <xercesc/util/XMemory.hpp>
#include <xercesc/framework/psvi/XSModel.hpp>


XERCES_CPP_NAMESPACE_BEGIN

class Grammar;
class XMLGrammarDescription;
class DTDGrammar;
class SchemaGrammar;
class XMLDTDDescription;
class XMLSchemaDescription;
class XMLStringPool;
class BinInputStream;
class BinOutputStream;

class XMLPARSER_EXPORT XMLGrammarPool : public XMemory
{
public :
    // -----------------------------------------------------------------------
    /** @name Virtual destructor for derived classes */
    // -----------------------------------------------------------------------
    //@{

    /**
      * virtual destructor
      *
      */
    virtual ~XMLGrammarPool(){};
    //@}

    // -----------------------------------------------------------------------
    /** @name The Grammar Pool Interface */
    // -----------------------------------------------------------------------
    //@{

    /**
      * cacheGrammar
      *
      * Provide the grammar pool with an opportunity
      * to cache the given grammar.  If the pool does not choose to do so,
      * it should return false; otherwise, it should return true, so that
      * the caller knows whether the grammar has been adopted.
      *
      * @param gramToCache the Grammar to be cached in the grammar pool
      * @return true if the grammar pool has elected to cache the grammar (in which case
      * it is assumed to have adopted it); false if it does not cache it
	  *
      */
    virtual bool           cacheGrammar(Grammar* const               gramToCache) = 0;
    
    /**
      * retrieveGrammar
      *
      * @param gramDesc the Grammar Description used to search for grammar
	  *                  cached in the grammar pool
	  *
      */
    virtual Grammar*       retrieveGrammar(XMLGrammarDescription* const gramDesc) = 0;
    
        
    /**
      * orphanGrammar
      *
      * grammar removed from the grammar pool and owned by the caller
      *
      * @param nameSpaceKey Key used to search for grammar in the grammar pool
      * @return the grammar that was removed from the pool (0 if none)
      */
    virtual Grammar*       orphanGrammar(const XMLCh* const nameSpaceKey) = 0;  


    /**
     * Get an enumeration of the cached Grammars in the Grammar pool
     *
     * @return enumeration of the cached Grammars in Grammar pool
     */
    virtual RefHashTableOfEnumerator<Grammar> getGrammarEnumerator() const = 0;

    /**
      * clear
      *
      * all grammars are removed from the grammar pool and deleted.
      * @return true if the grammar pool was cleared. false if it did not.
      */
    virtual bool           clear() = 0;
        
    /**
      * lockPool
      *
	  * When this method is called by the application, the 
      * grammar pool should stop adding new grammars to the cache.
      * This should result in the grammar pool being sharable
      * among parsers operating in different threads.
      *
      */
    virtual void           lockPool() = 0;
    
    /**
      * unlockPool
      *
	  * After this method has been called, the grammar pool implementation
      * should return to its default behaviour when cacheGrammars(...) is called.
      * One effect, depending on the underlying implementation, is that the grammar pool
      * may no longer be thread-safe (even on read operations).
      *
      * For PSVI support any previous XSModel that was produced will be deleted.
      */
    virtual void           unlockPool() = 0;

    //@}

    // -----------------------------------------------------------------------
    /** @name  Factory interface */
    // -----------------------------------------------------------------------
    //@{

    /**
      * createDTDGrammar
      *
      */
    virtual DTDGrammar*            createDTDGrammar() = 0;

    /**
      * createSchemaGrammar
      *
      */
    virtual SchemaGrammar*         createSchemaGrammar() = 0;
                    
    /**
      * createDTDDescription
      *
      */	
    virtual XMLDTDDescription*     createDTDDescription(const XMLCh* const systemId) = 0;
    /**
      * createSchemaDescription
      *
      */		
    virtual XMLSchemaDescription*  createSchemaDescription(const XMLCh* const targetNamespace) = 0;

    //@}

    // -----------------------------------------------------------------------
    /** @name  schema component model support */
    // -----------------------------------------------------------------------                                                        
    //@{

    /***
      * Return an XSModel derived from the components of all SchemaGrammars
      * in the grammar pool.  If the pool is locked, this should
      * be a thread-safe operation.
      *
      * NOTE: The function should NEVER return NULL.  If there are no grammars in
      *       the pool it should return an XSModel containing the Schema for Schema.      
      *
      * Calling getXSModel() on an unlocked grammar pool may result in the
      * creation of a new XSModel with the old XSModel being deleted.
      * The bool parameter will indicate if the XSModel was changed.
      *     
      */
    virtual XSModel *getXSModel(bool& XSModelWasChanged) = 0;
	
    // @}

    // -----------------------------------------------------------------------
    /** @name  Getter */
    // -----------------------------------------------------------------------                                                        
    //@{

    /**
      * getMemoryManager
      *
      */    
	inline MemoryManager*    getMemoryManager()
    {
        return fMemMgr;
    }

    /**
      * Return an XMLStringPool for use by validation routines.  
      * Implementations should not create a string pool on each call to this
      * method, but should maintain one string pool for all grammars
      * for which this pool is responsible.
      */
    virtual XMLStringPool *getURIStringPool() = 0;
    //@}

    // -----------------------------------------------------------------------
    /** serialization and deserialization support */
    // -----------------------------------------------------------------------                                                        

    /***
      *
      * 1. Context: Serialize/Deserialize All Grammars In One Session
      *
      *    Since it is common that a declaration in one grammar may reference 
      *    to definitions in other grammars, it is required to serialize those 
      *    related (or interdependent) grammars in to one persistent data store 
      *    in one serialization session (storing), and deserialize them from the
      *    persistent data store in one deserialization session (loading) back
      *    to the grammar pool.    
      *
      * 2. Multiple serializations
      *
      *    It is acceptable that client application requests more than one 
      *    grammar serialization on a particular grammar pool, to track the 
      *    different grammars cached, or for whatever reasons that client 
      *    application is interested in. 
      *
      * 3. Multiple deserializations
      * 
      *    Request for grammar deserialization either after the grammar pool has 
      *    its own cached grammars, or request for more than one grammar 
      *    deserialization, may cause undesired and unpredictable consequence
      *    and therefore client application shall be aware that individual 
      *    implementationis may NOT support this.
      *
      *    However it is strongly recommended that the client application requests 
      *    no more than one grammar deserialization even a particular implementation
      *    may allow multiple deserializations.
      *
      * 4. Locking
      *
      *    Both serialization and deserialization requires to lock the grammar pool
      *    before operation and unlock after operation. In the case the grammar pool
      *    is locked by a third party, the request for serialization/deserialization
      *    will NOT be entertained.
      *
      * 5. Versioning
      *
      *    The Persistent data store has a version tag to be verified during 
      *    deserialization, thus a grammar pool may decide if it supports
      *    a binary data created by a different release of Xerces.
      * 
      * 6. Clean up
      *
      *    The client application shall be aware that in the event of an exception
      *    thrown due to a corrupted data store during deserialization, implementation
      *    may not be able to clean up all resources allocated, and therefore it is 
      *    client application's responsibility to clean up those unreleased resources.
      *
      *
      */
    virtual void     serializeGrammars(BinOutputStream* const)  = 0; 
    virtual void     deserializeGrammars(BinInputStream* const) = 0;       
	   
    /*
     * Set/get a flag to not create XSAnnotations when deserializing the grammar.
     * Defaults to false (create XSAnnotations when deserializing the grammar).
     */
    inline void setIgnoreSerializedAnnotations(const bool flag)
    {
        fIgnoreSerializedAnnotations = flag;
    };
    inline bool getIgnoreSerializedAnnotations() const
    {
        return fIgnoreSerializedAnnotations;
    };

protected :
    // -----------------------------------------------------------------------
    /**  Hidden Constructors */
    // -----------------------------------------------------------------------
    //@{
    XMLGrammarPool(MemoryManager* const memMgr = XMLPlatformUtils::fgMemoryManager)
    :fMemMgr(memMgr)
    ,fIgnoreSerializedAnnotations(false)
    {
    };
    //@}

private :
    // -----------------------------------------------------------------------
    /** name  Unimplemented copy constructor and operator= */
    // -----------------------------------------------------------------------
    //@{
    XMLGrammarPool(const XMLGrammarPool& );
    XMLGrammarPool& operator=(const XMLGrammarPool& );
    //@}

    // -----------------------------------------------------------------------
    //
    // fMemMgr: plugged-in (or defaulted-in) memory manager
    //          not owned 
    //          no reset after initialization
    //
    // -----------------------------------------------------------------------
    
    MemoryManager* const  fMemMgr;
    bool fIgnoreSerializedAnnotations;

};

XERCES_CPP_NAMESPACE_END

#endif
