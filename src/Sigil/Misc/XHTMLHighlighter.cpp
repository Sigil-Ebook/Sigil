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

#include <QRegularExpressionMatch>

#include "Misc/SpellCheck.h"
#include "Misc/Utility.h"
#include "Misc/XHTMLHighlighter.h"
#include "Misc/HTMLSpellCheck.h"
#include "Misc/SettingsStore.h"

// All of our regular expressions
static const QString DOCTYPE_BEGIN          = "<!(?!--)";
static const QString HTML_ELEMENT_BEGIN     = "<(/\\?|/|\\?)?(?!!)";

// "(?=[^\\w:-])" emulates a boundary ("\\b") that counts ":" and "-" as word characters
static const QString HTML_ELEMENT_NAME      = "\\s*[\\w:-]+(?=[^\\w:-])\\s*(?!=)";
static const QString HTML_ELEMENT_END       = "(\\?|/)?>";

static const QString HTML_COMMENT_BEGIN     = "<!--";
static const QString HTML_COMMENT_END       = "-->";

static const QString CSS_BEGIN              = "<\\s*style[^>]*>";
static const QString CSS_END                = "</\\s*style[^>]*>";

static const QString CSS_COMMENT_BEGIN      = "/\\*";
static const QString CSS_COMMENT_END        = "\\*/";

static const QString ATTRIBUTE_VALUE        = "\"[^<\"]*\"|'[^<']*'";
static const QString ATTRIBUTE_NAME         = "[\\w:-]+";

static const QString ENTITY_BEGIN           = "&(?=[^\\s;]+;)";
static const QString ENTITY_END             = ";";


// Constructor
XHTMLHighlighter::XHTMLHighlighter(bool checkSpelling, QObject *parent)
    : QSyntaxHighlighter(parent),
      m_checkSpelling(checkSpelling)

{
    SettingsStore settings;
    m_codeViewAppearance = settings.codeViewAppearance();
    QTextCharFormat html_format;
    QTextCharFormat doctype_format;
    QTextCharFormat html_comment_format;
    QTextCharFormat css_format;
    QTextCharFormat css_comment_format;
    QTextCharFormat attribute_name_format;
    QTextCharFormat attribute_value_format;
    QTextCharFormat entity_format;
    doctype_format        .setForeground(m_codeViewAppearance.xhtml_doctype_color);
    html_format           .setForeground(m_codeViewAppearance.xhtml_html_color);
    html_comment_format   .setForeground(m_codeViewAppearance.xhtml_html_comment_color);
    css_format            .setForeground(m_codeViewAppearance.xhtml_css_color);
    css_comment_format    .setForeground(m_codeViewAppearance.xhtml_css_comment_color);
    attribute_name_format .setForeground(m_codeViewAppearance.xhtml_attribute_name_color);
    attribute_value_format.setForeground(m_codeViewAppearance.xhtml_attribute_value_color);
    entity_format         .setForeground(m_codeViewAppearance.xhtml_entity_color);
    HighlightingRule rule;
    rule.pattern = QRegularExpression(DOCTYPE_BEGIN);
    rule.format  = doctype_format;
    m_Rules[ "DOCTYPE_BEGIN" ] = rule;
    rule.pattern = QRegularExpression(HTML_ELEMENT_BEGIN);
    rule.format  = html_format;
    m_Rules[ "HTML_ELEMENT_BEGIN" ] = rule;
    rule.pattern = QRegularExpression(HTML_ELEMENT_END);
    rule.format  = html_format;
    m_Rules[ "HTML_ELEMENT_END" ] = rule;
    rule.pattern = QRegularExpression(HTML_COMMENT_BEGIN);
    rule.format  = html_comment_format;
    m_Rules[ "HTML_COMMENT_BEGIN" ] = rule;
    rule.pattern = QRegularExpression(HTML_COMMENT_END);
    rule.format  = html_comment_format;
    m_Rules[ "HTML_COMMENT_END" ] = rule;
    rule.pattern = QRegularExpression(CSS_BEGIN);
    rule.format  = css_format;
    m_Rules[ "CSS_BEGIN" ] = rule;
    rule.pattern = QRegularExpression(CSS_END);
    rule.format  = css_format;
    m_Rules[ "CSS_END" ] = rule;
    rule.pattern = QRegularExpression(CSS_COMMENT_BEGIN);
    rule.format  = css_comment_format;
    m_Rules[ "CSS_COMMENT_BEGIN" ] = rule;
    rule.pattern = QRegularExpression(CSS_COMMENT_END);
    rule.format  = css_comment_format;
    m_Rules[ "CSS_COMMENT_END" ] = rule;
    rule.pattern = QRegularExpression(ATTRIBUTE_NAME);
    rule.format  = attribute_name_format;
    m_Rules[ "ATTRIBUTE_NAME" ] = rule;
    rule.pattern = QRegularExpression(ATTRIBUTE_VALUE);
    rule.format  = attribute_value_format;
    m_Rules[ "ATTRIBUTE_VALUE" ] = rule;
    rule.pattern = QRegularExpression(ENTITY_BEGIN);
    rule.format  = entity_format;
    m_Rules[ "ENTITY_BEGIN" ] = rule;
    rule.pattern = QRegularExpression(ENTITY_END);
    rule.format  = entity_format;
    m_Rules[ "ENTITY_END" ] = rule;
}


