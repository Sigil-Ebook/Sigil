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

import sys
import os
from quickparser import QuickXHTMLParser
from preferences import JSONPrefs
from pluginhunspell import HunspellChecker

class ContainerException(Exception):
    pass

class BookContainer(object):

    def __init__(self, wrapper,  debug = False):
        self._debug = debug
        self._w = wrapper
        self.qp=QuickXHTMLParser()
        self.hspell=HunspellChecker(wrapper.get_hunspell_path())
        self.dictionary_dirs=wrapper.get_dictionary_dirs()
        self._prefs_store = JSONPrefs(wrapper.plugin_dir, wrapper.plugin_name)

    def getPrefs(self):
        return self._prefs_store

    def savePrefs(self, user_copy):
        self._prefs_store = user_copy
        self._prefs_store._commit()

    def launcher_version(self):
        return self._w.getversion()

    def epub_version(self):
        return self._w.getepubversion()

    def epub_is_standard(self):
        return self._w.epub_is_standard()

    @property    
    def sigil_ui_lang(self):
        if self._w.sigil_ui_lang is None:
            return 'en'
        return self._w.sigil_ui_lang

    @property
    def sigil_spellcheck_lang(self):
        if self._w.sigil_spellcheck_lang is None:
            return 'en_US'
        return self._w.sigil_spellcheck_lang

# OPF Acess and Manipulation Routines

# toc and pagemap access routines

    def gettocid(self):
        return self._w.gettocid()

    def getpagemapid(self):
        return self._w.getpagemapid()

# nav access routines

    def getnavid(self):
        return self._w.getnavid()

# spine get/set and access routines

    def getspine(self):
        # spine is an ordered list of tuples (id, linear)
        return self._w.getspine()

    def setspine(self, new_spine):
        # new_spine must be an ordered list of tuples (id, linear)
        self._w.setspine(new_spine)

    # New for epub3
    def getspine_epub3(self):
        # spine is an ordered list of tuples (id, linear, properties)
        return self._w.getspine_epub3()

    # New for epub3
    def setspine_epub3(self, new_spine):
        # new_spine must be an ordered list of tuples (id, linear, properties (or None))
        self._w.setspine_epub3(new_spine)

    # Modified for epub3
    # Note: for prepend, set pos = 0
    #       for append, set pos = -1 or pos >= current length of spine
    def spine_insert_before(self, pos, spid, linear, properties=None):
        self._w.spine_insert_before(pos, spid, linear, properties)

    def getspine_ppd(self):
        # spine_ppd is utf-8 string of page direction (rtl, ltr, None)
        return self._w.getspine_ppd()

    def setspine_ppd(self, ppd):
        # new pagedirection string
        self._w.setspine_ppd(ppd)

    # New for epub3
    def setspine_idref_epub3_attributes(idref, linear, properties):
        self._w.setspine_idref_attributes(idref, linear, properties)


# guide get/set

    def getguide(self):
        # guide is an ordered list of tuples (type, title, href)
        return self._w.guide

    def setguide(self, new_guide):
        # new_guide must be an ordered list of tupes (type, title, href)
        self._w.setguide(new_guide)


