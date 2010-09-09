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
 * $Id: VecAttrListImpl.hpp 672273 2008-06-27 13:57:00Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_VECATTRLISTIMPL_HPP)
#define XERCESC_INCLUDE_GUARD_VECATTRLISTIMPL_HPP

#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/framework/XMLAttr.hpp>
#include <xercesc/util/RefVectorOf.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XMLPARSER_EXPORT VecAttrListImpl : public XMemory, public AttributeList
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    VecAttrListImpl();
    ~VecAttrListImpl();


    // -----------------------------------------------------------------------
    //  Implementation of the attribute list interface
    // -----------------------------------------------------------------------
    virtual XMLSize_t getLength() const;
    virtual const XMLCh* getName(const XMLSize_t index) const;
    virtual const XMLCh* getType(const XMLSize_t index) const;
    virtual const XMLCh* getValue(const XMLSize_t index) const;
    virtual const XMLCh* getType(const XMLCh* const name) const;
    virtual const XMLCh* getValue(const XMLCh* const name) const;
    virtual const XMLCh* getValue(const char* const name) const;


    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------
    void setVector
    (
        const   RefVectorOf<XMLAttr>* const srcVec
        , const XMLSize_t                   count
        , const bool                        adopt = false
    );


private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    VecAttrListImpl(const VecAttrListImpl&);
    VecAttrListImpl& operator=(const VecAttrListImpl&);


    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fAdopt
    //      Indicates whether the passed vector is to be adopted or not. If
    //      so, we destroy it when we are destroyed (and when a new vector is
    //      set!)
    //
    //  fCount
    //      The count of elements in the vector that should be considered
    //      valid. This is an optimization to allow vector elements to be
    //      reused over and over but a different count of them be valid for
    //      each use.
    //
    //  fVector
    //      The vector that provides the backing for the list.
    // -----------------------------------------------------------------------
    bool                        fAdopt;
    XMLSize_t                   fCount;
    const RefVectorOf<XMLAttr>* fVector;
};

XERCES_CPP_NAMESPACE_END

#endif
