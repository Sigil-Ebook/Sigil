// Copyright 2010 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Author: jdtang@google.com (Jonathan Tang)
//
// We use Gumbo as a prefix for types, gumbo_ as a prefix for functions, and
// GUMBO_ as a prefix for enum constants (static constants get the Google-style
// kGumbo prefix).

/**
 * @file
 * @mainpage Gumbo HTML Parser
 *
 * This provides a conformant, no-dependencies implementation of the HTML5
 * parsing algorithm.  It supports only UTF8; if you need to parse a different
 * encoding, run a preprocessing step to convert to UTF8.  It returns a parse
 * tree made of the structs in this file.
 *
 * Example:
 * @code
 *    GumboOutput* output = gumbo_parse(input);
 *    do_something_with_doctype(output->document);
 *    do_something_with_html_tree(output->root);
 *    gumbo_destroy_output(&options, output);
 * @endcode
 * HTML5 Spec:
 *
 * http://www.whatwg.org/specs/web-apps/current-work/multipage/syntax.html
 */

#ifndef GUMBO_GUMBO_H_
#define GUMBO_GUMBO_H_

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#define fileno _fileno
#endif

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * A struct representing a character position within the original text buffer.
 * Line and column numbers are 1-based and offsets are 0-based, which matches
 * how most editors and command-line tools work.  Also, columns measure
 * positions in terms of characters while offsets measure by bytes; this is
 * because the offset field is often used to pull out a particular region of
 * text (which in most languages that bind to C implies pointer arithmetic on a
 * buffer of bytes), while the column field is often used to reference a
 * particular column on a printable display, which nowadays is usually UTF-8.
 */
typedef struct {
  unsigned int line;
  unsigned int column;
  unsigned int offset;
} GumboSourcePosition;

/**
 * A SourcePosition used for elements that have no source position, i.e.
 * parser-inserted elements.
 */
extern const GumboSourcePosition kGumboEmptySourcePosition;


/**
 * A struct representing a string or part of a string.  Strings within the
 * parser are represented by a char* and a length; the char* points into
 * an existing data buffer owned by some other code (often the original input).
 * GumboStringPieces are assumed (by convention) to be immutable, because they
 * may share data.  Use GumboStringBuffer if you need to construct a string.
 * Clients should assume that it is not NUL-terminated, and should always use
 * explicit lengths when manipulating them.
 */
typedef struct {
  /** A pointer to the beginning of the string.  NULL iff length == 0. */
  const char* data;

  /** The length of the string fragment, in bytes.  May be zero. */
  size_t length;
} GumboStringPiece;

/** A constant to represent a 0-length null string. */
extern const GumboStringPiece kGumboEmptyString;

/**
 * Compares two GumboStringPieces, and returns true if they're equal or false
 * otherwise.
 */
bool gumbo_string_equals(
    const GumboStringPiece* str1, const GumboStringPiece* str2);

/**
 * Compares two GumboStringPieces ignoring case, and returns true if they're
 * equal or false otherwise.
 */
bool gumbo_string_equals_ignore_case(
    const GumboStringPiece* str1, const GumboStringPiece* str2);


/**
 * A simple vector implementation.  This stores a pointer to a data array and a
 * length.  All elements are stored as void*; client code must cast to the
 * appropriate type.  Overflows upon addition result in reallocation of the data
 * array, with the size doubling to maintain O(1) amortized cost.  There is no
 * removal function, as this isn't needed for any of the operations within this
 * library.  Iteration can be done through inspecting the structure directly in
 * a for-loop.
 */
typedef struct {
  /** Data elements.  This points to a dynamically-allocated array of capacity
   * elements, each a void* to the element itself.
   */
  void** data;

  /** Number of elements currently in the vector. */
  unsigned int length;

  /** Current array capacity. */
  unsigned int capacity;
} GumboVector;

/** An empty (0-length, 0-capacity) GumboVector. */
extern const GumboVector kGumboEmptyVector;

/**
 * Returns the first index at which an element appears in this vector (testing
 * by pointer equality), or -1 if it never does.
 */
int gumbo_vector_index_of(GumboVector* vector, void* element);


