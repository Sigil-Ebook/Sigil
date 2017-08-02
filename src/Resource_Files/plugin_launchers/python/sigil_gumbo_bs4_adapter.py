# -*- coding: utf-8 -*-
# vim:ts=4:sw=4:softtabstop=4:smarttab:expandtab

from __future__ import unicode_literals, print_function

# Copyright 2012 Google Inc. All Rights Reserved.
# Modifications to use BeautifulSoup4 
#   Copyright 2015 Kevin B. Hendricks, Stratford, Ontario, Canada
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# Should this be reworked to be a bs4 treebuilder?

"""
  Adapter between Gumbo and BeautifulSoup4.
  This parses an HTML document and gives back a BeautifulSoup4 object, which you
  can then manipulate like a normal BeautifulSoup4 parse tree.

  Groks namespaces on elements and attributes
"""

__author__ = 'jdtang@google.com (Jonathan Tang)'

import sys
import sigil_gumboc as gumboc

import sigil_bs4
# uses sigil_bs4.element classes:
#      Comment, DocType, NavigableString, CData, Tag, NamespacedAttribute, whitespace_re

# These should be indexed by the enum
# values of gumboc.Namespace

_NAMESPACES = [
    'http://www.w3.org/1999/xhtml',
    'http://www.w3.org/2000/svg',
    'http://www.w3.org/1998/Math/MathML',
    ]


def _fromutf8(text):
    return text.decode('utf-8', 'replace')


def _add_source_info(obj, original_text, start_pos, end_pos):
    obj.original = _fromutf8(bytes(original_text))
    obj.line = start_pos.line
    obj.col = start_pos.column
    obj.offset = start_pos.offset
    if end_pos:
        obj.end_line = end_pos.line
        obj.end_col = end_pos.column
        obj.end_offset = end_pos.offset


def _convert_attrs(element_attrs):
    def maybe_namespace(attr):
        if attr.namespace != gumboc.AttributeNamespace.NONE:
            name = _fromutf8(attr.name)
            prefix = repr(attr.namespace).lower() if name != 'xmlns' else None
            nsurl = attr.namespace.to_url()
            return sigil_bs4.element.NamespacedAttribute(prefix, name, nsurl)
        else:
            return _fromutf8(attr.name)
    def maybe_value_list(attr):
        value = _fromutf8(attr.value)
        if " " in value:
            if _fromutf8(attr.name) == "class" and attr.namespace == gumboc.AttributeNamespace.NONE:
                value = sigil_bs4.element.whitespace_re.split(value)
        return value
    return dict((maybe_namespace(attr), maybe_value_list(attr)) for attr in element_attrs)


def _add_document(soup, element):
    if not element.has_doctype:
        # Mimic html5lib behavior: if no doctype token, no doctype node.
        return
    pub_id = _fromutf8(element.public_identifier)
    sys_id = _fromutf8(element.system_identifier)
    if pub_id == '':
        pub_id = None
    if sys_id == '':
        sys_id = None
    doctype = sigil_bs4.element.Doctype.for_name_and_ids(_fromutf8(element.name),
                                                   pub_id, sys_id)
    soup.object_was_parsed(doctype)


def _add_element(soup, element):
    tag = sigil_bs4.element.Tag(parser=soup,
                  name=_fromutf8(element.tag_name),
                  namespace=_NAMESPACES[element.tag_namespace.value],
                  attrs=_convert_attrs(element.attributes))
    for child in element.children:
        tag.append(_add_node(soup, child))
    _add_source_info(tag, element.original_tag, element.start_pos, element.end_pos)
    tag.original_end_tag = _fromutf8(bytes(element.original_end_tag))
    return tag


def _add_text(cls):
    def add_text_internal(soup, element):
        text = cls(_fromutf8(element.text))
        _add_source_info(text, element.original_text, element.start_pos, None)
        return text
    return add_text_internal


_HANDLERS = [
    _add_document,                              # DOCUMENT
    _add_element,                               # ELEMENT
    _add_text(sigil_bs4.element.NavigableString),     # TEXT
    _add_text(sigil_bs4.element.CData),               # CDATA
    _add_text(sigil_bs4.element.Comment),             # COMMENT
    _add_text(sigil_bs4.element.NavigableString),     # WHITESPACE
    _add_element,                               # TEMPLATE
    ]


def _add_node(soup, node):
    return _HANDLERS[node.type.value](soup, node.contents)


def _add_next_prev_pointers(soup):
    def _traverse(node):
        # .findAll requires the .next pointer, which is what we're trying to add
        # when we call this, and so we manually supply a generator to yield the
        # nodes in DOM order.
        yield node
        try:
            for child in node.contents:
                for descendant in _traverse(child):
                    yield descendant
        except AttributeError:
            # Not an element.
            return
    nodes = sorted(_traverse(soup), key=lambda node: node.offset)
    if nodes:
        nodes[0].previous_element = None
        nodes[-1].next_element = None
    for i, node in enumerate(nodes[1:-1], 1):
        nodes[i-1].next_element = node
        node.previous_element = nodes[i-1]


# The only input encoding that gumbo supports is utf-8
# Any full unicode passed to the parser will be utf-8 encoded first
# Also gumbo is an html5 parser and does not handle xml header declarations
# so any xml header declaration will be stripped out before parsing
# and added back when serialized or prettyprinted
def parse(text, **kwargs):
    with gumboc.parse(text, **kwargs) as output:
        soup = sigil_bs4.BeautifulSoup('', "html.parser")
        _add_document(soup, output.contents.document.contents)
        for node in output.contents.document.contents.children:
            soup.append(_add_node(soup, node))
        _add_next_prev_pointers(soup.html)
        return soup


def main():
    samp = """
<?xml versions="1.0" encoding="UTF-8" ?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN"
  "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
<html xmlns="http://www.w3.org/1999/xhtml/" xml:lang="en" lang="en-US">
<head><title>testing & entities</title></head>
<body>
  <p class="first second">this&nbsp;is&#160;the&#xa0;<i><b>copyright</i></b> symbol "&copy;"</p>
  <p xmlns:xlink="http://www.w3.org/xlink" class="second" xlink:href="http://www.ggogle.com">this used to test atribute namespaces</p>
</body>
</html>
"""
    print("Parsing: \n")
    soup = parse(samp)
    print(soup.prettyprint_xhtml())
    print("\n")
    print("Find contents of tag head:")
    for node in soup.findAll("head"):
        print(node)
    print("\n")
    print("Findall tags with class attribute 'second':")
    for node in soup.find_all(attrs={'class':'second'}):
        print(node)
    return 0

if __name__ == '__main__':
    sys.exit(main())
