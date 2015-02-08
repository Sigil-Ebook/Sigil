// Copyright 2015 Google Inc. All Rights Reserved.
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

#ifndef GUMBO_TAG_H_
#define GUMBO_TAG_H_

#include <stdbool.h>
#include <stddef.h>

#include "gumbo.h"

#ifdef __cplusplus
extern "C" {
#endif

bool gumbo_int_in_sorted_list(int tag, const int* taglist, int nlst);
bool gumbo_can_be_svg_tag(GumboTag tag);
bool gumbo_can_be_mathml_tag(GumboTag tag);
bool gumbo_can_be_shared_tag(GumboTag tag);

#ifdef __cplusplus
}
#endif

#endif  // GUMBO_TAG_H_
