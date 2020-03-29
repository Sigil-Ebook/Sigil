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
import shutil
import datetime
import time
import io
from io import BytesIO
from io import StringIO
import filecmp

from diffstat import diffstat
from sdifflibparser import DiffCode, DifflibParser

# Work around dulwich assumption about sys.argv being defined,
# which is not automatically the case on Linux with distribution-provided
# embedded Python versions older than 3.8.
if (sys.hexversion < 0x03080000) and not hasattr(sys, 'argv'):
    sys.argv = ['']

import dulwich
from dulwich import porcelain
from dulwich.repo import Repo
from dulwich.porcelain import open_repo_closing, show_object, print_commit, commit_decode
from dulwich.objects import Tag, Commit, Blob, check_hexsha, ShaFile, Tree, format_timezone
from dulwich.refs import ANNOTATED_TAG_SUFFIX
from dulwich.patch import write_tree_diff

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

_SKIP_COPY_LIST = [
    'encryption.xml',
    'rights.xml',
    '.gitignore',
    '.gitattributes',
    '.bookinfo'
]

_SKIP_CLEAN_LIST = [
    '.gitignore',
    '.gitattributes',
    '.bookinfo',
    '.git'
]

_SIGIL = b"Sigil <sigil@sigil-ebook.com>"

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
            apath = relpath(os.path.join(base, name), top)
            if not apath.startswith(".git"):
                if not os.path.basename(apath) in _SKIP_CLEAN_LIST:
                    rv.append(apath)
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
def copy_book_contents_to_destination(book_home, filepaths, destdir):
    copied = []
    for apath in filepaths:
        src = os.path.join(book_home, apath)
        dest = os.path.join(destdir, apath)
        # and make sure destination directory exists
        base = os.path.dirname(dest)
        if not os.path.exists(base):
            os.makedirs(base)
        data = b''
        with open(src, 'rb') as f:
            data = f.read()
        with open(dest,'wb') as fp:
            fp.write(data)
        copied.append(apath)
    # Finally Add the proper mimetype file
    data = b"application/epub+zip"
    with open(os.path.join(destdir,"mimetype"),'wb') as fm:
        fm.write(data)
    copied.append("mimetype")
    return copied

def add_gitignore(repo_path):
    ignoredata = []
    ignoredata.append(".DS_Store")
    ignoredata.append("*~")
    ignoredata.append("*.orig")
    ignoredata.append("*.bak")
    ignoredata.append(".bookinfo")
    ignoredata.append(".gitignore")
    ignoredata.append(".gitattributes")
    ignoredata.append("")
    data = "\n".join(ignoredata).encode('utf-8')
    with open(os.path.join(repo_path, ".gitignore"),'wb') as f1:
        f1.write(data)


def add_gitattributes(repo_path):
    adata = []
    adata.append(".git export-ignore")
    adata.append(".gitattributes export-ignore")
    adata.append(".gitignore export-ignore")
    adata.append(".bookinfo export-ignore")
    adata.append("")
    data = "\n".join(adata).encode('utf-8')
    with open(os.path.join(repo_path, ".gitattributes"),'wb') as f3:
        f3.write(data)

def add_bookinfo(repo_path, bookinfo, bookid, tagname):
    binfo_path = os.path.join(repo_path,".bookinfo");
    if os.path.exists(binfo_path):
        os.remove(binfo_path)
    (filename, booktitle, datetime) = bookinfo
    booktitle = booktitle.replace("\n", " ")
    bkdata = []
    bkdata.append(filename)
    bkdata.append(booktitle)
    bkdata.append(datetime)
    bkdata.append(tagname)
    bkdata.append(bookid)
    bkdata.append("")
    data = "\n".join(bkdata).encode('utf-8')
    with open(binfo_path,'wb') as f2:
        f2.write(data)

# return True if file should be copied to destination folder
def valid_file_to_copy(rpath):
    segs = rpath.split(os.sep)
    if ".git" in segs:
        return False
    filename = os.path.basename(rpath)
    keep = filename not in _SKIP_COPY_LIST
    return keep


