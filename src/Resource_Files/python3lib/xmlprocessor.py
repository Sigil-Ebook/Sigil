#!/usr/bin/env python3

import sys
import os
from sigil_bs4 import BeautifulSoup
from sigil_bs4.builder._lxml import LXMLTreeBuilderForXML
import re
from urllib.parse import unquote

ASCII_CHARS   = set(chr(x) for x in range(128))
URL_SAFE      = set('ABCDEFGHIJKLMNOPQRSTUVWXYZ'
                    'abcdefghijklmnopqrstuvwxyz'
                    '0123456789' '#' '_.-/~')
IRI_UNSAFE = ASCII_CHARS - URL_SAFE

# returns a quoted IRI (not a URI)
def quoteurl(href):
    if isinstance(href,bytes):
        href = href.decode('utf-8')
    result = []
    for char in href:
        if char in IRI_UNSAFE:
            char = "%%%02x" % ord(char)
        result.append(char)
    return ''.join(result)

# unquotes url/iri
def unquoteurl(href):
    if isinstance(href,bytes):
        href = href.decode('utf-8')
    href = unquote(href)
    return href


TEXT_FOLDER_NAME = "Text"
ebook_xml_empty_tags = ["meta", "item", "itemref", "reference", "content"]


def remove_xml_header(data):
    return re.sub(r'<\s*\?xml\s*[^>]*\?>\s*','',data, flags=re.I)

# ncx_text_pattern = re.compile(r'''(<text>)\s*(\S[^<]*\S)\s*(</text>)''',re.IGNORECASE)
# re.sub(ncx_text_pattern,r'\1\2\3',newdata)

# BS4 with lxml for xml strips whitespace so always will want to prettyprint xml
def repairXML(data, self_closing_tags=ebook_xml_empty_tags, indent_chars="  "):
    xmlbuilder = LXMLTreeBuilderForXML(parser=None, empty_element_tags=self_closing_tags)
    soup = BeautifulSoup(data, features=None, builder=xmlbuilder)
    newdata = soup.decodexml(indent_level=0, formatter='minimal', indent_chars=indent_chars)
    return newdata

def anchorNCXUpdates(data, originating_filename, keylist, valuelist):
    # rebuild serialized lookup dictionary
    id_dict = {}
    for i in range(0, len(keylist)):
        id_dict[ keylist[i] ] = valuelist[i]
    xmlbuilder = LXMLTreeBuilderForXML(parser=None, empty_element_tags=ebook_xml_empty_tags)
    soup = BeautifulSoup(data, features=None, builder=xmlbuilder)
    original_filename_with_relative_path = TEXT_FOLDER_NAME  + "/" + originating_filename
    for tag in soup.find_all("content"):
        if "src" in tag.attrs:
            src = tag["src"]
            if src.find(":") == -1:
                parts = src.split('#')
                if (parts is not None) and (len(parts) > 1) and (parts[0] == original_filename_with_relative_path) and (parts[1] != ""):
                    fragment_id = parts[1]
                    if fragment_id in id_dict:
                        attribute_value = TEXT_FOLDER_NAME + "/" + quoteurl(id_dict[fragment_id]) + "#" + fragment_id
                        tag["src"] = attribute_value
    newdata = soup.decodexml(indent_level=0, formatter='minimal', indent_chars="  ")
    return newdata


def performNCXSourceUpdates(data, currentdir, keylist, valuelist):
    # rebuild serialized lookup dictionary
    updates = {}
    for i in range(0, len(keylist)):
        updates[ keylist[i] ] = valuelist[i]
    xmlbuilder = LXMLTreeBuilderForXML(parser=None, empty_element_tags=ebook_xml_empty_tags)
    soup = BeautifulSoup(data, features=None, builder=xmlbuilder)
    for tag in soup.find_all("content"):
        if "src" in tag.attrs:
            src = tag["src"]
            if src.find(":") == -1:
                parts = src.split('#')
                url = parts[0]
                fragment = ""
                if len(parts) > 1:
                    fragment = parts[1]
                bookrelpath = os.path.join(currentdir, unquoteurl(url))
                bookrelpath = os.path.normpath(bookrelpath)
                bookrelpath = bookrelpath.replace(os.sep, "/")
                if bookrelpath in updates:
                    attribute_value = updates[bookrelpath]
                    if fragment != "":
                        attribute_value = attribute_value + "#" + fragment
                    attribute_value = quoteurl(attribute_value)
                    tag["src"] = attribute_value
    newdata = soup.decodexml(indent_level=0, formatter='minimal', indent_chars="  ")
    return newdata


def performOPFSourceUpdates(data, currentdir, keylist, valuelist):
    # rebuild serialized lookup dictionary
    updates = {}
    for i in range(0, len(keylist)):
        updates[ keylist[i] ] = valuelist[i]
    xmlbuilder = LXMLTreeBuilderForXML(parser=None, empty_element_tags=ebook_xml_empty_tags)
    soup = BeautifulSoup(data, features=None, builder=xmlbuilder)
    for tag in soup.find_all(["item","reference","site"]):
        if "href" in tag.attrs :
            href = tag["href"]
            if href.find(":") == -1 :
                parts = href.split('#')
                url = parts[0]
                fragment = ""
                if len(parts) > 1:
                    fragment = parts[1]
                bookrelpath = os.path.join(currentdir, unquoteurl(url))
                bookrelpath = os.path.normpath(bookrelpath)
                bookrelpath = bookrelpath.replace(os.sep, "/")
                if bookrelpath in updates:
                    attribute_value = updates[bookrelpath]
                    if fragment != "":
                        attribute_value = attribute_value + "#" + fragment
                    attribute_value = quoteurl(attribute_value)
                    tag["href"] = attribute_value
    newdata = soup.decodexml(indent_level=0, formatter='minimal', indent_chars="  ")
    return newdata


def main():
    opfxml = '''
<?xml version="1.0" encoding="utf-8" standalone="yes"?>
<package xmlns="http://www.idpf.org/2007/opf" unique-identifier="BookId" version="2.0">
  <metadata xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:opf="http://www.idpf.org/2007/opf">
    <dc:identifier id="BookId" opf:scheme="UUID">urn:uuid:a418a8f1-dcbc-4c5d-a18f-533765e34ee8</dc:identifier>
  </metadata>
  <manifest>
    <item href="toc.ncx" id="ncx" media-type="application/x-dtbncx+xml" />
    <item href="Text/Section0001.xhtml" id="Section0001.xhtml" media-type="application/xhtml+xml" />
  </manifest>
  <spine toc="ncx">
    <itemref idref="Section0001.xhtml" >
  </spine>
  <text>
    this is a bunch of nonsense
  </text>
  <text>
    this is a bunch of nonsense 1
  </text>
  <text>
    this is a bunch of nonsense 2
  </text>
  <guide />
</package>
'''
    print(repairXML(opfxml))
    return 0

if __name__ == '__main__':
    sys.exit(main())