/**
 * An enum for all the tags defined in the HTML5 standard.  These correspond to
 * the tag names themselves.  Enum constants exist only for tags which appear in
 * the spec itself (or for tags with special handling in the SVG and MathML
 * namespaces); any other tags appear as GUMBO_TAG_UNKNOWN and the actual tag
 * name can be obtained through original_tag.
 *
 * This is mostly for API convenience, so that clients of this library don't
 * need to perform a strcasecmp to find the normalized tag name.  It also has
 * efficiency benefits, by letting the parser work with enums instead of
 * strings.
 */
typedef enum {
  // http://www.whatwg.org/specs/web-apps/current-work/multipage/semantics.html#the-root-element
  GUMBO_TAG_HTML,
  // http://www.whatwg.org/specs/web-apps/current-work/multipage/semantics.html#document-metadata
  GUMBO_TAG_HEAD,
  GUMBO_TAG_TITLE,
  GUMBO_TAG_BASE,
  GUMBO_TAG_LINK,
  GUMBO_TAG_META,
  GUMBO_TAG_STYLE,
  // http://www.whatwg.org/specs/web-apps/current-work/multipage/scripting-1.html#scripting-1
  GUMBO_TAG_SCRIPT,
  GUMBO_TAG_NOSCRIPT,
  GUMBO_TAG_TEMPLATE,
  // http://www.whatwg.org/specs/web-apps/current-work/multipage/sections.html#sections
  GUMBO_TAG_BODY,
  GUMBO_TAG_ARTICLE,
  GUMBO_TAG_SECTION,
  GUMBO_TAG_NAV,
  GUMBO_TAG_ASIDE,
  GUMBO_TAG_H1,
  GUMBO_TAG_H2,
  GUMBO_TAG_H3,
  GUMBO_TAG_H4,
  GUMBO_TAG_H5,
  GUMBO_TAG_H6,
  GUMBO_TAG_HGROUP,
  GUMBO_TAG_HEADER,
  GUMBO_TAG_FOOTER,
  GUMBO_TAG_ADDRESS,
  // http://www.whatwg.org/specs/web-apps/current-work/multipage/grouping-content.html#grouping-content
  GUMBO_TAG_P,
  GUMBO_TAG_HR,
  GUMBO_TAG_PRE,
  GUMBO_TAG_BLOCKQUOTE,
  GUMBO_TAG_OL,
  GUMBO_TAG_UL,
  GUMBO_TAG_LI,
  GUMBO_TAG_DL,
  GUMBO_TAG_DT,
  GUMBO_TAG_DD,
  GUMBO_TAG_FIGURE,
  GUMBO_TAG_FIGCAPTION,
  GUMBO_TAG_MAIN,
  GUMBO_TAG_DIV,
  // http://www.whatwg.org/specs/web-apps/current-work/multipage/text-level-semantics.html#text-level-semantics
  GUMBO_TAG_A,
  GUMBO_TAG_EM,
  GUMBO_TAG_STRONG,
  GUMBO_TAG_SMALL,
  GUMBO_TAG_S,
  GUMBO_TAG_CITE,
  GUMBO_TAG_Q,
  GUMBO_TAG_DFN,
  GUMBO_TAG_ABBR,
  GUMBO_TAG_DATA,
  GUMBO_TAG_TIME,
  GUMBO_TAG_CODE,
  GUMBO_TAG_VAR,
  GUMBO_TAG_SAMP,
  GUMBO_TAG_KBD,
  GUMBO_TAG_SUB,
  GUMBO_TAG_SUP,
  GUMBO_TAG_I,
  GUMBO_TAG_B,
  GUMBO_TAG_U,
  GUMBO_TAG_MARK,
  GUMBO_TAG_RUBY,
  GUMBO_TAG_RT,
  GUMBO_TAG_RP,
  GUMBO_TAG_BDI,
  GUMBO_TAG_BDO,
  GUMBO_TAG_SPAN,
  GUMBO_TAG_BR,
  GUMBO_TAG_WBR,
  // http://www.whatwg.org/specs/web-apps/current-work/multipage/edits.html#edits
  GUMBO_TAG_INS,
  GUMBO_TAG_DEL,
  // http://www.whatwg.org/specs/web-apps/current-work/multipage/embedded-content-1.html#embedded-content-1
  GUMBO_TAG_IMAGE,
  GUMBO_TAG_IMG,
  GUMBO_TAG_IFRAME,
  GUMBO_TAG_EMBED,
  GUMBO_TAG_OBJECT,
  GUMBO_TAG_PARAM,
  GUMBO_TAG_VIDEO,
  GUMBO_TAG_AUDIO,
  GUMBO_TAG_SOURCE,
  GUMBO_TAG_TRACK,
  GUMBO_TAG_CANVAS,
  GUMBO_TAG_MAP,
  GUMBO_TAG_AREA,
  // http://www.whatwg.org/specs/web-apps/current-work/multipage/the-map-element.html#mathml
  GUMBO_TAG_MATH,
  GUMBO_TAG_MI,
  GUMBO_TAG_MO,
  GUMBO_TAG_MN,
  GUMBO_TAG_MS,
  GUMBO_TAG_MTEXT,
  GUMBO_TAG_MGLYPH,
  GUMBO_TAG_MALIGNMARK,
  GUMBO_TAG_ANNOTATION_XML,
  // http://www.whatwg.org/specs/web-apps/current-work/multipage/the-map-element.html#svg-0
  GUMBO_TAG_SVG,
  GUMBO_TAG_FOREIGNOBJECT,
  GUMBO_TAG_DESC,
  // SVG title tags will have GUMBO_TAG_TITLE as with HTML.
  // http://www.whatwg.org/specs/web-apps/current-work/multipage/tabular-data.html#tabular-data
  GUMBO_TAG_TABLE,
  GUMBO_TAG_CAPTION,
  GUMBO_TAG_COLGROUP,
  GUMBO_TAG_COL,
  GUMBO_TAG_TBODY,
  GUMBO_TAG_THEAD,
  GUMBO_TAG_TFOOT,
  GUMBO_TAG_TR,
  GUMBO_TAG_TD,
  GUMBO_TAG_TH,
  // http://www.whatwg.org/specs/web-apps/current-work/multipage/forms.html#forms
  GUMBO_TAG_FORM,
  GUMBO_TAG_FIELDSET,
  GUMBO_TAG_LEGEND,
  GUMBO_TAG_LABEL,
  GUMBO_TAG_INPUT,
  GUMBO_TAG_BUTTON,
  GUMBO_TAG_SELECT,
  GUMBO_TAG_DATALIST,
  GUMBO_TAG_OPTGROUP,
  GUMBO_TAG_OPTION,
  GUMBO_TAG_TEXTAREA,
  GUMBO_TAG_KEYGEN,
  GUMBO_TAG_OUTPUT,
  GUMBO_TAG_PROGRESS,
  GUMBO_TAG_METER,
  // http://www.whatwg.org/specs/web-apps/current-work/multipage/interactive-elements.html#interactive-elements
  GUMBO_TAG_DETAILS,
  GUMBO_TAG_SUMMARY,
  GUMBO_TAG_MENU,
  GUMBO_TAG_MENUITEM,
  // Non-conforming elements that nonetheless appear in the HTML5 spec.
  // http://www.whatwg.org/specs/web-apps/current-work/multipage/obsolete.html#non-conforming-features
  GUMBO_TAG_APPLET,
  GUMBO_TAG_ACRONYM,
  GUMBO_TAG_BGSOUND,
  GUMBO_TAG_DIR,
  GUMBO_TAG_FRAME,
  GUMBO_TAG_FRAMESET,
  GUMBO_TAG_NOFRAMES,
  GUMBO_TAG_ISINDEX,
  GUMBO_TAG_LISTING,
  GUMBO_TAG_XMP,
  GUMBO_TAG_NEXTID,
  GUMBO_TAG_NOEMBED,
  GUMBO_TAG_PLAINTEXT,
  GUMBO_TAG_RB,
  GUMBO_TAG_STRIKE,
  GUMBO_TAG_BASEFONT,
  GUMBO_TAG_BIG,
  GUMBO_TAG_BLINK,
  GUMBO_TAG_CENTER,
  GUMBO_TAG_FONT,
  GUMBO_TAG_MARQUEE,
  GUMBO_TAG_MULTICOL,
  GUMBO_TAG_NOBR,
  GUMBO_TAG_SPACER,
  GUMBO_TAG_TT,
  // Used for all tags that don't have special handling in HTML.
  GUMBO_TAG_UNKNOWN,
  // A marker value to indicate the end of the enum, for iterating over it.
  // Also used as the terminator for varargs functions that take tags.
  GUMBO_TAG_LAST,
} GumboTag;

