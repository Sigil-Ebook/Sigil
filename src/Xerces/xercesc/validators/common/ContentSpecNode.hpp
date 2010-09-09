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
 * $Id: ContentSpecNode.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_CONTENTSPECNODE_HPP)
#define XERCESC_INCLUDE_GUARD_CONTENTSPECNODE_HPP

#include <xercesc/framework/XMLElementDecl.hpp>
#include <xercesc/framework/MemoryManager.hpp>

#include <xercesc/internal/XSerializable.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XMLBuffer;
class Grammar;


class XMLUTIL_EXPORT ContentSpecNode : public XSerializable, public XMemory
{
public :
    // -----------------------------------------------------------------------
    //  Class specific types
    // -----------------------------------------------------------------------
    enum NodeTypes
    {
        Leaf = 0
        , ZeroOrOne
        , ZeroOrMore
        , OneOrMore
        , Choice
        , Sequence
        , Any
        , Any_Other
        , Any_NS = 8
        , All = 9
        , Loop = 10
        , Any_NS_Choice = 20            // 16 + 4 (Choice)
        , ModelGroupSequence = 21       // 16 + 5 (Sequence)
        , Any_Lax = 22                  // 16 + 6 (Any)
        , Any_Other_Lax = 23            // 16 + 7 (Any_Other)
        , Any_NS_Lax = 24               // 16 + 8 (Any_NS)
        , ModelGroupChoice = 36         // 32 + 4 (Choice)
        , Any_Skip = 38                 // 32 + 6 (Any)
        , Any_Other_Skip = 39           // 32 + 7 (Any_Other)
        , Any_NS_Skip = 40              // 32 + 8 (Any_NS)

        , UnknownType = -1
    };


    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    ContentSpecNode(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    ContentSpecNode
    (
        QName* const toAdopt
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );
    ContentSpecNode
    (
        XMLElementDecl* const elemDecl
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );
    ContentSpecNode
    (
        QName* const toAdopt
        , const bool copyQName
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );
    ContentSpecNode
    (
        const   NodeTypes               type
        ,       ContentSpecNode* const  firstToAdopt
        ,       ContentSpecNode* const  secondToAdopt
        , const bool                    adoptFirst = true
        , const bool                    adoptSecond = true
        ,       MemoryManager* const    manager = XMLPlatformUtils::fgMemoryManager
    );
    ContentSpecNode(const ContentSpecNode&);
	~ContentSpecNode();

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    QName* getElement();
    const QName* getElement() const;
    XMLElementDecl* getElementDecl();
    const XMLElementDecl* getElementDecl() const;
    ContentSpecNode* getFirst();
    const ContentSpecNode* getFirst() const;
    ContentSpecNode* getSecond();
    const ContentSpecNode* getSecond() const;
    NodeTypes getType() const;
    ContentSpecNode* orphanFirst();
    ContentSpecNode* orphanSecond();
    int getMinOccurs() const;
    int getMaxOccurs() const;
    bool isFirstAdopted() const;
    bool isSecondAdopted() const;


    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------
    void setElement(QName* const toAdopt);
    void setFirst(ContentSpecNode* const toAdopt);
    void setSecond(ContentSpecNode* const toAdopt);
    void setType(const NodeTypes type);
    void setMinOccurs(int min);
    void setMaxOccurs(int max);
    void setAdoptFirst(bool adoptFirst);
    void setAdoptSecond(bool adoptSecond);


    // -----------------------------------------------------------------------
    //  Miscellaneous
    // -----------------------------------------------------------------------
    void formatSpec (XMLBuffer&      bufToFill)   const;
    bool hasAllContent();
    int  getMinTotalRange() const;
    int  getMaxTotalRange() const;

    /***
     * Support for Serialization/De-serialization
     ***/
    DECL_XSERIALIZABLE(ContentSpecNode)

private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    ContentSpecNode& operator=(const ContentSpecNode&);


    // -----------------------------------------------------------------------
    //  Private Data Members
    //
    //  fElement
    //      If the type is Leaf/Any*, then this is the qName of the element. If the URI
    //      is fgPCDataElemId, then its a PCData node.  Else, it is zero.
    //
    //  fFirst
    //  fSecond
    //      The optional first and second nodes. The fType field indicates
    //      which of these are valid. The validity constraints are:
    //
    //          Leaf = Neither valid
    //          ZeroOrOne, ZeroOrMore = First
    //          Choice, Sequence, All = First and Second
    //          Any* = Neither valid
    //
    //  fType
    //      The type of node. This controls how many of the child node fields
    //      are used.
    //
    //  fAdoptFirst
    //      Indicate if this ContentSpecNode adopts the fFirst, and is responsible
    //      for deleting it.
    //
    //  fAdoptSecond
    //      Indicate if this ContentSpecNode adopts the fSecond, and is responsible
    //      for deleting it.
    //
    //  fMinOccurs
    //      Indicate the minimum times that this node can occur
    //
    //  fMaxOccurs
    //      Indicate the maximum times that this node can occur
    //      -1 (Unbounded), default (1)
    // -----------------------------------------------------------------------
    MemoryManager*      fMemoryManager;
    QName*              fElement;
    XMLElementDecl*     fElementDecl;
    ContentSpecNode*    fFirst;
    ContentSpecNode*    fSecond;
    NodeTypes           fType;
    bool                fAdoptFirst;
    bool                fAdoptSecond;
    int                 fMinOccurs;
    int                 fMaxOccurs;
};

// ---------------------------------------------------------------------------
//  ContentSpecNode: Constructors and Destructor
// ---------------------------------------------------------------------------
inline ContentSpecNode::ContentSpecNode(MemoryManager* const manager) :

