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

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
