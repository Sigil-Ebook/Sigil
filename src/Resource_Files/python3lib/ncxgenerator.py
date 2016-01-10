#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# vim:ts=4:sw=4:softtabstop=4:smarttab:expandtab

import sys
import os
from urllib.parse import unquote
from quickparser import QuickXHTMLParser

# unquotes url/iri
def unquoteurl(href):
    if isinstance(href,bytes):
        href = href.decode('utf-8')
    href = unquote(href)
    return href

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
def parse_nav(qp, navdata, navname):
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
                # nav lives in OEBPS/Text in Sigil
                # ncx lives in OEBPS/ in Sigil
                # So fixup href's to be what they need to be for the ncx
                # Also work with it in quoted form
                if href.startswith("./"):  
                    href= href[2:]
                    if href == "":
                        href = "Text/" + navname
                elif href.startswith("#"):
                    href = "Text/" + navname + href
                elif href.startswith("../"): 
                    href = href[3:]
                if not href.startswith("Text"):
                    href = "Text/" + href
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
                        href = unquoteurl(href)
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
    ncxres.append('<head>\n')
    ncxres.append('  <meta name="dtb:uid" content="' + mainid + '" />\n')
    ncxres.append('  <meta name="dtb:depth" content="' + str(maxlvl) + '" />\n')
    ncxres.append('  <meta name="dtb:totalPageCount" content="' + str(pgcnt) + '" />\n')
    ncxres.append('  <meta name="dtb:maxPageNumber" content="' + str(pgcnt) + '" />\n')
    ncxres.append('</head>\n')
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
        ncxres.append(space + '<navPoint id="navPoint' + porder +'" playOrder="' + porder + '">\n')
        ncxres.append(space + '  <navLabel>\n')
        ncxres.append(space + '    <text>' + title + '</text>\n')
        ncxres.append(space + '  </navLabel>\n')
        ncxres.append(space + '  <content src="' + href + '" />\n')
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
            target += ' value="' + title + '" playOrder="' + porder + '" >\n'
            ncxres.append(target)
            ncxres.append(ind*2 + '<navLabel><text>' + title + '</text></navLabel>\n')
            ncxres.append(ind*2 + '<content src="' + href + '" />\n')
            ncxres.append(ind + '</pageTarget>\n')
        ncxres.append('</pageList>\n')
    # now close it off
    ncxres.append('</ncx>\n')
    return  "".join(ncxres)


# the entry points
def generateNCX(navdata, navname, doctitle, mainid):
    has_error = False
    if mainid.startswith("urn:uuid:"): mainid = mainid[9:]
    # try:
    qp = QuickXHTMLParser()
    toclist, pagelist, landmarks, maxlvl, pgcnt = parse_nav(qp, navdata, navname)
    ncxdata = build_ncx(doctitle, mainid, maxlvl, pgcnt, toclist, pagelist)
    # except:
    #     has_error = True
    #     pass
    # if has_error:
    #     return ""
    return ncxdata
 

def generateGuideEntries(navdata, navname):
    has_error = False
    try:
        qp = QuickXHTMLParser()
        toclist, pagelist, landmarks, maxlvl, pgcnt = parse_nav(qp, navdata, navname)
    except:
        has_error = True
        pass
    if has_error:
        return [("","","")]
    return landmarks


def main():
    argv = sys.argv
    if argv[1] is None:
        sys.exit(0)

    if not os.path.exists(argv[1]):
        sys.exit(0)

    navname = os.path.basename(argv[1])
    navdata = ""
    with open(argv[1], 'rb') as f:
        navdata = f.read().decode('utf-8',errors='replace')

    ncxdata = generateNCX(navdata, navname, "This is a Test", "some_uuid_mess")
    print(ncxdata)

    guide = generateGuideEntries(navdata, navname)
    print(guide)
    return 0

# def main():
#     print("I reached main when I should not have\n")
#     return -1
    
if __name__ == '__main__':
    sys.exit(main())
