#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vim:ts=4:sw=4:softtabstop=4:smarttab:expandtab

from __future__ import unicode_literals, division, absolute_import, print_function

import sys
import os

PY3 = sys.version_info[0] == 3

if PY3:
    binary_type = bytes
else:
    binary_type = str

SPECIAL_HANDLING_TAGS = {
    '?xml'     : ('xmlheader', -1), 
    '!--'      : ('comment', -3),
    '!DOCTYPE' : ('doctype', -1),
}

SPECIAL_HANDLING_TYPES = ['xmlheader', 'doctype', 'comment']

MAX_TAG_LEN = 20

class SanityCheck(object):

    def __init__(self, data, codec = 'utf-8'):
        if data is None:
            data = ''
        if isinstance(data, binary_type):
            data = data.decode(codec)
        self.content = data
        self.clen = len(self.content)

        # parser position information
        self.pos = 0
        self.pp = 0
        self.line = 1
        self.col = 0
        self.tag_start = (-1,-1)

        # for basic structure sanity check
        self.htmlcnt = 0
        self.bodycnt = 0
        self.headcnt = 0
        self.xmldeclare = 0
        self.doctype = 0

        # to track tag nesting
        self.tagpath = []
        self.tagpositions = []

        # error reporting
        self.has_error = False
        self.errors = []


    def check(self):
        for text, tp, tname, ttype, tattr in self.parse_iter():
            if self.has_error:
                break
        if not self.has_error:
            if self.htmlcnt != 1:
                self.errors.append((1, 0, 'Missing or multiple "html" tags'))
                self.has_error = True
            if self.bodycnt != 1: 
                self.errors.append((1, 0, 'Missing or multiple "body" tags'))
                self.has_error = True
            if self.headcnt != 1: 
                self.errors.append((1, 0, 'Missing or multiple "head" tags'))
                self.has_error = True
        return (self.has_error, self.errors)

    # parses string version of tag to identify its name,
    # its type 'begin', 'end' or 'single',
    # plus build a hashtable of its atributes
    def parsetag(self, s):
        if self.has_error:
            return None, None, None
        taglen = len(s)
        p = 1
        # get the tag name
        tname = None
        ttype = None
        tattr = {}
        while s[p:p+1] == ' ' : p += 1
        if s[p:p+1] == '/':
            ttype = 'end'
            p += 1
            while s[p:p+1] == ' ' : p += 1
        b = p
        while s[p:p+1] not in ('>', '/', ' ', '"', "'", "\r", "\n") : 
            p += 1
            if (p - b) > MAX_TAG_LEN or p >= taglen:
                error_msg = 'Tag name not properly delimited: "' + self.s[b:p] + '"'
                self.errors.append((self.tag_start[0], self.tag_start[1], error_msg))
                self.has_error = True
                return None, None, None
        tname=s[b:p].lower()
        if tname == '!doctype':
            tname = '!DOCTYPE'
        # special cases
        if tname in SPECIAL_HANDLING_TAGS:
            ttype, backstep = SPECIAL_HANDLING_TAGS[tname]
            tattr['special'] = s[p:backstep]
        if ttype is None:
            # parse any attributes
            while s.find('=',p) != -1 :
                while s[p:p+1] == ' ' : p += 1
                b = p
                while s[p:p+1] != '=' : p += 1
                aname = s[b:p].lower()
                aname = aname.rstrip(' ')
                p += 1
                while s[p:p+1] == ' ' : p += 1
                if s[p:p+1] in ('"', "'") :
                    qt = s[p:p+1]
                    p = p + 1
                    b = p
                    while s[p:p+1] != qt: 
                        p += 1
                        if p >= taglen:
                            error_msg = 'Attribute "' +  aname +  '" has unmatched quotes on attribute value'
                            self.errors.append((self.tag_start[0], self.tag_start[1], error_msg))
                            self.has_error = True
                            return None, None, None
                    val = s[b:p]
                    p += 1
                else :
                    b = p
                    while s[p:p+1] not in ('>', '/', ' ') : 
                        p += 1
                        if p >= taglen:
                            error_msg = 'Attribute "' + aname + '" has unterminated attribute value' 
                            self.errors.append((self.tag_start[0], self.tag_start[1], error_msg))
                            self.has_error = True
                            return None, None, None
                    val = s[b:p]
                tattr[aname] = val
        # label beginning and single tags
        if ttype is None:
            ttype = 'begin'
            if s.find('/',p) >= 0:
                ttype = 'single'
        return tname, ttype, tattr

    # parse leading text of xhtml and tag
    # returns as tuple (Leading Text, Tag)
    # only one will have a value, the other will always be None
    def parseml(self):
        if self.has_error:
            return None, None
        self.pp = self.pos
        p = self.pos
        if p >= self.clen:
            return None, None
        if self.content[p] != '<':
            res = self.content.find('<',p)
            if res == -1 :
                res = len(self.content)
            self.pos = res
            return self.content[p:res], None
        # handle comment as a special case to deal with multi-line comments
        if self.content[p:p+4] == '<!--':
            te = self.content.find('-->',p+1)
            if te != -1:
                te = te+2
        else :
            tb = p
            te = self.content.find('>',p+1)
            ntb = self.content.find('<',p+1)
            if ntb != -1 and ntb < te:
                self.pos = ntb
                return self.content[p:ntb], None
        self.pos = te + 1
        return None, self.content[p:te+1]



    # yields leading text, tagpath prefix, tag name, tag type, tag attributes
    # tag prefix is a dotted history of all open parent ("begin') tags
    # tag types are "single", "begin", "end", "comment", "xmlheader", and "doctype"
    # tag attributes is a dictionary of key and value pairs
    def parse_iter(self):
        while True:
            text, tag = self.parseml()
            if self.has_error or text is None and tag is None:
                break
            tp = ".".join(self.tagpath)
            if text is not None:
                tname = ttype = tattr = None
                # walk the text and keep track of  line and col info
                for c in text:
                    self.col += 1
                    if c == '\n':
                        self.line += 1
                        self.col = 0

            if tag is not None:
                # walk the text of the tag to keep track of line/col info
                line = self.line
                col = self.col
                self.tag_start = (line, col)
                for c in self.content[self.pp: self.pos]:
                    self.col += 1
                    if c == '\n':
                        self.line += 1
                        self.col = 0

                text = None
                tname, ttype, tattr = self.parsetag(tag)

                if self.has_error:
                    break

                # basic structure sanity check
                if tname == "html" and ttype =='begin':
                    self.htmlcnt += 1
                    if self.bodycnt > 0:
                        error_msg = 'Tag "html" found after "body"'
                        self.errors.append((self.tag_start[0], self.tag_start[1], error_msg))
                        self.has_error = True
                        break
                if tname == "body" and ttype =='begin':
                    self.bodycnt += 1
                    if self.htmlcnt == 0:
                        error_msg = 'Tag "body" found before "html"'
                        self.errors.append((self.tag_start[0], self.tag_start[1], error_msg))
                        self.has_error = True
                        break
                if tname == "head" and ttype =='begin':
                    self.headcnt += 1
                    if self.bodycnt > 0:
                        error_msg = 'Tag "head" found after "body"'
                        self.errors.append((self.tag_start[0], self.tag_start[1], error_msg))
                        self.has_error = True
                        break
                if tname == "p" and ttype =='begin':
                    if "p" in self.tagpath:
                        error_msg = 'Can not nest a "p" tag inside another "p" tag'
                        self.errors.append((self.tag_start[0], self.tag_start[1], error_msg))
                        self.has_error = True
                        break
                if tname == "?xml":
                    self.xmldeclare += 1
                    if self.htmlcnt > 0 or self.doctype > 0:
                        error_msg = 'An xml declaration must come before the "html" tag and DOCTYPE'
                        self.errors.append((self.tag_start[0], self.tag_start[1], error_msg))
                        self.has_error = True
                        break
                if tname == "!DOCTYPE":
                    self.doctype += 1
                    if self.htmlcnt > 0:
                        error_msg = 'A DOCTYPE must come before the "html" tag'
                        self.errors.append((self.tag_start[0], self.tag_start[1], error_msg))
                        self.has_error = True
                        break

                # validate tag nesting
                if ttype == 'end':
                    last_begin = self.tagpath[-1]
                    last_pos = self.tagpositions[-1]
                    if last_begin != tname:
                        error_msg = 'Improperly nested tags: parsing end tag "' + tname + '" but current parse path is "' + tp + '". See line %d col %d' % last_pos
                        self.errors.append((self.tag_start[0], self.tag_start[1], error_msg))
                        self.has_error = True
                        break

            yield text, tp, tname, ttype, tattr

            if ttype is not None:
                if ttype == 'begin':
                    self.tagpath.append(tname)
                    self.tagpositions.append(self.tag_start)
                elif ttype == 'end':
                    self.tagpath.pop()
                    self.tagpositions.pop()