// Overrides the function from QSyntaxHighlighter;
// gets called by QTextEditor whenever
// a block (line of text) needs to be repainted
void XHTMLHighlighter::highlightBlock(const QString &text)
{
    // By default, all block states are -1;
    // in our implementation regular text is state == 1
    if (previousBlockState() == -1) {
        setCurrentBlockState(State_Text);
    }
    // Propagate previous state; needed for state tracking
    else {
        setCurrentBlockState(previousBlockState());
    }

    if (text.isEmpty()) {
        return;
    }

    SettingsStore settings;
    m_enableSpellCheck = settings.spellCheck();

    // Run spell check over the text.
    if (m_enableSpellCheck && m_checkSpelling) {
        CheckSpelling(text);
    }

    // The order of these operations is important
    // because some states format text over previous states!
    HighlightLine(text, State_Entity);
    HighlightLine(text, State_CSS);
    HighlightLine(text, State_HTML);
    HighlightLine(text, State_CSSComment);
    HighlightLine(text, State_HTMLComment);
    HighlightLine(text, State_DOCTYPE);
}


// Returns the regex that matches the left bracket of a state
QRegularExpression XHTMLHighlighter::GetLeftBracketRegEx(int state) const
{
    QRegularExpression empty;

    switch (state) {
        case State_Entity:
            return m_Rules.value("ENTITY_BEGIN").pattern;

        case State_HTML:
            return m_Rules.value("HTML_ELEMENT_BEGIN").pattern;

        case State_HTMLComment:
            return m_Rules.value("HTML_COMMENT_BEGIN").pattern;

        case State_CSS:
            return m_Rules.value("CSS_BEGIN").pattern;

        case State_CSSComment:
            return m_Rules.value("CSS_COMMENT_BEGIN").pattern;

        case State_DOCTYPE:
            return m_Rules.value("DOCTYPE_BEGIN").pattern;

        default:
            return empty;
    }
}


// Returns the regex that matches the right bracket of a state
QRegularExpression XHTMLHighlighter::GetRightBracketRegEx(int state) const
{
    QRegularExpression empty;

    switch (state) {
        case State_Entity:
            return m_Rules.value("ENTITY_END").pattern;

        case State_DOCTYPE:
        case State_HTML:
            return m_Rules.value("HTML_ELEMENT_END").pattern;

        case State_HTMLComment:
            return m_Rules.value("HTML_COMMENT_END").pattern;

        case State_CSS:
            return m_Rules.value("CSS_END").pattern;

        case State_CSSComment:
            return m_Rules.value("CSS_COMMENT_END").pattern;

        default:
            return empty;
    }
}


