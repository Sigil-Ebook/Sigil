#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# vim:ts=4:sw=4:softtabstop=4:smarttab:expandtab

# Copyright (c) 2014-2020 Kevin B. Hendricks, and Doug Massay
# Copyright (c) 2014      John Schember
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


# Sigil Python Script Launcher
#
# This launcher script is aways invoked by the script manager
# for python scripts.  It is passed in a number of things including
# ebook_root, output directory, script type, and path to target script location
#
# This launcher script will parse the opf and make available
# a wrapper and a type specific container object that acts as an
# interface to be used by target scripts to safely access the ebook files.
#
# The Wrapper acts as a record keeper of changed files while acting to
# shield the original files
#
# The Launcher script will then invoke the target's run() routine
# All target output to stdout and stderr is captured
# Upon completion simple xml formatted information is passed back
# to Sigil before the launcher script exits

import sys
import os

from opf_parser import Opf_Parser
from wrapper import Wrapper
from bookcontainer import BookContainer
from inputcontainer import InputContainer
from outputcontainer import OutputContainer
from validationcontainer import ValidationContainer
from hrefutils import urlencodepart

import html
from xml.sax.saxutils import escape as xmlescape

import traceback

def _utf8str(p):
    if p is None:
        return None
    if isinstance(p, bytes):
        return p
    return p.encode('utf-8', errors='replace')

def _unicodestr(p):
    if p is None:
        return None
    if isinstance(p, str):
        return p
    return p.decode('utf-8', errors='replace')


_DEBUG = False

SUPPORTED_SCRIPT_TYPES = ['input', 'output', 'edit', 'validation']

_XML_HEADER = '<?xml version="1.0" encoding="UTF-8"?>\n'

EXTRA_ENTITIES = {'"': '&quot;', "'": "&apos;"}

def escapeit(sval, EXTRAS=None):
    if EXTRAS:
        return xmlescape(html.unescape(sval), EXTRAS)
    return xmlescape(html.unescape(sval))

# Wrap a stream so that output gets saved
# using utf-8 encoding
class SavedStream:
    def __init__(self, stream, stype, ps):
        self.stream = stream
        self.encoding = stream.encoding
        self.ps = ps
        self.stype = stype
        if self.encoding is None:
            self.encoding = 'utf-8'
    def write(self, data):
        if isinstance(data, str):
            data = data.encode('utf-8')
        elif self.encoding not in ['utf-8', 'UTF-8', 'cp65001', 'CP65001']:
            udata = data.decode(self.encoding)
            data = udata.encode('utf-8')
        if self.stype == 'stdout':
            self.ps.stdouttext.append(data)
            self.stream.flush()
            self.stream.buffer.write(data)
        else:
            self.ps.stderrtext.append(data)
    def __getattr__(self, attr):
        if attr == 'mode':
            return 'wb'
        if attr == 'encoding':
            return 'utf-8'
        if attr == 'stream':
            return self.stream
        return getattr(self.stream, attr)


class ProcessScript(object):
    def __init__(self, script_type, script_module, container):
        self.container = container
        self.script_module = script_module
        self.script_type = script_type
        self.exitcode = None
        self.stdouttext = []
        self.stderrtext = []
        self.wrapout = []

    def launch(self):
        script_module = self.script_module
        script_type = self.script_type
        container = self.container
        sys.stdout = SavedStream(sys.stdout, 'stdout', self)
        sys.stderr = SavedStream(sys.stderr, 'stderr', self)
        try:
            target_script = __import__(script_module)
            self.exitcode = target_script.run(container)
            sys.stdout = sys.stdout.stream
            sys.stderr = sys.stderr.stream
        except Exception as e:
            sys.stderr.write(traceback.format_exc())
            sys.stderr.write("Error: %s\n" % e)
            sys.stdout = sys.stdout.stream
            sys.stderr = sys.stderr.stream
            self.exitcode = -1
            pass
        if self.exitcode != 0:
            self.wrapout.append(_XML_HEADER)
            self.wrapout.append('<wrapper type="%s">\n<result>failed</result>\n<changes/>\n' % script_type)
            return self.exitcode
        if script_type == "edit":
            # write out the final updated opf to the outdir
            container._w.write_opf()
        # save the wrapper results to a file before exiting the thread
        # Note: the hrefs generated for the result xml are all relative paths
        #       with no fragments since they must refer to entire files
        self.wrapout.append(_XML_HEADER)
        self.wrapout.append('<wrapper type="%s">\n' % script_type)
        self.wrapout.append('<result>success</result>\n')
        self.wrapout.append('<changes>\n')
        if script_type == "edit":
            for ftype, id, href in container._w.deleted:
                if ftype == 'manifest':
                    bookhref = href
                    mime = container._w.getmime(bookhref)
                else:
                    bookhref = id
                    id = ""
                    mime = container._w.getmime(bookhref)
                self.wrapout.append('<deleted href="%s" id="%s" media-type="%s" />\n' % (urlencodepart(bookhref), id, mime))
        if script_type in ['input', 'edit']:
            for id in container._w.added:
                if id in container._w.id_to_bookpath:
                    bookhref = container._w.id_to_bookpath[id]
                    mime = container._w.id_to_mime[id]
                else:
                    bookhref = id
                    id = ""
                    mime = container._w.getmime(bookhref)
                self.wrapout.append('<added href="%s" id="%s" media-type="%s" />\n' % (urlencodepart(bookhref), id, mime))
        if script_type == 'edit':
            for id in container._w.modified:
                if id in container._w.id_to_bookpath:
                    bookhref = container._w.id_to_bookpath[id]
                    mime = container._w.id_to_mime[id]
                else:
                    bookhref = id
                    id = ""
                    mime = container._w.getmime(bookhref)
                self.wrapout.append('<modified href="%s" id="%s" media-type="%s" />\n' % (urlencodepart(bookhref), id, mime))
        if script_type == 'validation':
            for vres in container.results:
                self.wrapout.append('<validationresult type="%s" bookpath="%s" linenumber="%s" charoffset="%s" message="%s" />\n' % (vres.restype, vres.bookpath, vres.linenumber, vres.charoffset, vres.message))
        self.wrapout.append('</changes>\n')
        self.exitcode = 0
        return


