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
import struct, array, binascii, re
from itertools import cycle

from compatibility_utils import text_type, binary_type, utf8_str, unicode_str, bord, bchr

import zlib
import zipfile
from zipfile import ZipFile

import unipath
from unipath import pathof

import hashlib

def SHA1(message):
    ctx = hashlib.sha1()
    ctx.update(message)
    return ctx.digest()

epub_mimetype = b'application/epub+zip'


def unzip_epub_to_dir(path_to_epub, destdir):
    f = open(pathof(path_to_epub), 'rb')
    sz = ZipFile(f)
    for name in sz.namelist():
        data = sz.read(name)
        name = name.replace("/", os.sep)
        filepath = os.path.join(destdir,name)
        basedir = os.path.dirname(filepath)
        if not os.path.isdir(basedir):
            os.makedirs(basedir)
        with open(filepath,'wb') as fp:
            fp.write(data)
    f.close()


def epub_zip_up_book_contents(ebook_path, epub_filepath):
    outzip = zipfile.ZipFile(pathof(epub_filepath), 'w')
    files = unipath.walk(ebook_path)
    if 'mimetype' in files:
        outzip.write(pathof(os.path.join(ebook_path, 'mimetype')), pathof('mimetype'), zipfile.ZIP_STORED)
    else:
        raise Exception('mimetype file is missing')
    files.remove('mimetype')
    for file in files:
        filepath = os.path.join(ebook_path, file)
        outzip.write(pathof(filepath),pathof(file),zipfile.ZIP_DEFLATED)
    outzip.close()


def build_container_xml(relative_path_to_opf):
    opf_path = unicode_str(relative_path_to_opf)
    container = '<?xml version="1.0" encoding="UTF-8"?>\n'
    container += '<container version="1.0" xmlns="urn:oasis:names:tc:opendocument:xmlns:container">\n'
    container += '    <rootfiles>\n'
    container += '<rootfile full-path="%s" media-type="application/oebps-package+xml"/>' % opf_path
    container += '    </rootfiles>\n</container>\n'
    return container


def build_adobe_encryption_xml(mangled_fonts):
    encryption = ''
    if mangled_fonts is not None and len(mangled_fonts) > 0:
        encryption = '<encryption xmlns="urn:oasis:names:tc:opendocument:xmlns:container" \
            xmlns:enc="http://www.w3.org/2001/04/xmlenc#" xmlns:deenc="http://ns.adobe.com/digitaleditions/enc">\n'
        for fontfile in mangled_fonts:
            encryption += '  <enc:EncryptedData>\n'
            encryption += '    <enc:EncryptionMethod Algorithm="http://ns.adobe.com/pdf/enc#RC"/>\n'
            encryption += '    <enc:CipherData>\n'
            encryption += '      <enc:CipherReference URI="OEBPS/Fonts/' + fontfile + '"/>\n'
            encryption += '    </enc:CipherData>\n'
            encryption += '  </enc:EncryptedData>\n'
        encryption += '</encryption>\n'
    return encryption


# mangeled_fonts is a simple list of Font file names (no href or path needed)
def build_idpf_encryption_xml(mangled_fonts):
    encryption = ''
    if mangled_fonts is not None and len(mangled_fonts) > 0:
        encryption = '<encryption xmlns="urn:oasis:names:tc:opendocument:xmlns:container" \
            xmlns:enc="http://www.w3.org/2001/04/xmlenc#">\n'
        for fontfile in mangled_fonts:
            encryption += '  <enc:EncryptedData>\n'
            encryption += '    <enc:EncryptionMethod Algorithm="http://www.idpf.org/2008/embedding"/>\n'
            encryption += '    <enc:CipherData>\n'
            encryption += '      <enc:CipherReference URI="OEBPS/Fonts/' + fontfile + '"/>\n'
            encryption += '    </enc:CipherData>\n'
            encryption += '  </enc:EncryptedData>\n'
        encryption += '</encryption>\n'
    return encryption


# mangeled_fonts is a simple list of Font file names (no href or path needed)
def Adobe_encryption_key(uid):
    # strip it down to simple valid hex characters
    # being careful to generate a string of 16 bytes in length
    key = utf8_str(uid)
    if key.startswith(b"urn:uuid:"):
        key = key[9:]
    key = key.replace(b'-',b'')
    key = re.sub(r'[^a-fA-F0-9]', b'', key)
    key = binascii.unhexlify((key + key)[:32])
    return key


def Idpf_encryption_key(uid):
    # remove whitespace changing nothing else
    key = utf8_str(uid)
    key = key.replace(bchr(0x20),b'')
    key = key.replace(bchr(0x09),b'')
    key = key.replace(bchr(0x0d),b'')
    key = key.replace(bchr(0x0a),b'')
    key = SHA1(key)
    return key


def Adobe_mangle_fonts(encryption_key, data):
    if isinstance(data, text_type):
        print('Error: font data must be a byte string')
    crypt = data[:1024]
    key = cycle(iter(map(bord, encryption_key)))
    encrypt = b''.join([bchr(bord(x)^next(key)) for x in crypt])
    return encrypt + data[1024:]


def Idpf_mangle_fonts(encryption_key, data):
    if isinstance(data, text_type):
        print('Error: font data must be a byte string')
    crypt = data[:1040]
    key = cycle(iter(map(bord, encryption_key)))
    encrypt = b''.join([bchr(bord(x)^next(key)) for x in crypt])
    return encrypt + data[1040:]
