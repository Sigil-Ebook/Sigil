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
import os
import re
from quickparser import QuickXHTMLParser

PY3 = sys.version_info[0] >= 3

if PY3:
    text_type = str
    binary_type = bytes
else:
    text_type = unicode
    binary_type = str

try:
    from urllib.parse import unquote
    from urllib.parse import urlsplit
except ImportError:
    from urllib import unquote
    from urlparse import urlsplit

SIGIL_REPLACE_LANDMARKS_HERE = "<!-- SIGIL_REPLACE_LANDMARKS_HERE -->"
SIGIL_REPLACE_PAGELIST_HERE  = "<!-- SIGIL_REPLACE_PAGELIST_HERE -->"
SIGIL_REPLACE_TOC_HERE       = "<!-- SIGIL_REPLACE_TOC_HERE -->"

NAV_TOC_PATTERN       = re.compile(r'''^\s*<!--\s*SIGIL_REPLACE_TOC_HERE\s*-->\s*$''',re.M)
NAV_PAGELIST_PATTERN  = re.compile(r'''^\s*<!--\s*SIGIL_REPLACE_PAGELIST_HERE\s*-->\s*$''',re.M)
NAV_LANDMARKS_PATTERN = re.compile(r'''^\s*<!--\s*SIGIL_REPLACE_LANDMARKS_HERE\s*-->\s*$''',re.M)

TOC_START_PATTERN       = re.compile(r'''(<\s*nav\s[^>]*epub:type[^>]*[\"']toc[\"'][^>]*>)''',re.I)
PAGELIST_START_PATTERN  = re.compile(r'''(<\s*nav\s[^>]*epub:type[^>]*[\"']page-list[\"'][^>]*>)''',re.I)
LANDMARKS_START_PATTERN = re.compile(r'''(<\s*nav\s[^>]*epub:type[^>]*[\"']landmarks[\"'][^>]*>)''',re.I)
NAV_TAG_END_PATTERN     = re.compile(r'''(</\s*nav\s*>)''', re.I)

ASCII_CHARS   = set(chr(x) for x in range(128))
URL_SAFE      = set('ABCDEFGHIJKLMNOPQRSTUVWXYZ'
                    'abcdefghijklmnopqrstuvwxyz'
                    '0123456789' '#' '_.-/~')
IRI_UNSAFE = ASCII_CHARS - URL_SAFE

# returns a quoted IRI (not a URI)
# modified to be scheme aware as external resources are possible in epub3
def quoteurl(href):
    if isinstance(href,binary_type):
        href = href.decode('utf-8')
    (ascheme, anetloc, apath, aquery, afragment) = urlsplit(href, scheme="", allow_fragments=True)
    if ascheme != "":
        ascheme += "://"
        href = href[len(ascheme):]
    result = []
    for char in href:
        if char in IRI_UNSAFE:
            char = "%%%02x" % ord(char)
        result.append(char)
    return ascheme + ''.join(result)

# unquotes url/iri
def unquoteurl(href):
    if isinstance(href,binary_type):
        href = href.decode('utf-8')
    href = unquote(href)
    return href

# encode/escape text to make it xml safe
def xmlencode(data):
    if data is None:
        return ''
    newdata = data
    newdata = newdata.replace('&', '&amp;')
    newdata = newdata.replace('<', '&lt;')
    newdata = newdata.replace('>', '&gt;')
    newdata = newdata.replace('"', '&quot;')
    return newdata

# decode xml encoded/escaped strings
def xmldecode(data):
    if data is None:
        return ''
    newdata = data
    newdata = newdata.replace('&quot;', '"')
    newdata = newdata.replace('&gt;', '>')
    newdata = newdata.replace('&lt;', '<')
    newdata = newdata.replace('&amp;', '&')
    return newdata


