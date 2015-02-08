// Copyright 2011 Google Inc. All Rights Reserved.
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

#include "gumbo.h"
#include "tag.h"

#include <assert.h>
#include <ctype.h>
#include <strings.h>    // For strcasecmp.


// NOTE(jdtang): Keep this in sync with the GumboTag enum in the header.
// keep sorted so that a binary search will always work

const char* kGumboTagNames[] = {
  "a",
  "abbr",
  "acronym",
  "address",
  "altGlyph",
  "altGlyphDef",
  "altGlyphItem",
  "animate",
  "animateColor",
  "animateMotion",
  "animateTransform",
  "annotation-xml",
  "applet",
  "area",
  "article",
  "aside",
  "audio",
  "b",
  "base",
  "basefont",
  "bdi",
  "bdo",
  "bgsound",
  "big",
  "blink",
  "blockquote",
  "body",
  "br",
  "button",
  "canvas",
  "caption",
  "center",
  "circle",
  "cite",
  "clippath",
  "code",
  "col",
  "colgroup",
  "color-profile",
  "cursor",
  "data",
  "datalist",
  "dd",
  "defs",
  "del",
  "desc",
  "details",
  "dfn",
  "dir",
  "div",
  "dl",
  "dt",
  "ellipse",
  "em",
  "embed",
  "feBlend",
  "feColorMatrix",
  "feComponentTransfer",
  "feComposite",
  "feConvolveMatrix",
  "feDiffuseLighting",
  "feDisplacementMap",
  "feDistantLight",
  "feFlood",
  "feFuncA",
  "feFuncB",
  "feFuncG",
  "feFuncR",
  "feGaussianBlur",
  "feImage",
  "feMerge",
  "feMergeNode",
  "feMorphology",
  "feOffset",
  "fePointLight",
  "feSpectactualrLighting",
  "feSpotLight",
  "feTile",
  "feTurbulence",
  "fieldset",
  "figcaption",
  "figure",
  "filter",
  "font",
  "font-face",
  "font-face-format",
  "font-face-name",
  "font-face-src",
  "font-face-uri",
  "footer",
  "foreignObject",
  "form",
  "frame",
  "frameset",
  "g",
  "glyph",
  "glyphRef",
  "h1",
  "h2",
  "h3",
  "h4",
  "h5",
  "h6",
  "head",
  "header",
  "hgroup",
  "hkern",
  "hr",
  "html",
  "i",
  "iframe",
  "image",
  "img",
  "input",
  "ins",
  "isindex",
  "kbd",
  "keygen",
  "label",
  "legend",
  "li",
  "line",
  "linearGradient",
  "link",
  "listing",
  "maction",
  "main",
  "maligngroup",
  "malignmark",
  "map",
  "mark",
  "marker",
  "marquee",
  "mask",
  "math",
  "menclose",
  "menu",
  "menuitem",
  "merror",
  "meta",
  "metadata",
  "meter",
  "mfenced",
  "mfrac",
  "mglyph",
  "mi",
  "missing-glyph",
  "mlabeledtr",
  "mlongdiv",
  "mmultiscripts",
  "mn",
  "mo",
  "mover",
  "mpadded",
  "mpath",
  "mphantom",
  "mprescripts",
  "mroot",
  "mrow",
  "ms",
  "mscarries",
  "mscarry",
  "msgroup",
  "msline",
  "mspace",
  "msqrt",
  "msrow",
  "mstack",
  "mstyle",
  "msub",
  "msubsup",
  "msup",
  "mtable",
  "mtd",
  "mtext",
  "mtr",
  "multicol",
  "munder",
  "munderover",
  "nav",
  "nextid",
  "nobr",
  "noembed",
  "noframes",
  "none",
  "noscript",
  "object",
  "ol",
  "optgroup",
  "option",
  "output",
  "p",
  "param",
  "path",
  "pattern",
  "plaintext",
  "polygon",
  "polyline",
  "pre",
  "progress",
  "q",
  "radialGradient",
  "rb",
  "rect",
  "rp",
  "rt",
  "ruby",
  "s",
  "samp",
  "script",
  "section",
  "select",
  "semantics",
  "set",
  "small",
  "source",
  "spacer",
  "span",
  "stop",
  "strike",
  "strong",
  "style",
  "sub",
  "summary",
  "sup",
  "svg",
  "switch",
  "symbol",
  "table",
  "tbody",
  "td",
  "template",
  "text",
  "textPath",
  "textarea",
  "tfoot",
  "th",
  "thead",
  "time",
  "title",
  "tr",
  "track",
  "tref",
  "tspan",
  "tt",
  "u",
  "ul",
  "use",
  "var",
  "video",
  "view",
  "vkern",
  "wbr",
  "xmp",
  "",
};


#define kGumboMathMLTagsLength 43

