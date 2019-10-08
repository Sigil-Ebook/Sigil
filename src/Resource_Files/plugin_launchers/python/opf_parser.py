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

import sys, os
from unipath import pathof
from hrefutils import unquoteurl, buildBookPath, startingDir, longestCommonPath
from hrefutils import ext_mime_map, mime_group_map

SPECIAL_HANDLING_TAGS = {
    '?xml'     : ('xmlheader', -1), 
    '!--'      : ('comment', -3),
    '!DOCTYPE' : ('doctype', -1),
}

SPECIAL_HANDLING_TYPES = ['xmlheader', 'doctype', 'comment']

_OPF_PARENT_TAGS = ['package', 'metadata', 'dc-metadata', 'x-metadata', 'manifest', 'spine', 'tours', 'guide', 'bindings']

def build_short_name(bookpath, lvl):
    pieces = bookpath.split("/")
    if lvl == 1: return pieces.pop()
    n = len(pieces)
    if lvl >= n: return "^" + bookpath
    pieces = pieces[n-lvl:n]
    return "/".join(pieces)
    
class Opf_Parser(object):

    def __init__(self, opf_path, opf_bookpath, debug = False):
        self._debug = debug
        opf_path = pathof(opf_path)
        self.opfname = os.path.basename(opf_path)
        self.opf_bookpath = opf_bookpath
        self.opf_dir = startingDir(opf_bookpath)
        self.opf = None
        with open(opf_path,'rb') as fp:
            self.opf = fp.read().decode('utf-8')
        self.opos = 0
        self.package = None
        self.metadata_attr = None
        self.metadata = []
        self.cover_id = None

        # let downstream invert any invertable dictionaries when needed
        self.manifest_id_to_href = {}
        self.manifest_id_to_bookpath = {}

        # create non-invertable dictionaries
        self.manifest_id_to_mime = {}
        self.manifest_id_to_properties = {}
        self.manifest_id_to_fallback = {}
        self.manifest_id_to_overlay = {}

        # spine and guide
        self.spine = []
        self.spine_ppd = None
        self.guide = []
        self.bindings = []
        
        # determine folder structure
        self.group_folder = {}
        self.group_count = {}
        self.group_folder["epub"] = ['META-INF']
        self.group_count["epub"] = [1]
        self.group_folder["opf"] = [self.opf_dir]
        self.group_count["opf"] = [1]

        # self.bookpaths = []
        # self.bookpaths.append(self.opf_bookpath)
        
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
                tcontent = text.rstrip(" \t\v\f\r\n")
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
            if self._debug:
                print ("   Parsing OPF: ", prefix, tname, tattr, tcontent)
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
                self.metadata.append((tname, tattr, tcontent))
                if tattr.get("name","") == "cover":
                    self.cover_id = tattr.get("content",None)
                continue
            # manifest
            if tname == "item" and "manifest" in prefix:
                nid = "xid%03d" %  cnt
                cnt += 1
                id = tattr.pop("id", nid)
                href = tattr.pop("href",'')
                mtype = tattr.pop("media-type",'')
                if mtype == "text/html":
                    mtype = "application/xhtml+xml"
                if mtype not in mime_group_map:
                    print("****Opf_Parser Warning****: Unknown MediaType: ",mtype)
                href = unquoteurl(href)
                properties = tattr.pop("properties",None)
                fallback = tattr.pop("fallback",None)
                overlay = tattr.pop("media-overlay",None)

                # external resources are now allowed in the opf under epub3
                # we can ignore fragments here as these are links to files
                self.manifest_id_to_href[id] = href

                bookpath = ""
                if href.find(":") == -1:
                    bookpath = buildBookPath(href, self.opf_dir)
                self.manifest_id_to_bookpath[id] = bookpath
                self.manifest_id_to_mime[id] = mtype
                # self.bookpaths.append(bookpath)
                group = mime_group_map.get(mtype,'')
                if bookpath != "" and group != "":
                    folderlst = self.group_folder.get(group,[])
                    countlst = self.group_count.get(group,[])
                    sdir = startingDir(bookpath)
                    if sdir not in folderlst:
                        folderlst.append(sdir)
                        countlst.append(1)
                    else:
                        pos = folderlst.index(sdir)
                        countlst[pos] = countlst[pos] + 1
                    self.group_folder[group] = folderlst
                    self.group_count[group] = countlst
                self.manifest_id_to_properties[id] = properties
                self.manifest_id_to_fallback[id] = fallback
                self.manifest_id_to_overlay[id] = overlay
                continue
            # spine
            if tname == "spine":
                if tattr is not None:
                    self.spine_ppd = tattr.get("page-progression-direction", None)
                continue
            if tname == "itemref" and "spine" in prefix:
                idref = tattr.pop("idref", "")
                linear = tattr.pop("linear", None)
                properties = tattr.pop("properties", None)
                self.spine.append((idref, linear, properties))
                continue
            # guide
            if tname == "reference" and  "guide" in prefix:
                type = tattr.pop("type",'')
                title = tattr.pop("title",'')
                href = unquoteurl(tattr.pop("href",''))
                self.guide.append((type, title, href))
                continue
            # bindings (stored but ignored for now)
            if tname in ["mediaType", "mediatype"] and "bindings" in prefix:
                mtype = tattr.pop("media-type","")
                handler = tattr.pop("handler","")
                self.bindings.append((mtype, handler))
                continue

        # determine unique ShortPathName for each bookpath
        # start with filename and work back up the folders
        # spn = {}
        # dupset = set()
        # nameset = {}
        # lvl = 1
        # for bkpath in self.bookpaths:
        #     aname = build_short_name(bkpath, lvl)
        #     spn[bkpath] = aname
        #     if aname in nameset:
        #         dupset.add(aname)
        #         nameset[aname].append(bkpath)
        #     else:
        #         nameset[aname]=[bkpath]
        #
        # now work just through any to-do list of duplicates
        # until all duplicates are gone
        #
        # todolst = list(dupset)
        # while(todolst):
        #     dupset = set()
        #     lvl += 1
        #     for aname in todolst:
        #         bklst = nameset[aname]
        #         del nameset[aname]
        #         for bkpath in bklst:
        #             newname = build_short_name(bkpath, lvl)
        #             spn[bkpath] = newname
        #             if newname in nameset:
        #                 dupset.add(newname)
        #                 nameset[newname].append(bkpath)
        #             else:
        #                 nameset[newname] = [bkpath]
        #     todolst = list(dupset)

        # finally sort by number of files in dir to find default folders for each group
        dirlst = []
        use_lower_case = False
        for group in self.group_folder.keys():
            folders = self.group_folder[group]
            cnts = self.group_count[group]
            folders = [x for _,x in sorted(zip(cnts,folders), reverse=True)]
            self.group_folder[group] = folders
            if group in ["Text", "Styles", "Images", "Audio", "Fonts", "Video", "Misc"]:
                afolder = folders[0]
                if afolder.find(group.lower()) > -1:
                    use_lower_case = True
                dirlst.append(folders[0])

        # now back fill any missing values
        # commonbase will end with a / if it exists
        commonbase = longestCommonPath(dirlst)
        for group in ["Styles", "Images", "Audio", "Fonts", "Video", "Misc"]:
            folders = self.group_folder.get(group,[])
            gname = group
            if use_lower_case:
                gname = gname.lower()
            if not folders:
                folders = [commonbase + gname]
                self.group_folder[group] = folders
      

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

    def handle_quoted_attribute_values(self, value):
        if '"' in value:
            value = value.replace('"', "&quot;")
        return value
        
    def taginfo_toxml(self, taginfo):
        res = []
        tname, tattr, tcontent = taginfo
        res.append('<' + tname)
        if tattr is not None:
            for key in tattr:
                val = self.handle_quoted_attribute_values(tattr[key])
                res.append(' ' + key + '="'+val+'"' )
        if tcontent is not None:
            res.append('>' + tcontent + '</' + tname + '>\n')
        else:
            res.append('/>\n')
        return "".join(res)

    def get_epub_version(self):
        (ver, uid, tattr) = self.package
        return ver

    def get_package_tag(self):
        (ver, uid, tattr) = self.package
        packout = []
        packout.append('<package version="%s" unique-identifier="%s"' % (ver, uid))
        if tattr is not None:
            for key in tattr:
                val = self.handle_quoted_attribute_values(tattr[key])
                packout.append(' %s="%s"' % (key, val))
        packout.append(">\n")
        return "".join(packout)

    def get_metadataxml(self):
        data = []
        tattr = self.metadata_attr
        tag = "<metadata"
        if tattr is not None:
            for key in tattr:
                val = self.handle_quoted_attribute_values(tattr[key])
                tag += ' ' + key + '="'+val+'"'
        tag += '>\n'
        data.append(tag)
        for taginfo in self.metadata:
            data.append(self.taginfo_toxml(taginfo))
        data.append('</metadata>\n')
        return "".join(data)

    def get_metadata_attr(self):
        return self.metadata_attr

    # list of (tname, tattr, tcontent) 
    def get_metadata(self):
        return self.metadata

    def get_manifest_id_to_href_dict(self):
        return self.manifest_id_to_href

    def get_manifest_id_to_mime_dict(self):
        return self.manifest_id_to_mime

    def get_manifest_id_to_bookpath_dict(self):
        return self.manifest_id_to_bookpath

    def get_manifest_id_to_properties_dict(self):
        return self.manifest_id_to_properties

    def get_manifest_id_to_fallback_dict(self):
        return self.manifest_id_to_fallback

    def get_manifest_id_to_overlay_dict(self):
        return self.manifest_id_to_overlay

    def get_spine_ppd(self):
        return self.spine_ppd

    # list of (idref, linear, properties)
    def get_spine(self):
        return self.spine

    # list of (type, title, href)
    def get_guide(self):
        return self.guide

    # list of (media-type, handler)
    def get_bindings(self):
        return self.bindings

    def get_group_paths(self):
        return self.group_folder