# clean dulwich repo skipping specific files and folders
# returns true on success, false otherwise
# folder is a full path to the folder
# make sure the cwd is not this folder to prevent erase issues
def cleanWorkingDir(folder):
    result = True
    if os.path.exists(folder):
        for the_obj in os.listdir(folder):
            obj_path = os.path.join(folder, the_obj)
            if the_obj not in _SKIP_CLEAN_LIST:
                if os.path.isfile(obj_path):
                    try:
                        os.remove(obj_path)
                    except Exception as e:
                        result = False
                        print(str(e))
                        pass
                else:
                    # a subdirectory is found
                    result = cleanWorkingDir(obj_path)
                    if not result: return result
                    try:
                        os.rmdir(obj_path)
                    except Exception as e:
                        result = False
                        print(str(e))
                        pass
            if not result: return result;
    return result


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

# will lose any untracked or unstaged changes    
# so add and commit to keep them before using this
# repo_path here must be a full path
def checkout_tag(repo_path, tagname):
    result = True
    cdir = os.getcwd()
    result = cleanWorkingDir(repo_path)
    if not result: return result
    os.chdir(repo_path)
    with open_repo_closing(".") as r:
        tagkey = utf8_str("refs/tags/" + tagname)
        refkey = tagkey
        # if annotated tag get the commit it pointed to
        if isinstance(r[tagkey], Tag):
            refkey = r[tagkey].object[1]
        r.reset_index(r[refkey].tree)
        # use this to reset HEAD to this tag (ie. revert)
        # r.refs.set_symbolic_ref(b"HEAD", tagkey)
        # cd out **before** the repo closes
        os.chdir(cdir) 
    os.chdir(cdir)
    return result


# will lose any untracked or unstaged changes    
# so add and commit to keep them before using this
# repo_path must be a full path
# Note: the Working Directory should always be left with HEAD checked out
def checkout_head(repo_path):
    result = True
    cdir = os.getcwd()
    result = cleanWorkingDir(repo_path)
    if not result:
        return result
    os.chdir(repo_path)
    with open_repo_closing(".") as r:
        r.reset_index(r[b"HEAD"].tree)
        # cd out **before** the repo closes
        os.chdir(cdir)
    os.chdir(cdir)
    return result

def clone_repo_and_checkout_tag(localRepo, bookid, tagname, filename, dest_path):
    repo_home = pathof(localRepo)
    repo_home = repo_home.replace("/", os.sep)
    repo_path = os.path.join(repo_home, "epub_" + bookid)
    dest_path = dest_path.replace("/", os.sep)
    cdir = os.getcwd()
    # first verify both repo and tagname exist
    taglst = []
    if os.path.exists(repo_path):
        if not os.path.exists(dest_path): return ""
        os.chdir(repo_path)
        tags = porcelain.list_tags(repo='.')
        for atag in tags:
            taglst.append(unicode_str(atag))
        # use dest_path to clone into
        # clone current repo "s" into new repo "r"
        with open_repo_closing(".") as s:
            s.clone(dest_path, mkdir=False, bare=False, origin=b"origin", checkout=False)
            # cd out **before** the repo closes
            os.chdir(dest_path)
        with open_repo_closing(".") as r:
            if tagname not in taglist or tagname == "HEAD":
                tagkey = utf8_str("HEAD")
            else:
                tagkey = utf8_str("refs/tags/" + tagname)
            refkey = tagkey
            # if annotated tag get the commit id it pointed to instead
            if isinstance(r[tagkey], Tag):
                refkey = r[tagkey].object[1]
            r.reset_index(r[refkey].tree)
            r.refs.set_symbolic_ref(b"HEAD", tagkey)
            # cd out **before** the repo closes
            os.chdir(cdir)
    return "success"


def logsummary(repo=".", paths=None, outstream=sys.stdout, max_entries=None, reverse=False, stats=False):
    """Write commit logs with optional diff stat summaries
    Args:
      repo: Path to repository
      paths: Optional set of specific paths to print entries for
      outstream: Stream to write log output to
      reverse: Reverse order in which entries are printed
      max_entries: Optional maximum number of entries to display
      stats: Print diff stats
    """
    with open_repo_closing(repo) as r:
        walker = r.get_walker(max_entries=max_entries, paths=paths, reverse=reverse)
        for entry in walker:
            def decode(x):
                return commit_decode(entry.commit, x)
            print_commit(entry.commit, decode, outstream)
            if stats:
                commit = entry.commit
                if commit.parents:
                    parent_commit = r[commit.parents[0]]
                    base_tree = parent_commit.tree
                else:
                    base_tree = None
                adiff = b""
                with BytesIO() as diffstream:
                    write_tree_diff(
                        diffstream,
                        r.object_store, base_tree, commit.tree)
                    diffstream.seek(0)
                    adiff = diffstream.getvalue()
                dsum = diffstat(adiff.split(b'\n'))
                outstream.write(dsum.decode('utf-8'))
                outstream.write("\n\n")


