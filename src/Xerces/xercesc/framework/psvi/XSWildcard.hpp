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
 * $Id: XSWildcard.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XSWILDCARD_HPP)
#define XERCESC_INCLUDE_GUARD_XSWILDCARD_HPP

#include <xercesc/framework/psvi/XSObject.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/**
 * This class describes all properties of a Schema Wildcard
 * component.
 * This is *always* owned by the validator /parser object from which
 * it is obtained.  
 */

// forward declarations
class XSAnnotation;
class SchemaAttDef;
class ContentSpecNode;

class XMLPARSER_EXPORT XSWildcard : public XSObject
{
public:

    // Namespace Constraint
    enum NAMESPACE_CONSTRAINT {
	    /**
	     * Namespace Constraint: any namespace is allowed.
	     */
	    NSCONSTRAINT_ANY          = 1,
	    /**
	     * Namespace Constraint: namespaces in the list are not allowed.
	     */
	    NSCONSTRAINT_NOT          = 2,
	    /**
	     * Namespace Constraint: namespaces in the list are allowed.
	     */
	    NSCONSTRAINT_DERIVATION_LIST         = 3
    };

    // Process contents
    enum PROCESS_CONTENTS {
	    /**
	     * There must be a top-level declaration for the item available, or the 
	     * item must have an xsi:type, and the item must be valid as appropriate.
	     */
	    PC_STRICT                 = 1,
	    /**
	     * No constraints at all: the item must simply be well-formed XML. 
	     */
	    PC_SKIP                   = 2,
	    /**
	     * If the item, or any items among its [children] is an element 
	     * information item, has a uniquely determined declaration available, it 
	     * must be valid with respect to that definition, that is, validate 
	     * where you can, don't worry when you can't.
	     */
	    PC_LAX                    = 3
    };

    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    /** @name Constructors */
    //@{

    /**
      * The default constructor 
      *
      * @param  attWildCard
      * @param  annot
      * @param  xsModel
      * @param  manager     The configurable memory manager
      */
    XSWildcard
    (
        SchemaAttDef* const attWildCard
        , XSAnnotation* const annot
        , XSModel* const xsModel
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );
    
    XSWildcard
    (
        const ContentSpecNode* const elmWildCard
        , XSAnnotation* const annot
        , XSModel* const xsModel
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    //@}

    /** @name Destructor */
    //@{
    ~XSWildcard();
    //@}

    //---------------------
    /** @name XSWildcard methods */

    //@{

    /**
     * Namespace constraint: A constraint type: any, not, list. 
     */
    NAMESPACE_CONSTRAINT getConstraintType() const;

    /**
     * Namespace constraint. For <code>constraintType</code> 
     * <code>NSCONSTRAINT_DERIVATION_LIST</code>, the list contains allowed namespaces. 
     * For <code>constraintType</code> <code>NSCONSTRAINT_NOT</code>, the 
     * list contains disallowed namespaces. 
     */
    StringList *getNsConstraintList();

    /**
     * [process contents]: one of skip, lax or strict. Valid constants values 
     * are: <code>PC_SKIP, PC_LAX, PC_STRICT</code>. 
     */
    PROCESS_CONTENTS getProcessContents() const;

    /**
     * Optional. An [annotation]. 
     */
    XSAnnotation *getAnnotation() const;

    //@}

    //----------------------------------
    /** methods needed by implementation */

    //@{

    //@}
private:

    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XSWildcard(const XSWildcard&);
    XSWildcard & operator=(const XSWildcard &);

   /**
     * Build namespace list
     */
   void buildNamespaceList(const ContentSpecNode* const rootNode);

protected:

    // -----------------------------------------------------------------------
    //  data members
    // -----------------------------------------------------------------------
    NAMESPACE_CONSTRAINT fConstraintType;
    PROCESS_CONTENTS     fProcessContents;
    StringList*          fNsConstraintList;
    XSAnnotation*        fAnnotation;
};

inline XSAnnotation *XSWildcard::getAnnotation() const
{
    return fAnnotation;
}

inline XSWildcard::PROCESS_CONTENTS XSWildcard::getProcessContents() const
{
    return fProcessContents;
}

inline StringList* XSWildcard::getNsConstraintList()
{
    return fNsConstraintList;
}

inline XSWildcard::NAMESPACE_CONSTRAINT XSWildcard::getConstraintType() const
{
    return fConstraintType;
}


XERCES_CPP_NAMESPACE_END

#endif
