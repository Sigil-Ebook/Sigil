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

_guide_epubtype_map = {
     'acknowledgements'   : 'acknowledgments',
     'other.afterword'    : 'afterword',
     'other.appendix'     : 'appendix',
     'other.backmatter'   : 'backmatter',
     'bibliography'       : 'bibliography',
     'text'               : 'bodymatter',
     'other.chapter'      : 'chapter',
     'colophon'           : 'colophon',
     'other.conclusion'   : 'conclusion',
     'other.contributors' : 'contributors',
     'copyright-page'     : 'copyright-page',
     'cover'              : 'cover',
     'dedication'         : 'dedication',
     'other.division'     : 'division',
     'epigraph'           : 'epigraph',
     'other.epilogue'     : 'epilogue',
     'other.errata'       : 'errata',
     'other.footnotes'    : 'footnotes',
     'foreword'           : 'foreword',
     'other.frontmatter'  : 'frontmatter',
     'glossary'           : 'glossary',
     'other.halftitlepage': 'halftitlepage',
     'other.imprint'      : 'imprint',
     'other.imprimatur'   : 'imprimatur',
     'index'              : 'index',
     'other.introduction' : 'introduction',
     'other.landmarks'    : 'landmarks',
     'other.loa'          : 'loa',
     'loi'                : 'loi',
     'lot'                : 'lot',
     'other.lov'          : 'lov',
     'notes'              : '',
     'other.notice'       : 'notice',
     'other.other-credits': 'other-credits',
     'other.part'         : 'part',
     'other.preamble'     : 'preamble',
     'preface'            : 'preface',
     'other.prologue'     : 'prologue',
     'other.rearnotes'    : 'rearnotes',
     'other.subchapter'   : 'subchapter',
     'title-page'         : 'titlepage',
     'toc'                : 'toc',
     'other.volume'       : 'volume',
     'other.warning'      : 'warning'
}

# the plugin entry point
def generateNav(opfdata, ncxdata, navtitle):
    # now parse opf
    opfparser = OPFParser(opfdata)
    guide_info = opfparser.get_guide()
    lang = opfparser.get_lang()
    id2href = opfparser.get_id2hrefmap()
    
    # It is possible that the original <guide> contains references
    # to files not in the spine. Putting those "dangling" references 
    # in the EPUB3 navigation document will result in validation error:
    # RSC-011 "Found a reference to a resource that is not a spine item.".
    # We must check that the referenced files are listed in the spine.

    # First generate a list of hrefs in the spine
    spinelst = opfparser.get_spine()
    spine_hrefs = []
    for (idref, attr) in spinelst:
        spine_hrefs.append(id2href[idref])

    # reduce guide to those references found to the spine
    guide_info_in_spine = []
    for gtyp, gtitle, ghref in guide_info:
        ahref = ghref.split('#')[0]
        if ahref in spine_hrefs:
            guide_info_in_spine.append((gtyp, gtitle, ghref))

    # need to take info from guide tag in opf and toc.ncx to create a valid nav.xhtml
    has_error = False
    try:
        qp = QuickXHTMLParser()
        doctitle, toclist, pagelist = parse_ncx(qp, ncxdata)
        navdata = build_nav(doctitle, toclist, pagelist, guide_info_in_spine, lang, navtitle)
    except:
        has_error = True
        pass
    if has_error:
        return ""
    return navdata
 

# parse the current toc.ncx to extract toc info, and pagelist info
def parse_ncx(qp, ncxdata):
    qp.setContent(ncxdata)
    pagelist = []
    toclist = []
    doctitle = None
    navlable = None
    pagenum = None
    lvl = 0
    for txt, tp, tname, ttype, tattr in qp.parse_iter():
        if txt is not None:
            if tp.endswith(".doctitle.text"):
                doctitle = txt
            if tp.endswith('.navpoint.navlabel.text'):
                navlabel = txt
        else:
            if tname == "navpoint" and ttype == "begin":
                lvl += 1
            elif tname == "navpoint" and ttype == "end":
                lvl -= 1
            elif tname == "content" and tattr is not None and "src" in tattr and tp.endswith("navpoint"):
                href =  "../" + tattr["src"]
                toclist.append((lvl, navlabel, href))
                navlabel = None
            elif tname == "pagetarget" and ttype == "begin" and tattr is not None:
                pagenum = tattr.get("value",None)
            elif tname == "content" and tattr is not None and "src" in tattr and tp.endswith("pagetarget"):
                pageref = "../" + tattr["src"]
                pagelist.append((pagenum, pageref))
                pagenum = None

    return doctitle, toclist, pagelist


