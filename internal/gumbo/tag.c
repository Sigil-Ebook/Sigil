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
#include "util.h"

#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <strings.h>    // For strcasecmp.
#include <string.h>    // For strcasecmp.

const char* kGumboTagNames[] = {
# include "tag_strings.h"
  "",                   // TAG_UNKNOWN
  "",                   // TAG_LAST
};

static const uint8_t kGumboTagSizes[] = {
# include "tag_sizes.h"
  0, // TAG_UNKNOWN
  0, // TAG_LAST
};

const char* gumbo_normalized_tagname(GumboTag tag) {
  assert(tag <= GUMBO_TAG_LAST);
  return kGumboTagNames[tag];
}

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

/*
 * Override the `tolower` implementation in the perfect hash
 * to use ours. We need a custom `tolower` that only does ASCII
 * characters and is locale-independent to remain truthy to the
 * standard
 */
#define perfhash_tolower(c) gumbo_tolower(c)
#include "tag_perf.h"

static int
case_memcmp(const char *s1, const char *s2, int n)
{
	while (n--) {
		unsigned char c1 = gumbo_tolower(*s1++);
		unsigned char c2 = gumbo_tolower(*s2++);
		if (c1 != c2)
			return (int)c1 - (int)c2;
	}
	return 0;
}


GumboTag gumbo_tagn_enum(const char* tagname, int length) {
  int position = perfhash((const unsigned char *)tagname, length);
  if (position >= 0 &&
      length == kGumboTagSizes[position] &&
      !case_memcmp(tagname, kGumboTagNames[position], length))
    return (GumboTag)position;
  return GUMBO_TAG_UNKNOWN;
}


#if 0
/**
 * This version removes unrecognized svg and mathml prefixes from
 * tags to force the gumbo parser to actually recognize that svg:svg is 
 * actually an svg tag and similarly for m:math and mml:math and even math:math.
 * Without it gumbo treats these as unknown tags in the html namespace 
 * and not to the correct svg or mathml namespaces 
 **/
GumboTag gumbo_tagn_enum(const char* tagname, int length) {
  /* handle replacement of standard prefixes */
  const char * tagnameptr;
  int tagnamelength;
  int position = -1;
  if (!case_memcmp(tagname, "svg:", 4)) {
    tagnameptr = tagname + 4;
    tagnamelength = length - 4;
  } else if (!case_memcmp(tagname, "m:", 2)) {
    tagnameptr = tagname + 2;
    tagnamelength = length - 2;
  } else if (!case_memcmp(tagname, "mml:", 4)) {
    tagnameptr = tagname + 4;
    tagnamelength = length - 4;
  } else if (!case_memcmp(tagname, "math:", 5)) {
    tagnameptr = tagname + 5;
    tagnamelength = length - 5;
  } else {
    tagnameptr = tagname;
    tagnamelength = length;
  }
  position = perfhash((const unsigned char *)tagnameptr, tagnamelength);
  if (position >= 0 &&
      tagnamelength == kGumboTagSizes[position] &&
      !case_memcmp(tagnameptr, kGumboTagNames[position], tagnamelength))
    return (GumboTag)position;
  return GUMBO_TAG_UNKNOWN;
}
#endif


GumboTag gumbo_tag_enum(const char* tagname) {
  return gumbo_tagn_enum(tagname, strlen(tagname));
}
