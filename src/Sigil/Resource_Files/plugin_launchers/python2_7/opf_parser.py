#!/usr/bin/env python
# vim:ts=4:sw=4:softtabstop=4:smarttab:expandtab

import sys, os
from utf8_utils import utf8_str
from path import pathof
from path import unquoteurl

_OPF_PARENT_TAGS = ['xml', 'package', 'metadata', 'dc-metadata', 'x-metadata', 'manifest', 'spine', 'tours', 'guide']

class Opf_Parser(object):

    def __init__(self, opf_path, debug = False):
        self._debug = debug
        self.opfname = utf8_str(os.path.basename(opf_path))
        self.opf = None
        with open(pathof(opf_path),'rb') as fp:
            self.opf = fp.read()
        self.opos = 0
        self.package_tag = [None, None]
        # self.package_version = None
        self.metadata_tag = [None, None]
        self.metadata = []
        self.cover_id = None
        self.manifest_id_to_href = {}
        self.manifest_id_to_mime = {}
        self.href_to_manifest_id ={}
        self.spine_ppd = None
        # self.spine_pageattributes = {}
        # self.spine_idrefs = {}
        self.spine = []
        self.guide = []
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
        for prefix, tname, tattr, tcontent in self._opf_tag_iter():
            if self._debug:
                print "   Parsing OPF: ", prefix, tname, tattr, tcontent
            # package
            if tname == "package":
                self.package_tag = [tname, tattr]
            # metadata
            if tname == "metadata":
                self.metadata_tag = [tname, tattr]
            if tname == "meta" or tname.startswith("dc:") and "metadata" in prefix:
                self.metadata.append([tname, tattr, tcontent])
                if tattr.get("name","") == "cover":
                    self.cover_id = tattr.get("content",None)
            # manifest
            if tname == "item" and  prefix.endswith("manifest"):
                id = tattr.pop("id",'')
                href = tattr.pop("href",'')
                mtype = tattr.pop("media-type",'')
                href = unquoteurl(href)
                self.manifest_id_to_href[id] = href
                self.manifest_id_to_mime[id] = mtype
                self.href_to_manifest_id[href] = id
            # spine
            if tname == "spine":
                if tattr is not None:
                    self.spine_ppd = tattr.get("page-progession-direction", None)
            if tname == "itemref" and prefix.endswith("spine"):
                idref = tattr.pop("idref", None)
                linear = tattr.pop("linear", None)
                self.spine.append((idref,linear))
                # ver 3 allows page properites per page
                # remove id since may be confusing
                # if "id" in tattr.keys():
                #     del tattr["id"]
                # if "properties in tattr.keys():
                #     self.spine_pageattributes[idref] = tattr

            # guide
            if tname == "reference" and  prefix.endswith("guide"):
                type = tattr.pop("type",'')
                title = tattr.pop("title",'')
                href = unquoteurl(tattr.pop("href",''))
                self.guide.append((type, title, href))

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
        p = 1
        tname = None
        ttype = None
        tattr = {}
        while s[p:p+1] == ' ' : p += 1
        if s[p:p+1] == '/':
            ttype = 'end'
            p += 1
            while s[p:p+1] == ' ' : p += 1
        b = p
        while s[p:p+1] not in ('>', '/', ' ', '"', "'","\r","\n") : p += 1
        tname=s[b:p].lower()
        # some special cases
        if tname == "?xml":
            tname = "xml"
        if tname == "!--":
            ttype = 'single'
            comment = s[p:-3].strip()
            tattr['comment'] = comment
        if ttype is None:
            # parse any attributes of begin or single tags
            while s.find('=',p) != -1 :
                while s[p:p+1] == ' ' : p += 1
                b = p
                while s[p:p+1] != '=' : p += 1
                aname = s[b:p].lower()
                aname = aname.rstrip(' ')
                p += 1
                while s[p:p+1] == ' ' : p += 1
                if s[p:p+1] in ('"', "'") :
                    p = p + 1
                    b = p
                    while s[p:p+1] not in ('"', "'"): p += 1
                    val = s[b:p]
                    p += 1
                else :
                    b = p
                    while s[p:p+1] not in ('>', '/', ' ') : p += 1
                    val = s[b:p]
                tattr[aname] = val
        if ttype is None:
            ttype = 'begin'
            if s.find('/',p) >= 0:
                ttype = 'single'
        return ttype, tname, tattr

    def taginfo_toxml(self, taginfo):
        res = []
        tname, tattr, tcontent = taginfo
        res.append('<' + tname)
        if tattr is not None:
            for key in tattr.keys():
                res.append(' ' + key + '="'+tattr[key]+'"' )
        if tcontent is not None:
            res.append('>' + tcontent + '</' + tname + '>\n')
        else:
            res.append('/>\n')
        return "".join(res)

    def get_package_tag(self):
        tname, tattr = self.package_tag
        if tname is None:
            return ''
        tag = "<" + tname
        if tattr is not None:
            for key in tattr.keys():
                tag += ' ' + key + '="'+tattr[key]+'"'
        tag += '>\n'
        return tag

    def get_metadataxml(self):
        data = []
        tname, tattr = self.metadata_tag
        if tname is None:
            return ''
        tag = "<" + tname
        if tattr is not None:
            for key in tattr.keys():
                tag += ' ' + key + '="'+tattr[key]+'"'
        tag += '>\n'
        data.append(tag)
        for taginfo in self.metadata:
            data.append(self.taginfo_toxml(taginfo))
        data.append('</metadata>\n')
        return "".join(data)

    def get_manifest_id_to_href_dict(self):
        return self.manifest_id_to_href

    def get_manifest_id_to_mime_dict(self):
        return self.manifest_id_to_mime

    def get_href_to_manifest_id_dict(self):
        return self.href_to_manifest_id

    def get_spine_ppd(self):
        return self.spine_ppd

    # list of (id, linear)
    def get_spine(self):
        return self.spine

    # list of (type, title, href)
    def get_guide(self):
        return self.guide
