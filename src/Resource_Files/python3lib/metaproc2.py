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

# For EPUB 2 Metadata

import sys
import os
from metadata_utils import quoteurl, unquoteurl, xmlencode, xmldecode, buildxml, valid_id, OPFMetadataParser

import re

_metadata_pattern = re.compile(r'''(<\s*metadata[^>]*>.*<\s*/\s*metadata\s*>\s*)''', re.IGNORECASE | re.MULTILINE | re.DOTALL)

_DEBUG = False

_recognized_dc = [
    "dc:identifier",
    "dc:title",
    "dc:creator",
    "dc:contributor",
    "dc:source",
    "dc:date",
    "dc:language",
    "dc:coverage",
    "dc:description",
    "dc:format",
    "dc:publisher",
    "dc:relation",
    "dc:rights",
    "dc:subject",
    "dc:type",
]

_skip_meta = [
    "cover",
]

_rec2root = {
    "dc:identifier" : "uid",
    "dc:title"      : "tle",
    "dc:creator"    : "cre",
    "dc:contributor": "con",
    "dc:source"     : "src",
    "dc:date"       : "dat",
    "dc:language"   : "lng",
    "dc:coverage"   : "cov",
    "dc:description": "des",
    "dc:format"     : "fmt",
    "dc:publisher"  : "pub",
    "dc:relation"   : "rln",
    "dc:rights"     : "rgt",
    "dc:subject"    : "sub",
    "dc:type"       : "typ",
}



# define record separator and unit separator
# and set indent that indicates parent/child relationship
_RS = chr(30)
_US = chr(31)
_IN = '  '

class MetadataProcessor(object):

    def __init__(self, opfdata):
        self.opfdata = opfdata
        self.rec = []
        self.pkg = None
        self.other = []
        self.op = None
        self.md = None
        self.id2rec = {}
        self.idlst = []
        self.metadata_attr = {}

    def extract_recognized_metadata(self):
        self.op = OPFMetadataParser(self.opfdata)
        self.md = self.op.get_metadata()
        self.idlst = self.op.get_idlst()
        self.metadata_attr = self.op.get_metadata_attr()
        self.pkg = self.op.get_package()

        # first sort out recognized dc and other metadata
        # while building up id2rec map, and removing id from idlst

        # special case the cover image meta and the unique id meta
        (ver, uid, attr) = self.pkg

        numrec = 0
        for mentry in self.md:
            (mname, mcontent, mattr) = mentry

            if mname == "dc:identifier" and mattr.get("id","") == uid:
                self.other.append(mentry)
                continue
                
            if mname in _recognized_dc:
                self.rec.append(mentry)
                id = mattr.get("id",None)
                if id is not None:
                    self.id2rec[id] = numrec
                    self.idlst.remove(id)
                numrec += 1

            elif mname == "meta" and "name" in mattr and mattr["name"] not in _skip_meta:
                # normal meta tag
                mname = mattr["name"]
                del mattr["name"]
                mcontent = mattr.get("content","")
                del mattr["content"]
                mentry = (mname, mcontent, mattr)
                self.rec.append(mentry)
                id = mattr.get("id",None)
                if id is not None:
                    self.id2rec[id] = numrec
                    self.idlst.remove(id)
                numrec += 1

            else:
                self.other.append(mentry)

        if _DEBUG:
            print("recongized", self.rec)
            print("other", self.other)
            print("idlst", self.idlst)

    # get recognized metadata with included refines as text based tree of 
    # metadata elements with properties/attributes as indented children
    def get_recognized_metadata(self):
        data=[]
        for (dname, dcontent, dattr) in self.rec:
            content = xmldecode(dcontent)
            data.append(dname + _US + content + _RS)
            keys = sorted(list(dattr.keys()))
            for key in keys:
                val = xmldecode(dattr[key])
                data.append(_IN + key + _US + val + _RS)
        return "".join(data)
    
    def get_other_meta_xml(self):
        res = []
        for mentry in self.other:
            res.append('  ' + buildxml(mentry))
        return "".join(res)

    def get_id_list(self):
        return self.idlst;

    def get_metadata_tag(self):
        res = []
        res.append('<metadata')
        if self.metadata_attr is not None:
            for key in self.metadata_attr:
                val = self.metadata_attr[key]
            res.append(' ' + key + '="'+val+'"' )
        res.append('>\n')
        return "".join(res)


# generate a MetaDataProcessor Object and start the extraction
def process_metadata(opfdata):
    mdp = MetadataProcessor(opfdata)
    try: 
        mdp.extract_recognized_metadata()
    except:
        return None
    return mdp


# take edited recognized metdata in text tree form with attributes 
# indented and convert it to structured metadata xml 
def set_new_metadata(data, other, idlst, metatag, opfdata):
    newmd = []
    datalst = data.split(_RS)
    if datalst[-1] == "":
        datalst = datalst[0:-1]
    pos = 0
    cnt = len(datalst)
    while pos < cnt:
        # always starts with a parent who may or may not have any children
        line = datalst[pos]
        mname = None
        mcontent = None
        id = None
        mattr = {}
        (name, value) = line.split(_US)
        mname = name.strip()
        mcontent = value.strip()
        # handle meta tags
        if mname not in _recognized_dc:
            name = mname
            mname = "meta"
            mattr["name"] = name
            mattr["content"] = mcontent
            mcontent = ""
        pos += 1
        # process any children
        while pos < cnt:
            line = datalst[pos]
            if not line.startswith(_IN):
                break
            (name, value) = line.split(_US)
            name = name.strip()
            value = value.strip()
            if name in ["id", "xml:lang", "dir", "opf:scheme", "opf:role", "opf:file-as"]:
                if name == "id":
                    id = valid_id(value, idlst)
                    mattr["id"] = id
                else:
                    mattr[name] = value
            else: 
                # not sure what this attribute is for so simply add it
                mattr[name] = value
            pos += 1

        # add in the metadata element itself
        newmd.append((mname, mcontent, mattr))

    # rebuild entire metadata section of opf
    res = []
    res.append(metatag)
    for mentry in newmd:
        res.append('  ' + buildxml(mentry))
    res.append(other)
    res.append('</metadata>\n')
    newmetadata = "".join(res)
    newopfdata = _metadata_pattern.sub(newmetadata,opfdata)
    return newopfdata
    

def main():
    argv = sys.argv
    if argv[1] is None:
        sys.exit(0)

    if not os.path.exists(argv[1]):
        sys.exit(0)

    opfdata = ""
    with open(argv[1], 'rb') as f:
        opfdata = f.read().decode('utf-8',errors='replace')

    mdp = process_metadata(opfdata)

    if mdp is not None:
        data = mdp.get_recognized_metadata()
        other = mdp.get_other_meta_xml()
        idlist = mdp.get_id_list()
        metatag = mdp.get_metadata_tag()

        # with open('/Users/kbhend/Desktop/epub2metadata.txt','wb') as f:
        #     f.write(data.encode('utf-8'))
    
        print(set_new_metadata(data, other, idlist, metatag, opfdata))

    return 0


if __name__ == '__main__':
    sys.exit(main())
