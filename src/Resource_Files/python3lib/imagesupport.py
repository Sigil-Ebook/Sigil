#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# vim:ts=4:sw=4:softtabstop=4:smarttab:expandtab

# Copyright (c) 2026 Kevin B. Hendricks, Stratford Ontario Canada
# Copyright (c) 2026 Doug Massay
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

_has_pil = False
try:
    from PIL import Image
    _has_pil = True
except ImportError:
    _has_pil = False


def convert_png_to_static_gif(pngpath, gifpath):
    if not _has_pil:
        print("import of PIL failed: PIL is now a required Python module for Sigil")
        return 0
    try:
        with Image.open(pngpath) as img:
            img.save(gifpath, "GIF")
            return 1
    except FileNotFoundError:
        print("File not found: ", pngpath)
        return 0
    except Exception as e:
        print("An error has occurred: ", str(e))
        return 0
    return 0

def main():
    argv = sys.argv
    return 0

if __name__ == '__main__':
    sys.exit(main())
