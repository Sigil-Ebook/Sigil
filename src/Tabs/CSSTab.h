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
#ifndef CSSTAB_H
#define CSSTAB_H

#include "sigil_constants.h"
#include "Tabs/TextTab.h"

class CSSResource;

class CSSTab : public TextTab
{
    Q_OBJECT

public:

    CSSTab(CSSResource *resource, int line_to_scroll_to = -1, int position_to_scroll_to = -1, QWidget *parent = 0);

public slots:

    void Bold();
    void Italic();
    void Underline();
    void Strikethrough();

    void AlignLeft();
    void AlignCenter();
    void AlignRight();
    void AlignJustify();

    void TextDirectionLeftToRight();
    void TextDirectionRightToLeft();
    void TextDirectionDefault();

    void HighlightRed();
    void HighlightGreen();
    void HighlightBlue();
    void HighlightCyan();
    void HighlightMegenta();
    void HighlightYellow();
signals:
    void CSSUpdated();

private slots:
    void EmitCSSUpdated();
};

#endif // CSSTAB_H
