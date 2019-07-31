from __future__ import unicode_literals, division, absolute_import, print_function

import sys
PY3 = sys.version_info[0] >= 3
if PY3:
    text_type = str
    binary_type = bytes
    unicode = str
else:
    range = xrange
    text_type = unicode
    binary_type = str

__all__ = [
    'LXMLTreeBuilderForXML',
    'LXMLTreeBuilder',
    ]

from io import BytesIO
if PY3:
    from io import StringIO
else:
    from StringIO import StringIO

import collections
from lxml import etree
from sigil_bs4.element import (
    Comment,
    Doctype,
    NamespacedAttribute,
    ProcessingInstruction,
)
from sigil_bs4.builder import (
    FAST,
    HTML,
    HTMLTreeBuilder,
    PERMISSIVE,
    ParserRejectedMarkup,
    TreeBuilder,
    XML)
from sigil_bs4.dammit import EncodingDetector

LXML = 'lxml'

_standard_epub_prefix_map = {
    "http://www.idpf.org/2007/opf"                             : "opf",
    "http://purl.org/dc/elements/1.1/"                         : "dc",
    "http://purl.org/dc/terms/"                                : "dcterms",
    "http://id.loc.gov/vocabulary/"                            : "marc",
    "http://www.idpf.org/vocab/rendition/#"                    : "rendition",
    "http://www.editeur/org/ONIX/book/codelists/current.html#" : "onix",
    "http://www.idpf.org/epub/vocab/overlays/#"                : "media",
    "http://www.idpf.org/2007/ops"                             : "epub"
}


