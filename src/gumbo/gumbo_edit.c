// Copyright 2015 Kevin B. Hendricks, Stratford Ontario  All Rights Reserved.
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

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "attribute.h"
#include "vector.h"
#include "gumbo.h"
#include "utf8.h"
#include "util.h"


/* main interface routine for editing in gumbo */
/**
 * const GumboVector kGumboEmptyVector = { NULL, 0, 0 };
 * const GumboSourcePosition kGumboEmptySourcePosition = { 0, 0, 0 };

 * void gumbo_attribute_set_value(GumboAttribute *attr, const char *value);
 * void gumbo_destroy_attribute(GumboAttribute* attribute);
 * void gumbo_element_set_attribute(GumboElement *element, const char *name, const char *value);
 * void gumbo_element_remove_attribute_at(GumboElement *element, unsigned int pos);
 * void gumbo_element_remove_attribute(GumboElement *element, GumboAttribute *attr);

 * void gumbo_vector_init(size_t initial_capacity, GumboVector* vector);
 * void gumbo_vector_destroy(GumboVector* vector);
 * void gumbo_vector_add(void* element, GumboVector* vector);
 * void* gumbo_vector_pop(GumboVector* vector);
 * void gumbo_vector_insert_at(void* element, int index, GumboVector* vector);
 * void gumbo_vector_remove(const void* element, GumboVector* vector);
 * void* gumbo_vector_remove_at(int index, GumboVector* vector);
 * int gumbo_vector_index_of(GumboVector* vector, const void* element);
 * void gumbo_vector_splice(int where, int n_to_remove, void **data, int n_to_insert, GumboVector* vector);

 * GumboTag gumbo_tag_enum(const char* tagname);
 * GumboTag gumbo_tagn_enum(const char* tagname, int length);

 * extern GumboNode *gumbo_create_node(GumboNodeType type);
 * extern void gumbo_destroy_node(GumboNode *node);
 */


// used internally by gumbo_new_output init 
static GumboNode* gumbo_new_document_node(void) {
  GumboNode* document_node = gumbo_create_node(GUMBO_NODE_DOCUMENT);
  gumbo_vector_init(1, &document_node->v.document.children);
  GumboDocument* document = &document_node->v.document;
  document->has_doctype = false;
  document->name = NULL;
  document->public_identifier = NULL;
  document->system_identifier = NULL;
  return document_node;
}


// create and initialize a completely new tree output area
GumboOutput*  gumbo_new_output_init(void) {
  GumboOutput* output = gumbo_malloc(sizeof(GumboOutput));
  output->root = NULL;
  output->document = gumbo_new_document_node();
  gumbo_vector_init(0, &output->errors);
  return output;
}


// Creates an text node of specified type and returns it.
// Types are GUMBO_NODE_TEXT, GUMBO_NODE_WHITESPACE, GUMBO_NODE_CDATA, and GUMBO_NODE_COMMENT
// No entities are allowed (replace them with their utf-8 character equivalents)
// Note: CDATA and COMMENTS text should NOT include their respective delimiters
// ie. No <-- --> and not CDATA[[ and ]]
GumboNode*  gumbo_create_text_node(GumboNode* node, GumboNodeType type, const char * text) {
  assert(type != GUMBO_NODE_DOCUMENT);
  assert(type != GUMBO_NODE_TEMPLATE);
  assert(type != GUMBO_NODE_ELEMENT);
  GumboNode* textnode = gumbo_create_node(type);
  textnode->type = GUMBO_NODE_COMMENT;
  textnode->parse_flags = GUMBO_INSERTION_NORMAL;
  textnode->v.text.text = gumbo_strdup(text);
  return textnode;
}


// Creates an element node with the tag (enum) in the specified namespace and returns it.
// Since no original text exists, any created element tag must already exist in the tag_enum.h
// This is why we have expanded the set of recognized tags to include all svg and mathml tags 
GumboNode* gumbo_create_element_node(GumboTag tag, GumboNamespaceEnum namespace) {
  GumboNode* node = gumbo_create_node(GUMBO_NODE_ELEMENT);
  GumboElement* element = &node->v.element;
  gumbo_vector_init(1, &element->children);
  gumbo_vector_init(0, &element->attributes);
  element->tag = tag;
  element->tag_namespace = namespace;
  element->original_tag = kGumboEmptyString;
  element->original_end_tag = kGumboEmptyString;
  element->start_pos = kGumboEmptySourcePosition;
  element->end_pos = kGumboEmptySourcePosition;
  return node;
}


