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
 * $Id: Grammar.hpp 676911 2008-07-15 13:27:32Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_GRAMMAR_HPP)
#define XERCESC_INCLUDE_GUARD_GRAMMAR_HPP

#include <limits.h>

#include <xercesc/framework/XMLElementDecl.hpp>
#include <xercesc/framework/XMLEntityDecl.hpp>
#include <xercesc/framework/XMLNotationDecl.hpp>

#include <xercesc/internal/XSerializable.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XMLGrammarDescription;

//
// This abstract class specifies the interface for a Grammar
//

class VALIDATORS_EXPORT Grammar : public XSerializable, public XMemory
{
public:

    // -----------------------------------------------------------------------
    //  Class Specific Types
    //
    //  DTDGrammarType    - Indicate this Grammar is built from a DTD.
    //  SchemaGrammarType - Indicate this Grammar is built from a Schema.
    //
    //  TOP_LEVEL_SCOPE - outermost scope level (i.e. global) of a declaration.
    //                    For DTD, all element decls and attribute decls always
    //                    have TOP_LEVEL_SCOPE.  For schema, it may vary if
    //                    it is inside a complex type.
    //
    //  UNKNOWN_SCOPE   - unknown scope level.  None of the decls should have this.
    //
    // -----------------------------------------------------------------------
    enum GrammarType {
        DTDGrammarType
      , SchemaGrammarType
      , UnKnown
    };

    enum {
    	// These are well-known values that must simply be larger
    	// than any reasonable scope
		 UNKNOWN_SCOPE		= UINT_MAX - 0
	   , TOP_LEVEL_SCOPE	= UINT_MAX - 1
    };

    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    virtual ~Grammar(){};

    // -----------------------------------------------------------------------
    //  Virtual Getter methods
    // -----------------------------------------------------------------------
    virtual GrammarType getGrammarType() const =0;
    virtual const XMLCh* getTargetNamespace() const =0;
    virtual bool getValidated() const = 0;

    // Element Decl

    // this method should only be used while the grammar is being
    // constructed, not while it is being used
    // in a validation episode!
    virtual XMLElementDecl* findOrAddElemDecl
    (
        const   unsigned int    uriId
        , const XMLCh* const    baseName
        , const XMLCh* const    prefixName
        , const XMLCh* const    qName
        , unsigned int          scope
        ,       bool&           wasAdded
    ) = 0;

    virtual XMLSize_t getElemId
    (
        const   unsigned int    uriId
        , const XMLCh* const    baseName
        , const XMLCh* const    qName
        , unsigned int          scope
    )   const = 0;

    virtual const XMLElementDecl* getElemDecl
    (
        const   unsigned int    uriId
        , const XMLCh* const    baseName
        , const XMLCh* const    qName
        , unsigned int          scope
    )   const = 0;

    virtual XMLElementDecl* getElemDecl
    (
        const   unsigned int    uriId
        , const XMLCh* const    baseName
        , const XMLCh* const    qName
        , unsigned int          scope
    ) = 0;

    virtual const XMLElementDecl* getElemDecl
    (
        const   unsigned int    elemId
    )   const = 0;

    virtual XMLElementDecl* getElemDecl
    (
        const   unsigned int    elemId
    ) = 0;

    // Notation
    virtual const XMLNotationDecl* getNotationDecl
    (
        const   XMLCh* const    notName
    )   const=0;

    virtual XMLNotationDecl* getNotationDecl
    (
        const   XMLCh* const    notName
    )=0;

    // -----------------------------------------------------------------------
    //  Virtual Setter methods
    // -----------------------------------------------------------------------
    virtual XMLElementDecl* putElemDecl
    (
        const   unsigned int    uriId
        , const XMLCh* const    baseName
        , const XMLCh* const    prefixName
        , const XMLCh* const    qName
        , unsigned int          scope
        , const bool            notDeclared = false
    ) = 0;

    virtual XMLSize_t putElemDecl
    (
        XMLElementDecl* const elemDecl
        , const bool          notDeclared = false
    )   = 0;

    virtual XMLSize_t putNotationDecl
    (
        XMLNotationDecl* const notationDecl
    )   const=0;

    virtual void setValidated(const bool newState) = 0;

    // -----------------------------------------------------------------------
    //  Virtual methods
    // -----------------------------------------------------------------------
    virtual void reset()=0;

    virtual void                    setGrammarDescription( XMLGrammarDescription*) = 0;
    virtual XMLGrammarDescription*  getGrammarDescription() const = 0;

    /***
     * Support for Serialization/De-serialization
     ***/
    DECL_XSERIALIZABLE(Grammar)

	static void     storeGrammar(XSerializeEngine&        serEng
                               , Grammar* const           grammar);

	static Grammar* loadGrammar(XSerializeEngine& serEng);

protected :
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    Grammar(){};

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    Grammar(const Grammar&);
    Grammar& operator=(const Grammar&);
};

XERCES_CPP_NAMESPACE_END

#endif
