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

#ifndef CSELECTION_H_
#define CSELECTION_H_

#include "Query/CObject.h"
#include <vector>
#include <string>
#include "gumbo.h"
#include "gumbo_edit.h"

class CNode;

class CSelection: public CObject
{

 public:

    CSelection(GumboNode* apNode, bool error = false);

    CSelection(std::vector<GumboNode*> aNodes, bool error = false);

    virtual ~CSelection();

 public:

    CSelection find(std::string aSelector);

    CNode nodeAt(size_t i);

    size_t nodeNum();

    bool parseError() { return mparseError; }

 private:

    std::vector<GumboNode*> mNodes;
    bool mparseError;
};

#endif /* CSELECTION_H_ */
