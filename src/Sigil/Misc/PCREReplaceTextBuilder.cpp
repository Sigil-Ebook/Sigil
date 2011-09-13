/************************************************************************
**
**  Copyright (C) 2011  John Schember <john@nachtimwald.com>
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

#include "stdafx.h"
#include "PCREReplaceTextBuilder.h"

#include <QRegExp>
#include "pcre.h"

#include "Utility.h"

const int PCRE_OVECTOR_SIZE = ( 1 + PCRE_MAX_GROUPS ) * 3;

PCREReplaceTextBuilder::PCREReplaceTextBuilder()
{
    resetState();
}

bool PCREReplaceTextBuilder::BuildReplacementText(const QString &search_regex,
                                             const QString &matched_text,
                                             const QString &replacement_pattern,
                                             QString &replacement_text)
{
    resetState();
    bool replacement_made = false;

    pcre *re;
    const char *error;
    int erroroffset;

    re = pcre_compile( search_regex.toUtf8().data(), PCRE_UTF8 | PCRE_MULTILINE, &error, &erroroffset, NULL );
    // compile faliure
    if ( re == NULL )
    {
        return replacement_made;
    }

    int rc = 0;
    // The vector needs to be a multiple of 3.
    // N match items * 3 = our total size.
    // We only want the first match which is the entire string.
    int ovector[PCRE_OVECTOR_SIZE];
    QByteArray utf_str = matched_text.toUtf8();

    rc = pcre_exec( re, NULL, utf_str.data(), utf_str.length(), 0, 0, ovector, PCRE_OVECTOR_SIZE );

    // Match succeeded. Go forward with replacing the matched text with
    // the replacement pattern.
    if ( rc >= 0 )
    {
        // If rc is 0 then we have more groups then our maximum allowed number.
        if ( rc == 0 )
        {
            rc = PCRE_MAX_GROUPS;
        }

        // Tempory character used as we loop though all characters so we can
        // determine if it is a control character or text.
        QChar c;
        // Named back referecnes can be in within {} or <>. Store which
        // character we've matched so we can match the appropriate closing
        // character.
        QChar backref_bracket_start_char;
        QChar control_char;
        // Stores a named back reference we build as we parse the string.
        QString backref_name;
        QString invalid_contol;
        // The state of our progress through the replacment string.
        bool in_control = false;

        // We are going to parse the replacment pattern one character at a time
        // to build the final replacement string. We need to replace subpatterns
        // numbered and named with the text matched by the regex.
        //
        // We do a linear replacement one character at a time instead of using
        // a regex because we don't want false positives or replacments
        // within subpatterns. E.G. replace \g{1a} with back reference 1 by
        // accident. It's also faster to do one pass through the string instead
        // of multiple times by using multiple regexes.
        //
        // Back references can be:
        // \# where # is 1 to 9.
        // \g# where # is 1 to 9.
        // \g{#s} where #s can be 1 to PCRE_MAX_GROUPS.
        // \g<#s>  where #s can be 1 to PCRE_MAX_GROUPS.
        // \g{text} where text can be any string.
        // \g<text> where text can be any string.
        for ( int i = 0; i < replacement_pattern.length(); i++ )
        {
            c = replacement_pattern.at( i );

            if ( in_control )
            {
                // Accumulate characters incase this is an invalid control.
                invalid_contol += c;

                // This is the first character after the \\ start of control.
                if ( control_char.isNull() )
                {
                    // Store the control character so we know we've already
                    // processed it.
                    control_char = c;

                    // Process the control character.
                    // # is special because it's a numbered back reference.
                    // We processed it here.
                    if ( c.isDigit() )
                    {
                        int backref_number = c.digitValue();

                        // Check if this number is a back reference we can
                        // actually get.
                        if ( backref_number > 0 && backref_number <= rc )
                        {
                            accumulateReplcementText(Utility::Substring( ovector[2 * backref_number], ovector[2 * backref_number + 1], matched_text ));
                        }
                        else
                        {
                            accumulateReplcementText(invalid_contol);
                        }

                        in_control = false;
                    }
                    // Quit the control.
                    else if ( c == 'E' )
                    {
                        m_caseChangeState = CaseChange_None;
                        in_control = false;
                    }
                    // Backreference.
                    else if ( c == 'g' )
                    {
                        backref_bracket_start_char = QChar();
                    }
                    // Lower case next character.
                    else if ( c == 'l' )
                    {
                        trySetCaseChange(CaseChange_LowerNext);
                        in_control = false;
                    }
                    // Lower case until \E.
                    else if ( c == 'L' )
                    {
                        trySetCaseChange(CaseChange_Lower);
                        in_control = false;
                    }
                    // Upper case next character.
                    else if ( c == 'u' )
                    {
                        trySetCaseChange(CaseChange_UpperNext);
                        in_control = false;
                    }
                    // Upper case until \E.
                    else if ( c == 'U' )
                    {
                        trySetCaseChange(CaseChange_Upper);
                        in_control = false;
                    }
                }
                else
                {
                    if ( control_char == 'g' )
                    {
                        if ( backref_bracket_start_char.isNull() )
                        {
                            if ( c == '{' || c == '<' ) {
                                backref_bracket_start_char = c;
                                backref_name.clear();
                            }
                            else {
                                in_control = false;
                                accumulateReplcementText(invalid_contol);
                            }
                        }
                        else
                        {
                            if ( ( c == '}' && backref_bracket_start_char == '{' ) ||
                                 ( c == '>' && backref_bracket_start_char == '<' )
                               )
                            {
                                // Either we have a back reference number in the bracket
                                // or we have a name which we will convert to a number.
                                int backref_number;

                                // Try to convert the backref name to a number.
                                backref_number = backref_name.toInt();
                                // The backref wasn't a number so get the number for the name.
                                if ( backref_number == 0 )
                                {
                                    backref_number = pcre_get_stringnumber( re, backref_name.toUtf8().data() );
                                }

                                // Check if there is we have a back reference we can
                                // actually get.
                                if ( backref_number > 0 && backref_number <= rc )
                                {
                                    accumulateReplcementText(Utility::Substring( ovector[2 * backref_number], ovector[2 * backref_number + 1], matched_text ));
                                }
                                else
                                {
                                   accumulateReplcementText(invalid_contol);
                                }

                                in_control = false;
                            }
                            else
                            {
                                backref_name += c;
                            }
                        }
                    }
                    // Invalid or unsupported control.
                    else
                    {
                        accumulateReplcementText(invalid_contol);
                        in_control = false;
                    }
                }
            }
            // We're not in a back reference.
            else
            {
                // Start a control character.
                if ( c == '\\' )
                {
                    // Reset our invalid control accumulator.
                    invalid_contol = c;
                    // Reset the control character that is after this
                    control_char = QChar();
                    // We are now in a control.
                    in_control = true;
                }
                // Normal text.
                else
                {
                    accumulateReplcementText(c);
                }
            }
        }
        // If we ended reading the replacement string and we're still in
        // a back reference then we have an invalid back reference because
        // it never ended. Put the invalid reference into the replacment string.
        if ( in_control )
        {
            accumulateReplcementText(invalid_contol);
        }

        replacement_made = true;
    }

    pcre_free( re );

    replacement_text = m_finalText;
    return replacement_made;
}

void PCREReplaceTextBuilder::accumulateReplcementText(const QChar &ch)
{
    accumulateReplcementText( QString(ch));
}

void PCREReplaceTextBuilder::accumulateReplcementText(const QString &text)
{
    processTextSegement(text);
}

void PCREReplaceTextBuilder::processTextSegement(const QChar &ch)
{
    processTextSegement(QString(ch));
}

void PCREReplaceTextBuilder::processTextSegement(const QString &text)
{
    if (text.length() == 0) {
        return;
    }

    switch (m_caseChangeState) {
    case CaseChange_LowerNext:
        m_finalText += text.at(0).toLower();
        m_finalText += text.mid(1);
        break;
    case CaseChange_Lower:
        m_finalText += text.toLower();
        break;
    case CaseChange_UpperNext:
        m_finalText += text.at(0).toUpper();
        m_finalText += text.mid(1);
        break;
    case CaseChange_Upper:
        m_finalText += text.toUpper();
        break;
    default:
        m_finalText += text;
        break;
    }
}

void PCREReplaceTextBuilder::trySetCaseChange(CaseChange state)
{
    if (m_caseChangeState == CaseChange_None)
    {
        m_caseChangeState = state;
    }
}

void PCREReplaceTextBuilder::resetState()
{
    m_finalText.clear();

    m_inControl = false;
    m_inBackref = false;
    m_caseChangeState = CaseChange_None;
}
