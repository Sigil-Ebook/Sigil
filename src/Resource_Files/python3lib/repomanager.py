#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# vim:ts=4:sw=4:softtabstop=4:smarttab:expandtab

# Copyright (c) 2020 Kevin B. Hendricks, Stratford Ontario Canada
# All rights reserved.
#
# This file is part of Sigil.
#
#  Sigil is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  Sigil is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with Sigil.  If not, see <http://www.gnu.org/licenses/>.

import sys
import os
import re
import dulwich
from dulwich import porcelain

import zlib
import zipfile
from zipfile import ZipFile

from contextlib import contextmanager

@contextmanager
def make_temp_directory():
    import tempfile
    import shutil
    temp_dir = tempfile.mkdtemp()
    yield temp_dir
    shutil.rmtree(temp_dir)

_SKIP_LIST = [
    'encryption.xml',
    'rights.xml',
    '.gitignore',
    '.gitattributes'
]

# convert string to utf-8
def utf8_str(p, enc='utf-8'):
    if p is None:
        return None
    if isinstance(p, str):
        return p.encode('utf-8')
    if enc != 'utf-8':
        return p.decode(enc, errors='replace').encode('utf-8')
    return p

# convert string to be unicode encoded
def unicode_str(p, enc='utf-8'):
    if p is None:
        return None
    if isinstance(p, str):
        return p
    return p.decode(enc, errors='replace')

fsencoding = sys.getfilesystemencoding()

# handle paths that might be filesystem encoded
def pathof(s, enc=fsencoding):
    if s is None:
        return None
    if isinstance(s, str):
        return s
    if isinstance(s, bytes):
        try:
            return s.decode(enc)
        except:
            pass
    return s

# properly handle relative paths
def relpath(path, start=None):
    return os.path.relpath(pathof(path) , pathof(start))

# generate a list of files in a folder
def walk_folder(top):
    top = pathof(top)
    rv = []
    for base, dnames, names  in os.walk(top):
        base = pathof(base)
        for name in names:
            name = pathof(name)
            rv.append(relpath(os.path.join(base, name), top))
    return rv


# borrowed from calibre from calibre/src/calibre/__init__.py
# added in removal of non-printing chars
# and removal of . at start
def cleanup_file_name(name):
    import string
    _filename_sanitize = re.compile(r'[\xae\0\\|\?\*<":>\+/]')
    substitute='_'
    one = ''.join(char for char in name if char in string.printable)
    one = _filename_sanitize.sub(substitute, one)
    one = re.sub(r'\s', '_', one).strip()
    one = re.sub(r'^\.+$', '_', one)
    one = one.replace('..', substitute)
    # Windows doesn't like path components that end with a period
    if one.endswith('.'):
        one = one[:-1]+substitute
    # Mac and Unix don't like file names that begin with a full stop
    if len(one) > 0 and one[0:1] == '.':
        one = substitute+one[1:]
    return one

# routine to copy the files internal to Sigil for the epub being edited
# to a destination folder
#   bookroot is path to root folder of epub inside Sigil
#   bookfiles is list of all bookpaths (relative to bookroot) that make up the epub
#   path to the destination folder
def copy_book_contents_to_destination(bookroot, bookfiles, destdir):
    copied = []
    destdir = pathof(destdir)
    root = bookroot.replace('/', os.sep)
    for bookpath in bookfiles:
        # convert to os specific path
        bookpath = bookpath.replace('/', os.sep)
        src = os.path.join(root, bookpath)
        dest = os.path.join(destdir, bookpath)
        # and make sure destination directory exists
        base = os.path.dirname(dest)
        if not os.path.exists(base):
            os.makedirs(base)
        data = b''
        with open(src, 'rb') as f:
            data = f.read()
        with open(dest,'wb') as fp:
            fp.write(data)
        copied.append(bookpath)
    # Finally Add the proper mimetype file
    data = "application/epub+zip"
    with open(os.path.join(destdir,"mimetype"),'wb') as fm:
        fm.write(data.encode('utf-8'))
    copied.append("mimetype")
    return copied

# return True if file should be copied to destination folder
def valid_file_to_copy(rpath):
    segs = rpath.split(os.sep)
    if ".git" in segs:
        return False
    filename = os.path.basename(rpath)
    keep = filename not in _SKIP_LIST
    return keep


def build_epub_from_folder_contents(foldpath, epub_filepath):
    outzip = zipfile.ZipFile(pathof(epub_filepath), mode='w')
    files = walk_folder(foldpath)
    if 'mimetype' in files:
        outzip.write(pathof(os.path.join(foldpath, 'mimetype')), pathof('mimetype'), zipfile.ZIP_STORED)
    else:
        raise Exception('mimetype file is missing')
    files.remove('mimetype')
    for file in files:
        if valid_file_to_copy(file):
            filepath = os.path.join(foldpath, file)
            outzip.write(pathof(filepath),pathof(file),zipfile.ZIP_DEFLATED)
    outzip.close()


def create_epub(booktitle, foldpath):
    # use title to create epub name
    epubname = cleanup_file_name(booktitle) + ".epub"            
    rv = -1
    data = b''        
    with make_temp_directory() as scratchdir:
        epubpath = os.path.join(scratchdir,epubname)
        try:
            build_epub_from_folder_contents(foldpath, epubpath)
            with open(epubpath,'rb') as fp:
                data = fp.read()
            rv = 0
        except Exception as e:
            print("Import from Folder failed")
            print(str(e))
            rv = -1

    if rv == -1:
        return -1
    return 0


# the entry points
def performCommit(localRepo, bookid, booktitle, bookroot, bookfiles):
    has_error = False
    staged = []
    added=[]
    ignored=[]
    repo_home = localRepo.replace("/", os.sep)
    repo_path = os.path.join(repo_home, "epub_" + bookid)
    if os.path.exists(repo_path):
        # handle updating the staged files and commiting and tagging
        print("not implemented yet")
    else:
        # this will be an initial commit to this repo
        tagname = 'V0001'
        os.makedirs(repo_path)
        cdir = os.getcwd()
        os.chdir(repo_path)
        repo = porcelain.init(path='.', bare=False)
        staged = copy_book_contents_to_destination(bookroot, bookfiles, repo_path)
        (added, ignored) = porcelain.add(repo='.',paths=staged)
        commit_sha1 = porcelain.commit(repo='.',message="Initial Commit", author=None, committer=None)
        tag = porcelain.tag_create(repo='.', tag=tagname, message="Tagging..." + tagname, author=None)
        os.chdir(cdir)

    result = "\n".join(added);
    result = result + "***********" + "\n".join(ignored)
    if not has_error:
        return result;
    return ''

def main():
    argv = sys.argv
    return 0

if __name__ == '__main__':
    sys.exit(main())
