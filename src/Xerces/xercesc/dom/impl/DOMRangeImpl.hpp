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
 * $Id: DOMRangeImpl.hpp 641193 2008-03-26 08:06:57Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMRANGEIMPL_HPP)
#define XERCESC_INCLUDE_GUARD_DOMRANGEIMPL_HPP

//
//  This file is part of the internal implementation of the C++ XML DOM.
//  It should NOT be included or used directly by application programs.
//
//  Applications should include the file <xercesc/dom/DOM.hpp> for the entire
//  DOM API, or xercesc/dom/DOM*.hpp for individual DOM classes, where the class
//  name is substituded for the *.
//

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/dom/DOMRange.hpp>
#include <xercesc/util/PlatformUtils.hpp>

XERCES_CPP_NAMESPACE_BEGIN



class       DOMNode;
class       DOMDocumentFragment;
class       DOMDocument;
class       DOMText;
class       MemoryManager;

class CDOM_EXPORT DOMRangeImpl: public DOMRange {
protected:
    enum TraversalType {
        EXTRACT_CONTENTS = 1,
        CLONE_CONTENTS   = 2,
        DELETE_CONTENTS  = 3
    };

    enum TraversePoint {
        BEFORE  = -1,
        START   = 0,
        AFTER   = 1
    };

    //private data

    DOMNode*     fStartContainer;
    XMLSize_t    fStartOffset;
    DOMNode*     fEndContainer;
    XMLSize_t    fEndOffset;
    bool         fCollapsed;
    DOMDocument* fDocument;
    bool         fDetached;

    DOMNode*     fRemoveChild;
    MemoryManager* fMemoryManager;

public:
    //c'tor
    DOMRangeImpl(DOMDocument* doc, MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    DOMRangeImpl(const DOMRangeImpl& other);

    //d'tor
    ~DOMRangeImpl();

    //getter functions
    virtual DOMNode* getStartContainer() const;
    virtual XMLSize_t getStartOffset() const;
    virtual DOMNode* getEndContainer() const;
    virtual XMLSize_t getEndOffset() const;
    virtual bool getCollapsed() const;
    virtual const DOMNode* getCommonAncestorContainer() const;

    //setter functions
    virtual void setStart(const DOMNode *parent, XMLSize_t offset);
    virtual void setEnd(const DOMNode *parent, XMLSize_t offset);

    virtual void setStartBefore(const DOMNode *refNode);
    virtual void setStartAfter(const DOMNode *refNode);
    virtual void setEndBefore(const DOMNode *refNode);
    virtual void setEndAfter(const DOMNode *refNode);

    //misc functions
    virtual void collapse(bool toStart);
    virtual void selectNode(const DOMNode *node);
    virtual void selectNodeContents(const DOMNode *node);

    //Functions related to comparing range Boundrary-Points
    virtual short compareBoundaryPoints(CompareHow how, const DOMRange* range) const;
    virtual void deleteContents();
    virtual DOMDocumentFragment* extractContents();
    virtual DOMDocumentFragment* cloneContents() const;
    virtual void insertNode(DOMNode* node);

    //Misc functions
    virtual void surroundContents(DOMNode *node);
    virtual DOMRange* cloneRange() const;
    virtual const XMLCh* toString() const;
    virtual void detach();
    virtual void release();

    //getter functions
    DOMDocument*         getDocument();

    // functions to inform all existing valid ranges about a change
    void updateSplitInfo(DOMNode* oldNode, DOMNode* startNode, XMLSize_t offset);
    void updateRangeForInsertedNode(DOMNode* node);
    void receiveReplacedText(DOMNode* node);
    void updateRangeForDeletedText(DOMNode* node, XMLSize_t offset, XMLSize_t count);
    void updateRangeForInsertedText(DOMNode* node, XMLSize_t offset, XMLSize_t count);
    void updateRangeForDeletedNode(DOMNode* node);

protected:
    //setter functions
    void        setStartContainer(const DOMNode* node);
    void        setStartOffset(XMLSize_t offset) ;
    void        setEndContainer(const DOMNode* node);
    void        setEndOffset(XMLSize_t offset) ;

    //misc functions
    void        validateNode(const DOMNode* node) const;
    bool        isValidAncestorType(const DOMNode* node) const;
    bool        hasLegalRootContainer(const DOMNode* node) const;
    bool        isLegalContainedNode(const DOMNode* node ) const;
    void        checkIndex(const DOMNode* node, XMLSize_t offset) const;
    static bool isAncestorOf(const DOMNode* a, const DOMNode* b);

    XMLSize_t   indexOf(const DOMNode* child, const DOMNode* parent) const;

    const DOMNode*       commonAncestorOf(const DOMNode* pointA, const DOMNode* pointB) const;
    DOMNode*             nextNode(const DOMNode* node, bool visitChildren) const;
    DOMDocumentFragment* traverseContents(TraversalType type);
    void                  checkReadOnly(DOMNode* start, DOMNode* end,
                                  XMLSize_t starOffset, XMLSize_t endOffset);
    void                  recurseTreeAndCheck(DOMNode* start, DOMNode* end);
    DOMNode*             removeChild(DOMNode* parent, DOMNode* child);

    DOMDocumentFragment* traverseSameContainer( int how );
    DOMDocumentFragment* traverseCommonStartContainer( DOMNode *endAncestor, int how );
    DOMDocumentFragment* traverseCommonEndContainer( DOMNode *startAncestor, int how );
    DOMDocumentFragment* traverseCommonAncestors( DOMNode *startAncestor, DOMNode *endAncestor, int how );
    DOMNode*    traverseRightBoundary( DOMNode *root, int how );
    DOMNode*    traverseLeftBoundary( DOMNode *root, int how );
    DOMNode*    traverseNode( DOMNode *n, bool isFullySelected, bool isLeft, int how );
    DOMNode*    traverseFullySelected( DOMNode *n, int how );
    DOMNode*    traversePartiallySelected( DOMNode *n, int how );
    DOMNode*    traverseTextNode( DOMNode *n, bool isLeft, int how );
    DOMNode*    getSelectedNode( DOMNode *container, int offset );

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    DOMRangeImpl & operator = (const DOMRangeImpl &);
};

XERCES_CPP_NAMESPACE_END

#endif
