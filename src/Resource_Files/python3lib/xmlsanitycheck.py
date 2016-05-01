#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vim:ts=4:sw=4:softtabstop=4:smarttab:expandtab

from __future__ import unicode_literals, division, absolute_import, print_function

import sys
import os
import re

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

# Map mediatype to package/module
# Maps to a tuple representing the file & classname
XML_TYPE_MAP ={}
# Seperate checkers of epub2|3 may not be necessary
XML_TYPE_MAP['opf2'] = ('opf2check', 'OPFCheck')
XML_TYPE_MAP['opf3'] = ('opf3check', 'OPFCheck')
XML_TYPE_MAP['ncx'] = ('ncxcheck', 'NCXCheck')
# XML_TYPE_MAP['adobepagemap'] = 'adobemap_check'
# XML_TYPE_MAP['smil'] = 'smil_check'

# Ampersand not part of any entity
AMP_PATTERN = re.compile(r'&(?!#?[a-zA-Z0-9]*;)')
# Entities of all kinds
ENTITY_PATTERN = re.compile(r'&[^\s;]*;')
# Valid xml tagname
TAG_PATTERN = re.compile(r'[a-zA-Z_][a-zA-Z0-9-_.:]+$')


class XMLSanityCheck(object):

    def __init__(self, data, codec='utf-8', media_type=None):
        if data is None:
            data = ''
        if isinstance(data, binary_type):
            data = data.decode(codec)
        self.content = data
        self.clen = len(self.content)
        self.media_type = media_type

        # Import specific self.media_type-based checker
        self.checker_obj = None
        if self.media_type is not None and self.media_type in XML_TYPE_MAP.keys():
            package = XML_TYPE_MAP[self.media_type][0]
            name = XML_TYPE_MAP[self.media_type][1]
            # Equivalent to -- from <package> import <name> as self.checker_obj
            # i.e. from opfchecker import OPFCheck as self.checker_obj
            self.checker_obj = getattr(__import__(package, fromlist=[name]), name)

        # parser position information
        self.pos = 0
        self.pp = 0
        self.line = 1
        self.col = 0
        self.tag_start = (-1,-1)

        # Previous tag's text
        self.last_text = None

        # to track tag nesting
        self.tagpath = []
        self.tagpositions = []

        # error reporting
        self.has_error = False
        self.errors = []

    def check(self):
        # persistent counter variables initialized
        # only import once per xmlsanitycheck.check()
        if self.checker_obj is not None:
            chk = self.checker_obj()
        for text, tp, tname, ttype, tattr in self.parse_iter():
            if self.has_error:
                break
            # pass the yielded data (and self) to the media-specifc xml checker's "check" method
            if self.checker_obj is not None:
                chk.check(self, text, tp, tname, ttype, tattr)
                if self.has_error:
                    break
        # Check to see if the mandatory_sections list is satisfied
        if self.checker_obj is not None and not self.has_error:
            if chk.mandatory_sections:
                self.errors.append((1, 0, 'Missing mandatory %s section' % chk.mandatory_sections[0]))
            self.has_error = True
        return (self.has_error, self.errors)

    def illegal_ampersand(self, s):
        m = AMP_PATTERN.search(s)
        if m:
            return True
        return False

    def valid_tagname(self, s):
        m = TAG_PATTERN.match(s)
        if m is not None:
            return True
        return False

    def illegal_entities(self, s):
        valid_entities = ['&amp;', '&quot;', '&apos;', '&lt;', '&gt;']
        m = ENTITY_PATTERN.search(s)
        if m:
            if m.group(0) not in valid_entities:
                return m.group(0)
        return None

    # parses string version of tag to identify its name,
    # its type 'begin', 'end' or 'single',
    # plus build a hashtable of its attributes
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
            if s[p+1:p+2] == ' ':
                error_msg = 'Unexpected whitespace following: "' + s[p:p+1] + '"'
                self.errors.append((self.tag_start[0], self.tag_start[1], error_msg))
                self.has_error = True
                return None, None, None
            ttype = 'end'
            p += 1
            while s[p:p+1] == ' ' : p += 1
        b = p
        # handle comment special case as there may be no spaces to delimit name begin or end
        if s[b:b+3] == "!--":
            p = b+3
            tname = "!--"
            ttype, backstep = SPECIAL_HANDLING_TAGS[tname]
            tattr['special'] = s[p:backstep]
            return tname, ttype, tattr
        while s[p:p+1] not in ('>', '/', ' ', '"', "'", "\r", "\n") :
            p += 1
            if (p - b) > MAX_TAG_LEN or p >= taglen:
                error_msg = 'Tag name not properly delimited: "' + s[b:p] + '"'
                self.errors.append((self.tag_start[0], self.tag_start[1], error_msg))
                self.has_error = True
                return None, None, None
        tname=s[b:p]
        if tname.lower() == '!doctype':
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
                if aname.find(' ') >= 0:
                    error_msg = 'Attribute "' + aname + '" contains unexpected whitespace'
                    self.errors.append((self.tag_start[0], self.tag_start[1], error_msg))
                    self.has_error = True
                    return None, None, None
                p += 1
                while s[p:p+1] == ' ' : p += 1
                if s[p:p+1] in ('"', "'") :
                    qt = s[p:p+1]
                    p = p + 1
                    b = p
                    while s[p:p+1] != qt:
                        p += 1
                        # Opening quote with no closing quote
                        if s[p:p+1] == '=' or p >= taglen:  # Don't cross the next '=' boundary when seeking matching quote
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
                        # Closing quote with no opening quote
                        if s[p:p+1] in ('"', "'"):
                            error_msg = 'Attribute "' +  aname +  '" has unmatched quotes on attribute value'
                            self.errors.append((self.tag_start[0], self.tag_start[1], error_msg))
                            self.has_error = True
                            return None, None, None
                        if p >= taglen:
                            error_msg = 'Attribute "' + aname + '" has unterminated attribute value'
                            self.errors.append((self.tag_start[0], self.tag_start[1], error_msg))
                            self.has_error = True
                            return None, None, None
                    val = s[b:p]
                # Check for ampersands not part of an entity
                if self.illegal_ampersand(val):
                    error_msg = 'Attribute "' + aname + '" contains an illegal "&" character'
                    self.errors.append((self.tag_start[0], self.tag_start[1], error_msg))
                    self.has_error = True
                # Check for illegal entities
                bad_entity = self.illegal_entities(val)
                if bad_entity is not None:
                    error_msg = 'Attribute "' + aname + '" contains an illegal "' + bad_entity + '" entity in its value'
                    self.errors.append((self.tag_start[0], self.tag_start[1], error_msg))
                    self.has_error = True
                    return None, None, None
                tattr[aname] = val

        # label beginning and single tags
        if ttype is None:
            ttype = 'begin'
            if s.find('/',p) >= 0:
                ttype = 'single'
        return tname, ttype, tattr

    # parse leading text of xml and tag
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
            tb = p
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
        error_msg = None
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
                bad_entity = self.illegal_entities(text)
                if self.illegal_ampersand(text) or bad_entity is not None:
                    error_msg = 'Text contains an illegal "&" character'
                    if bad_entity is not None:
                        error_msg = 'Text contains an illegal "' + bad_entity + '" entity'
                    self.errors.append((self.tag_start[0], self.tag_start[1], error_msg))
                    self.has_error = True

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

                if tag.startswith('< '):
                    error_msg = 'Unexpected whitespace following "<"'
                if tag.endswith(' >'):
                    error_msg = 'Unexpected whitespace preceeding ">"'
                if error_msg is not None:
                    self.errors.append((self.tag_start[0], self.tag_start[1], error_msg))
                    self.has_error = True
                    break

                text = None
                tname, ttype, tattr = self.parsetag(tag)

                if self.has_error:
                    break

                if tname not in SPECIAL_HANDLING_TAGS.keys() and not self.valid_tagname(tname):
                    error_msg = 'Invalid xml tagname - "' + tname
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

            self.last_text = text
            if ttype is not None:
                if ttype == 'begin':
                    self.tagpath.append(tname)
                    self.tagpositions.append(self.tag_start)
                elif ttype == 'end':
                    self.tagpath.pop()
                    self.tagpositions.pop()