static const int kGumboMathMLTags[] = {
  GUMBO_TAG_ANNOTATION_XML,
  GUMBO_TAG_MACTION,
  GUMBO_TAG_MALIGNGROUP,
  GUMBO_TAG_MALIGNMARK,
  GUMBO_TAG_MATH,
  GUMBO_TAG_MENCLOSE,
  GUMBO_TAG_MERROR,
  GUMBO_TAG_MFENCED,
  GUMBO_TAG_MFRAC,
  GUMBO_TAG_MGLYPH,
  GUMBO_TAG_MI,
  GUMBO_TAG_MLABELEDTR,
  GUMBO_TAG_MLONGDIV,
  GUMBO_TAG_MMULTISCRIPTS,
  GUMBO_TAG_MN,
  GUMBO_TAG_MO,
  GUMBO_TAG_MOVER,
  GUMBO_TAG_MPADDED,
  GUMBO_TAG_MPHANTOM,
  GUMBO_TAG_MPRESCRIPTS,
  GUMBO_TAG_MROOT,
  GUMBO_TAG_MROW,
  GUMBO_TAG_MS,
  GUMBO_TAG_MSCARRIES,
  GUMBO_TAG_MSCARRY,
  GUMBO_TAG_MSGROUP,
  GUMBO_TAG_MSLINE,
  GUMBO_TAG_MSPACE,
  GUMBO_TAG_MSQRT,
  GUMBO_TAG_MSROW,
  GUMBO_TAG_MSTACK,
  GUMBO_TAG_MSTYLE,
  GUMBO_TAG_MSUB,
  GUMBO_TAG_MSUBSUP,
  GUMBO_TAG_MSUP,
  GUMBO_TAG_MTABLE,
  GUMBO_TAG_MTD,
  GUMBO_TAG_MTEXT,
  GUMBO_TAG_MTR,
  GUMBO_TAG_MUNDER,
  GUMBO_TAG_MUNDEROVER,
  GUMBO_TAG_NONE,
  GUMBO_TAG_SEMANTICS
};


// these 5 tags are valid in both the html and svg namespaces
#define kGumboSharedTagsLength 5

static const int kGumboSharedTags[] = {
  GUMBO_TAG_A,
  GUMBO_TAG_FONT,
  GUMBO_TAG_SCRIPT,
  GUMBO_TAG_STYLE,
  GUMBO_TAG_TITLE
};


#define kGumboSVGTagsLength 80

static const int kGumboSVGTags[] = {
  GUMBO_TAG_A,
  GUMBO_TAG_ALTGLYPH,
  GUMBO_TAG_ALTGLYPHDEF,
  GUMBO_TAG_ALTGLYPHITEM,
  GUMBO_TAG_ANIMATE,
  GUMBO_TAG_ANIMATECOLOR,
  GUMBO_TAG_ANIMATEMOTION,
  GUMBO_TAG_ANIMATETRANSFORM,
  GUMBO_TAG_CIRCLE,
  GUMBO_TAG_CLIPPATH,
  GUMBO_TAG_COLOR_PROFILE,
  GUMBO_TAG_CURSOR,
  GUMBO_TAG_DEFS,
  GUMBO_TAG_DESC,
  GUMBO_TAG_ELLIPSE,
  GUMBO_TAG_FEBLEND,
  GUMBO_TAG_FECOLORMATRIX,
  GUMBO_TAG_FECOMPONENTTRANSFER,
  GUMBO_TAG_FECOMPOSITE,
  GUMBO_TAG_FECONVOLVEMATRIX,
  GUMBO_TAG_FEDIFFUSELIGHTING,
  GUMBO_TAG_FEDISPLACEMENTMAP,
  GUMBO_TAG_FEDISTANTLIGHT,
  GUMBO_TAG_FEFLOOD,
  GUMBO_TAG_FEFUNCA,
  GUMBO_TAG_FEFUNCB,
  GUMBO_TAG_FEFUNCG,
  GUMBO_TAG_FEFUNCR,
  GUMBO_TAG_FEGAUSSIANBLUR,
  GUMBO_TAG_FEIMAGE,
  GUMBO_TAG_FEMERGE,
  GUMBO_TAG_FEMERGENODE,
  GUMBO_TAG_FEMORPHOLOGY,
  GUMBO_TAG_FEOFFSET,
  GUMBO_TAG_FEPOINTLIGHT,
  GUMBO_TAG_FESPECTACTUALRLIGHTING,
  GUMBO_TAG_FESPOTLIGHT,
  GUMBO_TAG_FETILE,
  GUMBO_TAG_FETURBULENCE,
  GUMBO_TAG_FILTER,
  GUMBO_TAG_FONT,
  GUMBO_TAG_FONT_FACE,
  GUMBO_TAG_FONT_FACE_NAME,
  GUMBO_TAG_FONT_FACE_SRC,
  GUMBO_TAG_FONT_FACE_URI,
  GUMBO_TAG_FONT_FACE_FORMAT,
  GUMBO_TAG_FOREIGNOBJECT,
  GUMBO_TAG_G,
  GUMBO_TAG_GLYPH,
  GUMBO_TAG_GLYPHREF,
  GUMBO_TAG_HKERN,
  GUMBO_TAG_IMAGE,
  GUMBO_TAG_LINE,
  GUMBO_TAG_LINEARGRADIENT,
  GUMBO_TAG_MARKER,
  GUMBO_TAG_MASK,
  GUMBO_TAG_METADATA,
  GUMBO_TAG_MISSING_GLYPH,
  GUMBO_TAG_MPATH,
  GUMBO_TAG_PATH,
  GUMBO_TAG_PATTERN,
  GUMBO_TAG_POLYGON,
  GUMBO_TAG_POLYLINE,
  GUMBO_TAG_RADIALGRADIENT,
  GUMBO_TAG_RECT,
  GUMBO_TAG_SCRIPT,
  GUMBO_TAG_SET,
  GUMBO_TAG_STOP,
  GUMBO_TAG_STYLE,
  GUMBO_TAG_SVG,
  GUMBO_TAG_SWITCH,
  GUMBO_TAG_SYMBOL,
  GUMBO_TAG_TEXT,
  GUMBO_TAG_TEXTPATH,
  GUMBO_TAG_TITLE,
  GUMBO_TAG_TREF,
  GUMBO_TAG_TSPAN,
  GUMBO_TAG_USE,
  GUMBO_TAG_VIEW,
  GUMBO_TAG_VKERN
};


