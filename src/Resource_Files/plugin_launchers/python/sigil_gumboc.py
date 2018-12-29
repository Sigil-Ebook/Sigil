#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vim:ts=4:sw=4:softtabstop=4:smarttab:expandtab

from __future__ import unicode_literals, print_function

# Copyright 2015 Kevin B. Hendricks Stratford Ontario Canada
# Copyright 2012 Google Inc. All Rights Reserved.
#
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

import sys
import re

PY3 = sys.version_info[0] >= 3

if PY3:
    text_type = str
    binary_type = bytes
else:
    text_type = unicode
    binary_type = str

def _remove_xml_header(data):
    return re.sub(br'<\s*\?xml\s*[^\?>]*\?*>\s*',b'',data, flags=re.I)

# When supporting both python 2 and 3 using one code base, using str(obj) is confusing
# at best since its return type is python version specific
# Notes:
#   - The unicode(obj) operator does not exist in PY3
#   - The bytes(obj) exists and works on python >= 2.6 (as it aliased to str in python 2.X)

"""CTypes bindings for the Gumbo HTML5 parser.

This exports the raw interface of the library as a set of very thin ctypes
wrappers.  It's intended to be wrapped by other libraries to provide a more
Pythonic API.
"""

__author__ = 'jdtang@google.com (Jonathan Tang)'

import contextlib

import ctypes
from ctypes.util import find_library

import os
import os.path
import sigil_gumboc_tags as gumboc_tags


_sigilgumbolibpath = None

if 'SigilGumboLibPath' in os.environ:
    _sigilgumbolibpath = os.environ['SigilGumboLibPath']

if _sigilgumbolibpath is not None:
    try:
        _dll = ctypes.cdll.LoadLibrary(_sigilgumbolibpath)
    except OSError:
        _dll = ctypes.cdll.LoadLibrary(find_library('sigilgumbo'))
        pass
else:
    _dll = ctypes.cdll.LoadLibrary(find_library('sigilgumbo'))

# Some aliases for common types.
_bitvector = ctypes.c_uint
_Ptr = ctypes.POINTER

class EnumMetaclass(type(ctypes.c_uint)):
    def __new__(metaclass, name, bases, cls_dict):
        cls = type(ctypes.c_uint).__new__(metaclass, name, bases, cls_dict)
        if name == 'Enum':
            return cls
        try:
            for i, value in enumerate(cls_dict['_values_']):
                setattr(cls, value, cls.from_param(i))
        except KeyError:
            raise ValueError('No _values_ list found inside enum type.')
        except TypeError:
            raise ValueError('_values_ must be a list of names of enum constants.')
        return cls

def with_metaclass(mcls):
    def decorator(cls):
        body = vars(cls).copy()
        # clean out class body
        body.pop('__dict__', None)
        body.pop('__weakref__', None)
        return mcls(cls.__name__, cls.__bases__, body)
    return decorator

@with_metaclass(EnumMetaclass)
class Enum(ctypes.c_uint):
    @classmethod
    def from_param(cls, param):
        if isinstance(param, Enum):
            if param.__class__ != cls:
                raise ValueError("Can't mix enums of different types")
            return param
        if param < 0 or param > len(cls._values_):
            raise ValueError('%d is out of range for enum type %s; max %d.' %
                             (param, cls.__name__, len(cls._values_)))
        return cls(param)

    def __eq__(self, other):
        return self.value == other.value

    def __ne__(self, other):
        return self.value != other.value

    def __hash__(self):
        return hash(self.value)

    def __repr__(self):
        try:
            return self._values_[self.value]
        except IndexError:
            raise IndexError('Value %d is out of range for %r' %
                             (self.value, self._values_))



class StringPiece(ctypes.Structure):
    _fields_ = [
        ('data', _Ptr(ctypes.c_char)),
        ('length', ctypes.c_size_t),
        ]

    def __len__(self):
        return self.length

    def __str__(self):
        # Warning: in Python 3 the str() operator method may **never** return bytes
        #  to write code that employs gumboc.py that will work underboth Python 2 and 3 use bytes() instead
        if PY3:
            return ctypes.string_at(self.data, self.length).decode('utf-8')
        return ctypes.string_at(self.data, self.length)

    def __bytes__(self):
        return ctypes.string_at(self.data, self.length)


class SourcePosition(ctypes.Structure):
    _fields_ = [
        ('line', ctypes.c_uint),
        ('column', ctypes.c_uint),
        ('offset', ctypes.c_uint)
        ]
SourcePosition.EMPTY = SourcePosition.in_dll(_dll, 'kGumboEmptySourcePosition')


class AttributeNamespace(Enum):
    URLS = [
        'http://www.w3.org/1999/xhtml',
        'http://www.w3.org/1999/xlink',
        'http://www.w3.org/XML/1998/namespace',
        'http://www.w3.org/2000/xmlns',
    ]
    _values_ = ['NONE', 'XLINK', 'XML', 'XMLNS']

    def to_url(self):
        return self.URLS[self.value]


