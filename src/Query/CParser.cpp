/************************************************************************
 **
 **  Copyright (C) 2021 Kevin B. Hendricks, Stratford, ON, Canada
 **
 **  This file is part of Sigil.
 **
 **  Sigil is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Sigil is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Sigil.  If not, see <http://www.gnu.org/licenses/>.
 **
 **
 ** Taken from:
 ** 
 ** gumbo-query
 ** https://github.com/lazytiger/gumbo-query
 **
 ** A C++ library that provides jQuery-like selectors for Google's Gumbo-Parser.
 ** Selector engine is an implementation based on cascadia.
 **
 ** Available under the MIT License  
 ** See ORIGINAL_LICENSE file in the source code 
 ** hoping@baimashi.com, Copyright (C) 2016
 **
 *************************************************************************/

#include <exception>
#include <stdexcept>

#include <string>

class QueryParserException : public std::runtime_error
{
public:
    QueryParserException(const std::string &msg) : std::runtime_error(msg) {};
};

#include "Query/CParser.h"
#include "Query/CSelector.h"
#include "Query/CQueryUtil.h"

CParser::CParser(std::string aInput)
{
	mInput = aInput;
	mOffset = 0;
}

CParser::~CParser()
{
}

CSelector* CParser::create(std::string aInput)
{
	CParser parser(aInput);
	return parser.parseSelectorGroup();
}

CSelector* CParser::parseSelectorGroup()
{
	CSelector* ret = parseSelector();
	while (mOffset < mInput.size())
	{
		if (mInput[mOffset] != ',')
		{
			return ret;
		}

		mOffset++;

		CSelector* sel = parseSelector();
		CSelector* oldRet = ret;
		ret = new CBinarySelector(CBinarySelector::EUnion, ret, sel);
		sel->release();
		oldRet->release();
	}

	return ret;
}

CSelector* CParser::parseSelector()
{
	skipWhitespace();
	CSelector* ret = parseSimpleSelectorSequence();
	while (true)
	{
		char combinator = 0;
		if (skipWhitespace())
		{
			combinator = ' ';
		}

		if (mOffset >= mInput.size())
		{
			return ret;
		}

		char c = mInput[mOffset];
		if (c == '+' || c == '>' || c == '~')
		{
			combinator = c;
			mOffset++;
			skipWhitespace();
		}
		else if (c == ',' || c == ')')
		{
			return ret;
		}

		if (combinator == 0)
		{
			return ret;
		}

		CSelector* oldRet = ret;
		CSelector* sel = parseSimpleSelectorSequence();
		bool isOk = true;
		if (combinator == ' ')
		{
			ret = new CBinarySelector(CBinarySelector::EDescendant, oldRet, sel);
		}
		else if (combinator == '>')
		{
			ret = new CBinarySelector(CBinarySelector::EChild, oldRet, sel);
		}
		else if (combinator == '+')
		{
			ret = new CBinarySelector(oldRet, sel, true);
		}
		else if (combinator == '~')
		{
			ret = new CBinarySelector(oldRet, sel, true);
		}
		else
		{
			isOk = false;
		}
		oldRet->release();
		sel->release();
		if(!isOk) 
		{
                    throw QueryParserException(error("impossible"));
		}

	}
}

CSelector* CParser::parseSimpleSelectorSequence()
{
	CSelector* ret = NULL;
	if (mOffset >= mInput.size())
	{
            throw QueryParserException(error("expected selector, found EOF instead"));
	}

	char c = mInput[mOffset];
	if (c == '*')
	{
		mOffset++;
	}
	else if (c == '#' || c == '.' || c == '[' || c == ':')
	{

	}
	else
	{
		ret = parseTypeSelector();
	}

	while (mOffset < mInput.size())
	{
		char c = mInput[mOffset];
		CSelector* sel = NULL;
		if (c == '#')
		{
			sel = parseIDSelector();
		}
		else if (c == '.')
		{
			sel = parseClassSelector();
		}
		else if (c == '[')
		{
			sel = parseAttributeSelector();
		}
		else if (c == ':')
		{
			sel = parsePseudoclassSelector();
		}
		else
		{
			break;
		}

		if (ret == NULL)
		{
			ret = sel;
		}
		else
		{
			CSelector* oldRet = ret;
			ret = new CBinarySelector(CBinarySelector::EIntersection, ret, sel);
			sel->release();
			oldRet->release();
		}
	}

	if (ret == NULL)
	{
		ret = new CSelector();
	}

	return ret;
}

