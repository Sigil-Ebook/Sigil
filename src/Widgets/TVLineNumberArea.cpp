/************************************************************************
**
**  Copyright (C) 2020       Kevin B. Hendricks, Stratford Ontario Canada
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

#include <QtCore/QCoreApplication>

#include "Widgets/TextView.h"
#include "Widgets/TVLineNumberArea.h"

// Constructor;
// The parameter is the TextView to
// which this line number area belongs to
TVLineNumberArea::TVLineNumberArea(TextView *viewer) : QWidget(viewer)
{
    m_TextViewer = viewer;
}

// Implements QWidget::sizeHint();
// Asks the Codeviewer which width should it take
QSize TVLineNumberArea::sizeHint() const
{
    return QSize(m_TextViewer->CalculateLineNumberAreaWidth(), 0);
}

// The line number area delegates its rendering
// to the TextView that owns it
void TVLineNumberArea::paintEvent(QPaintEvent *event)
{
    m_TextViewer->LineNumberAreaPaintEvent(event);
}
