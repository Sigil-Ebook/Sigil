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
 ** hoping@baimashi.com, Copyright (C) 2016 **
 **
 *************************************************************************/

#include "CNode.h"
#include "CSelection.h"
#include "CQueryUtil.h"

CNode::CNode(GumboNode* apNode)
{
	mpNode = apNode;
}

CNode::~CNode()
{
}

CNode CNode::parent()
{
	return CNode(mpNode->parent);
}

CNode CNode::nextSibling()
{
	return parent().childAt(mpNode->index_within_parent + 1);
}

CNode CNode::prevSibling()
{
	return parent().childAt(mpNode->index_within_parent - 1);
}

unsigned int CNode::childNum()
{
	if (mpNode->type != GUMBO_NODE_ELEMENT)
	{
		return 0;
	}

	return mpNode->v.element.children.length;

}

bool CNode::valid()
{
	return mpNode != NULL;
}

CNode CNode::childAt(size_t i)
{
	if (mpNode->type != GUMBO_NODE_ELEMENT || i >= mpNode->v.element.children.length)
	{
		return CNode();
	}

	return CNode((GumboNode*) mpNode->v.element.children.data[i]);
}

std::string CNode::attribute(std::string key)
{
	if (mpNode->type != GUMBO_NODE_ELEMENT)
	{
		return "";
	}

	GumboVector attributes = mpNode->v.element.attributes;
	for (unsigned int i = 0; i < attributes.length; i++)
	{
		GumboAttribute* attr = (GumboAttribute*) attributes.data[i];
		if (key == attr->name)
		{
			return attr->value;
		}
	}

	return "";
}

std::string CNode::text()
{
	return CQueryUtil::nodeText(mpNode);
}

std::string CNode::ownText()
{
	return CQueryUtil::nodeOwnText(mpNode);
}

size_t CNode::startPos()
{
	switch(mpNode->type)
	{
	  case GUMBO_NODE_ELEMENT:
		  return mpNode->v.element.start_pos.offset + mpNode->v.element.original_tag.length;
	  case GUMBO_NODE_TEXT:
		  return mpNode->v.text.start_pos.offset;
	  default:
		  return 0;
  }
}

size_t CNode::endPos()
{
	switch(mpNode->type)
	{
	  case GUMBO_NODE_ELEMENT:
		  return mpNode->v.element.end_pos.offset;
	  case GUMBO_NODE_TEXT:
		  return mpNode->v.text.original_text.length + startPos();
	  default:
		  return 0;
	}
}

size_t CNode::startPosOuter()
{
	switch(mpNode->type)
	{
	case GUMBO_NODE_ELEMENT:
		return mpNode->v.element.start_pos.offset;
	case GUMBO_NODE_TEXT:
		return mpNode->v.text.start_pos.offset;
	default:
		return 0;
	}
}

size_t CNode::endPosOuter()
{
	switch(mpNode->type)
	{
	case GUMBO_NODE_ELEMENT:
		return mpNode->v.element.end_pos.offset + mpNode->v.element.original_end_tag.length;
	case GUMBO_NODE_TEXT:
		return mpNode->v.text.original_text.length + startPos();
	default:
		return 0;
	}
}

std::string CNode::tag()
{
	if (mpNode->type != GUMBO_NODE_ELEMENT)
	{
		return "";
	}

	return gumbo_normalized_tagname(mpNode->v.element.tag);
}

CSelection CNode::find(std::string aSelector)
{
	CSelection c(mpNode);
	return c.find(aSelector);
}
/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