/**
 * Returns the normalized (usually all-lowercased, except for foreign content)
 * tag name for an GumboTag enum.  Return value is static data owned by the
 * library.
 */
const char* gumbo_normalized_tagname(GumboTag tag);

/**
 * Extracts the tag name from the original_text field of an element or token by
 * stripping off </> characters and attributes and adjusting the passed-in
 * GumboStringPiece appropriately.  The tag name is in the original case and
 * shares a buffer with the original text, to simplify memory management.
 * Behavior is undefined if a string-piece that doesn't represent an HTML tag
 * (<tagname> or </tagname>) is passed in.  If the string piece is completely
 * empty (NULL data pointer), then this function will exit successfully as a
 * no-op.
 */
void gumbo_tag_from_original_text(GumboStringPiece* text);

/**
 * Fixes the case of SVG elements that are not all lowercase.
 * http://www.whatwg.org/specs/web-apps/current-work/multipage/tree-construction.html#parsing-main-inforeign
 * This is not done at parse time because there's no place to store a mutated
 * tag name.  tag_name is an enum (which will be TAG_UNKNOWN for most SVG tags
 * without special handling), while original_tag_name is a pointer into the
 * original buffer.  Instead, we provide this helper function that clients can
 * use to rename SVG tags as appropriate.
 * Returns the case-normalized SVG tagname if a replacement is found, or NULL if
 * no normalization is called for.  The return value is static data and owned by
 * the library.
 */
