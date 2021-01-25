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

#include "Query/CNode.h"
#include "Query/CSelection.h"
#include "Query/CQueryUtil.h"

/** Gumbo Node Types
 ** ----------------
 **
 ** GUMBO_NODE_DOCUMENT
 ** Document node.  v will be a GumboDocument.
 **
 ** GUMBO_NODE_ELEMENT
 ** Element node.  v will be a GumboElement.
 **
 ** GUMBO_NODE_TEXT
 ** Text node.  v will be a GumboText.
 **
 ** GUMBO_NODE_CDATA
 ** CDATA node. v will be a GumboText.
 **
 ** GUMBO_NODE_COMMENT
 ** Comment node.  v will be a GumboText, excluding comment delimiters.
 **
 ** GUMBO_NODE_WHITESPACE
 ** Text node, where all contents is whitespace.  v will be a GumboText.
 **
 ** GUMBO_NODE_TEMPLATE
 ** Template node.  This is separate from GUMBO_NODE_ELEMENT because many
 ** client libraries will want to ignore the contents of template nodes, as
 ** the spec suggests.  Recursing on GUMBO_NODE_ELEMENT will do the right thing
 ** here, while clients that want to include template contents should also
 ** check for GUMBO_NODE_TEMPLATE.  v will be a GumboElement.
 **
 **/


static std::unordered_set<std::string> void_tags          = {
    "area","base","basefont","bgsound","br","col","command","embed",
    "event-source","frame","hr","img","input","keygen","link",
    "meta","param","source","spacer","track","wbr", 
    "mbp:pagebreak", "mglyph", "mspace", "mprescripts", "none",
    "maligngroup", "malignmark", "msline"
};


bool CNode::in_set(std::unordered_set<std::string> &s, std::string key)
{
    return s.find(key) != s.end();
}


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

// starting position of possible contents of tag or text
size_t CNode::startPos()
{
    if (!valid()) return 0;

    switch(mpNode->type)
    {
        case GUMBO_NODE_ELEMENT:
            if (in_set(void_tags, tag())) {
                // nothing is contained in a void tag so where should this point?
                // after the void tag closing '>" makes no sense
                return 0;
            }
            return mpNode->v.element.start_pos.offset + mpNode->v.element.original_tag.length;
        case GUMBO_NODE_TEXT:
            return mpNode->v.text.start_pos.offset;
        default:
            return 0;
    }
}

// ending position of possible contents
size_t CNode::endPos()
{
    if (!valid()) return 0;

    switch(mpNode->type)
    {
        case GUMBO_NODE_ELEMENT:
            if (in_set(void_tags, tag())) {
                // nothing is contained in a void tag so where should this point?
                // mpNode->v.element.end_pos.offset + mpNode->v.element.original_tag_length;
                return 0;
            }
            return mpNode->v.element.end_pos.offset;
        case GUMBO_NODE_TEXT:
        case GUMBO_NODE_WHITESPACE:
            return mpNode->v.text.original_text.length + startPos();
        default:
            return 0;
    }
}

// starting point of tag or text itself 
size_t CNode::startPosOuter()
{
    if (!valid()) return 0;

    switch(mpNode->type)
    {
        case GUMBO_NODE_ELEMENT:
            return mpNode->v.element.start_pos.offset;
        case GUMBO_NODE_TEXT:
        case GUMBO_NODE_WHITESPACE:
            return mpNode->v.text.start_pos.offset;
    default:
        return 0;
    }
}

// ending position of its closing tag or end of text
size_t CNode::endPosOuter()
{
    if (!valid()) return 0;

    switch(mpNode->type)
    {
    case GUMBO_NODE_ELEMENT:
        if (in_set(void_tags, tag())) {
            return mpNode->v.element.end_pos.offset + mpNode->v.element.original_tag.length;
        }
        return mpNode->v.element.end_pos.offset + mpNode->v.element.original_end_tag.length;
    case GUMBO_NODE_TEXT:
    case GUMBO_NODE_WHITESPACE:
        return mpNode->v.text.original_text.length + startPos();
    default:
        return 0;
    }
}

std::string CNode::tag()
{
    if (!valid()) return "";

    GumboNode* node = mpNode;
    std::string tagname;
    if (node->type == GUMBO_NODE_DOCUMENT) {
        tagname = "#document";
        return tagname;
    } else if ((node->type == GUMBO_NODE_TEXT) || (node->type == GUMBO_NODE_WHITESPACE)) {
        tagname = "#text";
        return tagname;
    } else if (node->type == GUMBO_NODE_CDATA) {
        tagname = "#cdata";
        return tagname;
    } else if (node->type == GUMBO_NODE_COMMENT) {
        tagname = "#comment";
        return tagname;
    }
    tagname = gumbo_normalized_tagname(node->v.element.tag);
    if ((tagname.empty()) ||
        (node->v.element.tag_namespace == GUMBO_NAMESPACE_SVG)) {

        // set up to examine original text of tag.
        GumboStringPiece gsp = node->v.element.original_tag;
        gumbo_tag_from_original_text(&gsp);

        // special handling for some svg tag names.
        if (node->v.element.tag_namespace  == GUMBO_NAMESPACE_SVG) {
            const char * data = gumbo_normalize_svg_tagname(&gsp);
            // NOTE: data may not be null-terminated!
            // since case change only - length must be same as original
            // if no replacement found returns null, not original tag!
            if (data != NULL) {
                return std::string(data, gsp.length);
            }
        }
        
        if (tagname.empty()) {
            tagname = std::string(gsp.data, gsp.length); 
            // replace any quotes in tag name with underscores for safety
            replace_all(tagname, "'", "_");
            replace_all(tagname, "\"", "_");
            return tagname;
        }
    }
    return tagname;
}


GumboNode* CNode::raw() {
    return mpNode;
}


void CNode::replace_all(std::string &s, const char * s1, const char * s2)
{
    std::string t1(s1);
    size_t len = t1.length();
    size_t pos = s.find(t1);
    while (pos != std::string::npos) {
        s.replace(pos, len, s2);
        pos = s.find(t1, pos + len);
    }
}


CSelection CNode::find(std::string aSelector)
{
    if (valid()) {
        CSelection c(mpNode);
        return c.find(aSelector);
    }
    return CSelection(NULL);
}
