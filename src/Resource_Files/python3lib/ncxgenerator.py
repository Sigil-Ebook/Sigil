#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# vim:ts=4:sw=4:softtabstop=4:smarttab:expandtab

import sys
import os
from quickparser import QuickXHTMLParser
from hrefutils import quoteurl, unquoteurl, startingDir, buildBookPath, buildRelativePath, relativePath

_epubtype_guide_map = {
     'acknowledgements' : 'acknowledgments',
     'afterword'        : 'other.afterword',
     'appendix'         : 'other.appendix',
     'backmatter'       : 'other.backmatter',
     'bibliography'     : 'bibliography',
     'bodymatter'       : 'text',
     'chapter'          : 'other.chapter',
     'colophon'         : 'colophon',
     'conclusion'       : 'other.conclusion',
     'contributors'     : 'other.contributors',
     'copyright-page'   : 'copyright-page',
     'cover'            : 'cover',
     'dedication'       : 'dedication',
     'division'         : 'other.division',
     'epigraph'         : 'epigraph',
     'epilogue'         : 'other.epilogue',
     'errata'           : 'other.errata',
     'footnotes'        : 'other.footnotes',
     'foreword'         : 'foreword',
     'frontmatter'      : 'other.frontmatter',
     'glossary'         : 'glossary',
     'halftitlepage'    : 'other.halftitlepage',
     'imprint'          : 'other.imprint',
     'imprimatur'       : 'other.imprimatur',
     'index'            : 'index',
     'introduction'     : 'other.introduction',
     'landmarks'        : 'other.landmarks',
     'loa'              : 'other.loa',
     'loi'              : 'loi',
     'lot'              : 'lot',
     'lov'              : 'other.lov',
     # ''               : 'notes',
     'notice'           : 'other.notice',
     'other-credits'    : 'other.other-credits',
     'part'             : 'other.part',
     'preamble'         : 'other.preamble',
     'preface'          : 'preface',
     'prologue'         : 'other.prologue',
     'rearnotes'        : 'other.rearnotes',
     'subchapter'       : 'other.subchapter',
     'titlepage'        : 'title-page',
     'toc'              : 'toc',
     'volume'           : 'other.volume',
     'warning'          : 'other.warning'
}


# parse the current nav.xhtml to extract toc, pagelist, and landmarks
# note all hrefs, src  are in unquoted form when they are stored
# note all hrefs, src, have been converted to be relative to newdir
def parse_nav(qp, navdata, navbkpath, newdir):
    qp.setContent(navdata)
    toclist = []
    pagelist = []
    landmarks = []
    lvl = 0
    pgcnt = 0
    maxlvl = -1
    nav_type = None
    href = None
    title = ""
    play = 0
    navdir = startingDir(navbkpath)

    for txt, tp, tname, ttype, tattr in qp.parse_iter():
        if txt is not None:
            if ".a." in tp or tp.endswith(".a"):
                title = title + txt
            else:
                title = ""
        else:
            if tname == "nav":
                if ttype == "begin":
                    nav_type = tattr.get("epub:type", None)
                if ttype == "end":
                    nav_type = None
                continue
            if tname == "ol" and nav_type is not None and nav_type in ("toc","page-list","landmarks"):
                if ttype == "begin":
                    lvl += 1
                    if nav_type == "toc":
                        if lvl > maxlvl: maxlvl = lvl
                if ttype == "end": lvl -= 1
                continue
            if tname == "a" and ttype == "begin":
                href = tattr.get("href", "")
                href = unquoteurl(href)
                if href.find(":") == -1:
                    # first strip off any fragment
                    fragment = ""
                    if href.find("#") != -1:
                        href, fragment = href.split("#")
                    # find destination bookpath
                    if href.startswith("./"): href=href[2:]
                    if href == "":
                        destbkpath = navbkpath
                    else:
                        destbkpath = buildBookPath(href, navdir)
                    # create relative path to destbkpath from newdir
                    href = relativePath(destbkpath, newdir)
                    if fragment != "":
                        href = href + "#" + fragment
                epubtype = tattr.get("epub:type", None)
                continue
            if tname == "a" and ttype == "end":
                if nav_type == "toc":
                    play += 1
                    toclist.append((play, lvl, href, title))
                elif nav_type == "page-list":
                    pgcnt += 1
                    pagelist.append((pgcnt, href, title))
                elif nav_type == "landmarks":
                    if epubtype is not None:
                        gtype = _epubtype_guide_map.get(epubtype, None)
                        landmarks.append((gtype, href, title))
                title = ""
                continue

    return toclist, pagelist, landmarks, maxlvl, pgcnt