const char* gumbo_normalize_svg_tagname(const GumboStringPiece* tagname);

/**
 * Converts a tag name string (which may be in upper or mixed case) to a tag
 * enum.
 */
GumboTag gumbo_tag_enum(const char* tagname);

/**
 * Attribute namespaces.
 * HTML includes special handling for XLink, XML, and XMLNS namespaces on
 * attributes.  Everything else goes in the generic "NONE" namespace.
 */
typedef enum {
  GUMBO_ATTR_NAMESPACE_NONE,
  GUMBO_ATTR_NAMESPACE_XLINK,
  GUMBO_ATTR_NAMESPACE_XML,
  GUMBO_ATTR_NAMESPACE_XMLNS,
} GumboAttributeNamespaceEnum;

/**
 * A struct representing a single attribute on an HTML tag.  This is a
 * name-value pair, but also includes information about source locations and
 * original source text.
 */
typedef struct {
  /**
   * The namespace for the attribute.  This will usually be
   * GUMBO_ATTR_NAMESPACE_NONE, but some XLink/XMLNS/XML attributes take special
   * values, per:
   * http://www.whatwg.org/specs/web-apps/current-work/multipage/tree-construction.html#adjust-foreign-attributes
   */
  GumboAttributeNamespaceEnum attr_namespace;

  /**
   * The name of the attribute.  This is in a freshly-allocated buffer to deal
   * with case-normalization, and is null-terminated.
   */
  const char* name;

  /**
   * The original text of the attribute name, as a pointer into the original
   * source buffer.
   */
  GumboStringPiece original_name;

  /**
   * The value of the attribute.  This is in a freshly-allocated buffer to deal
   * with unescaping, and is null-terminated.  It does not include any quotes
   * that surround the attribute.  If the attribute has no value (for example,
   * 'selected' on a checkbox), this will be an empty string.
   */
  const char* value;

  /**
   * The original text of the value of the attribute.  This points into the
   * original source buffer.  It includes any quotes that surround the
   * attribute, and you can look at original_value.data[0] and
   * original_value.data[original_value.length - 1] to determine what the quote
   * characters were.  If the attribute has no value, this will be a 0-length
   * string.
   */
  GumboStringPiece original_value;

  /** The starting position of the attribute name. */
  GumboSourcePosition name_start;

  /**
   * The ending position of the attribute name.  This is not always derivable
   * from the starting position of the value because of the possibility of
   * whitespace around the = sign.
   */
  GumboSourcePosition name_end;

  /** The starting position of the attribute value. */
  GumboSourcePosition value_start;

  /** The ending position of the attribute value. */
  GumboSourcePosition value_end;
} GumboAttribute;