def failed(script_type, msg):
    wrapper = _XML_HEADER
    if script_type is None:
        wrapper += '<wrapper>\n<result>failed</result>\n<changes/>\n'
    else:
        wrapper += '<wrapper type="%s">\n<result>failed</result>\n<changes/>\n' % script_type
    wrapper += '<msg>%s</msg>\n</wrapper>\n' % msg
    # write it to stdout and exit
    sys.stdout.flush()
    sys.stdout.buffer.write(_utf8str(wrapper))


# uses the unicode_arv call to convert all command line paths to full unicode
# arguments:
#      path to Sigil's ebook_root
#      path to Sigil's output (temp) directory
#      script type ("input", "output", "edit")
#      path to script target file

def main(argv=sys.argv):

    if len(argv) != 5:
        failed(None, msg="Launcher: improper number of arguments passed to launcher.py")
        return -1

    ebook_root = argv[1]
    outdir = argv[2]
    script_type = argv[3]
    target_file = argv[4]
    script_home = os.path.dirname(target_file)
    plugin_name = os.path.split(script_home)[-1]
    plugin_dir = os.path.dirname(script_home)
    script_module = os.path.splitext(os.path.basename(target_file))[0]

    # remap cssutils to css_parser
    try:
        import css_parser
        sys.modules['cssutils'] = css_parser
    except ImportError:
        pass

    # do basic sanity checking anyway
    if script_type not in SUPPORTED_SCRIPT_TYPES:
        failed(None, msg="Launcher: script type %s is not supported" % script_type)
        return -1

    ok = os.path.exists(ebook_root) and os.path.isdir(ebook_root)
    ok = ok and os.path.exists(outdir) and os.path.isdir(outdir)
    ok = ok and os.path.exists(script_home) and os.path.isdir(script_home)
    ok = ok and os.path.exists(target_file) and os.path.isfile(target_file)
    if not ok:
        failed(None, msg="Launcher: missing or incorrect paths passed in")
        return -1

    # update sys with path to target module home directory
    sys.path.append(script_home)

    # load and parse opf if present
    op = None
    cfg = ''
    with open(os.path.join(outdir, 'sigil.cfg'), 'rb') as f:
        cfg = f.read().decode('utf-8')
    cfg = cfg.replace("\r", "")
    cfg_lst = cfg.split("\n")
    opfbookpath = cfg_lst[0]
    opf_path = os.path.join(ebook_root, opfbookpath.replace("/", os.sep))
    if os.path.exists(opf_path) and os.path.isfile(opf_path):
        op = Opf_Parser(opf_path, opfbookpath)
    # create a wrapper for record keeping and safety
    rk = Wrapper(ebook_root, outdir, op, plugin_dir, plugin_name)

    # get the correct container
    if script_type == 'edit':
        bc = BookContainer(rk)
    elif script_type == 'input':
        bc = InputContainer(rk)
    elif script_type == 'validation':
        bc = ValidationContainer(rk)
    else:
        bc = OutputContainer(rk)

    # start the target script
    ps = ProcessScript(script_type, script_module, bc)
    ps.launch()

    # get standard error and standard out from the target script
    successmsg = ''
    for data in ps.stdouttext:
        successmsg += _unicodestr(data)
    successmsg = escapeit(successmsg)
    errorlog = ''
    for data in ps.stderrtext:
        errorlog += _unicodestr(data)
    errorlog = escapeit(errorlog)

    # get the target's script wrapper xml
    resultxml = "".join(ps.wrapout)
    resultxml += "<msg>\n"
    if ps.exitcode == 0:
        resultxml += successmsg
        if _DEBUG:
            resultxml += errorlog
    else:
        resultxml += successmsg
        resultxml += errorlog
    resultxml += '</msg>\n</wrapper>\n'

    # write it to stdout and exit
    sys.stdout.flush
    sys.stdout.buffer.write(_utf8str(resultxml))
    return 0

if __name__ == "__main__":
    sys.exit(main())
