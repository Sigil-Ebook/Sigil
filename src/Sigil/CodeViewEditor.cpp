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
#include "Book.h"
#include <QDomDocument>

static const int COLOR_FADE_AMOUNT = 175;
                  
static const QString XML_OPENING_TAG = "(<[^>/][^>]*[^>/]>|<[^>/]>)";


// Constructor;
// the parameters is the object's parent
CodeViewEditor::CodeViewEditor( QWidget *parent )
    : 
    QPlainTextEdit( parent ),
    m_CaretLocationUpdate( 0, 0 )
{
    m_LineNumberArea = new LineNumberArea( this );

    connect( this, SIGNAL( blockCountChanged( int ) ),             this, SLOT( UpdateLineNumberAreaMargin() ) );
    connect( this, SIGNAL( updateRequest( const QRect &, int) ),   this, SLOT( UpdateLineNumberArea( const QRect &, int) ) );
    connect( this, SIGNAL( cursorPositionChanged() ),              this, SLOT( HighlightCurrentLine()       ) );

    UpdateLineNumberAreaMargin();
    HighlightCurrentLine();
}

// Sets the content of the View to the specified book
void CodeViewEditor::SetBook( const Book &book )
{
    setPlainText( book.source );
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


// Returns a list of elements representing a "chain"
// or "walk" through the XHTML document with which one
// can identify a single element in the document.
// This list identifies the element in which the 
// keyboard caret is currently located.
QList< ViewEditor::ElementIndex > CodeViewEditor::GetCaretLocation()
{
    QRegExp tag( XML_OPENING_TAG );

    // We search for the first opening tag behind the caret.
    // This specifies the element the caret is located in.
    int offset = toPlainText().lastIndexOf( tag, textCursor().position() );

    return ConvertStackToHierarchy( GetCaretLocationStack( offset + tag.matchedLength() ) );
}


// Accepts a list returned by a view's GetCaretLocation
// and creates and stores an update that sends the caret
// in this view to the specified element.
// The CodeView implementation initiates the update in
// the paint event handler.
void CodeViewEditor::StoreCaretLocationUpdate( const QList< ViewEditor::ElementIndex > &hierarchy )
{
    QDomDocument dom;

    dom.setContent( toPlainText() );

    QDomNode node = dom.elementsByTagName( "html" ).at( 0 );
    QDomNode end_node;

    for ( int i = 0; i < hierarchy.count() - 1; i++ )
    {
        node = node.childNodes().at( hierarchy[ i ].index );

        if ( !node.isNull() )

            end_node = node;

        else

            break;
    }

    QTextCursor cursor( document() );

    if ( !end_node.isNull() ) 
    {
        // We can't set the actual caret location here;
        // that is done in the paint event handler.
        // Here we just calculate the caret update.
        m_CaretLocationUpdate.vertical_lines   = end_node.lineNumber() - cursor.blockNumber();
        m_CaretLocationUpdate.horizontal_chars = end_node.columnNumber();   
    }

    else
    {   
        m_CaretLocationUpdate.vertical_lines   = 0;
        m_CaretLocationUpdate.horizontal_chars = 0;
    } 
}


// The base class implementation of the print()
// method is not a slot, and we need it as a slot
// for print preview support; so this is just
// a slot wrapper around that function 
void CodeViewEditor::print( QPrinter* printer )
{
    QPlainTextEdit::print( printer );
}

// Overridden because we need to update the cursor
// location if a cursor update (from BookView) 
// is waiting to be processed
bool CodeViewEditor::event( QEvent *event )
{
    // We just return whatever the "real" event handler returns
    bool real_return = QPlainTextEdit::event( event );
    
    // Executing the caret update inside the paint event
    // handler causes artifacts on mac. So we do it after
    // the event is processed and accepted.
    if ( event->type() == QEvent::Paint )
    {
        ExecuteCaretUpdate();
    }
    
    return real_return;
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


// Overridden because we want the ExecuteCaretUpdate()
// to be called from here when the user clicks inside
// this widget in SplitView. Leaving it up to our event()
// override causes graphical artifacts for SplitView.
// So in those conditions, this handler takes over.
void CodeViewEditor::mousePressEvent( QMouseEvent *event )
{
    // Propagate to base class
    QPlainTextEdit::mousePressEvent( event );   

    // Run the caret update if it's pending
    ExecuteCaretUpdate();
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


// Returns a stack of elements representing the
// current location of the caret in the document.
// Accepts the number of characters to the end of
// the start tag of the element the caret is residing in. 
QStack< CodeViewEditor::StackElement > CodeViewEditor::GetCaretLocationStack( int offset )
{
    QString source = toPlainText();
    QXmlStreamReader reader( source );

    QStack< StackElement > stack; 

    while ( !reader.atEnd() ) 
    {
        // Get the next token from the stream
        QXmlStreamReader::TokenType type = reader.readNext();

        if ( type == QXmlStreamReader::StartElement ) 
        {
            // If we detected the start of a new element, then
            // the element currently on the top of the stack
            // has one more child element
            if ( !stack.isEmpty() )

                stack.top().num_children++;
            
            StackElement new_element;
            new_element.name         = reader.name().toString();
            new_element.num_children = 0;

            stack.push( new_element );

            // Check if this is the element start tag
            // we are looking for
            if ( reader.characterOffset() == offset  )

                break;
        }

        // If we detect the end tag of an element,
        // we remove it from the top of the stack
        else if ( type == QXmlStreamReader::EndElement )
        {
            stack.pop();
        }
    }

    if ( reader.hasError() )
    {
        // Just return an empty location.
        // Maybe we could return the stack we currently have?
        return QStack< StackElement >();
    }

    return stack;
}


// Converts the stack provided by GetCaretLocationStack()
// and converts it into the element location hierarchy
QList< ViewEditor::ElementIndex > CodeViewEditor::ConvertStackToHierarchy( const QStack< StackElement > stack )
{
    QList< ViewEditor::ElementIndex > hierarchy;

    foreach( StackElement stack_element, stack )
    {
        ViewEditor::ElementIndex new_element;

        new_element.name  = stack_element.name;
        new_element.index = stack_element.num_children - 1;

        hierarchy.append( new_element );
    }

    return hierarchy;
}

// Executes the caret updating code
// if an update is pending;
// returns true if update was performed
bool CodeViewEditor::ExecuteCaretUpdate()
{
    // If there's a cursor/caret update waiting (from BookView),
    // we update the caret location and reset the update variable
    if (    m_CaretLocationUpdate.vertical_lines   == 0 &&
            m_CaretLocationUpdate.horizontal_chars == 0 
       )
    {
        return false;
    }

    QTextCursor cursor( document() );

    cursor.movePosition( QTextCursor::NextBlock, QTextCursor::MoveAnchor, m_CaretLocationUpdate.vertical_lines );
    cursor.movePosition( QTextCursor::Left     , QTextCursor::MoveAnchor, m_CaretLocationUpdate.horizontal_chars );

    m_CaretLocationUpdate.vertical_lines   = 0;
    m_CaretLocationUpdate.horizontal_chars = 0; 

    setTextCursor( cursor );

    // Center the screen on the cursor/caret location.
    // Centering requires fresh information about the
    // visible viewport, so we usually call this after
    // the paint event has been processed.
    centerCursor();

    return true;
}




