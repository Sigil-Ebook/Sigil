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
		if (child->type == GUMBO_NODE_TEXT)
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

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