def perform_sanity_check(apath):
    sep = chr(31)
    data = ''
    reslst = []
    filename = os.path.split(apath)[1]
    with open(apath,'rb') as f:
        data = f.read()
    p = SanityCheck(data)
    has_error, errlist = p.check()
    if has_error:
        for line, col, msg in errlist:
            msg += '.  near column %d' % col
            res = "%s%s%s%s%d%s%s" % ('error', sep, filename, sep, line, sep, msg)
            reslst.append(res)
    return reslst
            

def main():
    samp1 = '<html>\n<head><title>testing & entities</title></head>\n'
    samp1 += '<body>\n<p>this&nbsp;is&#160;the&#xa0;<i><b>copyright'
    samp1 += '</i></b> symbol "&copy;"</p>\n</body>\n</html>\n'

    samp2 = '''<html>
<head>
<title>testing & entities</title>
</head>
<body>
<p>this&nbsp;is&#160;the&#xa0;<b><i>copyright</i></b> symbol "&copy;"</p>
</html>
'''

    p = SanityCheck(samp1)
    has_error, errlist = p.check()
    print(samp1)
    print(has_error)
    print(errlist)

    p = SanityCheck(samp2)
    has_error, errlist = p.check()
    print(samp2)
    print(has_error)
    print(errlist)

    return 0

if __name__ == '__main__':
    sys.exit(main())

