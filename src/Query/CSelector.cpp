/**********************************************************************************
 **
 **  SigilQuery for Gumbo
 **
 **  A C++ library that provides jQuery-like selectors for Google's Gumbo-Parser.
 **  Selector engine is an implementation based on cascadia.
 **
 **  Based on: "gumbo-query" https://github.com/lazytiger/gumbo-query
 **  With bug fixes, extensions and improvements
 **
 **  The MIT License (MIT)
 **  Copyright (c) 2021 Kevin B. Hendricks, Stratford, Ontario Canada
 **  Copyright (c) 2015 baimashi.com. 
 **  Copyright (c) 2011 Andy Balholm. All rights reserved.
 **
 **
 **  Permission is hereby granted, free of charge, to any person obtaining a copy
 **  of this software and associated documentation files (the "Software"), to deal
 **  in the Software without restriction, including without limitation the rights
 **  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 **  copies of the Software, and to permit persons to whom the Software is
 **  furnished to do so, subject to the following conditions:
 **
 **  The above copyright notice and this permission notice shall be included in
 **  all copies or substantial portions of the Software.
 **
 **  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 **  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 **  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 **  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 **  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 **  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 **  THE SOFTWARE.
 **
 **********************************************************************************/

#include <iostream>

#include "Query/CSelector.h"
#include "Query/CQueryUtil.h"
#include "Query/CNode.h"

bool CSelector::match(GumboNode* apNode)
{
    switch (mOp)
    {
        case EDummy:
            if (apNode->type != GUMBO_NODE_ELEMENT)
            {
                return false;
            }
            return true;
        case EEmpty:
        {
            if (apNode->type != GUMBO_NODE_ELEMENT)
            {
                return false;
            }
            GumboVector children = apNode->v.element.children;
            for (unsigned int i = 0; i < children.length; i++)
            {
                GumboNode* child = (GumboNode*) children.data[i];
                // What about GUMBO_NODE_* WHITESPACE, DOCUMENT, CDATA, COMMENT, TEMPLATE
                if (child->type == GUMBO_NODE_TEXT ||
                    child->type == GUMBO_NODE_WHITESPACE ||
                    child->type == GUMBO_NODE_ELEMENT)
                {
                    return false;
                }
            }
            return true;
        }
        case EOnlyChild:
        {
            if (apNode->type != GUMBO_NODE_ELEMENT)
            {
                return false;
            }
            GumboNode* parent = apNode->parent;
            if (parent == NULL)
            {
                return false;
            }
            
            int count = 0;
            for (unsigned int i = 0; i < parent->v.element.children.length; i++)
            {
                GumboNode* child = (GumboNode*) parent->v.element.children.data[i];
                if (child->type != GUMBO_NODE_ELEMENT
                        || (mOfType && apNode->v.element.tag != child->v.element.tag))
                {
                    continue;
                }
                count++;
                if (count > 1)
                {
                    return false;
                }
            }

            return count == 1;
        }
        case ENthChild:
        {
            if (apNode->type != GUMBO_NODE_ELEMENT)
            {
                return false;
            }

            GumboNode* parent = apNode->parent;
            if (parent == NULL)
            {
                return false;
            }
            if (parent->type == GUMBO_NODE_DOCUMENT)
            {
                return false;
            }

            int i = -1;
            int count = 0;
            for (unsigned int j = 0; j < parent->v.element.children.length; j++)
            {
                GumboNode* child = (GumboNode*) parent->v.element.children.data[j];
                if (child->type != GUMBO_NODE_ELEMENT
                        || (mOfType && apNode->v.element.tag != child->v.element.tag))
                {
                    continue;
                }
                count++;
                if (apNode == child)
                {
                    i = count;
                    if (!mLast)
                    {
                        break;
                    }
                }
            }
            // if node did not match anythin above
            if (i == -1)
            {
                return false;
            }
            if (mLast)
            {
                i = count - i + 1;
            }
            i -= mB;
            if (mA == 0)
            {
                return i == 0;
            }
            return ((i % mA) == 0) && ((i / mA)) >= 0;
        }
        case ETag:
            return apNode->type == GUMBO_NODE_ELEMENT && apNode->v.element.tag == mTag;
        case ERoot:
        {
            if (apNode->type != GUMBO_NODE_ELEMENT)
            {
                return false;
            }

            GumboNode* parent = apNode->parent;
            if (parent == NULL)
            {
                return false;
            }
            return parent->type == GUMBO_NODE_DOCUMENT;
        }
            case ELang:
        {
            if (apNode->type != GUMBO_NODE_ELEMENT)
            {
                return false;
            }
            std::string elemLang;
            GumboNode * node = apNode;
            do {
                if (node->type == GUMBO_NODE_ELEMENT)
                {
                    GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, "xml:lang");
                    if (!attr)
                    {
                        attr = gumbo_get_attribute(&node->v.element.attributes, "lang");
                    }
                    if (attr)
                    {
                        elemLang = attr->value;
                        elemLang = CQueryUtil::tolower(elemLang);
                        return (elemLang == mLang) || (elemLang.find(mLang + "-") == 0);
                    }
                }
                node = node->parent;
            } while(node && node->type == GUMBO_NODE_ELEMENT);
            return false;
        }

            default:
            return false;
    }
}

