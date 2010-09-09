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
 * $Id: DOMDocumentImpl.hpp 679340 2008-07-24 10:28:29Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMDOCUMENTIMPL_HPP)
#define XERCESC_INCLUDE_GUARD_DOMDOCUMENTIMPL_HPP

//
//  This file is part of the internal implementation of the C++ XML DOM.
//  It should NOT be included or used directly by application programs.
//
//  Applications should include the file <xercesc/dom/DOM.hpp> for the entire
//  DOM API, or xercesc/dom/DOM*.hpp for individual DOM classes, where the class
//  name is substituded for the *.
//

#include <xercesc/util/RefArrayOf.hpp>
#include <xercesc/util/RefStackOf.hpp>
#include <xercesc/util/RefHash2KeysTableOf.hpp>
#include <xercesc/util/StringPool.hpp>
#include <xercesc/util/KeyRefPair.hpp>
#include <xercesc/util/XMLChar.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMUserDataHandler.hpp>
#include <xercesc/dom/DOMMemoryManager.hpp>
#include "DOMNodeImpl.hpp"
#include "DOMStringPool.hpp"
#include "DOMParentNode.hpp"
#include "DOMDeepNodeListPool.hpp"

XERCES_CPP_NAMESPACE_BEGIN


class DOMAttrImpl;
class DOMCDATASectionImpl;
class DOMCommentImpl;
class DOMConfiguration;
class DOMDeepNodeListImpl;
class DOMDocumentFragmentImpl;
class DOMDocumentTypeImpl;
class DOMElementImpl;
class DOMEntityImpl;
class DOMEntityReferenceImpl;
class DOMNotationImpl;
class DOMProcessingInstructionImpl;
class DOMTextImpl;
class DOMNodeIteratorImpl;
class DOMNormalizer;
class DOMTreeWalkerImpl;
class DOMNodeFilter;
class DOMNodeFilterImpl;
class DOMImplementation;
class DOMNodeIDMap;
class DOMRangeImpl;
class DOMBuffer;
class MemoryManager;
class XPathNSResolver;
class XPathExpression;

typedef RefVectorOf<DOMRangeImpl>        Ranges;
typedef RefVectorOf<DOMNodeIteratorImpl>     NodeIterators;
typedef KeyRefPair<void, DOMUserDataHandler> DOMUserDataRecord;
typedef RefStackOf<DOMNode>               DOMNodePtr;

