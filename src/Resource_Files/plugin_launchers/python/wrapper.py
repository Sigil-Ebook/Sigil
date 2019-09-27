#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vim:ts=4:sw=4:softtabstop=4:smarttab:expandtab

# Copyright (c) 2014-2019 Kevin B. Hendricks and Doug Massay
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


from compatibility_utils import PY3, PY2, text_type, binary_type, utf8_str, unicode_str, iswindows

import sys
import os

from hrefutils import quoteurl, unquoteurl, buildBookPath, startingDir, buildRelativePath
from hrefutils import ext_mime_map, mime_group_map

import re
import unipath
from unipath import pathof
import unicodedata

_launcher_version=20190927

_PKG_VER = re.compile(r'''<\s*package[^>]*version\s*=\s*["']([^'"]*)['"][^>]*>''',re.IGNORECASE)

# Wrapper Class is used to peform record keeping for Sigil.  It keeps track of modified,
# added, and deleted files while providing some degree of protection against files under
# Sigil's control from being directly manipulated.
# Uses "write-on-modify" and so removes the need for wholesale copying of files

_guide_types = ['cover','title-page','toc','index','glossary','acknowledgements',
                'bibliography','colophon','copyright-page','dedication',
                'epigraph','foreward','loi','lot','notes','preface','text']

PROTECTED_FILES = [
    'mimetype',
    'META-INF/container.xml',
]

TEXT_MIMETYPES = [
                'image/svg+xml',
                'application/xhtml+xml',
                'text/css',
                'application/x-dtbncx+xml',
                'application/oebps-package+xml',
                'application/oebs-page-map+xml',
                'application/smil+xml',
                'application/adobe-page-template+xml',
                'application/vnd.adobe-page-template+xml',
                'text/javascript',
                'application/javascript'
                'application/pls+xml'
]

class WrapperException(Exception):
    pass

