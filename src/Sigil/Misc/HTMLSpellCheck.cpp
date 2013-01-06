/************************************************************************
**
**  Copyright (C) 2009, 2010, 2011  Strahinja Markovic  <strahinja.markovic@gmail.com>
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

#include <QtCore/QRegExp>
#include <QtCore/QString>
#include <QtCore/QTextCodec>
#include <QRegularExpression>

#include "Misc/HTMLEncodingResolver.h"
#include "Misc/Utility.h"
#include "Misc/SpellCheck.h"
#include "Misc/HTMLSpellCheck.h"
#include "sigil_constants.h"
#include "sigil_exception.h"

const int MAX_WORD_LENGTH  = 90;

QList< HTMLSpellCheck::MisspelledWord > HTMLSpellCheck::GetMisspelledWords(const QString &orig_text,
        int start_offset,
        int end_offset,
        const QString &search_regex,
        bool first_only,
        bool include_all_words)
{
    SpellCheck *sc = SpellCheck::instance();
    bool in_tag = false;
    bool in_invalid_word = false;
    bool in_entity = false;
    int word_start = 0;
    QRegularExpression search(search_regex);
    QList< HTMLSpellCheck::MisspelledWord > misspellings;
    // Make sure text has beginning/end boundary markers for easier parsing
    QString text = QChar(' ') + orig_text + QChar(' ');
    // Ignore <style...</style> wherever it appears - change to spaces to keep text positions
    QRegularExpression style_re("<style[^<]*</style>");

    QRegularExpressionMatchIterator i = style_re.globalMatch(text);
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        for (int pos = match.capturedStart(0); pos < match.capturedStart(0) + match.capturedRef(0).size(); pos++) {
            text[pos] = QChar(' ');
        }
    }

    for (int i = 0; i < text.count(); i++) {
        QChar c = text.at(i);

        if (!in_tag) {
            QChar prev_c = i > 0 ? text.at(i - 1) : QChar(' ');
            QChar next_c = i < text.count() - 1 ? text.at(i + 1) : QChar(' ');

            if (IsBoundary(prev_c, c, next_c)) {
                // If we're in an entity and we hit a boundary and it isn't
                // part of an entity then this is an invalid entity.
                if (in_entity && c != QChar(';')) {
                    in_entity = false;
                }

                // Check possibilities that would mean this isn't a word worth considering.
                if (!in_invalid_word && !in_entity && word_start != -1 && (i - word_start) > 0) {
                    QString word = Utility::Substring(word_start, i, text);

                    if (!word.isEmpty() && word_start > start_offset && word_start <= end_offset) {
                        if (include_all_words || !sc->spell(word)) {
                            int cap_start = -1;

                            if (!search_regex.isEmpty()) {
                                QRegularExpressionMatch mo = search.match(word);
                                cap_start = mo.capturedStart(0);
                            }

                            if (search_regex.isEmpty() || cap_start != -1) {
                                struct MisspelledWord misspelled_word;
                                misspelled_word.text = word;
                                // Make sure we account for the extra boundary added at the beginning
                                misspelled_word.offset = word_start - 1;
                                misspelled_word.length = i - word_start ;
                                misspellings.append(misspelled_word);

                                if (first_only) {
                                    return misspellings;
                                }
                            }
                        }
                    }
                }

                // We want to start the word with the character after the boundary.
                // If the next character is another boundary we'll just move forwad one.
                word_start = i + 1;
                in_invalid_word = false;
            } else {
                // Ensure we're not dealing with some crazy run on text that isn't worth
                // considering as an actual word.
                if (!in_invalid_word && (i - word_start) > MAX_WORD_LENGTH) {
                    in_invalid_word = true;
                }
            }

            if (c == QChar('&')) {
                in_entity = true;
            }

            if (c == QChar(';')) {
                in_entity = false;
            }
        }

        if (c == QChar('<')) {
            in_tag = true;
            word_start = -1;
        }

        if (in_tag && c == QChar('>')) {
            word_start = i + 1;
            in_tag = false;
        }
    }

    return misspellings;
}

bool HTMLSpellCheck::IsBoundary(QChar prev_c, QChar c, QChar next_c)
{
    if (c.isLetter()) {
        return false;
    }

    // Single quotes of ' and curly version and hyphen/emdash are sometimes a boundary
    // and sometimes not, depending on whether they are surrounded by letters or not.
    // A sentence which 'has some text' should treat the ' as a boundary but didn't should not.
    bool is_potential_boundary = (c == '-' || c == QChar(0x2012) || c == '\'' || c == QChar(0x2019));

    if (is_potential_boundary && (!prev_c.isLetter() || !next_c.isLetter())) {
        return true;
    }

    return !(is_potential_boundary && (prev_c.isLetter() || next_c.isLetter()));
}


QList< HTMLSpellCheck::MisspelledWord > HTMLSpellCheck::GetMisspelledWords(const QString &text)
{
    return GetMisspelledWords(text, 0, text.count(), "");
}


HTMLSpellCheck::MisspelledWord HTMLSpellCheck::GetFirstMisspelledWord(const QString &text,
        int start_offset,
        int end_offset,
        const QString &search_regex)
{
    QList< HTMLSpellCheck::MisspelledWord > misspelled_words = GetMisspelledWords(text, start_offset, end_offset, search_regex, true);
    HTMLSpellCheck::MisspelledWord misspelled_word;

    if (!misspelled_words.isEmpty()) {
        misspelled_word = misspelled_words.first();
    }

    return misspelled_word;
}


HTMLSpellCheck::MisspelledWord HTMLSpellCheck::GetLastMisspelledWord(const QString &text,
        int start_offset,
        int end_offset,
        const QString &search_regex)
{
    QList< HTMLSpellCheck::MisspelledWord > misspelled_words = GetMisspelledWords(text, start_offset, end_offset, search_regex);
    HTMLSpellCheck::MisspelledWord misspelled_word;

    if (!misspelled_words.isEmpty()) {
        misspelled_word = misspelled_words.last();
    }

    return misspelled_word;
}


int HTMLSpellCheck::CountMisspelledWords(const QString &text,
        int start_offset,
        int end_offset,
        const QString &search_regex,
        bool first_only,
        bool include_all_words)
{
    return GetMisspelledWords(text, start_offset, end_offset, search_regex, first_only, include_all_words).count();
}


int HTMLSpellCheck::CountMisspelledWords(const QString &text)
{
    return CountMisspelledWords(text, 0, text.count(), "");
}


int HTMLSpellCheck::CountAllWords(const QString &text)
{
    return CountMisspelledWords(text, 0, text.count(), "", false, true);
}


QStringList HTMLSpellCheck::GetAllWords(const QString &text)
{
    QList< HTMLSpellCheck::MisspelledWord > words = GetMisspelledWords(text, 0, text.count(), "", false, true);
    QStringList all_words_text;
    foreach(HTMLSpellCheck::MisspelledWord word, words) {
        all_words_text.append(word.text);
    }
    return all_words_text;
}
