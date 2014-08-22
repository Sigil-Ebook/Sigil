#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vim:ts=4:sw=4:softtabstop=4:smarttab:expandtab

# Sigil Python Script Launcher
#
# This launcher script is aways invoked by the script manager 
# for python 2 scripts.  It is passed in:
# ebook_root, output directory, script type, and path to target script location
#
# This launcher script will parse the opf and make available 
# a wrapper and a type specific container object that acts as an 
# interface to be used by target scripts to safely access the ebook files.
#
# The Wrapper acts as a record keeper of changed files while acting to 
# shield the original files
#
# The Launcher script then spawns a thread and imports the target module 
# invoking its run() routine, thread shared queues are used to capture 
# all stdout and stderr from just the thread running the target script.
# Upon completion simple xml formatted information is passed back 
# to Sigil before the wrapper script exits

import sys
import os, os.path, urllib
import codecs

import path
from path import pathof

from opf_parser import Opf_Parser
from wrapper import Wrapper
from bookcontainer import BookContainer
from inputcontainer import InputContainer
from outputcontainer import OutputContainer

from xml.sax.saxutils import escape as xmlescape
from HTMLParser import HTMLParser

from multiprocessing import Process, Queue
from Queue import Full
from Queue import Empty

import traceback

from utf8_utils import add_cp65001_codec, utf8_argv, utf8_str
add_cp65001_codec()

_DEBUG=False

SUPPORTED_SCRIPT_TYPES = ['input', 'output', 'edit'] 

_XML_HEADER = '<?xml version="1.0" encoding="UTF-8"?>\n'

EXTRA_ENTITIES = {'"':'&quot;', "'":"&apos;"}
_h = HTMLParser()

def escapeit(sval, EXTRAS=None):
    global _h
    _h = HTMLParser()
    # note, xmlescape and unescape do not work with utf-8 bytestrings
    # so pre-convert to full unicode and then convert back since our result xml is utf-8 encoded
    uval = sval.decode('utf-8')
    if EXTRAS:
        ures = xmlescape(_h.unescape(uval), EXTRAS)
    else:
        ures = xmlescape(_h.unescape(uval))
    return ures.encode('utf-8')


# Wrap a stream so that output gets appended to shared queue
# using utf-8 encoding
class QueuedStream:
    def __init__(self, stream, q):
        self.stream = stream
        self.encoding = stream.encoding
        self.q = q
        if self.encoding == None:
            self.encoding = 'utf-8'
    def write(self, data):
        if isinstance(data,unicode):
            data = data.encode('utf-8')
        elif self.encoding not in ['utf-8','UTF-8','cp65001','CP65001']:
            udata = data.decode(self.encoding)
            data = udata.encode('utf-8')
        self.q.put(data)
    def __getattr__(self, attr):
        if attr == 'mode':
            return 'wb'
        if attr == 'encoding':
            return 'utf-8'
        return getattr(self.stream, attr)


class ProcessScript(object):
    def __init__(self, script_type, script_module, container):
        self.qout = Queue()
        self.qerr = Queue()
        self.container = container
        self.script_module = script_module
        self.script_type = script_type
        self.p2 = None
        self.exitcode = None
        self.stdouttext = []
        self.stderrtext = []

    # read queue shared between this main process and spawned child processes
    def readQueueUntilEmpty(self, q):
        done = False
        text = ''
        while not done:
            try:
                data = q.get_nowait()
                text += data
            except Empty:
                done = True
                pass
        return text

    # get output from target thread and store it away
    def collectOutput(self, stream, msg):
        if msg is not None and msg != '':
            if sys.platform.startswith('win'):
                msg = msg.replace('\r\n','\n')
            if stream == 'stdout':
                self.stdouttext.append(msg)
            else:
                self.stderrtext.append(msg)
        return

    # read from target thread queues without blocking
    def processQueues(self):
        while True:
            poll = self.p2.exitcode
            if poll is not None:
                self.exitcode = poll
                outtext = self.readQueueUntilEmpty(self.qout)
                self.collectOutput('stdout', outtext)
                if poll != 0:
                    errtext = self.readQueueUntilEmpty(self.qerr)
                    self.collectOutput('stderr', errtext)
                    msg = '\n'
                else:
                    msg = '\n'
                self.p2.join()
                # self.collectOutput('stdout', msg)
                break
            text = self.readQueueUntilEmpty(self.qout)
            self.collectOutput('stdout', text)
        return


    # run in a thread and collect its output
    def launch(self):
        if self.script_type == "edit":
            self.p2 = Process(target=editEbook, args=(self.qout, self.qerr, self.script_type, self.script_module, self.container))
        elif self.script_type == "input":
            self.p2 = Process(target=inputEbook, args=(self.qout, self.qerr, self.script_type, self.script_module, self.container))
        else : # "output"
            self.p2 = Process(target=outputEbook, args=(self.qout, self.qerr, self.script_type, self.script_module, self.container))
        self.p2.start()
        self.processQueues()
        return


