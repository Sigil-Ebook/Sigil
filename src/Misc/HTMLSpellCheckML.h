/************************************************************************
**
**  Copyright (C) 2020      Kevin B. Hendricks, Stratford Ontario Canada
**  Copyright (C) 2009-2011 Strahinja Markovic  <strahinja.markovic@gmail.com>
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
#ifndef HTMLSPELLCHECKML_H
#define HTMLSPELLCHECKML_H

#include <QStringList>

class HTMLSpellCheckML
{

public:

    struct AWord {
        QString text;
        int offset;
        int length;
    };

    static QList<AWord> GetWordList(const QString &text, const QString &default_lang = "");
    static QList<AWord> GetWords(const QString &text, const QString &default_lang="");
    static QStringList GetAllWords(const QString &text, const QString &default_lang="");
    static int WordPosition(QString text, QString word, int start_pos, const QString &default_lang="");
    static QString textOf(const QString &word);
    static QString langOf(const QString &word);

private:

    static bool IsBoundary(QChar prev_c, QChar c, QChar next_c, const QString & wordChars, bool use_nums);
    static bool IsValidChar(const QChar & c, bool use_nums);
    static void parse_text_into_words(QList<AWord> &wordlist,
                                      const QString &wc,
                                      bool  use_nums,
                                      const QString &lang,
                                      const QString &parsetext,
                                      int   pos);
};

#endif // HTMLSPELLCHECKML_H
