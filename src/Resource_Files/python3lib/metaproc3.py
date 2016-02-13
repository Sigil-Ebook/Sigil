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

_recognized_meta = [
    "belongs-to-collection",
    "dcterms:issued",
    "dcterms:created",
    # "dcterms:modified",
    # "rendition:flow",
    # "rendition:layout",
    # "rendition:orientation",
    # "rendition:spread",
    # "rendition_viewport",
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
        self.refines = []
        self.other = []
        self.op = None
        self.md = None
        self.pkg = None
        self.id2rec = {}
        self.idlst = []
        self.metadata_attr = {}

    def extract_recognized_metadata(self):
        self.op = OPFMetadataParser(self.opfdata)
        self.md = self.op.get_metadata()
        self.idlst = self.op.get_idlst()
        self.metadata_attr = self.op.get_metadata_attr()
        self.pkg = self.op.get_package()

        # first sort out recognized dc and primary meta from refines, and other metadata
        # while building up id2rec map, and removing id from idlst
        numrec = 0
        (ver, uid, attr) = self.pkg
        for mentry in self.md:
            (mname, mcontent, mattr) = mentry
            
            # do not allow the gui to play with the unique-identifier to 
            # prevent font obfuscation issues later
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
            elif mname == "meta" and "refines" in mattr:
                self.refines.append(mentry)
            elif mname == "meta" and "property" in mattr and mattr["property"] in _recognized_meta:
                # primary meta tag
                property = mattr["property"]
                del mattr["property"]
                mname = property
                mentry = (mname, mcontent, mattr)
                self.rec.append(mentry)
                id = mattr.get("id",None)
                if id is not None:
                    self.id2rec[id] = numrec
                    self.idlst.remove(id)
                numrec += 1
            else:
                self.other.append(mentry)

        # finally convert any refines on metadata to be extra attributes on their target tag
        # all other types of metadata are added to "others" to they are not touched in any way
        for mentry in self.refines:
            (rname, rcontent, rattr) = mentry
            rid = rattr.get("id",None)
            tid = rattr["refines"]
            prop = rattr["property"]
            scheme = rattr.get("scheme", None)
            propval = rcontent
            if tid.startswith("#"):
                tid = tid[1:]
                if tid in self.id2rec:
                    pos = self.id2rec[tid]
                    (dname, dcontent, dattr) = self.rec[pos]
                    dattr[prop] = propval
                    if scheme is not None:
                        dattr["scheme"] = scheme
                    if prop == "alternate-script":
                        dattr["altlang"] = rattr["xml:lang"]
                    self.rec[pos] = (dname, dcontent, dattr)
                    if rid is not None:
                        self.idlst.remove(rid) 
                else:
                    # these refines refer to something that is not recognized metadata
                    self.other.append(mentry)
            else:
                # this is refinement that doesn't seem to point to anything in the opf
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



# generate a MetadataProcessor and extract the metadata from the opf
def process_metadata(opfdata):
    mdp = MetadataProcessor(opfdata)
    try: 
        mdp.extract_recognized_metadata()
    except:
        return None
    return mdp


# take edited recognized metdata in text tree form with properties and attributes 
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
        refines = {}
        (name, value) = line.split(_US)
        mname = name.strip()
        mcontent = value.strip()
        # handle primary metadata
        if mname in _recognized_meta:
            property = mname
            mname = "meta"
            mattr["property"] = property
        pos += 1
        # process any children
        while pos < cnt:
            line = datalst[pos]
            if not line.startswith(_IN):
                break
            (name, value) = line.split(_US)
            name = name.strip()
            value = value.strip()
            if name in ["id", "xml:lang", "dir"]:
                if name == "id":
                    id = valid_id(value, idlst)
                    mattr["id"] = id
                else:
                    mattr[name] = value
            else: 
                # refinement
                refines[name] = value
            pos += 1

        # now ready to rebuild the metadata with separate refinements if needed
        # make sure if refinements needed that a valid id exists
        if refines:
            if not "id" in mattr:
                root = _rec2root.get(mname, "num")
                id = valid_id(root, idlst)
                mattr["id"] = id

        # add in the metadata element itself
        newmd.append((mname, mcontent, mattr))

        # add any needed refinements
        for prop in refines.keys():
            if prop == "scheme":
                continue
            if prop == "altlang":
                continue
            rattr = {}
            rattr["refines"] = "#" + id
            rattr["property"] = prop
            rname = "meta"
            rcontent = refines[prop]
            if prop == "alternate-script":
                if "altlang" in refines:
                    rattr["xml:lang"] = refines["altlang"]
            if prop in ["role", "identifier-type", "title-type","collection-type"]:
                if "scheme" in refines:
                    rattr["scheme"] = refines["scheme"]
            newmd.append((rname, rcontent, rattr))
            
    # rebuild entire metadata section
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
    # try:
    #     mdp.extract_recognized_metadata()
    # except Exception as e:
    #     print("Error: %s\n" % e)
    #     return -1

    if mdp is not None:
        data = mdp.get_recognized_metadata()
        other = mdp.get_other_meta_xml()
        idlist = mdp.get_id_list()
        metatag = mdp.get_metadata_tag()

        # with open('/Users/kbhend/Desktop/metadata.txt','wb') as f:
        #     f.write(data.encode('utf-8'))
    
        print(set_new_metadata(data, other, idlist, metatag, opfdata))

    return 0


if __name__ == '__main__':
    sys.exit(main())
