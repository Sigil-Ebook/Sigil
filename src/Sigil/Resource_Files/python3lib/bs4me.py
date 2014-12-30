#!/usr/bin/env python3

import html5lib
from html5lib import treebuilders
from html5lib import treewalkers
from html5lib import serializer
from html5lib.filters import sanitizer
from html5lib.constants import cdataElements, rcdataElements
from bs4 import BeautifulSoup
import re

def remove_xml_header(data):
    return re.sub(r'<\s*\?xml\s*[^>]*\?>','',data, flags=re.I)

# borrowed from Kovid's calibre to work around 
# <title/> parsing idiocy in html5lib 
# see: http://code.google.com/p/html5lib/issues/detail?id=195
def fix_self_closing_cdata_tags(data):
    return re.sub(r'<\s*(%s)\s*[^>]*/\s*>' % ('|'.join(cdataElements|rcdataElements)), r'<\1></\1>', data, flags=re.I)

def cleanUsingBS4(data):
    data = remove_xml_header(data)
    data = fix_self_closing_cdata_tags(data)
    soup = BeautifulSoup(data, 'html5lib')
    newdata = soup.decode(False,formatter="minimal")
    newdata = newdata.replace("\n\n</body></html>","</body>\n</html>\n")
    newdata = '<?xml version="1.0" encoding="UTF-8"?>\n' + newdata
    return newdata

def main():
    junk = "<html><head><title>testing & entities</title></head><body><p>this&nbsp;is&#160;the&#xa0;copyright symbol &copy;</p></body></html>"
    print(cleanUsingBS4(junk))
    return 0

if __name__ == '__main__':
    sys.exit(main())