# bindings get/set access routines

    # New for epub3
    def getbindings_epub3(self):
        # bindings is an ordered list of tuples (media-type, handler)
        return self._w.getbindings_epub3()

    # New for epub3
    def setbindings_epub3(self, new_bindings):
        # new_bindings is an ordered list of tuples (media-type, handler)
        self._w.setbindings_epub3(new_bindings)

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

    # Modified for epub3
    def addfile(self, uniqueid, basename, data, mime=None, properties=None, fallback=None, overlay=None):
        # creates a new file in the manifest with unique manifest id, basename, data, and mimetype
        self._w.addfile(uniqueid, basename, data, mime, properties, fallback, overlay)

    def deletefile(self, id):
        # removes the file associated with that manifest id, removes any existing spine entries as well
        self._w.deletefile(id)

    # New for epub3
    def set_manifest_epub3_attributes(self, id, properties=None, fallback=None, overlay=None):
        # sets the epub3 manifest attrobutes for this manifest id
        self._w.set_manifest_epub3_attributes(id, properties, fallback, overlay)

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
        # yields manifest id, href in spine order plus any non-spine items
        text_set = set([k for k,v in self._w.id_to_mime.items() if v == 'application/xhtml+xml'])
        for (id, linear, properties) in self._w.spine:
            if id in text_set:
                text_set -= set([id])
                href = self._w.id_to_href[id]
                yield id, href
        for id in text_set:
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
            if 'font-' in mime or 'truetype' in mime or 'opentype' in mime or mime.startswith('font/'):
                href = self._w.id_to_href[id]
                yield id, href, mime

    def manifest_iter(self):
        # yields manifest id, href, and mimetype
        for id in sorted(self._w.id_to_mime):
            mime = self._w.id_to_mime[id]
            href = self._w.id_to_href[id]
            yield id, href, mime

    # New for epub3
    def manifest_epub3_iter(self):
        # yields manifest id, href, mimetype, properties, fallback, media-overlay
        for id in sorted(self._w.id_to_mime):
            mime = self._w.id_to_mime[id]
            href = self._w.id_to_href[id]
            properties = self._w.id_to_props[id]
            fallback = self._w.id_to_fall[id]
            overlay = self._w.id_to_over[id]
            yield id, href, mime, properties, fallback, overlay

    def spine_iter(self):
        # yields spine idref, linear(yes,no,None), href in spine order
        for (id , linear, properties) in self._w.spine:
            href = self._w.id_to_href[id]
            yield id, linear, href

    # New for epub3
    def spine_epub3_iter(self):
        # yields spine idref, linear(yes,no,None), properties, href in spine order
        for (id , linear, properties) in self._w.spine:
            href = self._w.id_to_href[id]
            yield id, linear, properties, href


    def guide_iter(self):
        # yields guide reference type, title, href, and manifest id of href
        for (type, title, href) in self._w.guide:
            thref = href.split('#')[0]
            id = self._w.href_to_id.get(thref, None)
            yield type, title, href, id

    # New for epub3
    def bindings_epub3_iter(self):
        # yields media-type handler in bindings order
        for (mtype, handler) in self._w.bindings:
            handler_href = self._w.id_to_href[handler]
            yield mtype, handler, handler_href


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

    def selected_iter(self):
        # yields id type ('other' or 'manifest') and id/otherid for each file selected in the BookBrowser
        for book_href in self._w.selected:
            id_type = 'other'
            id = book_href
            if book_href in self._w.bookpath_to_id:
                id_type = 'manifest'
                id = self._w.bookpath_to_id[book_href]
            yield id_type, id


    # miscellaneous routines

    # build the current opf incorporating all changes to date and return it
    def get_opf(self):
        return self._w.build_opf()

    # create your own current copy of all ebook contents in destintation directory
    def copy_book_contents_to(self, destdir):
        self._w.copy_book_contents_to(destdir)

    # get a list of the directories that contain Sigil's hunspell dictionaries
    def get_dictionary_dirs(self):
        return self._w.get_dictionary_dirs()

    # get status of epub file open inside of Sigil
    def get_epub_is_modified(self):
        return self._w.epub_isDirty

    # get path to currently open epub or an inside Sigil or empty string if unsaved
    def get_epub_filepath(self):
        return self._w.epub_filepath


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
        if href is not None:
            return href.split('/')[-1]
        return ow

    # New for epub3
    def id_to_properties(self, id, ow=None):
        return self._w.map_id_to_props.get(id, ow)

    def id_to_fallback(self, id, ow=None):
        return self._w.map_id_to_fall.get(id, ow)

    def id_to_overlay(self, id, ow=None):
        return self._w.map_id_to_over.get(id, ow)



    # New in Sigil 1.0
    # ----------------

    # A book path (aka bookpath) is a unique relative path from the
    # ebook root to a specific file in the epub.  As a relative path meant
    # to be used in an href or src "link", it only uses forward slashes "/"
    # as path separators.  Since all files exist inside the
    # epub root (folder the epub was unzipped into), bookpaths will NEVER
    # have or use "./" or "../" ie they are in always in canonical form

    # For example under Sigil pre 1.0, all epubs were put into a standard
    # structure.  Under this standard structure book paths would look like
    # the following:
    #   OEBPS/content.opf
    #   OEBPS/toc.ncx
    #   OEBPS/Text/Section0001.xhtml
    #   OEBPS/Images/cover.jpg
    # 

    # and src and hrefs always looked like the following:
    #    from Section0001.xhtml to Section0002.xhtml: ../Text/Section0002.xhtml
    #    from Section0001.xhtml to cover.jpg:         ../Images/cover.jpg
    #    from content.opf to Section0001.xhtml        Text/Section0001.xhtml
    #    from toc.ncx to Section0001.xhtml            Text/Section0001.xhtml

    # Under Sigil 1.0 and later, the original epub structure can be preserved
    # meaning that files like content.opf could be named package.opf, and be placed
    # almost anyplace inside the epub.  This is true for almost all files.

    # So to uniquely identify a file, you need to know the bookpath of the OPF
    # and the manifest href to the specific file, or the path from the epub 
    # root to the file itself (ie. its bookpath)

    # so the Sigil plugin interface for Sigil 1.0 has been extended to allow 
    # the plugin developer to more easily work with bookpaths, create links
    # between bookpaths, etc.

    # we will use the terms book_href (or bookhref) interchangeably
    # with bookpath with the following convention:
    #    - use book_href when working with "other" files outside the manifest
    #    - use bookpath when working with files in the opf manifest
    #    - use either when working with the OPF file as it is at the intersection

    # returns the bookpath/book_href to the opf file
    def get_opfbookpath(self):
        return self._w.get_opfbookpath()

    # returns the book path of the folder containing this bookpath
    def get_startingdir(self, bookpath):
        return self._w.get_startingdir(bookpath)

    # return a bookpath for the file pointed to by the href
    # from the specified bookpath starting directory
    def build_bookpath(self, href, starting_dir):
        return self._w.build_bookpath(href, starting_dir)

    # returns the href relative path from source bookpath to target bookpath
    def get_relativepath(self, from_bookpath, to_bookpath):
        return self._w.get_relativepath(from_bookpath, to_bookpath)

    # adds a new file to the *manifest* with the stated bookpath with the provided
    # uniqueid, data, (and mediatype if specified)
    def addbookpath(self, uniqueid, bookpath, data, mime=None):
        return self._w.addbookpath(uniqueid, bookpath, data, mime)

    # functions for converting from  manifest id to bookpath and back
    def bookpath_to_id(self, bookpath, ow=None):
        return self._w.map_bookpath_to_id(bookpath,ow)

    def id_to_bookpath(self, id, ow=None):
        return self._w.map_id_to_bookpath(id, ow)

    # valid groups: Text, Styles, Images, Fonts, Audio, Video, ncx, opf, Misc
    # returns a sorted folder list of ebook paths for a group
    def group_to_folders(self, group, ow=None):
        return self._w.map_group_to_folders(group, ow)

    def mediatype_to_group(self, mediatype, ow=None):
        return self._w.map_mediatype_to_group(mediatype, ow)
