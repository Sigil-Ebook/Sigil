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

#include <stdafx.h>
#include "PCREReplaceTextBuilder.h"
#include "Misc/Utility.h"

PCREReplaceTextBuilder::PCREReplaceTextBuilder()
{
    resetState();
}

bool PCREReplaceTextBuilder::BuildReplacementText(SPCRE &sre,
                                                  const QString &text,
                                                  const QList<std::pair<int, int> > &capture_groups_offsets,
                                                  const QString &replacement_pattern,
                                                  QString &out)
{
    resetState();

    if (!sre.isValid()) {
        return false;
    }

    // Check if the \ start control is in the string.
    // If it's not we don't need to run though the replacment code and we
    // can just return the pattern as the replaced text.
    // This is a simple and quick way that will catch a large number of
    // cases but not all.
    if (!replacement_pattern.contains("\\")) {
        out = replacement_pattern;
        return true;
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
    // numbered and named with the text matched by the regex. As well as
    // do any required case chagnes.
    //
    // We do a linear replacement one character at a time instead of using
    // a regex because we don't want false positives or replacments
    // within subpatterns. E.G. replace \g{1a} with back reference 1 by
    // accident. It's also faster to do one pass through the string instead
    // of multiple times by using multiple regexes.
    //
    // Back references can be:
    // \# where # is 0 to 9.
    // \g{#s} where #s can be 0 to capture_groups_offsets.count().
    // \g<#s>  where #s can be 0 to capture_groups_offsets.count().
    // \g{text} where text can be any string.
    // \g<text> where text can be any string.
    //
    // Case changes can be:
    // \l Lower case next character.
    // \u Upper case next character.
    // \L Lower case until \E.
    // \U Upper case until \E.
    // \E End case modification.
    // * Note: case changes cannot stop within a segment. Meaning
    // a \L within a \U will be ignored and the \U will be honored until
    // \E is encountered.
    for (int i = 0; i < replacement_pattern.length(); i++)
    {
        c = replacement_pattern.at(i);

        if (in_control)
        {
            // Store characters incase this is an invalid control and we
            // need to put it in the final text.
            invalid_contol += c;

            // This is the first character after the \\ start of control.
            if (control_char.isNull()) {
                // Store the control character so we know we what it is.
                control_char = c;

                // Process the control character.
                // # is special because it's a numbered back reference.
                // We processed it here.
                if (c.isDigit()) {
                    int backref_number = c.digitValue();

                    // Check if this number is a back reference we can
                    // actually get.
                    if (backref_number >= 0 && backref_number < capture_groups_offsets.count()) {
                        accumulateReplcementText(Utility::Substring(capture_groups_offsets.at(backref_number).first, capture_groups_offsets.at(backref_number).second, text));
                    }
                    else {
                        accumulateReplcementText(invalid_contol);
                    }

                    in_control = false;
                }
                // End case change.
                else if (c == 'E') {
                    m_caseChangeState = CaseChange_None;
                    in_control = false;
                }
                // Backreference.
                else if (c == 'g') {
                    backref_bracket_start_char = QChar();
                }
                // Lower case next character.
                else if (c == 'l') {
                    trySetCaseChange(CaseChange_LowerNext);
                    in_control = false;
                }
                // Lower case until \E.
                else if (c == 'L') {
                    trySetCaseChange(CaseChange_Lower);
                    in_control = false;
                }
                // Upper case next character.
                else if (c == 'u') {
                    trySetCaseChange(CaseChange_UpperNext);
                    in_control = false;
                }
                // Upper case until \E.
                else if (c == 'U') {
                    trySetCaseChange(CaseChange_Upper);
                    in_control = false;
                }
            }
            // We know the control character.
            else {
                if (control_char == 'g') {
                    if ( backref_bracket_start_char.isNull() ) {
                        // We only support named references within
                        // {} and <>.
                        if (c == '{' || c == '<') {
                            backref_bracket_start_char = c;
                            backref_name.clear();
                        }
                        else {
                            in_control = false;
                            accumulateReplcementText(invalid_contol);
                        }
                    }
                    else {
                        if ((c == '}' && backref_bracket_start_char == '{') ||
                                (c == '>' && backref_bracket_start_char == '<'))
                        {
                            // Either we have a back reference number in the bracket
                            // or we have a name which we will convert to a number.
                            int backref_number;

                            // Try to convert the backref name to a number.
                            backref_number = backref_name.toInt();
                            // The backref wasn't a number so get the number for the name.
                            if (backref_number == 0 && backref_name != "0") {
                                backref_number = sre.getCaptureStringNumber(backref_name);
                            }

                            // Check if there is we have a back reference we can
                            // actually get.
                            if (backref_number >= 0 && backref_number < capture_groups_offsets.count()) {
                                accumulateReplcementText(Utility::Substring(capture_groups_offsets.at(i).first, capture_groups_offsets.at(i).second, text));
                            }
                            else {
                                accumulateReplcementText(invalid_contol);
                            }

                            in_control = false;
                        }
                        else {
                            backref_name += c;
                        }
                    }
                }
                // Invalid or unsupported control.
                else {
                    accumulateReplcementText(invalid_contol);
                    in_control = false;
                }
            }
        }
        // We're not in a control.
        else
        {
            // Start a control character.
            if (c == '\\') {
                // Reset our invalid control accumulator.
                invalid_contol = c;
                // Reset the control character that is after this
                control_char = QChar();
                // We are now in a control.
                in_control = true;
            }
            // Normal text.
            else {
                accumulateReplcementText(c);
            }
        }
    }
    // If we ended reading the replacement string and we're still in
    // a back reference then we have an invalid back reference because
    // it never ended. Put the invalid reference into the replacment string.
    if (in_control) {
        accumulateReplcementText(invalid_contol);
    }

    out = m_finalText;
    return true;
}

void PCREReplaceTextBuilder::accumulateReplcementText(const QChar &ch)
{
    m_finalText += processTextSegement(ch);
}

void PCREReplaceTextBuilder::accumulateReplcementText(const QString &text)
{
    m_finalText += processTextSegement(text);
}

QString PCREReplaceTextBuilder::processTextSegement(const QChar &ch)
{
    return processTextSegement(QString(ch));
}

QString PCREReplaceTextBuilder::processTextSegement(const QString &text)
{
    QString processedText;

    if (text.length() == 0) {
        return processedText;
    }

    switch (m_caseChangeState) {
    case CaseChange_LowerNext:
        processedText += text.at(0).toLower();
        processedText += text.mid(1);
        m_caseChangeState = CaseChange_None;
        break;
    case CaseChange_Lower:
        processedText += text.toLower();
        break;
    case CaseChange_UpperNext:
        processedText += text.at(0).toUpper();
        processedText += text.mid(1);
        m_caseChangeState = CaseChange_None;
        break;
    case CaseChange_Upper:
        processedText += text.toUpper();
        break;
    default:
        processedText += text;
        break;
    }

    return processedText;
}

void PCREReplaceTextBuilder::trySetCaseChange(CaseChange state)
{
    if (m_caseChangeState == CaseChange_None) {
        m_caseChangeState = state;
    }
}

void PCREReplaceTextBuilder::resetState()
{
    m_finalText.clear();
    m_caseChangeState = CaseChange_None;
}
