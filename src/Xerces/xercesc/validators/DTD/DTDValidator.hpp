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
 * $Id: DTDValidator.hpp 676911 2008-07-15 13:27:32Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DTDVALIDATOR_HPP)
#define XERCESC_INCLUDE_GUARD_DTDVALIDATOR_HPP

#include <xercesc/util/NameIdPool.hpp>
#include <xercesc/framework/XMLValidator.hpp>
#include <xercesc/validators/DTD/DTDGrammar.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XMLMsgLoader;


//
//  This is a derivative of the abstract validator interface. This class
//  implements a validator that supports standard XML 1.0 DTD semantics.
//  This class handles scanning the internal and external subsets of the
//  DTD, and provides the standard validation services against the DTD info
//  it found.
//
class VALIDATORS_EXPORT DTDValidator : public XMLValidator
{
public:
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    DTDValidator(XMLErrorReporter* const errReporter = 0);
    virtual ~DTDValidator();

    // -----------------------------------------------------------------------
    //  Implementation of the XMLValidator interface
    // -----------------------------------------------------------------------
    virtual bool checkContent
    (
        XMLElementDecl* const   elemDecl
        , QName** const         children
        , XMLSize_t             childCount
        , XMLSize_t*            indexFailingChild
    );

    virtual void faultInAttr
    (
                XMLAttr&    toFill
        , const XMLAttDef&  attDef
    )   const;

    virtual void preContentValidation(bool reuseGrammar,
                                      bool validateDefAttr = false);

    virtual void postParseValidation();

    virtual void reset();

    virtual bool requiresNamespaces() const;

    virtual void validateAttrValue
    (
        const   XMLAttDef*                  attDef
        , const XMLCh* const                attrValue
        , bool                              preValidation = false
        , const XMLElementDecl*             elemDecl = 0
    );
    virtual void validateElement
    (
        const   XMLElementDecl*             elemDef
    );
    virtual Grammar* getGrammar() const;
    virtual void setGrammar(Grammar* aGrammar);

    // -----------------------------------------------------------------------
    //  Virtual DTD handler interface.
    // -----------------------------------------------------------------------
    virtual bool handlesDTD() const;

    // -----------------------------------------------------------------------
    //  Virtual Schema handler interface. handlesSchema() always return false.
    // -----------------------------------------------------------------------
    virtual bool handlesSchema() const;

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    DTDValidator(const DTDValidator &);
    DTDValidator& operator = (const  DTDValidator&);

    // -----------------------------------------------------------------------
    //  Helper
    // -----------------------------------------------------------------------
    void   checkTokenList(const XMLAttDef&  attDef
                        ,       bool        toValidateNotation);

    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fDTDGrammar
    //      The DTD information stored.
    //
    // -----------------------------------------------------------------------
    DTDGrammar*                     fDTDGrammar;
};

// ---------------------------------------------------------------------------
//  Virtual interface
// ---------------------------------------------------------------------------
inline Grammar* DTDValidator::getGrammar() const {
    return fDTDGrammar;
}

inline void DTDValidator::setGrammar(Grammar* aGrammar) {
    fDTDGrammar = (DTDGrammar*) aGrammar;
}

inline void DTDValidator::validateElement (const   XMLElementDecl*) {
    // no special DTD Element validation
}

// ---------------------------------------------------------------------------
//  DTDValidator: DTD handler interface
// ---------------------------------------------------------------------------
inline bool DTDValidator::handlesDTD() const
{
    // We definitely want to handle DTD scanning
    return true;
}

// ---------------------------------------------------------------------------
//  DTDValidator: Schema handler interface
// ---------------------------------------------------------------------------
inline bool DTDValidator::handlesSchema() const
{
    // No Schema scanning
    return false;
}

XERCES_CPP_NAMESPACE_END

#endif
