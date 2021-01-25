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

#ifndef CSELECTOR_H_
#define CSELECTOR_H_

#include "gumbo.h"
#include "gumbo_edit.h"

#include <string>
#include <vector>
#include "Query/CObject.h"

class CSelector: public CObject
{

 public:

    typedef enum
    {
        //
        EDummy,
        //
        EEmpty,
        //
        EOnlyChild,
        //
        ENthChild,
        //
        ETag,
        //
        ERoot,
        //
        ELang,
    } TOperator;

 public:

    CSelector(TOperator aOp = EDummy)
    {
        init();
        mOp = aOp;
    }

    CSelector(bool aOfType)
    {
        init();
        mOp = EOnlyChild;
        mOfType = aOfType;
    }

    CSelector(int aA, int aB, bool aLast, bool aOfType)
    {
        init();
        mOp = ENthChild;
        mA = aA;
        mB = aB;
        mLast = aLast;
        mOfType = aOfType;
    }

    CSelector(GumboTag aTag)
    {
        init();
        mOp = ETag;
        mTag = aTag;
    }

    CSelector(std::string aLang)
    {
        init();
        mOp = ELang;
        mLang = aLang;
    }

    virtual ~CSelector()
    {
    }

 public:

    virtual bool match(GumboNode* apNode);

    std::vector<GumboNode*> filter(std::vector<GumboNode*> nodes);

    std::vector<GumboNode*> matchAll(GumboNode* apNode);

 private:

    void init()
    {
        mOfType = false;
        mA = 0;
        mB = 0;
        mLast = false;
        mTag = GumboTag(0);
        mLang = "";
    }

    void matchAllInto(GumboNode* apNode, std::vector<GumboNode*>& nodes);

 private:

    TOperator mOp;

    bool mOfType;

    int mA;

    int mB;

    bool mLast;

    GumboTag mTag;

    std::string mLang;
};


class CUnarySelector: public CSelector
{

 public:

    typedef enum
    {
        //
        ENot,
        //
        EHasDescendant,
        //
        EHasChild,
    } TOperator;

 public:

    CUnarySelector(TOperator aOp, CSelector* apS);

    virtual ~CUnarySelector();

 public:

    virtual bool match(GumboNode* apNode);

 private:

    bool hasDescendantMatch(GumboNode* apNode, CSelector* apS);

    bool hasChildMatch(GumboNode* apNode, CSelector* apS);

 private:

    CSelector* mpS;

    TOperator mOp;
};


class CBinarySelector: public CSelector
{

 public:

    typedef enum
    {
        //
        EUnion,
        //
        EIntersection,
        //
        EChild,
        //
        EDescendant,
        //
        ESibling,
            
    } TOperator;

 public:

    CBinarySelector(TOperator aOp, CSelector* apS1, CSelector* apS2);

    CBinarySelector(CSelector* apS1, CSelector* apS2, bool aAdjacent);

    ~CBinarySelector();

 public:

    virtual bool match(GumboNode* apNode);

 private:

    CSelector* mpS1;

    CSelector* mpS2;

    TOperator mOp;

    bool mAdjacent;
};


class CAttributeSelector: public CSelector
{

 public:

    typedef enum
    {
        //
        EExists,
        //
        EEquals,
        //
        EIncludes,
        //
        EDashMatch,
        //
        EPrefix,
        //
        ESuffix,
        //
        ESubString,
    } TOperator;

 public:

    CAttributeSelector(TOperator aOp, std::string aKey, std::string aValue = "");

 public:

    virtual bool match(GumboNode* apNode);

 private:

     std::string mKey;

     std::string mValue;

     TOperator mOp;
};


class CTextSelector: public CSelector
{

 public:

    typedef enum
    {
        //
        EOwnContains,
        //
        EContains,
    } TOperator;

 public:

    CTextSelector(TOperator aOp, std::string aValue)
    {
        mValue = aValue;
        mOp = aOp;
    }

    ~CTextSelector()
    {
    }

 public:

     virtual bool match(GumboNode* apNode);

 private:

    std::string mValue;

    TOperator mOp;
};

#endif /* CSELECTOR_H_ */
