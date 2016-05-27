#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vim:ts=4:sw=4:softtabstop=4:smarttab:expandtab

from __future__ import unicode_literals, division, absolute_import, print_function

xml_structure = {
    # [ (set of allowed parents), isVoid, min_count, max_count, (set of required predecessors) ]
    'package'        : [ (None),        False,   1,   1,  (None)       ],
    'metadata'       : [ ('package'),   False,   1,   1,  (None)       ],
    'dc:identifier'  : [ ('metadata'),  False,   1,  -1,  (None)       ],
    'dc:language'    : [ ('metadata'),  False,   1,  -1,  (None)       ],
    'dc:title'       : [ ('metadata'),  False,   1,   1,  (None)       ],
    'dc:creator'     : [ ('metadata'),  False,   0,  -1,  (None)       ],
    'dc:contributor' : [ ('metadata'),  False,   0,  -1,  (None)       ],
    'dc:subject'     : [ ('metadata'),  False,   0,  -1,  (None)       ],
    'dc:description' : [ ('metadata'),  False,   0,  -1,  (None)       ],
    'dc:publisher'   : [ ('metadata'),  False,   0,  -1,  (None)       ],
    'dc:date'        : [ ('metadata'),  False,   0,  -1,  (None)       ],
    'dc:type'        : [ ('metadata'),  False,   0,  -1,  (None)       ],
    'dc:format'      : [ ('metadata'),  False,   0,  -1,  (None)       ],
    'dc:source'      : [ ('metadata'),  False,   0,  -1,  (None)       ],
    'dc:relation'    : [ ('metadata'),  False,   0,  -1,  (None)       ],
    'dc:coverage'    : [ ('metadata'),  False,   0,  -1,  (None)       ],
    'meta'           : [ ('metadata'),  False,   0,  -1,  (None)       ],
    'manifest'       : [ ('package'),   False,   1,   1,  ['metadata'] ],
    'item'           : [ ('manifest'),  True,    1,  -1,  (None)       ],
    'spine'          : [ ('package'),   False,   1,   1,  ['manifest'] ],
    'itemref'        : [ ('spine'),     True,    1,  -1,  (None)       ],
    'guide'          : [ ('package'),   False,   0,   1,  ['spine']    ],
    'reference'      : [ ('guide'),     True,    0,  -1,  (None)       ],
    'link'           : [ ('metadata'),  True,    0,  -1,  (None)       ],
    'collections'    : [ ('package', 'collections'),   False,   0,  -1,  ['bindings']       ],
    'bindings'       : [ ('package'),   False,   0,   1,  ['guide']    ],
    'mediaType'      : [ ('bindings'),  True,    0,  -1,  (None)       ],
}

min_required_attribs = []
