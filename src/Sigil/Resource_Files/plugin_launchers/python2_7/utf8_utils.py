#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys, os
import locale
import codecs

iswindows = sys.platform.startswith('win')

# force string to be utf-8 encoded whether unicode or bytestring
def utf8_str(p, enc='utf-8'):
    if p is None:
        return None
    if isinstance(p, unicode):
        return p.encode('utf-8')
    if enc != 'utf-8':
        return p.decode(enc).encode('utf-8')
    return p


# get sys.argv arguments and encode them into utf-8
def utf8_argv():
    global iswindows
    if iswindows:
        # Versions 2.x of Python don't support Unicode in sys.argv on
        # Windows, with the underlying Windows API instead replacing multi-byte
        # characters with '?'.  So use shell32.GetCommandLineArgvW to get sys.argv 
        # as a list of Unicode strings and encode them as utf-8

        from ctypes import POINTER, byref, cdll, c_int, windll
        from ctypes.wintypes import LPCWSTR, LPWSTR

        GetCommandLineW = cdll.kernel32.GetCommandLineW
        GetCommandLineW.argtypes = []
        GetCommandLineW.restype = LPCWSTR

        CommandLineToArgvW = windll.shell32.CommandLineToArgvW
        CommandLineToArgvW.argtypes = [LPCWSTR, POINTER(c_int)]
        CommandLineToArgvW.restype = POINTER(LPWSTR)

        cmd = GetCommandLineW()
        argc = c_int(0)
        argv = CommandLineToArgvW(cmd, byref(argc))
        if argc.value > 0:
            # Remove Python executable and commands if present
            start = argc.value - len(sys.argv)
            return [argv[i].encode('utf-8') for i in
                    xrange(start, argc.value)]
        # this should never happen
        return None
    else:
        argv = []
        argvencoding = sys.stdin.encoding
        if argvencoding is None:
            argvencoding = sys.getfilesystemencoding()
        if argvencoding is None:
            argvencoding = 'utf-8'
        for arg in sys.argv:
            if type(arg) == unicode:
                argv.append(arg.encode('utf-8'))
            else:
                argv.append(arg.decode(argvencoding).encode('utf-8'))
        return argv


# Python 2.X is broken in that it does not recognize CP65001 as UTF-8
def add_cp65001_codec():
    try:
        codecs.lookup('cp65001')
    except LookupError:
        codecs.register(
            lambda name: name == 'cp65001' and codecs.lookup('utf-8') or None)
    return


# Almost all sane operating systems now default to utf-8 as the 
# proper default encoding so that all files and path names
# in any language can be properly represented.
#
# Python 3.X defaults to utf-8
#
# Many large python based programs that embed their own python 2.X interpreters
# also default to utf-8 in their embedded site.py (see for example Calibre)
#
# This routine allows us to do the same thing without changing a site.py

def set_utf8_default_encoding():
    if sys.getdefaultencoding() in ['utf-8', 'UTF-8','cp65001','CP65001']:
        return

    # Regenerate setdefaultencoding.
    reload(sys)
    sys.setdefaultencoding('utf-8')

    for attr in dir(locale):
        if attr[0:3] != 'LC_':
            continue
        aref = getattr(locale, attr)
        try:
            locale.setlocale(aref, '')
        except locale.Error:
            continue
        try:
            lang = locale.getlocale(aref)[0]
        except (TypeError, ValueError):
            continue
        if lang:
            try:
                locale.setlocale(aref, (lang, 'UTF-8'))
            except locale.Error:
                os.environ[attr] = lang + '.UTF-8'
    try:
        locale.setlocale(locale.LC_ALL, '')
    except locale.Error:
        pass
    return