void CParser::parseNth(int& aA, int& aB)
{

	if (mOffset >= mInput.size())
	{
		goto eof;
	}

	{
		char c = mInput[mOffset];
		if (c == '-')
		{
			mOffset++;
			goto negativeA;
		}
		else if (c == '+')
		{
			mOffset++;
			goto positiveA;
		}
		else if (c >= '0' && c <= '9')
		{
			goto positiveA;
		}
		else if (c == 'n' || c == 'N')
		{
			goto readN;
		}
		else if (c == 'o' || c == 'O' || c == 'e' || c == 'E')
		{
			std::string id = parseName();
			id = CQueryUtil::tolower(id);
			if (id == "odd")
			{
				aA = 2;
				aB = 1;
			}
			else if (id == "even")
			{
				aA = 2;
				aB = 0;
			}
			else
			{
                            throw QueryParserException(error("expected 'odd' or 'even', invalid found"));
			}
			return;
		}
		else
		{
			goto invalid;
		}
	}

	positiveA:
	{
		if (mOffset >= mInput.size())
		{
			goto eof;
		}
		char c = mInput[mOffset];
		if (c >= '0' && c <= '9')
		{
			aA = parseInteger();
			goto readA;
		}
		else if (c == 'n' || c == 'N')
		{
			aA = 1;
			mOffset++;
			goto readN;
		}
		else
		{
			goto invalid;
		}
	}

	negativeA:
	{
		if (mOffset >= mInput.size())
		{
			goto eof;
		}
		char c = mInput[mOffset];
		if (c >= '0' && c <= '9')
		{
			aA = -parseInteger();
			goto readA;
		}
		else if (c == 'n' || c == 'N')
		{
			aA = -1;
			mOffset++;
			goto readN;
		}
		else
		{
			goto invalid;
		}
	}

	readA:
	{
		if (mOffset >= mInput.size())
		{
			goto eof;
		}

		char c = mInput[mOffset];
		if (c == 'n' || c == 'N')
		{
			mOffset++;
			goto readN;
		}
		else
		{
			aB = aA;
			aA = 0;
			return;
		}

	}

	readN:
	{
		skipWhitespace();
		if (mOffset >= mInput.size())
		{
			goto eof;
		}

		char c = mInput[mOffset];
		if (c == '+')
		{
			mOffset++;
			skipWhitespace();
			aB = parseInteger();
			return;
		}
		else if (c == '-')
		{
			mOffset--;
			skipWhitespace();
			aB = -parseInteger();
			return;
		}
		else
		{
			aB = 0;
			return;
		}
	}

	eof:
	{
            throw QueryParserException(error("unexpected EOF while attempting to parse expression of form an+b"));
	}

	invalid:
	{
            throw QueryParserException(error("unexpected character while attempting to parse expression of form an+b"));
	}

}

int CParser::parseInteger()
{
	size_t offset = mOffset;
	int i = 0;
	for (; offset < mInput.size(); offset++)
	{
		char c = mInput[offset];
		if (c < '0' || c > '9')
		{
			break;
		}
		i = i * 10 + c - '0';
	}

	if (offset == mOffset)
	{
            throw QueryParserException(error("expected integer, but didn't find it."));
	}
	mOffset = offset;

	return i;
}

