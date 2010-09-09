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

//
//  This file is part of the internal implementation of the C++ XML DOM.
//  It should NOT be included or used directly by application programs.
//
//  Applications should include the file <xercesc/dom/DOM.hpp> for the entire
//  DOM API, or xercesc/dom/DOM*.hpp for individual DOM classes, where the class
//  name is substituded for the *.
//


#if !defined(XERCESC_INCLUDE_GUARD_DOMTYPEINFOIMPL_HPP)
#define XERCESC_INCLUDE_GUARD_DOMTYPEINFOIMPL_HPP

//------------------------------------------------------------------------------------
//  Includes
//------------------------------------------------------------------------------------
#include <xercesc/dom/DOMTypeInfo.hpp>
#include <xercesc/dom/DOMPSVITypeInfo.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class DOMDocumentImpl;

class CDOM_EXPORT DOMTypeInfoImpl : public DOMTypeInfo, public DOMPSVITypeInfo
{
public:

    //-----------------------------------------------------------------------------------
    //  Constructor
    //-----------------------------------------------------------------------------------
    DOMTypeInfoImpl(const XMLCh* namespaceUri=0, const XMLCh* name=0);
    DOMTypeInfoImpl(DOMDocumentImpl* ownerDoc, const DOMPSVITypeInfo* sourcePSVI);

    static DOMTypeInfoImpl  g_DtdValidatedElement;
    static DOMTypeInfoImpl  g_DtdNotValidatedAttribute;
    static DOMTypeInfoImpl  g_DtdValidatedCDATAAttribute;
    static DOMTypeInfoImpl  g_DtdValidatedIDAttribute;
    static DOMTypeInfoImpl  g_DtdValidatedIDREFAttribute;
    static DOMTypeInfoImpl  g_DtdValidatedIDREFSAttribute;
    static DOMTypeInfoImpl  g_DtdValidatedENTITYAttribute;
    static DOMTypeInfoImpl  g_DtdValidatedENTITIESAttribute;
    static DOMTypeInfoImpl  g_DtdValidatedNMTOKENAttribute;
    static DOMTypeInfoImpl  g_DtdValidatedNMTOKENSAttribute;
    static DOMTypeInfoImpl  g_DtdValidatedNOTATIONAttribute;
    static DOMTypeInfoImpl  g_DtdValidatedENUMERATIONAttribute;

    // -----------------------------------------------------------------------
    //  DOMTypeInfo interface
    // -----------------------------------------------------------------------
    virtual const XMLCh* getTypeName() const;
    virtual const XMLCh* getTypeNamespace() const;
    virtual bool isDerivedFrom(const XMLCh* typeNamespaceArg, const XMLCh* typeNameArg, DerivationMethods derivationMethod) const;

    // -----------------------------------------------------------------------
    //  DOMPSVITypeInfo interface
    // -----------------------------------------------------------------------
    virtual const XMLCh* getStringProperty(PSVIProperty prop) const;
    virtual int getNumericProperty(PSVIProperty prop) const;

    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------
    virtual void setStringProperty(PSVIProperty prop, const XMLCh* value);
    virtual void setNumericProperty(PSVIProperty prop, int value);

protected:
    int             fBitFields;
    const XMLCh*    fTypeName;
    const XMLCh*    fTypeNamespace;
    const XMLCh*    fMemberTypeName;
    const XMLCh*    fMemberTypeNamespace;
    const XMLCh*    fDefaultValue;
    const XMLCh*    fNormalizedValue;

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    DOMTypeInfoImpl (const DOMTypeInfoImpl&);
    DOMTypeInfoImpl & operator = (const DOMTypeInfoImpl &);
};

XERCES_CPP_NAMESPACE_END

#endif

/**
 * End of file DOMTypeInfo.hpp
 */
