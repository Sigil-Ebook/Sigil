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
 * $Id: CMAny.hpp 677396 2008-07-16 19:36:20Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_CMANY_HPP)
#define XERCESC_INCLUDE_GUARD_CMANY_HPP

#include <xercesc/validators/common/CMNode.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class CMStateSet;

class CMAny : public CMNode
{
public :
    // -----------------------------------------------------------------------
    //  Constructors
    // -----------------------------------------------------------------------
    CMAny
    (
        ContentSpecNode::NodeTypes      type
        , unsigned int                  URI
        , unsigned int                  position
        , unsigned int                  maxStates
        ,       MemoryManager* const    manager = XMLPlatformUtils::fgMemoryManager
    );
    ~CMAny();

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    unsigned int getURI() const;

    unsigned int getPosition() const;

    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------
    void setPosition(const unsigned int newPosition);

    // -----------------------------------------------------------------------
    //  Implementation of the public CMNode virtual interface
    // -----------------------------------------------------------------------
    virtual void orphanChild();


protected :
    // -----------------------------------------------------------------------
    //  Implementation of the protected CMNode virtual interface
    // -----------------------------------------------------------------------
    void calcFirstPos(CMStateSet& toSet) const;
    void calcLastPos(CMStateSet& toSet) const;

private :
    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fURI;
    //  URI of the any content model. This value is set if the type is
    //  of the following:
    //  XMLContentSpec.CONTENTSPECNODE_ANY,
    //  XMLContentSpec.CONTENTSPECNODE_ANY_OTHER.
    //
    //  fPosition
    //  Part of the algorithm to convert a regex directly to a DFA
    //  numbers each leaf sequentially. If its -1, that means its an
    //  epsilon node. Zero and greater are non-epsilon positions.
    // -----------------------------------------------------------------------
    unsigned int fURI;
    unsigned int fPosition;

    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    CMAny(const CMAny&);
    CMAny& operator=(const CMAny&);
};

XERCES_CPP_NAMESPACE_END

#endif