def main():
    argv = sys.argv

    opfpath = None
    data = None
    if len(argv) > 1:
        filepath = argv[1]
    if filepath:
        op = Opf_Parser(filepath, 'OEBPS/content.opf')

        print('Epub Version')
        print(op.get_epub_version())
        print (' ')

        print('Package Tag')
        print(op.get_package_tag())
        print (' ')

        print('Metadata')
        print(op.get_metadataxml())
        print (' ')

        print('Guide')
        for gentry in op.get_guide():
            print('    ',gentry)
        print (' ')

        print('Spine')
        for spentry in op.get_spine():
            print('    ',spentry)
        print (' ')

        print('Dict: id to href')
        d = op.get_manifest_id_to_href_dict()
        for k, v in d.items():
            print('    ', k, ' : ', v)
        print (' ')

        print('Dict: id to mime')
        d = op.get_manifest_id_to_mime_dict()
        for k, v in d.items():
            print('    ', k, ' : ', v)
        print (' ')

        print('Dict: id to bookpath')
        d = op.get_manifest_id_to_bookpath_dict()
        for k, v in d.items():
            print('    ', k, ' : ', v)
        print (' ')

        print('Dict: id to properties')
        d = op.get_manifest_id_to_properties_dict()
        for k, v in d.items():
            print('    ', k, ' : ', v)
        print (' ')

        print('Dict: id to overlay')
        d = op.get_manifest_id_to_overlay_dict()
        for k, v in d.items():
            print('    ', k, ' : ', v)
        print (' ')

        print('Group Paths')
        d = op.get_group_paths()
        for k, v in d.items():
            print('    ', k, ' : ', v)
        print (' ')

    return 0

if __name__ == '__main__':
    sys.exit(main())
