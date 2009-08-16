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
#ifndef CODEVIEWEDITOR_H
#define CODEVIEWEDITOR_H

#include <QPlainTextEdit>
#include <QObject>

class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
class QPrinter;
class LineNumberArea;
class Book;


class CodeViewEditor : public QPlainTextEdit
{
    Q_OBJECT

public:

    // Constructor;
    // the parameters is the object's parent
    CodeViewEditor( QWidget *parent = 0 );

    // Sets the content of the View to the specified book
    void SetBook( const Book &book );

    // Paints the line number area;
    // receives the event directly 
    // from the area's paintEvent() handler
    void LineNumberAreaPaintEvent( QPaintEvent *event );

    // Returns the width the LinuNumberArea
    // should take (in pixels)
    int CalculateLineNumberAreaWidth();

public slots:

    // The base class implementation of the print()
    // method is not a slot, and we need it as a slot
    // for print preview support; so this is just
    // a slot wrapper around that function 
    void print( QPrinter* printer );

protected:

    // Overridden because after updating itself on a resize event,
    // the editor needs to update its line number area too
    void resizeEvent( QResizeEvent *event );

private slots:

    // Called whenever the number of lines changes;
    // sets a margin where the line number area can be displayed
    void UpdateLineNumberAreaMargin();

    // The first parameter represents the area 
    // that the editor needs an update of, and the second
    // is the amount of pixels the viewport is vertically scrolled
    void UpdateLineNumberArea( const QRect &rectangle, int vertical_delta );

    // Highlights the line the user is editing
    void HighlightCurrentLine();

private:

    // The line number area widget of the code view
    LineNumberArea *m_LineNumberArea;
};

#endif // CODEVIEWEDITOR_H

