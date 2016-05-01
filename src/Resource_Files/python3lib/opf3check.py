#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vim:ts=4:sw=4:softtabstop=4:smarttab:expandtab

'''
Imported by xmlsanitycheck.py

Must have a "check" method (no return value) and a "mandatory_sections"
property/variable that resolves to an empty list if there are no missing elements
(contents of "mandatory_sections" specify missing elements).
'''

from __future__ import unicode_literals, division, absolute_import, print_function

import re

VALID_TAGS = ['package', 'metadata', 'meta', 'manifest', 'item',
              'spine', 'itemref', 'tours', 'tour', 'site', 'guide', 'reference']
VOID_TAGS = ['item', 'itemref', 'site', 'reference']
SPECIAL_HANDLING_TAGS = ['?xml', '!--']
ALL_SPACES = re.compile(r'^\s*$', re.U)

def empty(last_text):
    if last_text is None:
        return True
    return (ALL_SPACES.match(last_text) is not None)

class OPFCheck(object):

    def __init__(self):
        # elements are removed when encountered
        self.mandatory_sections = ['package', 'metadata', 'manifest', 'spine']
        self.xmldeclare = 0
        self.packagecnt = 0
        self.metadatacnt = 0
        self.manifestcnt = 0
        self.spinecnt = 0
        self.guidecnt = 0

    def check(self, sc, text, tp, tname, ttype, tattr):
        self.sc = sc        # xmlsanitychecker object
        self.text = text
        self.tp = tp
        self.tname = tname
        self.ttype = ttype
        self.tattr = tattr

        # Valid Tags
        if self.tname and 'metadata' not in self.sc.tagpath:  # skip metadata tags for now
            if self.tname not in SPECIAL_HANDLING_TAGS and self.tname not in VALID_TAGS:
                error_msg = 'tag: ' + self.tname + ' is not a valid OPF tag name.'
                self.sc.errors.append((self.sc.tag_start[0], self.sc.tag_start[1], error_msg))
                self.sc.has_error = True
                return

        # basic structure sanity check
        if self.tname == 'package' and self.ttype =='begin':
            self.mandatory_sections.remove(self.tname)
            self.packagecnt += 1
        if self.packagecnt > 1:
            error_msg = 'Only one "package" section allowed in an OPF file'
            self.sc.errors.append((self.sc.tag_start[0], self.sc.tag_start[1], error_msg))
            self.sc.has_error = True
            return
        if self.tname == 'metadata' and self.ttype =='begin':
            self.metadatacnt += 1
            self.mandatory_sections.remove(self.tname)
            if self.packagecnt == 0:
                error_msg = 'Tag "metadata" found before "package"'
                self.sc.errors.append((self.sc.tag_start[0], self.sc.tag_start[1], error_msg))
                self.sc.has_error = True
                return
        if self.metadatacnt > 1:
            error_msg = 'Only one "metadata" section allowed in an OPF file'
            self.sc.errors.append((self.sc.tag_start[0], self.sc.tag_start[1], error_msg))
            self.sc.has_error = True
            return
        if self.tname == 'manifest' and self.ttype =='begin':
            self.manifestcnt += 1
            self.mandatory_sections.remove(self.tname)
            if self.metadatacnt == 0:
                error_msg = 'Tag "manifest" found before "metadata"'
                self.sc.errors.append((self.sc.tag_start[0], self.sc.tag_start[1], error_msg))
                self.sc.has_error = True
                return
        if self.manifestcnt > 1:
            error_msg = 'Only one "manifest" section allowed in an OPF file'
            self.sc.errors.append((self.sc.tag_start[0], self.sc.tag_start[1], error_msg))
            self.sc.has_error = True
            return
        if self.tname == 'item' and 'manifest' not in self.sc.tagpath:
            error_msg = 'An "item" tag cannot occur outside of the "manifest" section.'
            self.sc.errors.append((self.sc.tag_start[0], self.sc.tag_start[1], error_msg))
            self.sc.has_error = True
            return
        if self.tname == 'spine' and self.ttype =='begin':
            self.spinecnt += 1
            self.mandatory_sections.remove(self.tname)
            if self.manifestcnt == 0:
                error_msg = 'Tag "spine" found before "manifest"'
                self.sc.errors.append((self.sc.tag_start[0], self.sc.tag_start[1], error_msg))
                self.sc.has_error = True
                return
        if self.spinecnt > 1:
            error_msg = 'Only one "spine" section allowed in an OPF file'
            self.sc.errors.append((self.sc.tag_start[0], self.sc.tag_start[1], error_msg))
            self.sc.has_error = True
            return
        if self.tname == 'itemref' and 'spine' not in self.sc.tagpath:
            error_msg = 'An "itemref" tag cannot occur outside of the "spine" section.'
            self.sc.errors.append((self.sc.tag_start[0], self.sc.tag_start[1], error_msg))
            self.sc.has_error = True
            return
        if self.tname == 'guide' and self.ttype =='begin':
            self.guidecnt += 1
            if self.spinecnt == 0:
                error_msg = 'Tag "guide" found before "spine"'
                self.sc.errors.append((self.sc.tag_start[0], self.sc.tag_start[1], error_msg))
                self.sc.has_error = True
                return
        if self.spinecnt > 1:
            error_msg = 'Only one "guide" section allowed in an OPF file'
            self.sc.errors.append((self.sc.tag_start[0], self.sc.tag_start[1], error_msg))
            self.sc.has_error = True
            return
        if self.tname == 'reference' and 'guide' not in self.sc.tagpath:
            error_msg = 'A "reference" tag cannot occur outside of the "guide" section.'
            self.sc.errors.append((self.sc.tag_start[0], self.sc.tag_start[1], error_msg))
            self.sc.has_error = True
            return
        if self.tname == "?xml":
            self.xmldeclare += 1
            if self.packagecnt > 0:
                error_msg = 'An xml declaration must come before the "package" tag'
                self.sc.errors.append((self.sc.tag_start[0], self.sc.tag_start[1], error_msg))
                self.sc.has_error = True
                return

        # Void tag check (closing tags are OK if last_text is None or all spaces)
        if self.tname in VOID_TAGS and self.ttype == "end" and not empty(self.sc.last_text):
            error_msg = 'Void tag: ' + self.tname + ' has an illegal ending tag'
            self.sc.errors.append((self.sc.tag_start[0], self.sc.tag_start[1], error_msg))
            self.sc.has_error = True
            return