CSelector* CParser::parsePseudoclassSelector()
{
	if (mOffset >= mInput.size() || mInput[mOffset] != ':')
	{
            throw QueryParserException(error("expected pseudoclass selector (:pseudoclass), found invalid char"));
	}
	mOffset++;
	std::string name = parseIdentifier();
	name = CQueryUtil::tolower(name);
	if (name == "not" || name == "has" || name == "haschild")
	{
		if (!consumeParenthesis())
		{
                    throw QueryParserException(error("expected '(' but didn't find it"));
		}

		CSelector* sel = parseSelectorGroup();
		if (!consumeClosingParenthesis())
		{
			sel->release();
			throw QueryParserException(error("expected ')' but didn't find it"));
		}

		CUnarySelector::TOperator op;
		if (name == "not")
		{
			op = CUnarySelector::ENot;
		}
		else if (name == "has")
		{
			op = CUnarySelector::EHasDescendant;
		}
		else if (name == "haschild")
		{
			op = CUnarySelector::EHasChild;
		}
		else
		{
			sel->release();
			throw QueryParserException(error("impossbile"));
		}
		CSelector* ret = new CUnarySelector(op, sel);
		sel->release();
		return ret;
	}
	else if (name == "contains" || name == "containsown")
	{
		if (!consumeParenthesis() || mOffset >= mInput.size())
		{
                    throw QueryParserException(error("expected '(' but didn't find it"));
		}

		std::string value;
		char c = mInput[mOffset];
		if (c == '\'' || c == '"')
		{
			value = parseString();
		}
		else
		{
			value = parseIdentifier();
		}
		value = CQueryUtil::tolower(value);
		skipWhitespace();

		if (!consumeClosingParenthesis())
		{
                   throw QueryParserException(error("expected ')' but didn't find it"));
		}

		CTextSelector::TOperator op;
		if (name == "contains")
		{
			op = CTextSelector::EContains;
		}
		else if (name == "containsown")
		{
			op = CTextSelector::EOwnContains;
		}
		else
		{
                    throw QueryParserException(error("impossibile"));
		}
		return new CTextSelector(op, value);
	}
	else if (name == "matches" || name == "matchesown")
	{
		//TODO
                throw QueryParserException(error("unsupported regex"));
	}
	else if (name == "nth-child" || name == "nth-last-child" || name == "nth-of-type"
			|| name == "nth-last-of-type")
	{
		if (!consumeParenthesis())
		{
                    throw QueryParserException(error("expected '(' but didn't find it"));
		}

		int a, b;
		parseNth(a, b);

		if (!consumeClosingParenthesis())
		{
                    throw QueryParserException(error("expected ')' but didn't find it"));
		}

		return new CSelector(a, b, name == "nth-last-child" || name == "nth-last-of-type",
				name == "nth-of-type" || name == "nth-last-of-type");
	}
	else if (name == "first-child")
	{
		return new CSelector(0, 1, false, false);
	}
	else if (name == "last-child")
	{
		return new CSelector(0, 1, true, false);
	}
	else if (name == "first-of-type")
	{
		return new CSelector(0, 1, false, true);
	}
	else if (name == "last-of-type")
	{
		return new CSelector(0, 1, true, true);
	}
	else if (name == "only-child")
	{
		return new CSelector(false);
	}
	else if (name == "only-of-type")
	{
		return new CSelector(true);
	}
	else if (name == "empty")
	{
		return new CSelector(CSelector::EEmpty);
	}
	else
	{
            throw QueryParserException(error("unsupported op:" + name));
	}
}

CSelector* CParser::parseAttributeSelector()
{
	if (mOffset >= mInput.size() || mInput[mOffset] != '[')
	{
            throw QueryParserException(error("expected attribute selector ([attribute]), found invalid char"));
	}
	mOffset++;
	skipWhitespace();
	std::string key = parseIdentifier();
	skipWhitespace();
	if (mOffset >= mInput.size())
	{
            throw QueryParserException(error("unexpected EOF in attribute selector"));
	}

	if (mInput[mOffset] == ']')
	{
		mOffset++;
		return new CAttributeSelector(CAttributeSelector::EExists, key);
	}

	if (mOffset + 2 > mInput.size())
	{
            throw QueryParserException(error("unexpected EOF in attribute selector"));
	}

	std::string op = mInput.substr(mOffset, 2);
	if (op[0] == '=')
	{
		op = "=";
	}
	else if (op[1] != '=')
	{
            throw QueryParserException(error("expected equality operator, found invalid char"));
	}

	mOffset += op.size();
	skipWhitespace();
	if (mOffset >= mInput.size())
	{
            throw QueryParserException(error("unexpected EOF in attribute selector"));
	}

	std::string value;
	if (op == "#=")
	{
		//TODo
                throw QueryParserException(error("unsupported regex"));
	}
	else
	{
		char c = mInput[mOffset];
		if (c == '\'' || c == '"')
		{
			value = parseString();
		}
		else
		{
			value = parseIdentifier();
		}
	}
	skipWhitespace();
	if (mOffset >= mInput.size() || mInput[mOffset] != ']')
	{
            throw QueryParserException(error("expected attribute selector ([attribute]), found invalid char"));
	}
	mOffset++;

	CAttributeSelector::TOperator aop;
	if (op == "=")
	{
		aop = CAttributeSelector::EEquals;
	}
	else if (op == "~=")
	{
		aop = CAttributeSelector::EIncludes;
	}
	else if (op == "|=")
	{
		aop = CAttributeSelector::EDashMatch;
	}
	else if (op == "^=")
	{
		aop = CAttributeSelector::EPrefix;
	}
	else if (op == "$=")
	{
		aop = CAttributeSelector::ESuffix;
	}
	else if (op == "*=")
	{
		aop = CAttributeSelector::ESubString;
	}
	else if (op == "#=")
	{
		//TODO
            throw QueryParserException(error("unsupported regex"));
	}
	else
	{
            throw QueryParserException(error("unsupported op:" + op));
	}
	return new CAttributeSelector(aop, key, value);
}

