/************************************************************************
**
**  Copyright (C) 2021 Kevin B. Hendricks, Stratford, Ontario, Canada
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

#include <QTranslator>
#include <QString>
#include <QHash>

#include "PCRE2/PCREErrors.h"

PCREErrors *PCREErrors::m_instance = 0;

PCREErrors *PCREErrors::instance()
{
    if (m_instance == 0) {
        m_instance = new PCREErrors();
    }

    return m_instance;
}

PCREErrors::PCREErrors()
{
    SetErrorMap();
}


QString PCREErrors::GetError(const QString &code, const QString &ow)
{
    QString emsg = m_XlateError.value(code, ow);
    return emsg;
}



void PCREErrors::SetErrorMap()
{
    if (!m_XlateError.isEmpty()) {
        return;
    }

    m_XlateError[ "no error" ] = tr(  "no error" );
    m_XlateError[ "\\ at end of pattern" ] = tr( "\\ at end of pattern" );
    m_XlateError[ "\\c at end of pattern" ] = tr( "\\c at end of pattern" );
    m_XlateError[ "unrecognized character follows \\" ] = tr( "unrecognized character follows \\" );
    m_XlateError[ "numbers out of order in {} quantifier" ] = tr( "numbers out of order in {} quantifier" );
    // 5
    m_XlateError[ "number too big in {} quantifier" ] = tr( "number too big in {} quantifier" );
    m_XlateError[ "missing terminating ] for character class" ] = tr( "missing terminating ] for character class" );
    m_XlateError[ "escape sequence is invalid in character class" ] = tr( "escape sequence is invalid in character class" );
    m_XlateError[ "range out of order in character class" ] = tr( "range out of order in character class" );
    m_XlateError[ "quantifier does not follow a repeatable item" ] = tr( "quantifier does not follow a repeatable item" );

    // 10
    m_XlateError[ "internal error: unexpected repeat" ] = tr( "internal error: unexpected repeat" );
    m_XlateError[ "unrecognized character after (? or (?-" ] = tr( "unrecognized character after (? or (?-" );
    m_XlateError[ "POSIX named classes are supported only within a class" ] = tr( "POSIX named classes are supported only within a class" );
    m_XlateError[ "POSIX collating elements are not supported" ] = tr( "POSIX collating elements are not supported" );
    m_XlateError[ "missing closing parenthesis" ] = tr( "missing closing parenthesis" );

    // 15
    m_XlateError[ "reference to non-existent subpattern" ] = tr( "reference to non-existent subpattern" );
    m_XlateError[ "pattern passed as NULL" ] = tr( "pattern passed as NULL" );
    m_XlateError[ "unrecognised compile-time option bit(s)" ] = tr( "unrecognised compile-time option bit(s)" );
    m_XlateError[ "missing ) after (?# comment" ] = tr( "missing ) after (?# comment" );
    m_XlateError[ "parentheses are too deeply nested" ] = tr( "parentheses are too deeply nested" );

    // 20
    m_XlateError[ "regular expression is too large" ] = tr( "regular expression is too large" );
    m_XlateError[ "failed to allocate heap memory" ] = tr( "failed to allocate heap memory" );
    m_XlateError[ "unmatched closing parenthesis" ] = tr( "unmatched closing parenthesis" );
    m_XlateError[ "internal error: code overflow" ] = tr( "internal error: code overflow" );
    m_XlateError[ "missing closing parenthesis for condition" ] = tr( "missing closing parenthesis for condition" );

    // 25
    m_XlateError[ "lookbehind assertion is not fixed length" ] = tr( "lookbehind assertion is not fixed length" );
    m_XlateError[ "a relative value of zero is not allowed" ] = tr( "a relative value of zero is not allowed" );
    m_XlateError[ "conditional subpattern contains more than two branches" ] = tr( "conditional subpattern contains more than two branches" );
    m_XlateError[ "assertion expected after (?( or (?(?C)" ] = tr( "assertion expected after (?( or (?(?C)" );
    m_XlateError[ "digit expected after (?+ or (?-\0" ] = tr( "digit expected after (?+ or (?-\0" );

    // 30
    m_XlateError[ "unknown POSIX class name" ] = tr( "unknown POSIX class name" );
    m_XlateError[ "internal error in pcre2_study(): should not occur" ] = tr( "internal error in pcre2_study(): should not occur" );
    m_XlateError[ "this version of PCRE2 does not have Unicode support" ] = tr( "this version of PCRE2 does not have Unicode support" );
    m_XlateError[ "parentheses are too deeply nested (stack check)" ] = tr( "parentheses are too deeply nested (stack check)" );
    m_XlateError[ "character code point value in \\x{} or \\o{} is too large" ] = tr( "character code point value in \\x{} or \\o{} is too large" );

    // 35
    m_XlateError[ "lookbehind is too complicated" ] = tr( "lookbehind is too complicated" );
    m_XlateError[ "\\C is not allowed in a lookbehind assertion in UTF-16 mode" ] = tr( "\\C is not allowed in a lookbehind assertion in UTF-16 mode" );
    m_XlateError[ "PCRE2 does not support \\F, \\L, \\l, \\N{name}, \\U, or \\u" ] = tr( "PCRE2 does not support \\F, \\L, \\l, \\N{name}, \\U, or \\u" );
    m_XlateError[ "number after (?C is greater than 255" ] = tr( "number after (?C is greater than 255" );
    m_XlateError[ "closing parenthesis for (?C expected" ] = tr( "closing parenthesis for (?C expected" );

    // 40
    m_XlateError[ "invalid escape sequence in (*VERB) name" ] = tr( "invalid escape sequence in (*VERB) name" );
    m_XlateError[ "unrecognized character after (?P" ] = tr( "unrecognized character after (?P" );
    m_XlateError[ "syntax error in subpattern name (missing terminator?)" ] = tr( "syntax error in subpattern name (missing terminator?)" );
    m_XlateError[ "two named subpatterns have the same name (PCRE2_DUPNAMES not set)" ] = tr( "two named subpatterns have the same name (PCRE2_DUPNAMES not set)" );
    m_XlateError[ "subpattern name must start with a non-digit" ] = tr( "subpattern name must start with a non-digit" );

    // 45
    m_XlateError[ "this version of PCRE2 does not have support for \\P, \\p, or \\X" ] = tr( "this version of PCRE2 does not have support for \\P, \\p, or \\X" );
    m_XlateError[ "malformed \\P or \\p sequence" ] = tr( "malformed \\P or \\p sequence" );
    m_XlateError[ "unknown property name after \\P or \\p" ] = tr( "unknown property name after \\P or \\p" );
    m_XlateError[ "subpattern name is too long (maximum 32 code units)" ] = tr( "subpattern name is too long (maximum 32 code units)" );
    m_XlateError[ "too many named subpatterns (maximum 10000)" ] = tr( "too many named subpatterns (maximum 10000)" );

    // 50
    m_XlateError[ "invalid range in character class" ] = tr( "invalid range in character class" );
    m_XlateError[ "octal value is greater than \\377 in 8-bit non-UTF-8 mode" ] = tr( "octal value is greater than \\377 in 8-bit non-UTF-8 mode" );
    m_XlateError[ "internal error: overran compiling workspace" ] = tr( "internal error: overran compiling workspace" );
    m_XlateError[ "internal error: previously-checked referenced subpattern not found" ] = tr( "internal error: previously-checked referenced subpattern not found" );
    m_XlateError[ "DEFINE subpattern contains more than one branch" ] = tr( "DEFINE subpattern contains more than one branch" );

    // 55
    m_XlateError[ "missing opening brace after \\o" ] = tr( "missing opening brace after \\o" );
    m_XlateError[ "internal error: unknown newline setting" ] = tr( "internal error: unknown newline setting" );
    m_XlateError[ "\\g is not followed by a braced, angle-bracketed, or quoted name/number or by a plain number" ] = tr( "\\g is not followed by a braced, angle-bracketed, or quoted name/number or by a plain number" );
    m_XlateError[ "(?R (recursive pattern call) must be followed by a closing parenthesis" ] = tr( "(?R (recursive pattern call) must be followed by a closing parenthesis" );
    m_XlateError[ "obsolete error (should not occur)" ] = tr( "obsolete error (should not occur)" );

    // 60
    m_XlateError[ "(*VERB) not recognized or malformed" ] = tr( "(*VERB) not recognized or malformed" );
    m_XlateError[ "subpattern number is too big" ] = tr( "subpattern number is too big" );
    m_XlateError[ "subpattern name expected" ] = tr( "subpattern name expected" );
    m_XlateError[ "internal error: parsed pattern overflow" ] = tr( "internal error: parsed pattern overflow" );
    m_XlateError[ "non-octal character in \\o{} (closing brace missing?)" ] = tr( "non-octal character in \\o{} (closing brace missing?)" );

    // 65
    m_XlateError[ "different names for subpatterns of the same number are not allowed" ] = tr( "different names for subpatterns of the same number are not allowed" );
    m_XlateError[ "(*MARK) must have an argument" ] = tr( "(*MARK) must have an argument" );
    m_XlateError[ "non-hex character in \\x{} (closing brace missing?)" ] = tr( "non-hex character in \\x{} (closing brace missing?)" );
    m_XlateError[ "\\c must be followed by a printable ASCII character" ] = tr( "\\c must be followed by a printable ASCII character" );
    m_XlateError[ "\\k is not followed by a braced, angle-bracketed, or quoted name" ] = tr( "\\k is not followed by a braced, angle-bracketed, or quoted name" );

    // 70
    m_XlateError[ "internal error: unknown meta code in check_lookbehinds()" ] = tr( "internal error: unknown meta code in check_lookbehinds()" );
    m_XlateError[ "\\N is not supported in a class" ] = tr( "\\N is not supported in a class" );
    m_XlateError[ "callout string is too long" ] = tr( "callout string is too long" );
    m_XlateError[ "disallowed Unicode code point (>= 0xd800 && <= 0xdfff)" ] = tr( "disallowed Unicode code point (>= 0xd800 && <= 0xdfff)" );
    m_XlateError[ "using UTF is disabled by the application" ] = tr( "using UTF is disabled by the application" );

    // 75
    m_XlateError[ "using UCP is disabled by the application" ] = tr( "using UCP is disabled by the application" );
    m_XlateError[ "name is too long in (*MARK), (*PRUNE), (*SKIP), or (*THEN)" ] = tr( "name is too long in (*MARK), (*PRUNE), (*SKIP), or (*THEN)" );
    m_XlateError[ "character code point value in \\u.... sequence is too large" ] = tr( "character code point value in \\u.... sequence is too large" );
    m_XlateError[ "digits missing in \\x{} or \\o{} or \\N{U+}" ] = tr( "digits missing in \\x{} or \\o{} or \\N{U+}" );
    m_XlateError[ "syntax error or number too big in (?(VERSION condition" ] = tr( "syntax error or number too big in (?(VERSION condition" );
 
    // 80
    m_XlateError[ "internal error: unknown opcode in auto_possessify()" ] = tr( "internal error: unknown opcode in auto_possessify()" );
    m_XlateError[ "missing terminating delimiter for callout with string argument" ] = tr( "missing terminating delimiter for callout with string argument" );
    m_XlateError[ "unrecognized string delimiter follows (?C" ] = tr( "unrecognized string delimiter follows (?C" );
    m_XlateError[ "using \\C is disabled by the application" ] = tr( "using \\C is disabled by the application" );
    m_XlateError[ "(?| and/or (?J: or (?x: parentheses are too deeply nested" ] = tr( "(?| and/or (?J: or (?x: parentheses are too deeply nested" );

    // 85
    m_XlateError[ "using \\C is disabled in this PCRE2 library" ] = tr( "using \\C is disabled in this PCRE2 library" );
    m_XlateError[ "regular expression is too complicated" ] = tr( "regular expression is too complicated" );
    m_XlateError[ "lookbehind assertion is too long" ] = tr( "lookbehind assertion is too long" );
    m_XlateError[ "pattern string is longer than the limit set by the application" ] = tr( "pattern string is longer than the limit set by the application" );
    m_XlateError[ "internal error: unknown code in parsed pattern" ] = tr( "internal error: unknown code in parsed pattern" );

    // 90
    m_XlateError[ "internal error: bad code value in parsed_skip()" ] = tr( "internal error: bad code value in parsed_skip()" );
    m_XlateError[ "PCRE2_EXTRA_ALLOW_SURROGATE_ESCAPES is not allowed in UTF-16 mode" ] = tr( "PCRE2_EXTRA_ALLOW_SURROGATE_ESCAPES is not allowed in UTF-16 mode" );
    m_XlateError[ "invalid option bits with PCRE2_LITERAL" ] = tr( "invalid option bits with PCRE2_LITERAL" );
    m_XlateError[ "\\N{U+dddd} is supported only in Unicode (UTF) mode" ] = tr( "\\N{U+dddd} is supported only in Unicode (UTF) mode" );
    m_XlateError[ "invalid hyphen in option setting" ] = tr( "invalid hyphen in option setting" );

    // 95
    m_XlateError[ "(*alpha_assertion) not recognized" ] = tr( "(*alpha_assertion) not recognized" );
    m_XlateError[ "script runs require Unicode support, which this version of PCRE2 does not have" ] = tr( "script runs require Unicode support, which this version of PCRE2 does not have" );
    m_XlateError[ "too many capturing groups (maximum 65535)" ] = tr( "too many capturing groups (maximum 65535)" );
    m_XlateError[ "atomic assertion expected after (?( or (?(?C)" ] = tr( "atomic assertion expected after (?( or (?(?C)" );
    m_XlateError[ "\\K is not allowed in lookarounds (but see PCRE2_EXTRA_ALLOW_LOOKAROUND_BSK)" ] = tr( "\\K is not allowed in lookarounds (but see PCRE2_EXTRA_ALLOW_LOOKAROUND_BSK)" );

}
