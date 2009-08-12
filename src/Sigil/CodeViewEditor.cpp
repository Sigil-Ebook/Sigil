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

#include "stdafx.h"
#include "CodeViewEditor.h"
#include "LineNumberArea.h"

static const int COLOR_FADE_AMOUNT = 175;


// Constructor;
// the parameters is the object's parent
CodeViewEditor::CodeViewEditor( QWidget *parent )
    : QPlainTextEdit( parent )
{
    m_LineNumberArea = new LineNumberArea( this );

    connect(  this, SIGNAL( blockCountChanged( int ) ),             this, SLOT( UpdateLineNumberAreaMargin() ) );
    connect(  this, SIGNAL( updateRequest( const QRect &, int) ),   this, SLOT( UpdateLineNumberArea( const QRect &, int) ) );
    connect(  this, SIGNAL( cursorPositionChanged() ),              this, SLOT( HighlightCurrentLine() ) );

    UpdateLineNumberAreaMargin();
    HighlightCurrentLine();
}


// Paints the line number area;
// receives the event directly 
// from the area's paintEvent() handler
void CodeViewEditor::LineNumberAreaPaintEvent( QPaintEvent *event )
{
    QPainter painter( m_LineNumberArea );

    // Paint the background first
    painter.fillRect( event->rect(), Qt::lightGray );

    // A "block" represents a line of text
    QTextBlock block = firstVisibleBlock();

    // Blocks are numbered from zero,
    // but we count lines of text from one
    int blockNumber  = block.blockNumber() + 1;

    // Getting the Y coordinates for
    // the top and bottom of a block
    int topY    = (int) blockBoundingGeometry( block ).translated( contentOffset() ).top();
    int bottomY = topY + (int) blockBoundingRect( block ).height();

    // We loop through all the visible and
    // unobscured blocks and paint line numbers for each
    while ( block.isValid() && ( topY <= event->rect().bottom() ) )
    {
        if ( block.isVisible() && ( bottomY >= event->rect().top() ) )
        {
            QString number = QString::number( blockNumber );

            // Draw the line number
            painter.setPen( Qt::black );
            painter.drawText(   0,
                                topY,
                                m_LineNumberArea->width(),
                                fontMetrics().height(),
                                Qt::AlignRight,
                                number
                            );
        }

        block    = block.next();
        topY     = bottomY;
        bottomY  = topY + (int) blockBoundingRect( block ).height();

        blockNumber++;
    }
}

// Returns the width the LinuNumberArea
// should take (in pixels)
int CodeViewEditor::CalculateLineNumberAreaWidth()
{
    int num_digits       = 1;
    int last_line_number = blockCount();

    // We count the number of digits
    // for the line number of the last line
    while ( last_line_number >= 10 )
    {
        last_line_number /= 10;
        num_digits++;
    }

    int needed_width = 3 + fontMetrics().width( QChar( '0' ) ) * num_digits;

    return needed_width;
}


// The base class implementation of the print()
// method is not a slot, and we need it as a slot
// for print preview support; so this is just
// a slot wrapper around that function 
void CodeViewEditor::print( QPrinter* printer )
{
    QPlainTextEdit::print( printer );
}


// Overridden because after updating itself on a resize event,
// the editor needs to update its line number area too
void CodeViewEditor::resizeEvent( QResizeEvent *event )
{
    // Update self normally
    QPlainTextEdit::resizeEvent( event );

    QRect contents_area = contentsRect();

    // Now update the line number area
    m_LineNumberArea->setGeometry( QRect(   contents_area.left(), 
                                            contents_area.top(), 
                                            CalculateLineNumberAreaWidth(), 
                                            contents_area.height() 
                                        ) 
                                 );
}

// Called whenever the number of lines changes;
// sets a margin where the line number area can be displayed
void CodeViewEditor::UpdateLineNumberAreaMargin()
{ 
    // The left margin width depends on width of the line number area
    setViewportMargins( CalculateLineNumberAreaWidth(), 0, 0, 0 );
}

// The first parameter represents the area 
// that the editor needs an update of, and the second
// is the amount of pixels the viewport is vertically scrolled
void CodeViewEditor::UpdateLineNumberArea( const QRect &area_to_update, int vertically_scrolled )
{
    // If the editor scrolled, scroll the line numbers too
    if ( vertically_scrolled != 0 )

        m_LineNumberArea->scroll( 0, vertically_scrolled );

    else // otherwise update the required portion
    
        m_LineNumberArea->update( 0, area_to_update.y(), m_LineNumberArea->width(), area_to_update.height() );

    if ( area_to_update.contains( viewport()->rect() ) )

        UpdateLineNumberAreaMargin();
}


// Highlights the line the user is editing
void CodeViewEditor::HighlightCurrentLine()
{
    QList< QTextEdit::ExtraSelection > extraSelections;

    QTextEdit::ExtraSelection selection;

    QColor lineColor = QColor( Qt::yellow ).lighter( COLOR_FADE_AMOUNT );

    selection.format.setBackground( lineColor );
    
    // Specifies that we want the whole line to be selected
    selection.format.setProperty( QTextFormat::FullWidthSelection, true );

    // We reset the selection of the cursor
    // so that only one line is highlighted
    selection.cursor = textCursor();
    selection.cursor.clearSelection();

    extraSelections.append( selection );    
    setExtraSelections( extraSelections );
}






