#!/usr/bin/env python3

import sys
import os
from sigil_bs4 import BeautifulSoup
from sigil_bs4.builder._lxml import LXMLTreeBuilderForXML
import re
from urllib.parse import unquote
from urllib.parse import urlsplit
from lxml import etree
from io import BytesIO
from opf_newparser import Opf_Parser
from hrefutils import startingDir, buildBookPath, buildRelativePath


ASCII_CHARS   = set(chr(x) for x in range(128))
URL_SAFE      = set('ABCDEFGHIJKLMNOPQRSTUVWXYZ'
                    'abcdefghijklmnopqrstuvwxyz'
                    '0123456789' '#' '_.-/~')
IRI_UNSAFE = ASCII_CHARS - URL_SAFE

TEXT_FOLDER_NAME = "Text"
ebook_xml_empty_tags = ["meta", "item", "itemref", "reference", "content"]

def get_void_tags(mtype):
    voidtags = []
    if mtype == "application/oebps-package+xml":
        voidtags = ["item", "itemref", "mediatype", "mediaType", "reference"]
    elif mtype == "application/x-dtbncx+xml":
        voidtags = ["meta", "reference", "content"]
    elif mtype == "application/smil+xml":
        voidtags = ["text", "audio"]
    elif mtype == "application/oebps-page-map+xml":
        voidtags = ["page"]
    else:
        voidtags = ebook_xml_empty_tags
    return voidtags


# returns a quoted IRI (not a URI)
def quoteurl(href):
    if isinstance(href,bytes):
        href = href.decode('utf-8')
    (scheme, netloc, path, query, fragment) = urlsplit(href, scheme="", allow_fragments=True)
    if scheme != "":
        scheme += "://"
        href = href[len(scheme):]
    result = []
    for char in href:
        if char in IRI_UNSAFE:
            char = "%%%02x" % ord(char)
        result.append(char)
    return scheme + ''.join(result)

# unquotes url/iri
def unquoteurl(href):
    if isinstance(href,bytes):
        href = href.decode('utf-8')
    href = unquote(href)
    return href


def _remove_xml_header(data):
    newdata = data
    return re.sub(r'<\s*\?xml\s*[^\?>]*\?*>\s*','',newdata, count=1,flags=re.I)

def _well_formed(data):
    result = True 
    newdata = data
    if isinstance(newdata, str):
        newdata = newdata.encode('utf-8')
    try:
        parser = etree.XMLParser(encoding='utf-8', recover=False)
        tree = etree.parse(BytesIO(newdata), parser)
    except Exception:
        result = False
        pass
    return result

def _reformat(data):
    newdata = data
    if isinstance(newdata, str):
        newdata = newdata.encode('utf-8')
    parser = etree.XMLParser(encoding='utf-8', recover=True, ns_clean=True, 
                             remove_comments=True, remove_pis=True, strip_cdata=True, resolve_entities=False)
    tree = etree.parse(BytesIO(newdata), parser)
    newdata = etree.tostring(tree.getroot(),encoding='UTF-8', xml_declaration=False)
    return newdata 

# does not support cdata sections yet
def _make_it_sane(data):
    # first remove all comments as they may contain unescaped xml reserved characters
    # that will confuse the remaining _make_it_sane regular expressions
    comments = re.compile(r'''<!--.*?-->''', re.DOTALL)
    data = comments.sub("",data)
    # remove invalid tags that freak out lxml
    emptytag = re.compile(r'''(<\s*[/]*\s*>)''')
    data=emptytag.sub("", data);
    # handle double tag start
    badtagstart = re.compile(r'''(<[^>]*<)''')
    extrastart = re.compile(r'''<\s*<''');
    missingend = re.compile(r'''<\s*[a-zA-Z:]+[^<]*\s<''')
    startinattrib = re.compile(r'''<\s*[a-z:A-Z]+[^<]*["'][^<"']*<''')
    mo = badtagstart.search(data)
    while mo is not None:
        fixdata = data[mo.start(1):mo.end(1)]
        mextra = extrastart.match(fixdata)
        mmiss = missingend.match(fixdata)
        mattr = startinattrib.match(fixdata)
        if mextra is not None:
            fixdata = fixdata[1:]
        elif mattr is not None:
            fixdata = fixdata[0:-1] + "&lt;"
        elif mmiss is not None:
            fixdata = fixdata[0:-1].rstrip() + "> <"
        else:
            fixdata = "&lt;" + fixdata[1:]
        data = data[0:mo.start(1)] + fixdata + data[mo.end(1):]
        mo = badtagstart.search(data)
    
    # handle double tag end
    badtagend = re.compile(r'''(>[^<]*>)''')
    extraend = re.compile(r'''>\s*>''');
    missingstart = re.compile(r'''>\s[^>]*[a-zA-Z:]+[^>]*>''')
    endinattrib = re.compile(r'''>[^>]*["'][^>'"]*>''')
    mo = badtagend.search(data)
    while mo is not None:
        fixdata = data[mo.start(1):mo.end(1)]
        mextra = extraend.match(fixdata)
        mmiss = missingstart.match(fixdata)
        mattr = endinattrib.match(fixdata)
        if mextra is not None:
            fixdata = fixdata[0:-1]
        elif mattr is not None:
            fixdata = "&gt;" + fixdata[1:]
        elif mmiss is not None:
            fixdata = "> <" + fixdata[1:].lstrip()
        else:
            fixdata = fixdata[0:-1] + "&gt;"
        data = data[0:mo.start(1)] + fixdata + data[mo.end(1):]
        mo = badtagend.search(data)
    return data


