#!/usr/bin/env python
# -*- coding: utf-8 -*-

# When using bytestrings in Python 2,  Windows requires full unicode 
# filenames and paths.  Therefore any bytestring paths *must* be utf-8 
# encoded as they will need to be converted on the fly to full unicode 
# for Windows platforms.  
# 
# Both Linunx and Mac OS X platforms will happily support utf-8 encoded paths
#
# These are simple support routines to ease use of utf-8 encoded bytestrings 
# as paths in main program to be converted on the fly to full unicode as 
# temporary un-named values to prevent the potential for inadvertent mixing 
# of unicode and bytestring and auto promotion issues elsewhere in the main 
# program 
#
# These include routines for path manipulation and encoding and decoding uri/iri

import sys, os
import locale
import codecs
from utf8_utils import utf8_str
from urllib import unquote
import path
import unicodedata

_iswindows = sys.platform.startswith('win')

# UNIBYTE_CHARS = set(chr(x) for x in xrange(256))
ASCII_CHARS   = set(chr(x) for x in xrange(128))
URL_SAFE      = set('ABCDEFGHIJKLMNOPQRSTUVWXYZ'
                    'abcdefghijklmnopqrstuvwxyz'
                    '0123456789' '_.-/~')
IRI_UNSAFE = ASCII_CHARS - URL_SAFE

# returns a utf-8 encoded quoted IRI (not a URI)
def quoteurl(href):
    href = utf8_str(href)
    result = []
    for char in href:
        if char in IRI_UNSAFE:
            char = "%%%02x" % ord(char)
        result.append(char)
    return ''.join(result)

# unquotes url to create a utf-8 encoded string
def unquoteurl(href):
    href = utf8_str(href)
    href = unquote(href)
    return href


# convert utf-8 encoded path string to proper type
# on windows that is full unicode
# on macosx and linux this is utf-8
def pathof(s):
    if s is None:
        return None
    if isinstance(s, unicode):
        print "Warning: pathof expects utf-8 encoded byestring: ", s
        if _iswindows:
            return s
        return s.encode('utf-8')
    if _iswindows:
        return s.decode('utf-8')
    return s

def exists(s):
    return os.path.exists(pathof(s))

def isfile(s):
    return os.path.isfile(pathof(s))

def isdir(s):
    return os.path.isdir(pathof(s))

def mkdir(s):
    return os.mkdir(pathof(s))

def listdir(s):
    rv = []
    for file in os.listdir(pathof(s)):
        rv.append(utf8_str(file, enc=sys.getfilesystemencoding()))
    return rv

def getcwd():
    return utf8_str(os.getcwdu())

def walk(top):
    toppath = top
    rv = []
    for base, dnames, names  in os.walk(pathof(top)):
        base = utf8_str(base, enc=sys.getfilesystemencoding())
        for name in names:
            name = utf8_str(name, enc=sys.getfilesystemencoding())
            filepath = relpath(os.path.join(base,name), toppath)
            rv.append(filepath)
    return rv

def relpath(path, start=None):
    rpath = os.path.relpath(utf8_str(path),utf8_str(start))
    return rpath

def abspath(path):
    return utf8_str(os.path.abspath(pathof(path)))