# the entry points from Cpp

def generate_epub_from_tag(localRepo, bookid, tagname, filename, dest_path):
    repo_home = pathof(localRepo)
    repo_home = repo_home.replace("/", os.sep)
    repo_path = os.path.join(repo_home, "epub_" + bookid)
    cdir = os.getcwd()
    # first verify both repo and tagname exist
    epub_filepath = ""
    epub_name = filename + "_" + tagname + ".epub"
    taglst = []
    if os.path.exists(repo_path):
        os.chdir(repo_path)
        tags = porcelain.list_tags(repo='.')
        os.chdir(cdir)
        for atag in tags:
            taglst.append(unicode_str(atag))
        if tagname not in taglst:
            return epub_file_path

        # FIXME: there should **never** be unstaged changes or untracked files
        # in the repo because of how Sigil handles it, but we really should use
        # dulwich status to verify that before proceeding and abort otherwise.
        # Just in case the user uses command line git to manipulate the repo 
        # outside of Sigil's control leaving it in a dirty state

        # Instead of cloning an entire repo just to do a checkout
        # of a tag, do all work in the current repo
        checkout_tag(repo_path, tagname)

        # working directory of the repo should now be populated
        epub_filepath = os.path.join(dest_path, epub_name)
        try:
            build_epub_from_folder_contents(repo_path, epub_filepath)
        except Exception as e:
            print("epub creation failed")
            print(str(e))
            epub_filepath = ""
            pass
        # **always** restore the repo working directory HEAD before leaving
        checkout_head(repo_path)
    return epub_filepath


def get_tag_list(localRepo, bookid):
    repo_home = pathof(localRepo)
    repo_home = repo_home.replace("/", os.sep)
    repo_path = os.path.join(repo_home, "epub_" + bookid)
    cdir = os.getcwd()
    taglst = []
    if os.path.exists(repo_path):
        os.chdir(repo_path)
        with open_repo_closing(".") as r:
            tags = sorted(r.refs.as_dict(b"refs/tags"))
            for atag in tags:
                tagkey = b"refs/tags/" + atag
                obj = r[tagkey]
                tag_name = unicode_str(atag)
                tag_message = ""
                tag_date = ""
                if isinstance(obj,Tag):
                    time_tuple = time.gmtime(obj.tag_time + obj.tag_timezone)
                    time_str = time.strftime("%a %b %d %Y %H:%M:%S",time_tuple)
                    timezone_str = format_timezone(obj.tag_timezone).decode('ascii')
                    tag_date = time_str + " " + timezone_str
                    tag_message = unicode_str(obj.message)
                if isinstance(obj, Commit):
                    time_tuple = time.gmtime(obj.author_time + obj.author_timezone)
                    time_str = time.strftime("%a %b %d %Y %H:%M:%S",time_tuple)
                    timezone_str = format_timezone(obj.author_timezone).decode('ascii')
                    tag_date = time_str + " " + timezone_str
                    tag_message = unicode_str(obj.message)
                taglst.append(tag_name + "|" + tag_date + "|" + tag_message)
        os.chdir(cdir)
    return taglst

