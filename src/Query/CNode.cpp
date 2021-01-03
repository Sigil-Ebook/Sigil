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

#include "Query/CNode.h"
#include "Query/CSelection.h"
#include "Query/CQueryUtil.h"

CNode::CNode(GumboNode* apNode)
{
	mpNode = apNode;
}

CNode::~CNode()
{
}

CNode CNode::parent()
{
    if (valid()) return CNode(mpNode->parent);
    return CNode();
}

CNode CNode::nextSibling()
{
    if (valid() && parent().valid()) return parent().childAt(mpNode->index_within_parent + 1);
    return CNode();
}

CNode CNode::prevSibling()
{
    if (valid() && parent().valid()) return parent().childAt(mpNode->index_within_parent - 1);
    return CNode();
}

unsigned int CNode::childNum()
{
    if (!valid() || mpNode->type != GUMBO_NODE_ELEMENT) {
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
    if (!valid() || mpNode->type != GUMBO_NODE_ELEMENT || i >= mpNode->v.element.children.length) {
        return CNode();
    }
    return CNode((GumboNode*) mpNode->v.element.children.data[i]);
}

std::string CNode::attribute(std::string key)
{
    if (!valid() || mpNode->type != GUMBO_NODE_ELEMENT) {
        return "";
    }
    GumboVector attributes = mpNode->v.element.attributes;
    for (unsigned int i = 0; i < attributes.length; i++) {
        GumboAttribute* attr = (GumboAttribute*) attributes.data[i];
	if (key == attr->name){
	    return attr->value;
	}
    }
    return "";
}

std::string CNode::text()
{
    if (valid()) return CQueryUtil::nodeText(mpNode);
    return "";
}

std::string CNode::ownText()
{
    if (valid()) return CQueryUtil::nodeOwnText(mpNode);
    return "";
}

size_t CNode::startPos()
{
    if (!valid()) return 0;

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
    if (!valid()) return 0;

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
    if (!valid()) return 0;

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
    if (!valid()) return 0;

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
    if (!valid() || mpNode->type != GUMBO_NODE_ELEMENT) {
        return "";
    }

    return gumbo_normalized_tagname(mpNode->v.element.tag);
}

GumboNode* CNode::raw() {
    return mpNode;
}

CSelection CNode::find(std::string aSelector)
{
    if (valid()) {
	CSelection c(mpNode);
	return c.find(aSelector);
    }
    return CSelection(NULL);
}
/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
