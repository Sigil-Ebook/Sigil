#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# vim:ts=4:sw=4:softtabstop=4:smarttab:expandtab

# Copyright (c) 2015-2020 Kevin B. Hendricks, and Doug Massay
# Copyright (c) 2014      Kevin B. Hendricks, John Schember, and Doug Massay
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this list of
# conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice, this list
# of conditions and the following disclaimer in the documentation and/or other materials
# provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
# SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
# TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
# WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

from quickparser import QuickXHTMLParser
from pluginhunspell import HunspellChecker
from preferences import JSONPrefs

class ContainerException(Exception):
    pass

class InputContainer(object):

    def __init__(self, wrapper, debug=False):
        self._debug = debug
        self._w = wrapper
        self.qp = QuickXHTMLParser()
        self.hspell = HunspellChecker(wrapper.get_hunspell_path())
        self.dictionary_dirs = wrapper.get_dictionary_dirs()
        self._prefs_store = JSONPrefs(wrapper.plugin_dir, wrapper.plugin_name)

    def getPrefs(self):
        return self._prefs_store

    def savePrefs(self, user_copy):
        self._prefs_store = user_copy
        self._prefs_store._commit()

    def launcher_version(self):
        return self._w.getversion()

    @property
    def sigil_ui_lang(self):
        if self._w.sigil_ui_lang is None:
            return 'en'
        return self._w.sigil_ui_lang

    @property
    def sigil_spellcheck_lang(self):
        if self._w.sigil_spellcheck_lang is None:
            return 'en_US'
        return self._w.sigil_spellcheck_lang

    def addotherfile(self, book_href, data):
        # creates a new file not in manifest with desired ebook root relative href
        self._w.addotherfile(book_href, data)

    # get path to hunspell dll / library
    def get_hunspell_library_path(self):
        return self._w.get_hunspell_path()

    # get a list of the directories that contain Sigil's hunspell dictionaries
    def get_dictionary_dirs(self):
        return self._w.get_dictionary_dirs()

    
    # New in Sigil 1.1
    # ----------------

    # returns "light" or "dark"
    def colorMode(self):
        return self._w.colorMode()

    # returns color as css or javascript hex color string "#xxxxxx"
    # acccepts the following color roles in a case insensitive manner:
    #    "Window", "Base", "Text", "Highlight", "HighlightedText"
    def color(self, role):
        return self._w.color(role)
