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
// TODO(jdtang): Investigate whether there're efficiency benefits to putting the
// most common tag names first, or to putting them in alphabetical order and
// using a binary search.
const char* kGumboTagNames[] = {
  "html",
  "head",
  "title",
  "base",
  "link",
  "meta",
  "style",
  "script",
  "noscript",
  "template",
  "body",
  "article",
  "section",
  "nav",
  "aside",
  "h1",
  "h2",
  "h3",
  "h4",
  "h5",
  "h6",
  "hgroup",
  "header",
  "footer",
  "address",
  "p",
  "hr",
  "pre",
  "blockquote",
  "ol",
  "ul",
  "li",
  "dl",
  "dt",
  "dd",
  "figure",
  "figcaption",
  "main",
  "div",
  "a",
  "em",
  "strong",
  "small",
  "s",
  "cite",
  "q",
  "dfn",
  "abbr",
  "data",
  "time",
  "code",
  "var",
  "samp",
  "kbd",
  "sub",
  "sup",
  "i",
  "b",
  "u",
  "mark",
  "ruby",
  "rt",
  "rp",
  "bdi",
  "bdo",
  "span",
  "br",
  "wbr",
  "ins",
  "del",
  "image",
  "img",
  "iframe",
  "embed",
  "object",
  "param",
  "video",
  "audio",
  "source",
  "track",
  "canvas",
  "map",
  "area",
  "math",
  "mi",
  "mo",
  "mn",
  "ms",
  "mtext",
  "mglyph",
  "malignmark",
  "annotation-xml",
  "svg",
  "foreignobject",
  "desc",
  "table",
  "caption",
  "colgroup",
  "col",
  "tbody",
  "thead",
  "tfoot",
  "tr",
  "td",
  "th",
  "form",
  "fieldset",
  "legend",
  "label",
  "input",
  "button",
  "select",
  "datalist",
  "optgroup",
  "option",
  "textarea",
  "keygen",
  "output",
  "progress",
  "meter",
  "details",
  "summary",
  "menu",
  "menuitem",
  "applet",
  "acronym",
  "bgsound",
  "dir",
  "frame",
  "frameset",
  "noframes",
  "isindex",
  "listing",
  "xmp",
  "nextid",
  "noembed",
  "plaintext",
  "rb",
  "strike",
  "basefont",
  "big",
  "blink",
  "center",
  "font",
  "marquee",
  "multicol",
  "nobr",
  "spacer",
  "tt",
  "",                   // TAG_UNKNOWN
  "",                   // TAG_LAST
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

GumboTag gumbo_tag_enum(const char* tagname) {
  for (int i = 0; i < GUMBO_TAG_LAST; ++i) {
    // TODO(jdtang): strcasecmp is non-portable, so if we want to support
    // non-GCC compilers, we'll need some #ifdef magic.  This source already has
    // pretty significant issues with MSVC6 anyway.
    if (strcasecmp(tagname, kGumboTagNames[i]) == 0) {
      return i;
    }
  }
  return GUMBO_TAG_UNKNOWN;
}
