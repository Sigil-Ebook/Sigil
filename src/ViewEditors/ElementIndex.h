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
 * it depends on the specific ViewEditor... BookViewEditor                          
 * does this, CodeViewEditor doesn't.                                               
 */

    struct ElementIndex {
        /**
         * The name of the element.
         */
        QString name;

        /**
         * The index of this element in its parent's list of children.
         */
        int index;
    };

#endif // ELEMENTINDEX_H


