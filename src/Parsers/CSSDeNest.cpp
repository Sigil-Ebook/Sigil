#include "CSSDeNest.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

// ------------------------------------------------------------
// String utilities
// ------------------------------------------------------------

std::string CSSDeNest::trim(const std::string& s) {
    size_t b = 0, e = s.size();
    while (b < e && std::isspace((unsigned char)s[b])) ++b;
    while (e > b && std::isspace((unsigned char)s[e - 1])) --e;
    return s.substr(b, e - b);
}

std::string CSSDeNest::replaceAll(std::string s, const std::string& from,
                                  const std::string& to) {
    if (from.empty()) return s;
    size_t pos = 0;
    while ((pos = s.find(from, pos)) != std::string::npos) {
        s.replace(pos, from.size(), to);
        pos += to.size();
    }
    return s;
}

// ------------------------------------------------------------
// Selector helpers
// ------------------------------------------------------------

// Split a comma-separated selector list, respecting parentheses.
std::vector<std::string> CSSDeNest::splitSelectors(const std::string& s) {
    std::vector<std::string> parts;
    std::string cur;
    int depth = 0;
    for (char c : s) {
        if (c == '(') ++depth;
        else if (c == ')' && depth > 0) --depth;
        if (c == ',' && depth == 0) {
            std::string t = trim(cur);
            if (!t.empty()) parts.push_back(t);
            cur.clear();
        } else {
            cur += c;
        }
    }
    std::string t = trim(cur);
    if (!t.empty()) parts.push_back(t);
    return parts;
}

// Combine one parent selector with one child selector.
// If child contains '&', every '&' is replaced by parent.
// Otherwise the child is treated as a descendant of parent.
std::string CSSDeNest::combineOne(const std::string& parent,
                               const std::string& child) {
    if (child.find('&') != std::string::npos)
        return replaceAll(child, "&", parent);
    return parent + " " + child;
}

// Cartesian product of two selector lists.
// ".a, .b"  x  ".c, &:hover"  →  ".a .c, .a:hover, .b .c, .b:hover"
std::string CSSDeNest::combineSelectorLists(const std::string& parent,
                                         const std::string& child) {
    auto ps = splitSelectors(parent);
    auto cs = splitSelectors(child);
    std::string result;
    bool first = true;
    for (const auto& p : ps) {
        for (const auto& c : cs) {
            if (!first) result += ",\n";
            result += combineOne(p, c);
            first = false;
        }
    }
    return result;
}

// ------------------------------------------------------------
// Low-level character-level readers
// (all advance `pos` and append consumed text to `out`)
// ------------------------------------------------------------

void CSSDeNest::readComment(const std::string& in, size_t& pos, std::string& out) {
    out += in[pos++]; // '/'
    out += in[pos++]; // '*'
    while (pos + 1 < in.size() &&
           !(in[pos] == '*' && in[pos + 1] == '/')) {
        out += in[pos++];
    }
    if (pos + 1 < in.size()) {
        out += in[pos++]; // '*'
        out += in[pos++]; // '/'
    }
}

void CSSDeNest::readString(const std::string& in, size_t& pos, std::string& out) {
    char q = in[pos];
    out += in[pos++]; // opening quote
    while (pos < in.size() && in[pos] != q) {
        if (in[pos] == '\\' && pos + 1 < in.size())
            out += in[pos++]; // backslash
        out += in[pos++];
    }
    if (pos < in.size()) out += in[pos++]; // closing quote
}

void CSSDeNest::readParens(const std::string& in, size_t& pos, std::string& out) {
    out += in[pos++]; // '('
    int depth = 1;
    while (pos < in.size() && depth > 0) {
        char c = in[pos];
        if (c == '(') { ++depth; out += in[pos++]; }
        else if (c == ')') { --depth; out += in[pos++]; }
        else if ((c == '"' || c == '\'') && depth > 0) {
            readString(in, pos, out);
        } else if (c == '/' && pos + 1 < in.size() && in[pos + 1] == '*') {
            readComment(in, pos, out);
        } else {
            out += in[pos++];
        }
    }
}

