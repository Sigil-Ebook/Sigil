#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vim:ts=4:sw=4:softtabstop=4:smarttab:expandtab

# Copyright (c) 2014 Kevin B. Hendricks, John Schember, and Doug Massay
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

from __future__ import unicode_literals, division, absolute_import, print_function
import sys
import codecs
import unicodedata
try:
    from urllib.parse import unquote
except ImportError:
    from urllib import unquote


PY2 = sys.version_info[0] == 2
PY3 = sys.version_info[0] == 3

if PY2:
    text_type = unicode
    binary_type = str
    def bchr(s):
        return chr(s)
    def bord(s):
        return ord(s)
else:
    text_type = str
    binary_type = bytes
    def bchr(s):
        return bytes([s])
    def bord(s):
        return s

iswindows = sys.platform.startswith('win')

# convert string to be utf-8 encoded
def utf8_str(p, enc='utf-8'):
    if p is None:
        return None
    if isinstance(p, text_type):
        return p.encode('utf-8')
    if enc != 'utf-8':
        return p.decode(enc).encode('utf-8')
    return p

# convert string to be unicode encoded
def unicode_str(p, enc='utf-8'):
    if p is None:
        return None
    if isinstance(p, text_type):
        return p
    return p.decode(enc)

ASCII_CHARS   = set(chr(x) for x in range(128))
URL_SAFE      = set('ABCDEFGHIJKLMNOPQRSTUVWXYZ'
                    'abcdefghijklmnopqrstuvwxyz'
                    '0123456789' '#' '_.-/~')
IRI_UNSAFE = ASCII_CHARS - URL_SAFE

# returns a quoted IRI (not a URI)
def quoteurl(href):
    if isinstance(href,binary_type):
        href = href.decode('utf-8')
    result = []
    for char in href:
        if char in IRI_UNSAFE:
            char = "%%%02x" % ord(char)
        result.append(char)
    return ''.join(result)

# unquotes url/iri
def unquoteurl(href):
    if isinstance(href,binary_type):
        href = href.decode('utf-8')
    href = unquote(href)
    return href

# get sys.argv arguments and properly encode them as unicode
def unicode_argv():
    global iswindows
    global PY3
    if PY3:
        return sys.argv
    if iswindows:
        # Versions 2.x of Python don't support Unicode in sys.argv on
        # Windows, with the underlying Windows API instead replacing multi-byte
        # characters with '?'.  So use shell32.GetCommandLineArgvW to get sys.argv
        # as a list of Unicode strings
        from ctypes import POINTER, byref, cdll, c_int, windll
        from ctypes.wintypes import LPCWSTR, LPWSTR

        GetCommandLineW = cdll.kernel32.GetCommandLineW
        GetCommandLineW.argtypes = []
        GetCommandLineW.restype = LPCWSTR

        CommandLineToArgvW = windll.shell32.CommandLineToArgvW
        CommandLineToArgvW.argtypes = [LPCWSTR, POINTER(c_int)]
        CommandLineToArgvW.restype = POINTER(LPWSTR)

        cmd = GetCommandLineW()
        argc = c_int(0)
        argv = CommandLineToArgvW(cmd, byref(argc))
        if argc.value > 0:
            # Remove Python executable and commands if present
            start = argc.value - len(sys.argv)
            return [argv[i] for i in
                    range(start, argc.value)]
        # this should never happen
        return None
    else:
        argv = []
        argvencoding = sys.stdin.encoding
        if argvencoding is None:
            argvencoding = sys.getfilesystemencoding()
        if argvencoding is None:
            argvencoding = 'utf-8'
        for arg in sys.argv:
            if isinstance(arg, text_type):
                argv.append(arg)
            else:
                argv.append(arg.decode(argvencoding))
        return argv


# Python 2.X is broken in that it does not recognize CP65001 as UTF-8
def add_cp65001_codec():
    if PY2:
        try:
            codecs.lookup('cp65001')
        except LookupError:
            codecs.register(
                lambda name: name == 'cp65001' and codecs.lookup('utf-8') or None)
    return
