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

#pragma once
#ifndef HTMLSPELLCHECK_H
#define HTMLSPELLCHECK_H

#include <QtCore/QStringList>
class QuickSerialHtmlParser;

class HTMLSpellCheck
{

public:

    struct MisspelledWord {
        QString text;
        QString language;
        int offset;
        int length;
    };

    static QList<MisspelledWord> GetMisspelledWords(const QString &text,
            int start_offset,
            int end_offset,
            const QString &search_regex,
            bool first_only = false,
            bool include_all_words = false);

    static QList<MisspelledWord> GetMisspelledWords(const QString &text, QuickSerialHtmlParser *QSHParser);

    static QList<HTMLSpellCheck::MisspelledWord> GetWords(const QString &text);

    static int CountMisspelledWords(const QString &text,
                                    int start_offset,
                                    int end_offset,
                                    const QString &search_regex,
                                    bool first_only = false,
                                    bool include_all_words = false);

    static int CountMisspelledWords(const QString &text);

    static int CountAllWords(const QString &text);

    static QStringList GetAllWords(const QString &text);


    static MisspelledWord GetFirstMisspelledWord(const QString &text,
            int start_offset,
            int end_offset,
            const QString &search_regex);

    static MisspelledWord GetLastMisspelledWord(const QString &text,
            int start_offset,
            int end_offset,
            const QString &search_regex);

    static int WordPosition(QString text, QString word, int start_pos);

    static int WordPosition(QString text, QString word, QString lang, int start_pos);
    static QList<MisspelledWord> GetMLMisspelledWords(const QString &text,
            int start_offset,
            int end_offset,
            const QString &search_regex,
            QuickSerialHtmlParser *qshp,
            bool first_only = false,
            bool include_all_words = false
);

private:

    static bool IsBoundary(QChar prev_c, QChar c, QChar next_c, const QString & wordChars);

};

#endif // HTMLSPELLCHECK_H
