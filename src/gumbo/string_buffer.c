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

#include "string_buffer.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdarg.h>

#include "string_piece.h"
#include "util.h"

static const size_t kDefaultStringBufferSize = 10;

static void maybe_resize_string_buffer(size_t additional_chars, GumboStringBuffer* buffer) {
  size_t new_length = buffer->length + additional_chars;
  size_t new_capacity = buffer->capacity;
  while (new_capacity < new_length) {
    new_capacity *= 2;
  }
  if (new_capacity != buffer->capacity) {
    buffer->capacity = new_capacity;
    buffer->data = gumbo_realloc(buffer->data, buffer->capacity);
  }
}

void gumbo_string_buffer_init(GumboStringBuffer* output) {
  output->data = gumbo_malloc(kDefaultStringBufferSize);
  output->length = 0;
  output->capacity = kDefaultStringBufferSize;
}

void gumbo_string_buffer_reserve(size_t min_capacity, GumboStringBuffer* output) {
  maybe_resize_string_buffer(min_capacity - output->length, output);
}

void gumbo_string_buffer_append_codepoint(int c, GumboStringBuffer* output) {
  // num_bytes is actually the number of continuation bytes, 1 less than the
  // total number of bytes.  This is done to keep the loop below simple and
  // should probably change if we unroll it.
  int num_bytes, prefix;
  if (c <= 0x7f) {
    num_bytes = 0;
    prefix = 0;
  } else if (c <= 0x7ff) {
    num_bytes = 1;
    prefix = 0xc0;
  } else if (c <= 0xffff) {
    num_bytes = 2;
    prefix = 0xe0;
  } else {
    num_bytes = 3;
    prefix = 0xf0;
  }
  maybe_resize_string_buffer(num_bytes + 1, output);
  output->data[output->length++] = prefix | (c >> (num_bytes * 6));
  for (int i = num_bytes - 1; i >= 0; --i) {
    output->data[output->length++] = 0x80 | (0x3f & (c >> (i * 6)));
  }
}

void gumbo_string_buffer_put(GumboStringBuffer *buffer,
  const char *data, size_t length)
{
  maybe_resize_string_buffer(length, buffer);
  memcpy(buffer->data + buffer->length, data, length);
  buffer->length += length;
}

void gumbo_string_buffer_putv(GumboStringBuffer *buffer, int count, ...)
{
  va_list ap;
  int i;
  size_t total_len = 0;

  va_start(ap, count);
  for (i = 0; i < count; ++i)
    total_len += strlen(va_arg(ap, const char *));
  va_end(ap);

  maybe_resize_string_buffer(total_len, buffer);

  va_start(ap, count);
  for (i = 0; i < count; ++i) {
    const char *data = va_arg(ap, const char *);
    size_t length = strlen(data);

    memcpy(buffer->data + buffer->length, data, length);
    buffer->length += length;
  }
  va_end(ap);
}

void gumbo_string_buffer_append_string(GumboStringPiece* str,
    GumboStringBuffer* output) {
  gumbo_string_buffer_put(output, str->data, str->length);
}

const char* gumbo_string_buffer_cstr(GumboStringBuffer *buffer) {
  maybe_resize_string_buffer(1, buffer);
  /* do not increase length of the string */
  buffer->data[buffer->length] = 0;
  return buffer->data;
}

char* gumbo_string_buffer_to_string(GumboStringBuffer* input) {
  char* buffer = gumbo_malloc(input->length + 1);
  memcpy(buffer, input->data, input->length);
  buffer[input->length] = '\0';
  return buffer;
}

void gumbo_string_buffer_destroy(GumboStringBuffer* buffer) {
  gumbo_free(buffer->data);
}
