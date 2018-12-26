#ifndef GUMBO_REPLACEMENT_H_
#define GUMBO_REPLACEMENT_H_

#pragma once

#include <stddef.h>
#include "gumbo.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  const char *const from;
  const char *const to;
} StringReplacement;

const StringReplacement *gumbo_get_svg_tag_replacement(
  const char* str,
  size_t len
);

const StringReplacement *gumbo_get_svg_attr_replacement(
  const char* str,
  size_t len
);

typedef struct {
  const char *const from;
  const char *const local_name;
  const GumboAttributeNamespaceEnum attr_namespace;
} ForeignAttrReplacement;

const ForeignAttrReplacement *gumbo_get_foreign_attr_replacement (
  const char* str,
  size_t len
);

#ifdef __cplusplus
}
#endif

#endif // GUMBO_REPLACEMENT_H_
