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
 * $Id: CMLeaf.hpp 677396 2008-07-16 19:36:20Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_CMLEAF_HPP)
#define XERCESC_INCLUDE_GUARD_CMLEAF_HPP

#include <xercesc/validators/common/CMNode.hpp>


XERCES_CPP_NAMESPACE_BEGIN

//
//  This class represents a leaf in the content spec node tree of an
//  element's content model. It just has an element qname and a position value,
//  the latter of which is used during the building of a DFA.
//
class CMLeaf : public CMNode
{
public :
    // -----------------------------------------------------------------------
    //  Constructors
    // -----------------------------------------------------------------------
    CMLeaf
    (
          QName* const          element
        , unsigned int          position
        , unsigned int          maxStates
        , MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
    );
    CMLeaf
    (
          QName* const          element
        , unsigned int          position
        , bool                  adopt
        , unsigned int          maxStates
        , MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
    );
    ~CMLeaf();


    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    QName* getElement();
    const QName* getElement() const;
    unsigned int getPosition() const;

    virtual bool isRepeatableLeaf() const;

    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------
    void setPosition(const unsigned int newPosition);


    // -----------------------------------------------------------------------
    //  Implementation of public CMNode virtual interface
    // -----------------------------------------------------------------------
    virtual void orphanChild();

protected :
    // -----------------------------------------------------------------------
    //  Implementation of protected CMNode virtual interface
    // -----------------------------------------------------------------------
    void calcFirstPos(CMStateSet& toSet) const;
    void calcLastPos(CMStateSet& toSet) const;


private :
    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fElement
    //      This is the element that this leaf represents.
    //
    //  fPosition
    //      Part of the algorithm to convert a regex directly to a DFA
    //      numbers each leaf sequentially. If its -1, that means its an
    //      epsilon node. All others are non-epsilon positions.
    //
    //  fAdopt
    //      This node is responsible for the storage of the fElement QName.
    // -----------------------------------------------------------------------
    QName*          fElement;
    unsigned int    fPosition;
    bool            fAdopt;

    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    CMLeaf(const CMLeaf&);
    CMLeaf& operator=(const CMLeaf&);
};


// -----------------------------------------------------------------------
//  Constructors
// -----------------------------------------------------------------------
inline CMLeaf::CMLeaf( QName* const         element
                     , unsigned int         position
                     , unsigned int         maxStates
                     , MemoryManager* const manager) :
    CMNode(ContentSpecNode::Leaf, maxStates, manager)
    , fElement(0)
    , fPosition(position)
    , fAdopt(false)
{
    if (!element)
    {
        fElement = new (fMemoryManager) QName
        (
              XMLUni::fgZeroLenString
            , XMLUni::fgZeroLenString
            , XMLElementDecl::fgInvalidElemId
            , fMemoryManager
        );
        // We have to be responsible for this QName - override default fAdopt
        fAdopt = true;
    }
    else
    {
        fElement = element;
    }
    // Leaf nodes are never nullable unless its an epsilon node
    fIsNullable=(fPosition == epsilonNode);
}

inline CMLeaf::CMLeaf( QName* const         element
                     , unsigned int         position
                     , bool                 adopt
                     , unsigned int         maxStates
                     , MemoryManager* const manager) :
    CMNode(ContentSpecNode::Leaf, maxStates, manager)
    , fElement(0)
    , fPosition(position)
    , fAdopt(adopt)
{
    if (!element)
    {
        fElement = new (fMemoryManager) QName
        (
              XMLUni::fgZeroLenString
            , XMLUni::fgZeroLenString
            , XMLElementDecl::fgInvalidElemId
            , fMemoryManager
        );
        // We have to be responsible for this QName - override adopt parameter
        fAdopt = true;
    }
    else
    {
        fElement = element;
    }
    // Leaf nodes are never nullable unless its an epsilon node
    fIsNullable=(fPosition == epsilonNode);
}

inline CMLeaf::~CMLeaf()
{
    if (fAdopt)
        delete fElement;
}


// ---------------------------------------------------------------------------
//  Getter methods
// ---------------------------------------------------------------------------
inline QName* CMLeaf::getElement()
{
    return fElement;
}

inline const QName* CMLeaf::getElement() const
{
    return fElement;
}

inline unsigned int CMLeaf::getPosition() const
{
    return fPosition;
}

inline bool CMLeaf::isRepeatableLeaf() const
{
    return false;
}

// ---------------------------------------------------------------------------
//  Setter methods
// ---------------------------------------------------------------------------
inline void CMLeaf::setPosition(const unsigned int newPosition)
{
    fPosition = newPosition;
}


// ---------------------------------------------------------------------------
//  Implementation of public CMNode virtual interface
// ---------------------------------------------------------------------------
inline void CMLeaf::orphanChild()
{
}

// ---------------------------------------------------------------------------
//  Implementation of protected CMNode virtual interface
// ---------------------------------------------------------------------------
inline void CMLeaf::calcFirstPos(CMStateSet& toSet) const
{
    // If we are an epsilon node, then the first pos is an empty set
    if (isNullable())
    {
        toSet.zeroBits();
        return;
    }

    // Otherwise, its just the one bit of our position
    toSet.setBit(fPosition);
}

inline void CMLeaf::calcLastPos(CMStateSet& toSet) const
{
    // If we are an epsilon node, then the last pos is an empty set
    if (isNullable())
    {
        toSet.zeroBits();
        return;
    }

    // Otherwise, its just the one bit of our position
    toSet.setBit(fPosition);
}

XERCES_CPP_NAMESPACE_END

#endif
