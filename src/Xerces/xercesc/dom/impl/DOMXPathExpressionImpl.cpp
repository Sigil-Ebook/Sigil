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

#include "DOMXPathExpressionImpl.hpp"
#include "DOMXPathResultImpl.hpp"
#include <xercesc/validators/schema/identity/XercesXPath.hpp>
#include <xercesc/validators/schema/identity/XPathMatcher.hpp>
#include <xercesc/validators/schema/identity/XPathException.hpp>
#include <xercesc/validators/schema/SchemaElementDecl.hpp>
#include <xercesc/util/StringPool.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/dom/DOMXPathException.hpp>
#include <xercesc/dom/DOM.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class WrapperForXPathNSResolver : public XercesNamespaceResolver
{
public:
    WrapperForXPathNSResolver(XMLStringPool* pool, const DOMXPathNSResolver *resolver, MemoryManager* const manager) :
      fStringPool(pool),
      fResolver(resolver),
      fMemoryManager(manager)
    {
    }

    virtual unsigned int getNamespaceForPrefix(const XMLCh* const prefix) const
    {
        if(fResolver==NULL)
            throw DOMException(DOMException::NAMESPACE_ERR, 0, fMemoryManager);
        const XMLCh* nsUri=fResolver->lookupNamespaceURI(prefix);
        if(nsUri==NULL)
            throw DOMException(DOMException::NAMESPACE_ERR, 0, fMemoryManager);
        return fStringPool->addOrFind(nsUri);
    }

protected:
    XMLStringPool*              fStringPool;
    const DOMXPathNSResolver *  fResolver;
    MemoryManager* const        fMemoryManager;
};


typedef JanitorMemFunCall<DOMXPathExpressionImpl>     CleanupType;

DOMXPathExpressionImpl::DOMXPathExpressionImpl(const XMLCh *expression, const DOMXPathNSResolver *resolver, MemoryManager* const manager) :
 fStringPool(NULL),
 fParsedExpression(NULL),
 fExpression(NULL),
 fMoveToRoot(false),
 fMemoryManager(manager)
{
    if(expression==NULL || *expression==0)
        throw DOMXPathException(DOMXPathException::INVALID_EXPRESSION_ERR, 0, fMemoryManager);

    CleanupType cleanup(this, &DOMXPathExpressionImpl::cleanUp);
    fStringPool = new (fMemoryManager) XMLStringPool(109, fMemoryManager);
    // XercesPath will complain if the expression starts with '/', add a "." in front of it and start from the document root
    if(*expression==chForwardSlash)
    {
        fExpression=(XMLCh*)fMemoryManager->allocate((XMLString::stringLen(expression)+2)*sizeof(XMLCh));
        *fExpression = chPeriod;
        *(fExpression+1) = chNull;
        XMLString::catString(fExpression, expression);
        fMoveToRoot=true;
    }
    else
        fExpression=XMLString::replicate(expression);

    try
    {
        WrapperForXPathNSResolver wrappedResolver(fStringPool, resolver, fMemoryManager);
        fParsedExpression = new (fMemoryManager) XercesXPath(fExpression, fStringPool, &wrappedResolver, 0, true, fMemoryManager);
    }
    catch(const XPathException& )
    {
        throw DOMXPathException(DOMXPathException::INVALID_EXPRESSION_ERR, 0, fMemoryManager);
    }
    catch(const OutOfMemoryException&)
    {
        cleanup.release();

        throw;
    }

    cleanup.release();
}

DOMXPathExpressionImpl::~DOMXPathExpressionImpl()
{
    cleanUp();
}

void DOMXPathExpressionImpl::cleanUp()
{
    XMLString::release(&fExpression, fMemoryManager);
    delete fParsedExpression;
    delete fStringPool;
}

