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
 * $Id: RangeFactory.hpp 678879 2008-07-22 20:05:05Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_RANGEFACTORY_HPP)
#define XERCESC_INCLUDE_GUARD_RANGEFACTORY_HPP

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/XMemory.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Forward Declaration
// ---------------------------------------------------------------------------
class RangeTokenMap;

class XMLUTIL_EXPORT RangeFactory : public XMemory
{
public:
    // -----------------------------------------------------------------------
    //  Constructors and destructors
    // -----------------------------------------------------------------------
    virtual ~RangeFactory();

    //-----------------------------------------------------------------------
    //  Initialization methods
    // -----------------------------------------------------------------------
    /**
     * To maintain src code compatibility, we added a default parameter.
     * The caller is expected to pass in a valid RangeTokenMap instance.
     */
    virtual void initializeKeywordMap(RangeTokenMap *rangeTokMap = 0) = 0;

    /*
     * Used by children to build commonly used ranges
     * To maintain src code compatibility, we added a default parameter.
     * The caller is expected to pass in a valid RangeTokenMap instance.
     */
    virtual void buildRanges(RangeTokenMap *rangeTokMap = 0) = 0;

protected:
    // -----------------------------------------------------------------------
    //  Constructor and destructors
    // -----------------------------------------------------------------------
    RangeFactory();

    //friend class RangeTokenMap;

    // -----------------------------------------------------------------------
    //  Data
    // -----------------------------------------------------------------------
    bool fRangesCreated;
    bool fKeywordsInitialized;

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    RangeFactory(const RangeFactory&);
    RangeFactory& operator=(const RangeFactory&);
};

XERCES_CPP_NAMESPACE_END

#endif

/**
  *    End file RangeFactory.hpp
  */
