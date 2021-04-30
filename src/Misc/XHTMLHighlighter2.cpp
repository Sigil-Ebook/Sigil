/************************************************************************
**
**  Copyright (C) 2021 Kevin B. Hendricks, Stratford Ontario Canada
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

#include <QApplication>
#include <QSyntaxHighlighter>
#include <QTextDocument>
#include <QBrush>
#include <QColor>
#include <QDebug>

#include "Misc/SpellCheck.h"
#include "Misc/Utility.h"
#include "Misc/HTMLSpellCheck.h"
#include "Misc/SettingsStore.h"
#include "Misc/XHTMLHighlighter2.h"

#define DBG if(0)

// Parser states are mutually exclusive
static const int State_Text           = -1;

static const int State_TagStart       = 1;
static const int State_TagName        = 2;
static const int State_InsideTag      = 3;
static const int State_AttName        = 4;
static const int State_SingleQuote    = 5;
static const int State_DoubleQuote    = 6;
static const int State_AttValue       = 7;

static const int State_CSSTagStart    = 11;
static const int State_CSSTagName     = 12;
static const int State_CSSInsideTag   = 13;
static const int State_CSSAttName     = 14;
static const int State_CSSSingleQuote = 15;
static const int State_CSSDoubleQuote = 16;
static const int State_CSSAttValue    = 17;


static const int State_DOCTYPE        = 8;
static const int State_Comment        = 9;

static const int State_CSS            = 18;
static const int State_CSSComment     = 19;


static const QString CSS_BEGIN              = "<\\s*style[^>]*>";
static const QRegularExpression RXCSSBegin(CSS_BEGIN);

// Special Spaces
// "[\\x{00A0}\\x{2000}-\\x{200A}\\x{202F}\\x{3000}]+";
static const QList<QChar> SPECIAL_SPACES  = QList<QChar>() <<  QChar(160) << QChar(8192) << QChar(8193) << 
                                                               QChar(8194) << QChar(8195) << QChar(8196) << 
                                                               QChar(8197) << QChar(8198) << QChar(8199) << 
                                                               QChar(8200) << QChar(8201) << QChar(8202) << 
                                                               QChar(8239) << QChar(12288);

static const QString WHITESPACE = " \f\t\r\n";


// Constructor
XHTMLHighlighter2::XHTMLHighlighter2(bool checkSpelling, QObject *parent)
    : QSyntaxHighlighter(parent),
      m_checkSpelling(checkSpelling)

{
    SetRules();
}

void XHTMLHighlighter2::SetRules()
{
    m_Rules.clear();

    SettingsStore settings;
    if (Utility::IsDarkMode()) {
        m_codeViewAppearance = settings.codeViewDarkAppearance();
    } else {
        m_codeViewAppearance = settings.codeViewAppearance();
    }

    QTextCharFormat html_format;
    QTextCharFormat doctype_format;
    QTextCharFormat html_comment_format;
    QTextCharFormat css_format;
    QTextCharFormat css_comment_format;
    QTextCharFormat attribute_name_format;
    QTextCharFormat attribute_value_format;
    QTextCharFormat entity_format;
    QTextCharFormat special_space_format;

    doctype_format        .setForeground(m_codeViewAppearance.xhtml_doctype_color);
    html_format           .setForeground(m_codeViewAppearance.xhtml_html_color);
    html_comment_format   .setForeground(m_codeViewAppearance.xhtml_html_comment_color);
    css_format            .setForeground(m_codeViewAppearance.xhtml_css_color);
    css_comment_format    .setForeground(m_codeViewAppearance.xhtml_css_comment_color);
    attribute_name_format .setForeground(m_codeViewAppearance.xhtml_attribute_name_color);
    attribute_value_format.setForeground(m_codeViewAppearance.xhtml_attribute_value_color);
    entity_format         .setForeground(m_codeViewAppearance.xhtml_entity_color);
    special_space_format  .setUnderlineColor(m_codeViewAppearance.xhtml_entity_color);
    special_space_format  .setUnderlineStyle(QTextCharFormat::DashUnderline);

    m_Rules["doctype"] = doctype_format;
    m_Rules["tagname"] = html_format;
    m_Rules["comment"] = html_comment_format;
    m_Rules["css"] = css_format;
    m_Rules["css_comment"] = css_comment_format;
    m_Rules["attname"] = attribute_name_format;
    m_Rules["attvalue"] = attribute_value_format;
    m_Rules["entity"] = entity_format;
    m_Rules["spspace"] = special_space_format;    

}

// Overrides the function from QSyntaxHighlighter;
// gets called by QTextEditor whenever
// a block (line of text) needs to be repainted
void XHTMLHighlighter2::highlightBlock(const QString &text)
{
    // By default, all block states are -1;
    // in our implementation regular text is state == 1
    int state = previousBlockState();
    int n = text.length();
    int start = 0;
    int pos = 0;
    int nstate = -1;
    QChar ch;

    // Run spell check over the text if needed first.
    SettingsStore settings;
    bool enableSpellCheck = settings.spellCheck();
    if (enableSpellCheck && m_checkSpelling) {
        CheckSpelling(text);
    }

    DBG qDebug() << "----- ";
    DBG qDebug() <<  text;

    while(pos < n) {

        switch (state) {

        case State_Comment:
            {
                start = pos;
                nstate = state;
                while(pos < n) {
                    if (Utility::SubstringRef(pos, pos+3, text) == "-->") {
                        pos += 3;
                        nstate = State_Text;
                        break;
                    } else pos++;
                } 
                setFormat(start, pos-start, m_Rules["comment"]);
                break;
            }

        case State_DOCTYPE:
            {
                nstate = state;
                start = pos;
                while(pos < n) {
                    ch = text.at(pos++);
                    if (ch == '>') {
                        nstate = State_Text;
                        break;
                    }
                }
                setFormat(start, pos - start, m_Rules["doctype"]);
                break;
                
            }

        case State_TagStart:
        case State_CSSTagStart:
            {
                // at '<' in e.g. "<span>foo</span>"
                nstate = state;
                start = pos + 1;
                while(pos < n) {
                    ch = text.at(pos++);
                    if (ch == '>') {
                        if (state == State_CSSTagStart) nstate = State_CSS;
                        if (state == State_TagStart) nstate = State_Text;
                        break;
                    }
                    if (ch != ' ') {
                        pos--;
                        if (state == State_CSSTagStart) nstate = State_CSSTagName;
                        if (state == State_TagStart) nstate = State_TagName;
                        break;
                    }
                }
                break;
                
            }

        case State_TagName:
        case State_CSSTagName:
            {
                // at 'b' in e.g "<blockquote>foo</blockquote>"
                nstate = state;
                start = pos;
                while (pos < n) {
                    ch = text.at(pos++);
                    if (WHITESPACE.contains(ch)) {
                        pos--;
                        if (state == State_CSSTagName) nstate = State_CSSInsideTag;
                        if (state == State_TagName) nstate = State_InsideTag;
                        break;
                    }
                    if (ch == '>') {
                        if (state == State_CSSTagName) nstate = State_CSS;
                        if (state == State_TagName) nstate = State_Text;
                        break;
                    }
                }
                setFormat(start, pos - start, m_Rules["tagname"]);
                break;
            }
            

        case State_InsideTag:
        case State_CSSInsideTag:
            {
                // anywhere after tag name and before tag closing ('>')
                nstate = state;
                start = pos;
                while (pos < n) {
                    ch = text.at(pos++);
                    if (ch == '/') continue;
                    if (ch == '>') {
                        if (state == State_CSSInsideTag) nstate = State_CSS;
                        if (state == State_InsideTag) nstate = State_Text;
                        setFormat(pos-1, 1, m_Rules["tagname"]);
                        break;
                    }
                    if (!WHITESPACE.contains(ch)) {
                        pos--;
                        if (state == State_CSSInsideTag) nstate = State_CSSAttName;
                        if (state == State_InsideTag) nstate = State_AttName;
                        break;
                    }
                }
                break;
            }

        case State_AttName:
        case State_CSSAttName:
            {
                // at 's' in e.g. <img src=bla.png/>
                nstate = state;
                start = pos;
                while (pos < n) {
                    ch = text.at(pos++);
                    if (ch == '=') {
                        if (state == State_CSSAttName) nstate = State_CSSAttValue;
                        if (state == State_AttName) nstate = State_AttValue;
                        break;
                    }
                    if (ch == '/') {
                        if (state == State_CSSAttName) nstate = State_CSSInsideTag;
                        if (state == State_AttName) nstate = State_InsideTag;
                        break;
                    }
                    if (ch == '>') {
                        if (state == State_CSSAttName) nstate = State_CSS;
                        if (state == State_AttName) nstate = State_Text;
                        break;
                    }
                }
                setFormat(start, pos - start, m_Rules["attname"]);
                break;
            }

        case State_AttValue:
        case State_CSSAttValue:
            {
                // after '=' in e.g. <img src=bla.png/>
                start = pos;
                nstate = state;
                // find first non-space character
                while (pos < n) {
                    ch = text.at(pos++);
                    // handle opening single quote
                    if (ch == '\'') {
                        if (state == State_CSSAttValue) nstate = State_CSSSingleQuote;
                        if (state == State_AttValue) nstate = State_SingleQuote;
                        setFormat(pos - 1, 1, m_Rules["attvalue"]);
                        break;
                    }   
                    // handle opening double quote
                    if (ch == '"') {
                        if (state == State_CSSAttValue) nstate = State_CSSDoubleQuote;
                        if (state == State_AttValue) nstate = State_DoubleQuote;
                        setFormat(pos - 1, 1, m_Rules["attvalue"]);
                        break;
                    }
                    if (ch != ' ') {
                        break;
                    }
                }
                if ((nstate == State_AttValue) || (nstate == State_CSSAttValue)) {
                    // attribute value without quote
                    // just stop at non-space or tag delimiter
                    start = pos;
                    while (pos < n) {
                        ch = text.at(pos);
                        if (WHITESPACE.contains(ch)) {
                            break;
                        }
                        if (ch == '>' || ch == '/') {
                            break;
                        }
                        pos++;
                    }
                    if (state == State_CSSAttValue) nstate = State_CSSInsideTag;
                    if (state == State_AttValue) nstate = State_InsideTag;
                    setFormat(start, pos - start, m_Rules["attvalue"]);
                }
                break;
            }

        case State_SingleQuote:
        case State_CSSSingleQuote:
            {
                // after the opening single quote in an attribute value
                nstate = state;
                start = pos;
                while (pos < n) {
                    ch = text.at(pos++);
                    if (ch == '\'') {
                        break;
                    }
                }
                if (state == State_CSSSingleQuote) nstate = State_CSSInsideTag;
                if (state == State_SingleQuote) nstate = State_InsideTag;
                setFormat(start, pos - start, m_Rules["attvalue"]);
                break;
            }

        case State_DoubleQuote:
        case State_CSSDoubleQuote:
            {
                // after the opening double quote in an attribute value
                nstate = state;
                start = pos;
                while (pos < n) {
                    ch = text.at(pos++);
                    if (ch == '"') {
                        break;
                    }
                }
                if (state == State_CSSDoubleQuote) nstate = State_CSSInsideTag;
                if (state == State_DoubleQuote) nstate = State_InsideTag;
                setFormat(start, pos - start, m_Rules["attvalue"]);
                break;
            }

        case State_CSS:
            {
                nstate = state;
                start = pos;
                while(pos < n) {
                    if (Utility::SubstringRef(pos, pos+2, text) == "/*") {
                        pos += 2;
                        nstate = State_CSSComment;
                        break;
                    } 
                    ch = text.at(pos);
                    if (ch == '<') {
                        nstate = State_TagStart;
                        break;
                    }
                    pos++;
                } 
                setFormat(start, pos-start, m_Rules["css"]);
                break;
            }

        case State_CSSComment:
            {
                nstate = state;
                start = pos;
                while(pos < n) {
                    if (Utility::SubstringRef(pos, pos+2, text) == "*/") {
                        pos += 2;
                        nstate = State_CSS;
                        break;
                    } else pos++;
                } 
                setFormat(start, pos-start, m_Rules["css_comment"]);
                break;
            }

        default:
            {
                // State_Text (also handle entity and SpSpaces)
                // start = pos;
                nstate = state;
                while (pos < n) {
                    ch = text.at(pos);
                    if (ch == '<') {
                        if (Utility::SubstringRef(pos, pos+4, text) == "<!--") {
                            DBG qDebug() << " found a comment";
                            nstate = State_Comment;
                        }
                        else if (Utility::SubstringRef(pos, pos+9, text) == "<!DOCTYPE") {
                            DBG qDebug() << " found a doctype";
                            nstate = State_DOCTYPE;
                        }
                        else if (text.indexOf(RXCSSBegin, pos) == pos) {
                            DBG qDebug() << " found a style";
                            nstate = State_CSSTagStart;
                        } else {
                            DBG qDebug() << " found a regular tag";
                            nstate = State_TagStart;
                        }
                        break;
                    } else if (ch == '&') {
                        start = pos;
                        while (pos < n && text[pos] != ';') {
                            pos++;
                        }
                        setFormat(start, pos - start, m_Rules["entity"]);
                    } else if (SPECIAL_SPACES.contains(ch)) {
                        setFormat(pos, 1, m_Rules["spspace"]);
                        pos ++;
                    } else {
                        pos++;
                    }
                }
                break;
            }
        }
        DBG qDebug() << "old state: " << state << " newstate: " << nstate;
        state = nstate;
    }
    setCurrentBlockState(nstate);
}


void XHTMLHighlighter2::do_rehighlight()
{
    SetRules();
    // bool do_spelling = m_checkSpelling;
    // m_checkSpelling = false;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    rehighlight();
    QApplication::restoreOverrideCursor();
    // m_checkSpelling = do_spelling;
}


void XHTMLHighlighter2::CheckSpelling(const QString &text)
{
    QTextCharFormat format;
    format.setUnderlineColor(m_codeViewAppearance.spelling_underline_color);
    // QTextCharFormat::SpellCheckUnderline has issues with Qt 5. It only displays
    // at some zoom levels and often doesn't display at all. So we're using wave
    // underline since it's good enough for most people.
    format.setUnderlineStyle(QTextCharFormat::WaveUnderline);
    QList<HTMLSpellCheck::MisspelledWord> misspelled_words = HTMLSpellCheck::GetMisspelledWords(text);
    foreach(HTMLSpellCheck::MisspelledWord misspelled_word, misspelled_words) {
        setFormat(misspelled_word.offset, misspelled_word.length, format);
    }
}