/**
 * Given a vector of GumboAttributes, look up the one with the specified name
 * and return it, or NULL if no such attribute exists.  This uses a
 * case-insensitive match, as HTML is case-insensitive.
 */
GumboAttribute* gumbo_get_attribute(const GumboVector* attrs, const char* name);

/**
 * Enum denoting the type of node.  This determines the type of the node.v
 * union.
 */
typedef enum {
  /** Document node.  v will be a GumboDocument. */
  GUMBO_NODE_DOCUMENT,
  /** Element node.  v will be a GumboElement. */
  GUMBO_NODE_ELEMENT,
  /** Text node.  v will be a GumboText. */
  GUMBO_NODE_TEXT,
  /** CDATA node. v will be a GumboText. */
  GUMBO_NODE_CDATA,
  /** Comment node.  v. will be a GumboText, excluding comment delimiters. */
  GUMBO_NODE_COMMENT,
  /** Text node, where all contents is whitespace.  v will be a GumboText. */
  GUMBO_NODE_WHITESPACE
} GumboNodeType;

/**
 * Forward declaration of GumboNode so it can be used recursively in
 * GumboNode.parent.
 */
typedef struct GumboInternalNode GumboNode;

/** http://www.whatwg.org/specs/web-apps/current-work/complete/dom.html#quirks-mode */
typedef enum {
  GUMBO_DOCTYPE_NO_QUIRKS,
  GUMBO_DOCTYPE_QUIRKS,
  GUMBO_DOCTYPE_LIMITED_QUIRKS
} GumboQuirksModeEnum;

/**
 * Namespaces.
 * Unlike in X(HT)ML, namespaces in HTML5 are not denoted by a prefix.  Rather,
 * anything inside an <svg> tag is in the SVG namespace, anything inside the
 * <math> tag is in the MathML namespace, and anything else is inside the HTML
 * namespace.  No other namespaces are supported, so this can be an enum only.
 */
typedef enum {
  GUMBO_NAMESPACE_HTML,
  GUMBO_NAMESPACE_SVG,
  GUMBO_NAMESPACE_MATHML
} GumboNamespaceEnum;

/**
 * Parse flags.
 * We track the reasons for parser insertion of nodes and store them in a
 * bitvector in the node itself.  This lets client code optimize out nodes that
 * are implied by the HTML structure of the document, or flag constructs that
 * may not be allowed by a style guide, or track the prevalence of incorrect or
 * tricky HTML code.
 */
typedef enum {
  /**
   * A normal node - both start and end tags appear in the source, nothing has
   * been reparented.
   */
  GUMBO_INSERTION_NORMAL = 0,

  /**
   * A node inserted by the parser to fulfill some implicit insertion rule.
   * This is usually set in addition to some other flag giving a more specific
   * insertion reason; it's a generic catch-all term meaning "The start tag for
   * this node did not appear in the document source".
   */
  GUMBO_INSERTION_BY_PARSER = 1 << 0,

  /**
   * A flag indicating that the end tag for this node did not appear in the
   * document source.  Note that in some cases, you can still have
   * parser-inserted nodes with an explicit end tag: for example, "Text</html>"
   * has GUMBO_INSERTED_BY_PARSER set on the <html> node, but
   * GUMBO_INSERTED_END_TAG_IMPLICITLY is unset, as the </html> tag actually
   * exists.  This flag will be set only if the end tag is completely missing;
   * in some cases, the end tag may be misplaced (eg. a </body> tag with text
   * afterwards), which will leave this flag unset and require clients to
   * inspect the parse errors for that case.
   */
  GUMBO_INSERTION_IMPLICIT_END_TAG = 1 << 1,

  // Value 1 << 2 was for a flag that has since been removed.

  /**
   * A flag for nodes that are inserted because their presence is implied by
   * other tags, eg. <html>, <head>, <body>, <tbody>, etc.
   */
  GUMBO_INSERTION_IMPLIED = 1 << 3,

  /**
   * A flag for nodes that are converted from their end tag equivalents.  For
   * example, </p> when no paragraph is open implies that the parser should
   * create a <p> tag and immediately close it, while </br> means the same thing
   * as <br>.
   */
  GUMBO_INSERTION_CONVERTED_FROM_END_TAG = 1 << 4,

  /** A flag for nodes that are converted from the parse of an <isindex> tag. */
  GUMBO_INSERTION_FROM_ISINDEX = 1 << 5,

  /** A flag for <image> tags that are rewritten as <img>. */
  GUMBO_INSERTION_FROM_IMAGE = 1 << 6,

  /**
   * A flag for nodes that are cloned as a result of the reconstruction of
   * active formatting elements.  This is set only on the clone; the initial
   * portion of the formatting run is a NORMAL node with an IMPLICIT_END_TAG.
   */
  GUMBO_INSERTION_RECONSTRUCTED_FORMATTING_ELEMENT = 1 << 7,

  /** A flag for nodes that are cloned by the adoption agency algorithm. */
  GUMBO_INSERTION_ADOPTION_AGENCY_CLONED = 1 << 8,

  /** A flag for nodes that are moved by the adoption agency algorithm. */
  GUMBO_INSERTION_ADOPTION_AGENCY_MOVED = 1 << 9,

  /**
   * A flag for nodes that have been foster-parented out of a table (or
   * should've been foster-parented, if verbatim mode is set).
   */
  GUMBO_INSERTION_FOSTER_PARENTED = 1 << 10,
} GumboParseFlags;