const char* gumbo_normalized_tagname(GumboTag tag) {
  assert(tag <= GUMBO_TAG_LAST);
  return kGumboTagNames[tag];
}

// TODO(jdtang): Add test for this.
void gumbo_tag_from_original_text(GumboStringPiece* text) {
  if (text->data == NULL) {
    return;
  }
  assert(text->length >= 2);
  assert(text->data[0] == '<');
  assert(text->data[text->length - 1] == '>');
  if (text->data[1] == '/') {
    // End tag.
    assert(text->length >= 3);
    text->data += 2;    // Move past </
    text->length -= 3;
  } else {
    // Start tag.
    text->data += 1;    // Move past <
    text->length -= 2;
    // strnchr is apparently not a standard C library function, so I loop
    // explicitly looking for whitespace or other illegal tag characters.
    for (const char* c = text->data; c != text->data + text->length; ++c) {
      if (isspace(*c) || *c == '/') {
        text->length = c - text->data;
        break;
      }
    }
  }
}


//  performs a binary search on case insensitive null terminated character strings,
//  looks for string sw in the provided list
//  returns: -1 on not found or the index of entry in the list[]
static int binsearch(const char * sw, const char* list[], int nlst) 
{
    int lp, up, mp, j, indx;
    lp = 0;
    up = nlst-1;
    indx = -1;
    if (strcasecmp(sw,list[lp]) < 0) return -1;
    if (strcasecmp(sw,list[up]) > 0) return -1;
    while (indx < 0 ) {
        mp = (int)((lp+up) >> 1);
        j = strcmp(sw,list[mp]);
        if ( j > 0) {
            lp = mp + 1;
        } else if (j < 0 ) {
            up = mp - 1;
        } else {
            indx = mp;
        }
        if (lp > up) return -1;      
    }
    return indx;
}


bool gumbo_int_in_sorted_list(int tag, const int* taglist, int nlst)
{
    int lp, up, mp, indx;
    lp = 0;
    up = nlst-1;
    indx = -1;
    if ((int) tag < taglist[lp]) return false;
    if ((int) tag > taglist[up]) return false;
    while (indx < 0 ) {
        mp = (int)((lp+up) >> 1);
        if ((int) tag > taglist[mp]) {
          lp = mp + 1;
        } else if ((int) tag < taglist[mp]) {
          up = mp - 1;
        } else {
          indx = mp;
        }
        if (lp > up) return false;      
    }
    return true;
}


GumboTag gumbo_tag_enum(const char* tagname) {
  int i = binsearch(tagname, kGumboTagNames, GUMBO_TAG_UNKNOWN);
  if (i < 0) {
    return GUMBO_TAG_UNKNOWN;
  }
  return (GumboTag) i;
}


bool gumbo_can_be_svg_tag(GumboTag tag)
{
  return gumbo_int_in_sorted_list( (int)tag, kGumboSVGTags, kGumboSVGTagsLength); 
}


bool gumbo_can_be_mathml_tag(GumboTag tag)
{
  return gumbo_int_in_sorted_list( (int)tag, kGumboMathMLTags, kGumboMathMLTagsLength); 
}


bool gumbo_can_be_shared_tag(GumboTag tag)
{
  return gumbo_int_in_sorted_list( (int)tag, kGumboSharedTags, kGumboSharedTagsLength); 
}