class OutputStatus(Enum):
    STATUS_MSG = [
        'OK',
        'Document tree depth limit exceeded',
        'System allocator returned NULL during parsing',
     ]
    _values_ = ['STATUS_OK', 'STATUS_TREE_TOO_DEEP', 'STATUS_OUT_OF_MEMORY']

    def to_string(self):
        return self.STATUS_MSG[self.value]


class Attribute(ctypes.Structure):
    _fields_ = [
        ('namespace', AttributeNamespace),
        ('name', ctypes.c_char_p),
        ('original_name', StringPiece),
        ('value', ctypes.c_char_p),
        ('original_value', StringPiece),
        ('name_start', SourcePosition),
        ('name_end', SourcePosition),
        ('value_start', SourcePosition),
        ('value_end', SourcePosition)
        ]


class Vector(ctypes.Structure):
    _type_ = ctypes.c_void_p
    _fields_ = [
        ('data', _Ptr(ctypes.c_void_p)),
        ('length', ctypes.c_uint),
        ('capacity', ctypes.c_uint)
        ]

    class Iter(object):
        def __init__(self, vector):
            self.current = 0
            self.vector = vector

        def __iter__(self):
            return self

        def __next__(self):
            # Python 3
            if self.current >= self.vector.length:
                raise StopIteration
            obj = self.vector[self.current]
            self.current += 1
            return obj

        def next(self):
            # Python 2
            return self.__next__()

    def __len__(self):
        return self.length

    def __getitem__(self, i):
        try:
            # Python 2
            numeric_types = (int, long)
        except NameError:
            # Python 3
            numeric_types = int

        if isinstance(i, numeric_types):
            if i < 0:
                i += self.length
            if i > self.length:
                raise IndexError
            array_type = _Ptr(_Ptr(self._type_))
            return ctypes.cast(self.data, array_type)[i].contents
        return list(self)[i]

    def __iter__(self):
        return Vector.Iter(self)


Vector.EMPTY = Vector.in_dll(_dll, 'kGumboEmptyVector')


class AttributeVector(Vector):
    _type_ = Attribute


class NodeVector(Vector):
    # _type_ assigned later, to avoid circular references with Node
    pass


class QuirksMode(Enum):
    _values_ = ['NO_QUIRKS', 'QUIRKS', 'LIMITED_QUIRKS']


class Document(ctypes.Structure):
    _fields_ = [
        ('children', NodeVector),
        ('has_doctype', ctypes.c_bool),
        ('name', ctypes.c_char_p),
        ('public_identifier', ctypes.c_char_p),
        ('system_identifier', ctypes.c_char_p),
        ('doc_type_quirks_mode', QuirksMode),
        ]

    def __repr__(self):
        return 'Document'


class Namespace(Enum):
    URLS = [
        'http://www.w3.org/1999/xhtml',
        'http://www.w3.org/2000/svg',
        'http://www.w3.org/1998/Math/MathML',
    ]
    _values_ = ['HTML', 'SVG', 'MATHML']

    def to_url(self):
        return self.URLS[self.value]


class Tag(Enum):
    @staticmethod
    def from_str(tagname):
        text_ptr = ctypes.c_char_p(tagname.encode('utf-8'))
        return _tag_enum(text_ptr)

    _values_ = gumboc_tags.TagNames + ['UNKNOWN', 'LAST']

class Element(ctypes.Structure):
    _fields_ = [
        ('children', NodeVector),
        ('tag', Tag),
        ('tag_namespace', Namespace),
        ('original_tag', StringPiece),
        ('original_end_tag', StringPiece),
        ('start_pos', SourcePosition),
        ('end_pos', SourcePosition),
        ('attributes', AttributeVector),
        ]

    @property
    def tag_name(self):
        original_tag = StringPiece.from_buffer_copy(self.original_tag)
        _tag_from_original_text(ctypes.byref(original_tag))
        if self.tag_namespace == Namespace.SVG:
            svg_tagname = _normalize_svg_tagname(ctypes.byref(original_tag))
            if svg_tagname is not None:
                return bytes(svg_tagname)
        if self.tag == Tag.UNKNOWN:
            if original_tag.data is None:
                return ''
            return (bytes(original_tag).decode('utf-8').lower()).encode('utf-8')
        return _tagname(self.tag)

    def __repr__(self):
        return ('<%r>\n' % self.tag +
                '\n'.join(repr(child) for child in self.children) +
                '</%r>' % self.tag)


class Text(ctypes.Structure):
    _fields_ = [
        ('text', ctypes.c_char_p),
        ('original_text', StringPiece),
        ('start_pos', SourcePosition)
        ]

    def __repr__(self):
        return 'Text(%r)' % self.text


