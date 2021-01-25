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

#ifndef CNODE_H_
#define CNODE_H_

#include "gumbo.h"
#include "gumbo_edit.h"

#include <string>
#include <unordered_set>

class CSelection;

class CNode
{

 public:

    CNode(GumboNode* apNode = NULL);

    virtual ~CNode();

 public:

    bool valid();

    CNode parent();

    CNode nextSibling();

    CNode prevSibling();

    unsigned int childNum();

    CNode childAt(size_t i);

    std::string attribute(std::string key);

    std::string text();

    std::string ownText();

    size_t startPos();

    size_t endPos();

    size_t startPosOuter();

    size_t endPosOuter();

    std::string tag();

    GumboNode* raw();

    CSelection find(std::string aSelector);

 private:

    static bool in_set(std::unordered_set<std::string> &s, std::string key);

    void replace_all(std::string &s, const char * s1, const char * s2);
    
    GumboNode* mpNode;
};

#endif /* CNODE_H_ */
