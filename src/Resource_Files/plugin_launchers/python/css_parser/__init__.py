#!/usr/bin/env python
from __future__ import unicode_literals, division, absolute_import, print_function
from .profiles import Profiles
from .serialize import CSSSerializer
from .parse import CSSParser
from . import css
from . import errorhandler
from .version import VERSION
import xml.dom
import os.path
import sys
"""css_parser - CSS Cascading Style Sheets library for Python

    Copyright (C) 2004-2013 Christof Hoeke

    css_parser is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.


A Python package to parse and build CSS Cascading Style Sheets. DOM only, not
any rendering facilities!

Based upon and partly implementing the following specifications :

`CSS 2.1 <http://www.w3.org/TR/CSS2/>`__
    General CSS rules and properties are defined here
`CSS 2.1 Errata  <http://www.w3.org/Style/css2-updates/CR-CSS21-20070719-errata.html>`__
    A few errata, mainly the definition of CHARSET_SYM tokens
`CSS3 Module: Syntax <http://www.w3.org/TR/css3-syntax/>`__
    Used in parts since css_parser 0.9.4. css_parser tries to use the features from
    CSS 2.1 and CSS 3 with preference to CSS3 but as this is not final yet some
    parts are from CSS 2.1
`MediaQueries <http://www.w3.org/TR/css3-mediaqueries/>`__
    MediaQueries are part of ``stylesheets.MediaList`` since v0.9.4, used in
    @import and @media rules.
`Namespaces <http://dev.w3.org/csswg/css3-namespace/>`__
    Added in v0.9.1, updated to definition in CSSOM in v0.9.4, updated in 0.9.5
    for dev version
`CSS3 Module: Pages Media <http://www.w3.org/TR/css3-page/>`__
    Most properties of this spec are implemented including MarginRules
`Selectors <http://www.w3.org/TR/css3-selectors/>`__
    The selector syntax defined here (and not in CSS 2.1) should be parsable
    with css_parser (*should* mind though ;) )

`DOM Level 2 Style CSS <http://www.w3.org/TR/DOM-Level-2-Style/css.html>`__
    DOM for package css. 0.9.8 removes support for CSSValue and related API,
    see PropertyValue and Value API for now
`DOM Level 2 Style Stylesheets <http://www.w3.org/TR/DOM-Level-2-Style/stylesheets.html>`__
    DOM for package stylesheets
`CSSOM <http://dev.w3.org/csswg/cssom/>`__
    A few details (mainly the NamespaceRule DOM) is taken from here. Plan is
    to move implementation to the stuff defined here which is newer but still
    no REC so might change anytime...


The css_parser tokenizer is a customized implementation of `CSS3 Module: Syntax
(W3C Working Draft 13 August 2003) <http://www.w3.org/TR/css3-syntax/>`__ which
itself is based on the CSS 2.1 tokenizer. It tries to be as compliant as
possible but uses some (helpful) parts of the CSS 2.1 tokenizer.

I guess css_parser is neither CSS 2.1 nor CSS 3 compliant but tries to at least
be able to parse both grammars including some more real world cases (some CSS
hacks are actually parsed and serialized). Both official grammars are not final
nor bugfree but still feasible. css_parser aim is not to be fully compliant to
any CSS specification (the specifications seem to be in a constant flow anyway)
but css_parser *should* be able to read and write as many as possible CSS
stylesheets "in the wild" while at the same time implement the official APIs
which are well documented. Some minor extensions are provided as well.

Please visit http://cthedot.de/css_parser/ for more details.


Tested with Python 2.7.6 and 3.3.3 on Windows 8.1 64bit.


This library may be used ``from css_parser import *`` which
import subpackages ``css`` and ``stylesheets``, CSSParser and
CSSSerializer classes only.

Usage may be::

    >>> from css_parser import *
    >>> parser = CSSParser()
    >>> sheet = parser.parseString(u'a { color: red}')
    >>> print sheet.cssText
    a {
        color: red
        }

"""

__all__ = list(map(str, ('css', 'stylesheets', 'CSSParser', 'CSSSerializer')))
__docformat__ = 'restructuredtext'
__author__ = 'Christof Hoeke with contributions by Walter Doerwald'
__date__ = '$LastChangedDate::                            $:'


if sys.version_info[0] >= 3:
    text_type = str
    from urllib.parse import urlsplit as urllib_urlsplit
    from urllib.request import pathname2url as urllib_pathname2url
else:
    text_type = unicode
    from urlparse import urlsplit as urllib_urlsplit
    from urllib import pathname2url as urllib_pathname2url

if sys.version_info < (2, 6):
    bytes = str


# order of imports is important (partly circular)
log = errorhandler.ErrorHandler()
VERSION

ser = CSSSerializer()

