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

  GumboNode*  gumbo_create_text_node(GumboNode* node, GumboNodeType type, const char * text);

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

#ifdef __cplusplus
}
#endif

#endif  // GUMBO_EDIT_H_