DOMXPathResult* DOMXPathExpressionImpl::evaluate(const DOMNode *contextNode,
                                                 DOMXPathResult::ResultType type,
                                                 DOMXPathResult* result) const
{
    if(type!=DOMXPathResult::FIRST_ORDERED_NODE_TYPE && type!=DOMXPathResult::ORDERED_NODE_SNAPSHOT_TYPE &&
       type!=DOMXPathResult::ANY_UNORDERED_NODE_TYPE && type!=DOMXPathResult::UNORDERED_NODE_SNAPSHOT_TYPE)
        throw DOMXPathException(DOMXPathException::TYPE_ERR, 0, fMemoryManager);

    if(contextNode==NULL || contextNode->getNodeType()!=DOMNode::ELEMENT_NODE)
        throw DOMException(DOMException::NOT_SUPPORTED_ERR, 0, fMemoryManager);

    JanitorMemFunCall<DOMXPathResultImpl> r_cleanup (
      0, &DOMXPathResultImpl::release);
    DOMXPathResultImpl* r=(DOMXPathResultImpl*)result;
    if(r==NULL)
    {
      r=new (fMemoryManager) DOMXPathResultImpl(type, fMemoryManager);
      r_cleanup.reset (r);
    }
    else
        r->reset(type);

    XPathMatcher matcher(fParsedExpression, fMemoryManager);
    matcher.startDocumentFragment();

    if(fMoveToRoot)
    {
        contextNode=contextNode->getOwnerDocument();
        if(contextNode==NULL)
            throw DOMException(DOMException::NOT_SUPPORTED_ERR, 0, fMemoryManager);

        QName qName(contextNode->getNodeName(), 0, fMemoryManager);
        SchemaElementDecl elemDecl(&qName);
        RefVectorOf<XMLAttr> attrList(0, true, fMemoryManager);
        matcher.startElement(elemDecl, 0, XMLUni::fgZeroLenString, attrList, 0);
        DOMNode* child=contextNode->getFirstChild();
        while(child)
        {
            if(child->getNodeType()==DOMNode::ELEMENT_NODE)
                testNode(&matcher, r, (DOMElement*)child);
            child=child->getNextSibling();
        }
        matcher.endElement(elemDecl, XMLUni::fgZeroLenString);
    }
    else
        testNode(&matcher, r, (DOMElement*)contextNode);

    r_cleanup.release ();
    return r;
}

bool DOMXPathExpressionImpl::testNode(XPathMatcher* matcher, DOMXPathResultImpl* result, DOMElement *node) const
{
    int uriId=fStringPool->addOrFind(node->getNamespaceURI());
    QName qName(node->getNodeName(), uriId, fMemoryManager);
    SchemaElementDecl elemDecl(&qName);
    DOMNamedNodeMap* attrMap=node->getAttributes();
    XMLSize_t attrCount = attrMap->getLength();
    RefVectorOf<XMLAttr> attrList(attrCount, true, fMemoryManager);
    for(XMLSize_t i=0;i<attrCount;i++)
    {
        DOMAttr* attr=(DOMAttr*)attrMap->item(i);
        attrList.addElement(new (fMemoryManager) XMLAttr(fStringPool->addOrFind(attr->getNamespaceURI()),
                                                         attr->getNodeName(),
                                                         attr->getNodeValue(),
                                                         XMLAttDef::CData,
                                                         attr->getSpecified(),
                                                         fMemoryManager,
                                                         NULL,
                                                         true));
    }
    matcher->startElement(elemDecl, uriId, node->getPrefix(), attrList, attrCount);
    unsigned char nMatch=matcher->isMatched();
    if(nMatch!=0 && nMatch!=XPathMatcher::XP_MATCHED_DP)
    {
        result->addResult(node);
        if(result->getResultType()==DOMXPathResult::ANY_UNORDERED_NODE_TYPE || result->getResultType()==DOMXPathResult::FIRST_ORDERED_NODE_TYPE)
            return true;    // abort navigation, we found one result
    }

    if(nMatch==0 || nMatch==XPathMatcher::XP_MATCHED_D || nMatch==XPathMatcher::XP_MATCHED_DP)
    {
        DOMNode* child=node->getFirstChild();
        while(child)
        {
            if(child->getNodeType()==DOMNode::ELEMENT_NODE)
                if(testNode(matcher, result, (DOMElement*)child))
                    return true;
            child=child->getNextSibling();
        }
    }
    matcher->endElement(elemDecl, XMLUni::fgZeroLenString);
    return false;
}

void DOMXPathExpressionImpl::release()
{
    DOMXPathExpressionImpl* me = this;
    delete me;
}

XERCES_CPP_NAMESPACE_END
