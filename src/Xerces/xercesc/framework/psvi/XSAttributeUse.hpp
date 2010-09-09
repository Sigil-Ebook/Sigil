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
 * $Id: XSAttributeUse.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XSATTRIBUTEUSE_HPP)
#define XERCESC_INCLUDE_GUARD_XSATTRIBUTEUSE_HPP

#include <xercesc/framework/psvi/XSObject.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/**
 * This class describes all properties of a Schema Attribute
 * Use component.
 * This is *always* owned by the validator /parser object from which
 * it is obtained.  
 */

// forward declarations
class XSAttributeDeclaration;

class XMLPARSER_EXPORT XSAttributeUse : public XSObject
{
public:

    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    /** @name Constructors */
    //@{

    /**
      * The default constructor 
      * @param  xsAttDecl
      * @param  xsModel
      * @param  manager     The configurable memory manager
      */
    XSAttributeUse
    (
        XSAttributeDeclaration* const xsAttDecl,
        XSModel* const xsModel,
        MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    //@};

    /** @name Destructor */
    //@{
    ~XSAttributeUse();
    //@}

    //---------------------
    /** @name XSAttributeUse methods */

    //@{

    /**
     * [required]: determines whether this use of an attribute declaration 
     * requires an appropriate attribute information item to be present, or 
     * merely allows it. 
     */
    bool getRequired() const;

    /**
     * [attribute declaration]: provides the attribute declaration itself, 
     * which will in turn determine the simple type definition used. 
     */
    XSAttributeDeclaration *getAttrDeclaration() const;

    /**
     * Value Constraint: one of default, fixed. 
     */
    XSConstants::VALUE_CONSTRAINT getConstraintType() const;

    /**
     * Value Constraint: The actual value. 
     */
    const XMLCh *getConstraintValue();

    //@}

    //----------------------------------
    /** methods needed by implementation */

    //@{

    //@}

private:

    // set data
    void set
    (
        const bool isRequired
        , XSConstants::VALUE_CONSTRAINT constraintType
        , const XMLCh* const constraintValue
    );

    friend class XSObjectFactory;
    
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XSAttributeUse(const XSAttributeUse&);
    XSAttributeUse & operator=(const XSAttributeUse &);

protected:

    // -----------------------------------------------------------------------
    //  data members
    // -----------------------------------------------------------------------
    bool                          fRequired;
    XSConstants::VALUE_CONSTRAINT fConstraintType;   
    const XMLCh*                  fConstraintValue;
    XSAttributeDeclaration*       fXSAttributeDeclaration;
};

inline XSAttributeDeclaration *XSAttributeUse::getAttrDeclaration() const
{
    return fXSAttributeDeclaration;
}

inline bool XSAttributeUse::getRequired() const
{
    return fRequired;
}

inline XSConstants::VALUE_CONSTRAINT XSAttributeUse::getConstraintType() const
{
    return fConstraintType;
}

inline const XMLCh *XSAttributeUse::getConstraintValue()
{
    return fConstraintValue;
}

XERCES_CPP_NAMESPACE_END

#endif
