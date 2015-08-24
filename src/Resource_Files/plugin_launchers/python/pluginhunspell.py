#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vim:ts=4:sw=4:softtabstop=4:smarttab:expandtab

from __future__ import unicode_literals, division, absolute_import, print_function

import sys
import os
from ctypes import *
import codecs

PY3 = sys.version_info[0] == 3
if PY3:
    binary_type = bytes
    text_type = str
else:
    binary_type = str
    text_type = unicode
    range = xrange

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
        try:
            self.hunspell = cdll[hunspell_dllpath]
        except OSError:
            return None
        self.hunspell.Hunspell_create.restype = POINTER(c_int)
        self.hunspell.Hunspell_create.argtypes = (c_char_p, c_char_p)
        self.hunspell.Hunspell_destroy.argtype = POINTER(c_int)
        self.hunspell.Hunspell_get_dic_encoding.restype = c_char_p
        self.hunspell.Hunspell_get_dic_encoding.argtype = POINTER(c_int)
        self.hunspell.Hunspell_spell.argtypes = (POINTER(c_int), c_char_p)
        self.hunspell.Hunspell_suggest.argtypes = (POINTER(c_int), POINTER(POINTER(c_char_p)), c_char_p)
        self.hunspell.Hunspell_free_list.argtypes = (POINTER(c_int), POINTER(POINTER(c_char_p)), c_int)


    def loadDictionary(self, affpath, dpath):
        if type(affpath) == text_type:
            affpath = affpath.encode('utf-8')
        if type(dpath) == text_type:
            dpath = dpath.encode('utf-8')
        if self.hunhandle is not None:
            self.cleanUp()
        self.hunhandle = self.hunspell.Hunspell_create(affpath, dpath)
        encdic = self.hunspell.Hunspell_get_dic_encoding(self.hunhandle)
        if type(encdic) == binary_type:
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
        if type(word) == binary_type:
            word = word.decode('utf-8')
        return self.hunspell.Hunspell_spell(self.hunhandle, self.encodeit(word))

    def suggest(self,word):
        if type(word) == binary_type:
            word = word.decode('utf-8')
        self.retval = self.hunspell.Hunspell_suggest(self.hunhandle, self.pp, self.encodeit(word))
        p = self.pp.contents
        suggestions = []
        for i in range(0, self.retval):
             suggestions.append( self.decodeit( c_char_p(p[i]).value ))
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

