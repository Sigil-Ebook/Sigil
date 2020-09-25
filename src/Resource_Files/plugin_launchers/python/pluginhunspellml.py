#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# vim:ts=4:sw=4:softtabstop=4:smarttab:expandtab

# Copyright (c) 2020 Kevin B. Hendricks and Doug Massay
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
from uuid import uuid4
from ctypes import c_char_p, c_int, POINTER, pointer, cdll
from ctypes.util import find_library

import codecs

class HunspellDictionary(object):

    def __init__(self, handle, encoder, decoder, wc):
        self.handle = handle
        self.encoder = encoder
        self.decoder = decoder
        self.wordchars = wc
    
    def encodeit(self, word):
        encoded_word, alen = self.encoder(word)
        return encoded_word

    def decodeit(self, word):
        decoded_word, alen = self.decoder(word)
        return decoded_word

    def get_wordchars(self):
        return self.wordchars
    
    
class HunspellMLChecker(object):

    def __init__(self, hunspell_dllpath):
        # ctypes interface to hunspell spellchecker
        self.dicts = {}
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

        # WORKAROUND for missing hunspell interface to support
        # external tokenization of words
        #   manually parse aff file to get wordchars since no
        #   c hunspell library call supports it and they are needed
        #   for proper tokenization of text
        wcbytes = b''
        try:
            with open(affpath, 'rb') as f:
                affdata = f.read()
            for aline in affdata.split(b'\n'):
                if aline.startswith(b'WORDCHARS '):
                    wcbytes = aline[10:]
        except:
            pass
        
        if sys.platform.startswith('win'):
            # Win32 bug needs a longpath prefix to trick hunspell into always using _wfopen instead of fopen.
            # Only matters if library paths contain non-ascii characters.
            affpath = '\\\\?\\' + affpath
            dpath = '\\\\?\\' + dpath
        if type(affpath) == str:
            affpath = affpath.encode('utf-8')
        if type(dpath) == str:
            dpath = dpath.encode('utf-8')
        
        dickey = None
        dichandle = self.hunspell.Hunspell_create(affpath, dpath)
        if dichandle:
            dickey = uuid4()
            encdic = self.hunspell.Hunspell_get_dic_encoding(dichandle)
            if type(encdic) == bytes:
                encdic = encdic.decode('utf-8')
            try:
                encoder = codecs.getencoder(encdic)
                decoder = codecs.getdecoder(encdic)
            except codecs.LookupError:
                encoder = codecs.getencoder('utf-8')
                decoder = codecs.getdecoder('utf-8')

            wordchars, alen = decoder(wcbytes)
            self.dicts[dickey] = HunspellDictionary(dichandle, encoder, decoder, wordchars)
        return dickey

    def get_wordchars(self, dickey):
        if dickey in self.dicts:
            adic = self.dicts[dickey]
            return adic.get_wordchars()
        return ''

    def cleanUpAll(self):
        for dickey in self.dicts.keys():
            adic = self.dicts[dickey]
            self.hunspell.Hunspell_destroy(adic.handle)
            del self.dicts[dickey]

    def cleanUp(self, dickey):
        if dickey is not None and dickey in self.dicts:
            adic = self.dicts[dickey]
            self.hunspell.Hunspell_destroy(adic.handle)
            del self.dicts[dickey]

    def check(self, dickey, word):
        if type(word) == bytes:
            word = word.decode('utf-8')
        if dickey in self.dicts:
            adic = self.dicts[dickey]
            return self.hunspell.Hunspell_spell(adic.handle, adic.encodeit(word))
        return 0

    def suggest(self, dickey, word):
        if type(word) == bytes:
            word = word.decode('utf-8')
        suggestions = []
        a = c_char_p()
        p = pointer(a)
        pp = pointer(p)
        if dickey in self.dicts:
            adic = self.dicts[dickey]
            retval = self.hunspell.Hunspell_suggest(adic.handle, pp, adic.encodeit(word))
            p = pp.contents
            for i in range(0, retval):
                suggestions.append(adic.decodeit(c_char_p(p[i]).value))
            self.hunspell.Hunspell_free_list(adic.handle, pp, retval)
        return suggestions


    
def main():
    checklst = ["hello", "goodbye.", "don't", "junkj", "misteak"]

    hunspell_dllpath = "/Volumes/SSD-Drive/repo/build9/bin/Sigil.app/Contents/lib/libhunspell.dylib"
    hsp = HunspellMLChecker(hunspell_dllpath)

    if hsp is None:
        print("Error could not load hunspell shared library", hunspell_dllpath)
        return 1

    affpath = "/Volumes/SSD-Drive/repo/build9/bin/Sigil.app/Contents/hunspell_dictionaries/en_US.aff"
    dpath = "/Volumes/SSD-Drive/repo/build9/bin/Sigil.app/Contents/hunspell_dictionaries/en_US.dic"

    dickey = hsp.loadDictionary(affpath, dpath)

    if dickey:
        print(hsp.get_wordchars(dickey))
    
        for word in checklst:
            res = hsp.check(dickey, word)
            if res != 1:
                print(word, "incorrect", hsp.suggest(dickey,word))
            else:
                print(word, "correct")

        # or hsp.cleanUpAll()
        hsp.cleanUp(dickey)

    return 0


if __name__ == '__main__':
    sys.exit(main())
