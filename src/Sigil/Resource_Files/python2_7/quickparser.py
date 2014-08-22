#! /usr/bin/python
# vim:ts=4:sw=4:softtabstop=4:smarttab:expandtab

import sys
import os

SPECIAL_HANDLING_TAGS = {
    '?xml'     : ('xmlheader', -1), 
    '!--'      : ('comment', -3),
    '!DOCTYPE' : ('doctype', -1),
}

SPECIAL_HANDLING_TYPES = ['xmlheader', 'doctype', 'comment']

class QuickXHTMLParser(object):

    def __init__(self):
        self.pos = 0
        self.content = None
        self.clen = 0
        self.tagpath = None

    def setContent(self, data, codec = 'utf-8' ):
        if data is None:
            data = ''
        if isinstance(data, unicode):
            self.content = data.encode('utf-8')
        else:
            if codec in  ['utf-8', 'UTF-8','cp65001','CP65001', 'utf8', 'UTF8']: 
                self.content = data
            else:
                udata = data.decode(codec)
                self.content = udata.encode('utf-8')
        self.pos = 0
        self.clen = len(self.content)
        self.tagpath = ['']


    # parses string version of tag to identify its name,
    # its type 'begin', 'end' or 'single',
    # plus build a hashtable of its atributes
    def parsetag(self, s):
        p = 1
        # get the tag name
        tname = None
        ttype = None
        tattr = {}
        while s[p:p+1] == ' ' : p += 1
        if s[p:p+1] == '/':
            ttype = 'end'
            p += 1
            while s[p:p+1] == ' ' : p += 1
        b = p
        while s[p:p+1] not in ('>', '/', ' ', '"', "'", "\r", "\n") : p += 1
        tname=s[b:p].lower()
        if tname == '!doctype':
            tname = '!DOCTYPE'
        # special cases
        if tname in SPECIAL_HANDLING_TAGS.keys():
            ttype, backstep = SPECIAL_HANDLING_TAGS[tname]
            tattr['special'] = s[p:backstep]
        if ttype is None:
            # parse any attributes
            while s.find('=',p) != -1 :
                while s[p:p+1] == ' ' : p += 1
                b = p
                while s[p:p+1] != '=' : p += 1
                aname = s[b:p].lower()
                aname = aname.rstrip(' ')
                p += 1
                while s[p:p+1] == ' ' : p += 1
                if s[p:p+1] in ('"', "'") :
                    p = p + 1
                    b = p
                    while s[p:p+1] not in ('"', "'") : p += 1
                    val = s[b:p]
                    p += 1
                else :
                    b = p
                    while s[p:p+1] not in ('>', '/', ' ') : p += 1
                    val = s[b:p]
                tattr[aname] = val
        # label beginning and single tags
        if ttype is None:
            ttype = 'begin'
            if s.find('/',p) >= 0:
                ttype = 'single'
        return tname, ttype, tattr

    # parse leading text of xhtml and tag
    # returns as tuple (Leading Text, Tag)
    # only one will have a value, the other will always be None
    def parseml(self):
        p = self.pos
        if p >= self.clen:
            return None, None
        if self.content[p] != '<':
            res = self.content.find('<',p)
            if res == -1 :
                res = len(self.content)
            self.pos = res
            return self.content[p:res], None
        # handle comment as a special case to deal with multi-line comments
        if self.content[p:p+4] == '<!--':
            te = self.content.find('-->',p+1)
            if te != -1:
                te = te+2
        else :
            tb = p
            te = self.content.find('>',p+1)
            ntb = self.content.find('<',p+1)
            if ntb != -1 and ntb < te:
                self.pos = ntb
                return self.content[p:ntb], None
        self.pos = te + 1
        return None, self.content[p:te+1]



    # yields leading text, tagpath prefix, tag name, tag type, tag attributes
    # tag prefix is a dotted history of all open parent ("begin') tags
    # tag types are "single", "begin", "end", "comment", "xmlheader", and "doctype"
    # tag attributes is a dictionary of key and value pairs
    def parse_iter(self):
        while True:
            text, tag = self.parseml()
            if text is None and tag is None:
                break
            tp = ".".join(self.tagpath)
            if text is not None:
                tname = ttype = tattr = None
            if tag is not None:
                text = None
                tname, ttype, tattr = self.parsetag(tag)
                if ttype == 'end':
                    last_begin = self.tagpath[-1]
                    if last_begin != tname:
                        print 'Warning: Improperly Nested Tags: %s %s' % (last_begin, tname)
            yield text, tp, tname, ttype, tattr
            if ttype is not None:
                if ttype == 'begin':
                    self.tagpath.append(tname)
                elif ttype == 'end':
                    self.tagpath.pop()


    # create xml tag from tag info
    def tag_info_to_xml(self, tname, ttype, tattr=None):
        if ttype is None or tname is None:
            return ''
        if ttype == 'end':
            return '</%s>' % tname
        if ttype in SPECIAL_HANDLING_TYPES and tattr is not None and 'special' in tattr.keys():
            info = tattr['special']
            if ttype == 'comment':
                return '<%s %s-->' % tname, info
            else:
                return '<%s %s>' % tname, info
        res = []
        res.append('<%s' % tname)
        if tattr is not None:
            for key in tattr.keys():
                res.append(' %s="%s"' % (key, tattr[key]))
        if ttype == 'single':
            res.append('/>')
        else :
            res.append('>')
        return "".join(res)

