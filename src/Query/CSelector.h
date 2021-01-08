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

		CSelector(unsigned int aA, unsigned int aB, bool aLast, bool aOfType)
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
		}

		void matchAllInto(GumboNode* apNode, std::vector<GumboNode*>& nodes);

	private:

		TOperator mOp;

		bool mOfType;

		unsigned int mA;

		unsigned int mB;

		bool mLast;

		GumboTag mTag;
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
			// || 操作符
			EUnion,
			// && 操作符
			EIntersection,
			//
			EChild,
			//
			EDescendant,
			//
			EAdjacent,
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
			/**
			 * 是否存在
			 */
			EExists,
			/**
			 * 是否相等
			 */
			EEquals,
			/**
			 * 是否包含
			 */
			EIncludes,
			/**
			 * 是否-开始
			 */
			EDashMatch,
			/**
			 * 是否前缀
			 */
			EPrefix,
			/**
			 * 是否后缀
			 */
			ESuffix,
			/**
			 * 是否子串
			 */
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

	private:

		std::string mValue;

		TOperator mOp;
};

#endif /* CSELECTOR_H_ */

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
