#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vim:ts=4:sw=4:softtabstop=4:smarttab:expandtab

from __future__ import unicode_literals, division, absolute_import, print_function

import re

xml_structure={}
min_required_attribs = []

ALL_SPACES = re.compile(r'^\s*$', re.U)

def empty(last_text):
    if last_text is None:
        return True
    return (ALL_SPACES.match(last_text) is not None)

def get_xml_specs(mtype, pkgver):
    global xml_structure
    global min_required_attribs
    if mtype == 'application/oebps-package+xml':
        import opf2data
        xml_structure = opf2data.xml_structure
        min_required_attribs = opf2data.min_required_attribs
        if pkgver == '3.0':
            import opf3data
            xml_structure = opf3data.xml_structure
            min_required_attribs = opf3data.min_required_attribs
    elif mtype == 'application/x-dtbncx+xml':
        import ncxdata
        xml_structure = ncxdata.xml_structure
        min_required_attribs = ncxdata.min_required_attribs
    elif mtype == 'application/smil+xml':
        import smildata
        xml_structure = smildata.xml_structure
        min_required_attribs = smildata.min_required_attribs
    elif mtype == 'application/oebps-page-map+xml':
        import pagedata
        xml_structure = pagedata.xml_structure
        min_required_attribs = pagedata.min_required_attribs


class GenCheck(object):

    def __init__(self, mtype, pkgver):
        get_xml_specs(mtype, pkgver)
        self.mtype, self.pkgver = mtype, pkgver
        self.cnt_structure = {}
        for key in xml_structure:
            self.cnt_structure[key] = 0

    def check(self, sc, text, tp, tname, ttype, tattr):
        if tname in xml_structure:
        
            [allowed_parents, isVoid, mincnt, maxcnt, predecessors] = xml_structure[tname]

            if ttype in ('begin', 'single'):
                self.cnt_structure[tname] = self.cnt_structure[tname] + 1
                cnt = self.cnt_structure[tname]
            
                if maxcnt != -1 and cnt > maxcnt:
                    error_msg = 'tag occurred too many times: %s' % tname
                    sc.errors.append((sc.tag_start[0], sc.tag_start[1], error_msg))
                    sc.has_error = True
                    return
            
                if sc.tagpath and sc.tagpath[-1] not in allowed_parents:
                    error_msg = 'incorrect parent for tag: %s' % tname
                    sc.errors.append((sc.tag_start[0], sc.tag_start[1], error_msg))
                    sc.has_error = True
                    return
            
                if predecessors is not None:
                    for tag in predecessors:
                        if tag is not None:
                            if self.cnt_structure[tag] == 0:
                                error_msg = '%s must precede %s' % (tag, tname)
                                sc.errors.append((sc.tag_start[0], sc.tag_start[1], error_msg))
                                sc.has_error = True
                                return
                if self.pkgver is not None and self.pkgver == '2.0':
                    # illegal EPUB3 manifest properties in an EPUB2
                    if tname == 'item' and 'properties' in [x.lower() for x in tattr.keys()]:
                        error_msg = 'invalid EPUB3 manifest properties for manifest entry: %s' % (tname)
                        sc.errors.append((sc.tag_start[0], sc.tag_start[1], error_msg))
                        sc.has_error = True

                if self.pkgver is not None and self.pkgver == '3.0':
                    # invalid manifest properties in an EPUB3
                    pass

            elif ttype == 'end':
                # Void tag check (closing tags are OK if last_text is None or all spaces)
                if isVoid and ttype == 'end' and not empty(sc.last_text):
                    error_msg = 'Void tag: ' + tname + ' has illegal contents'
                    sc.errors.append((sc.tag_start[0], sc.tag_start[1], error_msg))
                    sc.has_error = True
                    return
        else:
            error_msg = 'illegal tag found: %s' % tname
            sc.errors.append((sc.tag_start[0], sc.tag_start[1], error_msg))
            sc.has_error = True
            return

    def mincounts(self):
        error_msg = None
        for key in self.cnt_structure:
            cnt = self.cnt_structure[key]
            [allowed_parents, isVoid, mincnt, maxcnt, predecessors] = xml_structure[key]
            if cnt < mincnt:
                error_msg = 'required tag missing: %s' % key
                break
        return error_msg

