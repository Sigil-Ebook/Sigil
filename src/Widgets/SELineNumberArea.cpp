/************************************************************************
**
**  Copyright (C) 2025       Kevin B. Hendricks, Stratford Ontario Canada
**
**  Based on LineNumberArea.cpp which are:
**     Copyright (C) 2009-2011  Strahinja Markovic
**                              <strahinja.markovic@gmail.com>, Nokia Corporation
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

#include <QCoreApplication>

#include "Widgets/SourceEditor.h"
#include "Widgets/SELineNumberArea.h"

// Constructor;
// The parameter is the SourceEditor to
// which this line number area belongs to
SELineNumberArea::SELineNumberArea(SourceEditor *editor) : QWidget(editor)
{
    m_SourceEditor = editor;
}

// Implements QWidget::sizeHint();
// Asks the editor which width should it take
QSize SELineNumberArea::sizeHint() const
{
    return QSize(m_SourceEditor->CalculateLineNumberAreaWidth(), 0);
}

// The line number area delegates its rendering
// to the TextView that owns it
void SELineNumberArea::paintEvent(QPaintEvent *event)
{
    m_SourceEditor->LineNumberAreaPaintEvent(event);
}
