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

#ifndef CPARSER_H_
#define CPARSER_H_

#include <string>
#include "gumbo.h"
#include "gumbo_edit.h"
#include "Query/CSelector.h"

class CParser
{

 private:

    CParser(std::string aInput);

 public:

    virtual ~CParser();

 public:

    static CSelector* create(std::string aInput);

 private:

    CSelector* parseSelectorGroup();

    CSelector* parseSelector();

    CSelector* parseSimpleSelectorSequence();

    void parseNth(int& aA, int& aB);

    int parseInteger();

    CSelector* parsePseudoclassSelector();

    CSelector* parseAttributeSelector();

    CSelector* parseClassSelector();

    CSelector* parseIDSelector();

    CSelector* parseTypeSelector();

    bool consumeClosingParenthesis();

    bool consumeParenthesis();

    bool skipWhitespace();

    std::string parseString();

    std::string parseName();

    std::string parseIdentifier();

    bool nameChar(char c);

    bool nameStart(char c);

    bool hexDigit(char c);

    std::string parseEscape();

    std::string error(std::string message);


 private:

    std::string mInput;
    size_t mOffset;
};

#endif /* CPARSER_H_ */
