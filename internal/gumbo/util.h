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
// This contains some utility functions that didn't fit into any of the other
// headers.

#ifndef GUMBO_UTIL_H_
#define GUMBO_UTIL_H_
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void *(* gumbo_user_allocator)(void *, size_t);
extern void (* gumbo_user_free)(void *);

static inline void *gumbo_malloc(size_t size)
{
  return gumbo_user_allocator(NULL, size);
}

static inline void *gumbo_realloc(void *ptr, size_t size)
{
  return gumbo_user_allocator(ptr, size);
}

static inline char *gumbo_strdup(const char *str)
{
  size_t len = strlen(str) + 1;
  char *copy = (char *)gumbo_malloc(len);
  memcpy(copy, str, len);
  return copy;
}

static inline void gumbo_free(void *ptr)
{
  gumbo_user_free(ptr);
}

static inline int gumbo_tolower(int c)
{
  return c | ((c >= 'A' && c <= 'Z') << 5);
}

static inline bool gumbo_isalpha(int c)
{
  return (c | 0x20) >= 'a' && (c | 0x20) <= 'z';
}

// Debug wrapper for printf, to make it easier to turn off debugging info when
// required.
void gumbo_debug(const char* format, ...);

#ifdef __cplusplus
}
#endif

#endif  // GUMBO_UTIL_H_
