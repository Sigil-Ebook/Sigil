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
 * $Id: RangeTokenMap.hpp 678879 2008-07-22 20:05:05Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_RANGETOKENMAP_HPP)
#define XERCESC_INCLUDE_GUARD_RANGETOKENMAP_HPP

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/Mutexes.hpp>
#include <xercesc/util/RefHashTableOf.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Forward Declaration
// ---------------------------------------------------------------------------
class RangeToken;
class RangeFactory;
class TokenFactory;
class XMLStringPool;

class XMLUTIL_EXPORT RangeTokenElemMap : public XMemory
{

public:
    RangeTokenElemMap(unsigned int categoryId);
    ~RangeTokenElemMap();

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    unsigned int getCategoryId() const;
    RangeToken*  getRangeToken(const bool complement = false) const;

    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------
    void setRangeToken(RangeToken* const tok, const bool complement = false);
    void setCategoryId(const unsigned int categId);

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    RangeTokenElemMap(const RangeTokenElemMap&);
    RangeTokenElemMap& operator=(const RangeTokenElemMap&);

    // Data members
    unsigned int fCategoryId;
    RangeToken*  fRange;
    RangeToken*  fNRange;
};


class XMLUTIL_EXPORT RangeTokenMap : public XMemory
{
public:
    // -----------------------------------------------------------------------
    //  Putter methods
    // -----------------------------------------------------------------------
    void addCategory(const XMLCh* const categoryName);
    void addRangeMap(const XMLCh* const categoryName,
                     RangeFactory* const rangeFactory);
    void addKeywordMap(const XMLCh* const keyword,
                       const XMLCh* const categoryName);

    // -----------------------------------------------------------------------
    //  Instance methods
    // -----------------------------------------------------------------------
    static RangeTokenMap* instance();

    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------
    void setRangeToken(const XMLCh* const keyword, RangeToken* const tok,
                       const bool complement = false);

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    TokenFactory* getTokenFactory() const;

protected:
    // -----------------------------------------------------------------------
    //  Constructor and destructors
    // -----------------------------------------------------------------------
    RangeTokenMap(MemoryManager* manager);
    ~RangeTokenMap();

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    /*
     *  Gets a commonly used RangeToken from the token registry based on the
     *  range name - Called by TokenFactory.
     */
     RangeToken* getRange(const XMLCh* const name,
                          const bool complement = false);

     RefHashTableOf<RangeTokenElemMap>* getTokenRegistry() const;
     RefHashTableOf<RangeFactory>* getRangeMap() const;
     XMLStringPool* getCategories() const;

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    RangeTokenMap(const RangeTokenMap&);
    RangeTokenMap& operator=(const RangeTokenMap&);

    // -----------------------------------------------------------------------
    //  Private Helpers methods
    // -----------------------------------------------------------------------
    /*
     *  Initializes the registry with a set of commonly used RangeToken
     *  objects.
     */
    void initializeRegistry();
    void buildTokenRanges();
    void cleanUp();
    friend class TokenFactory;

    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fTokenRegistry
    //      Contains a set of commonly used tokens
    //
    //  fRangeMap
    //      Contains a map between a category name and a RangeFactory object.
    //
    //  fCategories
    //      Contains range categories names
    //
    //  fTokenFactory
    //      Token factory object
    //
    //  fInstance
    //      A RangeTokenMap instance
    //
    //  fMutex
    //      A mutex object for synchronization
    // -----------------------------------------------------------------------
    RefHashTableOf<RangeTokenElemMap>* fTokenRegistry;
    RefHashTableOf<RangeFactory>*      fRangeMap;
    XMLStringPool*                     fCategories;
    TokenFactory*                      fTokenFactory;
    XMLMutex                           fMutex;
    static RangeTokenMap*              fInstance;

    friend class XMLInitializer;
};

// ---------------------------------------------------------------------------
//  RangeTokenElemMap: Getter methods
// ---------------------------------------------------------------------------
inline unsigned int RangeTokenElemMap::getCategoryId() const {

    return fCategoryId;
}

inline RangeToken* RangeTokenElemMap::getRangeToken(const bool complement) const {

    return complement ? fNRange : fRange;
}

// ---------------------------------------------------------------------------
//  RangeTokenElemMap: Setter methods
// ---------------------------------------------------------------------------
inline void RangeTokenElemMap::setCategoryId(const unsigned int categId) {

    fCategoryId = categId;
}

inline void RangeTokenElemMap::setRangeToken(RangeToken* const tok,
                                      const bool complement) {

    if (complement)
        fNRange = tok;
    else
        fRange = tok;
}

// ---------------------------------------------------------------------------
//  RangeTokenMap: Getter methods
// ---------------------------------------------------------------------------
inline RefHashTableOf<RangeTokenElemMap>* RangeTokenMap::getTokenRegistry() const {

    return fTokenRegistry;
}

inline RefHashTableOf<RangeFactory>* RangeTokenMap::getRangeMap() const {

    return fRangeMap;
}

inline XMLStringPool* RangeTokenMap::getCategories() const {

    return fCategories;
}

inline TokenFactory* RangeTokenMap::getTokenFactory() const {

    return fTokenFactory;
}

XERCES_CPP_NAMESPACE_END

#endif

/**
  *    End file RangeToken.hpp
  */
