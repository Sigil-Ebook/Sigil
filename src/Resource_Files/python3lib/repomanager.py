#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# vim:ts=4:sw=4:softtabstop=4:smarttab:expandtab

import sys
import os
import dulwich
from dulwich import porcelain

# the entry points
def performCommit(localRepo, bookid, booktitle, bookroot, bookfiles):
    has_error = False
    # simply echo parameters back for now
    parms = []
    parms.append(localRepo)
    parms.append("\n")
    parms.append(bookid)
    parms.append("\n")
    parms.append(booktitle)
    parms.append("\n")
    parms.append(bookroot)
    parms.append("\n")
    for bp in bookfiles:
        parms.append(bp)
        parms.append("\n")
    result = ''.join(parms);
    if not has_error:
        return result;
    return ''

def main():
    argv = sys.argv
    return 0

if __name__ == '__main__':
    sys.exit(main())