# build ncx from epub3 toc from toclist, pagelist and old opf2 guide info for landmarks
def build_ncx(doctitle, mainid, maxlvl, pgcnt, toclist, pagelist):
    ncxres = []
    ind = '  '
    ncxres.append('<?xml version="1.0" encoding="utf-8"?>\n')
    ncxres.append('<ncx xmlns="http://www.daisy.org/z3986/2005/ncx/" version="2005-1">\n')
    ncxres.append('  <head>\n')
    ncxres.append('    <meta name="dtb:uid" content="' + mainid + '" />\n')
    ncxres.append('    <meta name="dtb:depth" content="' + str(maxlvl) + '" />\n')
    ncxres.append('    <meta name="dtb:totalPageCount" content="' + str(pgcnt) + '" />\n')
    ncxres.append('    <meta name="dtb:maxPageNumber" content="' + str(pgcnt) + '" />\n')
    ncxres.append('  </head>\n')
    ncxres.append('<docTitle>\n')
    ncxres.append('  <text>' + doctitle + '</text>\n')
    ncxres.append('</docTitle>\n')
    ncxres.append('<navMap>\n')
    plvl = -1
    for (po, lvl, href, title) in toclist:
        # first close off any already opened navPoints
        while lvl <= plvl:
            space = ind*plvl
            ncxres.append(space + '</navPoint>\n')
            plvl -= 1
        # now append this navpoint
        space = ind*lvl
        porder = str(po)
        if title is None:
            title = ""
        ncxres.append(space + '<navPoint id="navPoint' + porder +'">\n')
        ncxres.append(space + '  <navLabel>\n')
        ncxres.append(space + '    <text>' + title + '</text>\n')
        ncxres.append(space + '  </navLabel>\n')
        ncxres.append(space + '  <content src="' + quoteurl(href) + '" />\n')
        plvl = lvl
    # now finish off any open navpoints
    while plvl > 0:
        space = ind*plvl
        ncxres.append(space + '</navPoint>\n')
        plvl -= 1
    ncxres.append('</navMap>\n')
    if pgcnt > 0:
        play = len(toclist)
        ncxres.append('<pageList>\n')
        for (cnt, href, title) in pagelist:
            porder = str(play + cnt)
            target = ind + '<pageTarget id="navPoint' + porder + '" type="normal"'
            target += ' value="' + title + '">\n'
            ncxres.append(target)
            ncxres.append(ind*2 + '<navLabel><text>' + title + '</text></navLabel>\n')
            ncxres.append(ind*2 + '<content src="' + quoteurl(href) + '" />\n')
            ncxres.append(ind + '</pageTarget>\n')
        ncxres.append('</pageList>\n')
    # now close it off
    ncxres.append('</ncx>\n')
    return  "".join(ncxres)


# the entry points
def generateNCX(navdata, navbkpath, ncxdir, doctitle, mainid):
    has_error = False
    # main id must exactly match used in the opf
    # if mainid.startswith("urn:uuid:"): mainid = mainid[9:]
    # try:
    qp = QuickXHTMLParser()
    toclist, pagelist, landmarks, maxlvl, pgcnt = parse_nav(qp, navdata, navbkpath, ncxdir)
    ncxdata = build_ncx(doctitle, mainid, maxlvl, pgcnt, toclist, pagelist)
    # except:
    #     has_error = True
    #     pass
    # if has_error:
    #     return ""
    return ncxdata

def generateGuideEntries(navdata, navbkpath, opfdir):
     has_error = False
     try:
         qp = QuickXHTMLParser()
         toclist, pagelist, landmarks, maxlvl, pgcnt = parse_nav(qp, navdata, navbkpath, opfdir)
     except:
         has_error = True
         pass
     if has_error:
         return [("","","")]
     return landmarks


def main():
    argv = sys.argv
    return 0

if __name__ == '__main__':
    sys.exit(main())
