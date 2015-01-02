#!/usr/bin/env python3

import sys

# until we get a properly embedded python 3 with its own site-packages
# force our current module path to come before site_packages
# to prevent name collisions with our versions and site-packages
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

import html5lib
from html5lib import treebuilders
from html5lib import treewalkers
from html5lib import serializer
from html5lib.filters import sanitizer
from html5lib.constants import cdataElements, rcdataElements
from bs4 import BeautifulSoup
import re

def remove_xml_header(data):
    return re.sub(r'<\s*\?xml\s*[^>]*\?>\s*','',data, flags=re.I)

# borrowed from Kovid's calibre to work around 
# <title/> parsing idiocy in html5lib 
# see: http://code.google.com/p/html5lib/issues/detail?id=195
def fix_self_closing_cdata_tags(data):
    return re.sub(r'<\s*(%s)\s*[^>]*/\s*>' % ('|'.join(cdataElements|rcdataElements)), r'<\1></\1>', data, flags=re.I)

def cleanUsingBS4(data):
    data = remove_xml_header(data)
    data = fix_self_closing_cdata_tags(data)
    soup = BeautifulSoup(data, 'html5lib')
    newdata = soup.serialize()
    return newdata

def prettyPrintUsingBS4(data, indent_chars="  "):
    res = []
    data = remove_xml_header(data)
    data = fix_self_closing_cdata_tags(data)
    soup = BeautifulSoup(data, 'html5lib')
    res.append('<?xml version="1.0"?>\n')
    res.append(soup.decode(pretty_print=True,formatter='minimal',indent_chars=indent_chars))
    return ''.join(res)
    

def main():
    junk = '<html><head><title>testing & entities</title></head>'
    junk += '<body><p>this&nbsp;is&#160;the&#xa0;<b><i>copyright'
    junk += '</i></b> symbol "&copy;"</p></body></html>'
    print(cleanUsingBS4(junk))
    print(prettyPrintUsingBS4(junk))

    junk = '''
<html>
<head>
<title>testing & entities</title>
</head>
<body>
<p>this&nbsp;is&#160;the&#xa0;<b><i>copyright</i></b> symbol "&copy;"</p>
</body>
</html>
'''
    print(cleanUsingBS4(junk))
    print(prettyPrintUsingBS4(junk))

    return 0

if __name__ == '__main__':
    sys.exit(main())
