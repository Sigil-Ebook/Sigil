/************************************************************************
**
**  Copyright (C) 2021  Kevin B. Hendricks, Stratford Ontario Canada
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

#include <QString>
// #include <QDebug>

#include "PCRE2/SPCRE.h"
#include "PCRE2/PCREReplaceTextBuilder.h"
#include "Misc/Utility.h"
#include "sigil_constants.h"

#define PCRE_NO_JIT 1

// The maximum number of catpures that we will allow.
const int PCRE_MAX_CAPTURE_GROUPS = 30;

SPCRE::SPCRE(const QString &patten)
{
    m_pattern = patten;
    m_re = NULL;
    m_matchdata = NULL;
    m_mcontext = NULL;

#ifndef PCRE_NO_JIT
    m_jitstack = NULL;
#endif

    m_captureSubpatternCount = 0;
    m_error = QString();
    m_errpos = -1;
    int errorno = -1;
    PCRE2_SIZE erroroffset = 0;
    m_re = pcre2_compile_16(m_pattern.utf16(), PCRE2_ZERO_TERMINATED, PCRE2_UTF | PCRE2_MULTILINE, &errorno, &erroroffset, NULL);

    // Pattern is valid.
    if (m_re != NULL) {
        m_valid = true;
        m_matchdata = pcre2_match_data_create_from_pattern_16(m_re, NULL);


#ifndef PCRE_NO_JIT
        int rc = pcre2_jit_compile_16(m_re, PCRE2_JIT_COMPLETE);
        if (rc == 0) {
            m_mcontext = pcre2_match_context_create_16(NULL);
            m_jitstack = pcre2_jit_stack_create_16(32*1024, 1024*1024, NULL);
            if (m_jitstack != NULL) {
                pcre2_jit_stack_assign_16(m_mcontext, NULL, m_jitstack);
            }
        }
#endif

        // Store the number of capture patterns (pairs).
        // pcre2_pattern_info_16(m_re, PCRE2_INFO_CAPTURECOUNT, &m_captureSubpatternCount);
        m_captureSubpatternCount = pcre2_get_ovector_count_16(m_matchdata);
    }
    // Pattern is not valid.
    else {
        m_valid = false;
        PCRE2_UCHAR16 buffer[256];
        pcre2_get_error_message_16(errorno, buffer, sizeof(buffer));
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        m_error = QString::fromUtf16(buffer);
#else
        m_error = QString::fromUtf16(reinterpret_cast<char16_t*>(buffer));
#endif
        m_errpos = erroroffset;
        // qDebug() << "SPCRE invalid pattern: " << m_pattern;
        // qDebug() << "SPCRE error: " << m_error;
        // qDebug() << "SPCRE error position: " << m_errpos;

    }
}

SPCRE::~SPCRE()
{
    if (m_re != NULL) {
        pcre2_code_free_16(m_re);
        m_re = NULL;
    }

    if (m_matchdata != NULL) {
        pcre2_match_data_free_16(m_matchdata);
        m_matchdata = NULL;
    }

#ifndef PCRE_NO_JIT
    if (m_jitstack) {
        pcre2_jit_stack_free_16(m_jitstack);
        m_jitstack = NULL;
    }
    if (m_mcontext) {
        pcre2_match_context_free_16(m_mcontext);
        m_mcontext = NULL;
    }
#endif

}

bool SPCRE::isValid()
{
    return m_valid;
}

QString SPCRE::getError()
{
    return m_error;
}

int SPCRE::getErrPos()
{
    return m_errpos;
}

QString SPCRE::getPattern()
{
    return m_pattern;
}

pcre2_code_16 *SPCRE::getCompiledPattern()
{
    return m_re;
}

int SPCRE::getCaptureSubpatternCount()
{
    return m_captureSubpatternCount;
}

// get capture group number for named capture group
int SPCRE::getCaptureStringNumber(const QString &name)
{
    if (m_re == NULL) {
        return -1;
    }

    uint32_t namecount = 0;
    pcre2_pattern_info(m_re, PCRE2_INFO_NAMECOUNT, &namecount);
    if (namecount == 0) return -1;

    int number = pcre2_substring_number_from_name_16(m_re, name.utf16());

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

    PCRE2_SIZE * ovector = NULL;

    // Set the size of the array based on the number of capture subpatterns
    // if it does not exceed our maximum size.
    int ovector_count = getCaptureSubpatternCount();

    if (ovector_count > PCRE_MAX_CAPTURE_GROUPS) {
        ovector_count = PCRE_MAX_CAPTURE_GROUPS;
    }

    // We keep track of the last offsets as we move though the string matching
    // sub strings.
    int last_offset[2] = {0};
    bool done = false;
    
    // Run until no matches are found.
    do {

        rc = pcre2_match_16(m_re, text.utf16(), text.length(), last_offset[1], 0, m_matchdata, m_mcontext);

        // NOTE: until a call to pcre2_match_16 happens even through m_matchdata exists
        // and the ovector count is known, the pcre2_get_ovector_pointer returns a pointer
        // to invalid ovector data
        ovector = pcre2_get_ovector_pointer_16(m_matchdata);

        done = (ovector[1] == last_offset[1]) || (ovector[0] >= ovector[1]);

        last_offset[0] = ovector[0];
        last_offset[1] = ovector[1];

        if (rc >= 0 && ovector[0] != ovector[1] && ovector[0] < ovector[1]) {
            info.append(generateMatchInfo(ovector, ovector_count));
        }
    } while (rc >= 0 && !done);
    
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
    // int ovector_size = (1 + ovector_count) * 3;
    // Allocate only the amount of memory we need for the search pattern and
    // the number of capture patterns it contains.
    // Can't use dynamic array alloction (int ovector[ovector_size]) because
    // MSVC doesn't support it.
    // int *ovector = new int[ovector_size];
    // memset(ovector, 0, sizeof(int)*ovector_size);
    rc = pcre2_match_16(m_re, text.utf16(), text.length(), 0, 0, m_matchdata, m_mcontext);
    PCRE2_SIZE * ovector = pcre2_get_ovector_pointer_16(m_matchdata);

    if (rc >= 0 && ovector[0] != ovector[1]) {
        match_info = generateMatchInfo(ovector, ovector_count);
    }

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

SPCRE::MatchInfo SPCRE::generateMatchInfo(PCRE2_SIZE* ovector, int capture_pattern_count)
{
    MatchInfo match_info;
    // Store the offsets in the QString text that we are matching
    // against.
    int match_start = ovector[0];
    int match_end = ovector[1];
    match_info.offset = std::pair<int, int>(match_start, match_end);
    // We keep a list of the substrings within the matched string that
    // are captured by capture patterns.
    // The capture_pattern_count is the number of begin, end pairs in the ovector
    //
    // The first match is always the string itself.
    match_info.capture_groups_offsets.append(std::pair<int, int>(0, match_end - match_start));

    // Translate the subpattern offsets into locations within the
    // matched substring.
    for (int i = 1; i < capture_pattern_count; i++) {
        int subpattern_start = ovector[2 * i] - match_start;
        int subpattern_end = ovector[2 * i + 1] - match_start;
        match_info.capture_groups_offsets.append(std::pair<int, int>(subpattern_start, subpattern_end));
    }

    return match_info;
}
