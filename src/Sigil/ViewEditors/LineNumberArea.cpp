/************************************************************************
**
**  Copyright (C) 2009, 2010, 2011  Strahinja Markovic  <strahinja.markovic@gmail.com>, Nokia Corporation
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

#include <stdafx.h>
#include "LineNumberArea.h"
#include "CodeViewEditor.h"

// Constructor;
// The parameter is the CodeViewEditor to
// which this line number area belongs to
LineNumberArea::LineNumberArea( CodeViewEditor *editor ) : QWidget( editor )
{
    m_CodeEditor = editor;
}

// Updates the size of the line number area.
// Needed for zoom changes.
void LineNumberArea::MyUpdateGeometry()
{
    resize( m_CodeEditor->CalculateLineNumberAreaWidth(), size().height() );
}


// Implements QWidget::sizeHint();
// Asks the CodeEditor which width should it take
QSize LineNumberArea::sizeHint() const 
{
    return QSize( m_CodeEditor->CalculateLineNumberAreaWidth(), 0 );
}


// The line number area delegates its rendering
// to the CodeViewEditor that owns it
void LineNumberArea::paintEvent( QPaintEvent *event ) 
{
    m_CodeEditor->LineNumberAreaPaintEvent( event );
}