# ncx_text_pattern = re.compile(r'''(<text>)\s*(\S[^<]*\S)\s*(</text>)''',re.IGNORECASE)
# re.sub(ncx_text_pattern,r'\1\2\3',newdata)

# data is expectedd to be in unicode
def WellFormedXMLErrorCheck(data, mtype=""):
    newdata = _remove_xml_header(data)
    if isinstance(newdata, str):
        newdata = newdata.encode('utf-8')
    line = "-1"
    column = "-1"
    message = "well-formed"
    try:
        parser = etree.XMLParser(encoding='utf-8', recover=False)
        tree = etree.parse(BytesIO(newdata), parser)
    except Exception:
        line = "0"
        column = "0"
        message = "exception"
        if len(parser.error_log) > 0:
            error = parser.error_log[0]
            message = error.message
            if isinstance(message, bytes):
                message = message.decode('utf-8')
            line = "%d" % error.line
            column = "%d" % error.column
        pass
    result = [line, column, message]
    return result


def IsWellFormedXML(data, mtype=""):
    [line, column, message] = WellFormedXMLErrorCheck(data, mtype)
    result = line == "-1"
    return result


# data is expected to be in unicode
# note: bs4 with lxml for xml strips whitespace so always prettyprint xml
def repairXML(data, mtype="", indent_chars="  "):
    newdata = _remove_xml_header(data)
    # if well-formed - don't mess with it
    if _well_formed(newdata):
        return data
    newdata = _make_it_sane(newdata)
    if not _well_formed(newdata):
        newdata = _reformat(newdata)
        if mtype == "application/oebps-package+xml":
            newdata = newdata.decode('utf-8')
            newdata = Opf_Parser(newdata).rebuild_opfxml()
    # lxml requires utf-8 on Mac, won't work with unicode
    if isinstance(newdata, str):
        newdata = newdata.encode('utf-8')
    voidtags = get_void_tags(mtype)
    xmlbuilder = LXMLTreeBuilderForXML(parser=None, empty_element_tags=voidtags)
    soup = BeautifulSoup(newdata, features=None, from_encoding="utf-8", builder=xmlbuilder)
    newdata = soup.decodexml(indent_level=0, formatter='minimal', indent_chars=indent_chars)
    return newdata


# this is used after a xhtml file split to update hrefs in the ncx
def anchorNCXUpdates(data, ncx_bookpath, originating_bookpath, keylist, valuelist):
    data = _remove_xml_header(data)
    # lxml on a Mac does not seem to handle full unicode properly, so encode as utf-8
    data = data.encode('utf-8')
    # rebuild serialized lookup dictionary
    id_dict = {}
    for i in range(0, len(keylist)):
        id_dict[ keylist[i] ] = valuelist[i]
    startdir = startingDir(ncx_bookpath)
    xmlbuilder = LXMLTreeBuilderForXML(parser=None, empty_element_tags=ebook_xml_empty_tags)
    soup = BeautifulSoup(data, features=None, from_encoding="utf-8", builder=xmlbuilder)
    for tag in soup.find_all("content"):
        if "src" in tag.attrs:
            src = tag["src"]
            if src.find(":") == -1:
                parts = src.split('#')
                ahref = unquoteurl(parts[0])
                # convert this href to its target bookpath
                target_bookpath = buildBookPath(ahref,startdir)
                if (parts is not None) and (len(parts) > 1) and (target_bookpath == originating_bookpath) and (parts[1] != ""):
                    fragment_id = parts[1]
                    if fragment_id in id_dict:
                        target_bookpath = id_dict[fragment_id]
                        attribute_value = buildRelativePath(ncx_bookpath, target_bookpath) + "#" + fragment_id
                        tag["src"] = quoteurl(attribute_value)
    newdata = soup.decodexml(indent_level=0, formatter='minimal', indent_chars="  ")
    return newdata