class Wrapper(object):

    def __init__(self, ebook_root, outdir, op, plugin_dir, plugin_name, debug = False):
        self._debug = debug
        self.ebook_root = pathof(ebook_root)
        # plugins and plugin containers can get name and user plugin dir
        self.plugin_dir = pathof(plugin_dir)
        self.plugin_name = plugin_name
        self.outdir = pathof(outdir)

        # initialize the sigil cofiguration info passed in outdir with sigil.cfg
        self.opfbookpath = None
        self.appdir = None
        self.usrsupdir = None
        # Location of directory containing hunspell dictionaries on Linux
        self.linux_hunspell_dict_dirs = []
        # Sigil interface language code
        self.sigil_ui_lang = None
        # Default Sigil spell check dictionary
        self.sigil_spellcheck_lang = None
        # status of epub inside Sigil (isDirty) and CurrentFilePath of current epub file
        self.epub_isDirty = False
        self.epub_filepath = ""
        # File selected in Sigil's Book Browser
        self.selected = []
        cfg = ''
        with open(os.path.join(self.outdir, 'sigil.cfg'), 'rb') as f:
            cfg = f.read().decode('utf-8')
        cfg = cfg.replace("\r", "")
        cfg_lst = cfg.split("\n")
        if len(cfg_lst) >= 7:
            self.opfbookpath = cfg_lst.pop(0)
            self.appdir = cfg_lst.pop(0)
            self.usrsupdir = cfg_lst.pop(0)
            if not sys.platform.startswith('darwin') and not sys.platform.startswith('win'):
                self.linux_hunspell_dict_dirs = cfg_lst.pop(0).split(":")
            self.sigil_ui_lang = cfg_lst.pop(0)
            self.sigil_spellcheck_lang = cfg_lst.pop(0)
            self.epub_isDirty = (cfg_lst.pop(0) == "True")
            self.epub_filepath = cfg_lst.pop(0)
            self.selected = cfg_lst
        os.environ['SigilGumboLibPath'] = self.get_gumbo_path()

        # dictionaries used to map opf manifest information
        self.id_to_href = {}
        self.id_to_mime = {}
        self.id_to_props = {}
        self.id_to_fall = {}
        self.id_to_over = {}
        self.id_to_bookpath = {}
        self.href_to_id = {}
        self.bookpath_to_id = {}
        self.spine_ppd = None
        self.spine = []
        self.guide = []
        self.bindings = []
        self.package_tag = None
        self.epub_version = None
        # self.metadata_attr = None
        # self.metadata = []
        self.metadataxml = ''
        self.op = op
        if self.op is not None:
            # copy in data from parsing of initial opf
            self.opf_dir = op.opf_dir
            self.id_to_href = op.get_manifest_id_to_href_dict().copy()
            self.id_to_mime = op.get_manifest_id_to_mime_dict().copy()
            self.id_to_props = op.get_manifest_id_to_properties_dict().copy()
            self.id_to_fall = op.get_manifest_id_to_fallback_dict().copy()
            self.id_to_over = op.get_manifest_id_to_overlay_dict().copy()
            self.id_to_bookpath = op.get_manifest_id_to_bookpath_dict().copy()
            self.group_paths = op.get_group_paths().copy()
            self.spine_ppd = op.get_spine_ppd()
            self.spine = op.get_spine()
            self.guide = op.get_guide()
            self.package_tag = op.get_package_tag()
            self.epub_version = op.get_epub_version()
            self.bindings = op.get_bindings()
            self.metadataxml = op.get_metadataxml()
            # invert key dictionaries to allow for reverse access
            self.href_to_id = {v: k for k, v in self.id_to_href.items()}
            self.bookpath_to_id = {v: k for k, v in self.id_to_bookpath.items()}
            # self.metadata = op.get_metadata()
            # self.metadata_attr = op.get_metadata_attr()
        self.other = []  # non-manifest file information
        self.id_to_filepath = {}
        self.book_href_to_filepath = {}
        self.modified = {}
        self.added = []
        self.deleted = []

        # walk the ebook directory tree building up initial list of
        # all unmanifested (other) files
        for filepath in unipath.walk(ebook_root):
            book_href = filepath.replace(os.sep, "/")
            # OS X file names and paths use NFD form. The EPUB
            # spec requires all text including filenames to be in NFC form.
            book_href = unicodedata.normalize('NFC', book_href)
            # if book_href file in manifest convert to manifest id
            id = self.bookpath_to_id.get(book_href,None)
            if id is None:
                self.other.append(book_href)
                self.book_href_to_filepath[book_href] = filepath
            else:
                self.id_to_filepath[id] = filepath

    def getversion(self):
        global _launcher_version
        return _launcher_version

    def getepubversion(self):
        return self.epub_version

    # utility routine to get mime from href (book href or opf href)
    def getmime(self,  href):
        href = unicode_str(href)
        href = unquoteurl(href)
        filename = os.path.basename(href)
        ext = os.path.splitext(filename)[1]
        ext = ext.lower()
        return ext_mime_map.get(ext, "")



    # New in Sigil 1.0
    # ----------------

    # A book path (aka "bookpath" or "book_path") is a unique relative path 
    # from the ebook root to a specific file.  As a relative path meant to
    # be used in an href or src link it only uses forward slashes "/"
    # as path segment separators.  Since all files exist inside the
    # epub root (folder the epub was unzipped into), book paths will NEVER
    # have or use "./" or "../" ie they are in always in canonical form

    # We will use the terms book_href (aka "bookhref") interchangeabily
    # with bookpath with the following convention:
    #   - use book_href when working with "other" files outside of the manifest
    #   - use bookpath when working with files in the manifest
    #   - use either when the file in question in the OPF as it exists in the intersection

    # returns the bookpath/book_href to the opf file 
    def get_opfbookpath(self):
        return self.opfbookpath

    # returns the book path to the folder containing this bookpath
    def get_startingdir(self, bookpath):
        bookpath = unicode_str(bookpath)
        return startingDir(bookpath)

    # return a bookpath for the file pointed to by the href from
    # the specified bookpath starting directory
    def build_bookpath(self, href, starting_dir):
        href = unicode_str(href)
        href = unquoteurl(href)
        starting_dir = unicode_str(starting_dir)
        return buildBookPath(href, starting_dir)

    # returns the href relative path from source bookpath to target bookpath
    def get_relativepath(self, from_bookpath, to_bookpath):
        from_bookpath = unicode_str(from_bookpath)
        to_bookpath = unicode_str(to_bookpath)
        return buildRelativePath(from_bookpath, to_bookpath)

    # ----------



    # routines to rebuild the opf on the fly from current information

    def build_package_starttag(self):
        return self.package_tag

    def build_manifest_xml(self):
        manout = []
        manout.append('  <manifest>\n')
        for id in sorted(self.id_to_mime):
            href = quoteurl(self.id_to_href[id])
            mime = self.id_to_mime[id]
            props = ''
            properties = self.id_to_props[id]
            if properties is not None:
                props = ' properties="%s"' % properties
            fall = ''
            fallback = self.id_to_fall[id]
            if fallback is not None:
                fall = ' fallback="%s"' % fallback
            over = ''
            overlay = self.id_to_over[id]
            if overlay is not None:
                over = ' media-overlay="%s"' % overlay
            manout.append('    <item id="%s" href="%s" media-type="%s"%s%s%s />\n' % (id, href, mime, props, fall, over))
        manout.append('  </manifest>\n')
        return "".join(manout)

    def build_spine_xml(self):
        spineout = []
        ppd = ''
        ncx = ''
        map = ''
        if self.spine_ppd is not None:
            ppd = ' page-progression-direction="%s"' % self.spine_ppd
        tocid = self.gettocid()
        if tocid is not None:
            ncx = ' toc="%s"' % tocid
        pagemapid = self.getpagemapid()
        if pagemapid is not None:
            map = ' page-map="%s"' % pagemapid
        spineout.append('  <spine%s%s%s>\n' %(ppd, ncx, map))
        for (id, linear, properties) in self.spine:
            lin = ''
            if linear is not None:
                lin = ' linear="%s"' % linear
            props = ''
            if properties is not None:
                props = ' properties="%s"' % properties
            spineout.append('    <itemref idref="%s"%s%s/>\n' % (id, lin, props))
        spineout.append('  </spine>\n')
        return "".join(spineout)

    def build_guide_xml(self):
        guideout = []
        if len(self.guide) > 0:
            guideout.append('  <guide>\n')
            for (type, title, href) in self.guide:
                href = quoteurl(href)
                guideout.append('    <reference type="%s" href="%s" title="%s"/>\n' % (type, href, title))
            guideout.append('  </guide>\n')
        return "".join(guideout)

    def build_bindings_xml(self):
        bindout = []
        if len(self.bindings) > 0 and self.epub_version.startswith('3'):
            bindout.append('  <bindings>\n')
            for (mtype, handler) in self.bindings:
                bindout.append('    <mediaType media-type="%s" handler="%s"/>\n' % (mtype, handler))
            bindout.append('  </bindings>\n')
        return "".join(bindout)

    def build_opf(self):
        data = []
        data.append('<?xml version="1.0" encoding="utf-8" standalone="yes"?>\n')
        data.append(self.build_package_starttag())
        data.append(self.metadataxml)
        data.append(self.build_manifest_xml())
        data.append(self.build_spine_xml())
        data.append(self.build_guide_xml())
        data.append(self.build_bindings_xml())
        data.append('</package>\n')
        return "".join(data)

    def write_opf(self):
        if self.op is not None:
            platpath = self.opfbookpath.replace('/',os.sep)
            filepath = pathof(os.path.join(self.outdir, platpath))
            base = os.path.dirname(filepath)
            if not unipath.exists(base):
                os.makedirs(base)
            with open(filepath,'wb') as fp:
                data = utf8_str(self.build_opf())
                fp.write(data)


    # routines to help find the manifest id of toc.ncx and page-map.xml

    def gettocid(self):
        for id in self.id_to_mime:
            mime = self.id_to_mime[id]
            if mime == "application/x-dtbncx+xml":
                return id
        return None

    def getpagemapid(self):
        for id in self.id_to_mime:
            mime = self.id_to_mime[id]
            if mime ==  "application/oebs-page-map+xml":
                return id
        return None


    # routines to help find the manifest id of the nav
    def getnavid(self):
        if self.epub_version == "2.0":
            return None
        for id in self.id_to_mime:
            mime = self.id_to_mime[id]
            if mime == "application/xhtml+xml":
                properties = self.id_to_props[id]
                if properties is not None and "nav" in properties:
                    return id
        return None


    # routines to manipulate the spine

    def getspine(self):
        osp = []
        for (sid, linear, properties) in self.spine:
            osp.append((sid, linear))
        return osp

    def setspine(self,new_spine):
        spine = []
        for (sid, linear) in new_spine:
            properties = None
            sid = unicode_str(sid)
            linear = unicode_str(linear)
            if sid not in self.id_to_href:
                raise WrapperException('Spine Id not in Manifest')
            if linear is not None:
                linear = linear.lower()
                if linear not in ['yes', 'no']:
                    raise Exception('Improper Spine Linear Attribute')
            spine.append((sid, linear, properties))
        self.spine = spine
        self.modified[self.opfbookpath] = 'file'

    def getspine_epub3(self):
        return self.spine

    def setspine_epub3(self, new_spine):
        spine = []
        for (sid, linear, properties) in new_spine:
            sid = unicode_str(sid)
            linear = unicode_str(linear)
            properties = unicode_str(properties)
            if properties is not None and properties == "":
                properties = None
            if sid not in self.id_to_href:
                raise WrapperException('Spine Id not in Manifest')
            if linear is not None:
                linear = linear.lower()
                if linear not in ['yes', 'no']:
                    raise Exception('Improper Spine Linear Attribute')
            if properties is not None:
                properties = properties.lower()
            spine.append((sid, linear, properties))
        self.spine = spine
        self.modified[self.opfbookpath] = 'file'

    def getbindings_epub3(self):
        return self.bindings

    def setbindings_epub3(self, new_bindings):
        bindings = []
        for (mtype, handler) in new_bindings:
            mtype = unicode_str(mtype)
            handler = unicode_str(handler)
            if mtype is None or mtype == "":
                continue
            if handler is None or handler == "":
                continue
            if handler not in self.id_to_href:
                raise WrapperException('Handler not in Manifest')
            bindings.append((mtype, handler))
        self.bindings = bindings
        self.modified[self.opfbookpath] = 'file'

    def spine_insert_before(self, pos, sid, linear, properties=None):
        sid = unicode_str(sid)
        linear = unicode_str(linear)
        properties = unicode_str(properties)
        if properties is not None and properties == "":
            properties = None
        if sid not in self.id_to_mime:
            raise WrapperException('that spine idref does not exist in manifest')
        n = len(self.spine)
        if pos == 0:
            self.spine = [(sid, linear, properties)] + self.spine
        elif pos == -1 or pos >= n:
            self.spine.append((sid, linear, properties))
        else:
            self.spine = self.spine[0:pos] + [(sid, linear, properties)] + self.spine[pos:]
        self.modified[self.opfbookpath] = 'file'

    def getspine_ppd(self):
        return self.spine_ppd

    def setspine_ppd(self, ppd):
        ppd = unicode_str(ppd)
        if ppd not in ['rtl', 'ltr', None]:
            raise WrapperException('incorrect page-progression direction')
        self.spine_ppd = ppd
        self.modified[self.opfbookpath] = 'file'

    def setspine_itemref_epub3_attributes(idref, linear, properties):
        idref = unicode_str(idref)
        linear = unicode_str(linear)
        properties = unicode_str(properties)
        if properties is not None and properties == "":
            properties = None
        pos = -1
        i = 0
        for (sid, slinear, sproperties) in self.spine:
            if sid == idref:
                pos = i
                break;
            i += 1
        if pos == -1:
            raise WrapperException('that idref is not exist in the spine')
        self.spine[pos] = (sid, linear, properties)
        self.modified[self.opfbookpath] = 'file'


    # routines to get and set the guide

    def getguide(self):
        return self.guide

    def setguide(self, new_guide):
        guide = []
        for (type, title, href) in new_guide:
            type = unicode_str(type)
            title = unicode_str(title)
            href = unicode_str(href)
            href = unquoteurl(href)
            if type not in _guide_types:
                type = "other." + type
            if title is None:
                title = 'title missing'
            thref = href.split('#')[0]
            if thref not in self.href_to_id:
                raise WrapperException('guide href not in manifest')
            guide.append((type, title, href))
        self.guide = guide
        self.modified[self.opfbookpath] = 'file'


    # routines to get and set metadata xml fragment

    def getmetadataxml(self):
        return self.metadataxml

    def setmetadataxml(self, new_metadata):
        self.metadataxml = unicode_str(new_metadata)
        self.modified[self.opfbookpath] = 'file'


    # routines to get and set the package tag
    def getpackagetag(self):
        return self.package_tag

    def setpackagetag(self, new_packagetag):
        pkgtag = unicode_str(new_packagetag)
        version = ""
        mo = _PKG_VER.search(pkgtag)
        if mo:
            version = mo.group(1)
        if version != self.epub_version:
            raise WrapperException('Illegal to change the package version attribute')
        self.package_tag = pkgtag
        self.modified[self.opfbookpath] = 'file'


    # routines to manipulate files in the manifest (updates the opf automagically)

    def readfile(self, id):
        id = unicode_str(id)
        if id not in self.id_to_href:
            raise WrapperException('Id does not exist in manifest')
        filepath = self.id_to_filepath.get(id, None)
        if filepath is None:
            raise WrapperException('Id does not exist in manifest')
        # already added or modified it will be in outdir
        basedir = self.ebook_root
        if id in self.added or id in self.modified:
            basedir = self.outdir
        filepath = os.path.join(basedir, filepath)
        if not unipath.exists(filepath):
            raise WrapperException('File Does Not Exist')
        data = ''
        with open(filepath,'rb') as fp:
            data = fp.read()
        mime = self.id_to_mime.get(id,'')
        if mime in TEXT_MIMETYPES:
            data = unicode_str(data)
        return data

    def writefile(self, id, data):
        id = unicode_str(id)
        if id not in self.id_to_href:
            raise WrapperException('Id does not exist in manifest')
        filepath = self.id_to_filepath.get(id, None)
        if filepath is None:
            raise WrapperException('Id does not exist in manifest')
        mime = self.id_to_mime.get(id,'')
        filepath = os.path.join(self.outdir, filepath)
        base = os.path.dirname(filepath)
        if not unipath.exists(base):
            os.makedirs(pathof(base))
        if mime in TEXT_MIMETYPES or isinstance(data, text_type):
            data = utf8_str(data)
        with open(filepath,'wb') as fp:
            fp.write(data)
        self.modified[id] = 'file'


    def addfile(self, uniqueid, basename, data, mime=None, properties=None, fallback=None, overlay=None):
        uniqueid = unicode_str(uniqueid)
        if uniqueid in self.id_to_href:
            raise WrapperException('Manifest Id is not unique')
        basename = unicode_str(basename)
        mime = unicode_str(mime)
        if mime is None:
            ext = os.path.splitext(basename)[1]
            ext = ext.lower()
            mime = ext_mime_map.get(ext, None)
        if mime is None:
            raise WrapperException("Mime Type Missing")
        if mime == "application/x-dtbncx+xml" and self.epub_version.startswith("2"):
            raise WrapperException('Can not add or remove an ncx under epub2')
        group = mime_group_map.get(mime,"Misc")
        bookpath = self.group_paths[group] + basename
        href = buildRelativePath(self.opfbookpath, bookpath)
        if href in self.href_to_id:
            raise WrapperException('Basename already exists')
        # now actually write out the new file
        filepath = bookpath.replace("/",os.sep)
        self.id_to_filepath[uniqueid] = filepath
        filepath = os.path.join(self.outdir,filepath)
        base = os.path.dirname(filepath)
        if not unipath.exists(base):
            os.makedirs(base)
        if mime in TEXT_MIMETYPES or isinstance(data, text_type):
            data = utf8_str(data)
        with open(filepath,'wb') as fp:
            fp.write(data)
        self.id_to_href[uniqueid] = href
        self.id_to_mime[uniqueid] = mime
        self.id_to_props[uniqueid] = properties
        self.id_to_fall[uniqueid] = fallback
        self.id_to_over[uniqueid] = overlay
        self.id_to_bookpath[uniqueid] = bookpath
        self.href_to_id[href] = uniqueid
        self.bookpath_to_id[bookpath] = uniqueid
        self.added.append(uniqueid)
        self.modified[self.opfbookpath] = 'file'
        return uniqueid


    # new in Sigil 1.0

    # adds bookpath specified file to the manifest with given uniqueid data, and mime
    def addbookpath(self, uniqueid, bookpath, data, mime=None):
        uniqueid = unicode_str(uniqueid)
        if uniqueid in self.id_to_href:
            raise WrapperException('Manifest Id is not unique')
        bookpath = unicode_str(bookpath)
        basename = bookpath.split("/")[-1]
        mime = unicode_str(mime)
        if mime is None:
            ext = os.path.splitext(basename)[1]
            ext = ext.lower()
            mime = ext_mime_map.get(ext, None)
        if mime is None:
            raise WrapperException("Mime Type Missing")
        if mime == "application/x-dtbncx+xml" and self.epub_version.startswith("2"):
            raise WrapperException('Can not add or remove an ncx under epub2')
        href = buildRelativePath(self.opfbookpath, bookpath)
        if href in self.href_to_id:
            raise WrapperException('bookpath already exists')
        # now actually write out the new file
        filepath = bookpath.replace("/",os.sep)
        self.id_to_filepath[uniqueid] = filepath
        filepath = os.path.join(self.outdir,filepath)
        base = os.path.dirname(filepath)
        if not unipath.exists(base):
            os.makedirs(base)
        if mime in TEXT_MIMETYPES or isinstance(data, text_type):
            data = utf8_str(data)
        with open(filepath,'wb') as fp:
            fp.write(data)
        self.id_to_href[uniqueid] = href
        self.id_to_mime[uniqueid] = mime
        self.id_to_props[uniqueid] = None
        self.id_to_fall[uniqueid] = None
        self.id_to_over[uniqueid] = None
        self.id_to_bookpath[uniqueid] = bookpath
        self.href_to_id[href] = uniqueid
        self.bookpath_to_id[bookpath] = uniqueid
        self.added.append(uniqueid)
        self.modified[self.opfbookpath] = 'file'
        return uniqueid


    def deletefile(self, id):
        id = unicode_str(id)
        if id not in self.id_to_href:
            raise WrapperException('Id does not exist in manifest')
        filepath = self.id_to_filepath.get(id, None)
        if id is None:
            raise WrapperException('Id does not exist in manifest')
        if self.epub_version.startswith("2") and id == self.gettocid():
            raise WrapperException('Can not add or remove an ncx under epub2')
        add_to_deleted = True
        # if file was added or modified, delete file from outdir
        if id in self.added or id in self.modified:
            filepath = os.path.join(self.outdir,filepath)
            if unipath.exists(filepath) and unipath.isfile(filepath):
                os.remove(pathof(filepath))
            if id in self.added:
                self.added.remove(id)
                add_to_deleted = False
            if id in self.modified:
                del self.modified[id]
        # remove from manifest
        href = self.id_to_href[id]
        mime = self.id_to_mime[id]
        bookpath = self.id_to_bookpath[id]
        del self.id_to_href[id]
        del self.id_to_mime[id]
        del self.id_to_props[id]
        del self.id_to_fall[id]
        del self.id_to_over[id]
        del self.id_to_bookpath[id]
        del self.href_to_id[href]
        del self.bookpath_to_id[bookpath]
        # remove from spine
        new_spine = []
        was_modified = False
        for sid, linear, properties in self.spine:
            if sid != id:
                new_spine.append((sid, linear, properties))
            else:
                was_modified = True
        if was_modified:
            self.setspine_epub3(new_spine)
        if add_to_deleted:
            self.deleted.append(('manifest', id, href))
            self.modified[self.opfbookpath] = 'file'
        del self.id_to_filepath[id]

    def set_manifest_epub3_attributes(self, id, properties=None, fallback=None, overlay=None):
        id = unicode_str(id)
        properties = unicode_str(properties)
        if properties is not None and properties == "":
            properties = None
        fallback = unicode_str(fallback)
        if fallback is not None and fallback == "":
            fallback = None
        overlay = unicode_str(overlay)
        if overlay is not None and overlay == "":
            overlay = None
        if id not in self.id_to_hrefs:
            raise WrapperException('Id does not exist in manifest')
        del self.id_to_props[id]
        del self.id_to_fall[id]
        del self.id_to_over[id]
        self.id_to_props[id] = properties
        self.id_to_fall[id] = fallback
        self.id_to_over[id] = overlay
        self.modified[self.opfbookpath] = 'file'


    # helpful mapping routines for file info from the opf manifest

    def map_href_to_id(self, href, ow):
        href = unicode_str(href)
        href = unquoteurl(href)
        return self.href_to_id.get(href,ow)

    # new in Sigil 1.0
    def map_bookpath_to_id(self, bookpath, ow):
        bookpath = unicode_str(bookpath)
        return self.bookpath_to_id.get(bookpath,ow)

    def map_basename_to_id(self, basename, ow):
        for bookpath in self.bookpath_to_id:
            filename = bookpath.split("/")[-1]
            if filename == basename:
                return self.bookpath_to_id[bookpath]
        return ow

    def map_id_to_href(self, id, ow):
        id = unicode_str(id)
        return self.id_to_href.get(id, ow)

    # new in Sigil 1.0
    def map_id_to_bookpath(self, id, ow):
        id = unicode_str(id)
        return self.id_to_bookpath.get(id, ow)

    def map_id_to_mime(self, id, ow):
        id = unicode_str(id)
        return self.id_to_mime.get(id, ow)

    def map_id_to_properties(self, id, ow):
        id = unicode_str(id)
        return self.id_to_props.get(id, ow)

    def map_id_to_fallback(self, id, ow):
        id = unicode_str(id)
        return self.id_to_fall.get(id, ow)

    def map_id_to_overlay(self, id, ow):
        id = unicode_str(id)
        return self.id_to_over.get(id, ow)


    # routines to work on ebook files that are not part of an opf manifest
    # their "id" is actually their unique relative path from book root
    # this is called either a book href or a book path
    # we use book_href or bookhref  when working with "other" files
    # we use bookpath when working with files in the manifest

    def readotherfile(self, book_href):
        id = unicode_str(book_href)
        id = unquoteurl(id)
        if id is None:
            raise WrapperException('None is not a valid book href')
        if id not in self.other and id in self.id_to_href:
            raise WrapperException('Incorrect interface routine - use readfile')
        # handle special case of trying to read the opf after it has been modified
        if id == self.opfbookpath:
            if id in self.modified:
                return self.build_opf()
        filepath = self.book_href_to_filepath.get(id, None)
        if filepath is None:
            raise WrapperException('Book href does not exist')
        basedir = self.ebook_root
        if id in self.added or id in self.modified:
            basedir = self.outdir
        filepath = os.path.join(basedir, filepath)
        if not unipath.exists(filepath):
            raise WrapperException('File Does Not Exist')
        basename = os.path.basename(filepath)
        ext = os.path.splitext(basename)[1]
        ext = ext.lower()
        mime = ext_mime_map.get(ext,"")
        data = b''
        with open(filepath,'rb') as fp:
            data = fp.read()
        if mime in TEXT_MIMETYPES:
            data = unicode_str(data)
        return data

    def writeotherfile(self, book_href, data):
        id = unicode_str(book_href)
        id = unquoteurl(id)
        if id is None:
            raise WrapperException('None is not a valid book href')
        if id not in self.other and id in self.id_to_href:
            raise WrapperException('Incorrect interface routine - use writefile')
        filepath = self.book_href_to_filepath.get(id, None)
        if filepath is None:
            raise WrapperException('Book href does not exist')
        if id in PROTECTED_FILES or id == self.opfbookpath:
            raise WrapperException('Attempt to modify protected file')
        filepath = os.path.join(self.outdir, filepath)
        base = os.path.dirname(filepath)
        if not unipath.exists(base):
            os.makedirs(base)
        if isinstance(data, text_type):
            data = utf8_str(data)
        with open(filepath,'wb') as fp:
            fp.write(data)
        self.modified[id] = 'file'

    def addotherfile(self, book_href, data) :
        id = unicode_str(book_href)
        id = unquoteurl(id)
        if id is None:
            raise WrapperException('None is not a valid book href')
        if id in self.other:
            raise WrapperException('Book href must be unique')
        desired_path = id.replace("/",os.sep)
        filepath = os.path.join(self.outdir,desired_path)
        if unipath.isfile(filepath):
            raise WrapperException('Desired path already exists')
        base = os.path.dirname(filepath)
        if not unipath.exists(base):
            os.makedirs(pathof(base))
        if isinstance(data, text_type):
            data = utf8_str(data)
        with open(pathof(filepath),'wb')as fp:
            fp.write(data)
        self.other.append(id)
        self.added.append(id)
        self.book_href_to_filepath[id] = desired_path

    def deleteotherfile(self, book_href):
        id = unicode_str(book_href)
        id = unquoteurl(id)
        if id is None:
            raise WrapperException('None is not a valid book hrefbook href')
        if id not in self.other and id in self.id_to_href:
            raise WrapperException('Incorrect interface routine - use deletefile')
        filepath = self.book_href_to_filepath.get(id, None)
        if filepath is None:
            raise WrapperException('Book href does not exist')
        if id in PROTECTED_FILES or id == self.opfbookpath:
            raise WrapperException('attempt to delete protected file')
        add_to_deleted = True
        # if file was added or modified delete file from outdir
        if id in self.added or id in self.modified:
            filepath = os.path.join(self.outdir,filepath)
            if unipath.exists(filepath) and unipath.isfile(filepath):
                os.remove(filepath)
            if id in self.added:
                self.added.remove(id)
                add_to_deleted = False
            if id in self.other:
                self.other.remove(id)
            if id in self.modified:
                del self.modified[id]
        if add_to_deleted:
            self.deleted.append(('other', id, book_href))
        del self.book_href_to_filepath[id]


    # utility routine to copy entire ebook to a destination directory
    # including the any prior updates and changes to the opf

    def copy_book_contents_to(self, destdir):
        destdir = unicode_str(destdir)
        if destdir is None or not unipath.isdir(destdir):
            raise WrapperException('destination directory does not exist')
        for id in self.id_to_filepath:
            rpath = self.id_to_filepath[id]
            in_manifest = id in self.id_to_mime
            data = self.readfile(id)
            filepath = os.path.join(destdir,rpath)
            base = os.path.dirname(filepath)
            if not unipath.exists(base):
                os.makedirs(base)
            if isinstance(data,text_type):
                data = utf8_str(data)
            with open(pathof(filepath),'wb') as fp:
                fp.write(data)
        for id in self.book_href_to_filepath:
            rpath = self.book_href_to_filepath[id]
            data = self.readotherfile(id)
            filepath = os.path.join(destdir,rpath)
            base = os.path.dirname(filepath)
            if not unipath.exists(base):
                os.makedirs(base)
            if isinstance(data,text_type):
                data = utf8_str(data)
            with open(pathof(filepath),'wb') as fp:
                fp.write(data)

    def get_dictionary_dirs(self):
        apaths = []
        if sys.platform.startswith('darwin'):
            apaths.append(unipath.abspath(os.path.join(self.appdir,"..","hunspell_dictionaries")))
            apaths.append(unipath.abspath(os.path.join(self.usrsupdir,"hunspell_dictionaries")))
        elif sys.platform.startswith('win'):
            apaths.append(unipath.abspath(os.path.join(self.appdir,"hunspell_dictionaries")))
            apaths.append(unipath.abspath(os.path.join(self.usrsupdir,"hunspell_dictionaries")))
        else:
            # Linux
            for path in self.linux_hunspell_dict_dirs:
                apaths.append(unipath.abspath(path.strip()))
            apaths.append(unipath.abspath(os.path.join(self.usrsupdir,"hunspell_dictionaries")))
        return apaths

    def get_gumbo_path(self):
        if sys.platform.startswith('darwin'):
            lib_dir = unipath.abspath(os.path.join(self.appdir,"..","lib"))
            lib_name = 'libsigilgumbo.dylib'
        elif sys.platform.startswith('win'):
            lib_dir = unipath.abspath(self.appdir)
            lib_name = 'sigilgumbo.dll'
        else:
            lib_dir = unipath.abspath(self.appdir)
            lib_name = 'libsigilgumbo.so'
        return os.path.join(lib_dir, lib_name)

    def get_hunspell_path(self):
        if sys.platform.startswith('darwin'):
            lib_dir = unipath.abspath(os.path.join(self.appdir,"..","lib"))
            lib_name = 'libhunspell.dylib'
        elif sys.platform.startswith('win'):
            lib_dir = unipath.abspath(self.appdir)
            lib_name = 'hunspell.dll'
        else:
            lib_dir = unipath.abspath(self.appdir)
            lib_name = 'libhunspell.so'
        return os.path.join(lib_dir, lib_name)