CSelector* CParser::parseClassSelector()
{
	if (mOffset >= mInput.size() || mInput[mOffset] != '.')
	{
            throw QueryParserException(error("expected class selector (.class), found invalid char"));
	}
	mOffset++;
	std::string clazz = parseIdentifier();

	return new CAttributeSelector(CAttributeSelector::EIncludes, "class", clazz);
}

CSelector* CParser::parseIDSelector()
{
	if (mOffset >= mInput.size() || mInput[mOffset] != '#')
	{
            throw QueryParserException(error("expected id selector (#id), found invalid char"));
	}
	mOffset++;
	std::string id = parseName();

	return new CAttributeSelector(CAttributeSelector::EEquals, "id", id);
}

CSelector* CParser::parseTypeSelector()
{
	std::string tag = parseIdentifier();
	return new CSelector(gumbo_tag_enum(tag.c_str()));
}

bool CParser::consumeClosingParenthesis()
{
	size_t offset = mOffset;
	skipWhitespace();
	if (mOffset < mInput.size() && mInput[mOffset] == ')')
	{
		mOffset++;
		return true;
	}
	mOffset = offset;
	return false;
}

bool CParser::consumeParenthesis()
{
	if (mOffset < mInput.size() && mInput[mOffset] == '(')
	{
		mOffset++;
		skipWhitespace();
		return true;
	}
	return false;
}

bool CParser::skipWhitespace()
{
	size_t offset = mOffset;
	while (offset < mInput.size())
	{
		char c = mInput[offset];
		if (c == ' ' || c == '\r' || c == '\t' || c == '\n' || c == '\f')
		{
			offset++;
			continue;
		}
		else if (c == '/')
		{
			if (mInput.size() > offset + 1 && mInput[offset + 1] == '*')
			{
				size_t pos = mInput.find("*/", offset + 2);
				if (pos != std::string::npos)
				{
					offset = pos + 2;
					continue;
				}
			}
		}
		break;
	}

	if (offset > mOffset)
	{
		mOffset = offset;
		return true;
	}

	return false;
}

std::string CParser::parseString()
{
	size_t offset = mOffset;
	if (mInput.size() < offset + 2)
	{
            throw QueryParserException(error("expected string, found EOF instead"));
	}

	char quote = mInput[offset];
	offset++;
	std::string ret;

	while (offset < mInput.size())
	{
		char c = mInput[offset];
		if (c == '\\')
		{
			if (mInput.size() > offset + 1)
			{
				char c = mInput[offset + 1];
				if (c == '\r')
				{
					if (mInput.size() > offset + 2 && mInput[offset + 2] == '\n')
					{
						offset += 3;
						continue;
					}
				}

				if (c == '\r' || c == '\n' || c == '\f')
				{
					offset += 2;
					continue;
				}
			}
			mOffset = offset;
			ret += parseEscape();
			offset = mOffset;
		}
		else if (c == quote)
		{
			break;
		}
		else if (c == '\r' || c == '\n' || c == '\f')
		{
                    throw QueryParserException(error("unexpected end of line in string"));
		}
		else
		{
			size_t start = offset;
			while (offset < mInput.size())
			{
				char c = mInput[offset];
				if (c == quote || c == '\\' || c == '\r' || c == '\n' || c == '\f')
				{
					break;
				}
				offset++;
			}
			ret += mInput.substr(start, offset - start);
		}
	}

	if (offset >= mInput.size())
	{
            throw QueryParserException(error("EOF in string"));
	}

	offset++;
	mOffset = offset;

	return ret;
}

