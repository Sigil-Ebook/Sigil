/************************************************************************
**
**  Copyright (C) 2019 Kevin B. Hendricks, Stratford, Ontario, Canada
**
**  This file is part of Sigil.
**
**  Sigil is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  Sigil is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with Sigil.  If not, see <http://www.gnu.org/licenses/>.
**
*************************************************************************/

#pragma once
#ifndef ELEMENTINDEX_H
#define ELEMENTINDEX_H

/**
 *   Represents an element in the XHTML document tree
 * and the index of its child that selects the
 * next element in the chain.
 *
 *   By constructing a list of these items, one can
 * navigate the tree by selecting the element,
 * its child with the specified index, its child
 * with its index and so on until reaching
 * the element ultimately identified by this chain.
 *
 *   Because of WebKit limitations, this hierarchy
 * does not really look at all child nodes, but only
 * at element child nodes. The text nodes are considered
 * children only for the last element... and even then,
 * it depends on the specific ViewEditor or PreView
 * does this, CodeViewEditor doesn't.
 */

    struct ElementIndex {
        /**
         * The name of the element.
         */
        QString name;

        /**
         * The child index (counting elements only) of this node to take
         * to reach the next node in the hierarchy (index starts at 0)
         *
         * index of -1 means this is a destination node
         */
        int index;
    };

// For example for this hierarchy:

// name:  "html"  index:  1
// name:  "body"  index:  20
// name:  "p"  index:  -1

// 1. start at the html tag and take element child 1 (start counting at 0) to reach the body tag
// 2. start at the body tag and take element child 20 (again start counting at 0) to reach a p tag
// 3. reach the p tag - you are at your destination


// As a second example:

// name:  "html"  index:  1
// name:  "body"  index:  0
// name:  "section"  index:  0
// name:  "section"  index:  4
// name:  "section"  index:  1
// name:  "p"  index:  -1

// 1. start at html tag and take element child index 1 to reach the body tag
// 2. at the body tag take element child index 0 to reach the section tag
// 3. at the section tag take element child index 0 to reach another section tag
// 4. at that section tag take elemnt child index 4 to reach another section tag
// 5. at that section tag take element child index 1 to reach a p tag
// 6. reached the destination p tag

#endif // ELEMENTINDEX_H

