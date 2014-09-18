#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vim:ts=4:sw=4:softtabstop=4:smarttab:expandtab                                                                               

from __future__ import unicode_literals, division, absolute_import, print_function

import sys
import os
from quickparser import QuickXHTMLParser

class ContainerException(Exception):
    pass

class InputContainer(object):

    def __init__(self, wrapper,  debug = False):
        self._debug = debug
        self._w = wrapper
        self.qp=QuickXHTMLParser()

    def addotherfile(self, book_href, data):
        # creates a new file not in manifest with desired ebook root relative href
        self._w.addotherfile(book_href, data)
        
