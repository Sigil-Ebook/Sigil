#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vim:ts=4:sw=4:softtabstop=4:smarttab:expandtab

import sys
import os

def relativePath(to_bkpath, start_dir):
    # remove any trailing path separators from both paths
    dsegs = to_bkpath.rstrip('/').split('/')
    ssegs = start_dir.rstrip('/').split('/')
    if dsegs == ['']: dsegs=[]
    if ssegs == ['']: ssegs=[]
    res = []
    i = 0
    for s1, s2 in zip(dsegs, ssegs):
        if s1 != s2: break
        i+=1
    for p in range(i, len(ssegs),1): res.append('..')
    for p in range(i, len(dsegs),1): res.append(dsegs[p])
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
        return dest_relpath;
    bookpath = start_folder.rstrip('/') + '/' + dest_relpath
    return resolveRelativeSegmentsInFilePath(bookpath)


def startingDir(file_path):
    ssegs = file_path.split('/')
    ssegs.pop()
    return '/'.join(ssegs)



def main():
    argv = sys.argv
    p1 = 'This/is/the/../../end.txt'
    print('Testing resolveRelativeSegmentsInFilePath(file_path)')
    print('    file_path: ', p1)
    print(resolveRelativeSegmentsInFilePath(p1))
    print('    ')

    p1 = 'hello.txt'
    p2 = 'goodbye.txt'
    print('Testing buildRelativePath(from_bkpath,to_bkpath')
    print('    from_bkpath: ',p1)
    print('    to_bkpath:   ',p2)
    print(buildRelativePath(p1, p2))
    print('    ')

    p1 = 'OEBPS/Text/book1/chapter1.xhtml'
    p2 = 'OEBPS/Text/book2/chapter1.xhtml'
    print('Testing buildRelativePath(from_bkpath,to_bkpath)')
    print('    from_bkpath: ',p1)
    print('    to_bkpath:   ',p2)
    print(buildRelativePath(p1, p2))
    print('    ')
    
    p1 = 'OEBPS/package.opf'
    p2 = 'OEBPS/Text/book1/chapter1.xhtml'
    print('Testing buildRelativePath(from_bkpath, to_bkpath)')
    print('    from_bkpath: ',p1)
    print('    to_bkpath:   ',p2)
    print(buildRelativePath(p1,p2))
    print('    ')

    p1 = '../../Images/image.png'
    p2 = 'OEBPS/Text/book1/'
    print('Testing buildBookPath(destination_href, start_dir)')
    print('    destination_href: ',p1)
    print('    starting_dir:     ',p2)
    print(buildBookPath(p1, p2))
    print('    ')

    p1 = 'image.png'
    p2 = ''
    print('Testing buildBookPath(destination_href, start_dir)')
    print('    destination_href: ',p1)
    print('    starting_dir:     ',p2)
    print(buildBookPath(p1, p2))
    print('    ')

    p1 = 'content.opf'
    print('Testing startingDir(bookpath')
    print('    bookpath: ',p1)
    print('"'+ startingDir(p1)+'"')
    print('    ')
    return 0

if __name__ == '__main__':
    sys.exit(main())
