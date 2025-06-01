#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# vim:ts=4:sw=4:softtabstop=4:smarttab:expandtab

# Copyright (c) 2025 Kevin B. Hendricks, Stratford, and Doug Massay
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

import sys
import os
import json

from functionrep import *
import re

_EMPTY_FUNCTION = 'def replace(match, number, file_name, metadata, data):\n\tif match:\n\t\treturn match.group(0)'

_CaseChgFunctions = {
    "uppercase": "def replace(match, number, file_name, metadata, data):\n\tif match:\n\t\treturn replace_uppercase(match, number, file_name, metadata, data)\n",
            
    "lowercase": "def replace(match, number, file_name, metadata, data):\n\tif match:\n\t\treturn replace_lowercase(match, number, file_name, metadata, data)\n",

    "capitalize": "def replace(match, number, file_name, metadata, data):\n\tif match:\n\t\treturn replace_capitalize(match, number, file_name, metadata, data)\n",

    "titlecase": "def replace(match, number, file_name, metadata, data):\n\tif match:\n\t\treturn replace_titlecase(match, number, file_name, metadata, data)\n",

    "swapcase": "def replace(match, number, file_name, metadata, data):\n\tif match:\n\t\treturn replace_swapcase(match, number, file_name, metadata, data)\n",

    "uppercase_ignore_tags": "def replace(match, number, file_name, metadata, data):\n\tif match:\n\t\treturn replace_uppercase_ignore_tags(match, number, file_name, metadata, data)\n",

    "lowercase_ignore_tags": "def replace(match, number, file_name, metadata, data):\n\tif match:\n\t\treturn replace_lowercase_ignore_tags(match, number, file_name, metadata, data)\n",

    "capitalize_ignore_tags": "def replace(match, number, file_name, metadata, data):\n\tif match:\n\t\treturn replace_capitalize_ignore_tags(match, number, file_name, metadata, data)\n",

    "titlecase_ignore_tags": "def replace(match, number, file_name, metadata, data):\n\tif match:\n\t\treturn replace_titlecase_ignore_tags(match, number, file_name, metadata, data)\n",

    "swapcase_ignore_tags": "def replace(match, number, file_name, metadata, data):\n\tif match:\n\t\treturn replace_swapcase_ignore_tags(match, number, file_name, metadata, data)\n"
}

def read_json(jsonpath):
    d = {}
    if os.path.exists(jsonpath):
        with open(jsonpath, 'r', encoding='utf-8') as f:
            try:
                d = json.load(f)
            except SystemError:
                pass
            except Exception as e:
                print(e)
                pass
    return d


def createJsonFile(jsonpath):
    d = {}
    d.update(_CaseChgFunctions)
    if not os.path.exists(jsonpath):
        with open(jsonpath, 'w', encoding='utf-8') as f:
            json.dump(d, f, indent=2, ensure_ascii=False)
        return 1
    return 0


class SigilMatch(object):

    def __init__(self, text, groupslist):
        self.string = text
        self.groupslist = groupslist
        self.beg, self.end = groupslist[0]

    def start(self):
        return self.beg

    def end(self):
        return self.end

    def span(self, i):
        if i < 0 or i >= len(self.groupslist):
            raise IndexError("match group index not valid");
        return self.groupslist[i][0], self.groupslist[i][1]

    def group(self, i=0):
        if i < 0 or i >= len(self.groupslist):
            raise IndexError("match group index not valid");
        return self.string[self.groupslist[i][0] : self.groupslist[i][1]]

    def groups(self):
        results = []
        for i, v in enumerate(self.groupslist):
            if i > 0:
                results.append(self.string[v[0]: v[1]])
        return results 


