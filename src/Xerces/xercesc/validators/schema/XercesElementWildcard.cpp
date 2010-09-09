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
 * $Id: XercesElementWildcard.cpp 671133 2008-06-24 11:22:29Z borisk $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/schema/XercesElementWildcard.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Local methods
// ---------------------------------------------------------------------------

/**
 * XercesElementWildcard is used to check whether two element declarations conflict
 */
/**
 * Check whether two element declarations conflict
 */
bool XercesElementWildcard::conflict(SchemaGrammar* const pGrammar,
                                     ContentSpecNode::NodeTypes   type1,
                                     QName*                       q1,
                                     ContentSpecNode::NodeTypes   type2,
                                     QName*                       q2,
                                     SubstitutionGroupComparator* comparator)
{
    if (type1 == ContentSpecNode::Leaf &&
        type2 == ContentSpecNode::Leaf) {
        if (comparator->isEquivalentTo(q1, q2) || comparator->isEquivalentTo(q2, q1))
            return true;
    }
    else if (type1 == ContentSpecNode::Leaf) {
        return uriInWildcard(pGrammar, q1, q2->getURI(), type2, comparator);
    }
    else if (type2 == ContentSpecNode::Leaf) {
        return uriInWildcard(pGrammar, q2, q1->getURI(), type1, comparator);
    }
    else {
        return wildcardIntersect(type1, q1->getURI(), type2, q2->getURI());
    }
    return false;
}

bool XercesElementWildcard::uriInWildcard(SchemaGrammar* const         pGrammar,
                                          QName*                       qname,
                                          unsigned int                 wildcard,
                                          ContentSpecNode::NodeTypes   wtype,
                                          SubstitutionGroupComparator* comparator)
{
    if ((wtype & 0x0f)== ContentSpecNode::Any) {
        return true;
    }
    else if ((wtype & 0x0f)== ContentSpecNode::Any_NS) {
        // substitution of "uri" satisfies "wtype:wildcard"
        return comparator->isAllowedByWildcard(pGrammar, qname, wildcard, false);
    }
    else if ((wtype & 0x0f)== ContentSpecNode::Any_Other) {
        // substitution of "uri" satisfies "wtype:wildcard"
        return comparator->isAllowedByWildcard(pGrammar, qname, wildcard, true);
    }

    return false;
}

bool XercesElementWildcard::wildcardIntersect(ContentSpecNode::NodeTypes t1,
                                              unsigned int               w1,
                                              ContentSpecNode::NodeTypes t2,
                                              unsigned int               w2)
{
    if (((t1 & 0x0f) == ContentSpecNode::Any) ||
        ((t2 & 0x0f) == ContentSpecNode::Any)) {
        // if either one is "##any", then intersects
        return true;
    }
    else if (((t1 & 0x0f) == ContentSpecNode::Any_NS) &&
             ((t2 & 0x0f) == ContentSpecNode::Any_NS)) {
        // if both are "some_namespace" and equal, then intersects
        return w1 == w2;
    }
    else if (((t1 & 0x0f) == ContentSpecNode::Any_Other) &&
             ((t2 & 0x0f) == ContentSpecNode::Any_Other)) {
        // if both are "##other", then intersects
        return true;
    }
    // Below we assume that empty string has id 1.
    //
    else if (((t1 & 0x0f) == ContentSpecNode::Any_NS) &&
             ((t2 & 0x0f) == ContentSpecNode::Any_Other))
    {
      return w1 != w2 && w1 != 1;
    }
    else if (((t1 & 0x0f) == ContentSpecNode::Any_Other) &&
             ((t2 & 0x0f) == ContentSpecNode::Any_NS))
    {
      return w1 != w2 && w2 != 1;
    }
    return false;
}

XERCES_CPP_NAMESPACE_END