    fMemoryManager(manager)
    , fElement(0)
    , fElementDecl(0)
    , fFirst(0)
    , fSecond(0)
    , fType(ContentSpecNode::Leaf)
    , fAdoptFirst(true)
    , fAdoptSecond(true)
    , fMinOccurs(1)
    , fMaxOccurs(1)
{
}

inline
ContentSpecNode::ContentSpecNode(QName* const element,
                                 MemoryManager* const manager) :

    fMemoryManager(manager)
    , fElement(0)
    , fElementDecl(0)
    , fFirst(0)
    , fSecond(0)
    , fType(ContentSpecNode::Leaf)
    , fAdoptFirst(true)
    , fAdoptSecond(true)
    , fMinOccurs(1)
    , fMaxOccurs(1)
{
    if (element)
        fElement = new (fMemoryManager) QName(*element);
}

inline
ContentSpecNode::ContentSpecNode(XMLElementDecl* const elemDecl,
                                 MemoryManager* const manager) :

    fMemoryManager(manager)
    , fElement(0)
    , fElementDecl(elemDecl)
    , fFirst(0)
    , fSecond(0)
    , fType(ContentSpecNode::Leaf)
    , fAdoptFirst(true)
    , fAdoptSecond(true)
    , fMinOccurs(1)
    , fMaxOccurs(1)
{
    if (elemDecl)
        fElement = new (manager) QName(*(elemDecl->getElementName()));
}

inline
ContentSpecNode::ContentSpecNode( QName* const element
                                , const bool copyQName
                                , MemoryManager* const manager) :

    fMemoryManager(manager)
    , fElement(0)
    , fElementDecl(0)
    , fFirst(0)
    , fSecond(0)
    , fType(ContentSpecNode::Leaf)
    , fAdoptFirst(true)
    , fAdoptSecond(true)
    , fMinOccurs(1)
    , fMaxOccurs(1)
{
    if (copyQName)
    {
        if (element)
            fElement = new (fMemoryManager) QName(*element);
    }
    else
    {
        fElement = element;
    }
}

inline
ContentSpecNode::ContentSpecNode(const  NodeTypes              type
                                ,       ContentSpecNode* const firstAdopt
                                ,       ContentSpecNode* const secondAdopt
                                , const bool                   adoptFirst
                                , const bool                   adoptSecond
                                ,       MemoryManager* const   manager) :