std::string CParser::parseName()
{
	size_t offset = mOffset;
	std::string ret;
	while (offset < mInput.size())
	{
		char c = mInput[offset];
		if (nameChar(c))
		{
			size_t start = offset;
			while (offset < mInput.size() && nameChar(mInput[offset]))
			{
				offset++;
			}
			ret += mInput.substr(start, offset - start);
		}
		else if (c == '\\')
		{
			mOffset = offset;
			ret += parseEscape();
			offset = mOffset;
		}
		else
		{
			break;
		}
	}

	if (ret == "")
	{
            throw QueryParserException(error("expected name, found EOF instead"));
	}

	mOffset = offset;
	return ret;
}

std::string CParser::parseIdentifier()
{
	bool startingDash = false;
	if (mInput.size() > mOffset && mInput[mOffset] == '-')
	{
		startingDash = true;
		mOffset++;
	}

	if (mInput.size() <= mOffset)
	{
            throw QueryParserException(error("expected identifier, found EOF instead"));
	}

	char c = mInput[mOffset];
	if (!nameStart(c) && c != '\\')
	{
            throw QueryParserException(error("expected identifier, found invalid char"));
	}

	std::string name = parseName();
	if (startingDash)
	{
		name = "-" + name;
	}

	return name;
}

bool CParser::nameChar(char c)
{
	return nameStart(c) || (c == '-') || (c >= '0' && c <= '9');
}

bool CParser::nameStart(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_') || (c > 127);
}

bool CParser::hexDigit(char c)
{
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

std::string CParser::parseEscape()
{
	if (mInput.size() < mOffset + 2 || mInput[mOffset] != '\\')
	{
            throw QueryParserException(error("invalid escape sequence"));
	}

	size_t start = mOffset + 1;
	char c = mInput[start];
	if (c == '\r' || c == '\n' || c == '\f')
	{
            throw QueryParserException(error("escaped line ending outside string"));
	}

	if (!hexDigit(c))
	{
		std::string ret = mInput.substr(start, 1);
		mOffset += 2;
		return ret;
	}

	size_t i = 0;
	std::string ret;
	c = 0;
	for (i = start; i < mOffset + 6 && i < mInput.size() && hexDigit(mInput[i]); i++)
	{
		unsigned int d = 0;
		char ch = mInput[i];
		if (ch >= '0' && ch <= '9')
		{
			d = ch - '0';
		}
		else if (ch >= 'a' && ch <= 'f')
		{
			d = ch - 'a' + 10;
		}
		else if (ch >= 'A' && ch <= 'F')
		{
			d = ch - 'A' + 10;
		}
		else
		{
                    throw QueryParserException(error("impossible"));
		}

		if ((i - start) % 2)
		{
			c += d;
			ret.push_back(c);
			c = 0;
		}
		else
		{
			c += (d << 4);
		}
	}

	if (ret.size() == 0 || c != 0)
	{
            throw QueryParserException(error("invalid hex digit"));
	}

	if (mInput.size() > i)
	{
		switch (mInput[i])
		{
			case '\r':
				i++;
				if (mInput.size() > i && mInput[i] == '\n')
				{
					i++;
				}
				break;
			case ' ':
			case '\t':
			case '\n':
			case '\f':
				i++;
				break;
		}
	}

	mOffset = i;
	return ret;
}

std::string CParser::error(std::string message)
{
	size_t d = mOffset;
	std::string ds;
	if (d == 0)
	{
		ds = '0';
	}

	while (d)
	{
		ds.push_back(d % 10 + '0');
		d /= 10;
	}

	std::string ret = message + " at:";
	for (std::string::reverse_iterator rit = ds.rbegin(); rit != ds.rend(); ++rit) {
		ret.push_back(*rit);
	}

	return ret;
}

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */

