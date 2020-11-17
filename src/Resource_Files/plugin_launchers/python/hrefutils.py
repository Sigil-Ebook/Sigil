#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# vim:ts=4:sw=4:softtabstop=4:smarttab:expandtab

import sys

from urllib.parse import unquote
from urllib.parse import urlsplit

# default to using the preferred media-types ffrom the epub 3.2 spec
# https://www.w3.org/publishing/epub3/epub-spec.html#sec-cmt-supported

ext_mime_map = {
    '.bm'    : 'image/bmp',
    '.bmp'   : 'image/bmp',
    '.css'   : 'text/css',
    '.epub'  : 'application/epub+zip',
    '.gif'   : 'image/gif',
    '.htm'   : 'application/xhtml+xml',
    '.html'  : 'application/xhtml+xml',
    '.jpeg'  : 'image/jpeg',
    '.jpg'   : 'image/jpeg',
    '.js'    : 'application/javascript',
    '.m4a'   : 'audio/mp4',
    '.m4v'   : 'video/mp4',
    '.mp3'   : 'audio/mpeg',
    '.mp4'   : 'video/mp4',
    '.ncx'   : 'application/x-dtbncx+xml',
    '.oga'   : 'audio/ogg',
    '.ogg'   : 'audio/ogg',
    '.ogv'   : 'video/ogg',
    '.opf'   : 'application/oebps-package+xml',
    '.otf'   : 'font/otf',
    '.pls'   : 'application/pls+xml',
    '.png'   : 'image/png',
    '.smil'  : 'application/smil+xml',
    '.svg'   : 'image/svg+xml',
    '.tif'   : 'image/tiff',
    '.tiff'  : 'image/tiff',
    '.ttc'   : 'font/collection',
    '.ttf'   : 'font/ttf',
    '.ttml'  : 'application/ttml+xml',
    '.txt'   : 'text/plain',
    '.vtt'   : 'text/vtt',
    '.webm'  : 'video/webm',
    '.webp'  : 'image/webp',
    '.woff'  : 'font/woff',
    '.woff2' : 'font/woff2',
    '.xhtml' : 'application/xhtml+xml',
    '.xml'   : 'application/oebps-page-map+xml',
    '.xpgt'  : 'application/vnd.adobe-page-template+xml',
    # '.js'    : = "text/javascript',
    # '.otf'   : 'application/x-font-opentype',
    # '.otf'   : 'application/font-sfnt',
}


# deprecated font mediatypes
# See https://www.iana.org/assignments/media-types/media-types.xhtml#font

mime_group_map = {
    'image/jpeg'                              : 'Images',
    'image/png'                               : 'Images',
    'image/gif'                               : 'Images',
    'image/svg+xml'                           : 'Images',
    'image/bmp'                               : 'Images',  # not a core media type
    'image/tiff'                              : 'Images',  # not a core media type
    'image/webp'                              : 'Images',  # not a core media type
    'text/html'                               : 'Text',
    'application/xhtml+xml'                   : 'Text',
    'application/x-dtbook+xml'                : 'Text',
    'font/woff2'                              : 'Fonts',
    'font/woff'                               : 'Fonts',
    'font/ttf'                                : 'Fonts',
    'font/otf'                                : 'Fonts',
    'font/sfnt'                               : 'Fonts',
    'font/collection'                         : 'Fonts',
    'application/vnd.ms-opentype'             : 'Fonts',
    'application/font-sfnt'                   : 'Fonts',  # deprecated
    'application/font-ttf'                    : 'Fonts',  # deprecated
    'application/font-otf'                    : 'Fonts',  # deprecated
    'application/font-woff'                   : 'Fonts',  # deprecated
    'application/font-woff2'                  : 'Fonts',  # deprecated
    'application/x-font-ttf'                  : 'Fonts',  # deprecated
    'application/x-truetype-font'             : 'Fonts',  # deprecated
    'application/x-opentype-font'             : 'Fonts',  # deprecated
    'application/x-font-ttf'                  : 'Fonts',  # deprecated
    'application/x-font-otf'                  : 'Fonts',  # deprecated
    'application/x-font-opentype'             : 'Fonts',  # deprecated
    'application/x-font-truetype'             : 'Fonts',  # deprecated
    'application/x-font-truetype-collection'  : 'Fonts',  # deprecated
    'audio/mpeg'                              : 'Audio',
    'audio/mp3'                               : 'Audio',
    'audio/mp4'                               : 'Audio',
    'audio/ogg'                               : 'Audio',  # not a core media type
    'video/mp4'                               : 'Video',
    'video/ogg'                               : 'Video',
    'video/webm'                              : 'Video',
    'text/vtt'                                : 'Video',
    'application/ttml+xml'                    : 'Video',
    'text/css'                                : 'Styles',
    'application/x-dtbncx+xml'                : 'ncx',
    'application/oebps-package+xml'           : 'opf',
    'application/oebps-page-map+xml'          : 'Misc',
    'application/vnd.adobe-page-map+xml'      : 'Misc',
    'application/vnd.adobe.page-map+xml'      : 'Misc',
    'application/smil+xml'                    : 'Misc',
    'application/adobe-page-template+xml'     : 'Misc',
    'application/vnd.adobe-page-template+xml' : 'Misc',
    'text/javascript'                         : 'Misc',
    'application/javascript'                  : 'Misc',
    'application/pls+xml'                     : 'Misc',
    'text/plain'                              : 'Misc',
}

# Note any # char in the href path component must be url encoded
# if fragments can exist

_ASCII_CHARS   = set(chr(x) for x in range(128))

_URL_SAFE      = set('ABCDEFGHIJKLMNOPQRSTUVWXYZ'
                     'abcdefghijklmnopqrstuvwxyz'
                     '0123456789' '_.-/~')


# From the IRI spec rfc3987
# iunreserved    = ALPHA / DIGIT / "-" / "." / "_" / "~" / ucschar
# 
#    ucschar        = %xA0-D7FF / %xF900-FDCF / %xFDF0-FFEF
#                   / %x10000-1FFFD / %x20000-2FFFD / %x30000-3FFFD
#                   / %x40000-4FFFD / %x50000-5FFFD / %x60000-6FFFD
#                   / %x70000-7FFFD / %x80000-8FFFD / %x90000-9FFFD
#                   / %xA0000-AFFFD / %xB0000-BFFFD / %xC0000-CFFFD
#                   / %xD0000-DFFFD / %xE1000-EFFFD
# But currently nothing *after* the 0x30000 plane is even defined
def need_to_percent_encode(char):
    cp = ord(char)
    if cp < 128:
        if char in _URL_SAFE: return False;
        return True;
    if cp <  0xA0: return True;
    if cp <= 0xD7FF: return False;
    if cp <  0xF900: return True;
    if cp <= 0xFDCF: return False;
    if cp <  0xFDF0: return True;
    if cp <= 0xFFEF: return False;
    if cp <  0x10000: return True;
    if cp <= 0x1FFFD: return False;
    if cp <  0x20000: return True;
    if cp <= 0x2FFFD: return False;
    if cp <  0x30000: return True;
    if cp <= 0x3FFFD: return False;
    return True;


def urlencodepart(part):
    if isinstance(part,bytes):
        part = part.decode('utf-8')
    result = []
    for char in part:
        if need_to_percent_encode(char):
            bs = char.encode('utf-8')
            for b in bs:
                result.append("%%%02X" % b)
        else:
            result.append(char)
    return ''.join(result)


def urldecodepart(part):
    if isinstance(part,bytes):
        part = part.decode('utf-8')
    part = unquote(part)
    return part


# return a properly url encoded relative href
# from path and fragment components
def getRelativeHREF(apath, afragment):
    if isinstance(apath, bytes):
        apath = apath.decode('utf-8')
    if afragment and isinstance(afragment, bytes):
        afragment = afragment.decode('utf-8')
    href = urlencodepart(apath)
    if afragment:
        href = href + "#" + urlencodepart(fragment)
    return href


# return a properly url decoded path
# and fragment (None) from a url encoded relative href
def parseRelativeHREF(href):
    if isinstance(href, bytes):
        href = href.decode('utf-8')
    parts = href.split('#')
    apath = urldecodepart(parts[0])
    afragment = None
    if len(parts) > 1:
        afragment = urldecodepart(parts[1])
    return apath, afragment


def relativePath(to_bkpath, start_dir):
    # remove any trailing path separators from both paths
    dsegs = to_bkpath.rstrip('/').split('/')
    ssegs = start_dir.rstrip('/').split('/')
    if dsegs == ['']: dsegs = []
    if ssegs == ['']: ssegs = []
    res = []
    i = 0
    for s1, s2 in zip(dsegs, ssegs):
        if s1 != s2: break
        i += 1
    for p in range(i, len(ssegs), 1): res.append('..')
    for p in range(i, len(dsegs), 1): res.append(dsegs[p])
    return '/'.join(res)


def resolveRelativeSegmentsInFilePath(file_path):
    res = []
    segs = file_path.split('/')
    for i in range(len(segs)):
        if segs[i] == '.': continue
        if segs[i] == '..':
            if res:
                res.pop()
            else:
                print("Error resolving relative path segments")
        else:
            res.append(segs[i])
    return '/'.join(res)


def buildRelativePath(from_bkpath, to_bkpath):
    if from_bkpath == to_bkpath: return ""
    return relativePath(to_bkpath, startingDir(from_bkpath))


def buildBookPath(dest_relpath, start_folder):
    if start_folder == "" or start_folder.strip() == "":
        return dest_relpath
    bookpath = start_folder.rstrip('/') + '/' + dest_relpath
    return resolveRelativeSegmentsInFilePath(bookpath)


def startingDir(file_path):
    ssegs = file_path.split('/')
    ssegs.pop()
    return '/'.join(ssegs)


def longestCommonPath(bookpaths):
    # handle special cases
    if len(bookpaths) == 0: return ""
    if len(bookpaths) == 1: return startingDir(bookpaths[0]) + '/'
    fpaths = bookpaths
    fpaths.sort()
    segs1 = fpaths[0].split('/')
    segs2 = fpaths[-1].split('/')
    res = []
    for s1, s2 in zip(segs1, segs2):
        if s1 != s2: break
        res.append(s1)
    if not res or len(res) == 0:
        return ""
    return '/'.join(res) + '/'


# DEPRECATED: kept only for backwards compatibility
# as it assumes no # chars will ever exist in an href path
_XURL_SAFE      = set('ABCDEFGHIJKLMNOPQRSTUVWXYZ'
                      'abcdefghijklmnopqrstuvwxyz'
                      '0123456789' '#' '_.-/~')
_XIRI_UNSAFE = _ASCII_CHARS - _XURL_SAFE


# DEPRECATED: kept only for backwards compatibility
# as it assumes no # chars will ever exist in an href path
# returns a quoted IRI (not a URI)
def quoteurl(href):
    if isinstance(href, bytes):
        href = href.decode('utf-8')
    (scheme, netloc, path, query, fragment) = urlsplit(href, scheme="", allow_fragments=True)
    if scheme != "":
        scheme += "://"
        href = href[len(scheme):]
    result = []
    for char in href:
        if char in _XIRI_UNSAFE:
            char = "%%%02x" % ord(char)
        result.append(char)
    return scheme + ''.join(result)


# DEPRECATED: kept only for backwards compatibility
# as it assumes no # chars will ever exist in an href path
# unquotes url/iri
def unquoteurl(href):
    if isinstance(href, bytes):
        href = href.decode('utf-8')
    href = unquote(href)
    return href


def main():
    p1 = 'This/is/the/../../end.txt'
    print('Testing resolveRelativeSegmentsInFilePath(file_path)')
    print('    file_path: ', p1)
    print(resolveRelativeSegmentsInFilePath(p1))
    print('    ')

    p1 = 'hello.txt'
    p2 = 'goodbye.txt'
    print('Testing buildRelativePath(from_bkpath,to_bkpath')
    print('    from_bkpath: ', p1)
    print('    to_bkpath:   ', p2)
    print(buildRelativePath(p1, p2))
    print('    ')

    p1 = 'OEBPS/Text/book1/chapter1.xhtml'
    p2 = 'OEBPS/Text/book2/chapter1.xhtml'
    print('Testing buildRelativePath(from_bkpath,to_bkpath)')
    print('    from_bkpath: ', p1)
    print('    to_bkpath:   ', p2)
    print(buildRelativePath(p1, p2))
    print('    ')

    p1 = 'OEBPS/package.opf'
    p2 = 'OEBPS/Text/book1/chapter1.xhtml'
    print('Testing buildRelativePath(from_bkpath, to_bkpath)')
    print('    from_bkpath: ', p1)
    print('    to_bkpath:   ', p2)
    print(buildRelativePath(p1, p2))
    print('    ')

    p1 = '../../Images/image.png'
    p2 = 'OEBPS/Text/book1/'
    print('Testing buildBookPath(destination_href, start_dir)')
    print('    destination_href: ', p1)
    print('    starting_dir:     ', p2)
    print(buildBookPath(p1, p2))
    print('    ')

    p1 = 'image.png'
    p2 = ''
    print('Testing buildBookPath(destination_href, start_dir)')
    print('    destination_href: ', p1)
    print('    starting_dir:     ', p2)
    print(buildBookPath(p1, p2))
    print('    ')

    p1 = 'content.opf'
    print('Testing startingDir(bookpath')
    print('    bookpath: ', p1)
    print('"' + startingDir(p1) + '"')
    print('    ')

    bookpaths = []
    bookpaths.append('OEBPS/book1/text/chapter1.xhtml')
    bookpaths.append('OEBPS/book1/html/chapter2.xhtml')
    bookpaths.append('OEBPS/book2/text/chapter3.xhtml')
    print('Testing longestCommonPath(bookpaths)')
    print('    bookpaths: ', bookpaths)
    print('"' + longestCommonPath(bookpaths) + '"')
    print('    ')

    return 0


if __name__ == '__main__':
    sys.exit(main())
