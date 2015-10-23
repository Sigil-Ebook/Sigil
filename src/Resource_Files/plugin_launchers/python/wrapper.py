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
from compatibility_utils import PY3, PY2, text_type, binary_type, utf8_str, unicode_str, iswindows, quoteurl, unquoteurl

import sys
import os
import re
import unipath
from unipath import pathof
import unicodedata

_launcher_version=20151014

# Wrapper Class is used to peform record keeping for Sigil.  It keeps track of modified,
# added, and deleted files while providing some degree of protection against files under
# Sigil's control from being directly manipulated.
# Uses "write-on-modify" and so removes the need for wholesale copying of files

_guide_types = ['cover','title-page','toc','index','glossary','acknowledgements',
                'bibliography','colophon','copyright-page','dedication',
                'epigraph','foreward','loi','lot','notes','preface','text']

ext_mime_map = {
                '.jpg'  : 'image/jpeg',
                '.jpeg' : 'image/jpeg',
                '.png'  : 'image/png',
                '.gif'  : 'image/gif',
                '.svg'  : 'image/svg+xml',
                '.xhtml': 'application/xhtml+xml',
                '.html' : 'application/xhtml+xml',
                '.ttf'  : 'application/x-font-ttf',
                '.otf'  : 'application/x-font-opentype',
                '.woff' : 'application/font-woff',
                '.mp3'  : 'audio/mpeg',
                '.mp4'  : 'video/mp4',
                '.css'  : 'text/css',
                '.ncx'  : 'application/x-dtbncx+xml',
                '.xml'  : 'application/oebs-page-map+xml',
                '.opf'  : 'application/oebps-package+xml',
                '.smil' : 'application/smil+xml',
                '.pls'  : 'application/pls-xml',
                '.js'   : 'text/javascript',
                '.epub' : 'application/epub+zip',
                #'.js'   : 'application/javascript',
                #'.otf'  : 'application/vnd.ms-opentype',
                }

mime_base_map = {
                'image/jpeg'                    : 'Images',
                'image/png'                     : 'Images',
                'image/gif'                     : 'Images',
                'image/svg+xml'                 : 'Images',
                'application/xhtml+xml'         : 'Text',
                'application/x-font-ttf'        : 'Fonts',
                'application/x-font-opentype'   : 'Fonts',
                'application/vnd.ms-opentype'   : 'Fonts',
                'application/font-woff'         : 'Fonts',
                'audio/mpeg'                    : 'Audio',
                'audio/mp3'                     : 'Audio',
                'audio/mp4'                     : 'Audio',
                'video/mp4'                     : 'Video',
                'text/css'                      : 'Styles',
                'application/x-dtbncx+xml'      : '',
                'application/oebps-package+xml' : '',
                'application/oebs-page-map+xml' : 'Misc',
                'application/smil+xml'          : 'Misc',
                'application/pls-xml'           : 'Misc',
                }