// just copied here from the header to make the code more easily readable
// ------------------------------------------------------------
// AST node
// ------------------------------------------------------------
#if 0  
struct CSSDeNest::Node {
    enum Type { TEXT, BLOCK } type = TEXT;

    // TEXT node: raw text (may include a trailing ';')
    std::string text;

    // BLOCK node fields
    std::string header;                      // selector or at-rule, trimmed, without '{'
    std::string rawBody;                     // raw text between '{' and '}' (before parsing)
    std::vector<CSSDeNest::Node> children;   // parsed children of this block
    bool isAtRule = false;                   // header starts with '@'
};
#endif

// ------------------------------------------------------------
// Parser
// ------------------------------------------------------------

// Read the raw content between '{' (already consumed) and the matching '}'.
// Advances pos past the closing '}'.
std::string CSSDeNest::readRawBody(const std::string& in, size_t& pos) {
    std::string body;
    int depth = 1;
    while (pos < in.size() && depth > 0) {
        char c = in[pos];
        if (c == '/' && pos + 1 < in.size() && in[pos + 1] == '*') {
            readComment(in, pos, body); continue;
        }
        if (c == '"' || c == '\'') { readString(in, pos, body); continue; }
        if (c == '(')              { readParens(in, pos, body); continue; }
        if (c == '{') ++depth;
        else if (c == '}') {
            --depth;
            if (depth == 0) { ++pos; break; } // consume the closing '}'
        }
        body += in[pos++];
    }
    return body;
}

// Parse CSS text at the current nesting level into a list of Nodes.
// Stops at EOF or an unmatched '}' (without consuming it).
std::vector<CSSDeNest::Node> CSSDeNest::parse(const std::string& in, size_t& pos) {
    std::vector<Node> nodes;

    while (pos < in.size() && in[pos] != '}') {

        // Accumulate text until '{', '}', or ';'.
        std::string chunk;
        while (pos < in.size()) {
            char c = in[pos];
            if (c == '{' || c == '}' || c == ';') break;
            if (c == '/' && pos + 1 < in.size() && in[pos + 1] == '*') {
                readComment(in, pos, chunk); continue;
            }
            if (c == '"' || c == '\'') { readString(in, pos, chunk); continue; }
            if (c == '(')              { readParens(in, pos, chunk); continue; }
            chunk += in[pos++];
        }

        if (pos >= in.size() || in[pos] == '}') {
            if (!chunk.empty())
                nodes.push_back({Node::TEXT, chunk, "", "", {}, false});
            break;
        }

        char stop = in[pos];

        if (stop == ';') {
            chunk += in[pos++]; // include ';'
            nodes.push_back({Node::TEXT, chunk, "", "", {}, false});
        } else { // stop == '{'
            // Separate any leading whitespace and CSS block comments from the
            // actual selector/at-rule header.  Comments that appear between
            // the previous block and this opening brace logically belong to
            // the preceding context, not to the header itself.
            size_t ci = 0;
            while (ci < chunk.size()) {
                size_t prev = ci;
                // skip whitespace
                while (ci < chunk.size() &&
                       std::isspace((unsigned char)chunk[ci])) ++ci;
                // skip /* … */ comment
                if (ci + 1 < chunk.size() &&
                    chunk[ci] == '/' && chunk[ci + 1] == '*') {
                    ci += 2;
                    while (ci + 1 < chunk.size() &&
                           !(chunk[ci] == '*' && chunk[ci + 1] == '/')) ++ci;
                    if (ci + 1 < chunk.size()) ci += 2;
                } else if (ci == prev) {
                    break; // no progress → start of actual header
                }
            }

            // Anything before `ci` is pre-header whitespace/comments.
            if (ci > 0)
                nodes.push_back({Node::TEXT, chunk.substr(0, ci),
                                 "", "", {}, false});

            std::string header = trim(chunk.substr(ci));

            ++pos; // consume '{'

            std::string rawBody = readRawBody(in, pos);

            size_t innerPos = 0;
            std::vector<Node> children = parse(rawBody, innerPos);

            CSSDeNest::Node n;
            n.type     = Node::BLOCK;
            n.header   = header;
            n.rawBody  = rawBody;
            n.children = std::move(children);
            n.isAtRule = (!header.empty() && header[0] == '@');
            nodes.push_back(std::move(n));
        }
    }

    return nodes;
}

