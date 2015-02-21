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

#include "vector.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "util.h"

struct GumboInternalParser;

const GumboVector kGumboEmptyVector = { NULL, 0, 0 };

void gumbo_vector_init(size_t initial_capacity, GumboVector* vector) {
  vector->length = 0;
  vector->capacity = initial_capacity;
  vector->data = NULL;
  if (initial_capacity)
    vector->data = gumbo_malloc(sizeof(void*) * initial_capacity);
}

void gumbo_vector_destroy(GumboVector* vector) {
  gumbo_free(vector->data);
}

static void enlarge_vector_if_full(GumboVector* vector, int space) {
  unsigned int new_length = vector->length + space;
  unsigned int new_capacity = vector->capacity;

  if (!new_capacity)
    new_capacity = 2;

  while (new_capacity < new_length)
    new_capacity *= 2;

  if (new_capacity != vector->capacity) {
    vector->capacity = new_capacity;
    vector->data = gumbo_realloc(vector->data,
        sizeof(void *) * vector->capacity);
  }
}

void gumbo_vector_add(void* element, GumboVector* vector) {
  enlarge_vector_if_full(vector, 1);
  assert(vector->data);
  assert(vector->length < vector->capacity);
  vector->data[vector->length++] = element;
}

void* gumbo_vector_pop(GumboVector* vector) {
  if (vector->length == 0) {
    return NULL;
  }
  return vector->data[--vector->length];
}

int gumbo_vector_index_of(GumboVector* vector, const void* element) {
  for (int i = 0; i < vector->length; ++i) {
    if (vector->data[i] == element) {
      return i;
    }
  }
  return -1;
}

void gumbo_vector_insert_at(void* element, int index, GumboVector* vector) {
  assert(index >= 0);
  assert(index <= vector->length);
  enlarge_vector_if_full(vector, 1);
  ++vector->length;
  memmove(&vector->data[index + 1], &vector->data[index],
      sizeof(void*) * (vector->length - index - 1));
  vector->data[index] = element;
}

void gumbo_vector_splice(int where, int n_to_remove,
    void **data, int n_to_insert,
    GumboVector* vector) {
  enlarge_vector_if_full(vector, n_to_insert - n_to_remove);
  memmove(vector->data + where + n_to_insert,
      vector->data + where + n_to_remove,
      sizeof(void *) * (vector->length - where - n_to_remove));
  memcpy(vector->data + where, data, sizeof(void *) * n_to_insert);
  vector->length = vector->length + n_to_insert - n_to_remove;
}

void gumbo_vector_remove(const void* node, GumboVector* vector) {
  int index = gumbo_vector_index_of(vector, node);
  if (index == -1) {
    return;
  }
  gumbo_vector_remove_at(index, vector);
}

void* gumbo_vector_remove_at(int index, GumboVector* vector) {
  assert(index >= 0);
  assert(index < vector->length);
  void* result = vector->data[index];
  memmove(&vector->data[index], &vector->data[index + 1],
      sizeof(void*) * (vector->length - index - 1));
  --vector->length;
  return result;
}