# build up nave from toclist, pagelist and old opf2 guide info for landmarks
def build_nav(doctitle, toclist, pagelist, guide_info, lang, navtitle):
    navres = []
    ind = '  '
    ibase = ind*3
    incr = ind*2
    navres.append('<?xml version="1.0" encoding="utf-8"?>\n')
    navres.append('<!DOCTYPE html>\n')
    navres.append('<html xmlns="http://www.w3.org/1999/xhtml" xmlns:epub="http://www.idpf.org/2007/ops"')
    navres.append(' lang="%s" xml:lang="%s">\n' % (lang, lang))
    navres.append(ind + '<head>\n')
    navres.append(ind*2 + '<meta charset="utf-8" />\n')
    navres.append(ind*2 + '<style type="text/css">\n')
    navres.append(ind*2 + 'nav#landmarks, nav#page-list { display:none; }\n')
    navres.append(ind*2 + 'ol { list-style-type: none; }\n')
    navres.append(ind*2 + '</style>\n')
    navres.append(ind + '</head>\n')
    navres.append(ind + '<body epub:type="frontmatter">\n')

    # start with the toc
    navres.append(ind*2 + '<nav epub:type="toc" id="toc">\n')
    navres.append(ind*3 + '<h1>' + navtitle + '</h1>\n')
    navres.append(ibase + '<ol>\n')
    curlvl = 1
    initial = True
    for lvl, lbl, href in toclist:
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

    # add any existing page-list if need be
    if len(pagelist) > 0:
        navres.append(ind*2 + '<nav epub:type="page-list" id="page-list" hidden="">\n')
        navres.append(ind*3 + '<ol>\n')
        for pn, href in pagelist:
            navres.append(ind*4 + '<li><a href="%s">%s</a></li>\n' % (href, pn))
        navres.append(ind*3 + '</ol>\n')
        navres.append(ind*2 + '</nav>\n')
    
    # use the guide from the opf2 to create the landmarks section
    navres.append(ind*2 + '<nav epub:type="landmarks" id="landmarks" hidden="">\n')
    navres.append(ind*3 + '<h2>Guide</h2>\n')
    navres.append(ind*3 + '<ol>\n')
    for gtyp, gtitle, ghref in guide_info:
        etyp = _guide_epubtype_map.get(gtyp, "")
        if etyp != "":
            navres.append(ind*4 + '<li>\n')
            navres.append(ind*5 + '<a epub:type="%s" href="%s">%s</a>\n' % (etyp, ghref, gtitle))
            navres.append(ind*4 + '</li>\n')
    navres.append(ind*3 + '</ol>\n')
    navres.append(ind*2 + '</nav>\n')

    # now close it off
    navres.append(ind + '</body>\n')
    navres.append('</html>\n')
    return  "".join(navres)


_OPF_PARENT_TAGS = ['xml', 'package', 'metadata', 'dc-metadata', 'x-metadata', 'manifest', 'spine', 'tours', 'guide', 'bindings']

