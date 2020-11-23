#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# vim:ts=4:sw=4:softtabstop=4:smarttab:expandtab

# Copyright (c) 2019-2020 Kevin B. Hendricks
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
import re
from quickparser import QuickXHTMLParser

SIGIL_REPLACE_LANDMARKS_HERE = "<!-- SIGIL_REPLACE_LANDMARKS_HERE -->"
SIGIL_REPLACE_PAGELIST_HERE  = "<!-- SIGIL_REPLACE_PAGELIST_HERE -->"
SIGIL_REPLACE_TOC_HERE       = "<!-- SIGIL_REPLACE_TOC_HERE -->"

NAV_TOC_PATTERN       = re.compile(r'''^\s*<!--\s*SIGIL_REPLACE_TOC_HERE\s*-->\s*$''', re.M)
NAV_PAGELIST_PATTERN  = re.compile(r'''^\s*<!--\s*SIGIL_REPLACE_PAGELIST_HERE\s*-->\s*$''', re.M)
NAV_LANDMARKS_PATTERN = re.compile(r'''^\s*<!--\s*SIGIL_REPLACE_LANDMARKS_HERE\s*-->\s*$''', re.M)

# encode/escape text to make it xml safe
def xmlencode(data):
    if data is None:
        return ''
    newdata = xmldecode(data)
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

    def __init__(self, navsrc, codec='utf-8'):
        if navsrc is None:
            navsrc = ""
        if isinstance(navsrc, bytes):
            self.content = navsrc.decode(codec)
        else:
            self.content = navsrc

    # returns ordered list of tuples (play_order, nesting_level, href, title)
    # href is in url encoded form (percent encodings used if needed)
    # title has been xml decoded/unescaped
    def getTOC(self):
        # parse the nav to get the table of contents
        navsrc = self.content
        toclist = []

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
                    continue
                if tname == "nav" and ttype == "end":
                    nav_type = None
                    continue
                if nav_type is not None and nav_type == "toc":
                    if tname == "ol":
                        if ttype == "begin": lvl += 1
                        if ttype == "end": lvl -= 1
                        continue
                    if tname == "a" and ttype == "begin":
                        href = tattr.get("href", "")
                        # must leave all url hrefs in raw url encoded form
                        # if they can ever contain fragments
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
    # href should be url encoded (percent encodings present if needed)
    # title should be xml decoded/unescaped
    def setTOC(self, toclist):
        toc_xhtml = self.buildTOC(toclist)
        # replace the TOC in the current navsrc with a placeholder
        navsrc = self.content
        qp = QuickXHTMLParser()
        qp.setContent(navsrc)
        nav_type = None
        res = []
        skip_output = False
        for txt, tp, tname, ttype, tattr in qp.parse_iter():
            if txt is not None:
                if not skip_output:
                    res.append(txt)
            else:
                if tname == "nav" and ttype == "begin":
                    nav_type = tattr.get("epub:type", None)
                    if nav_type is not None and nav_type == "toc":
                        res.append(SIGIL_REPLACE_TOC_HERE)
                        skip_output = True
                        continue
                if tname == "nav" and ttype == "end" and nav_type == "toc":
                    nav_type = None
                    skip_output = False
                    continue

                if not skip_output:
                    res.append(qp.tag_info_to_xml(tname, ttype, tattr))

        navsrc = "".join(res)
        m = re.search(NAV_TOC_PATTERN, navsrc)
        if m is None:
            return False
        navsrc = navsrc[0:m.start()] + toc_xhtml + navsrc[m.end():]
        self.content = navsrc
        return True

    # returns ordered list of tuples (epubtype, href, title)
    # href is url encoded (percent encodings present if needed)
    # title has been xml decoded/unescaped
    def getLandmarks(self):
        # parse the nav to get the landmarks
        navsrc = self.content
        landmarks = []

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
                if tname == "nav" and ttype == "end":
                    nav_type = None
                    continue

                if nav_type is not None and nav_type == "landmarks":
                    if tname == "a" and ttype == "begin":
                        href = tattr.get("href", "")
                        # must leave all hrefs in raw url encoded form
                        # if they can contain fragments
                        epubtype = tattr.get("epub:type", None)
                        continue
                    if tname == "a" and ttype == "end":
                        if epubtype is not None:
                            title = xmldecode(title)
                            landmarks.append((epubtype, href, title))
                        title = ""
                        epubtype = None
                        href = None
                        continue
        return landmarks

    # replace the landmarks with ordered list of tuples (epubtype, href, title)
    # href should be url encoded (percent encodings present if needed)
    # title should be xml decoded/unescaped
    def setLandmarks(self, landmarks):
        landmarks_xhtml = self.buildLandmarks(landmarks)
        # replace the landmarks from the navsrc with a placeholer
        navsrc = self.content
        qp = QuickXHTMLParser()
        qp.setContent(navsrc)
        nav_type = None
        res = []
        skip_output = False
        for txt, tp, tname, ttype, tattr in qp.parse_iter():
            if txt is not None:
                if not skip_output:
                    res.append(txt)
            else:
                if tname == "nav" and ttype == "begin":
                    nav_type = tattr.get("epub:type", None)
                    if nav_type is not None and nav_type == "landmarks":
                        res.append(SIGIL_REPLACE_LANDMARKS_HERE)
                        skip_output = True
                        continue
                if tname == "nav" and ttype == "end" and nav_type == "landmarks":
                    nav_type = None
                    skip_output = False
                    continue

                if not skip_output:
                    res.append(qp.tag_info_to_xml(tname, ttype, tattr))

        navsrc = "".join(res)
        m = re.search(NAV_LANDMARKS_PATTERN, navsrc)
        if m is None:
            return False
        navsrc = navsrc[0:m.start()] + landmarks_xhtml + navsrc[m.end():]
        self.content = navsrc
        return True

    # returns ordered list of tuples (page_number, href, title)
    # href is url encoded (percent encodings if needed should be present))
    # title has been xml decoded/unescaped
    def getPageList(self):
        # parse the nav source to get the page-list
        navsrc = self.content
        pagelist = []

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
                if tname == "nav" and ttype == "end":
                    nav_type = None
                    continue
                if nav_type is not None and nav_type == "page-list":
                    if tname == "a" and ttype == "begin" and nav_type == "page-list":
                        href = tattr.get("href", "")
                        # hrefs must be kept in raw urlencoded form that may contain fragments
                        continue
                    if tname == "a" and ttype == "end":
                        pgcnt += 1
                        title = xmldecode(title)
                        pagelist.append((pgcnt, href, title))
                        title = ""
                        continue

        return pagelist

    # replace the page with ordered list of tuples (page_number, href, title)
    # href should be url encoded (percent encodings present if needed))
    # title should be xml decoded/unescaped
    def setPageList(self, pagelist):
        pagelist_xhtml = self.buildPageList(pagelist)
        # replace the pagelist from the navsrc with a placeholer
        navsrc = self.content
        qp = QuickXHTMLParser()
        qp.setContent(navsrc)
        nav_type = None
        res = []
        skip_output = False
        found_page_list = False

        for txt, tp, tname, ttype, tattr in qp.parse_iter():
            if txt is not None:
                if not skip_output:
                    res.append(txt)
            else:
                if tname == "nav" and ttype == "begin":
                    nav_type = tattr.get("epub:type", None)
                    if nav_type is not None and nav_type == "page-list":
                        res.append(SIGIL_REPLACE_PAGELIST_HERE)
                        found_page_list = True
                        skip_output = True
                        continue
                if tname == "nav" and ttype == "end" and nav_type == "page-list":
                    nav_type = None
                    skip_output = False
                    continue
                if tname == "body" and ttype == "end":
                    if not found_page_list and len(pagelist) > 0:
                        padding = res[-1]
                        res.append(SIGIL_REPLACE_PAGELIST_HERE)
                        res.append(padding)
                        found_page_list = True

                if not skip_output:
                    res.append(qp.tag_info_to_xml(tname, ttype, tattr))

        navsrc = "".join(res)
        m = re.search(NAV_PAGELIST_PATTERN, navsrc)
        if m is None:
            return False
        navsrc = navsrc[0:m.start()] + pagelist_xhtml + navsrc[m.end():]
        self.content = navsrc
        return True

    # self.toclist is an ordered list of tuples (play_order, nesting_level, href, title)
    # hrefs should be in url encoded form (percent encodings present if needed)  
    def buildTOC(self, toclist):
        navres = []
        ind = '  '
        ibase = ind * 3
        incr = ind * 2
        # start with the toc
        navres.append(ind * 2 + '<nav epub:type="toc" id="toc">\n')
        navres.append(ind * 3 + '<h1>Table of Contents</h1>\n')
        navres.append(ibase + '<ol>\n')
        curlvl = 1
        initial = True
        for po, lvl, href, lbl in toclist:
            lbl = xmlencode(lbl)
            if lvl > curlvl:
                while lvl > curlvl:
                    indent = ibase + incr * (curlvl)
                    navres.append(indent + '<ol>\n')
                    navres.append(indent + ind + '<li>\n')
                    navres.append(indent + ind * 2 + '<a href="%s">%s</a>\n' % (href, lbl))
                    curlvl += 1
            elif lvl < curlvl:
                while lvl < curlvl:
                    indent = ibase + incr * (curlvl - 1)
                    navres.append(indent + ind + '</li>\n')
                    navres.append(indent + '</ol>\n')
                    curlvl -= 1
                indent = ibase + incr * (lvl - 1)
                navres.append(indent + ind + '</li>\n')
                navres.append(indent + ind + '<li>\n')
                navres.append(indent + ind * 2 + '<a href="%s">%s</a>\n' % (href, lbl))
            else:
                indent = ibase + incr * (lvl - 1)
                if not initial:
                    navres.append(indent + ind + '</li>\n')
                navres.append(indent + ind + '<li>\n')
                navres.append(indent + ind * 2 + '<a href="%s">%s</a>\n' % (href, lbl))
            initial = False
            curlvl = lvl
        while(curlvl > 0):
            indent = ibase + incr * (curlvl - 1)
            navres.append(indent + ind + "</li>\n")
            navres.append(indent + "</ol>\n")
            curlvl -= 1
        navres.append(ind * 2 + '</nav>\n')
        return "".join(navres)


    # self.pagelist is an ordered list of tuples (page_number, href, title)
    # href should be url encoded (percent encodings present if needed)
    def buildPageList(self, pagelist):
        navres = []
        ind = '  '
        # add any existing page-list if need be
        if len(pagelist) > 0:
            navres.append(ind * 2 + '<nav epub:type="page-list" id="page-list" hidden="">\n')
            navres.append(ind * 3 + '<ol>\n')
            for pn, href, title in pagelist:
                title = xmlencode(title)
                navres.append(ind * 4 + '<li><a href="%s">%s</a></li>\n' % (href, title))
            navres.append(ind * 3 + '</ol>\n')
            navres.append(ind * 2 + '</nav>\n')
        return "".join(navres)


    # self.landmarks is an ordered list of tuples (epub_type, href, title)
    # href should be url encoded (percent encodings present if needed) 
    def buildLandmarks(self, landmarks):
        navres = []
        ind = '  '
        navres.append(ind * 2 + '<nav epub:type="landmarks" id="landmarks" hidden="">\n')
        navres.append(ind * 3 + '<h2>Guide</h2>\n')
        navres.append(ind * 3 + '<ol>\n')
        for etyp, href, title in landmarks:
            title = xmlencode(title)
            navres.append(ind * 4 + '<li>\n')
            navres.append(ind * 5 + '<a epub:type="%s" href="%s">%s</a>\n' % (etyp, href, title))
            navres.append(ind * 4 + '</li>\n')
        navres.append(ind * 3 + '</ol>\n')
        navres.append(ind * 2 + '</nav>\n')
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
    with open(navpath, 'rb') as f:
        navsrc = f.read()
    navsrc = navsrc.decode('utf-8')
    np = NavProcessor(navsrc)
    landmarks = np.getLandmarks()
    pagelist = np.getPageList()
    toclist = np.getTOC()
    print(toclist)
    print(landmarks)
    print(pagelist)
    print(np.setLandmarks(landmarks))
    print(np.setPageList(pagelist))
    print(np.setTOC(toclist))
    print(np.getNavSrc())
    return 0

if __name__ == '__main__':
    sys.exit(main())