/**
 * Information specific to document nodes.
 */
typedef struct {
  /**
   * An array of GumboNodes, containing the children of this element.  This will
   * normally consist of the <html> element and any comment nodes found.
   * Pointers are owned.
   */
  GumboVector /* GumboNode* */ children;

  // True if there was an explicit doctype token as opposed to it being omitted.
  bool has_doctype;

  // Fields from the doctype token, copied verbatim.
  const char* name;
  const char* public_identifier;
  const char* system_identifier;

  /**
   * Whether or not the document is in QuirksMode, as determined by the values
   * in the GumboTokenDocType template.
   */
  GumboQuirksModeEnum doc_type_quirks_mode;
} GumboDocument;

/**
 * The struct used to represent TEXT, CDATA, COMMENT, and WHITESPACE elements.
 * This contains just a block of text and its position.
 */
typedef struct {
  /**
   * The text of this node, after entities have been parsed and decoded.  For
   * comment/cdata nodes, this does not include the comment delimiters.
   */
  const char* text;

  /**
   * The original text of this node, as a pointer into the original buffer.  For
   * comment/cdata nodes, this includes the comment delimiters.
   */
  GumboStringPiece original_text;

  /**
   * The starting position of this node.  This corresponds to the position of
   * original_text, before entities are decoded.
   * */
  GumboSourcePosition start_pos;
} GumboText;

/**
 * The struct used to represent all HTML elements.  This contains information
 * about the tag, attributes, and child nodes.
 */
typedef struct {
  /**
   * An array of GumboNodes, containing the children of this element.  Pointers
   * are owned.
   */
  GumboVector /* GumboNode* */ children;

  /** The GumboTag enum for this element. */
  GumboTag tag;

  /** The GumboNamespaceEnum for this element. */
  GumboNamespaceEnum tag_namespace;

  /**
   * A GumboStringPiece pointing to the original tag text for this element,
   * pointing directly into the source buffer.  If the tag was inserted
   * algorithmically (for example, <head> or <tbody> insertion), this will be a
   * zero-length string.
   */
  GumboStringPiece original_tag;

  /**
   * A GumboStringPiece pointing to the original end tag text for this element.
   * If the end tag was inserted algorithmically, (for example, closing a
   * self-closing tag), this will be a zero-length string.
   */
  GumboStringPiece original_end_tag;

  /** The source position for the start of the start tag. */
  GumboSourcePosition start_pos;

  /** The source position for the start of the end tag. */
  GumboSourcePosition end_pos;

  /**
   * An array of GumboAttributes, containing the attributes for this tag in the
   * order that they were parsed.  Pointers are owned.
   */
  GumboVector /* GumboAttribute* */ attributes;
} GumboElement;