def performCommit(localRepo, bookid, bookinfo, bookroot, bookfiles):
    has_error = False
    staged = []
    added=[]
    ignored=[]
    # convert url paths to os specific paths
    repo_home = pathof(localRepo)
    repo_home = repo_home.replace("/", os.sep)
    repo_path = os.path.join(repo_home, "epub_" + bookid)
    book_home = pathof(bookroot)
    book_home = book_home.replace("/", os.sep);
    # convert from bookpaths to os relative file paths
    filepaths = []
    for bkpath in bookfiles:
        afile = pathof(bkpath)
        afile = afile.replace("/", os.sep)
        filepaths.append(afile)

    cdir = os.getcwd()
    if os.path.exists(repo_path):
        # handle updating the staged files and commiting and tagging
        # first collect info to determine files to delete form repo
        # current tag, etc
        os.chdir(repo_path)
        # determine the new tag
        tags = porcelain.list_tags(repo='.')
        tagname = "V%04d" % (len(tags) + 1)
        tagmessage = "Tag: " + tagname
        message = "updating to " + tagname
        # extra parameters must be passed as bytes if annotated is true
        tagname = utf8_str(tagname)
        message = utf8_str(message)
        tagmessage = utf8_str(tagmessage)
        # delete files that are no longer needed from staging area
        tracked = []
        tracked = porcelain.ls_files(repo='.')
        files_to_delete = []
        for afile in tracked:
            afile = pathof(afile)
            if afile not in filepaths:
                if afile not in  ["mimetype", ".gitignore", ".bookinfo"]:
                    files_to_delete.append(afile)
        if len(files_to_delete) > 0:
            porcelain.rm(repo='.',paths=files_to_delete)
        # copy over current files
        copy_book_contents_to_destination(book_home, filepaths, repo_path)
        (staged, unstaged, untracked) = porcelain.status(repo='.')
        files_to_update = []
        for afile in unstaged:
            afile = pathof(afile)
            files_to_update.append(afile)
        for afile in untracked:
            afile = pathof(afile)
            files_to_update.append(afile)
        (added, ignored) = porcelain.add(repo='.', paths=files_to_update)
        commit_sha1 = porcelain.commit(repo='.',message=message, author=_SIGIL, committer=_SIGIL)
        # create annotated tags so we can get a date history
        tag = porcelain.tag_create(repo='.', tag=tagname, message=tagmessage, annotated=True, author=_SIGIL)
        os.chdir(cdir)
        add_bookinfo(repo_path, bookinfo, bookid, unicode_str(tagname))
    else:
        # this will be an initial commit to this repo
        tagname = b"V0001"
        tagmessage = b'First Tag'
        message = b"Initial Commit"
        os.makedirs(repo_path)
        add_gitignore(repo_path)
        add_gitattributes(repo_path)
        cdir = os.getcwd()
        os.chdir(repo_path)
        r = porcelain.init(path='.', bare=False)
        # set local git config to no convert crlf since always a non-shared local repo
        c = r.get_config()
        c.set("core","autocrlf","false")
        c.write_to_path()
        staged = copy_book_contents_to_destination(book_home, filepaths, repo_path)
        (added, ignored) = porcelain.add(repo='.',paths=staged)
        # it seems author, committer, messages, and tagname only work with bytes if annotated=True
        commit_sha1 = porcelain.commit(repo='.',message=message, author=_SIGIL, committer=_SIGIL)
        tag = porcelain.tag_create(repo='.', tag=tagname, message=tagmessage, annotated=True, author=_SIGIL)
        os.chdir(cdir)
        add_bookinfo(repo_path, bookinfo, bookid, unicode_str(tagname))
    result = "\n".join(added);
    result = result + "***********" + "\n".join(ignored)
    if not has_error:
        return result;
    return ''


def eraseRepo(localRepo, bookid):
    repo_home = pathof(localRepo)
    repo_home = repo_home.replace("/", os.sep)
    repo_path = os.path.join(repo_home, "epub_" + bookid)
    success = 1
    cdir = os.getcwd()
    if os.path.exists(repo_path):
        try:
            shutil.rmtree(repo_path)
        except Exception as e:
            print("repo erasure failed")
            print(str(e))
            success = 0
            pass
    return success


def generate_diff_from_checkpoints(localRepo, bookid, leftchkpoint, rightchkpoint):
    repo_home = pathof(localRepo)
    repo_home = repo_home.replace("/", os.sep)
    repo_path = os.path.join(repo_home, "epub_" + bookid)
    success = True
    if os.path.exists(repo_path):
        os.chdir(repo_path)
        with open_repo_closing(".") as r:
            tags = r.refs.as_dict(b"refs/tags")
            commit1 = r[r[tags[utf8_str(leftchkpoint)]].object[1]]
            commit2 = r[r[tags[utf8_str(rightchkpoint)]].object[1]]
            output = io.BytesIO()
            try:
                write_tree_diff(output, r.object_store, commit1.tree, commit2.tree)
            except Exception as e:
                print("diff failed in python")
                print(str(e))
                success = False
                pass
        if success:
            return output.getvalue()
        return ''


def generate_log_summary(localRepo, bookid):
    repo_home = pathof(localRepo)
    repo_home = repo_home.replace("/", os.sep)
    repo_path = os.path.join(repo_home, "epub_" + bookid)
    results = ""
    cdir = os.getcwd()
    if os.path.exists(repo_path):
        os.chdir(repo_path)
        with StringIO() as sf:
            logsummary(repo=".", outstream=sf, stats=True)
            sf.seek(0)
            results = sf.getvalue()
        os.chdir(cdir)
    return results


def generate_parsed_ndiff(path1, path2):
    path1 = pathof(path1)
    path2 = pathof(path2)
    try:
        leftFileContents = open(path1,'rb').read().decode('utf-8')
    except:
        leftFileContents = ''
    try:
        rightFileContents = open(path2, 'rb').read().decode('utf-8')
    except:
        rightFileContents = ''
    diff = DifflibParser(leftFileContents.splitlines(), rightFileContents.splitlines())
    results = []
    for dinfo in diff:
        results.append(dinfo)
    return results


def generate_unified_diff(path1, path2):
    path1 = pathof(path1)
    path2 = pathof(path2)
    try:
        leftContents = open(path1,'rb').read().decode('utf-8')
    except:
        leftContents = ''
    try:
        rightContents = open(path2, 'rb').read().decode('utf-8')
    except:
        rightContents = ''

    diffs = difflib.unified_diff(leftContents.splitlines(keepends=True), 
                                 rightContents.splitlines(keepends=True),
                                 fromfile=path1, tofile=path2, n=3)
    results = "diff a/%s b/%s\n" % (path1, path2)
    with StringIO() as sf:
        for a in diffs:
            sf.write(a)
        sf.seek(0)
        results += sf.getvalue()
    return results
    

def copy_tag_to_destdir(localRepo, bookid, tagname, destdir):
    # convert posix paths to os specific paths
    repo_home = pathof(localRepo)
    repo_home = repo_home.replace("/", os.sep)
    repo_path = os.path.join(repo_home, "epub_" + bookid)
    dest_path = pathof(destdir).replace("/", os.sep)
    copied = []
    if tagname != "HEAD":
        # checkout the proper base tag in the repo if needed
        checkout_tag(repo_path, tagname)
    # walk the list of files and copy them
    repolist = walk_folder(repo_path)
    for apath in repolist:
        src = os.path.join(repo_path, apath)
        dest = os.path.join(dest_path, apath)
        # and make sure destination directory exists
        base = os.path.dirname(dest)
        if not os.path.exists(base):
            os.makedirs(base)
        data = b''
        with open(src, 'rb') as f:
            data = f.read()
        with open(dest,'wb') as fp:
            fp.write(data)
        copied.append(apath)
    # return the repo to its normal state if needed
    if tagname != "HEAD":
        checkout_head(repo_path)
    return "\n".join(copied)


def get_current_status_vs_destdir(bookroot, bookfiles, destdir):
    # convert posix paths to os specific paths
    book_home = pathof(bookroot).replace("/", os.sep);
    dest_path = pathof(destdir).replace("/", os.sep);
    # convert from bookpaths to os relative file paths
    filepaths = []
    for bkpath in bookfiles:
        afile = pathof(bkpath)
        afile = afile.replace("/", os.sep)
        filepaths.append(afile)
    if "mimetype" in filepaths:
        filepaths.remove("mimetype")
    repolist = walk_folder(dest_path)
    # determine what has been deleted
    deleted = []
    for fpath in repolist:
        if fpath not in filepaths:
            deleted.append(fpath)
    if "mimetype" in deleted:
        deleted.remove("mimetype")
    # now use pythons built in filecmp to determine added and modified
    (unchanged, modified, added) = filecmp.cmpfiles(dest_path, book_home, filepaths, shallow=False)
    # convert everything back to posix style bookpaths
    for i in range(len(deleted)):
        deleted[i] = deleted[i].replace(os.sep,"/")
    for i in range(len(added)):
        added[i] = added[i].replace(os.sep,"/")
    for i in range(len(modified)):
        modified[i] = modified[i].replace(os.sep,"/")
    return (deleted, added, modified)


def main():
    argv = sys.argv
    return 0

if __name__ == '__main__':
    sys.exit(main())
