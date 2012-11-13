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
#ifndef XHTMLHIGHLIGHTER_H
#define XHTMLHIGHLIGHTER_H

#include <QtGui/QSyntaxHighlighter>

#include "Misc/SettingsStore.h"

class XHTMLHighlighter : public QSyntaxHighlighter
{

public:

    // Constructor
    XHTMLHighlighter(bool checkSpelling, QObject *parent = 0);

protected:

    // Overrides the function from QSyntaxHighlighter;
    // gets called by QTextEditor whenever
    // a block (line of text) needs to be repainted
    void highlightBlock(const QString &text);

private:

    // Returns the regex that matches the left bracket of a state
    QRegExp GetLeftBracketRegEx(int state) const;

    // Returns the regex that matches the right bracket of a state
    QRegExp GetRightBracketRegEx(int state) const;

    // Sets the requested state for the current text block
    void SetState(int state);

    // Clears the requested state for the current text block
    void ClearState(int state);

    // Checks if the requested state is set
    // for the current text block
    bool StateChecked(int state) const;

    // Formats the inside of a node;
    // "text" is the textblock/line;
    // "state" describes the node;
    // "index" is the index to start formatting from
    // "length" is the length of chars to format
    void FormatBody(const QString &text, int state, int index, int length);

    // Highlights the current line according to the state requested;
    // check to see if the node of type "state" is present;
    // if it is, the node is formatted
    void HighlightLine(const QString &text, int state);

    void CheckSpelling(const QString &text);


    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    // Al the possible nodes/states
    enum BlockState {
        State_Text          = 1 << 0,
        State_Entity        = 1 << 1,
        State_HTML          = 1 << 2,
        State_CSS           = 1 << 3,
        State_CSSComment    = 1 << 4,
        State_HTMLComment   = 1 << 5,
        State_DOCTYPE       = 1 << 6
    };

    struct HighlightingRule {
        QRegExp pattern;
        QTextCharFormat format;
    };

    // Stores all of our highlighting rules
    // and the text formats used
    QHash<QString, HighlightingRule> m_Rules;

    // Determine if spell check should be used on the document.
    bool m_checkSpelling;

    // Determine if automatic spell check is enabled
    bool m_enableSpellCheck;

    SettingsStore::CodeViewAppearance m_codeViewAppearance;
};

#endif // XHTMLHIGHLIGHTER_H

