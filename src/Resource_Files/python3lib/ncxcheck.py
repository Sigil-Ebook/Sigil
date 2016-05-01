#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vim:ts=4:sw=4:softtabstop=4:smarttab:expandtab

'''
Imported by xmlsanitycheck.py

Must have a "check" method (7 parameters/no return value) and a "mandatory_sections"
property/variable that resolves to an empty list if there are no missing elements
(contenrs of "mandatory_sections" specify missing elements).
'''

from __future__ import unicode_literals, division, absolute_import, print_function

import re

VALID_TAGS = ['ncx', 'head', 'meta', 'docTitle', 'docAuthor', 'text', 'content', 'navMap',
              'navPoint', 'navLabel', 'pageList', 'pageTarget', 'navList', 'navTarget']
VOID_TAGS = ['content', 'meta']
SPECIAL_HANDLING_TAGS = ['?xml', '!DOCTYPE', '!--']
ALL_SPACES = re.compile(r'^\s*$', re.U)

def empty(last_text):
    if last_text is None:
        return True
    return (ALL_SPACES.match(last_text) is not None)

class NCXCheck(object):

    def __init__(self):
        self.mandatory_sections = ['ncx', 'head']
        self.ncxcnt = 0
        self.headcnt = 0
        self.xmldeclare = 0
        self.doctype = 0

    def check(self, sc, text, tp, tname, ttype, tattr):
        self.sc = sc        # xmlsanitychecker object
        self.text = text
        self.tp = tp
        self.tname = tname
        self.ttype = ttype
        self.tattr = tattr

        # Valid Tags
        if self.tname and self.tname not in SPECIAL_HANDLING_TAGS and self.tname not in VALID_TAGS:
            error_msg = 'tag: ' + self.tname + ' is not a valid NCX tag name.'
            self.sc.errors.append((self.sc.tag_start[0], self.sc.tag_start[1], error_msg))
            self.sc.has_error = True
            return

        # basic structure sanity check
        if self.tname == 'ncx' and self.ttype == 'begin':
            self.ncxcnt += 1
            self.mandatory_sections.remove(self.tname)
        if self.ncxcnt > 1:
            error_msg = 'Only one "ncx" section allowed in an NCX file'
            self.sc.errors.append((self.sc.tag_start[0], self.sc.tag_start[1], error_msg))
            self.sc.has_error = True
            return
        if self.tname == 'head' and self.ttype == 'begin':
            self.headcnt += 1
            self.mandatory_sections.remove(self.tname)
        if self.headcnt > 1:
            error_msg = 'Only one "head" section allowed in an NCX file'
            self.sc.errors.append((self.sc.tag_start[0], self.sc.tag_start[1], error_msg))
            self.sc.has_error = True
            return
        if self.tname == "?xml":
            self.xmldeclare += 1
            if self.ncxcnt > 0:
                error_msg = 'An xml declaration must come before the "ncx" tag'
                self.sc.errors.append((self.sc.tag_start[0], self.sc.tag_start[1], error_msg))
                self.sc.has_error = True
                return

        # Void tag check (closing tags are OK if last_text is None or spaces)
        if self.tname in VOID_TAGS and self.ttype == "end" and not empty(self.sc.last_text):
            error_msg = 'Void tag: ' + self.tname + ' has an illegal ending tag'
            self.sc.errors.append((self.sc.tag_start[0], self.sc.tag_start[1], error_msg))
            self.sc.has_error = True
            return
