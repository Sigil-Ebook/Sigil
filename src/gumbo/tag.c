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


GumboTag gumbo_tag_enum(const char* tagname) {
  int i = binsearch(tagname, kGumboTagNames, GUMBO_TAG_UNKNOWN);
  if (i < 0) {
    return GUMBO_TAG_UNKNOWN;
  }
  return (GumboTag) i;
}
