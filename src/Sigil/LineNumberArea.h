/************************************************************************
**
**  Copyright (C) 2009  Strahinja Markovic, Nokia Corporation
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
#ifndef LINENUMBERAREA_H
#define LINENUMBERAREA_H

class CodeViewEditor;

class LineNumberArea : public QWidget
{

public:

    // Constructor;
    // The parameter is the CodeViewEditor to
    // which this line number area belongs to
    LineNumberArea( CodeViewEditor *editor );

    // Implements QWidget::sizeHint();
    // Asks the CodeViewEditor which width should it take
    QSize sizeHint() const; 

protected:

    // The line number area delegates its rendering
    // to the CodeViewEditor that owns it
    void paintEvent( QPaintEvent *event );

private:

    // The CodeViewEditor to which
    // this line number area belongs to
    CodeViewEditor *m_CodeEditor;
};

#endif // LINENUMBERAREA_H