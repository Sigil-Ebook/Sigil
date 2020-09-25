#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# vim:ts=4:sw=4:softtabstop=4:smarttab:expandtab

# Copyright (c) 2014-2020 Kevin B. Hendricks, and Doug Massay
# Copyright (c) 2014      Kevin B. Hendricks, John Schember, and Doug Massay
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

import os
import binascii
import re
from itertools import cycle

import zipfile
from zipfile import ZipFile

import hashlib

def SHA1(message):
    ctx = hashlib.sha1()
    ctx.update(message)
    return ctx.digest()

epub_mimetype = b'application/epub+zip'


def utf8str(s):
    if s is None:
        return None
    if isinstance(s, bytes):
        return s
    return s.encode('utf-8', errors='replace')


def epub_file_walk(top):
    top = os.fsdecode(top)
    rv = []
    for base, dnames, names in os.walk(top):
        for name in names:
            rv.append(os.path.relpath(os.path.join(base, name), top))
    return rv


def unzip_epub_to_dir(path_to_epub, destdir):
    f = open(os.fsdecode(path_to_epub), 'rb')
    sz = ZipFile(f)
    for name in sz.namelist():
        data = sz.read(name)
        name = name.replace("/", os.sep)
        filepath = os.path.join(destdir, name)
        basedir = os.path.dirname(filepath)
        if not os.path.isdir(basedir):
            os.makedirs(basedir)
        with open(filepath, 'wb') as fp:
            fp.write(data)
    f.close()


def epub_zip_up_book_contents(ebook_path, epub_filepath):
    outzip = zipfile.ZipFile(os.fsdecode(epub_filepath), 'w')
    book_path = os.fsdecode(ebook_path)
    files = epub_file_walk(book_path)
    if 'mimetype' in files:
        outzip.write(os.path.join(book_path, 'mimetype'), 'mimetype', zipfile.ZIP_STORED)
    else:
        raise Exception('mimetype file is missing')
    files.remove('mimetype')
    for file in files:
        filepath = os.path.join(book_path, file)
        outzip.write(filepath, file, zipfile.ZIP_DEFLATED)
    outzip.close()


def build_container_xml(bookpath_to_opf):
    opf_path = os.fsdecode(bookpath_to_opf)
    container = '<?xml version="1.0" encoding="UTF-8"?>\n'
    container += '<container version="1.0" xmlns="urn:oasis:names:tc:opendocument:xmlns:container">\n'
    container += '    <rootfiles>\n'
    container += '<rootfile full-path="%s" media-type="application/oebps-package+xml"/>' % opf_path
    container += '    </rootfiles>\n</container>\n'
    return container


# mangled_fonts is a list of the book paths to the font files to be (de)mangled
def build_adobe_encryption_xml(mangled_fonts):
    encryption = ''
    if mangled_fonts is not None and len(mangled_fonts) > 0:
        encryption = '<encryption xmlns="urn:oasis:names:tc:opendocument:xmlns:container" \
            xmlns:enc="http://www.w3.org/2001/04/xmlenc#" xmlns:deenc="http://ns.adobe.com/digitaleditions/enc">\n'
        for fontbkpath in mangled_fonts:
            encryption += '  <enc:EncryptedData>\n'
            encryption += '    <enc:EncryptionMethod Algorithm="http://ns.adobe.com/pdf/enc#RC"/>\n'
            encryption += '    <enc:CipherData>\n'
            encryption += '      <enc:CipherReference URI="' + fontbkpath + '"/>\n'
            encryption += '    </enc:CipherData>\n'
            encryption += '  </enc:EncryptedData>\n'
        encryption += '</encryption>\n'
    return encryption


# mangled_fonts is a list of book paths to the font files to be (de)mangled
def build_idpf_encryption_xml(mangled_fonts):
    encryption = ''
    if mangled_fonts is not None and len(mangled_fonts) > 0:
        encryption = '<encryption xmlns="urn:oasis:names:tc:opendocument:xmlns:container" \
            xmlns:enc="http://www.w3.org/2001/04/xmlenc#">\n'
        for fontbkpath in mangled_fonts:
            encryption += '  <enc:EncryptedData>\n'
            encryption += '    <enc:EncryptionMethod Algorithm="http://www.idpf.org/2008/embedding"/>\n'
            encryption += '    <enc:CipherData>\n'
            encryption += '      <enc:CipherReference URI="' + fontbkpath + '"/>\n'
            encryption += '    </enc:CipherData>\n'
            encryption += '  </enc:EncryptedData>\n'
        encryption += '</encryption>\n'
    return encryption


def Adobe_encryption_key(uid):
    # strip it down to simple valid hex characters
    # being careful to generate a string of 16 bytes in length
    key = utf8str(uid)
    if key.startswith(b"urn:uuid:"):
        key = key[9:]
    key = key.replace(b'-', b'')
    key = re.sub(r'[^a-fA-F0-9]', b'', key)
    key = binascii.unhexlify((key + key)[:32])
    return key


def Idpf_encryption_key(uid):
    # remove whitespace changing nothing else
    key = utf8str(uid)
    key = key.replace(bytes([0x20]), b'')
    key = key.replace(bytes([0x09]), b'')
    key = key.replace(bytes([0x0d]), b'')
    key = key.replace(bytes([0x0a]), b'')
    key = SHA1(key)
    return key


def Adobe_mangle_fonts(encryption_key, data):
    def bord(s):
        return s
    if not isinstance(data, bytes):
        print('Error: font data must be a byte string')
    crypt = data[:1024]
    key = cycle(iter(map(bord, encryption_key)))
    encrypt = b''.join([bytes([x ^ next(key)]) for x in crypt])
    return encrypt + data[1024:]


def Idpf_mangle_fonts(encryption_key, data):
    def bord(s):
        return s
    if not isinstance(data, bytes):
        print('Error: font data must be a byte string')
    crypt = data[:1040]
    key = cycle(iter(map(bord, encryption_key)))
    encrypt = b''.join([bytes([x ^ next(key)]) for x in crypt])
    return encrypt + data[1040:]