class NodeType(Enum):
    _values_ = ['DOCUMENT', 'ELEMENT', 'TEXT', 'CDATA',
                'COMMENT', 'WHITESPACE', 'TEMPLATE']


class NodeUnion(ctypes.Union):
    _fields_ = [
        ('document', Document),
        ('element', Element),
        ('text', Text),
        ]


class Node(ctypes.Structure):
    # _fields_ set later to avoid a circular reference

    def _contents(self):
        # Python3 enters an infinite loop if you use an @property within
        # __getattr__, so we factor it out to a helper.
        if self.type == NodeType.DOCUMENT:
            return self.v.document
        elif self.type in (NodeType.ELEMENT, NodeType.TEMPLATE):
            return self.v.element
        else:
            return self.v.text

    @property
    def contents(self):
        return self._contents()

    def __getattr__(self, name):
        return getattr(self._contents(), name)

    def __setattr__(self, name, value):
        return setattr(self._contents(), name, value)

    def __repr__(self):
        return repr(self.contents)


Node._fields_ = [
    ('type', NodeType),
    # Set the type to Node later to avoid a circular dependency.
    ('parent', _Ptr(Node)),
    ('index_within_parent', ctypes.c_uint),
    # TODO(jdtang): Make a real list of enum constants for this.
    ('parse_flags', _bitvector),
    ('v', NodeUnion)
    ]
NodeVector._type_ = Node


class Options(ctypes.Structure):
    _fields_ = [
        ('tab_stop', ctypes.c_int),
        ('use_xhtml_rules', ctypes.c_bool),
        ('stop_on_first_error', ctypes.c_bool),
        ('max_tree_depth', ctypes.c_uint),
        ('max_errors', ctypes.c_int),
        ]


class Output(ctypes.Structure):
    _fields_ = [
        ('document', _Ptr(Node)),
        ('root', _Ptr(Node)),
        ('status', OutputStatus),
        # TODO(jdtang): Error type.
        ('errors', Vector),
        ]


# Important Note: gumbo only supports the utf-8 encoding
# Also gumbo is an html5 parser and does not grok xml pi headers
@contextlib.contextmanager
def parse(text, **kwargs):
    options = Options()
    context_tag = kwargs.get('container', Tag.LAST)
    context_namespace = kwargs.get('container_namespace', Namespace.HTML)
    for field_name, _ in Options._fields_:
        try:
            setattr(options, field_name, kwargs[field_name])
        except KeyError:
            setattr(options, field_name, getattr(_DEFAULT_OPTIONS, field_name))
    # We have to manually take a reference to the input text here so that it
    # outlives the parse output.  If we let ctypes do it automatically on function
    # call, it creates a temporary buffer which is destroyed when the call
    # completes, and then the original_text pointers point into invalid memory.
    # convert string to be utf-8 encoded
    if isinstance(text, text_type):
        text = text.encode('utf-8')
    text = _remove_xml_header(text)
    text_ptr = ctypes.c_char_p(text)
    output = _parse_fragment(
        ctypes.byref(options), text_ptr, len(text),
        context_tag, context_namespace)
    try:
        yield output
    finally:
        _destroy_output(output)

_DEFAULT_OPTIONS = Options.in_dll(_dll, 'kGumboDefaultOptions')

_parse_with_options = _dll.gumbo_parse_with_options
_parse_with_options.argtypes = [_Ptr(Options), ctypes.c_char_p, ctypes.c_size_t]
_parse_with_options.restype = _Ptr(Output)

_parse_fragment = _dll.gumbo_parse_fragment
_parse_fragment.argtypes = [
    _Ptr(Options), ctypes.c_char_p, ctypes.c_size_t, Tag, Namespace]
_parse_fragment.restype = _Ptr(Output)

_tag_from_original_text = _dll.gumbo_tag_from_original_text
_tag_from_original_text.argtypes = [_Ptr(StringPiece)]
_tag_from_original_text.restype = None

_normalize_svg_tagname = _dll.gumbo_normalize_svg_tagname
_normalize_svg_tagname.argtypes = [_Ptr(StringPiece)]
_normalize_svg_tagname.restype = ctypes.c_char_p

_destroy_output = _dll.gumbo_destroy_output
_destroy_output.argtypes = [_Ptr(Output)]
_destroy_output.restype = None

_tagname = _dll.gumbo_normalized_tagname
_tagname.argtypes = [Tag]
_tagname.restype = ctypes.c_char_p

_tag_enum = _dll.gumbo_tag_enum
_tag_enum.argtypes = [ctypes.c_char_p]
_tag_enum.restype = Tag

__all__ = ['StringPiece', 'SourcePosition', 'AttributeNamespace', 'Attribute',
           'Vector', 'AttributeVector', 'NodeVector', 'QuirksMode', 'Document',
           'Namespace', 'Tag', 'Element', 'Text', 'NodeType', 'Node',
           'Options', 'Output', 'parse']