std::vector<GumboNode*> CSelector::filter(std::vector<GumboNode*> nodes)
{
    std::vector<GumboNode*> ret;
    for (std::vector<GumboNode*>::iterator it = nodes.begin(); it != nodes.end(); it++)
    {
        GumboNode* n = *it;
        if (match(n))
        {
            ret.push_back(n);
        }
    }
    return ret;
}

std::vector<GumboNode*> CSelector::matchAll(GumboNode* apNode)
{
    std::vector<GumboNode*> ret;
    matchAllInto(apNode, ret);
    return ret;
}

void CSelector::matchAllInto(GumboNode* apNode, std::vector<GumboNode*>& nodes)
{
    if (match(apNode))
    {
        nodes.push_back(apNode);
    }

    if (apNode->type != GUMBO_NODE_ELEMENT)
    {
        return;
    }

    for (unsigned int i = 0; i < apNode->v.element.children.length; i++)
    {
        GumboNode* child = (GumboNode*) apNode->v.element.children.data[i];
        matchAllInto(child, nodes);
    }
}

CBinarySelector::CBinarySelector(TOperator aOp, CSelector* apS1, CSelector* apS2)
{
    mpS1 = apS1;
    mpS1->retain();
    mpS2 = apS2;
    mpS2->retain();
    mOp = aOp;
    mAdjacent = false;
}

CBinarySelector::~CBinarySelector()
{
    if (mpS1 != NULL)
    {
        mpS1->release();
        mpS1 = NULL;
    }

    if (mpS2 != NULL)
    {
        mpS2->release();
        mpS2 = NULL;
    }
}

CBinarySelector::CBinarySelector(CSelector* apS1, CSelector* apS2, bool aAdjacent)
{
    mpS1 = apS1;
    mpS1->retain();
    mpS2 = apS2;
    mpS2->retain();
    mOp = ESibling;
    mAdjacent = aAdjacent;
}

bool CBinarySelector::match(GumboNode* apNode)
{
    switch (mOp)
    {
        case EUnion:
            return mpS1->match(apNode) || mpS2->match(apNode);
        case EIntersection:
            return mpS1->match(apNode) && mpS2->match(apNode);
        case EChild:
            return mpS2->match(apNode) && apNode->parent != NULL && mpS1->match(apNode->parent);
        case EDescendant:
        {
            if (!mpS2->match(apNode))
            {
                return false;
            }

            for (GumboNode* p = apNode->parent; p != NULL; p = p->parent)
            {
                if (mpS1->match(p))
                {
                    return true;
                }
            }
            return false;
        }
        case ESibling:
        {
            if (!mpS2->match(apNode))
            {
                return false;
            }

            if (apNode->type != GUMBO_NODE_ELEMENT)
            {
                return false;
            }

            int pos = apNode->index_within_parent;
            GumboNode* parent = apNode->parent;
           
            // examine your immediate predecessor in the parent to determine if you
            // are immediately after something in set 1
            if (mAdjacent)
            {
                for (int i = pos-1; i >= 0; i--)
                {
                    GumboNode* sibling = (GumboNode*) parent->v.element.children.data[i];
                    // There are many types of text nodes in gumbo to skip over
                    // GUMBO_NODE_TEXT, GUMBO_NODE_COMMENT, GUMBO_NODE_WHITESPACE, GUMBO_NODE_CDATA
                    if(sibling->type != GUMBO_NODE_ELEMENT)
                    {
                        continue;
                    }
                    return mpS1->match(sibling);
                }
                return false;
            }
            // examine all earlier predecessor siblings to see if any are in set 1
            for (long i = pos-1; i >= 0; i--)
            {
                GumboNode* sibling = (GumboNode*) parent->v.element.children.data[i];
                if (mpS1->match(sibling))
                {
                    return true;
                }
            }
            return false;
        }
        default:
            return false;
    }

    return false;
}

