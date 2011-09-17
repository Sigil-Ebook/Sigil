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


#include "SPCRE.h"
#include "PCREReplaceTextBuilder.h"
#include "constants.h"

SPCRE::SPCRE(const QString &patten)
{
    m_pattern = patten;

    const char *error;
    int erroroffset;
    m_re = pcre_compile(m_pattern.toUtf8().data(), PCRE_UTF8 | PCRE_MULTILINE, &error, &erroroffset, NULL);

    // Pattern is valid.
    if (m_re != NULL) {
        m_valid = true;
        // Study the pattern and save the results of the study.
        m_study = pcre_study(m_re, 0, &error);

        // Store the number of capture subpatterns.
        pcre_fullinfo(m_re, m_study, PCRE_INFO_CAPTURECOUNT, &m_captureSubpatternCount);
    }
    // Pattern is not valid.
    else {
        m_valid = false;
    }
}

SPCRE::~SPCRE()
{
    pcre_free(m_re);
    pcre_free(m_study);
}

bool SPCRE::isValid()
{
    return m_valid;
}

QString SPCRE::getPattern()
{
    return m_pattern;
}

pcre *SPCRE::getCompiledPattern()
{
    return m_re;
}

pcre_extra *SPCRE::getStudy()
{
    return m_study;
}

int SPCRE::getCaptureSubpatternCount()
{
    return m_captureSubpatternCount;
}

QList<std::pair<int, int> > SPCRE::getMatchOffsets(const QString &text)
{
    QList<std::pair<int, int> > offsets;

    int rc = 0;
    // We only care about the first match with is the full length of the match.
    int ovector[3] = {0};
    int last_offset[2] = {0};
    char *utf_data = text.toUtf8().data();

    // Run until no matches are found.
    do {
        // Store the matching offsets so we can tell when we need to quit this
        // loop.
        last_offset[0] = ovector[0];
        last_offset[1] = ovector[1];

        // We only care about matches that have text in it.
        if (last_offset[0] != last_offset[1]) {
            // pcre works on char*'s and holds UTF-8 data as a sequence of
            // chars. Meaning a UTF-8 character like ’ will be stored across
            // multiple chars. ’ for instance is 3 chars. pcre understands these
            // sequences as a single character but the offsets within the
            // sequence as returned by pcre_exec are the span of all chars that
            // represent the single UTF-8 character. For example ’ starts at 10
            // and the ending offset is 13.
            //
            // QString is a UTF-16 capable data store. It represents a single
            // UTF-8 character as 1. This is unlike pcre with can represent
            // a singe character as a variable amount of chars.
            //
            // Due to the difference in offsets between a char* and QString
            // we need translate the offsets.

            // Temporary QString. We load the char* data into it to figure out
            // how many characters are represented by the data.
            QString tmp_str;
            // The difference up until last_offset[0] between the char* and
            // tmp_str.
            int offset_diff = 0;
            // The differece in length between the matched text and it's length
            // as a QString.
            int len_diff = 0;

            // Load the data into a QString and find out what the difference
            // between the offset and the character count is.
            tmp_str = QString::fromUtf8(utf_data, last_offset[0]);
            offset_diff = last_offset[0] - tmp_str.length();

            // Load the matched into the QString and find out how many
            // characters it is.
            tmp_str = QString::fromUtf8(utf_data + last_offset[0], last_offset[1] - last_offset[0]);
            len_diff = last_offset[1] - last_offset[0] - tmp_str.length();

            // Store the offsets in the QString text that we ar matching
            // against.
            std::pair<int, int> offset(last_offset[0] - offset_diff, last_offset[1] - offset_diff - len_diff);
            offsets.append(offset);
        }

        rc = pcre_exec(m_re, m_study, utf_data, strlen(utf_data), last_offset[1], 0, ovector, 3);
    } while(rc >= 0  && ovector[0] != ovector[1] && ovector[1] != last_offset[1]);

    return offsets;
}

bool SPCRE::replaceText(const QString &text, const QString &replacement_pattern, QString &out)
{
    PCREReplaceTextBuilder builder;
    return builder.BuildReplacementText(*this, text, replacement_pattern, out);
}
