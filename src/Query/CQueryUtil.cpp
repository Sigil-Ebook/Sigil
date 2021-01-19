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

#include "Query/CQueryUtil.h"

std::string CQueryUtil::tolower(std::string s)
{
    for (unsigned int i = 0; i < s.size(); i++)
    {
        char c = s[i];
        if (c >= 'A' && c <= 'Z')
        {
            c = 'a' + c - 'A';
            s[i] = c;
        }
    }

    return s;
}

std::vector<GumboNode*> CQueryUtil::unionNodes(std::vector<GumboNode*> aNodes1,
        std::vector<GumboNode*> aNodes2)
{
    for (std::vector<GumboNode*>::iterator it = aNodes2.begin(); it != aNodes2.end(); it++)
    {
        GumboNode* pNode = *it;
        if (nodeExists(aNodes1, pNode))
        {
            continue;
        }

        aNodes1.push_back(pNode);
    }

    return aNodes1;
}

bool CQueryUtil::nodeExists(std::vector<GumboNode*> aNodes, GumboNode* apNode)
{
    for (std::vector<GumboNode*>::iterator it = aNodes.begin(); it != aNodes.end(); it++)
    {
        GumboNode* pNode = *it;
        if (pNode == apNode)
        {
            return true;
        }
    }
    return false;
}

std::string CQueryUtil::nodeText(GumboNode* apNode)
{
    std::string text;
    writeNodeText(apNode, text);
    return text;
}

std::string CQueryUtil::nodeOwnText(GumboNode* apNode)
{
    std::string text;
    if (apNode->type != GUMBO_NODE_ELEMENT)
    {
        return text;
    }

    GumboVector children = apNode->v.element.children;
    for (unsigned int i = 0; i < children.length; i++)
    {
        GumboNode* child = (GumboNode*) children.data[i];
        if ((child->type == GUMBO_NODE_TEXT) || (child->type == GUMBO_NODE_WHITESPACE))
        {
            text.append(child->v.text.text);
        }
    }

    return text;
}

void CQueryUtil::writeNodeText(GumboNode* apNode, std::string& aText)
{
    switch (apNode->type)
    {
        case GUMBO_NODE_TEXT:
        case GUMBO_NODE_WHITESPACE:
            aText.append(apNode->v.text.text);
            break;
        case GUMBO_NODE_ELEMENT:
        {
            GumboVector children = apNode->v.element.children;
            for (unsigned int i = 0; i < children.length; i++)
            {
                GumboNode* child = (GumboNode*) children.data[i];
                writeNodeText(child, aText);
            }
            break;
        }
        default:
            break;
    }
}
