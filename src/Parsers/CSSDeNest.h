/************************************************************************
 **
 **  CSSDeNest.h
 **  Used for the Sigil C++ CSS Parser
 **
 **  Copyright (C) 2026  Kevin B. Hendricks, Stratford, Ontario, Canada
 **  Co-authored with CoPilot: Claude 4.6 engine
 **
 **  License: MIT
 **  Permission is hereby granted, free of charge, to any person obtaining a
 **  copy of this software and associated documentation files (the “Software”),
 **  to deal in the Software without restriction, including without limitation
 **  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 **  and/or sell copies of the Software, and to permit persons to whom the
 **  Software is furnished to do so, subject to the following conditions:
 **
 **  The above copyright notice and this permission notice shall be included
 **  in all copies or substantial portions of the Software.
 **
 **  THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS
 **  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 **  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 **  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 **  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 **  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 **  DEALINGS IN THE SOFTWARE.
 **
 ***********************************************************************/
#ifndef CSSDENEST_H
#define CSSDENEST_H 

#include <string>
#include <vector>

// Flattens native CSS nesting so that older CSS parsers can consume the output.
//
// Any CSS that is not nested is passed through unchanged.
// Nested selectors are combined with their parent using the "&" nesting
// selector rules (if "&" appears in the child it is replaced by the parent;
// otherwise the child becomes a descendant of the parent).
// Nested at-rules (@media, @supports, …) are hoisted above the enclosing
// selector so that the selector is wrapped inside the at-rule.
// At-rules whose content must not be rewritten (@keyframes, @font-face, …)
// are emitted verbatim.
//
// Parameters:
//   input  – contents of a CSS file
// Returns:
//   flattened CSS as a std::string

class CSSDeNest {

  public:

    struct Node {
        enum Type { TEXT, BLOCK } type = TEXT;

        // TEXT node: raw text (may include a trailing ';')
        std::string text;

        // BLOCK node fields
        std::string header;           // selector or at-rule, trimmed, without '{'
        std::string rawBody;          // raw text between '{' and '}' (before parsing)
        std::vector<Node> children;   // parsed children of this block
        bool isAtRule = false;        // header starts with '@'
    };


    static std::string denest_css(const std::string& input);
    
  private:

    // ------------------------------------------------------------
    // String utilities
    // ------------------------------------------------------------

    static std::string trim(const std::string& s);
    static std::string replaceAll(std::string s, const std::string& from, const std::string& to);

    // ------------------------------------------------------------
    // Selector helpers
    // ------------------------------------------------------------

    // Split a comma-separated selector list, respecting parentheses.
    static std::vector<std::string> splitSelectors(const std::string& s);

    // Combine one parent selector with one child selector.
    // If child contains '&', every '&' is replaced by parent.
    // Otherwise the child is treated as a descendant of parent.
    static std::string combineOne(const std::string& parent, const std::string& child);

    // Cartesian product of two selector lists.
    // ".a, .b"  x  ".c, &:hover"  →  ".a .c, .a:hover, .b .c, .b:hover"
    static std::string combineSelectorLists(const std::string& parent, const std::string& child);

    // ------------------------------------------------------------
    // Low-level character-level readers
    // (all advance `pos` and append consumed text to `out`)
    // ------------------------------------------------------------

    static void readComment(const std::string& in, size_t& pos, std::string& out);
    static void readString(const std::string& in, size_t& pos, std::string& out);
    static void readParens(const std::string& in, size_t& pos, std::string& out);

    // ------------------------------------------------------------
    // Parser
    // ------------------------------------------------------------

    // Read the raw content between '{' (already consumed) and the matching '}'.
    // Advances pos past the closing '}'.
    static std::string readRawBody(const std::string& in, size_t& pos);

    // Parse CSS text at the current nesting level into a list of Nodes.
    // Stops at EOF or an unmatched '}' (without consuming it).
    static std::vector<CSSDeNest::Node> parse(const std::string& in, size_t& pos);

    // ------------------------------------------------------------
    // At-rule classification
    // ------------------------------------------------------------

    // Extract the @-keyword (lowercased), e.g. "@media", "@keyframes".
    static std::string atRuleKeyword(const std::string& header);

    // At-rules whose block content must NOT be rewritten for nesting.
    // Their body is reproduced verbatim.
    static bool isPassThroughAtRule(const std::string& header);

    // ------------------------------------------------------------
    // Flattener (mutually recursive)
    // ------------------------------------------------------------

    static std::string flattenNodes(const std::vector<CSSDeNest::Node>& nodes,
                                    const std::string& parentSel);
    
    static std::string flattenBlock(const CSSDeNest::Node& block,
                                    const std::string& parentSel);

};
#endif // CSSDENEST_H
