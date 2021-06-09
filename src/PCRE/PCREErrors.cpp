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

#include "PCRE/PCREErrors.h"

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
    m_XlateError[ "number too big in {} quantifier" ] = tr( "number too big in {} quantifier" );
    m_XlateError[ "missing terminating ] for character class" ] = tr( "missing terminating ] for character class" );
    m_XlateError[ "invalid escape sequence in character class" ] = tr( "invalid escape sequence in character class" );
    m_XlateError[ "range out of order in character class" ] = tr( "range out of order in character class" );
    m_XlateError[ "nothing to repeat" ] = tr( "nothing to repeat" );
    m_XlateError[ "operand of unlimited repeat could match the empty string" ] = tr( "operand of unlimited repeat could match the empty string" );
    m_XlateError[ "internal error: unexpected repeat" ] = tr( "internal error: unexpected repeat" );
    m_XlateError[ "unrecognized character after (? or (?-" ] = tr( "unrecognized character after (? or (?-" );
    m_XlateError[ "POSIX named classes are supported only within a class" ] = tr( "POSIX named classes are supported only within a class" );
    m_XlateError[ "missing )" ] = tr( "missing )" );
    m_XlateError[ "reference to non-existent subpattern" ] = tr( "reference to non-existent subpattern" );
    m_XlateError[ "erroffset passed as NULL" ] = tr( "erroffset passed as NULL" );
    m_XlateError[ "unknown option bit(s) set" ] = tr( "unknown option bit(s) set" );
    m_XlateError[ "missing ) after comment" ] = tr( "missing ) after comment" );
    m_XlateError[ "parentheses nested too deeply" ] = tr( "parentheses nested too deeply" );
    m_XlateError[ "regular expression is too large" ] = tr( "regular expression is too large" );
    m_XlateError[ "failed to get memory" ] = tr( "failed to get memory" );
    m_XlateError[ "unmatched parentheses" ] = tr( "unmatched parentheses" );
    m_XlateError[ "internal error: code overflow" ] = tr( "internal error: code overflow" );
    m_XlateError[ "unrecognized character after (?<" ] = tr( "unrecognized character after (?<" );
    m_XlateError[ "lookbehind assertion is not fixed length" ] = tr( "lookbehind assertion is not fixed length" );
    m_XlateError[ "malformed number or name after (?(" ] = tr( "malformed number or name after (?(" );
    m_XlateError[ "conditional group contains more than two branches" ] = tr( "conditional group contains more than two branches" );
    m_XlateError[ "assertion expected after (?(" ] = tr( "assertion expected after (?(" );
    m_XlateError[ "(?R or (?[+-]digits must be followed by )" ] = tr( "(?R or (?[+-]digits must be followed by )" );
    m_XlateError[ "unknown POSIX class name" ] = tr( "unknown POSIX class name" );
    m_XlateError[ "POSIX collating elements are not supported" ] = tr( "POSIX collating elements are not supported" );
    m_XlateError[ "this version of PCRE is compiled without UTF support" ] = tr( "this version of PCRE is compiled without UTF support" );
    m_XlateError[ "spare error" ] = tr( "spare error" );
    m_XlateError[ "character value in \\x{} or \\o{} is too large" ] = tr( "character value in \\x{} or \\o{} is too large" );
    m_XlateError[ "invalid condition (?(0)" ] = tr( "invalid condition (?(0)" );
    m_XlateError[ "\\C not allowed in lookbehind assertion" ] = tr( "\\C not allowed in lookbehind assertion" );
    m_XlateError[ "PCRE does not support \\L, \\l, \\N{name}, \\U, or \\u" ] = tr( "PCRE does not support \\L, \\l, \\N{name}, \\U, or \\u" );
    m_XlateError[ "number after (?C is > 255" ] = tr( "number after (?C is > 255" );
    m_XlateError[ "closing ) for (?C expected" ] = tr( "closing ) for (?C expected" );
    m_XlateError[ "recursive call could loop indefinitely" ] = tr( "recursive call could loop indefinitely" );
    m_XlateError[ "unrecognized character after (?P" ] = tr( "unrecognized character after (?P" );
    m_XlateError[ "syntax error in subpattern name (missing terminator)" ] = tr( "syntax error in subpattern name (missing terminator)" );
    m_XlateError[ "two named subpatterns have the same name" ] = tr( "two named subpatterns have the same name" );
    m_XlateError[ "invalid UTF-8 string" ] = tr( "invalid UTF-8 string" );
    m_XlateError[ "support for \\P, \\p, and \\X has not been compiled" ] = tr( "support for \\P, \\p, and \\X has not been compiled" );
    m_XlateError[ "malformed \\P or \\p sequence" ] = tr( "malformed \\P or \\p sequence" );
    m_XlateError[ "unknown property name after \\P or \\p" ] = tr( "unknown property name after \\P or \\p" );
    m_XlateError[ "subpattern name is too long (maximum 32 characters)" ] = tr( "subpattern name is too long (maximum 32 characters)" );
    m_XlateError[ "too many named subpatterns (maximum 10000)" ] = tr( "too many named subpatterns (maximum 10000)" );
    m_XlateError[ "repeated subpattern is too long" ] = tr( "repeated subpattern is too long" );
    m_XlateError[ "octal value is greater than \\377 in 8-bit non-UTF-8 mode" ] = tr( "octal value is greater than \\377 in 8-bit non-UTF-8 mode" );
    m_XlateError[ "internal error: overran compiling workspace" ] = tr( "internal error: overran compiling workspace" );
    m_XlateError[ "internal error: previously-checked referenced subpattern not found" ] = tr( "internal error: previously-checked referenced subpattern not found" );
    m_XlateError[ "DEFINE group contains more than one branch" ] = tr( "DEFINE group contains more than one branch" );
    m_XlateError[ "repeating a DEFINE group is not allowed" ] = tr( "repeating a DEFINE group is not allowed" );
    m_XlateError[ "inconsistent NEWLINE options" ] = tr( "inconsistent NEWLINE options" );
    m_XlateError[ "\\g is not followed by a braced, angle-bracketed, or quoted name/number or by a plain number" ] = tr( "\\g is not followed by a braced, angle-bracketed, or quoted name/number or by a plain number" );
    m_XlateError[ "a numbered reference must not be zero" ] = tr( "a numbered reference must not be zero" );
    m_XlateError[ "an argument is not allowed for (*ACCEPT), (*FAIL), or (*COMMIT)" ] = tr( "an argument is not allowed for (*ACCEPT), (*FAIL), or (*COMMIT)" );
    m_XlateError[ "(*VERB) not recognized or malformed" ] = tr( "(*VERB) not recognized or malformed" );
    m_XlateError[ "number is too big" ] = tr( "number is too big" );
    m_XlateError[ "subpattern name expected" ] = tr( "subpattern name expected" );
    m_XlateError[ "digit expected after (?+" ] = tr( "digit expected after (?+" );
    m_XlateError[ "] is an invalid data character in JavaScript compatibility mode" ] = tr( "] is an invalid data character in JavaScript compatibility mode" );
    m_XlateError[ "different names for subpatterns of the same number are not allowed" ] = tr( "different names for subpatterns of the same number are not allowed" );
    m_XlateError[ "(*MARK) must have an argument" ] = tr( "(*MARK) must have an argument" );
    m_XlateError[ "this version of PCRE is not compiled with Unicode property support" ] = tr( "this version of PCRE is not compiled with Unicode property support" );
    m_XlateError[ "\\c must be followed by an ASCII character" ] = tr( "\\c must be followed by an ASCII character" );
    m_XlateError[ "\\k is not followed by a braced, angle-bracketed, or quoted name" ] = tr( "\\k is not followed by a braced, angle-bracketed, or quoted name" );
    m_XlateError[ "internal error: unknown opcode in find_fixedlength()" ] = tr( "internal error: unknown opcode in find_fixedlength()" );
    m_XlateError[ "\\N is not supported in a class" ] = tr( "\\N is not supported in a class" );
    m_XlateError[ "too many forward references" ] = tr( "too many forward references" );
    m_XlateError[ "disallowed Unicode code point (>= 0xd800 && <= 0xdfff)" ] = tr( "disallowed Unicode code point (>= 0xd800 && <= 0xdfff)" );
    m_XlateError[ "invalid UTF-16 string" ] = tr( "invalid UTF-16 string" );
    m_XlateError[ "name is too long in (*MARK), (*PRUNE), (*SKIP), or (*THEN)" ] = tr( "name is too long in (*MARK), (*PRUNE), (*SKIP), or (*THEN)" );
    m_XlateError[ "character value in \\u.... sequence is too large" ] = tr( "character value in \\u.... sequence is too large" );
    m_XlateError[ "invalid UTF-32 string" ] = tr( "invalid UTF-32 string" );
    m_XlateError[ "setting UTF is disabled by the application" ] = tr( "setting UTF is disabled by the application" );
    m_XlateError[ "non-hex character in \\x{} (closing brace missing?)" ] = tr( "non-hex character in \\x{} (closing brace missing?)" );
    m_XlateError[ "non-octal character in \\o{} (closing brace missing?)" ] = tr( "non-octal character in \\o{} (closing brace missing?)" );
    m_XlateError[ "missing opening brace after \\o" ] = tr( "missing opening brace after \\o" );
    m_XlateError[ "parentheses are too deeply nested" ] = tr( "parentheses are too deeply nested" );
    m_XlateError[ "invalid range in character class" ] = tr( "invalid range in character class" );
    m_XlateError[ "group name must start with a non-digit" ] = tr( "group name must start with a non-digit" );
    m_XlateError[ "parentheses are too deeply nested (stack check)" ] = tr( "parentheses are too deeply nested (stack check)" );
    m_XlateError[ "digits missing in \\x{} or \\o{}" ] = tr( "digits missing in \\x{} or \\o{}" );
}
