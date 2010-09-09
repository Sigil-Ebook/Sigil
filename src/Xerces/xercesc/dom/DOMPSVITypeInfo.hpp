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

#if !defined(XERCESC_INCLUDE_GUARD_DOMPSVITYPEINFO_HPP)
#define XERCESC_INCLUDE_GUARD_DOMPSVITYPEINFO_HPP

//------------------------------------------------------------------------------------
//  Includes
//------------------------------------------------------------------------------------
#include <xercesc/util/XMLString.hpp>


XERCES_CPP_NAMESPACE_BEGIN

/**
  * The <code>DOMPSVITypeInfo</code> interface represent the PSVI info used by 
  * <code>DOMElement</code> or <code>DOMAttr</code> nodes, specified in the 
  * schemas associated with the document. 
  */
class CDOM_EXPORT DOMPSVITypeInfo
{
protected:
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    /** @name Hidden constructors */
    //@{
    DOMPSVITypeInfo() {};
    //@}

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    /** @name Unimplemented constructors and operators */
    //@{
    DOMPSVITypeInfo(const DOMPSVITypeInfo &);
    DOMPSVITypeInfo & operator = (const DOMPSVITypeInfo &);
    //@}

public:

    enum PSVIProperty
    {
        PSVI_Validity
        , PSVI_Validation_Attempted
        , PSVI_Type_Definition_Type
        , PSVI_Type_Definition_Name
        , PSVI_Type_Definition_Namespace
        , PSVI_Type_Definition_Anonymous
        , PSVI_Nil
        , PSVI_Member_Type_Definition_Name
        , PSVI_Member_Type_Definition_Namespace
        , PSVI_Member_Type_Definition_Anonymous
        , PSVI_Schema_Default
        , PSVI_Schema_Normalized_Value
        , PSVI_Schema_Specified
    };

    // -----------------------------------------------------------------------
    //  All constructors are hidden, just the destructor is available
    // -----------------------------------------------------------------------
    /** @name Destructor */
    //@{
    /**
     * Destructor
     *
     */
    virtual ~DOMPSVITypeInfo() {};
    //@}

    //@{
    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    /**
     * Returns the string value of the specified PSVI property associated to a 
     * <code>DOMElement</code> or <code>DOMAttr</code>, or null if not available.
     *
     *
     * @return the string value of the specified PSVI property associated to a 
     * <code>DOMElement</code> or <code>DOMAttr</code>, or null if not available.
     */
    virtual const XMLCh* getStringProperty(PSVIProperty prop) const = 0;

    /**
     * Returns the numeric value of the specified PSVI property associated to a 
     * <code>DOMElement</code> or <code>DOMAttr</code>, or null if not available.
     *
     *
     * @return the numeric value of the specified PSVI property associated to a 
     * <code>DOMElement</code> or <code>DOMAttr</code>, or null if not available.
     */
    virtual int getNumericProperty(PSVIProperty prop) const = 0;
    //@}
};

XERCES_CPP_NAMESPACE_END

#endif

/**
 * End of file DOMPSVITypeInfo.hpp
 */
