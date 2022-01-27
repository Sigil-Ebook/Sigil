/************************************************************************
**
**  Copyright (C) 2020-2022 Kevin B. Hendrickws, Stratford Ontario Canada
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
#include "Misc/Utility.h"
#include "Misc/SettingsStore.h"
#include "Misc/SpellCheck.h"
#include "Parsers/QuickParser.h"
#include "Misc/HTMLSpellCheckML.h"

const int MAX_WORD_LENGTH  = 90;
const QString ENTITYWORDCHARS = ";#01234567890abcdefABCDEFxX";

QList<HTMLSpellCheckML::AWord> HTMLSpellCheckML::GetWordList(const QString &source, const QString & default_lang)
{
    QList<HTMLSpellCheckML::AWord> wordlist;
    SpellCheck *sc = SpellCheck::instance();
    QString wc = sc->getWordChars() + QChar(0x00ad); // add in soft hyphen
    SettingsStore ss;
    bool use_nums = ss.spellCheckNumbers();
    QuickParser qp(source, default_lang);
    while(true) {
        QuickParser::MarkupInfo mi = qp.parse_next();
        if (mi.pos < 0) break;
        if (!mi.text.isEmpty() && !(mi.tpath.endsWith(".style") || mi.tpath.endsWith("script"))) {
            parse_text_into_words(wordlist, wc, use_nums, mi.lang, mi.text, mi.pos); 
        }
    }
    return wordlist;
}


void HTMLSpellCheckML::parse_text_into_words(QList<HTMLSpellCheckML::AWord> &wordlist,
                                             const QString &wc,
                                             bool  use_nums,
                                             const QString &lang,
                                             const QString &parsetext,
                                             int   pos)
{
    bool in_entity = false;
    bool in_invalid_word = false;
    int word_start = 0;
    QString text = QChar(' ') + parsetext + QChar(' ');
    for (int i = 0; i < text.count(); i++) {
        QChar c = text.at(i);
        QChar prev_c = i > 0 ? text.at(i - 1) : QChar(' ');
        QChar next_c = i < text.count() - 1 ? text.at(i + 1) : QChar(' ');
        if (IsBoundary(prev_c, c, next_c, wc, use_nums)) {
            // If we're in an entity and we hit a boundary and it isn't
            // part of an entity then this is an invalid entity.
            // if (in_entity && c != QChar(';')) in_entity = false;
            if (in_entity && !ENTITYWORDCHARS.contains(c)) in_entity = false;
            if (!in_invalid_word && !in_entity && word_start != -1 && (i - word_start) > 0) {
                QString word = Utility::Substring(word_start, i, text);
                if (!word.isEmpty()) {
                    HTMLSpellCheckML::AWord aword;
                    aword.text = lang + ": " + word;
                    aword.offset = pos + word_start - 1;
                    aword.length = i - word_start;
                    wordlist.append(aword);
                }
            }
            word_start = i + 1;
            in_invalid_word = false;
        } else {
             // Ensure we're not dealing with some crazy run on text
             if (!in_invalid_word && (i - word_start) > MAX_WORD_LENGTH) in_invalid_word = true;
        }
        if (c == QChar('&')) in_entity = true;
        if (c == QChar(';')) in_entity = false;
    }
    return;
}


bool HTMLSpellCheckML::IsValidChar(const QChar & c, bool use_nums)
{ 
    if (use_nums) return c.isLetterOrNumber();
    return c.isLetter();
}


bool HTMLSpellCheckML::IsBoundary(QChar prev_c, QChar c, QChar next_c, const QString & wordChars, bool use_nums)
{
        
    if (IsValidChar(c,use_nums) ) return false;
    if (c == '.' && wordChars.contains(c)) return false;  // allow ending period for abbreviations
    // Single quotes of ' and curly version and hyphen/emdash are sometimes a boundary
    // and sometimes not, depending on whether they are surrounded by letters or not.
    // A sentence which 'has some text' should treat the ' as a boundary but didn't should not.
    bool is_potential_boundary = (c == '-' || 
                                  c == QChar(0x2012) || 
                                  c == '\'' || 
                                  c == QChar(0x2019) ||
                                  (!wordChars.isEmpty() && wordChars.contains(c)));
    if (is_potential_boundary && (!IsValidChar(prev_c, use_nums) || !IsValidChar(next_c, use_nums))) {
        return true;
    }
    return !(is_potential_boundary && (IsValidChar(prev_c, use_nums) || IsValidChar(next_c, use_nums)));
}


QList<HTMLSpellCheckML::AWord> HTMLSpellCheckML::GetWords(const QString &text, const QString &default_lang)
{
    if (default_lang.isEmpty()) {
        SettingsStore ss;
        return GetWordList(text, ss.defaultMetadataLang().replace("_","-"));
    }
    return GetWordList(text, default_lang);
}


QStringList HTMLSpellCheckML::GetAllWords(const QString &text, const QString& default_lang)
{
    QList<HTMLSpellCheckML::AWord> words;

    if (default_lang.isEmpty()) {
        SettingsStore ss;
        words = GetWordList(text, ss.defaultMetadataLang().replace("_","-"));
    } else {
        words = GetWordList(text, default_lang);
    }
    QStringList all_words_text;
    foreach(HTMLSpellCheckML::AWord word, words) {
        all_words_text.append(word.text);
    }
    return all_words_text;
}


QString HTMLSpellCheckML::textOf(const QString& word) 
{
    int p = word.indexOf(":",0);
    if (p < 0) return word;
    return word.mid(p+2,-1);
}


QString HTMLSpellCheckML::langOf(const QString& word)
{
    int p = word.indexOf(":",0);
    if (p != -1) return word.mid(0,p);
    SettingsStore ss;
    return ss.defaultMetadataLang().replace("_","-");
}


int HTMLSpellCheckML::WordPosition(QString text, QString word, int start_pos, const QString &default_lang)
{
    SettingsStore ss;
    QList<HTMLSpellCheckML::AWord> words = GetWordList(text, default_lang);
    foreach (HTMLSpellCheckML::AWord w, words) {
        if (w.offset < start_pos) {
            continue;
        }
        if ((textOf(w.text) == textOf(word)) && (langOf(w.text) == langOf(word))) {
            return w.offset;
        }
    }
    return -1;
}