// ------------------------------------------------------------
// At-rule classification
// ------------------------------------------------------------

// Extract the @-keyword (lowercased), e.g. "@media", "@keyframes".
std::string CSSDeNest::atRuleKeyword(const std::string& header) {
    std::string kw;
    for (char c : header) {
        if (std::isspace((unsigned char)c) || c == '(') break;
        kw += (char)std::tolower((unsigned char)c);
    }
    return kw;
}

// At-rules whose block content must NOT be rewritten for nesting.
// Their body is reproduced verbatim.
bool CSSDeNest::isPassThroughAtRule(const std::string& header) {
    std::string kw = atRuleKeyword(header);
    return kw == "@keyframes"         ||
           kw == "@-webkit-keyframes" ||
           kw == "@-moz-keyframes"    ||
           kw == "@-o-keyframes"      ||
           kw == "@font-face"         ||
           kw == "@counter-style"     ||
           kw == "@viewport"          ||
           kw == "@-ms-viewport";
}

// ------------------------------------------------------------
// Flattener (mutually recursive)
// ------------------------------------------------------------

std::string CSSDeNest::flattenBlock(const Node& block, const std::string& parentSel) {
    std::ostringstream out;

    if (block.isAtRule) {
        if (isPassThroughAtRule(block.header)) {
            // @keyframes, @font-face, etc. – emit verbatim
            out << block.header << " {\n" << block.rawBody << "}\n";
            return out.str();
        }

        // @media, @supports, @layer, @document, etc.
        // Flatten the children under the same parentSel, then wrap.
        std::string inner = flattenNodes(block.children, parentSel);
        out << block.header << " {\n" << inner << "}\n";
    } else {
        // Regular selector block.
        std::string selector = parentSel.empty()
            ? block.header
            : combineSelectorLists(parentSel, block.header);

        out << flattenNodes(block.children, selector);
    }

    return out.str();
}

std::string CSSDeNest::flattenNodes(const std::vector<Node>& nodes,
                                    const std::string& parentSel) {
    std::ostringstream out;

    if (parentSel.empty()) {
        // Top-level (or inside an at-rule with no owning selector):
        // TEXT nodes pass through directly; BLOCK nodes are flattened.
        for (const auto& n : nodes) {
            if (n.type == Node::TEXT)
                out << n.text;
            else
                out << flattenBlock(n, "");
        }
    } else {
        // Inside a selector context:
        // 1. Collect all direct declarations (TEXT nodes) and emit as
        //    "selector { decls }" (preserving the original whitespace).
        // 2. Flatten nested BLOCK nodes afterwards.
        std::string decls;
        for (const auto& n : nodes)
            if (n.type == Node::TEXT) decls += n.text;

        if (!trim(decls).empty())
            out << parentSel << " {" << decls << "}\n";

        for (const auto& n : nodes)
            if (n.type == Node::BLOCK)
                out << flattenBlock(n, parentSel);
    }

    return out.str();
}

// ============================================================
// Public
// ============================================================

std::string CSSDeNest::denest_css(const std::string& input) {
    size_t pos = 0;
    std::vector<Node> nodes = parse(input, pos);
    return flattenNodes(nodes, "");
}

