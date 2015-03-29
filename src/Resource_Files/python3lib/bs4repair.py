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

ebook_xml_empty_tags = ["meta", "item", "itemref", "reference", "content"]

def remove_xml_header(data):
    return re.sub(r'<\s*\?xml\s*[^>]*\?>\s*','',data, flags=re.I)

# borrowed from Kovid's calibre to work around 
# <title/> parsing idiocy in html5lib 
# see: http://code.google.com/p/html5lib/issues/detail?id=195
def fix_self_closing_cdata_tags(data):
    return re.sub(r'<\s*(%s)\s*[^>]*/\s*>' % ('|'.join(cdataElements|rcdataElements)), r'<\1></\1>', data, flags=re.I)

# BS4 with lxml for xml strips whitespace so always will want to prettyprint xml
# def repairXML(data, self_closing_tags=ebook_xml_empty_tags):
#     xmlbuilder = LXMLTreeBuilderForXML(parser=None, empty_element_tags=self_closing_tags)
#     soup = BeautifulSoup(data, features=None, builder=xmlbuilder)
#     newdata = soup.serialize()
#    return newdata

def repairPrettyPrintXML(data, self_closing_tags=ebook_xml_empty_tags, indent_chars="  "):
    xmlbuilder = LXMLTreeBuilderForXML(parser=None, empty_element_tags=self_closing_tags)
    soup = BeautifulSoup(data, features=None, builder=xmlbuilder)
    newdata = soup.decodexml(indent_level=0, formatter='minimal', indent_chars=indent_chars)
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

    print(repairPrettyPrintXML(opfxml, indent_chars="  "))
    return 0

if __name__ == '__main__':
    sys.exit(main())