# multiprocessing target thread starts here
def editEbook(qout, qerr, script_type, script_module, container):
    sys.stdout = QueuedStream(sys.stdout, qout)
    sys.stderr = QueuedStream(sys.stderr, qerr)
    try:
        target_script = __import__(script_module)
        target_script.run(container)
    except Exception, e:
        sys.stderr.write(traceback.format_exc())
        sys.stderr.write("Error: %s\n" % e)
        wrapper = _XML_HEADER
        wrapper += '<wrapper type="%s">\n<result>failed</result>\n<changes/>\n' % script_type
        with open(pathof(os.path.join(container._w.outdir,'wrapresult')),"wb") as fp:
            fp.write(wrapper)
        sys.exit(1)

    # always write out the final updated opf to the outdir
    container._w.write_opf()
    # save the wrapper results to a file before exiting the thread
    wrapout = []
    wrapout.append(_XML_HEADER)
    wrapout.append('<wrapper type="%s">\n' % script_type)
    wrapout.append('<result>success</result>\n')
    wrapout.append('<changes>\n')
    for id in container._w.deleted:
        if id in container._w.id_to_href.keys():
            href = container._w.id_to_href[id]
            mime = container._w.id_to_mime[id]
            bookhref = 'OEBPS/' + href
        else:
            bookhref = id
            id = ""
            mime = container._w.getmime(bookhref)
        wrapout.append('<deleted href="%s" id="%s" media-type="%s" />\n' % (bookhref, id, mime))
    for id in container._w.added:
        if id in container._w.id_to_href.keys():
            href = container._w.id_to_href[id]
            bookhref = 'OEBPS/' + href
            mime = container._w.id_to_mime[id]
        else:
            bookhref = id
            id = ""
            mime = container._w.getmime(bookhref)
        wrapout.append('<added href="%s" id="%s" media-type="%s" />\n' % (bookhref, id, mime))
    for id in container._w.modified.keys():
        if id in container._w.id_to_href.keys():
            href = container._w.id_to_href[id]
            bookhref = 'OEBPS/' + href
            mime = container._w.id_to_mime[id]
        else:
            bookhref = id
            id = ""
            mime = container._w.getmime(bookhref)
            # type = container._w.modified[id]
        wrapout.append('<modified href="%s" id="%s" media-type="%s" />\n' % (bookhref, id, mime))
    wrapout.append('</changes>\n')
    with open(pathof(os.path.join(container._w.outdir,'wrapresult')),"wb") as fp:
        fp.write("".join(wrapout))
    sys.exit(0)


# multiprocessing target thread starts here
def outputEbook(qout, qerr, script_type, script_module, container):
    sys.stdout = QueuedStream(sys.stdout, qout)
    sys.stderr = QueuedStream(sys.stderr, qerr)
    try:
        target_script = __import__(script_module)
        target_script.run(container)
    except Exception, e:
        sys.stderr.write(traceback.format_exc())
        sys.stderr.write("Error: %s\n" % e)
        wrapper = _XML_HEADER
        wrapper += '<wrapper type="%s">\n<result>failed</result>\n<changes/>\n' % script_type
        with open(pathof(os.path.join(container._w.outdir,'wrapresult')),"wb") as fp:
            fp.write(wrapper)
        sys.exit(1)
    # save the wrapper results to a file before exiting the thread
    wrapout = []
    wrapout.append(_XML_HEADER)
    wrapout.append('<wrapper type="%s">\n' % script_type)
    wrapout.append('<result>success</result>\n<changes/>\n')
    with open(pathof(os.path.join(container._w.outdir,'wrapresult')),"wb") as fp:
        fp.write("".join(wrapout))
    sys.exit(0)


