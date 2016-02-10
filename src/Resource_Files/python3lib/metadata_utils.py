#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# vim:ts=4:sw=4:softtabstop=4:smarttab:expandtab

# Copyright (c) 2016 Kevin B. Hendricks, Stratford, and Doug Massay
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
import os

from urllib.parse import unquote
from urllib.parse import urlsplit

ASCII_CHARS   = set(chr(x) for x in range(128))
URL_SAFE      = set('ABCDEFGHIJKLMNOPQRSTUVWXYZ'
                    'abcdefghijklmnopqrstuvwxyz'
                    '0123456789' '#' '_.-/~')
IRI_UNSAFE = ASCII_CHARS - URL_SAFE

# returns a quoted IRI (not a URI)
def quoteurl(href):
    if isinstance(href,bytes):
        href = href.decode('utf-8')
    (scheme, netloc, path, query, fragment) = urlsplit(href, scheme="", allow_fragments=True)
    if scheme != "":
        scheme += "://"
        href = href[len(scheme):]
    result = []
    for char in href:
        if char in IRI_UNSAFE:
            char = "%%%02x" % ord(char)
        result.append(char)
    return scheme + ''.join(result)

# unquotes url/iri
def unquoteurl(href):
    if isinstance(href,bytes):
        href = href.decode('utf-8')
    href = unquote(href)
    return href

# encode to make xml safe
def xmlencode(data):
    newdata = data
    newdata.replace('&', '&amp;')
    newdata.replace('<', '&lt;')
    newdata.replace('>', '&gt;')
    newdata.replace('"', '&quot;')
    return newdata

#decode xml encoded strings
def xmldecode(data):
    newdata = data
    newdata.replace('&quot;', '"')
    newdata.replace('&gt;', '>')
    newdata.replace('&lt;', '<')
    newdata.replace('&amp;', '&')
    return newdata

def buildxml(mentry):
    tname, tcontent, tattr = mentry
    if tname is None:
        return ""
    tag = []
    tag.append('<' + tname)
    if tattr is not None:
        for key in tattr:
            val = tattr[key]
            val.replace('"','&quot;')
            tag.append(' ' + key + '="'+val+'"' )
    if tcontent is not None:
        tag.append('>' + xmlencode(tcontent) + '</' + tname + '>\n')
    else:
        tag.append(' />\n')
    return "".join(tag)

def valid_id(id, idlst):
    pos = 1
    nid = id
    while nid in idlst:
        nid = id + "%03d" % pos
        pos += 1
    return nid


_OPF_PARENT_TAGS = ['xml', 'package', 'metadata', 'dc-metadata', 'x-metadata', 'manifest', 'spine', 'tours', 'guide','bindings']

class OPFMetadataParser(object):

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
        self.idlst = []
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
                if "id" in tattr:
                    self.idlst.append(tattr["id"])
                continue
            # metadata
            if tname == "metadata":
                self.metadata_attr = tattr
                if "id" in tattr:
                    self.idlst.append(tattr["id"])
                continue
            if tname in ["meta", "link"] or tname.startswith("dc:") and "metadata" in prefix:
                self.metadata.append((tname, tcontent, tattr))
                if "id" in tattr:
                    self.idlst.append(tattr["id"])
                continue
            # manifest
            if tname == "item" and  prefix.endswith("manifest"):
                nid = "xid%03d" %  cnt
                cnt += 1
                id = tattr.pop("id",nid)
                href = tattr.pop("href","")
                mtype = tattr.pop("media-type","")
                href = unquoteurl(href)
                self.manifest.append((id, href, mtype, tattr))
                self.idlst.append(id)
                continue
            # spine
            if tname == "spine":
                self.spine_attr = tattr
                if "id" in tattr:
                    self.idlst.append(tattr["id"])
                continue
            if tname == "itemref" and prefix.endswith("spine"):
                idref = tattr.pop("idref","")
                self.spine.append((idref, tattr))
                if "id" in tattr:
                    self.idlst.append(tattr["id"])
                continue
            # guide
            if tname == "reference" and  prefix.endswith("guide"):
                type = tattr.pop("type","")
                title = tattr.pop("title","")
                href = unquoteurl(tattr.pop("href",""))
                self.guide.append((type, title, href))
                if "id" in tattr:
                    self.idlst.append(tattr["id"])
                continue
            # bindings
            if tname in ["mediaTypes", "mediatypes"] and prefix.endswith("bindings"):
                mtype = tattr.pop("media-type","")
                handler = tattr.pop("handler","")
                self.bindings.append((mtype, handler))
                if "id" in tattr:
                    self.idlst.append(tattr["id"])
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


    def get_package(self):
        # (ver, uid, attr)
        return self.package

    def get_manifest(self):
        # (id, href, mtype, attr)
        return self.manifest

    def get_spine_attr(self):
        # (attr)
        return self.spine_attr

    def get_spine(self):
        spine = []
        # (idref attr)
        return self.spine

    def get_guide(self):
        # (gtype, gtitle, ghref)
        return self.guide

    def get_bindings(self):
        # (mtype, handler)
        return self.bindings

    def get_metadata_attr(self):
        return self.metadata_attr

    def get_metadata(self):
        # (mname, mcontent, attr)
        return self.metadata

    def get_idlst(self):
        return self.idlst

