#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vim:ts=4:sw=4:softtabstop=4:smarttab:expandtab                                                                               

from __future__ import unicode_literals, division, absolute_import, print_function

import sys
import os
from quickparser import QuickXHTMLParser

class ContainerException(Exception):
    pass

class BookContainer(object):

    def __init__(self, wrapper,  debug = False):
        self._debug = debug
        self._w = wrapper
        self.qp=QuickXHTMLParser()

# OPF Acess and Manipulation Routines

# toc and pagemap access routines

    def gettocid(self):
        return self._w.gettocid()

    def getpagemapid(self):
        return self._w.getpagemapid()

# spine get/set and access routines

    def getspine(self):
        # spine is an ordered list of tuples (id, linear) 
        return self._w.getspine()

    def setspine(self, new_spine):
        # new_spine must be an ordered list of tuples (id, linear)
        self._w.setspine(new_spine)

    def spine_insert_before(self, pos, spid, linear):
        self._w.spine_insert_before(pos, spid, linear)

    def getspine_ppd(self):
        # spine_ppd is utf-8 string of page direction (rtl, ltr, None)
        return self._w.getspine_ppd()

    def setspineppd(self, ppd):
        # new pagedirection string
        self._w.setspineppd(ppd)


# guide get/set

    def getguide(self):
        # guide is an ordered list of tuples (type, title, href)
        return self._w.guide

    def setguide(self, new_guide):
        # new_guide must be an ordered list of tupes (type, title, href)
        self._w.setguide(new_guide)

# metadata get/set

    def getmetadataxml(self):
        # returns a utf-8 encoded metadata xml fragement
        return self._w.getmetadataxml()

    def setmetadataxml(self, new_metadata):
        # new_metadata must be a metadata xml fragmment
        self._w.setmetadataxml(new_metadata)

# package tag get/set

    def getpackagetag(self):
        # returns a utf-8 encoded metadata xml fragement
        return self._w.getpackagetag()

    def setpackagetag(self, new_tag):
        # new_tag must be a xml package tag
        self._w.setpackagetag(new_tag)


# reading / writing / adding / deleting files in the opf manifest

    def readfile(self, id):
        # returns the contents of the file with manifest id  (text files are utf-8 encoded)
        return self._w.readfile(id)

    def writefile(self, id, data):
        # writes data to a currently existing file pointed to by the manifest id
        self._w.writefile(id, data)

    def addfile(self, uniqueid, basename, data, mime=None):
        # creates a new file in the manifest with unique manifest id, basename, data, and mimetype
        self._w.addfile(uniqueid, basename, data, mime)
        
        
    def deletefile(self, id):
        # removes the file associated with that manifest id, removes any existing spine entries as well
        self._w.deletefile(id)


# reading / writing / adding / deleting other ebook files that DO NOT exist in the opf manifest

    def readotherfile(self, book_href):
        # returns the contents of the file pointed to by the ebook href
        return self._w.readotherfile(book_href)

    def writeotherfile(self, book_href, data):
        # writes data to a currently existing file pointed to by the ebook href
        self._w.writeotherfile(book_href, data)

    def addotherfile(self, book_href, data):
        # creates a new file with desired ebook href
        self._w.addotherfile(book_href, data)
        
    def deleteotherfile(self, book_href):
        # removes file pointed to by the ebook href
        self._w.deleteotherfile(book_href)


# iterators

    def text_iter(self):
        # yields manifest id, href
        for id in sorted(self._w.id_to_mime):
            mime = self._w.id_to_mime[id]
            if mime == 'application/xhtml+xml':
                href = self._w.id_to_href[id]
                yield id, href

    def css_iter(self):
        # yields manifest id, href
        for id in sorted(self._w.id_to_mime):
            mime = self._w.id_to_mime[id]
            if mime == 'text/css':
                href = self._w.id_to_href[id]
                yield id, href

    def image_iter(self):
        # yields manifest id, href, and mimetype
        for id in sorted(self._w.id_to_mime):
            mime = self._w.id_to_mime[id]
            if mime.startswith('image'):
                href = self._w.id_to_href[id]
                yield id, href, mime

    def font_iter(self):
        # yields manifest id, href, and mimetype
        for id in sorted(self._w.id_to_mime):
            mime = self._w.id_to_mime[id]
            if mime.find('font-') > -1 or mime.endswith('-ttf') or mime.endswith('truetype') or mime.endswith('opentype'):
                href = self._w.id_to_href[id]
                yield id, href, mime

    def manifest_iter(self):
        # yields manifest id, href, and mimetype
        for id in sorted(self._w.id_to_mime):
            mime = self._w.id_to_mime[id]
            href = self._w.id_to_href[id]
            yield id, href, mime

    def spine_iter(self):
        # yields spine idref, linear(yes,no,None), href in spine order 
        for (id , linear) in self._w.spine:
            href = self._w.id_to_href[id]
            yield id, linear, href

    def guide_iter(self):
        # yields guide reference type, title, href, and manifest id of href  
        for (type, title, href) in self._w.guide:
            thref = href.split('#')[0]
            id = self._w.href_to_id.get(thref, None)
            yield type, title, href, id

    def media_iter(self):
        # yields manifest, title, href, and manifest id of href  
        for id in sorted(self._w.id_to_mime):
            mime = self._w.id_to_mime[id]
            if mime.startswith('audio') or mime.startswith('video'):
                href = self._w.id_to_href[id]
                yield id, href, mime
        
    def other_iter(self):
        # yields otherid for each file not in the manifest
        for book_href in self._w.other:
            yield book_href


    # miscellaneous routines

    # build the current opf incorporating all changes to date and return it
    def get_opf(self):
        return self._w.build_opf()

    # create your own current copy of all ebook contents in destintation directory
    def copy_book_contents_to(self, destdir):
        self._w.copy_book_contents_to(destdir)


    # functions for converting from  manifest id to href, basename, mimetype etc
    def href_to_id(self, href, ow=None):
        return self._w.map_href_to_id(href, ow)

    def id_to_mime(self, id, ow=None):
        return self._w.map_id_to_mime(id, ow)

    def basename_to_id(self, basename, ow=None):
        return self._w.map_basename_to_id(basename, ow)
        
    def id_to_href(self, id, ow=None):
        return self._w.map_id_to_href(id, ow)

    def href_to_basename(self, href, ow=None):
        if basename is not None:
            return href.split('/')[-1]
        return ow

