#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# vim:ts=4:sw=4:softtabstop=4:smarttab:expandtab

# Copyright (c) 2014-2020 Kevin B. Hendricks and Doug Massay
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

import sys
from ctypes import c_char_p, c_int, POINTER, pointer, cdll
from ctypes.util import find_library

import codecs

class HunspellChecker(object):

    def __init__(self, hunspell_dllpath):
        self.hunhandle = None
        self.encoder = None
        self.decoder = None
        a = c_char_p()
        p = pointer(a)
        self.pp = pointer(p)
        self.retval = 0
        # ctypes interface to hunspell spellchecker
        self.hunspell = None
        sys_hunspell_location = None
        try:
            # First use bundled hunspell location.
            self.hunspell = cdll[hunspell_dllpath]
        except OSError:
            # No bundled (or incompatible bundled) libhunspell found.
            # found. So search for system libhunspell.
            self.hunspell = None
            sys_hunspell_location = find_library('hunspell')
        if sys_hunspell_location is not None:
            try:
                self.hunspell = cdll.LoadLibrary(sys_hunspell_location)
            except OSError:
                # If the system libhunspell can't be found/loaded, then
                # then punt, so plugins that don't utilize libhunspell
                # can still function without error.
                self.hunspell = None
        if self.hunspell is None:
            return
        self.hunspell.Hunspell_create.restype = POINTER(c_int)
        self.hunspell.Hunspell_create.argtypes = (c_char_p, c_char_p)
        self.hunspell.Hunspell_destroy.argtype = POINTER(c_int)
        self.hunspell.Hunspell_get_dic_encoding.restype = c_char_p
        self.hunspell.Hunspell_get_dic_encoding.argtype = POINTER(c_int)
        self.hunspell.Hunspell_spell.argtypes = (POINTER(c_int), c_char_p)
        self.hunspell.Hunspell_suggest.argtypes = (POINTER(c_int), POINTER(POINTER(c_char_p)), c_char_p)
        self.hunspell.Hunspell_free_list.argtypes = (POINTER(c_int), POINTER(POINTER(c_char_p)), c_int)


    def loadDictionary(self, affpath, dpath):
        if sys.platform.startswith('win'):
            # Win32 bug needs a longpath prefix to trick hunspell into always using _wfopen instead of fopen.
            # Only matters if library paths contain non-ascii characters.
            affpath = '\\\\?\\' + affpath
            dpath = '\\\\?\\' + dpath
        if type(affpath) == str:
            affpath = affpath.encode('utf-8')
        if type(dpath) == str:
            dpath = dpath.encode('utf-8')
        if self.hunhandle is not None:
            self.cleanUp()
        self.hunhandle = self.hunspell.Hunspell_create(affpath, dpath)
        encdic = self.hunspell.Hunspell_get_dic_encoding(self.hunhandle)
        if type(encdic) == bytes:
            encdic = encdic.decode('utf-8')
        try:
            self.encoder = codecs.getencoder(encdic)
            self.decoder = codecs.getdecoder(encdic)
        except codecs.LookupError:
            self.encoder = codecs.getencoder('utf-8')
            self.decoder = codecs.getdecoder('utf-8')

    def cleanUp(self):
        if self.hunhandle is not None:
            self.hunspell.Hunspell_destroy(self.hunhandle)
        self.hunhandle = None

    def encodeit(self, word):
        encoded_word, encoded_len = self.encoder(word)
        return encoded_word

    def decodeit(self, word):
        decoded_word, encoded_len = self.decoder(word)
        return decoded_word

    def check(self, word):
        if type(word) == bytes:
            word = word.decode('utf-8')
        return self.hunspell.Hunspell_spell(self.hunhandle, self.encodeit(word))

    def suggest(self, word):
        if type(word) == bytes:
            word = word.decode('utf-8')
        self.retval = self.hunspell.Hunspell_suggest(self.hunhandle, self.pp, self.encodeit(word))
        p = self.pp.contents
        suggestions = []
        for i in range(0, self.retval):
            suggestions.append(self.decodeit(c_char_p(p[i]).value))
        self.hunspell.Hunspell_free_list(self.hunhandle, self.pp, self.retval)
        return suggestions


def main():
    checklst = ["hello", "goodbye", "don't", "junkj", "misteak"]

    hunspell_dllpath = "/Users/kbhend/repo/build/bin/Sigil.app/Contents/lib/libhunspell.dylib"
    hsp = HunspellChecker(hunspell_dllpath)

    if hsp is None:
        print("Error could not load hunspell shared library", hunspell_dllpath)
        return 1

    affpath = "/Users/kbhend/repo/build/bin/Sigil.app/Contents/hunspell_dictionaries/en_US.aff"
    dpath = "/Users/kbhend/repo/build/bin/Sigil.app/Contents/hunspell_dictionaries/en_US.dic"

    hsp.loadDictionary(affpath, dpath)

    for word in checklst:
        res = hsp.check(word)
        if res != 1:
            print(word, "incorrect", hsp.suggest(word))
        else:
            print(word, "correct")

    hsp.cleanUp()

    return 0


if __name__ == '__main__':
    sys.exit(main())