class FunctionSearch(object):

    def __init__(self, metadataxml, function_name, repfuncs):
        global replace_lowercase
        global replace_uppercase
        global replace_capitalize
        global replace_titlecase
        global replace_swapcase
        global replace_lowercase_ignore_tags
        global replace_uppercase_ignore_tags
        global replace_capitalize_ignore_tags
        global replace_titlecase_ignore_tags
        global replace_swapcase_ignore_tags
        global replace_debug_log

        self.metadataxml = metadataxml
        self.function_name = function_name
        self.repfuncs = repfuncs
        self.number = 0
        self.funcData = {}
        self.bookpath =''
        self.replace = None
        if function_name in self.repfuncs:
            self.func = self.repfuncs[function_name]
        else:
            self.func = _EMPTY_FUNCTION
        self.replaceDict = globals()
        exec(self.func, self.replaceDict)
        if 'replace' in self.replaceDict:
            self.replace=self.replaceDict['replace']
            if not hasattr(self.replace, '__call__'):
                self.replace = None

    def do_text_replacements(self, pattern, bookpath, text):
        self.bookpath = bookpath

        def repl(match):
            if match:
                self.number += 1
                result=match.group(0)
                if self.replace is None:
                    return result
            else:
                result=''
            try:
                result=self.replace(match,self.number,self.bookpath,self.metadataxml,self.funcData)
            except Exception as e:
                replace_debug_log(str(e))
                print(e)
            return result

        newtext = re.sub(pattern, repl, text, flags=re.M) 
        return newtext

    def get_current_replacement_count(self):
        return self.number

    def get_single_replacement_by_function(self, bookpath, text, capture_groups):
        self.bookpath = bookpath
        match = SigilMatch(text, capture_groups)
        if match.start() == -1:
            return ''
        self.number += 1
        result = match.group(0);
        try:
            result=self.replace(match,self.number,self.bookpath,self.metadataxml,self.funcData)
        except Exception as e:
            replace_debug_log(str(e))
            print(e)
        return result


def getFunctionSearchEnv(metadataxml, function_name, jsonpath=None):
    repfuncs = {}
    if jsonpath:
        repfuncs = read_json(jsonpath)
    if not repfuncs:
        repfuncs = _CaseChgFunctions
    return FunctionSearch(metadataxml,function_name, repfuncs)


def main():
    argv = sys.argv
    text = '''<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html>
<html xmlns="http://www.w3.org/1999/xhtml" xmlns:ops="http://www.idpf.org/2007/ops" xml:lang="en" xmlns:epub="http://www.idpf.org/2007/ops">
<head>
  <title>Contents</title>
  <link href="css/style.css" type="text/css" rel="stylesheet"/>
</head>
<body>

<h1 class="title">this is a line of text to titlecase</h1>

<div class="toc">

<div class="frontmatter">
<a href="Cover.xhtml">Cover</a><br/>
<a href="Title.xhtml">Title Page</a><br/>
<a href="Contents.xhtml">Contents</a><br/>
<a href="Illustrations.xhtml">List of Illustrations</a><br/>
</div>

<div class="chapter">
<a href="Chapter-01.xhtml">Chapter 1</a>: Down the Rabbit-Hole<br/>
<a href="Chapter-02.xhtml">Chapter 2</a>: The Pool of Tears<br/>
<a href="Chapter-03.xhtml">Chapter 3</a>: A Caucus-Race and a Long Tale<br/>
<a href="Chapter-04.xhtml">Chapter 4</a>: The Rabbit Sends in a Little Bill<br/>
<a href="Chapter-05.xhtml">Chapter 5</a>: Advice from a Caterpillar<br/>
<a href="Chapter-06.xhtml">Chapter 6</a>: Pig and Pepper<br/>
<a href="Chapter-07.xhtml">Chapter 7</a>: A Mad Tea-Party<br/>
<a href="Chapter-08.xhtml">Chapter 8</a>: The Queen&#8217;s Croquet-Ground<br/>
<a href="Chapter-09.xhtml">Chapter 9</a>: The Mock Turtle&#8217;s Story<br/>
<a href="Chapter-10.xhtml">Chapter 10</a>: The Lobster Quadrille<br/>
<a href="Chapter-11.xhtml">Chapter 11</a>: Who Stole the Tarts?<br/>
<a href="Chapter-12.xhtml">Chapter 12</a>: Alice&#8217;s Evidence<br/>
</div>

</div>

</body>
</html>
'''
    # apath = "/Users/kbhend/Library/Application Support/sigil-ebook/sigil/replace_functions.json"
    # createJsonFile(apath)
    
    metadataxml = "<metadata><dc:title>Hello</dc:title></metadata>"
    function_name = "titlecase"
    pattern = r'''<h1\s[^>]*>([^<>]*)</h1>'''
    bookpath = "OEBPS/Contents.xhtml"

    gfsenv = getFunctionSearchEnv(metadataxml, function_name, None)
    result = gfsenv.do_text_replacements(pattern, bookpath, text)
    print(result)

    text = "<h1 class=\"title\">this is a long line of text that is now even longer.</h1>"
    metadataxml = "<metadata><dc:title>Hello</dc:title></metadata>"
    function_name = "titlecase_ignore_tags"
    nenv = getFunctionSearchEnv(metadataxml, function_name, None)
    capture_groups = [(0,75)]
    result = nenv.get_single_replacement_by_function(text, capture_groups)
    print(result)
    return 0



if __name__ == '__main__':
    sys.exit(main())