class OPFParser(object):

    def __init__(self, opfdata):
        self.opf = opfdata
        self.opos = 0
        self.lang = "en"
        self.id2href= {}
        self.ncxpath = None
        self.spine=[]
        self.guide=[]
        self._parseData()

    # OPF tag iterator
    def _opf_tag_iter(self):
        tcontent = last_tattr = None
        prefix = []
        while True:
            text, tag = self._parseopf()
            if text is None and tag is None:
                break
            if text is not None:
                tcontent = text.rstrip(" \r\n")
            else: # we have a tag
                ttype, tname, tattr = self._parsetag(tag)
                if ttype == "begin":
                    tcontent = None
                    prefix.append(tname)
                    if tname in _OPF_PARENT_TAGS:
                        yield ".".join(prefix), tname, tattr, tcontent
                    else:
                        last_tattr = tattr
                else: # single or end
                    if ttype == "end":
                        prefix.pop()
                        tattr = last_tattr
                        if tattr is None:
                            tattr = {}
                        last_tattr = None
                    elif ttype == 'single':
                        tcontent = None
                    if ttype == 'single' or (ttype == 'end' and tname not in _OPF_PARENT_TAGS):
                        yield ".".join(prefix), tname, tattr, tcontent
                    tcontent = None

    # now parse the OPF to extract manifest, spine , and metadata
    def _parseData(self):
        cnt = 0
        for prefix, tname, tattr, tcontent in self._opf_tag_iter():
            # metadata for language
            if tname in ["meta", "link"] or tname.startswith("dc:") and "metadata" in prefix:
                if tname == "dc:language":
                    self.lang = tcontent
                continue
            # manifest for ncxpath and idtohref map
            if tname == "item" and  prefix.endswith("manifest"):
                nid = "xid%03d" %  cnt
                cnt += 1
                id = tattr.pop("id",nid)
                href = tattr.pop("href","")
                mtype = tattr.pop("media-type","")
                href = unquoteurl(href)
                self.id2href[id] = href
                if mtype == "application/x-dtbncx+xml":
                    self.ncxpath = href
                continue
            # spine
            if tname == "itemref" and prefix.endswith("spine"):
                idref = tattr.pop("idref","")
                self.spine.append((idref, tattr))
                continue
            # guide
            if tname == "reference" and  prefix.endswith("guide"):
                type = tattr.pop("type","")
                title = tattr.pop("title","")
                href = unquoteurl(tattr.pop("href",""))
                self.guide.append((type, title, href))
                continue

    # parse and return either leading text or the next tag
    def _parseopf(self):
        p = self.opos
        if p >= len(self.opf):
            return None, None
        if self.opf[p] != '<':
            res = self.opf.find('<',p)
            if res == -1 :
                res = len(self.opf)
            self.opos = res
            return self.opf[p:res], None
        # handle comment as a special case
        if self.opf[p:p+4] == '<!--':
            te = self.opf.find('-->',p+1)
            if te != -1:
                te = te+2
        else:
            te = self.opf.find('>',p+1)
            ntb = self.opf.find('<',p+1)
            if ntb != -1 and ntb < te:
                self.opos = ntb
                return self.opf[p:ntb], None
        self.opos = te + 1
        return None, self.opf[p:te+1]

    # parses tag to identify:  [tname, ttype, tattr]
    #    tname: tag name,    ttype: tag type ('begin', 'end' or 'single');
    #    tattr: dictionary of tag atributes
    def _parsetag(self, s):
        n = len(s)
        p = 1
        tname = None
        ttype = None
        tattr = {}
        while p < n and s[p:p+1] == ' ' : p += 1
        if s[p:p+1] == '/':
            ttype = 'end'
            p += 1
            while p < n and s[p:p+1] == ' ' : p += 1
        b = p
        while p < n and s[p:p+1] not in ('>', '/', ' ', '"', "'","\r","\n") : p += 1
        tname=s[b:p].lower()
        # remove redundant opf: namespace prefixes on opf tags
        if tname.startswith("opf:"):
            tname = tname[4:]
        # some special cases
        # handle comments with no spaces to delimit 
        if tname.startswith("!--"):
            tname = "!--"
            ttype = 'single'
            comment = s[4:-3].strip()
            tattr['comment'] = comment
        if tname == "?xml":
            tname = "xml"
        if ttype is None:
            # parse any attributes of begin or single tags
            while s.find('=',p) != -1 :
                while p < n and s[p:p+1] == ' ' : p += 1
                b = p
                while p < n and s[p:p+1] != '=' : p += 1
                aname = s[b:p].lower()
                aname = aname.rstrip(' ')
                p += 1
                while p < n and s[p:p+1] == ' ' : p += 1
                if s[p:p+1] in ('"', "'") :
                    qt = s[p:p+1]
                    p = p + 1
                    b = p
                    while p < n and s[p:p+1] != qt: p += 1
                    val = s[b:p]
                    p += 1
                else :
                    b = p
                    while p < n and s[p:p+1] not in ('>', '/', ' ') : p += 1
                    val = s[b:p]
                tattr[aname] = val
        if ttype is None:
            ttype = 'begin'
            if s.find('/',p) >= 0:
                ttype = 'single'
        return ttype, tname, tattr

    def get_id2hrefmap(self):
        return self.id2href

    def get_spine(self): 
        # (idref, attr)
        return self.spine

    def get_guide(self):
        # (gtype, gtitle, ghref)
        return self.guide

    def get_ncxpath(self):
        return self.ncxpath

    def get_lang(self):
        return self.lang

def main():
    print("I reached main when I should not have\n")
    return -1
    
if __name__ == '__main__':
    sys.exit(main())