profile = Profiles(log=log)

# used by Selector defining namespace prefix '*'
_ANYNS = -1


class DOMImplementationCSS(object):
    """This interface allows the DOM user to create a CSSStyleSheet
    outside the context of a document. There is no way to associate
    the new CSSStyleSheet with a document in DOM Level 2.

    This class is its *own factory*, as it is given to
    xml.dom.registerDOMImplementation which simply calls it and receives
    an instance of this class then.
    """
    _features = [
        ('css', '1.0'),
        ('css', '2.0'),
        ('stylesheets', '1.0'),
        ('stylesheets', '2.0')
    ]

    def createCSSStyleSheet(self, title, media):
        """
        Creates a new CSSStyleSheet.

        title of type DOMString
            The advisory title. See also the Style Sheet Interfaces
            section.
        media of type DOMString
            The comma-separated list of media associated with the new style
            sheet. See also the Style Sheet Interfaces section.

        returns
            CSSStyleSheet: A new CSS style sheet.

        TODO: DOMException
            SYNTAX_ERR: Raised if the specified media string value has a
            syntax error and is unparsable.
        """
        import warnings
        warning = ("Deprecated, see "
                   "https://bitbucket.org/cthedot/css_parser/issues/69#comment-30669799")
        warnings.warn(warning, DeprecationWarning)
        return css.CSSStyleSheet(title=title, media=media)

    def createDocument(self, *args, **kwargs):
        # sometimes css_parser is picked automatically for
        # xml.dom.getDOMImplementation, so we should provide an implementation
        # see https://bitbucket.org/cthedot/css_parser/issues/69
        import xml.dom.minidom as minidom
        return minidom.DOMImplementation().createDocument(*args, **kwargs)

    def createDocumentType(self, *args, **kwargs):
        # sometimes css_parser is picked automatically for
        # xml.dom.getDOMImplementation, so we should provide an implementation
        # see https://bitbucket.org/cthedot/css_parser/issues/69
        import xml.dom.minidom as minidom
        return minidom.DOMImplementation().createDocumentType(*args, **kwargs)

    def hasFeature(self, feature, version):
        return (feature.lower(), text_type(version)) in self._features


xml.dom.registerDOMImplementation('css_parser', DOMImplementationCSS)


def parseString(*a, **k):
    return CSSParser().parseString(*a, **k)


parseString.__doc__ = CSSParser.parseString.__doc__


def parseFile(*a, **k):
    return CSSParser().parseFile(*a, **k)


parseFile.__doc__ = CSSParser.parseFile.__doc__


def parseUrl(*a, **k):
    return CSSParser().parseUrl(*a, **k)


parseUrl.__doc__ = CSSParser.parseUrl.__doc__


def parseStyle(*a, **k):
    return CSSParser().parseStyle(*a, **k)


parseStyle.__doc__ = CSSParser.parseStyle.__doc__

# set "ser", default serializer


def setSerializer(serializer):
    """Set the global serializer used by all class in css_parser."""
    global ser
    ser = serializer


def getUrls(sheet):
    """Retrieve all ``url(urlstring)`` values (in e.g.
    :class:`css_parser.css.CSSImportRule` or :class:`css_parser.css.CSSValue`
    objects of given `sheet`.

    :param sheet:
        :class:`css_parser.css.CSSStyleSheet` object whose URLs are yielded

    This function is a generator. The generated URL values exclude ``url(`` and
    ``)`` and surrounding single or double quotes.
    """
    for importrule in (r for r in sheet if r.type == r.IMPORT_RULE):
        yield importrule.href

    def styleDeclarations(base):
        "recursive generator to find all CSSStyleDeclarations"
        if hasattr(base, 'cssRules'):
            for rule in base.cssRules:
                for s in styleDeclarations(rule):
                    yield s
        elif hasattr(base, 'style'):
            yield base.style

    for style in styleDeclarations(sheet):
        for p in style.getProperties(all=True):
            for v in p.propertyValue:
                if v.type == 'URI':
                    yield v.uri