/**
 * A supertype for GumboElement and GumboText, so that we can include one
 * generic type in lists of children and cast as necessary to subtypes.
 */
struct GumboInternalNode {
  /** The type of node that this is. */
  GumboNodeType type;

  /** Pointer back to parent node.  Not owned. */
  GumboNode* parent;

  /** The index within the parent's children vector of this node. */
  size_t index_within_parent;

  /**
   * A bitvector of flags containing information about why this element was
   * inserted into the parse tree, including a variety of special parse
   * situations.
   */
  GumboParseFlags parse_flags;

  /** The actual node data. */
  union {
    GumboDocument document;      // For GUMBO_NODE_DOCUMENT.
    GumboElement element;        // For GUMBO_NODE_ELEMENT.
    GumboText text;              // For everything else.
  } v;
};

/**
 * The type for an allocator function.  Takes the 'userdata' member of the
 * GumboParser struct as its first argument.  Semantics should be the same as
 * malloc, i.e. return a block of size_t bytes on success or NULL on failure.
 * Allocating a block of 0 bytes behaves as per malloc.
 */
// TODO(jdtang): Add checks throughout the codebase for out-of-memory condition.
typedef void* (*GumboAllocatorFunction)(void* userdata, size_t size);

/**
 * The type for a deallocator function.  Takes the 'userdata' member of the
 * GumboParser struct as its first argument.
 */
typedef void (*GumboDeallocatorFunction)(void* userdata, void* ptr);

/**
 * Input struct containing configuration options for the parser.
 * These let you specify alternate memory managers, provide different error
 * handling, etc.
 * Use kGumboDefaultOptions for sensible defaults, and only set what you need.
 */
typedef struct GumboInternalOptions {
  /** A memory allocator function.  Default: malloc. */
  GumboAllocatorFunction allocator;

  /** A memory deallocator function. Default: free. */
  GumboDeallocatorFunction deallocator;

  /**
   * An opaque object that's passed in as the first argument to all callbacks
   * used by this library.  Default: NULL.
   */
  void* userdata;

  /**
   * The tab-stop size, for computing positions in source code that uses tabs.
   * Default: 8.
   */
  int tab_stop;

  /**
   * Whether or not to stop parsing when the first error is encountered.
   * Default: false.
   */
  bool stop_on_first_error;

  /**
   * The maximum number of errors before the parser stops recording them.  This
   * is provided so that if the page is totally borked, we don't completely fill
   * up the errors vector and exhaust memory with useless redundant errors.  Set
   * to -1 to disable the limit.
   * Default: -1
   */
  int max_errors;
} GumboOptions;

/** Default options struct; use this with gumbo_parse_with_options. */
extern const GumboOptions kGumboDefaultOptions;

/** The output struct containing the results of the parse. */
typedef struct GumboInternalOutput {
  /**
   * Pointer to the document node.  This is a GumboNode of type NODE_DOCUMENT
   * that contains the entire document as its child.
   */
  GumboNode* document;

  /**
   * Pointer to the root node.  This the <html> tag that forms the root of the
   * document.
   */
  GumboNode* root;

  /**
   * A list of errors that occurred during the parse.
   * NOTE: In version 1.0 of this library, the API for errors hasn't been fully
   * fleshed out and may change in the future.  For this reason, the GumboError
   * header isn't part of the public API.  Contact us if you need errors
   * reported so we can work out something appropriate for your use-case.
   */
  GumboVector /* GumboError */ errors;
} GumboOutput;

/**
 * Parses a buffer of UTF8 text into an GumboNode parse tree.  The buffer must
 * live at least as long as the parse tree, as some fields (eg. original_text)
 * point directly into the original buffer.
 *
 * This doesn't support buffers longer than 4 gigabytes.
 */
GumboOutput* gumbo_parse(const char* buffer);

/**
 * Extended version of gumbo_parse that takes an explicit options structure,
 * buffer, and length.
 */
GumboOutput* gumbo_parse_with_options(
    const GumboOptions* options, const char* buffer, size_t buffer_length);

/** Release the memory used for the parse tree & parse errors. */
void gumbo_destroy_output(
    const GumboOptions* options, GumboOutput* output);


#ifdef __cplusplus
}
#endif

#endif  // GUMBO_GUMBO_H_
