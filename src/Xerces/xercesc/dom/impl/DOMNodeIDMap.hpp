/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * $Id: DOMNodeIDMap.hpp 676911 2008-07-15 13:27:32Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMNODEIDMAP_HPP)
#define XERCESC_INCLUDE_GUARD_DOMNODEIDMAP_HPP

//
//  This file is part of the internal implementation of the C++ XML DOM.
//  It should NOT be included or used directly by application programs.
//
//  Applications should include the file <xercesc/dom/DOM.hpp> for the entire
//  DOM API, or xercesc/dom/DOM*.hpp for individual DOM classes, where the class
//  name is substituded for the *.
//

#include <xercesc/util/XercesDefs.hpp>
XERCES_CPP_NAMESPACE_BEGIN


//
//  Class DOMNodeIDMap is a hash table that is used in the implementation of
//   of DOM_Document::getElementsByID().
//
//  Why Yet Another HashTable implementation?  Becuase it can be significantly
//  smaller when tuned for this exact usage, and the generic RefHashTableOf
//  from the xerces utils project is not a paricularly good fit.
//
class DOMAttr;
class DOMDocument;


class DOMNodeIDMap {
public:

    DOMNodeIDMap(XMLSize_t initialSize, DOMDocument *doc);    // Create a new hash table, sized to hold "initialSize"
                                     //  Entries.  It will automatically grow if need be.

    ~DOMNodeIDMap();

private:
    DOMNodeIDMap(const DOMNodeIDMap &other);   // No copy, assignement, comparison.
    DOMNodeIDMap &operator = (const DOMNodeIDMap &other);
    bool operator == (const DOMNodeIDMap &other);

public:
    void  add(DOMAttr *attr);       // Add the specified attribute to the table.
    void  remove(DOMAttr *other);   // Remove the specified attribute.
                                           //   Does nothing if the node is not in the table.
    DOMAttr *find(const XMLCh *ID);   // Find the attribute node in the table with this ID

private:
    void growTable();

private:
    DOMAttr      **fTable;
    XMLSize_t      fSizeIndex;              // Index of the current table size in the
                                            //   array of possible table sizes.
	XMLSize_t      fSize;                   // The current size of the table array
                                            //   (number of slots, not bytes.)
    XMLSize_t      fNumEntries;              // The number of entries used.
    XMLSize_t      fMaxEntries;              // The max number of entries to use before
                                            //   growing the table.
    DOMDocument *fDoc;                      // The owning document.
};

XERCES_CPP_NAMESPACE_END

#endif