CAttributeSelector::CAttributeSelector(TOperator aOp, std::string aKey, std::string aValue)
{
    mKey = aKey;
    mValue = aValue;
    mOp = aOp;
}

bool CAttributeSelector::match(GumboNode* apNode)
{
    if (apNode->type != GUMBO_NODE_ELEMENT)
    {
        return false;
    }

    GumboVector attributes = apNode->v.element.attributes;
    for (unsigned int i = 0; i < attributes.length; i++)
    {
        GumboAttribute* attr = (GumboAttribute*) attributes.data[i];
        if (mKey != attr->name)
        {
            continue;
        }

        std::string value = attr->value;
        switch (mOp)
        {
            case EExists:
                return true;
            case EEquals:
                return mValue == value;
            case EIncludes:
                for (unsigned int i = 0, j = 0; i < value.size(); i++)
                {
                    if (value[i] == ' ' || value[i] == '\t' || value[i] == '\r' || value[i] == '\n'
                            || value[i] == '\f' || i == value.size() - 1)
                    {
                        unsigned int length = i - j;
                        if (i == value.size() - 1)
                        {
                            length++;
                        }
                        std::string segment = value.substr(j, length);
                        if (segment == mValue)
                        {
                            return true;
                        }
                        j = i + 1;
                    }
                }
                return false;
            case EDashMatch:
                if (mValue == value)
                {
                    return true;
                }
                if (value.size() < mValue.size())
                {
                    return false;
                }
                return value.substr(0, mValue.size()) == mValue && value[mValue.size()] == '-';
            case EPrefix:
                return value.size() >= mValue.size() && value.substr(0, mValue.size()) == mValue;
            case ESuffix:
                return value.size() >= mValue.size()
                        && value.substr(value.size() - mValue.size(), mValue.size()) == mValue;
            case ESubString:
                return value.find(mValue) != std::string::npos;
            default:
                return false;
        }
    }
    return false;
}

CUnarySelector::CUnarySelector(TOperator aOp, CSelector* apS)
{
    mpS = apS;
    mpS->retain();
    mOp = aOp;
}

CUnarySelector::~CUnarySelector()
{
    if (mpS != NULL)
    {
        mpS->release();
        mpS = NULL;
    }
}

bool CUnarySelector::hasDescendantMatch(GumboNode* apNode, CSelector* apS)
{
    for (unsigned int i = 0; i < apNode->v.element.children.length; i++)
    {
        GumboNode* child = (GumboNode*) apNode->v.element.children.data[i];
        if (apS->match(child)
                || (child->type == GUMBO_NODE_ELEMENT && hasDescendantMatch(child, apS)))
        {
            return true;
        }
    }
    return false;
}

bool CUnarySelector::hasChildMatch(GumboNode* apNode, CSelector* apS)
{
    for (unsigned int i = 0; i < apNode->v.element.children.length; i++)
    {
        GumboNode* child = (GumboNode*) apNode->v.element.children.data[i];
        if (apS->match(child))
        {
            return true;
        }
    }
    return false;
}

bool CUnarySelector::match(GumboNode* apNode)
{
    switch (mOp)
    {
        case ENot:
            if (apNode->type != GUMBO_NODE_ELEMENT)
            {
                return false;
            }
            return !mpS->match(apNode);
        case EHasDescendant:
            if (apNode->type != GUMBO_NODE_ELEMENT)
            {
                return false;
            }
            return hasDescendantMatch(apNode, mpS);
        case EHasChild:
            if (apNode->type != GUMBO_NODE_ELEMENT)
            {
                return false;
            }
            return hasChildMatch(apNode, mpS);
        default:
            return false;
    }
}

bool CTextSelector::match(GumboNode* apNode)
{
    std::string text;
    switch (mOp)
    {
        case EContains:
            text = CQueryUtil::nodeText(apNode);
            break;
        case EOwnContains:
            text = CQueryUtil::nodeOwnText(apNode);
            break;
        default:
            return false;
    }

    text = CQueryUtil::tolower(text);
    return text.find(mValue) != std::string::npos;
}