    fMemoryManager(manager)
    , fElement(0)
    , fElementDecl(0)
    , fFirst(firstAdopt)
    , fSecond(secondAdopt)
    , fType(type)
    , fAdoptFirst(adoptFirst)
    , fAdoptSecond(adoptSecond)
    , fMinOccurs(1)
    , fMaxOccurs(1)
{
}

inline ContentSpecNode::~ContentSpecNode()
{
    // Delete our children, which cause recursive cleanup
    if (fAdoptFirst) {
		delete fFirst;
    }

    if (fAdoptSecond) {
		delete fSecond;
    }

    delete fElement;
}

// ---------------------------------------------------------------------------
//  ContentSpecNode: Getter methods
// ---------------------------------------------------------------------------
inline QName* ContentSpecNode::getElement()
{
    return fElement;
}

inline const QName* ContentSpecNode::getElement() const
{
    return fElement;
}

inline XMLElementDecl* ContentSpecNode::getElementDecl()
{
    return fElementDecl;
}

inline const XMLElementDecl* ContentSpecNode::getElementDecl() const
{
    return fElementDecl;
}

inline ContentSpecNode* ContentSpecNode::getFirst()
{
    return fFirst;
}

inline const ContentSpecNode* ContentSpecNode::getFirst() const
{
    return fFirst;
}

inline ContentSpecNode* ContentSpecNode::getSecond()
{
    return fSecond;
}

inline const ContentSpecNode* ContentSpecNode::getSecond() const
{
    return fSecond;
}

inline ContentSpecNode::NodeTypes ContentSpecNode::getType() const
{
    return fType;
}

inline ContentSpecNode* ContentSpecNode::orphanFirst()
{
    ContentSpecNode* retNode = fFirst;
    fFirst = 0;
    return retNode;
}

inline ContentSpecNode* ContentSpecNode::orphanSecond()
{
    ContentSpecNode* retNode = fSecond;
    fSecond = 0;
    return retNode;
}

inline int ContentSpecNode::getMinOccurs() const
{
    return fMinOccurs;
}

inline int ContentSpecNode::getMaxOccurs() const
{
    return fMaxOccurs;
}

inline bool ContentSpecNode::isFirstAdopted() const
{
    return fAdoptFirst;
}

inline bool ContentSpecNode::isSecondAdopted() const
{
    return fAdoptSecond;
}


// ---------------------------------------------------------------------------
//  ContentSpecType: Setter methods
// ---------------------------------------------------------------------------
inline void ContentSpecNode::setElement(QName* const element)
{
    delete fElement;
    fElement = 0;
    if (element)
        fElement = new (fMemoryManager) QName(*element);
}

inline void ContentSpecNode::setFirst(ContentSpecNode* const toAdopt)
{
    if (fAdoptFirst)
        delete fFirst;
    fFirst = toAdopt;
}

inline void ContentSpecNode::setSecond(ContentSpecNode* const toAdopt)
{
    if (fAdoptSecond)
        delete fSecond;
    fSecond = toAdopt;
}

inline void ContentSpecNode::setType(const NodeTypes type)
{
    fType = type;
}

inline void ContentSpecNode::setMinOccurs(int min)
{
    fMinOccurs = min;
}

inline void ContentSpecNode::setMaxOccurs(int max)
{
    fMaxOccurs = max;
}

inline void ContentSpecNode::setAdoptFirst(bool newState)
{
    fAdoptFirst = newState;
}

inline void ContentSpecNode::setAdoptSecond(bool newState)
{
    fAdoptSecond = newState;
}

// ---------------------------------------------------------------------------
//  ContentSpecNode: Miscellaneous
// ---------------------------------------------------------------------------
inline bool ContentSpecNode::hasAllContent() {

    if (fType == ContentSpecNode::ZeroOrOne) {
        return (fFirst->getType() == ContentSpecNode::All);
    }

    return (fType == ContentSpecNode::All);
}

XERCES_CPP_NAMESPACE_END

#endif