def replaceUrls(sheetOrStyle, replacer, ignoreImportRules=False):
    """Replace all URLs in :class:`css_parser.css.CSSImportRule` or
    :class:`css_parser.css.CSSValue` objects of given `sheetOrStyle`.

    :param sheetOrStyle:
        a :class:`css_parser.css.CSSStyleSheet` or a
        :class:`css_parser.css.CSSStyleDeclaration` which is changed in place
    :param replacer:
        a function which is called with a single argument `url` which
        is the current value of each url() excluding ``url(``, ``)`` and
        surrounding (single or double) quotes.
    :param ignoreImportRules:
        if ``True`` does not call `replacer` with URLs from @import rules.
    """
    if not ignoreImportRules and not isinstance(sheetOrStyle,
                                                css.CSSStyleDeclaration):
        for importrule in (r for r in sheetOrStyle if r.type == r.IMPORT_RULE):
            importrule.href = replacer(importrule.href)

    def styleDeclarations(base):
        "recursive generator to find all CSSStyleDeclarations"
        if hasattr(base, 'cssRules'):
            for rule in base.cssRules:
                for s in styleDeclarations(rule):
                    yield s
        elif hasattr(base, 'style'):
            yield base.style
        elif isinstance(sheetOrStyle, css.CSSStyleDeclaration):
            # base is a style already
            yield base

    for style in styleDeclarations(sheetOrStyle):
        for p in style.getProperties(all=True):
            for v in p.propertyValue:
                if v.type == v.URI:
                    v.uri = replacer(v.uri)


def resolveImports(sheet, target=None):
    """Recurcively combine all rules in given `sheet` into a `target` sheet.
    @import rules which use media information are tried to be wrapped into
    @media rules so keeping the media information. This may not work in
    all instances (if e.g. an @import rule itself contains an @import rule
    with different media infos or if it contains rules which may not be
    used inside an @media block like @namespace rules.). In these cases
    the @import rule is kept as in the original sheet and a WARNING is issued.

    :param sheet:
        in this given :class:`css_parser.css.CSSStyleSheet` all import rules are
        resolved and added to a resulting *flat* sheet.
    :param target:
        A :class:`css_parser.css.CSSStyleSheet` object which will be the
        resulting *flat* sheet if given
    :returns: given `target` or a new :class:`css_parser.css.CSSStyleSheet`
        object
    """
    if not target:
        target = css.CSSStyleSheet(href=sheet.href,
                                   media=sheet.media,
                                   title=sheet.title)

    def getReplacer(targetbase):
        "Return a replacer which uses base to return adjusted URLs"
        basesch, baseloc, basepath, basequery, basefrag = urllib_urlsplit(targetbase)
        basepath, basepathfilename = os.path.split(basepath)

        def replacer(uri):
            scheme, location, path, query, fragment = urllib_urlsplit(uri)
            if not scheme and not location and not path.startswith('/'):
                # relative
                path, filename = os.path.split(path)
                combined = os.path.normpath(os.path.join(basepath, path, filename))
                return urllib_pathname2url(combined)
            else:
                # keep anything absolute
                return uri

        return replacer

    for rule in sheet.cssRules:
        if rule.type == rule.CHARSET_RULE:
            pass
        elif rule.type == rule.IMPORT_RULE:
            log.info('Processing @import %r' % rule.href, neverraise=True)

            if rule.hrefFound:
                # add all rules of @import to current sheet
                target.add(css.CSSComment(cssText='/* START @import "%s" */'
                                          % rule.href))

                try:
                    # nested imports
                    importedSheet = resolveImports(rule.styleSheet)
                except xml.dom.HierarchyRequestErr as e:
                    log.warn('@import: Cannot resolve target, keeping rule: %s'
                             % e, neverraise=True)
                    target.add(rule)
                else:
                    # adjust relative URI references
                    log.info('@import: Adjusting paths for %r' % rule.href,
                             neverraise=True)
                    replaceUrls(importedSheet,
                                getReplacer(rule.href),
                                ignoreImportRules=True)

                    # might have to wrap rules in @media if media given
                    if rule.media.mediaText == 'all':
                        mediaproxy = None
                    else:
                        keepimport = False
                        for r in importedSheet:
                            # check if rules present which may not be
                            # combined with media
                            if r.type not in (r.COMMENT,
                                              r.STYLE_RULE,
                                              r.IMPORT_RULE):
                                keepimport = True
                                break
                        if keepimport:
                            log.warn('Cannot combine imported sheet with'
                                     ' given media as other rules then'
                                     ' comments or stylerules found %r,'
                                     ' keeping %r' % (r,
                                                      rule.cssText),
                                     neverraise=True)
                            target.add(rule)
                            continue

                        # wrap in @media if media is not `all`
                        log.info('@import: Wrapping some rules in @media '
                                 ' to keep media: %s'
                                 % rule.media.mediaText, neverraise=True)
                        mediaproxy = css.CSSMediaRule(rule.media.mediaText)

                    for r in importedSheet:
                        if mediaproxy:
                            mediaproxy.add(r)
                        else:
                            # add to top sheet directly but are difficult anyway
                            target.add(r)

                    if mediaproxy:
                        target.add(mediaproxy)

            else:
                # keep @import as it is
                log.error('Cannot get referenced stylesheet %r, keeping rule'
                          % rule.href, neverraise=True)
                target.add(rule)

        else:
            target.add(rule)

    return target


if __name__ == '__main__':
    print(__doc__)