# multiprocessing target thread starts here
def inputEbook(qout, qerr, script_type, script_module, container):
    sys.stdout = QueuedStream(sys.stdout, qout)
    sys.stderr = QueuedStream(sys.stderr, qerr)
    try:
        target_script = __import__(script_module)
        target_script.run(container)
    except Exception, e:
        sys.stderr.write(traceback.format_exc())
        sys.stderr.write("Error: %s\n" % e)
        wrapper = _XML_HEADER
        wrapper += '<wrapper type="%s">\n<result>failed</result>\n<changes/>\n' % script_type
        with open(pathof(os.path.join(container._w.outdir,'wrapresult')),"wb") as fp:
            fp.write(wrapper)
        sys.exit(1)
    # save the wrapper results to a file before exiting the thread
    wrapout = []
    wrapout.append(_XML_HEADER)
    wrapout.append('<wrapper type="%s">\n' % script_type)
    wrapout.append('<result>success</result>\n')
    wrapout.append('<changes>\n')
    for id in container._w.added:
        if id in container._w.id_to_href.keys():
            href = container._w.id_to_href[id]
            bookhref = 'OEBPS/' + href
            mime = container._w.id_to_mime[id]
        else:
            bookhref = id
            id = ""
            mime = container._w.getmime(bookhref)
        wrapout.append('<added href="%s" id="%s" media-type="%s" />\n' % (bookhref, id, mime))
    wrapout.append('</changes>\n')
    with open(pathof(os.path.join(container._w.outdir,'wrapresult')),"wb") as fp:
        fp.write("".join(wrapout))
    sys.exit(0)



def failed(script_type, msg):
    wrapper = _XML_HEADER
    if script_type is None:
        wrapper += '<wrapper>\n<result>failed</result>\n<changes/>\n'
    else:
        wrapper += '<wrapper type="%s">\n<result>failed</result>\n<changes/>\n' % script_type
    wrapper += '<msg>%s</msg>\n</wrapper>\n' % msg
    sys.stderr.write(wrapper)


# uses the utf8_arv call to properly handle full unicode paths on Windows
# arguments:
#      path to Sigil's ebook_root
#      path to Sigil's output (temp) directory
#      script type ("input", "output", "edit")
#      path to script target file

def main(argv=utf8_argv()):

    if len(argv) != 5:
        failed(None, msg="Launcher: improper number of arguments passed to launcher.py")
        return -1
    
    ebook_root = argv[1]
    outdir = argv[2]
    script_type = argv[3]
    target_file = argv[4]
    script_home = os.path.dirname(target_file)
    script_module = os.path.splitext(os.path.basename(target_file))[0]

    # do basic sanity checking anyway
    if script_type not in SUPPORTED_SCRIPT_TYPES:
        failed(None, msg="Launcher: script type %s is not supported" % script_type)
        return -1

    ok = path.exists(ebook_root) and path.isdir(ebook_root)
    ok = ok and path.exists(outdir) and path.isdir(outdir)
    ok = ok and path.exists(script_home) and path.isdir(script_home)
    ok = ok and path.exists(target_file) and path.isfile(target_file)
    if not ok:
        failed(None, msg="Launcher: missing or incorrect paths passed in")
        return -1

    # update sys with path to target module home directory
    if pathof(script_home) not in sys.path:
        sys.path.append(pathof(script_home))

    # load and parse opf if present
    op = None
    opf_path = os.path.join(ebook_root,'OEBPS','content.opf')
    if path.exists(opf_path) and path.isfile(opf_path):
        op = Opf_Parser(opf_path)

    # create a wrapper for record keeping and safety
    rk = Wrapper(ebook_root, outdir, op)

    # get the correct container
    if script_type == 'edit':
        bc = BookContainer(rk)
    elif script_type == "input":
        bc = InputContainer(rk)
    else:
        bc = OutputContainer(rk)
    
    # start the target script and wait
    ps = ProcessScript(script_type, script_module, bc)
    ps.launch();

    # get standard error and standard out from thread that ran target script
    # n = len(ps.stdouttext)
    # successmsg =  "".join(ps.stdouttext[n-2:n])
    successmsg =  "".join(ps.stdouttext)
    successmsg = escapeit(successmsg)
    errorlog =  "".join(ps.stderrtext)
    errorlog = escapeit(errorlog)

    # get the exited threads wrapper data
    resultpath = os.path.join(outdir, 'wrapresult')
    wrapresult = ''
    if path.exists(resultpath) and path.isfile(resultpath):
        with open(pathof(resultpath),'rb') as fp:
            wrapresult = fp.read()
        os.remove(resultpath)
    
    # Add in final success message or error log
    if ps.exitcode == 0:
        # success
        wrapresult += "<msg>\n"
        wrapresult += successmsg
        if _DEBUG:
            wrapresult += errorlog
        wrapresult +='</msg>\n</wrapper>\n'
    else:
        wrapresult += "<msg>\n"
        if _DEBUG:
            wrapresult += successmsg
        wrapresult += errorlog
        wrapresult +='</msg>\n</wrapper>\n'

    # write it to stdout and exit
    sys.stdout.write("".join(wrapresult))
    return 0
    
if __name__ == "__main__":
    sys.exit(main())

