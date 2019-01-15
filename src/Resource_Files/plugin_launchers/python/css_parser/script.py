from __future__ import (absolute_import, division, print_function,
                        unicode_literals)

import codecs
import errno
import logging
import os
import sys

import css_parser
import css_parser.encutils as encutils

"""classes and functions used by css_parser scripts
"""

__all__ = ['CSSCapture', 'csscombine']
__docformat__ = 'restructuredtext'
__version__ = '$Id: parse.py 1323 2008-07-06 18:13:57Z cthedot $'


if sys.version_info[0] >= 3:
    from html.parser import HTMLParser as htmlparser_HTMLParser
    from urllib.parse import urlsplit as urllib_urlsplit
    from urllib.parse import urljoin as urllib_urljoin
    from urllib.request import urlopen as urllib_urlopen
    from urllib.request import Request as urllib_Request
    from urllib.error import HTTPError as urllib_HTTPError
else:
    from HTMLParser import HTMLParser as htmlparser_HTMLParser
    from urlparse import urlsplit as urllib_urlsplit
    from urlparse import urljoin as urllib_urljoin
    from urllib2 import urlopen as urllib_urlopen
    from urllib2 import HTTPError as urllib_HTTPError
    from urllib2 import Request as urllib_Request


# types of sheets in HTML
LINK = 0  # <link rel="stylesheet" type="text/css" href="..." [@title="..." @media="..."]/>
STYLE = 1  # <style type="text/css" [@title="..."]>...</style>


class CSSCaptureHTMLParser(htmlparser_HTMLParser):
    """CSSCapture helper: Parse given data for link and style elements"""
    curtag = ''
    sheets = []  # (type, [atts, cssText])

    def _loweratts(self, atts):
        return dict([(a.lower(), v.lower()) for a, v in atts])

    def handle_starttag(self, tag, atts):
        if tag == 'link':
            atts = self._loweratts(atts)
            if 'text/css' == atts.get('type', ''):
                self.sheets.append((LINK, atts))
        elif tag == 'style':
            # also get content of style
            atts = self._loweratts(atts)
            if 'text/css' == atts.get('type', ''):
                self.sheets.append((STYLE, [atts, '']))
                self.curtag = tag
        else:
            # close as only intersting <style> cannot contain any elements
            self.curtag = ''

    def handle_data(self, data):
        if self.curtag == 'style':
            self.sheets[-1][1][1] = data  # replace cssText

    def handle_comment(self, data):
        # style might have comment content, treat same as data
        self.handle_data(data)

    def handle_endtag(self, tag):
        # close as style cannot contain any elements
        self.curtag = ''