class CDOM_EXPORT DOMDocumentImpl: public XMemory, public DOMMemoryManager, public DOMDocument {
public:
    // -----------------------------------------------------------------------
    //  data
    // -----------------------------------------------------------------------
    DOMNodeImpl           fNode;           // Implements common node functionality.
    DOMParentNode         fParent;         // Implements common parent node functionality
    DOMNodeIDMap*         fNodeIDMap;     // for use by GetElementsById().

public:
    DOMDocumentImpl(DOMImplementation* domImpl, MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    DOMDocumentImpl(const XMLCh*     namespaceURI,     //DOM Level 2
                    const XMLCh*     qualifiedName,
                    DOMDocumentType* doctype,
                    DOMImplementation* domImpl,
                    MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    virtual ~DOMDocumentImpl();

    void                         setDocumentType(DOMDocumentType *doctype);

public:
    // Add all functions that are pure virtual in DOMNODE
    DOMNODE_FUNCTIONS;

public:
    // Add all functions that are pure virtual in DOMDocument
    virtual DOMAttr*             createAttribute(const XMLCh *name);
    virtual DOMCDATASection*     createCDATASection(const XMLCh *data);
    virtual DOMComment*          createComment(const XMLCh *data);
    virtual DOMDocumentFragment* createDocumentFragment();
    virtual DOMDocumentType*     createDocumentType(const XMLCh *name);
    virtual DOMDocumentType*     createDocumentType(const XMLCh *qName,
                                                    const XMLCh *publicId,
                                                    const XMLCh *systemId);
    virtual DOMElement*          createElement(const XMLCh * tagName);
    virtual DOMElement*          createElementNoCheck(const XMLCh *tagName);
    virtual DOMEntity*           createEntity(const XMLCh * name);
    virtual DOMEntityReference*  createEntityReference(const XMLCh * name);
    virtual DOMNotation*         createNotation(const XMLCh * name);
    virtual DOMProcessingInstruction* createProcessingInstruction(const XMLCh * target, const XMLCh * data);
    virtual DOMText*             createTextNode(const XMLCh * data);
    virtual DOMDocumentType*     getDoctype() const;
    virtual DOMElement*          getDocumentElement() const;
    virtual DOMNodeList*         getElementsByTagName(const XMLCh * tagname) const;
    virtual DOMImplementation*   getImplementation() const;
    bool                         isXMLName(const XMLCh * s);
    virtual DOMNodeIterator*     createNodeIterator(DOMNode *root,
                                                    DOMNodeFilter::ShowType whatToShow,
                                                    DOMNodeFilter* filter,
                                                    bool entityReferenceExpansion);
    virtual DOMTreeWalker*       createTreeWalker(DOMNode *root,
                                                  DOMNodeFilter::ShowType whatToShow,
                                                  DOMNodeFilter* filter,
                                                  bool entityReferenceExpansion);


    virtual DOMRange*            createRange();
    virtual Ranges*              getRanges() const;  //non-standard api
    virtual NodeIterators*       getNodeIterators() const;  //non-standard api
    virtual void                 removeRange(DOMRangeImpl* range); //non-standard api
    virtual void                 removeNodeIterator(DOMNodeIteratorImpl* nodeIterator); //non-standard api

    virtual DOMXPathExpression* createExpression(const XMLCh *expression,
                                                 const DOMXPathNSResolver *resolver);
    virtual DOMXPathNSResolver* createNSResolver(const DOMNode *nodeResolver);
    virtual DOMXPathResult* evaluate(const XMLCh *expression,
                                     const DOMNode *contextNode,
                                     const DOMXPathNSResolver *resolver,
                                     DOMXPathResult::ResultType type,
                                     DOMXPathResult* result);


    // Extension to be called by the Parser
    DOMEntityReference*  createEntityReferenceByParser(const XMLCh * name);

    // Add all functions that are pure virtual in DOMMemoryManager
    virtual XMLSize_t getMemoryAllocationBlockSize() const;
    virtual void setMemoryAllocationBlockSize(XMLSize_t size);
    virtual void* allocate(XMLSize_t amount);
    virtual void* allocate(XMLSize_t amount, DOMMemoryManager::NodeObjectType type);
    virtual void release(DOMNode* object, DOMMemoryManager::NodeObjectType type);
    virtual XMLCh* cloneString(const XMLCh *src);

    //
    // Functions to keep track of document mutations, so that node list chached
    //   information can be invalidated.  One global changes counter per document.
    //
    virtual void                 changed();
    virtual int                  changes() const;

    /**
     * Sets whether the DOM implementation performs error checking
     * upon operations. Turning off error checking only affects
     * the following DOM checks:
     * <ul>
     * <li>Checking strings to make sure that all characters are
     *     legal XML characters
     * <li>Hierarchy checking such as allowed children, checks for
     *     cycles, etc.
     * </ul>
     * <p>
     * Turning off error checking does <em>not</em> turn off the
     * following checks:
     * <ul>
     * <li>Read only checks
     * <li>Checks related to DOM events
     * </ul>
     */
    inline void setErrorChecking(bool check) {
        errorChecking = check;
    }

    /**
     * Returns true if the DOM implementation performs error checking.
     */
    inline bool getErrorChecking() const {
        return errorChecking;
    }

    //Introduced in DOM Level 2
    virtual DOMNode*             importNode(const DOMNode *source, bool deep);
    virtual DOMElement*          createElementNS(const XMLCh *namespaceURI,
                                                 const XMLCh *qualifiedName);
    virtual DOMElement*          createElementNS(const XMLCh *namespaceURI,
                                                 const XMLCh *qualifiedName,
                                                 const XMLFileLoc lineNo,
                                                 const XMLFileLoc columnNo);
    virtual DOMAttr*             createAttributeNS(const XMLCh *namespaceURI,
                                                   const XMLCh *qualifiedName);
    virtual DOMNodeList*         getElementsByTagNameNS(const XMLCh *namespaceURI,
                                                        const XMLCh *localName) const;
    virtual DOMElement*          getElementById(const XMLCh *elementId) const;

    //Introduced in DOM Level 3
    virtual const XMLCh*         getInputEncoding() const;
    virtual const XMLCh*         getXmlEncoding() const;
    virtual bool                 getXmlStandalone() const;
    virtual void                 setXmlStandalone(bool standalone);
    virtual const XMLCh*         getXmlVersion() const;
    virtual void                 setXmlVersion(const XMLCh* version);
    virtual const XMLCh*         getDocumentURI() const;
    virtual void                 setDocumentURI(const XMLCh* documentURI);
    virtual bool                 getStrictErrorChecking() const;
    virtual void                 setStrictErrorChecking(bool strictErrorChecking);
    virtual DOMNode*             adoptNode(DOMNode* source);
    virtual void                 normalizeDocument();
    virtual DOMConfiguration*    getDOMConfig() const;

    void                         setInputEncoding(const XMLCh* actualEncoding);
    void                         setXmlEncoding(const XMLCh* encoding);
    // helper functions to prevent storing userdata pointers on every node.
    void*                        setUserData(DOMNodeImpl* n,
                                            const XMLCh* key,
                                            void* data,
                                            DOMUserDataHandler* handler);
    void*                        getUserData(const DOMNodeImpl* n,
                                             const XMLCh* key) const;
    void                         callUserDataHandlers(const DOMNodeImpl* n,
                                                      DOMUserDataHandler::DOMOperationType operation,
                                                      const DOMNode* src,
                                                      DOMNode* dst) const;
    void                         transferUserData(DOMNodeImpl* n1, DOMNodeImpl* n2);

    DOMNode*                     renameNode(DOMNode* n,
                                            const XMLCh* namespaceURI,
                                            const XMLCh* name);

    //Return the index > 0 of ':' in the given qualified name qName="prefix:localName".
    //Return 0 if there is no ':', or -1 if qName is malformed such as ":abcd".
    static  int                  indexofQualifiedName(const XMLCh * qName);
    static  bool                 isKidOK(DOMNode *parent, DOMNode *child);

    inline DOMNodeIDMap*         getNodeIDMap() {return fNodeIDMap;};


    //
    // Memory Management Functions.  All memory is allocated by and owned by
    //                               a document, and is not recovered until the
    //                               document itself is deleted.
    //
    const XMLCh*                 getPooledString(const XMLCh*);
    const XMLCh*                 getPooledNString(const XMLCh*, XMLSize_t);
    void                         deleteHeap();
    void                         releaseDocNotifyUserData(DOMNode* object);
    void                         releaseBuffer(DOMBuffer* buffer);
    DOMBuffer*                   popBuffer(XMLSize_t nMinSize);
    MemoryManager*               getMemoryManager() const;

    // Factory methods for getting/creating node lists.
    // Because nothing is ever deleted, the implementation caches and recycles
    //  previously used instances of DOMDeepNodeList
    //
    DOMNodeList*                 getDeepNodeList(const DOMNode *rootNode, const XMLCh *tagName);
    DOMNodeList*                 getDeepNodeList(const DOMNode *rootNode,    //DOM Level 2
                                                 const XMLCh *namespaceURI,
                                                 const XMLCh *localName);

protected:
    //Internal helper functions
    virtual DOMNode*             importNode(const DOMNode *source, bool deep, bool cloningNode);

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    DOMDocumentImpl(const DOMDocumentImpl &);
    DOMDocumentImpl & operator = (const DOMDocumentImpl &);

protected:
    // -----------------------------------------------------------------------
    //  data
    // -----------------------------------------------------------------------
    // New data introduced in DOM Level 3
    const XMLCh*          fInputEncoding;
    const XMLCh*          fXmlEncoding;
    bool                  fXmlStandalone;
    const XMLCh*          fXmlVersion;
    const XMLCh*          fDocumentURI;
    DOMConfiguration*     fDOMConfiguration;

    XMLStringPool         fUserDataTableKeys;
    RefHash2KeysTableOf<DOMUserDataRecord, PtrHasher>* fUserDataTable;


    // Per-Document heap Variables.
    //   The heap consists of one or more biggish blocks which are
    //   sub-allocated for individual allocations of nodes, strings, etc.
    //   The big blocks form a linked list, allowing them to be located for deletion.
    //
    //   There is no provision for deleting suballocated blocks, other than
    //     deleting the entire heap when the document is deleted.
    //
    //   There is no header on individual sub-allocated blocks.
    //   The header on big blocks consists only of a single back pointer to
    //    the previously allocated big block (our linked list of big blocks)
    //
    //
    //   revisit - this heap should be encapsulated into its own
    //                  class, rather than hanging naked on Document.
    //
    void*                 fCurrentBlock;
    char*                 fFreePtr;
    XMLSize_t             fFreeBytesRemaining,
                          fHeapAllocSize;

    // To recycle the DOMNode pointer
    RefArrayOf<DOMNodePtr>* fRecycleNodePtr;

    // To recycle DOMBuffer pointer
    RefStackOf<DOMBuffer>* fRecycleBufferPtr;

    // Pool of DOMNodeList for getElementsByTagName
    DOMDeepNodeListPool<DOMDeepNodeListImpl>* fNodeListPool;

    // Other data
    DOMDocumentType*      fDocType;
    DOMElement*           fDocElement;

    DOMStringPoolEntry**  fNameTable;
    XMLSize_t             fNameTableSize;

    DOMNormalizer*        fNormalizer;
    Ranges*               fRanges;
    NodeIterators*        fNodeIterators;
    MemoryManager*        fMemoryManager;   // configurable memory manager
    DOMImplementation*    fDOMImplementation;

    int                   fChanges;
    bool                  errorChecking;    // Bypass error checking.

};

inline MemoryManager* DOMDocumentImpl::getMemoryManager() const
{
    return fMemoryManager;
}

inline const XMLCh*  DOMDocumentImpl::getPooledString(const XMLCh *in)
{
  if (in == 0)
    return 0;

  DOMStringPoolEntry    **pspe;
  DOMStringPoolEntry    *spe;

  XMLSize_t inHash = XMLString::hash(in, fNameTableSize);
  pspe = &fNameTable[inHash];
  while (*pspe != 0)
  {
    if (XMLString::equals((*pspe)->fString, in))
      return (*pspe)->fString;
    pspe = &((*pspe)->fNext);
  }

  // This string hasn't been seen before.  Add it to the pool.
  //

  // Compute size to allocate.  Note that there's 1 char of string
  // declared in the struct, so we don't need to add one again to
  // account for the trailing null.
  //
  XMLSize_t sizeToAllocate = sizeof(DOMStringPoolEntry) + XMLString::stringLen(in)*sizeof(XMLCh);
  *pspe = spe = (DOMStringPoolEntry *)allocate(sizeToAllocate);
  spe->fNext = 0;
  XMLString::copyString((XMLCh*)spe->fString, in);

  return spe->fString;
}

inline const XMLCh* DOMDocumentImpl::
getPooledNString(const XMLCh *in, XMLSize_t n)
{
  if (in == 0)
    return 0;

  DOMStringPoolEntry    **pspe;
  DOMStringPoolEntry    *spe;

  XMLSize_t inHash = XMLString::hashN(in, n, fNameTableSize);
  pspe = &fNameTable[inHash];
  while (*pspe != 0)
  {
    if (XMLString::equalsN((*pspe)->fString, in, n))
      return (*pspe)->fString;
    pspe = &((*pspe)->fNext);
  }

  // This string hasn't been seen before.  Add it to the pool.
  //

  // Compute size to allocate.  Note that there's 1 char of string
  // declared in the struct, so we don't need to add one again to
  // account for the trailing null.
  //
  XMLSize_t sizeToAllocate = sizeof(DOMStringPoolEntry) + n*sizeof(XMLCh);
  *pspe = spe = (DOMStringPoolEntry *)allocate(sizeToAllocate);
  spe->fNext = 0;
  XMLString::copyNString((XMLCh*)spe->fString, in, n);

  return spe->fString;
}

inline int DOMDocumentImpl::indexofQualifiedName(const XMLCh* name)
{
  int i = 0;
  int colon = -1;
  int colon_count = 0;
  for (; *name != 0; ++i, ++name)
  {
    if (*name == chColon)
    {
      ++colon_count;
      colon = i;
    }
  }

  if (i == 0 || colon == 0 || colon == (i - 1) || colon_count > 1)
    return -1;

  return colon != -1 ? colon : 0;
}

XERCES_CPP_NAMESPACE_END

// ---------------------------------------------------------------------------
//
//  Operator new.  Global overloaded version, lets any object be allocated on
//                 the heap owned by a document.
//
// ---------------------------------------------------------------------------
inline void * operator new(size_t amt, XERCES_CPP_NAMESPACE_QUALIFIER DOMDocumentImpl *doc, XERCES_CPP_NAMESPACE_QUALIFIER DOMMemoryManager::NodeObjectType type)
{
    void *p = doc->allocate(amt, type);
    return p;
}

inline void * operator new(size_t amt, XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *doc, XERCES_CPP_NAMESPACE_QUALIFIER DOMMemoryManager::NodeObjectType type)
{
    XERCES_CPP_NAMESPACE_QUALIFIER DOMMemoryManager* mgr=(XERCES_CPP_NAMESPACE_QUALIFIER DOMMemoryManager*)doc->getFeature(XERCES_CPP_NAMESPACE_QUALIFIER XMLUni::fgXercescInterfaceDOMMemoryManager,0);
    void* p=0;
    if(mgr)
        p = mgr->allocate(amt, type);
    return p;
}

inline void * operator new(size_t amt, XERCES_CPP_NAMESPACE_QUALIFIER DOMDocumentImpl *doc)
{
    void* p = doc->allocate(amt);
    return p;
}

inline void * operator new(size_t amt, XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *doc)
{
    XERCES_CPP_NAMESPACE_QUALIFIER DOMMemoryManager* mgr=(XERCES_CPP_NAMESPACE_QUALIFIER DOMMemoryManager*)doc->getFeature(XERCES_CPP_NAMESPACE_QUALIFIER XMLUni::fgXercescInterfaceDOMMemoryManager,0);
    void* p=0;
    if(mgr)
        p = mgr->allocate(amt);
    return p;
}

// ---------------------------------------------------------------------------
//  For DOM:
//  Bypass compiler warning:
//    no matching operator delete found; memory will not be freed if initialization throws an exception
// ---------------------------------------------------------------------------
#if !defined(XERCES_NO_MATCHING_DELETE_OPERATOR)
inline void operator delete(void* /*ptr*/, XERCES_CPP_NAMESPACE_QUALIFIER DOMDocumentImpl * /*doc*/, XERCES_CPP_NAMESPACE_QUALIFIER DOMMemoryManager::NodeObjectType /*type*/)
{
    return;
}
inline void operator delete(void* /*ptr*/, XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument * /*doc*/, XERCES_CPP_NAMESPACE_QUALIFIER DOMMemoryManager::NodeObjectType /*type*/)
{
    return;
}

inline void operator delete(void* /*ptr*/, XERCES_CPP_NAMESPACE_QUALIFIER DOMDocumentImpl * /*doc*/)
{
    return;
}
inline void operator delete(void* /*ptr*/, XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument * /*doc*/)
{
    return;
}
#endif

#endif
