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

#include "attribute.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "util.h"
#include "vector.h"

struct GumboInternalParser;

GumboAttribute* gumbo_get_attribute(
    const GumboVector* attributes, const char* name) {
  for (int i = 0; i < attributes->length; ++i) {
    GumboAttribute* attr = attributes->data[i];
    if (!strcasecmp(attr->name, name)) {
      return attr;
    }
  }
  return NULL;
}

void gumbo_attribute_set_value(GumboAttribute *attr, const char *value)
{
  gumbo_free((void *)attr->value);
  attr->value = gumbo_strdup(value);
  attr->original_value = kGumboEmptyString;
  attr->value_start = kGumboEmptySourcePosition;
  attr->value_end = kGumboEmptySourcePosition;
}

void gumbo_destroy_attribute(GumboAttribute* attribute) {
  gumbo_free((void*) attribute->name);
  gumbo_free((void*) attribute->value);
  gumbo_free((void*) attribute);
}

void gumbo_element_set_attribute(
    GumboElement *element, const char *name, const char *value)
{
  GumboVector *attributes = &element->attributes;
  GumboAttribute *attr = gumbo_get_attribute(attributes, name);

  if (!attr) {
    attr = gumbo_malloc(sizeof(GumboAttribute));
    attr->value = NULL;
    attr->attr_namespace = GUMBO_ATTR_NAMESPACE_NONE;

    attr->name = gumbo_strdup(name);
    attr->original_name = kGumboEmptyString;
    attr->name_start = kGumboEmptySourcePosition;
    attr->name_end = kGumboEmptySourcePosition;

    gumbo_vector_add(attr, attributes);
  }

  gumbo_attribute_set_value(attr, value);
}

void gumbo_element_remove_attribute_at(GumboElement *element, unsigned int pos) {
  GumboAttribute *attr = element->attributes.data[pos];
  gumbo_vector_remove_at(pos, &element->attributes);
  gumbo_destroy_attribute(attr);
}

void gumbo_element_remove_attribute(GumboElement *element, GumboAttribute *attr) {
  int idx = gumbo_vector_index_of(&element->attributes, attr);
  if (idx >= 0) {
    gumbo_vector_remove_at(idx, &element->attributes);
    gumbo_destroy_attribute(attr);
  }
}