class CSSCapture(object):
    """
    Retrieve all CSS stylesheets including embedded for a given URL.
    Optional setting of User-Agent used for retrieval possible
    to handle browser sniffing servers.

    raises urllib2.HTTPError
    """

    def __init__(self, ua=None, log=None, defaultloglevel=logging.INFO):
        """
        initialize a new Capture object

        ua
            init User-Agent to use for requests
        log
            supply a log object which is used instead of the default
            log which writes to sys.stderr
        defaultloglevel
            constant of logging package which defines the level of the
            default log if no explicit log given
        """
        self._ua = ua

        if log:
            self._log = log
        else:
            self._log = logging.getLogger('CSSCapture')
            hdlr = logging.StreamHandler(sys.stderr)
            formatter = logging.Formatter('%(message)s')
            hdlr.setFormatter(formatter)
            self._log.addHandler(hdlr)
            self._log.setLevel(defaultloglevel)
            self._log.debug('Using default log')

        self._htmlparser = CSSCaptureHTMLParser()
        self._cssparser = css_parser.CSSParser(log=self._log)

    def _doRequest(self, url):
        """Do an HTTP request

        Return (url, rawcontent)
            url might have been changed by server due to redirects etc
        """
        self._log.debug('    CSSCapture._doRequest\n        * URL: %s' % url)

        req = urllib_Request(url)
        if self._ua:
            req.add_header('User-agent', self._ua)
            self._log.info('        * Using User-Agent: %s', self._ua)

        try:
            res = urllib_urlopen(req)
        except urllib_HTTPError as e:
            self._log.critical('    %s\n%s %s\n%s' % (
                e.geturl(), e.code, e.msg, e.headers))
            return None, None

        # get real url
        if url != res.geturl():
            url = res.geturl()
            self._log.info('        URL retrieved: %s', url)

        return url, res

    def _createStyleSheet(self, href=None,
                          media=None,
                          parentStyleSheet=None,
                          title='',
                          cssText=None,
                          encoding=None):
        """
        Return CSSStyleSheet read from href or if cssText is given use that.

        encoding
            used if inline style found, same as self.docencoding
        """
        if cssText is None:
            encoding, enctype, cssText = css_parser.util._readUrl(href, parentEncoding=self.docencoding)
            encoding = None  # already decoded???

        sheet = self._cssparser.parseString(cssText, href=href, media=media, title=title,
                                            encoding=encoding)

        if not sheet:
            return None

        else:
            self._log.info('    %s\n' % sheet)
            self._nonparsed[sheet] = cssText
            return sheet

    def _findStyleSheets(self, docurl, doctext):
        """
        parse text for stylesheets
        fills stylesheetlist with all found StyleSheets

        docurl
            to build a full url of found StyleSheets @href
        doctext
            to parse
        """
        # TODO: ownerNode should be set to the <link> node
        self._htmlparser.feed(doctext)

        for typ, data in self._htmlparser.sheets:
            sheet = None

            if LINK == typ:
                self._log.info('+ PROCESSING <link> %r' % data)

                atts = data
                href = urllib_urljoin(docurl, atts.get('href', None))
                sheet = self._createStyleSheet(href=href,
                                               media=atts.get('media', None),
                                               title=atts.get('title', None))
            elif STYLE == typ:
                self._log.info('+ PROCESSING <style> %r' % data)

                atts, cssText = data
                sheet = self._createStyleSheet(cssText=cssText,
                                               href=docurl,
                                               media=atts.get('media', None),
                                               title=atts.get('title', None),
                                               encoding=self.docencoding)
                if sheet:
                    sheet._href = None  # inline have no href!
                print(sheet.cssText)

            if sheet:
                self.stylesheetlist.append(sheet)
                self._doImports(sheet, base=docurl)

    def _doImports(self, parentStyleSheet, base=None):
        """
        handle all @import CSS stylesheet recursively
        found CSS stylesheets are appended to stylesheetlist
        """
        # TODO: only if not parsed these have to be read extra!

        for rule in parentStyleSheet.cssRules:
            if rule.type == rule.IMPORT_RULE:
                self._log.info('+ PROCESSING @import:')
                self._log.debug('    IN: %s\n' % parentStyleSheet.href)
                sheet = rule.styleSheet
                href = urllib_urljoin(base, rule.href)
                if sheet:
                    self._log.info('    %s\n' % sheet)
                    self.stylesheetlist.append(sheet)
                    self._doImports(sheet, base=href)

    def capture(self, url):
        """
        Capture all stylesheets at given URL's HTML document.
        Any HTTPError is raised to caller.

        url
            to capture CSS from

        Returns ``css_parser.stylesheets.StyleSheetList``.
        """
        self._log.info('\nCapturing CSS from URL:\n    %s\n', url)
        self._nonparsed = {}
        self.stylesheetlist = css_parser.stylesheets.StyleSheetList()

        # used to save inline styles
        scheme, loc, path, query, fragment = urllib_urlsplit(url)
        self._filename = os.path.basename(path)

        # get url content
        url, res = self._doRequest(url)
        if not res:
            sys.exit(1)

        rawdoc = res.read()

        self.docencoding = encutils.getEncodingInfo(
            res, rawdoc, log=self._log).encoding
        self._log.info('\nUsing Encoding: %s\n', self.docencoding)

        doctext = rawdoc.decode(self.docencoding)

        # fill list of stylesheets and list of raw css
        self._findStyleSheets(url, doctext)

        return self.stylesheetlist

    def saveto(self, dir, saveraw=False, minified=False):
        """
        saves css in "dir" in the same layout as on the server
        internal stylesheets are saved as "dir/__INLINE_STYLE__.html.css"

        dir
            directory to save files to
        saveparsed
            save literal CSS from server or save the parsed CSS
        minified
            save minified CSS

        Both parsed and minified (which is also parsed of course) will
        loose information which css_parser is unable to understand or where
        it is simple buggy. You might to first save the raw version before
        parsing of even minifying it.
        """
        msg = 'parsed'
        if saveraw:
            msg = 'raw'
        if minified:
            css_parser.ser.prefs.useMinified()
            msg = 'minified'

        inlines = 0
        for i, sheet in enumerate(self.stylesheetlist):
            url = sheet.href
            if not url:
                inlines += 1
                url = '%s_INLINE_%s.css' % (self._filename, inlines)

            # build savepath
            scheme, loc, path, query, fragment = urllib_urlsplit(url)
            # no absolute path
            if path and path.startswith('/'):
                path = path[1:]
            path = os.path.normpath(path)
            path, fn = os.path.split(path)
            savepath = os.path.join(dir, path)
            savefn = os.path.join(savepath, fn)
            try:
                os.makedirs(savepath)
            except OSError as e:
                if e.errno != errno.EEXIST:
                    raise e
                self._log.debug('Path "%s" already exists.', savepath)

            self._log.info('SAVING %s, %s %r' % (i+1, msg, savefn))

            sf = open(savefn, 'wb')
            if saveraw:
                cssText = self._nonparsed[sheet]
                uf = codecs.getwriter('css')(sf)
                uf.write(cssText)
            else:
                sf.write(sheet.cssText)
            sf.close()


