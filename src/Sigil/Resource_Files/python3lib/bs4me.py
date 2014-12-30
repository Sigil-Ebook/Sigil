#!/usr/bin/env python3

import html5lib
from html5lib import treebuilders
from html5lib import treewalkers
from html5lib import serializer
from html5lib.filters import sanitizer
from bs4 import BeautifulSoup

def cleanUsingBS4(data):
    soup = BeautifulSoup(data, 'html5lib')
    return soup.decode(False,formatter="minimal")

def main():
    junk = "<html><head><title>testing & entities</title></head><body><p>this&nbsp;is&#160;the&#xa0;copyright symbol &copy;</p></body></html>"
    print(cleanUsingBS4(junk))
    return 0

if __name__ == '__main__':
    sys.exit(main())