def anchorNCXUpdatesAfterMerge(data, ncx_bookpath, sink_bookpath, merged_bookpaths):
    data = _remove_xml_header(data)
    startdir = startingDir(ncx_bookpath)
    # lxml on a Mac does not seem to handle full unicode properly, so encode as utf-8
    data = data.encode('utf-8')
    xmlbuilder = LXMLTreeBuilderForXML(parser=None, empty_element_tags=ebook_xml_empty_tags)
    soup = BeautifulSoup(data, features=None, from_encoding="utf-8", builder=xmlbuilder)
    for tag in soup.find_all("content"):
        if "src" in tag.attrs:
            src = tag["src"]
            if src.find(":") == -1:
                parts = src.split('#')
                if parts is not None:
                    ahref = unquoteurl(parts[0])
                    target_bookpath = buildBookPath(ahref, startdir)
                    if target_bookpath in merged_bookpaths:
                        attribute_value = buildRelativePath(ncx_bookpath, sink_bookpath)
                        if len(parts) > 1 and parts[1] != "":
                            attribute_value += "#" + parts[1]
                        tag["src"] = quoteurl(attribute_value)
    newdata = soup.decodexml(indent_level=0, formatter='minimal', indent_chars="  ")
    return newdata


def performNCXSourceUpdates(data, newbkpath, oldbkpath, keylist, valuelist):
    data = _remove_xml_header(data)
    # lxml on a Mac does not seem to handle full unicode properly, so encode as utf-8
    data = data.encode('utf-8')
    # rebuild serialized lookup dictionary
    updates = {}
    for i in range(0, len(keylist)):
        updates[ keylist[i] ] = valuelist[i]
    xmlbuilder = LXMLTreeBuilderForXML(parser=None, empty_element_tags=ebook_xml_empty_tags)
    soup = BeautifulSoup(data, features=None, from_encoding="utf-8", builder=xmlbuilder)
    for tag in soup.find_all("content"):
        if "src" in tag.attrs:
            src = tag["src"]
            if src.find(":") == -1:
                parts = src.split('#')
                ahref = unquoteurl(parts[0])
                fragment = ""
                if len(parts) > 1:
                    fragment = parts[1]
                oldtarget = buildBookPath(ahref, startingDir(oldbkpath))
                newtarget = updates.get(oldtarget, oldtarget)
                attribute_value = buildRelativePath(newbkpath, newtarget)
                if fragment != "":
                    attribute_value = attribute_value + "#" + fragment
                attribute_value = quoteurl(attribute_value)
                tag["src"] = attribute_value
    newdata = soup.decodexml(indent_level=0, formatter='minimal', indent_chars="  ")
    return newdata


def performOPFSourceUpdates(data, newbkpath, oldbkpath, keylist, valuelist):
    data = _remove_xml_header(data)
    # lxml on a Mac does not seem to handle full unicode properly, so encode as utf-8
    data = data.encode('utf-8')
    # rebuild serialized lookup dictionary
    updates = {}
    for i in range(0, len(keylist)):
        updates[ keylist[i] ] = valuelist[i]
    xmlbuilder = LXMLTreeBuilderForXML(parser=None, empty_element_tags=ebook_xml_empty_tags)
    soup = BeautifulSoup(data, features=None, from_encoding="utf-8", builder=xmlbuilder)
    for tag in soup.find_all(["link","item","reference","site"]):
        if "href" in tag.attrs :
            href = tag["href"]
            if href.find(":") == -1 :
                parts = href.split('#')
                ahref = unquoteurl(parts[0])
                fragment = ""
                if len(parts) > 1:
                    fragment = parts[1]
                oldtarget = buildBookPath(ahref, startingDir(oldbkpath))
                newtarget = updates.get(oldtarget, oldtarget)
                attribute_value = buildRelativePath(newbkpath, newtarget)
                if fragment != "":
                    attribute_value = attribute_value + "#" + fragment
                attribute_value = quoteurl(attribute_value)
                tag["href"] = attribute_value
    newdata = soup.decodexml(indent_level=0, formatter='minimal', indent_chars="  ")
    return newdata