def csscombine(path=None, url=None, cssText=None, href=None,
               sourceencoding=None, targetencoding=None,
               minify=True, resolveVariables=True):
    """Combine sheets referred to by @import rules in given CSS proxy sheet
    into a single new sheet.

    :returns: combined cssText, normal or minified
    :Parameters:
        `path` or `url` or `cssText` + `href`
            path or URL to a CSSStyleSheet or a cssText of a sheet which imports
            other sheets which are then combined into one sheet.
            `cssText` normally needs `href` to be able to resolve relative
            imports.
        `sourceencoding` = 'utf-8'
            explicit encoding of the source proxysheet
        `targetencoding`
            encoding of the combined stylesheet
        `minify` = True
            defines if the combined sheet should be minified, in this case
            comments are not parsed at all!
        `resolveVariables` = True
            defines if variables in combined sheet should be resolved
    """
    css_parser.log.info('Combining files from %r' % url, neverraise=True)
    if sourceencoding is not None:
        css_parser.log.info('Using source encoding %r' % sourceencoding, neverraise=True)

    parser = css_parser.CSSParser(parseComments=not minify)

    if path and not cssText:
        src = parser.parseFile(path, encoding=sourceencoding)
    elif url:
        src = parser.parseUrl(url, encoding=sourceencoding)
    elif cssText:
        src = parser.parseString(cssText, href=href, encoding=sourceencoding)
    else:
        sys.exit('Path or URL must be given')

    result = css_parser.resolveImports(src)
    result.encoding = targetencoding
    css_parser.log.info('Using target encoding: %r' % targetencoding, neverraise=True)

    oldser = css_parser.ser
    css_parser.setSerializer(css_parser.serialize.CSSSerializer())
    if minify:
        css_parser.ser.prefs.useMinified()
    css_parser.ser.prefs.resolveVariables = resolveVariables
    cssText = result.cssText
    css_parser.setSerializer(oldser)

    return cssText