def perform_sanity_check(apath, media_type=None):
    sep = chr(31)
    data = ''
    reslst = []
    filename = os.path.split(apath)[1]
    with open(apath,'rb') as f:
        data = f.read()
    p = XMLSanityCheck(data, media_type=media_type)
    has_error, errlist = p.check()
    if has_error:
        for line, col, msg in errlist:
            msg += '.  near column %d' % col
            res = "%s%s%s%s%d%s%s" % ('error', sep, filename, sep, line, sep, msg)
            reslst.append(res)
    return reslst


def main():
    samp1 = '''<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE ncx PUBLIC "-//NISO//DTD ncx 2005-1//EN"
   "http://www.daisy.org/z3986/2005/ncx-2005-1.dtd">
<ncx xmlns="http://www.daisy.org/z3986/2005/ncx/" version="2005-1">
<head>
   <meta name="dtb:uid" content="urn:uuid:11f66712-1bef-4bad-9d1e-afd8030bf3f2" />
   <meta name="dtb:depth" content="0" />
   <meta name="dtb:totalPageCount" content="0" />
   <meta name="dtb:maxPageNumber" content="0"/>
</head>
<docTitle>
   <text>Unknown</text>
</docTitle>
<navMap>
<navPoint id="navPoint-1" playOrder="1">
  <navLabel>
    <text>Start</text>
  </navLabel>
  <content src="Text/Section0001.xhtml" />
</navPoint>
</navMap>
</ncx>
'''

    samp2 = '''<?xml version="1.0" encoding="utf-8"?>
<package version="2.0" unique-identifier="BookId" xmlns="http://www.idpf.org/2007/opf">
  <metadata xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:opf="http://www.idpf.org/2007/opf">
    <dc:identifier id="BookId" opf:scheme="UUID">urn:uuid:11f66712-1bef-4bad-9d1e-afd8030bf3f2</dc:identifier>
    <dc:language>en</dc:language>
    <dc:title>[No data]</dc:title>
  </metadata>
  <manifest>
    <item  id="ncx" href="toc.ncx" media-type="application/x-dtbncx+xml"/>
    <item  id="Section0001.xhtml" href="Text/Section0001.xhtml" media-type="application/xhtml+xml"/>
  </manifest>
  <spine toc="ncx">
    <itemref  idref="Section0001.xhtml"/>
  </spine>
  <guide>
  </guide>
</package>
'''

    p = XMLSanityCheck(samp1, media_type="ncx")
    has_error, errlist = p.check()
    print(samp1)
    print(has_error)
    print(errlist)

    p = XMLSanityCheck(samp2, media_type="opf")
    has_error, errlist = p.check()
    print(samp2)
    print(has_error)
    print(errlist)

    return 0

if __name__ == '__main__':
    sys.exit(main())

