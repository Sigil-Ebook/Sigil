// Copyright 2015-2016 Kevin B. Hendricks, Stratford Ontario  All Rights Reserved.
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

#ifndef GUMBO_EDIT_H_
#define GUMBO_EDIT_H_

#include "gumbo.h"

#ifdef __cplusplus
extern "C" {
#endif

  // See gumbo.h for:
  //   void gumbo_create_node(void);
  //   void gumbo_destroy_node(GumboNode* node)

  // create and initialize a completely new output tree
  GumboOutput*  gumbo_new_output_init(void);

  // Creates an text node of specified type and returns it.
  // Types are GUMBO_NODE_TEXT, GUMBO_NODE_WHITESPACE, GUMBO_NODE_CDATA, and GUMBO_NODE_COMMENT
  // No entities are allowed (replace them with their utf-8 character equivalents)
  // Note: CDATA and COMMENTS text should NOT include their respective delimiters
  // ie. No <-- --> and not CDATA[[ and ]]

  // Note: Use gumbo_destroy_node(GumboNode * node) to properly destroy the node if outside 
  // the final output tree

  GumboNode*  gumbo_create_text_node(GumboNodeType type, const char * text);

  // Creates an element node with the tag (enum) in the specified namespace and returns it.
  // Since no original text exists, any created element tag must already exist in the tag_enum.h
  // This is why we have expanded the set of recognized tags to include all svg and mathml tags 

  // Note: Use gumbo_destroy_node(GumboNode * node) to properly destroy the node if outside 
  // the final output tree

  GumboNode* gumbo_create_element_node(GumboTag tag, GumboNamespaceEnum gns);

  // Creates an template node and returns it.

  // Note: Use gumbo_destroy_node(GumboNode * node) to properly destroy the node if outside 
  // the final output tree.

  GumboNode* gumbo_create_template_node(void);

  // Appends a node to the end of its parent, setting the "parent" and
  // "index_within_parent" fields appropriately.

  void gumbo_append_node(GumboNode* parent, GumboNode* node);

  // Inserts a node at the specified index in the specified parent, 
  // updating the "parent" and "index_within_parent" fields of it and all its siblings.
  // If the index is -1, this simply calls gumbo_append_node.

  void gumbo_insert_node(GumboNode* node, GumboNode* target_parent, int target_index);

  // removes a node from its parent but does not destroy it

  // Note: Use gumbo_destroy_node(GumboNode * node) to properly destroy the node if outside 
  // the final output tree.

  void gumbo_remove_from_parent(GumboNode* node);

  // Clones attributes, tags, etc. of a node, but does not copy the content (its children).  
  // The clone shares no structure with the original node: all owned strings and
  // values are fresh copies.

  // Note: Use gumbo_destroy_node(GumboNode * node) to properly destroy the node if outside 
  // the output tree.

  GumboNode* clone_element_node(const GumboNode* node);


  // interface from attribute.h
  void gumbo_attribute_set_value(GumboAttribute *attr, const char *value);
  void gumbo_destroy_attribute(GumboAttribute* attribute);
  void gumbo_element_set_attribute(GumboElement *element, const char *name, const char *value);
  void gumbo_element_remove_attribute_at(GumboElement *element, unsigned int pos);
  void gumbo_element_remove_attribute(GumboElement *element, GumboAttribute *attr);

  // interface from vector.h
  // Initializes a new GumboVector with the specified initial capacity.
  void gumbo_vector_init(size_t initial_capacity, GumboVector* vector);

  // Frees the memory used by an GumboVector.  Does not free the contained pointers.
  void gumbo_vector_destroy(GumboVector* vector);

  // Adds a new element to an GumboVector.
  void gumbo_vector_add(void* element, GumboVector* vector);

  // Removes and returns the element most recently added to the GumboVector.
  // Ownership is transferred to caller.  Capacity is unchanged.  If the vector is
  // empty, NULL is returned.
  void* gumbo_vector_pop(GumboVector* vector);

  // Inserts an element at a specific index.  This is potentially O(N) time, but
  // is necessary for some of the spec's behavior.
  void gumbo_vector_insert_at(void* element, int index, GumboVector* vector);

  // Removes an element from the vector, or does nothing if the element is not in the vector.
  void gumbo_vector_remove(const void* element, GumboVector* vector);

  // Removes and returns an element at a specific index.  Note that this is
  // potentially O(N) time and should be used sparingly.
  void* gumbo_vector_remove_at(int index, GumboVector* vector);

  int gumbo_vector_index_of(GumboVector* vector, const void* element);
  void gumbo_vector_splice(int where, int n_to_remove, void **data, int n_to_insert, GumboVector* vector);

#ifdef __cplusplus
}
#endif

#endif  // GUMBO_EDIT_H_
