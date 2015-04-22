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

#include "pcre/SPCRE.h"
#include "pcre/PCREReplaceTextBuilder.h"
#include "sigil_constants.h"

// The maximum number of catpures that we will allow.
const int PCRE_MAX_CAPTURE_GROUPS = 30;

SPCRE::SPCRE(const QString &patten)
{
    m_pattern = patten;
    m_re = NULL;
    m_study = NULL;
    m_captureSubpatternCount = 0;
    const char *error;
    int erroroffset;
    m_re = pcre16_compile(m_pattern.utf16(), PCRE_UTF16 | PCRE_MULTILINE, &error, &erroroffset, NULL);

    // Pattern is valid.
    if (m_re != NULL) {
        m_valid = true;
        // Study the pattern and save the results of the study.
        m_study = pcre16_study(m_re, 0, &error);
        // Store the number of capture subpatterns.
        pcre16_fullinfo(m_re, m_study, PCRE_INFO_CAPTURECOUNT, &m_captureSubpatternCount);
    }
    // Pattern is not valid.
    else {
        m_valid = false;
    }
}

SPCRE::~SPCRE()
{
    if (m_re != NULL) {
        pcre16_free(m_re);
        m_re = NULL;
    }

    if (m_study != NULL) {
        pcre16_free(m_study);
        m_study = NULL;
    }
}

bool SPCRE::isValid()
{
    return m_valid;
}

QString SPCRE::getPattern()
{
    return m_pattern;
}

pcre16 *SPCRE::getCompiledPattern()
{
    return m_re;
}

pcre16_extra *SPCRE::getStudy()
{
    return m_study;
}

int SPCRE::getCaptureSubpatternCount()
{
    return m_captureSubpatternCount;
}

int SPCRE::getCaptureStringNumber(const QString &name)
{
    if (m_re == NULL) {
        return -1;
    }

    int number = pcre16_get_stringnumber(m_re, name.utf16());

    if (number == 0) {
        number = -1;
    }

    return number;
}

QList<SPCRE::MatchInfo> SPCRE::getEveryMatchInfo(const QString &text)
{
    // This function is very similar to getNextMatchInfo but we don't
    // want to put a call to getNextMatchInfo in a loop because it allocates
    // a new ovector. We want to avoid this and only do one allocation so we
    // reuse the logic and put the call to generateMatchInfo in the loop.
    QList<SPCRE::MatchInfo> info;

    if (m_re == NULL || text.isEmpty()) {
        return info;
    }

    int rc = 0;
    // Set the size of the array based on the number of capture subpatterns
    // if it does not exceed our maximum size.
    int ovector_count = getCaptureSubpatternCount();

    if (ovector_count > PCRE_MAX_CAPTURE_GROUPS) {
        ovector_count = PCRE_MAX_CAPTURE_GROUPS;
    }

    // The vector needs to be a multiple of 3 and have at least one location
    // for the full matched string.
    int ovector_size = (1 + ovector_count) * 3;
    // Allocate only the amount of memory we need for the search pattern and
    // the number of capture patterns it contains.
    // Can't use dynamic array alloction (int ovector[ovector_size]) because
    // MSVC doesn't support it.
    int *ovector = new int[ovector_size];
    memset(ovector, 0, sizeof(int)*ovector_size);
    // We keep track of the last offsets as we move though the string matching
    // sub strings.
    int last_offset[2] = {0};

    // Run until no matches are found.
    do {
        // Store the matching offsets so we can tell when we need to quit this
        // loop.
        last_offset[0] = ovector[0];
        last_offset[1] = ovector[1];

        // We only care about matches that have text in it.
        if (last_offset[0] != last_offset[1]) {
            // Add the matched information to the list.
            info.append(generateMatchInfo(ovector, ovector_count));
        }

        rc = pcre16_exec(m_re, m_study, text.utf16(), text.length(), last_offset[1], 0, ovector, ovector_size);
    } while (rc >= 0 && ovector[0] != ovector[1] && ovector[1] != last_offset[1] && ovector[0] < ovector[1]);

    delete[] ovector;
    return info;
}

SPCRE::MatchInfo SPCRE::getFirstMatchInfo(const QString &text)
{
    SPCRE::MatchInfo match_info;

    if (m_re == NULL || text.isEmpty()) {
        return match_info;
    }

    int rc = 0;
    // Set the size of the array based on the number of capture subpatterns
    // if it does not exceed our maximum size.
    int ovector_count = getCaptureSubpatternCount();

    if (ovector_count > PCRE_MAX_CAPTURE_GROUPS) {
        ovector_count = PCRE_MAX_CAPTURE_GROUPS;
    }

    // The vector needs to be a multiple of 3 and have at least one location
    // for the full matched string.
    int ovector_size = (1 + ovector_count) * 3;
    // Allocate only the amount of memory we need for the search pattern and
    // the number of capture patterns it contains.
    // Can't use dynamic array alloction (int ovector[ovector_size]) because
    // MSVC doesn't support it.
    int *ovector = new int[ovector_size];
    memset(ovector, 0, sizeof(int)*ovector_size);
    rc = pcre16_exec(m_re, m_study, text.utf16(), text.length(), 0, 0, ovector, ovector_size);

    if (rc >= 0 && ovector[0] != ovector[1]) {
        match_info = generateMatchInfo(ovector, ovector_count);
    }

    delete[] ovector;
    return match_info;
}

SPCRE::MatchInfo SPCRE::getLastMatchInfo(const QString &text)
{
    QList<SPCRE::MatchInfo> info;
    info = getEveryMatchInfo(text);

    if (!info.isEmpty()) {
        return info.last();
    } else {
        return SPCRE::MatchInfo();
    }
}

bool SPCRE::replaceText(const QString &text, const QList<std::pair<int, int>> &capture_groups_offsets, const QString &replacement_pattern, QString &out)
{
    PCREReplaceTextBuilder builder;
    return builder.BuildReplacementText(*this, text, capture_groups_offsets, replacement_pattern, out);
}

SPCRE::MatchInfo SPCRE::generateMatchInfo(int ovector[], int ovector_count)
{
    MatchInfo match_info;
    // Store the offsets in the QString text that we ar matching
    // against.
    int match_start = ovector[0];
    int match_end = ovector[1];
    match_info.offset = std::pair<int, int>(match_start, match_end);
    // We keep a list of the substrings within the matched string that
    // are captured by capture patterns.
    //
    // The first match is always the string itself.
    match_info.capture_groups_offsets.append(std::pair<int, int>(0, match_end - match_start));

    // Translate the subpattern offsets into locations within the
    // matched substring.
    for (int i = 1; i <= ovector_count; i++) {
        int subpattern_start = ovector[2 * i] - match_start;
        int subpattern_end = ovector[2 * i + 1] - match_start;
        match_info.capture_groups_offsets.append(std::pair<int, int>(subpattern_start, subpattern_end));
    }

    return match_info;
}

