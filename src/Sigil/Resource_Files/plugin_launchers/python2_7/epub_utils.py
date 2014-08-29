#!/usr/bin/env python
# vim:fileencoding=UTF-8:ts=4:sw=4:sta:et:sts=4:ai

import sys, os
import struct, array, binascii, re

from itertools import cycle

import zlib
import zipfile
from zipfile import ZipFile

import path
from path import pathof

import hashlib

def SHA1(message):
    ctx = hashlib.sha1()
    ctx.update(message)
    return ctx.digest()

epub_mimetype = 'application/epub+zip'


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
        files = path.walk(ebook_path)
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
    opf_path = utf8_str(relative_path_to_opf)
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
            encryption += '      <enc:CipherReference URI="OEBPS/Fonts/' + font + '"/>\n' % fontfile
            encryption += '    </enc:CipherData>\n'
            encryption += '  </enc:EncryptedData>\n'
        encryption += '</encryption>\n'
    return encryption



def Adobe_encryption_key(uid):
    # strip it down to simple valid hex characters
    # being careful to generate a string of 16 bytes in length
    key = utf8_str(uid)
    if key.startswith("urn:uuid:"):
        key = key[9:]
    key = key.replace('-','')
    key = re.sub(r'[^a-fA-F0-9]', '', key)
    key = binascii.unhexlify((key + key)[:32])
    return key
    


def Idpf_encryption_key(uid):
    # remove whitespace changing nothing else
    key = utf8_str(uid)
    key = key.replace(chr(x20),'')
    key = key.replace(chr(x09),'')
    key = key.replace(chr(x0d),'')
    key = key.replace(chr(x0a),'')
    key = SHA1(key)
    return key



def Adobe_mangle_fonts(encryption_key, data):
    """
    encryption_key = tuple(map(ord, encryption_key))
    encrypted_data_list = []
    for i in range(1024):
        encrypted_data_list.append(\
        chr(ord(data[i]) ^ encryption_key[i%16]))
    encrypted_data_list.append(data[1024:])
    return "".join(encrypted_data_list)
    """
    crypt = data[:1024]
    key = cycle(iter(map(ord, encryption_key)))
    encrypt = ''.join([chr(ord(x)^key.next()) for x in crypt])
    return encrypt + data[1024:]



def Idpf_mangle_fonts(encryption_key, data):
    crypt = data[:1040]
    key = cycle(iter(map(ord, encryption_key)))
    encrypt = ''.join([chr(ord(x)^key.next()) for x in crypt])
    return encrypt + data[1040:]

