#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# vim:ts=4:sw=4:softtabstop=4:smarttab:expandtab

# Copyright (c) 2014-2020 Kevin B. Hendricks, John Schember, and Doug Massay
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this list of
# conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice, this list
# of conditions and the following disclaimer in the documentation and/or other materials
# provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
# SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
# TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
# WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


# DEPRECATED:  This entire module is deprecated.  It is only kept to help
# maintain older plugins.  DO NOT USE for any new or revised code.


import sys
import html
import binascii
from urllib.parse import unquote, urlsplit

PY2 = sys.version_info[0] == 2
PY3 = sys.version_info[0] >= 3

iswindows = sys.platform.startswith('win')

text_type = str
binary_type = bytes


def bchr(s):
    return bytes([s])


def bstr(s):
    if isinstance(s, str):
        return bytes(s, 'latin-1')
    else:
        return bytes(s)


def bord(s):
    return s


def bchar(s):
    return bytes([s])


def lrange(*args, **kwargs):
    return list(range(*args, **kwargs))


def lzip(*args, **kwargs):
    return list(zip(*args, **kwargs))


def lmap(*args, **kwargs):
    return list(map(*args, **kwargs))


def lfilter(*args, **kwargs):
    return list(filter(*args, **kwargs))


def hexlify(bdata):
    return (binascii.hexlify(bdata)).decode('ascii')


def utf8_str(p, enc='utf-8'):
    if p is None:
        return None
    if isinstance(p, str):
        return p.encode('utf-8')
    if enc != 'utf-8':
        return p.decode(enc, errors='replace').encode('utf-8')
    return p


def unicode_str(p, enc='utf-8'):
    if p is None:
        return None
    if isinstance(p, str):
        return p
    return p.decode(enc, errors='replace')


_ASCII_CHARS   = set(chr(x) for x in range(128))
_URL_SAFE      = set('ABCDEFGHIJKLMNOPQRSTUVWXYZ'
                     'abcdefghijklmnopqrstuvwxyz'
                     '0123456789' '#' '_.-/~')
_IRI_UNSAFE = _ASCII_CHARS - _URL_SAFE


def quoteurl(href):
    if isinstance(href, binary_type):
        href = href.decode('utf-8')
    (ascheme, anetloc, apath, aquery, afragment) = urlsplit(href, scheme="", allow_fragments=True)
    if ascheme != "":
        ascheme += "://"
        href = href[len(ascheme):]
    result = []
    for char in href:
        if char in _IRI_UNSAFE:
            char = "%%%02x" % ord(char)
        result.append(char)
    return ascheme + ''.join(result)


def unquoteurl(href):
    if isinstance(href, binary_type):
        href = href.decode('utf-8')
    href = unquote(href)
    return href


def unescapeit(sval):
    return html.unescape(sval)


def unicode_argv():
    return sys.argv


def add_cp65001_codec():
    return