class LXMLTreeBuilderForXML(TreeBuilder):
    DEFAULT_PARSER_CLASS = etree.XMLParser

    is_xml = True

    NAME = "lxml-xml"
    ALTERNATE_NAMES = ["xml"]

    # Well, it's permissive by XML parser standards.
    features = [NAME, LXML, XML, FAST, PERMISSIVE]

    CHUNK_SIZE = 512

    # This namespace mapping is specified in the XML Namespace
    # standard.
    DEFAULT_NSMAPS = {'http://www.w3.org/XML/1998/namespace' : "xml"}

    def default_parser(self, encoding):
        # This can either return a parser object or a class, which
        # will be instantiated with default arguments.
        if self._default_parser is not None:
            return self._default_parser
        return etree.XMLParser(
            target=self, strip_cdata=False, recover=True, encoding=encoding)

    def parser_for(self, encoding):
        # Use the default parser.
        parser = self.default_parser(encoding)

        if isinstance(parser, collections.Callable):
            # Instantiate the parser with default arguments
            parser = parser(target=self, strip_cdata=False, encoding=encoding)
        return parser

    def __init__(self, parser=None, empty_element_tags=None):
        # TODO: Issue a warning if parser is present but not a
        # callable, since that means there's no way to create new
        # parsers for different encodings.
        self._default_parser = parser
        if empty_element_tags is not None:
            self.empty_element_tags = set(empty_element_tags)
        self.soup = None
        self.nsmaps = [self.DEFAULT_NSMAPS]

    def _getNsTag(self, tag):
        # Split the namespace URL out of a fully-qualified lxml tag
        # name. Copied from lxml's src/lxml/sax.py.
        if tag[0] == '{':
            return tuple(tag[1:].split('}', 1))
        else:
            return (None, tag)

    def prepare_markup(self, markup, user_specified_encoding=None,
                       exclude_encodings=None,
                       document_declared_encoding=None):
        """
        :yield: A series of 4-tuples.
         (markup, encoding, declared encoding,
          has undergone character replacement)

        Each 4-tuple represents a strategy for parsing the document.
        """
        if isinstance(markup, unicode):
            # We were given Unicode. Maybe lxml can parse Unicode on
            # this system?
            yield markup, None, document_declared_encoding, False

        if isinstance(markup, unicode):
            # No, apparently not. Convert the Unicode to UTF-8 and
            # tell lxml to parse it as UTF-8.
            yield (markup.encode("utf8"), "utf8",
                   document_declared_encoding, False)

        # Instead of using UnicodeDammit to convert the bytestring to
        # Unicode using different encodings, use EncodingDetector to
        # iterate over the encodings, and tell lxml to try to parse
        # the document as each one in turn.
        is_html = not self.is_xml
        try_encodings = [user_specified_encoding, document_declared_encoding]
        detector = EncodingDetector(
            markup, try_encodings, is_html, exclude_encodings)
        for encoding in detector.encodings:
            yield (detector.markup, encoding, document_declared_encoding, False)

    def feed(self, markup):
        if isinstance(markup, bytes):
            markup = BytesIO(markup)
        elif isinstance(markup, unicode):
            markup = StringIO(markup)

        # Call feed() at least once, even if the markup is empty,
        # or the parser won't be initialized.
        data = markup.read(self.CHUNK_SIZE)
        try:
            self.parser = self.parser_for(self.soup.original_encoding)
            self.parser.feed(data)
            while len(data) != 0:
                # Now call feed() on the rest of the data, chunk by chunk.
                data = markup.read(self.CHUNK_SIZE)
                if len(data) != 0:
                    self.parser.feed(data)
            self.parser.close()
        except (UnicodeDecodeError, LookupError, etree.ParserError) as e:
            raise ParserRejectedMarkup(str(e))

    def close(self):
        self.nsmaps = [self.DEFAULT_NSMAPS]

    def start(self, name, attrs, nsmap={}):
        # Make sure attrs is a mutable dict--lxml may send an immutable dictproxy.
        attrs = dict(attrs)
        nsprefix = None

        # ARRGGHH lxml 4.4.X has changes empty prefixes on namespaces to be the null string 
        # and not None which all prior versions used! So remap '' prefixes to be None
        # so we can work with all lxml versions
        new_nsmap = {}
        for pfx, ns in list(nsmap.items()):
            if pfx is not None and pfx == '':
                pfx = None
            new_nsmap[pfx] = ns
        nsmap = new_nsmap
        
        # Fix bug in bs4 _lxml.py that ignores attributes that specify namespaces on this tag
        # Invert each namespace map as it comes in.
        if len(nsmap) > 0:

            # to make later processing easier, remap epub namespaces to use
            # epub standard prefixes
            new_nsmap = {}
            for pfx, ns in list(nsmap.items()):
                new_pfx = _standard_epub_prefix_map.get(ns, pfx)
                if new_pfx is not None and new_pfx == "opf" and pfx is None:
                    new_pfx = None
                new_nsmap[new_pfx] = ns
            nsmap = new_nsmap

            # A new namespace mapping has come into play.
            inverted_nsmap = dict((value, key) for key, value in list(nsmap.items()))
            self.nsmaps.append(inverted_nsmap)
        
            # Also treat the namespace mapping as a set of attributes on the
            # tag, so we can properly recreate it later.
            attrs = attrs.copy()
            for prefix, namespace in list(nsmap.items()):
                attribute = NamespacedAttribute(
                    "xmlns", prefix, "http://www.w3.org/2000/xmlns/")
                attrs[attribute] = namespace

        elif len(self.nsmaps) > 1:
            # There are no new namespaces for this tag, but
            # non-default namespaces are in play, so we need a
            # separate tag stack to know when they end.
            self.nsmaps.append(None)

        # Namespaces are in play. Find any attributes that came in
        # from lxml with namespaces attached to their names, and
        # turn then into NamespacedAttribute objects.
        new_attrs = {}
        for attr, value in list(attrs.items()):
            namespace, attr = self._getNsTag(attr)
            if namespace is None:
                new_attrs[attr] = value
            else:
                nsprefix = self._prefix_for_attr_namespace(namespace)
                attr = NamespacedAttribute(nsprefix, attr, namespace)
                new_attrs[attr] = value
        attrs = new_attrs

        namespace, name = self._getNsTag(name)
        nsprefix = self._prefix_for_tag_namespace(namespace)
        self.soup.handle_starttag(name, namespace, nsprefix, attrs)

    def _prefix_for_attr_namespace(self, namespace):
        """Find the currently active prefix for the given namespace."""
        if namespace is None:
            return None
        for inverted_nsmap in reversed(self.nsmaps):
            if inverted_nsmap is not None and namespace in inverted_nsmap:
                return inverted_nsmap[namespace]
        return None

    # To keep the tag prefixes as clean/simple as possible if there is 
    # more than one possible prefix allowed and it includes None use it instead
    # This happens when a namespace prefix is added for an attribute that duplicates
    # an earlier namespace meant for tags that had set that  namespace prefix to None
    def _prefix_for_tag_namespace(self, namespace):
        """Find the currently active prefix for the given namespace for a tag."""
        if namespace is None:
            return None
        prefixes = []
        for inverted_nsmap in self.nsmaps:
            if inverted_nsmap is not None and namespace in inverted_nsmap:
                prefixes.append(inverted_nsmap[namespace])
        if len(prefixes) == 0 or (None in prefixes):
            return None
        # ow return the last (most recent) viable prefix
        return prefixes[-1]

    def end(self, name):
        self.soup.endData()
        completed_tag = self.soup.tagStack[-1]
        namespace, name = self._getNsTag(name)
        nsprefix = self._prefix_for_tag_namespace(namespace)
        self.soup.handle_endtag(name, nsprefix)
        if len(self.nsmaps) > 1:
            # This tag, or one of its parents, introduced a namespace
            # mapping, so pop it off the stack.
            self.nsmaps.pop()

    def pi(self, target, data):
        self.soup.endData()
        self.soup.handle_data(target + ' ' + data)
        self.soup.endData(ProcessingInstruction)

    def data(self, content):
        self.soup.handle_data(content)

    def doctype(self, name, pubid, system):
        self.soup.endData()
        doctype = Doctype.for_name_and_ids(name, pubid, system)
        self.soup.object_was_parsed(doctype)

    def comment(self, content):
        "Handle comments as Comment objects."
        self.soup.endData()
        self.soup.handle_data(content)
        self.soup.endData(Comment)

    def test_fragment_to_document(self, fragment):
        """See `TreeBuilder`."""
        return '<?xml version="1.0" encoding="utf-8"?>\n%s' % fragment


class LXMLTreeBuilder(HTMLTreeBuilder, LXMLTreeBuilderForXML):

    NAME = LXML
    ALTERNATE_NAMES = ["lxml-html"]

    features = ALTERNATE_NAMES + [NAME, HTML, FAST, PERMISSIVE]
    is_xml = False

    def default_parser(self, encoding):
        return etree.HTMLParser

    def feed(self, markup):
        encoding = self.soup.original_encoding
        try:
            self.parser = self.parser_for(encoding)
            self.parser.feed(markup)
            self.parser.close()
        except (UnicodeDecodeError, LookupError, etree.ParserError) as e:
            raise ParserRejectedMarkup(str(e))


    def test_fragment_to_document(self, fragment):
        """See `TreeBuilder`."""
        return '<html><body>%s</body></html>' % fragment