class NavProcessor(object):

    def __init__(self, navsrc, codec = 'utf-8'):
        if navsrc is None:
            navsrc = ""
        if isinstance(navsrc, binary_type):
            self.content = navsrc.decode(codec)
        else:
            self.content = navsrc

    # returns ordered list of tuples (play_order, nesting_level, href, title)
    # href is unquoted (percent encodings removed)
    # title has been xml decoded/unescaped
    def getTOC(self):
        # as the user may have left the nav in an unparseable state
        # use regular expressions to try to extract exactly what we want
        navsrc = self.content
        toclist = []

        # extract the TOC from the navsrc
        m_beg = re.search(TOC_START_PATTERN, navsrc)
        if m_beg is None:
            return toclist
        bp = m_beg.start()
        m_end = re.search(NAV_TAG_END_PATTERN, navsrc[m_beg.end():])
        if m_end is None:
            return toclist
        ep = m_end.end() + m_beg.end() + 1
        navsrc = navsrc[bp:ep]

        # now parse this snippet to extract the toc
        qp = QuickXHTMLParser()
        qp.setContent(navsrc)

        lvl = 0
        po = 0
        title = ""
        nav_type = None
        href = None
        for txt, tp, tname, ttype, tattr in qp.parse_iter():
            if txt is not None:
                if ".a." in tp or tp.endswith(".a"):
                    title = title + txt
                else:
                    title = ""
            else:
                if tname == "nav" and ttype == "begin":
                    nav_type = tattr.get("epub:type", None)
                if tname == "ol":
                    if ttype == "begin": lvl += 1
                    if ttype == "end": lvl -= 1
                    continue
                if tname == "a" and ttype == "begin":
                    href = tattr.get("href", "")
                    href = unquoteurl(href)
                    continue
                if tname == "a" and ttype == "end":
                    po += 1
                    title = xmldecode(title)
                    toclist.append((po, lvl, href, title))
                    title = ""
                    href = None
                    continue

        return toclist

    # replace the TOC with ordered list of tuples (play_order, nesting_level, href, title)
    # href should be unquoted (percent encodings removed)
    # title should be xml decoded/unescaped
    def setTOC(self, toclist):
        toc_xhtml =  self.buildTOC(toclist)
        # replace the TOC from the navsrc with a placeholer                                                            
        navsrc = self.content
        m_beg = re.search(TOC_START_PATTERN, navsrc)
        # the toc is not optional so it should always be found
        if m_beg is None:
            return False
        bp = m_beg.start()
        m_end = re.search(NAV_TAG_END_PATTERN, navsrc[m_beg.end():])
        if m_end is None:
            return False
        ep = m_end.end() + m_beg.end() + 1
        navsrc = navsrc[0:bp] + SIGIL_REPLACE_TOC_HERE + navsrc[ep:]
        m = re.search(NAV_TOC_PATTERN, navsrc)
        if m is None:
            return False
        navsrc = navsrc[0:m.start()] + toc_xhtml + navsrc[m.end():]
        self.content = navsrc
        return True

    # returns ordered list of tuples (epubtype, href, title)
    # href is unquoted (percent encodings removed)
    # title has been xml decoded/unescaped
    def getLandmarks(self):
        # as the user may have left the nav in an unparseable state
        # use regular expressions to try to extract exactly what we want
        navsrc = self.content
        landmarks = []

        # extract the landmarks from the navsrc
        m_beg = re.search(LANDMARKS_START_PATTERN, navsrc)
        if m_beg is None:
            return landmarks
        bp = m_beg.start()
        m_end = re.search(NAV_TAG_END_PATTERN, navsrc[m_beg.end():])
        if m_end is None:
            return landmarks
        ep = m_end.end() + m_beg.end() + 1
        navsrc = navsrc[bp:ep]

        # now parse this snippet to extract the landmarks
        qp = QuickXHTMLParser()
        qp.setContent(navsrc)
        title = ""
        nav_type = None
        href = None
        epubtype = None
        for txt, tp, tname, ttype, tattr in qp.parse_iter():
            if txt is not None:
                if ".a." in tp or tp.endswith(".a"):
                    title = title + txt
                else:
                    title = ""
            else:
                if tname == "nav" and ttype == "begin":
                    nav_type = tattr.get("epub:type", None)
                    continue
                if tname == "a" and ttype == "begin":
                    href = tattr.get("href", "")
                    href = unquoteurl(href)
                    epubtype = tattr.get("epub:type", None)
                    continue
                if tname == "a" and ttype == "end":
                    if epubtype is not None:
                        title = xmldecode(title)
                        landmarks.append((epubtype, href, title))
                    title = ""
                    epubtype = None
                    href=None
                    continue
        return landmarks

    # replace the landmarks with ordered list of tuples (epubtype, href, title)
    # href should be unquoted (percent encodings removed)
    # title should be xml decoded/unescaped
    def setLandmarks(self, landmarks):
        landmarks_xhtml =  self.buildLandmarks(landmarks)
        # replace the landmarks from the navsrc with a placeholer
        navsrc = self.content
        m_beg = re.search(LANDMARKS_START_PATTERN, navsrc)
        # landmarks is not optional so it should always be found
        if m_beg is None:
            return False
        bp = m_beg.start()
        m_end = re.search(NAV_TAG_END_PATTERN, navsrc[m_beg.end():])
        if m_end is None:
            return False
        ep = m_end.end() + m_beg.end() + 1
        navsrc = navsrc[0:bp] + SIGIL_REPLACE_LANDMARKS_HERE + navsrc[ep:]
        m = re.search(NAV_LANDMARKS_PATTERN, navsrc)
        if m is None:
            return False
        navsrc = navsrc[0:m.start()] + landmarks_xhtml + navsrc[m.end():]
        self.content = navsrc
        return True

    # returns ordered list of tuples (page_number, href, title)
    # href is unquoted (percent encodings removed)
    # title has been xml decoded/unescaped
    def getPageList(self):
        # as the user may have left the nav in an unparseable state
        # use regular expressions to try to extract exactly what we want
        navsrc = self.content
        pagelist = []

        # extract the page-list from the navsrc
        m_beg = re.search(PAGELIST_START_PATTERN, navsrc)
        if m_beg is None:
            return pagelist
        bp = m_beg.start()
        m_end = re.search(NAV_TAG_END_PATTERN, navsrc[m_beg.end():])
        if m_end is None:
            return pagelist
        ep = m_end.end() + m_beg.end() + 1
        navsrc = navsrc[bp:ep]

        # now parse this snippet to extract the page-list
        qp = QuickXHTMLParser()
        qp.setContent(navsrc)
        pgcnt = 0
        nav_type = None
        href = None
        title = ""
        for txt, tp, tname, ttype, tattr in qp.parse_iter():
            if txt is not None:
                if ".a." in tp or tp.endswith(".a"):
                    title = title + txt
                else:
                    title = ""
            else:
                if tname == "nav" and ttype == "begin":
                    nav_type = tattr.get("epub:type", None)
                    continue
                if tname == "a" and ttype == "begin":
                    href = tattr.get("href", "")
                    href = unquoteurl(href)
                    continue
                if tname == "a" and ttype == "end":
                    pgcnt += 1
                    title = xmldecode(title)
                    pagelist.append((pgcnt, href, title))
                    title = ""
                    continue
        return pagelist

    # replace the page with ordered list of tuples (page_number, href, title)
    # href should be unquoted (percent encodings removed)
    # title should be xml decoded/unescaped
    def setPageList(self, pagelist):
        pagelist_xhtml =  self.buildPageList(pagelist)
        # replace the pagelist from the navsrc with a placeholer
        navsrc = self.content
        m_beg = re.search(PAGELIST_START_PATTERN, navsrc)
        # the page-list is optional, but we may want to add one even if not currently present
        if m_beg is None:
            # no page-list nav section was found, so inject it before the nav landmarks
            if len(pagelist) > 0:
                m_landmark = re.search(LANDMARKS_START_PATTERN, navsrc)
                if m_landmark is None:
                    return False
                inject_xhtml = '<nav epub:type="page-list" id="page-list" hidden=""></nav>\n    '
                navsrc = navsrc[0:m_landmarks.start()] + inject_xhtml + navsrc[m_landmarks.start():]
        # try again
        m_beg = re.search(PAGELIST_START_PATTERN, navsrc)
        if m_beg is None:
            return False
        bp = m_beg.start()
        m_end = re.search(NAV_TAG_END_PATTERN, navsrc[m_beg.end():])
        if m_end is None:
            return False
        ep = m_end.end() + m_beg.end() + 1
        navsrc = navsrc[0:bp] + SIGIL_REPLACE_PAGELIST_HERE + navsrc[ep:]
        m = re.search(NAV_PAGELIST_PATTERN, navsrc)
        if m is None:
            return False
        navsrc = navsrc[0:m.start()] + pagelist_xhtml + navsrc[m.end():]
        self.content = navsrc
        return True

    # self.toclist is an ordered list of tuples (play_order, nesting_level, href, title)
    # href is unquoted in self.toclist is unquoted
    def buildTOC(self, toclist):
        navres = []
        ind = '  '
        ibase = ind*3
        incr = ind*2
        # start with the toc
        navres.append(ind*2 + '<nav epub:type="toc" id="toc">\n')
        navres.append(ind*3 + '<h1>Table of Contents</h1>\n')
        navres.append(ibase + '<ol>\n')
        curlvl = 1
        initial = True
        for po, lvl, href, lbl in toclist:
            href = quoteurl(href)
            lbl = xmlencode(lbl)
            if lvl > curlvl:
                while lvl > curlvl:
                    indent = ibase + incr*(curlvl)
                    navres.append(indent + "<ol>\n")
                    navres.append(indent + ind + '<li>\n')
                    navres.append(indent + ind*2 + '<a href="%s">%s</a>\n' % (href, lbl))
                    curlvl += 1
            elif lvl <  curlvl:
                while lvl < curlvl:
                    indent = ibase + incr*(curlvl-1)
                    navres.append(indent + ind + "</li>\n")
                    navres.append(indent + "</ol>\n")
                    curlvl -= 1
                indent = ibase + incr*(lvl-1)
                navres.append(indent + ind +  "</li>\n")
                navres.append(indent + ind + '<li>\n')
                navres.append(indent + ind*2 + '<a href="%s">%s</a>\n' % (href, lbl))
            else:
                indent = ibase + incr*(lvl-1)
                if not initial:
                    navres.append(indent + ind + '</li>\n')    
                navres.append(indent + ind + '<li>\n')
                navres.append(indent + ind*2 + '<a href="%s">%s</a>\n' % (href, lbl))
            initial = False
            curlvl=lvl
        while(curlvl > 0):
            indent = ibase + incr*(curlvl-1)
            navres.append(indent + ind + "</li>\n")
            navres.append(indent + "</ol>\n")
            curlvl -= 1
        navres.append(ind*2 + '</nav>\n')
        return "".join(navres)


    # self.pagelist is an ordered list of tuples (page_number, href, title)
    # href is unquoted in self.pagelist
    def buildPageList(self, pagelist):
        navres = []
        ind = '  '
        ibase = ind*3
        incr = ind*2
        # add any existing page-list if need be
        if len(pagelist) > 0:
            navres.append(ind*2 + '<nav epub:type="page-list" id="page-list" hidden="">\n')
            navres.append(ind*3 + '<ol>\n')
            for pn, href, title in pagelist:
                href = quoteurl(href)
                title = xmlencode(title)
                navres.append(ind*4 + '<li><a href="%s">%s</a></li>\n' % (href, title))
            navres.append(ind*3 + '</ol>\n')
            navres.append(ind*2 + '</nav>\n')
        return "".join(navres)


    # self.landmarks is an ordered list of tuples (epub_type, href, title)
    # href is unquoted in self.landmarks
    def buildLandmarks(self, landmarks):
        navres = []
        ind = '  '
        ibase = ind*3
        incr = ind*2
        navres.append(ind*2 + '<nav epub:type="landmarks" id="landmarks" hidden="">\n')
        navres.append(ind*3 + '<h2>Guide</h2>\n')
        navres.append(ind*3 + '<ol>\n')
        for etyp, href, title in landmarks:
            href = quoteurl(href)
            title = xmlencode(title)
            navres.append(ind*4 + '<li>\n')
            navres.append(ind*5 + '<a epub:type="%s" href="%s">%s</a>\n' % (etyp, href, title))
            navres.append(ind*4 + '</li>\n')
        navres.append(ind*3 + '</ol>\n')
        navres.append(ind*2 + '</nav>\n')
        return "".join(navres)

    # returns the nav source code as a unicode string in its current form
    def getNavSrc(self):
        return self.content


def main(argv=sys.argv):
    if len(argv) != 2:
        print("navprocessor.py nav_file_path")
        return -1
    navpath = argv[1]
    navsrc = ""
    with open(navpath,'rb') as f:
        navsrc = f.read()
    navsrc = navsrc.decode('utf-8')
    np = NavProcessor(navsrc)
    landmarks = np.getLandmarks()
    pagelist = np.getPageList()
    toclist = np.getTOC()
    print(np.setLandmarks(landmarks))
    print(np.setPageList(pagelist))
    print(np.setTOC(toclist))
    print(np.getNavSrc())

    return 0

if __name__ == '__main__':
    sys.exit(main())
