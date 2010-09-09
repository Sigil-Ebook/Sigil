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
 * $Id: XSParticle.hpp 883665 2009-11-24 11:41:38Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XSPARTICLE_HPP)
#define XERCESC_INCLUDE_GUARD_XSPARTICLE_HPP

#include <xercesc/framework/psvi/XSObject.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/**
 * This class describes all properties of a Schema Particle
 * component.
 * This is *always* owned by the validator /parser object from which
 * it is obtained.
 */

// forward declarations
class XSElementDeclaration;
class XSModelGroup;
class XSWildcard;

class XMLPARSER_EXPORT XSParticle : public XSObject
{
public:

    // possible terms of this particle
    enum TERM_TYPE {
        /*
         * an empty particle
         */
        TERM_EMPTY          = 0,
        /*
         * the particle has element content
         */
        TERM_ELEMENT        = XSConstants::ELEMENT_DECLARATION,
        /*
         * the particle's content is a model group
         */
        TERM_MODELGROUP     = XSConstants::MODEL_GROUP_DEFINITION,
        /*
         * the particle's content is a wildcard
         */
        TERM_WILDCARD       = XSConstants::WILDCARD
    };

    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    /** @name Constructors */
    //@{

    /**
      * The default constructor
      *
      * @param  termType
      * @param  xsModel
      * @param  particleTerm
      * @param  minOccurs
      * @param  maxOccurs
      * @param  unbounded
      * @param  manager     The configurable memory manager
      */
    XSParticle
    (
        TERM_TYPE              termType
        , XSModel* const       xsModel
        , XSObject* const      particleTerm
        , XMLSize_t            minOccurs
        , XMLSize_t            maxOccurs
        , bool                 unbounded
        , MemoryManager* const manager
    );

    //@};

    /** @name Destructor */
    //@{
    ~XSParticle();
    //@}

    //---------------------
    /** @name XSParticle methods */
    //@{

    /**
     * [min occurs]: determines the minimum number of terms that can occur.
     */
    XMLSize_t getMinOccurs() const;

    /**
     * [max occurs] determines the maximum number of terms that can occur. To
     * query for value of unbounded use <code>maxOccursUnbounded</code>.
     */
    XMLSize_t getMaxOccurs() const;

    /**
     * [max occurs] whether the maxOccurs value is unbounded.
     */
    bool getMaxOccursUnbounded() const;

    /**
     * Returns the type of the [term]: one of
     * TERM_EMPTY, TERM_ELEMENT, TERM_MODELGROUP, or TERM_WILDCARD.
     */
    TERM_TYPE getTermType() const;

    /**
     * If this particle has an [element declaration] for its term,
     * this method returns that declaration; otherwise, it returns 0.
     * @returns The element declaration that is the [term] of this Particle
     * if and only if getTermType() == TERM_ELEMENT.
     */
    XSElementDeclaration *getElementTerm();

    /**
     * If this particle has a [model group] for its term,
     * this method returns that definition; otherwise, it returns 0.
     * @returns The model group that is the [term] of this Particle
     * if and only if getTermType() == TERM_MODELGROUP.
     */
    XSModelGroup *getModelGroupTerm();

    /**
     * If this particle has an [wildcard] for its term,
     * this method returns that declaration; otherwise, it returns 0.
     * @returns The wildcard declaration that is the [term] of this Particle
     * if and only if getTermType() == TERM_WILDCARD.
     */
    XSWildcard *getWildcardTerm();

    //@}

    //----------------------------------
    /** methods needed by implementation */
    //@{

    //@}
private:

    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XSParticle(const XSParticle&);
    XSParticle & operator=(const XSParticle &);

protected:

    // -----------------------------------------------------------------------
    //  data members
    // -----------------------------------------------------------------------
    TERM_TYPE fTermType;
    XMLSize_t fMinOccurs;
    XMLSize_t fMaxOccurs;
    bool      fUnbounded;
    XSObject* fTerm;
};

inline XMLSize_t XSParticle::getMinOccurs() const
{
    return fMinOccurs;
}

inline XMLSize_t XSParticle::getMaxOccurs() const
{
    return fMaxOccurs;
}

inline bool XSParticle::getMaxOccursUnbounded() const
{
    return fUnbounded;
}

inline XSParticle::TERM_TYPE XSParticle::getTermType() const
{
    return fTermType;
}

XERCES_CPP_NAMESPACE_END

#endif
