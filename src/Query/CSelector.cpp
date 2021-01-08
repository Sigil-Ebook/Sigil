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

#include "Query/CSelector.h"
#include "Query/CQueryUtil.h"

bool CSelector::match(GumboNode* apNode)
{
	switch (mOp)
	{
		case EDummy:
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
				if (child->type == GUMBO_NODE_TEXT || child->type == GUMBO_NODE_ELEMENT)
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

			unsigned int count = 0;
			for (unsigned int i = 0; i < parent->v.element.children.length; i++)
			{
				GumboNode* child = (GumboNode*) parent->v.element.children.data[i];
				if (child->type != GUMBO_NODE_ELEMENT
						|| (mOfType && apNode->v.element.tag == child->v.element.tag))
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

			unsigned int i = 0;
			unsigned int count = 0;
			for (unsigned int j = 0; j < parent->v.element.children.length; j++)
			{
				GumboNode* child = (GumboNode*) parent->v.element.children.data[j];
				if (child->type != GUMBO_NODE_ELEMENT
						|| (mOfType && apNode->v.element.tag == child->v.element.tag))
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

			if (mLast)
			{
				i = count - i + 1;
			}
			i -= mB;
			if (mA == 0)
			{
				return i == 0;
			}

			return i % mA == 0 && i / mA > 0;
		}
		case ETag:
			return apNode->type == GUMBO_NODE_ELEMENT && apNode->v.element.tag == mTag;
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
	mOp = EAdjacent;
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
		case EAdjacent:
		{
			if (!mpS2->match(apNode))
			{
				return false;
			}

			if (apNode->type != GUMBO_NODE_ELEMENT)
			{
				return false;
			}

			size_t pos = apNode->index_within_parent;
			GumboNode* parent = apNode->parent;
			if (mAdjacent)
			{
				for (long i = pos; i >= 0; i--)
				{
					GumboNode* sibling = (GumboNode*) parent->v.element.children.data[i];
					if (sibling->type == GUMBO_NODE_TEXT || sibling->type == GUMBO_NODE_COMMENT)
					{
						continue;
					}

					return mpS1->match(sibling);
				}
				return false;
			}

			for (long i = pos; i >= 0; i--)
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

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */

