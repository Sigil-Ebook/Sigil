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

# This diffstat code is a modified version of code extracted from Mercurial patch.py
# which has the following license:
#
# Copyright 2006 Brendan Cully <brendan@kublai.com>
# Copyright 2007 Chris Mason <chris.mason@oracle.com>
#
# This software may be used and distributed according to the terms of the
# GNU General Public License version 2 or any later version.
#

import os
import sys
import re

_diffre = re.compile(br'^diff .*-r [a-z0-9]+\s(.*)$')
_gitre = re.compile(br'diff --git a/(.*) b/(.*)')

def diffstatsum(stats):
    maxfile, maxtotal, addtotal, removetotal, binary = 0, 0, 0, 0, False
    for f, a, r, b in stats:
        maxfile = max(maxfile, len(f))
        maxtotal = max(maxtotal, a + r)
        addtotal += a
        removetotal += r
        binary = binary or b
    return maxfile, maxtotal, addtotal, removetotal, binary

def diffstatdata(lines):
    results = []
    filename, adds, removes, isbinary = None, 0, 0, False

    def addresult():
        if filename:
            results.append((filename, adds, removes, isbinary))

    # inheader is used to track if a line is in the
    # header portion of the diff.  This helps properly account
    # for lines that start with '--' or '++'
    inheader = False
    for line in lines:
        if line.startswith(b'diff'):
            addresult()
            # starting a new file diff
            # set numbers to 0 and reset inheader
            inheader = True
            adds, removes, isbinary = 0, 0, False
            if line.startswith(b'diff --git a/'):
                filename = _gitre.search(line).group(2)
            elif line.startswith(b'diff -r'):
                # format: "diff -r ... -r ... filename"
                filename = _diffre.search(line).group(1)
        elif line.startswith(b'@@'):
            inheader = False
        elif line.startswith(b'+') and not inheader:
            adds += 1
        elif line.startswith(b'-') and not inheader:
            removes += 1
        elif line.startswith(b'GIT binary patch') or line.startswith(b'Binary file'):
            isbinary = True
        elif line.startswith(b'rename from'):
            filename = line[12:]
        elif line.startswith(b'rename to'):
            filename += b' => %s' % line[10:]
    addresult()
    return results

def diffstat(lines, width=80):
    output = []
    stats = diffstatdata(lines)
    maxname, maxtotal, totaladds, totalremoves, hasbinary = diffstatsum(stats)
    countwidth = len(str(maxtotal))
    if hasbinary and countwidth < 3:
        countwidth = 3
    graphwidth = width - countwidth - maxname - 6
    if graphwidth < 10:
        graphwidth = 10

    def scale(i):
        if maxtotal <= graphwidth:
            return i
        return max(i * graphwidth // maxtotal, int(bool(i)))

    for filename, adds, removes, isbinary in stats:
        if isbinary:
            count = b'Bin'
        else:
            count = b'%d' % (adds + removes)
        pluses = b'+' * scale(adds)
        minuses = b'-' * scale(removes)
        output.append(b' %s%s |  %*s %s%s\n' 
        	% (filename, b' ' * (maxname - len(filename)), 
        	countwidth, count, pluses, minuses)
        )
    if stats:
        output.append(
            (b' %d files changed, %d insertions(+), %d deletions(-)\n')
            % (len(stats), totaladds, totalremoves)
        )
    return b''.join(output)


def main():
    argv = sys.argv
    diffpath = argv[1]
    data=b""
    with open(diffpath, 'rb') as f:
        data = f.read()
    lines = data.split(b"\n")
    result = diffstat(lines)
    print(result.decode("utf-8"))
    return 0

if __name__ == '__main__':
    sys.exit(main())
