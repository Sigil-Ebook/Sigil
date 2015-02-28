#!/usr/bin/env python3

import sys

# until we get a properly embedded python 3 with its own site-packages
# force our current module path to come before site_packages
# to prevent name collisions with our versions and any site-packages
def insert_into_syspath():
    n = 0
    sp = None
    ourhome = sys.path[-1]
    for apath in sys.path:
        if apath.endswith("site-packages"):
            sp = n
            break
        n += 1
    if sp is not None:
        sys.path.insert(sp,ourhome)


insert_into_syspath()

from bs4 import BeautifulSoup
from bs4.builder._lxml import LXMLTreeBuilderForXML
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


# BS4 with lxml for xml strips whitespace so always will want to prettyprint xml
# def repairXML(data, self_closing_tags=ebook_xml_empty_tags):
#     xmlbuilder = LXMLTreeBuilderForXML(parser=None, empty_element_tags=self_closing_tags)
#     soup = BeautifulSoup(data, features=None, builder=xmlbuilder)
#     newdata = soup.serialize()
#    return newdata

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
                        tag[src] = attribute_value
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
  <guide />
</package>
'''
    print(repairXML(opfxml, indent_chars=" "))
    return 0

if __name__ == '__main__':
    sys.exit(main())
