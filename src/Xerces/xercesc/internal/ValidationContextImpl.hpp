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
 * $Id: ValidationContextImpl.hpp 729944 2008-12-29 17:03:32Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_VALIDATION_CONTEXTIMPL_HPP)
#define XERCESC_INCLUDE_GUARD_VALIDATION_CONTEXTIMPL_HPP

#include <xercesc/framework/ValidationContext.hpp>

XERCES_CPP_NAMESPACE_BEGIN
class ElemStack;
class NamespaceScope;

class XMLPARSER_EXPORT ValidationContextImpl : public ValidationContext
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
    virtual ~ValidationContextImpl();

    ValidationContextImpl(MemoryManager* const memMgr = XMLPlatformUtils::fgMemoryManager);

    //@}

    // -----------------------------------------------------------------------
    /** @name The ValidationContextImpl Interface */
    // -----------------------------------------------------------------------
    //@{

    /**
      * IDRefList
      *
      */
    virtual RefHashTableOf<XMLRefInfo>*  getIdRefList() const;

    virtual void                         setIdRefList(RefHashTableOf<XMLRefInfo>* const);

    virtual void                         clearIdRefList();

    virtual void                         addId(const XMLCh * const );

    virtual void                         addIdRef(const XMLCh * const );

    virtual void                         toCheckIdRefList(bool);

    /**
      * EntityDeclPool
      *
      */
    virtual const NameIdPool<DTDEntityDecl>* getEntityDeclPool() const;

    virtual const NameIdPool<DTDEntityDecl>* setEntityDeclPool(const NameIdPool<DTDEntityDecl>* const);    
           
    virtual void                             checkEntity(const XMLCh * const ) const;


    /**
      * Union datatype handling
      *
      */

    virtual DatatypeValidator * getValidatingMemberType() const;
    virtual void setValidatingMemberType(DatatypeValidator * validatingMemberType) ;

    /**
      * QName datatype handling
      * Create default implementations for source code compatibility
      */
    virtual bool isPrefixUnknown(XMLCh* prefix);
    virtual void setElemStack(ElemStack* elemStack);
    virtual const XMLCh* getURIForPrefix(XMLCh* prefix);
    virtual void setScanner(XMLScanner* scanner);   
    virtual void setNamespaceScope(NamespaceScope* nsStack);


    //@}
  
private:
    // -----------------------------------------------------------------------
    /** name  Unimplemented copy constructor and operator= */
    // -----------------------------------------------------------------------
    //@{
    ValidationContextImpl(const ValidationContextImpl& );
    ValidationContextImpl& operator=(const ValidationContextImpl& );
    //@}

    // -----------------------------------------------------------------------
    //  Data members
    //
    //  fIDRefList:  owned/adopted
    //      This is a list of XMLRefInfo objects. This member lets us do all
    //      needed ID-IDREF balancing checks.
    //
    //  fEntityDeclPool: referenced only
    //      This is a pool of EntityDecl objects, which contains all of the
    //      general entities that are declared in the DTD subsets, plus the
    //      default entities (such as &gt; &lt; ...) defined by the XML Standard.
    //
    //  fToAddToList
    //  fValidatingMemberType
    //      The member type in a union that actually
    //      validated some text.  Note that the validationContext does not
    //      own this object, and the value of getValidatingMemberType
    //      will not be accurate unless the type of the most recently-validated
    //      element/attribute is in fact a union datatype.
    //  fElemStack
    //      Need access to elemstack to look up URI's that are inscope (while validating an XML).
    //  fNamespaceScope
    //      Need access to namespace scope to look up URI's that are inscope (while loading a schema).
    // -----------------------------------------------------------------------

    RefHashTableOf<XMLRefInfo>*         fIdRefList;
    const NameIdPool<DTDEntityDecl>*    fEntityDeclPool;
    bool                                fToCheckIdRefList;
    DatatypeValidator *                 fValidatingMemberType;    
    ElemStack*                          fElemStack;
    XMLScanner*                         fScanner;
    NamespaceScope*                     fNamespaceScope;

};



inline DatatypeValidator * ValidationContextImpl::getValidatingMemberType() const
{
    return fValidatingMemberType;
}

inline void ValidationContextImpl::setValidatingMemberType(DatatypeValidator * validatingMemberType) 
{
    fValidatingMemberType = validatingMemberType;
}

inline void ValidationContextImpl::setElemStack(ElemStack* elemStack) {
    fElemStack = elemStack;
}

inline void ValidationContextImpl::setScanner(XMLScanner* scanner) {
    fScanner = scanner;
}

inline void ValidationContextImpl::setNamespaceScope(NamespaceScope* nsStack) {
    fNamespaceScope = nsStack;
}

XERCES_CPP_NAMESPACE_END

#endif

