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
#include <exception>
#include <stdexcept>

#include "Query/CParser.h"
#include "Query/CQueryUtil.h"
#include "Query/CNode.h"
#include "Query/CSelection.h"

CSelection::CSelection(GumboNode* apNode, bool error)
{
    if(apNode) mNodes.push_back(apNode);
    mparseError = error;
}

CSelection::CSelection(std::vector<GumboNode*> aNodes, bool error)
{
    mNodes = aNodes;
    mparseError = error;
}

CSelection::~CSelection()
{
}

CSelection CSelection::find(std::string aSelector)
{
    // parsing the any selector can throw exceptions
    // try to fail gracefully 
    try {
        CSelector* sel = CParser::create(aSelector);
        std::vector<GumboNode*> ret;
        for (std::vector<GumboNode*>::iterator it = mNodes.begin(); it != mNodes.end(); it++)
        {
            GumboNode* pNode = *it;
            std::vector<GumboNode*> matched = sel->matchAll(pNode);
            ret = CQueryUtil::unionNodes(ret, matched);
        }
        sel->release();
        return CSelection(ret);
    } catch(const std::runtime_error &e) {
        std::cout << "***Query Parser Error***: " << e.what() << std::endl;
        return CSelection(NULL, true);
    }
}

CNode CSelection::nodeAt(size_t i)
{
    if (i >= mNodes.size())
    {
        return CNode();
    }

    return CNode(mNodes[i]);
}

size_t CSelection::nodeNum()
{
    return mNodes.size();
}