// Creates an template node and returns it.
GumboNode* gumbo_create_template_node() {
  GumboNode* node = gumbo_create_node(GUMBO_NODE_TEMPLATE);
  GumboElement* element = &node->v.element;
  gumbo_vector_init(1, &element->children);
  gumbo_vector_init(0, &element->attributes);
  element->tag = GUMBO_TAG_TEMPLATE;
  element->tag_namespace = GUMBO_NAMESPACE_HTML;
  element->original_tag = kGumboEmptyString;
  element->original_end_tag = kGumboEmptyString;
  element->start_pos = kGumboEmptySourcePosition;
  element->end_pos = kGumboEmptySourcePosition;
  return node;
}


// Appends a node to the end of its parent, setting the "parent" and
// "index_within_parent" fields appropriately.
void gumbo_append_node(GumboNode* parent, GumboNode* node) {
  assert(node->parent == NULL);
  assert(node->index_within_parent == -1);
  GumboVector* children;
  if (parent->type == GUMBO_NODE_ELEMENT || parent->type == GUMBO_NODE_TEMPLATE) {
    children = &parent->v.element.children;
  } else {
    assert(parent->type == GUMBO_NODE_DOCUMENT);
    children = &parent->v.document.children;
  }
  node->parent = parent;
  node->index_within_parent = children->length;
  gumbo_vector_add((void*) node, children);
  assert(node->index_within_parent < children->length);
}


// Inserts a node at the specified index in the specified parent, 
// updating the "parent" and "index_within_parent" fields of it and all its siblings.
// If the index is -1, this simply calls gumbo_append_node.
void gumbo_insert_node(GumboNode* node, GumboNode* target_parent, int target_index) {
  assert(node->parent == NULL);
  assert(node->index_within_parent == -1);
  GumboNode* parent = target_parent;
  int index = target_index;
  if (index != -1) {
    GumboVector* children = NULL;
    if (parent->type == GUMBO_NODE_ELEMENT ||
        parent->type == GUMBO_NODE_TEMPLATE) {
      children = &parent->v.element.children;
    } else if (parent->type == GUMBO_NODE_DOCUMENT) {
      children = &parent->v.document.children;
    } else {
      assert(0);
    }
    assert(index >= 0);
    assert(index < children->length);
    node->parent = parent;
    node->index_within_parent = index;
    gumbo_vector_insert_at((void*) node, index, children);
    assert(node->index_within_parent < children->length);
    for (int i = index + 1; i < children->length; ++i) {
      GumboNode* sibling = children->data[i];
      sibling->index_within_parent = i;
      assert(sibling->index_within_parent < children->length);
    }
  } else {
    gumbo_append_node(parent, node);
  }
}


void gumbo_remove_from_parent(GumboNode* node) {
  if (!node->parent) {
    return;
  }
  assert(node->parent->type == GUMBO_NODE_ELEMENT || 
         node->parent->type == GUMBO_NODE_TEMPLATE ||
         node->parent->type == GUMBO_NODE_DOCUMENT);
  GumboVector* children = &node->parent->v.element.children;
  if (&node->parent->type == GUMBO_NODE_DOCUMENT) {
      children = &node->parent->v.document.children;
  }
  int index = gumbo_vector_index_of(children, node);
  assert(index != -1);
  gumbo_vector_remove_at(index, children);
  node->parent = NULL;
  node->index_within_parent = -1;
  for (int i = index; i < children->length; ++i) {
    GumboNode* child = children->data[i];
    child->index_within_parent = i;
  }
}


// Clones attributes, tags, etc. of a node, but does not copy the content (its children).  
// The clone shares no structure with the original node: all owned strings and
// values are fresh copies.
GumboNode* clone_element_node(const GumboNode* node) {
  assert(node->type == GUMBO_NODE_ELEMENT || node->type == GUMBO_NODE_TEMPLATE);
  GumboNode* new_node = gumbo_malloc(sizeof(GumboNode));
  *new_node = *node;
  new_node->parent = NULL;
  new_node->index_within_parent = -1;
  GumboElement* element = &new_node->v.element;
  gumbo_vector_init(1, &element->children);
  const GumboVector* old_attributes = &node->v.element.attributes;
  gumbo_vector_init(old_attributes->length, &element->attributes);
  for (int i = 0; i < old_attributes->length; ++i) {
    const GumboAttribute* old_attr = old_attributes->data[i];
    GumboAttribute* attr = gumbo_malloc(sizeof(GumboAttribute));
    *attr = *old_attr;
    attr->name = gumbo_strdup(old_attr->name);
    attr->value = gumbo_strdup(old_attr->value);
    gumbo_vector_add(attr, &element->attributes);
  }
  return new_node;
}