PROTECTED_FILES = [
    'mimetype',
    'META-INF/container.xml',
    'OEBPS/content.opf'
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
        self.appdir = None
        self.usrsupdir = None
        self.selected = []
        cfg = ''
        with open(os.path.join(self.outdir, 'sigil.cfg'), 'rb') as f:
            cfg = f.read().decode('utf-8')
        cfg = cfg.replace("\r", "")
        cfg_lst = cfg.split("\n")
        if len(cfg_lst) >= 2:
            self.appdir = cfg_lst.pop(0)
            self.usrsupdir = cfg_lst.pop(0)
            self.selected = cfg_lst
        os.environ['SigilGumboLibPath'] = self.get_gumbo_path()

        # dictionaries used to map opf manifest information
        self.id_to_href = {}
        self.id_to_mime = {}
        self.href_to_id = {}
        self.id_to_props = {}
        self.spine_ppd = None
        self.spine = []
        self.guide = []
        self.package_tag = None
        # self.metadata_attr = None
        # self.metadata = []
        self.metadataxml = ''
        self.op = op
        if self.op is not None:
            # copy in data from parsing of initial opf
            self.opfname = op.opfname
            self.id_to_href = op.get_manifest_id_to_href_dict().copy()
            self.id_to_mime = op.get_manifest_id_to_mime_dict().copy()
            self.href_to_id = op.get_href_to_manifest_id_dict().copy()
            self.id_to_props = op.get_manifest_id_to_properties_dict().copy()
            self.spine_ppd = op.get_spine_ppd()
            self.spine = op.get_spine()
            self.guide = op.get_guide()
            self.package_tag = op.get_package_tag()
            # self.metadata = op.get_metadata()
            # self.metadata_attr = op.get_metadata_attr()
            self.metadataxml = op.get_metadataxml()
        self.other = []  # non-manifest file information
        self.id_to_filepath = {}
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
            id = None
            if book_href.startswith('OEBPS/'):
                href = book_href[6:]
                id = self.href_to_id.get(href,None)
            if id is None:
                self.other.append(book_href)
                self.id_to_filepath[book_href] = filepath
            else:
                self.id_to_filepath[id] = filepath

    def getversion(self):
        global _launcher_version
        return _launcher_version


    # utility routine to get mime from href
    def getmime(self,  href):
        href = unicode_str(href)
        filename = os.path.basename(href)
        ext = os.path.splitext(filename)[1]
        ext = ext.lower()
        return ext_mime_map.get(ext, "")


    # routines to rebuild the opf on the fly from current information

    def build_package_starttag(self):
        return self.package_tag

    def build_manifest_xml(self):
        manout = []
        manout.append('<manifest>\n')
        for id in sorted(self.id_to_mime):
            href = quoteurl(self.id_to_href[id])
            mime = self.id_to_mime[id]
            props = ''
            properties = self.id_to_props[id]
            if properties is not None:
                props = ' properties="%s"' % properties
            manout.append('<item id="%s" href="%s" media-type="%s"%s />\n' % (id, href, mime, props))
        manout.append('</manifest>\n')
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
        spineout.append('<spine%s%s%s>\n' %(ppd, ncx, map))
        for (id, linear, properties) in self.spine:
            lin = ''
            if linear is not None:
                lin = ' linear="%s"' % linear
            props = ''
            if properties is not None:
                props = ' properties="%s"' % properties
            spineout.append('<itemref idref="%s"%s%s/>\n' % (id, lin, props))
        spineout.append('</spine>\n')
        return "".join(spineout)

    def build_guide_xml(self):
        guideout = []
        guideout.append('<guide>\n')
        for (type, title, href) in self.guide:
            href = quoteurl(href)
            guideout.append('<reference type="%s" href="%s" title="%s"/>\n' % (type, href, title))
        guideout.append('</guide>\n')
        return "".join(guideout)

    def build_opf(self):
        data = []
        data.append('<?xml version="1.0" encoding="utf-8" standalone="yes"?>\n')
        data.append(self.build_package_starttag())
        data.append(self.metadataxml)
        data.append(self.build_manifest_xml())
        data.append(self.build_spine_xml())
        data.append(self.build_guide_xml())
        data.append('</package>\n')
        return "".join(data)

    def write_opf(self):
        if self.op is not None:
            filepath = pathof(os.path.join(self.outdir, 'OEBPS', self.opfname))
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
        self.modified['OEBPS/content.opf'] = 'file'

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
        self.modified['OEBPS/content.opf'] = 'file'

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
            self.spine = self.spine.append((sid, linear, properties))
        else:
            self.spine = self.spine[0:pos] + [(sid, linear, properties)] + self.spine[pos:]
        self.modified['OEBPS/content.opf'] = 'file'

    def getspine_ppd(self):
        return self.spine_ppd

    def setspine_ppd(self, ppd):
        ppd = unicode_str(ppd)
        if ppd not in ['rtl', 'ltr', None]:
            raise WrapperException('incorrect page-progression direction')
        self.spine_ppd = ppd
        self.modified['OEBPS/content.opf'] = 'file'

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
        self.modified['OEBPS/content.opf'] = 'file'


    # routines to get and set the guide

    def getguide(self):
        return self.guide

    def setguide(self, new_guide):
        guide = []
        for (type, title, href) in new_guide:
            type = unicode_str(type)
            title = unicode_str(title)
            href = unicode_str(href)
            if type not in _guide_types:
                type = "other." + type
            if title is None:
                title = 'title missing'
            thref = href.split('#')[0]
            if thref not in self.href_to_id:
                raise WrapperException('guide href not in manifest')
            guide.append((type, title, href))
        self.guide = guide
        self.modified['OEBPS/content.opf'] = 'file'


    # routines to get and set metadata xml fragment

    def getmetadataxml(self):
        return self.metadataxml

    def setmetadataxml(self, new_metadata):
        self.metadataxml = unicode_str(new_metadata)
        self.modified['OEBPS/content.opf'] = 'file'


    # routines to get and set the package tag
    def getpackagetag(self):
        return self.package_tag

    def setpackagetag(self, new_packagetag):
        self.package_tag = unicode_str(new_packagetag)
        self.modified['OEBPS/content.opf'] = 'file'


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
        if mime.endswith('+xml'):
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
        if mime.endswith('+xml') or isinstance(data, text_type):
            data = utf8_str(data)
        with open(filepath,'wb') as fp:
            fp.write(data)
        self.modified[id] = 'file'

    def addfile(self, uniqueid, basename, data, mime=None, properties=None):
        uniqueid = unicode_str(uniqueid)
        basename = unicode_str(basename)
        mime = unicode_str(mime)
        if mime is None:
            ext = os.path.splitext(basename)[1]
            ext = ext.lower()
            mime = ext_mime_map.get(ext, None)
        if mime is None:
            raise WrapperException("Mime Type Missing")
        if mime.startswith("audio"):
            base = 'Audio'
        elif mime.startswith("video"):
            base = "Video"
        else:
            base = mime_base_map.get(mime,'Misc')
        href = base + "/" + basename
        if uniqueid in self.id_to_href:
            raise WrapperException('Manifest Id is not unique')
        if href in self.href_to_id:
            raise WrapperException('Basename is not unique')
        # now actually write out the new file
        filepath = href.replace("/",os.sep)
        filepath = os.path.join('OEBPS', filepath)
        self.id_to_filepath[uniqueid] = filepath
        filepath = os.path.join(self.outdir,filepath)
        base = os.path.dirname(filepath)
        if not unipath.exists(base):
            os.makedirs(base)
        if mime.endswith('+xml') or isinstance(data, text_type):
            data = utf8_str(data)
        with open(filepath,'wb') as fp:
            fp.write(data)
        self.id_to_href[uniqueid] = href
        self.id_to_mime[uniqueid] = mime
        self.id_to_props[uniqueid] = properties
        self.href_to_id[href] = uniqueid
        self.added.append(uniqueid)
        self.modified['OEBPS/content.opf'] = 'file'
        return uniqueid

    def deletefile(self, id):
        id = unicode_str(id)
        if id not in self.id_to_href:
            raise WrapperException('Id does not exist in manifest')
        filepath = self.id_to_filepath.get(id, None)
        if id is None:
            raise WrapperException('Id does not exist in manifest')
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
        del self.id_to_href[id]
        del self.id_to_mime[id]
        del self.id_to_props[id]
        del self.href_to_id[href]
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
            self.modified['OEBPS/content.opf'] = 'file'
        del self.id_to_filepath[id]

    def set_manifest_epub3_properties(self, id, properties):
        id = unicode_str(id)
        properties = unicode_str(properties)
        if properties is not None and properties == "":
            properties = None
        if id not in self.id_to_props:
            raise WrapperException('Id does not exist in manifest')
        del self.id_to_props[id]
        self.id_to_props[id] = properties
        self.modified['OEBPS/content.opf'] = 'file'


    # helpful mapping routines for file info from the opf manifest

    def map_href_to_id(self, href, ow):
        href = unicode_str(href)
        return self.href_to_id.get(href,ow)

    def map_basename_to_id(self, basename, ow):
        basename = unicode_str(basename)
        ext = os.path.splitext(basename)[1]
        ext = ext.lower()
        mime = ext_mime_map.get(ext,None)
        if mime.startswith("audio"):
            base = 'Audio'
        elif mime.startswith("video"):
            base = "Video"
        else:
            base = mime_base_map.get(mime,'Misc')
        href = base + "/" + basename
        return self.href_to_id.get(href,ow)

    def map_id_to_href(self, id, ow):
        id = unicode_str(id)
        return self.id_to_href.get(id, ow)

    def map_id_to_mime(self, id, ow):
        id = unicode_str(id)
        return self.id_to_mime.get(id, ow)

    def map_id_to_properties(self, id, ow):
        id = unicode_str(id)
        return self.id_to_props.get(id, ow)


    # routines to work on ebook files that are not part of an opf manifest

    def readotherfile(self, book_href):
        id = unicode_str(book_href)
        if id in self.id_to_href:
            raise WrapperException('Incorrect interface routine - use readfile')
        # handle special case of trying to read the opf
        if id is not None and id == "OEBPS/content.opf":
            return self.build_opf()
        filepath = self.id_to_filepath.get(id, None)
        if filepath is None:
            raise WrapperException('book href does not exist')
        basedir = self.ebook_root
        if id in self.added or id in self.modified:
            basedir = self.outdir
        filepath = os.path.join(basedir, filepath)
        if not unipath.exists(filepath):
            raise WrapperException('File Does Not Exist')
        basename = os.path.basename(filepath)
        ext = os.path.splitext(basename)[1]
        ext = ext.lower()
        mime = ext_mime_map.get(ext,'')
        data = b''
        with open(filepath,'rb') as fp:
            data = fp.read()
        if mime.endswith('+xml'):
            data = unicode_str(data)
        return data

    def writeotherfile(self, book_href, data):
        id = unicode_str(book_href)
        if id in self.id_to_href:
            raise WrapperException('Incorrect interface routine - use writefile')
        filepath = self.id_to_filepath.get(id, None)
        if filepath is None:
            raise WrapperException('book href does not exist')
        if id in PROTECTED_FILES:
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
        if id in self.other:
            raise WrapperException('book href must be unquie')
        desired_path = id.replace("/",os.sep)
        filepath = os.path.join(self.outdir,desired_path)
        if unipath.isfile(filepath):
            raise WrapperException('desired path already exists')
        base = os.path.dirname(filepath)
        if not unipath.exists(base):
            os.makedirs(pathof(base))
        if isinstance(data, text_type):
            data = utf8_str(data)
        with open(pathof(filepath),'wb')as fp:
            fp.write(data)
        self.other.append(id)
        self.added.append(id)
        self.id_to_filepath[id] = desired_path

    def deleteotherfile(self, book_href):
        id = unicode_str(book_href)
        if id in self.id_to_href:
            raise WrapperException('Incorrect interface routine - use deletefile')
        filepath = self.id_to_filepath.get(id, None)
        if id is None:
            raise WrapperException('book href does not exist')
        if id in PROTECTED_FILES:
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
        del self.id_to_filepath[id]


    # utility routine to copy entire ebook to a destination directory
    # including the any prior updates and changes to the opf

    def copy_book_contents_to(self, destdir):
        destdir = unicode_str(destdir)
        if destdir is None or not unipath.isdir(destdir):
            raise WrapperException('destination directory does not exist')
        for id in self.id_to_filepath:
            rpath = self.id_to_filepath[id]
            in_manifest = id in self.id_to_mime
            if in_manifest:
                data = self.readfile(id)
            else:
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
            system_hunspell_dicts = ''
            share_prefix = ''
            share_override = ''
            sigil_share_dir_name = 'sigil'

            # get the env var SIGIL_{HUNSPELL,HYPHEN}_DICTIONARIES set at launch time.
            if 'SIGIL_HUNSPELL_DICTIONARIES' in os.environ.keys():
                system_hunspell_dicts = os.environ['SIGIL_HUNSPELL_DICTIONARIES']
            if 'SIGIL_HYPHEN_DICTIONARIES' in os.environ.keys():
                system_hyphen_dicts = os.environ['SIGIL_HYPHEN_DICTIONARIES']

            # Runtime env var override of 'share/sigil' directory.
            if 'SIGIL_EXTRA_ROOT' in os.environ.keys():
                share_override = os.environ['SIGIL_EXTRA_ROOT']

            # The sigil launch script in <install_prefix>/bin knows where Sigil's build time
            # share prefix is and sets the env var SIGIL_SHARE_PREFIX to its value.
            if 'SIGIL_SHARE_PREFIX' in os.environ.keys():
                share_prefix = os.environ['SIGIL_SHARE_PREFIX']

            # If someone didn't launch Sigil with its launch script, this may save the
            # day (as long as the user didn't override SHARE_INSTALL_PREFIX at compile time).
            if not len(share_prefix) and not len(share_override):
                share_prefix = unipath.abspath(os.path.join(self.appdir,"..",".."))

            # If the 'share/sigil' location has indeed been overridden at runtime, get its prefix and use that.
            if len(share_override):
                sigil_share_dir_name = os.path.basename(share_override)
                share_prefix = unipath.abspath(os.path.join(share_override,"..",".."))

            # If the SIGIL_{HUNSPELL,HYPHEN}_DICTIONARIES env var has content, use it for the dictionary location.
            if len(system_hunspell_dicts) or len(system_hypen_dicts):
                apaths.append(unipath.abspath(system_hunspell_dicts))
                apaths.append(unipath.abspath(system_hyphen_dicts))
            else:
                # Otherwise, use Sigil's bundled hunspell dictionary location.
                apaths.append(unipath.abspath(os.path.join(share_prefix, "share", sigil_share_dir_name, "hunspell_dictionaries")))
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
