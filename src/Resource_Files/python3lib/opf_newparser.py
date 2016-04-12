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

import sys, os, codecs
from urllib.parse import unquote

# unquotes url/iri
def unquoteurl(href):
    if isinstance(href,bytes):
        href = href.decode('utf-8')
    href = unquote(href)
    return href

SPECIAL_HANDLING_TAGS = {
    '?xml'     : ('xmlheader', -1), 
    '!--'      : ('comment', -3),
    '!DOCTYPE' : ('doctype', -1),
}

SPECIAL_HANDLING_TYPES = ['xmlheader', 'doctype', 'comment']

_OPF_PARENT_TAGS = ['package', 'metadata', 'dc-metadata', 'x-metadata', 'manifest', 'spine', 'tours', 'guide', 'bindings']

class Opf_Parser(object):

    def __init__(self, opfdata):
        self.opf = opfdata
        self.opos = 0
        self.package = None
        self.metadata_attr = None
        self.metadata = []
        self.manifest = []
        self.spine_attr = None
        self.spine=[]
        self.guide=[]
        self.bindings=[]
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
            # package
            if tname == "package":
                ver = tattr.pop("version", "2.0")
                uid = tattr.pop("unique-identifier","bookid")
                self.package = (ver, uid, tattr)
                continue
            # metadata
            if tname == "metadata":
                self.metadata_attr = tattr
                continue
            if tname in ["meta", "link"] or tname.startswith("dc:") and "metadata" in prefix:
                self.metadata.append((tname, tcontent, tattr))
                continue
            # manifest
            if tname == "item" and  "manifest" in prefix:
                nid = "xid%03d" %  cnt
                cnt += 1
                id = tattr.pop("id",nid)
                href = tattr.pop("href","")
                mtype = tattr.pop("media-type","")
                href = unquoteurl(href)
                self.manifest.append((id, href, mtype, tattr))
                continue
            # spine
            if tname == "spine":
                self.spine_attr = tattr
                continue
            if tname == "itemref" and "spine" in prefix:
                idref = tattr.pop("idref","")
                self.spine.append((idref, tattr))
                continue
            # guide
            if tname == "reference" and  "guide" in prefix:
                type = tattr.pop("type","")
                title = tattr.pop("title","")
                href = unquoteurl(tattr.pop("href",""))
                self.guide.append((type, title, href))
                continue
            # bindings
            if tname in ["mediaType", "mediatype"] and "bindings" in prefix:
                mtype = tattr.pop("media-type","")
                handler = tattr.pop("handler","")
                self.bindings.append((mtype, handler))
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
        # handle comment special case as there may be no spaces to 
        # delimit name begin or end 
        if s[b:].startswith('!--'):
            p = b+3
            tname = '!--'
            ttype, backstep = SPECIAL_HANDLING_TAGS[tname]
            tattr['special'] = s[p:backstep].strip()
            return tname, ttype, tattr
        while p < n and s[p:p+1] not in ('>', '/', ' ', '"', "'","\r","\n") : p += 1
        tname=s[b:p].lower()
        # remove redundant opf: namespace prefixes on opf tags
        if tname.startswith("opf:"):
            tname = tname[4:]
        # more special cases
        if tname == '!doctype':
            tname = '!DOCTYPE'
        if tname in SPECIAL_HANDLING_TAGS:
            ttype, backstep = SPECIAL_HANDLING_TAGS[tname]
            tattr['special'] = s[p:backstep]
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

    def xlate_dict(self, attr):
        keylist = []
        vallist = []
        if attr is not None:
            keylist = list(attr.keys());
            for key in keylist:
                vallist.append(attr[key])
        return (keylist, vallist)

    def get_package(self):
        (ver, uid, attr) = self.package
        (keylist, vallist) = self.xlate_dict(attr)
        return (ver, uid, keylist, vallist)

    def get_manifest(self):
        manlist = []
        for (id, href, mtype, attr) in self.manifest:
            (keylist, vallist) = self.xlate_dict(attr)
            manlist.append((id, href, mtype, keylist, vallist))
        return manlist

    def get_spine_attr(self):
        (keylist, vallist) = self.xlate_dict(self.spine_attr)
        return (keylist, vallist)

    def get_spine(self):
        spine = []
        for (idref, attr) in self.spine:
            (keylist, vallist) = self.xlate_dict(attr)
            spine.append((idref, keylist, vallist))
        return spine

    def get_guide(self):
        guide = []
        for (gtype, gtitle,  ghref) in self.guide:
            guide.append((gtype, gtitle, ghref))
        return guide

    def get_bindings(self):
        bindings = []
        for (mtype, handler) in self.bindings:
            bindings.append((mtype, handler))
        return bindings

    def get_metadata_attr(self):
        (keylist, vallist) = self.xlate_dict(self.metadata_attr)
        return (keylist, vallist)

    def get_metadata(self):
        metadata = []
        for (mname, mcontent, attr) in self.metadata:
            (keylist, vallist) = self.xlate_dict(attr)
            metadata.append((mname, mcontent, keylist, vallist))
        return metadata


def parseopf(opfdata):
    opfparser = Opf_Parser(opfdata)
    return opfparser


def main():
    argv = sys.argv
    if argv[1] is None:
        sys.exit(0)

    if not os.path.exists(argv[1]):
        sys.exit(0)

    with open(argv[1], 'rb') as f:
        data = f.read()
        data = data.decode('utf-8')

    op = parseopf(data)
    print(op.get_package())
    print(op.get_metadata_attr())
    print(op.get_metadata())
    print(op.get_manifest())
    print(op.get_spine_attr())
    print(op.get_spine())
    print(op.get_guide())
    print(op.get_bindings())
    return 0


if __name__ == '__main__':
    sys.exit(main())