// Sets the requested state for the current text block
void XHTMLHighlighter::SetState(int state)
{
    int current_state = currentBlockState();
    // Add the current state to the list
    current_state = current_state | state;
    setCurrentBlockState(current_state);
}


// Clears the requested state for the current text block
void XHTMLHighlighter::ClearState(int state)
{
    int current_state = currentBlockState();
    // Remove the current state from the list
    current_state = current_state & ~state;
    setCurrentBlockState(current_state);
}


// Checks if the requested state is set
// for the current text block
bool XHTMLHighlighter::StateChecked(int state) const
{
    int current_state = currentBlockState();

    // Check if our state is in the list
    if ((current_state & state) != 0) {
        return true;
    } else {
        return false;
    }
}

// Formats the inside of a node;
// "text" is the textblock/line of text;
// "state" describes the node;
// "index" is the index to start formatting from;
// "length" is the length of chars to format.
void XHTMLHighlighter::FormatBody(const QString &text, int state, int index, int length)
{
    if (state == State_HTML) {
        // First paint everything the color of the brackets
        setFormat(index, length, m_Rules.value("HTML_ELEMENT_BEGIN").format);
        QRegularExpression name  = m_Rules.value("ATTRIBUTE_NAME").pattern;
        QRegularExpression value = m_Rules.value("ATTRIBUTE_VALUE").pattern;
        // Used to move over the line
        int main_index = index;

        // We skip over the left bracket (if it's present)
        QRegularExpression bracket(HTML_ELEMENT_BEGIN);
        QRegularExpressionMatch bracket_match = bracket.match(text, main_index);
        if (bracket_match.hasMatch() && bracket_match.capturedStart() == main_index) {
            main_index += bracket_match.capturedLength();
        }

        // We skip over the element name (if it's present)
        // because we want it to be the same color as the brackets
        QRegularExpression elem_name(HTML_ELEMENT_NAME);
        QRegularExpressionMatch elem_name_match = elem_name.match(text, main_index);
        if (elem_name_match.hasMatch() && elem_name_match.capturedStart() == main_index) {
            main_index += elem_name_match.capturedLength();
        }

        while (true) {
            // Get the indexes of the attribute names and values
            int name_index = -1;
            int name_len = 0;
            QRegularExpressionMatch name_match = name.match(text, main_index);
            if (name_match.hasMatch()) {
                name_index = name_match.capturedStart();
                name_len = name_match.capturedLength();
            }

            int value_index = -1;
            int value_len = 0;
            QRegularExpressionMatch value_match = value.match(text, main_index);
            if (value_match.hasMatch()) {
                value_index = value_match.capturedStart();
                value_len = value_match.capturedLength();
            }

            // If we can't find the names and values or we found them
            // outside of the area we are formatting, we exit
            if (((name_index  != -1) && (name_index  < index + length)) ||
                ((value_index != -1) && (value_index < index + length))) {
                // ... otherwise format the found sections
                setFormat(name_index,  name_len,  m_Rules.value("ATTRIBUTE_NAME").format);
                setFormat(value_index, value_len, m_Rules.value("ATTRIBUTE_VALUE").format);
            } else {
                break;
            }

            // Update the main index with the regex that matched "further down the line"
            if (name_index + name_len > value_index + value_len) {
                main_index = name_index + name_len;
            } else {
                main_index = value_index + value_len;
            }
        }
    } else if (state == State_HTMLComment) {
        setFormat(index, length, m_Rules.value("HTML_COMMENT_BEGIN").format);
    } else if (state == State_CSS) {
        setFormat(index, length, m_Rules.value("CSS_BEGIN").format);
    } else if (state == State_CSSComment) {
        setFormat(index, length, m_Rules.value("CSS_COMMENT_BEGIN").format);
    } else if (state == State_Entity) {
        setFormat(index, length, m_Rules.value("ENTITY_BEGIN").format);
    } else if (state == State_DOCTYPE) {
        setFormat(index, length, m_Rules.value("DOCTYPE_BEGIN").format);
    }
}


