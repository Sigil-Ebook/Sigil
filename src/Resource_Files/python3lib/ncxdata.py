#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vim:ts=4:sw=4:softtabstop=4:smarttab:expandtab

from __future__ import unicode_literals, division, absolute_import, print_function

xml_structure = {
    # [ (set of allowed parents), isVoid, min_count, max_count, (set of required predecessors) ]
    'navLabel'       : [ ('navPoint', 'pageTarget', 'navList', 'navTarget'),  False,   0,  -1,  (None) ],
    'text'           : [ ('docTitle', 'docAuthor', 'navLabel'),               False,   1,  -1,  (None) ],
    'content'        : [ ('navPoint', 'pageTarget', 'navTarget'),             True,    1,  -1,  (None) ],
    'navPoint'       : [ ('navMap', 'navPoint'),                              False,   1,  -1,  (None) ],
    'ncx'            : [ (None),        False,   1,   1,  (None)       ],
    'head'           : [ ('ncx'),       False,   1,   1,  (None)       ],
    'meta'           : [ ('head'),      True,    0,  -1,  (None)       ],
    'docTitle'       : [ ('ncx'),       False,   0,   1,  (None)       ],
    'docAuthor'      : [ ('ncx'),       False,   0,   1,  (None)       ],
    'navMap'         : [ ('ncx'),       False,   1,   1,  (None)       ],
    'pageList'       : [ ('ncx'),       False,   0,   1,  (None)       ],
    'navList'        : [ ('ncx'),       False,   0,   1,  (None)       ],
    'pageTarget'     : [ ('pageList'),  False,   0,  -1,  (None)       ],
    'navTarget'      : [ ('navList'),   False,   0,  -1,  (None)       ],
}

min_required_attribs = []