# Note xml_updates has paths relative to the OEBPS folder as base
# As if they were meant only for OEBPS/content.opf and OEBPS/toc.ncx
# So adjust them to be relative to the Misc directory where .smil files live in Sigil
def performSMILUpdates(data, newbkpath, oldbkpath, keylist, valuelist):
    data = _remove_xml_header(data)
    # lxml on a Mac does not seem to handle full unicode properly, so encode as utf-8
    data = data.encode('utf-8')
    # rebuild serialized lookup dictionary of xml_updates, properly adjusted
    updates = {}
    for i in range(0, len(keylist)):
        updates[ keylist[i] ] = "../" + valuelist[i]
    xml_empty_tags = ["text", "audio"]
    xmlbuilder = LXMLTreeBuilderForXML(parser=None, empty_element_tags=xml_empty_tags)
    soup = BeautifulSoup(data, features=None, from_encoding="utf-8", builder=xmlbuilder)
    for tag in soup.find_all(["body","seq","text","audio"]):
        for att in ["src", "epub:textref"]:
            if att in tag.attrs :
                ref = tag[att]
                if ref.find(":") == -1 :
                    parts = ref.split('#')
                    ahref = unquoteurl(parts[0])
                    fragment = ""
                    if len(parts) > 1:
                        fragment = parts[1]
                    oldtarget = buildBookPath(ahref, startingDir(oldbkpath))
                    newtarget = updates.get(oldtarget, oldtarget)
                    attribute_value = buildRelativePath(newbkpath, newtarget)
                    if fragment != "":
                        attribute_value = attribute_value + "#" + fragment
                    attribute_value = quoteurl(attribute_value)
                    tag[att] = attribute_value
    newdata = soup.decodexml(indent_level=0, formatter='minimal', indent_chars="  ")
    return newdata

# Note xml_updates has urls/iris relative to the OEBPS folder as base
# As if they were meant only for OEBPS/content.opf and OEBPS/toc.ncx
# So adjust them to be relative to the Misc directory where page-map.xml lives
def performPageMapUpdates(data, newbkpath, oldbkpath, keylist, valuelist):
    data = _remove_xml_header(data)
    # lxml on a Mac does not seem to handle full unicode properly, so encode as utf-8
    data = data.encode('utf-8')
    # rebuild serialized lookup dictionary of xml_updates properly adjusted
    updates = {}
    for i in range(0, len(keylist)):
        updates[ keylist[i] ] = "../" + valuelist[i]
    xml_empty_tags = ["page"]
    xmlbuilder = LXMLTreeBuilderForXML(parser=None, empty_element_tags=xml_empty_tags)
    soup = BeautifulSoup(data, features=None, from_encoding="utf-8", builder=xmlbuilder)
    for tag in soup.find_all(["page"]):
        for att in ["href"]:
            if att in tag.attrs :
                ref = tag[att]
                if ref.find(":") == -1 :
                    parts = ref.split('#')
                    ahref = unquoteurl(parts[0])
                    fragment = ""
                    if len(parts) > 1:
                        fragment = parts[1]
                    oldtarget = buildBookPath(ahref, startingDir(oldbkpath))
                    newtarget = updates.get(oldtarget, oldtarget)
                    attribute_value = buildRelativePath(newbkpath, newtarget)
                    if fragment != "":
                        attribute_value = attribute_value + "#" + fragment
                    attribute_value = quoteurl(attribute_value)
                    tag[att] = attribute_value
    newdata = soup.decodexml(indent_level=0, formatter='minimal', indent_chars="  ")
    return newdata


def main():
    argv = sys.argv
    opfxml = '''
<?xml version="1.0" encoding="utf-8" standalone="yes"?>
<package xmlns="http://www.idpf.org/2007/opf" unique-identifier="BookId" version="2.0">
  <metadata xmlns:mydc="http://purl.org/dc/elements/1.1/" xmlns:opf="http://www.idpf.org/2007/opf">
    <mydc:identifier id="BookId" opf:scheme="UUID">urn:uuid:a418a8f1-dcbc-4c5d-a18f-533765e34ee8</mydc:identifier>
  </metadata>
  <manifest>
    <!-- this has a lot of bad characters & < > \" \'-->
    <item href="toc.ncx" id="ncx" media-type="application/x-dtbncx+xml" />
    <item href="Text/Section0001.xhtml" id="Section0001.xhtml" media-type="application/xhtml+xml" />
  </manifest>
< 
  <spine toc="ncx">
    <itemref idref="Section0001.xhtml">
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
    print(argv)
    if not argv[-1].endswith("xmlprocessor.py"):
        with open(argv[-1],'rb') as f:
            opfxml = f.read();
            if isinstance(opfxml, bytes):
                opfxml = opfxml.decode('utf-8')

    print(repairXML(opfxml, "application/oebps-package+xml"))
    return 0

if __name__ == '__main__':
    sys.exit(main())
