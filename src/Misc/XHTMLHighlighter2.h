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
#ifndef XHTMLHIGHLIGHTER2_H
#define XHTMLHIGHLIGHTER2_H

#include <QtGui/QSyntaxHighlighter>
#include <QRegularExpression>

#include "Misc/SettingsStore.h"

class QTextDocument;

class XHTMLHighlighter2 : public QSyntaxHighlighter
{
    Q_OBJECT

public:

    // Constructor
    XHTMLHighlighter2(bool checkSpelling, QObject *parent = 0);

    void do_rehighlight();

    void SetRules();

protected:

    // Overrides the function from QSyntaxHighlighter;
    // gets called by QTextEditor whenever
    // a block (line of text) needs to be repainted
    void highlightBlock(const QString &text);

private:

    void CheckSpelling(const QString &text);


    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    QHash<QString, QTextCharFormat> m_Rules;

    // Determine if spell check should be used on the document.
    bool m_checkSpelling;

    SettingsStore::CodeViewAppearance m_codeViewAppearance;
};

#endif // XHTMLHIGHLIGHTER2_H