// Highlights the current line according to the state requested;
// check to see if the node of type "state" is present;
// if it is, the node is formatted
void XHTMLHighlighter::HighlightLine(const QString &text, int state)
{
    QRegularExpression left_bracket_regex  = GetLeftBracketRegEx(state);
    QRegularExpression right_bracket_regex = GetRightBracketRegEx(state);
    int main_index = 0;

    // We loop over the line several times
    // because we could have several nodes on it
    while (main_index < text.length()) {
        int left_bracket_index  = -1;
        int left_bracket_len = 0;
        int right_bracket_index = -1;
        int right_bracket_len = 0;

        if (!left_bracket_regex.pattern().isEmpty()) {
            QRegularExpressionMatch left_bracket_match = left_bracket_regex.match(text, main_index);
            if (left_bracket_match.hasMatch()) {
                left_bracket_index = left_bracket_match.capturedStart();
                left_bracket_len = left_bracket_match.capturedLength();
            }
        }

        if (!right_bracket_regex.pattern().isEmpty()) {
            QRegularExpressionMatch right_bracket_match = right_bracket_regex.match(text, main_index);
            if (right_bracket_match.hasMatch()) {
                right_bracket_index = right_bracket_match.capturedStart();
                right_bracket_len = right_bracket_match.capturedLength();
            }
        }

        // If we are not starting our state and our state is
        // not already set, we don't format; see the four cases explanation below
        if (left_bracket_index == -1 && !StateChecked(state)) {
            return;
        }

        // Every node/state has a left "bracket", a right "bracket" and the inside body.
        // This example uses HTML tags, but the principle is the same for every node/state.
        // There are four possible cases:
        // (1)  <......>    (both brackets on the same line; state starts and stops here)
        // (2)  <.......    (only the left bracket; next line continues state)
        // (3)  .......>    (only the right bracket; current line ends state)
        // (4)  ........    (no brackets; a line between (2) and (3))

        // We also check the state because we don't want to start a new node
        // if the current node of the same type hasn't finished
        if (left_bracket_index != -1 && !StateChecked(state)) {
            main_index = left_bracket_index + left_bracket_len;

            // (1)
            if (right_bracket_index != -1) {
                main_index = right_bracket_index + right_bracket_len;
                int length = right_bracket_index - left_bracket_index + right_bracket_len;
                FormatBody(text, state, left_bracket_index, length);
                // There's no point in setting the state here because the state
                // starts and ends on this line.
            }
            // (2)
            else {
                int length = text.length() - left_bracket_index;
                FormatBody(text, state, left_bracket_index, length);
                main_index += length;
                // Set the current state so the next line can continue
                // with the formatting.
                SetState(state);
            }
        } else {
            // (3)
            if (right_bracket_index != -1) {
                main_index = right_bracket_index + right_bracket_len;
                int length = right_bracket_index + right_bracket_len;
                FormatBody(text, state, 0, length);
                // Clear the current state because our state has just ended.
                ClearState(state);
            }
            // (4)
            else {
                int length = text.length();
                FormatBody(text, state, 0, length);
                main_index += length;
            }
        }
    }
}


void XHTMLHighlighter::CheckSpelling(const QString &text)
{
    QTextCharFormat format;
    format.setUnderlineColor(m_codeViewAppearance.spelling_underline_color);
    // QTextCharFormat::SpellCheckUnderline has issues with Qt 5. It only displays
    // at some zoom levels and often doesn't display at all. So we're using wave
    // underline since it's good enough for most people.
    format.setUnderlineStyle(QTextCharFormat::WaveUnderline);
    QList< HTMLSpellCheck::MisspelledWord > misspelled_words = HTMLSpellCheck::GetMisspelledWords(text);
    foreach(HTMLSpellCheck::MisspelledWord misspelled_word, misspelled_words) {
        setFormat(misspelled_word.offset, misspelled_word.length, format);
    }
}
