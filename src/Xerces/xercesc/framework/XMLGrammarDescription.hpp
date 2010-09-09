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
 * $Id: XMLGrammarDescription.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XMLGRAMMARDESCRIPTION_HPP)
#define XERCESC_INCLUDE_GUARD_XMLGRAMMARDESCRIPTION_HPP

#include <xercesc/util/XMemory.hpp>
#include <xercesc/validators/common/Grammar.hpp>

#include <xercesc/internal/XSerializable.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XMLPARSER_EXPORT XMLGrammarDescription : public XSerializable, public XMemory
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
    virtual ~XMLGrammarDescription();
    //@}

    // -----------------------------------------------------------------------
    /** @name The Grammar Description Interface */
    // -----------------------------------------------------------------------
    //@{     
    /**
      * getGrammarType
      *
      */
    virtual Grammar::GrammarType   getGrammarType() const = 0;
    
    /**
      * getGrammarKey
      *
      */
    virtual const XMLCh*           getGrammarKey() const = 0;    
    //@}
    
    inline MemoryManager*          getMemoryManager() const;

    /***
     * Support for Serialization/De-serialization
     ***/
    DECL_XSERIALIZABLE(XMLGrammarDescription)

protected :
    // -----------------------------------------------------------------------
    /**  Hidden Constructors */
    // -----------------------------------------------------------------------
    //@{
    XMLGrammarDescription(MemoryManager* const memMgr = XMLPlatformUtils::fgMemoryManager);
    //@}

private :
    // -----------------------------------------------------------------------
    /** name  Unimplemented copy constructor and operator= */
    // -----------------------------------------------------------------------
    //@{
    XMLGrammarDescription(const XMLGrammarDescription& );
    XMLGrammarDescription& operator=(const XMLGrammarDescription& );
    //@}

    // -----------------------------------------------------------------------
    //
    // fMemMgr: plugged-in (or defaulted-in) memory manager, 
    //          not owned 
    //          no reset after initialization
	//          allow derivatives to access directly
    //
    // -----------------------------------------------------------------------    
    MemoryManager* const  fMemMgr;     
};

inline MemoryManager* XMLGrammarDescription::getMemoryManager() const
{
    return fMemMgr;
}

XERCES_CPP_NAMESPACE_END

#endif
