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
#include <QContextMenuEvent>
#include "CodeViewEditor.h"
#include "LineNumberArea.h"
#include "BookManipulation/Book.h"
#include "BookManipulation/XhtmlDoc.h"
#include "Misc/XHTMLHighlighter.h"
#include "Misc/CSSHighlighter.h"
#include "Misc/Utility.h"

static const int COLOR_FADE_AMOUNT       = 175;
static const int TAB_SPACES_WIDTH        = 4;
static const int BASE_FONT_SIZE          = 10;
static const int LINE_NUMBER_MARGIN      = 5;
static const QColor NUMBER_AREA_BGCOLOR  = QColor( 230, 230, 230 );
static const QColor NUMBER_AREA_NUMCOLOR = QColor( 100, 100, 100 );
                  
static const QString XML_OPENING_TAG        = "(<[^>/][^>]*[^>/]>|<[^>/]>)";
static const QString NEXT_OPEN_TAG_LOCATION = "<\\s*(?!/)";


CodeViewEditor::CodeViewEditor( HighlighterType high_type, QWidget *parent )
    :
    QPlainTextEdit( parent ),
    m_isUndoAvailable( false ),
    m_LastBlockCount( 0 ),
    m_LineNumberArea( new LineNumberArea( this ) ),
    m_CurrentZoomFactor( 1.0 ),
    m_ScrollOneLineUp( *(   new QShortcut( QKeySequence( Qt::ControlModifier + Qt::Key_Up   ), this, 0, 0, Qt::WidgetShortcut ) ) ),
    m_ScrollOneLineDown( *( new QShortcut( QKeySequence( Qt::ControlModifier + Qt::Key_Down ), this, 0, 0, Qt::WidgetShortcut ) ) ),
    m_isLoadFinished( false ),
    m_DelayedCursorScreenCenteringRequired( false )
{
    if ( high_type == CodeViewEditor::Highlight_XHTML )

        m_Highlighter = new XHTMLHighlighter( this );

    else

        m_Highlighter = new CSSHighlighter( this );

    setFocusPolicy( Qt::StrongFocus );

    ConnectSignalsToSlots();
    UpdateLineNumberAreaMargin();
    HighlightCurrentLine();

    setFrameStyle( QFrame::NoFrame );
}


void CodeViewEditor::CustomSetDocument( QTextDocument &document )
{
    setDocument( &document );
    m_Highlighter->setDocument( &document );

    ResetFont();

    m_isLoadFinished = true;
}


QString CodeViewEditor::SplitChapter()
{
    QString text = toPlainText();

    QRegExp body_search( BODY_START ); 
    int body_tag_start = text.indexOf( body_search );
    int body_tag_end   = body_tag_start + body_search.matchedLength();

    QRegExp body_end_search( BODY_END );
    int body_contents_end = text.indexOf( body_end_search );

    QString head = text.left( body_tag_start );

    int next_open_tag_index = text.indexOf( QRegExp( NEXT_OPEN_TAG_LOCATION ), textCursor().position() );
    if ( next_open_tag_index == -1 )
    {
        // Cursor is at end of file
        next_open_tag_index = body_contents_end;
    }
    else
    {
        if ( next_open_tag_index < body_tag_end )
        {
            // Cursor is before the start of the body
            next_open_tag_index = body_tag_end;
        }
    }

    const QString &text_segment = next_open_tag_index != body_tag_end                             ? 
                                  Utility::Substring( body_tag_start, next_open_tag_index, text ) :
                                  QString( "<p>&nbsp;</p>" );

    // Remove the text that will be in 
    // the new chapter from the View.
    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();
    cursor.setPosition( body_tag_end );
    cursor.setPosition( next_open_tag_index, QTextCursor::KeepAnchor );
    cursor.removeSelectedText();

    // We add a newline if the next tag
    // is sitting right next to the end of the body tag.
    if ( toPlainText().at( body_tag_end ) == QChar( '<' ) )
        
        cursor.insertBlock();
    
    cursor.endEditBlock();

    return QString()
           .append( head )
           .append( text_segment )
           .append( "</body></html>" );
}


void CodeViewEditor::InsertSGFChapterMarker()
{
    textCursor().insertText( BREAK_TAG_INSERT );
}


void CodeViewEditor::LineNumberAreaPaintEvent( QPaintEvent *event )
{
    QPainter painter( m_LineNumberArea );

    // Paint the background first
    painter.fillRect( event->rect(), NUMBER_AREA_BGCOLOR );

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
            QString number_to_paint = QString::number( blockNumber );

            // Draw the line number
            painter.setPen( NUMBER_AREA_NUMCOLOR );

            painter.drawText( - LINE_NUMBER_MARGIN,
                              topY,
                              m_LineNumberArea->width(),
                              fontMetrics().height(),
                              Qt::AlignRight,
                              number_to_paint
                            );
        }

        block   = block.next();
        topY    = bottomY;
        bottomY = topY + (int) blockBoundingRect( block ).height();

        blockNumber++;
    }
}


int CodeViewEditor::CalculateLineNumberAreaWidth()
{
    int current_block_count = blockCount();

    // QTextDocument::setPlainText sets the current block count
    // to 1 before updating it (for no damn good reason), but  
    // we need it to *not* be 1, ever.
    int last_line_number = current_block_count != 1 ? current_block_count : m_LastBlockCount;
    m_LastBlockCount = last_line_number;

    int num_digits = 1;

    // We count the number of digits
    // for the line number of the last line
    while ( last_line_number >= 10 )
    {
        last_line_number /= 10;
        num_digits++;
    }

    return LINE_NUMBER_MARGIN * 2 + fontMetrics().width( QChar( '0' ) ) * num_digits;
}


void CodeViewEditor::ReplaceDocumentText( const QString &new_text )
{
    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();

    cursor.select( QTextCursor::Document );
    cursor.removeSelectedText();
    cursor.insertText( new_text );

    cursor.endEditBlock();
}


void CodeViewEditor::ScrollToTop()
{
    verticalScrollBar()->setValue( 0 );
}


void CodeViewEditor::ScrollToLine( int line )
{
    if ( line <= 0 )

        return;

    // A pending caret update will overrule us, 
    // and we don't want that.
    m_CaretUpdate.clear();

    QTextCursor cursor( document() );
    cursor.movePosition( QTextCursor::NextBlock, QTextCursor::MoveAnchor, line - 1 );
    setTextCursor( cursor );

    // If height is 0, then the widget is still collapsed
    // and centering the screen will do squat.
    if ( height() > 0 )
    
        centerCursor();

    else

        m_DelayedCursorScreenCenteringRequired = true;
}


QList< ViewEditor::ElementIndex > CodeViewEditor::GetCaretLocation()
{
    QRegExp tag( XML_OPENING_TAG );

    // We search for the first opening tag *behind* the caret.
    // This specifies the element the caret is located in.
    int offset = toPlainText().lastIndexOf( tag, textCursor().position() );

    return ConvertStackToHierarchy( GetCaretLocationStack( offset + tag.matchedLength() ) );
}


void CodeViewEditor::StoreCaretLocationUpdate( const QList< ViewEditor::ElementIndex > &hierarchy )
{
    m_CaretUpdate = hierarchy;
}


bool CodeViewEditor::IsLoadingFinished()
{
    return m_isLoadFinished;
}


void CodeViewEditor::SetZoomFactor( float factor )
{
    m_CurrentZoomFactor = factor;

    QFont current_font = font();
    current_font.setPointSizeF( BASE_FONT_SIZE * m_CurrentZoomFactor );
    setFont( current_font );
    
    UpdateLineNumberAreaFont( current_font );

    emit ZoomFactorChanged( factor );
}


float CodeViewEditor::GetZoomFactor() const
{
    return m_CurrentZoomFactor;
}


bool CodeViewEditor::FindNext( const QRegExp &search_regex, 
                               Searchable::Direction search_direction,
                               bool ignore_selection_offset )
{
    int selection_offset = GetSelectionOffset( search_direction, ignore_selection_offset );

    QRegExp result_regex = search_regex;
    RunSearchRegex( result_regex, toPlainText(), selection_offset, search_direction ); 

    if ( result_regex.pos() != -1 )
    {
        QTextCursor cursor = textCursor();

        cursor.setPosition( result_regex.pos() );
        cursor.setPosition( result_regex.pos() + result_regex.matchedLength(), QTextCursor::KeepAnchor );

        setTextCursor( cursor );

        return true;
    } 

    return false;
}


int CodeViewEditor::Count( const QRegExp &search_regex )
{
    return toPlainText().count( search_regex );
}


bool CodeViewEditor::ReplaceSelected( const QRegExp &search_regex, const QString &replacement )
{
    int selection_start = textCursor().selectionStart();

    QRegExp result_regex = search_regex;
    RunSearchRegex( result_regex, toPlainText(), selection_start, Searchable::Direction_Down ); 

    // If we are currently sitting at the start 
    // of a matching substring, we replace it.
    if ( result_regex.pos() == selection_start )
    {
        QString final_replacement = FillWithCapturedTexts( result_regex.capturedTexts(), replacement );
        textCursor().insertText( final_replacement );

        return true;
    }

    return false;
}


int CodeViewEditor::ReplaceAll( const QRegExp &search_regex, const QString &replacement )
{
    QRegExp result_regex  = search_regex;
    QTextCursor cursor    = textCursor();

    int index = 0;
    int count = 0;

    QProgressDialog progress( tr( "Replacing search term..." ), QString(), 0, Count( search_regex ) );
    progress.setMinimumDuration( PROGRESS_BAR_MINIMUM_DURATION );

    // This is one edit operation, so all of it
    // can be undone with undo.
    cursor.beginEditBlock();

    while ( toPlainText().indexOf( result_regex, index ) != -1 )
    {
        // Update the progress bar
        progress.setValue( count );

        cursor.setPosition( result_regex.pos() );
        cursor.setPosition( result_regex.pos() + result_regex.matchedLength(), QTextCursor::KeepAnchor );

        QString final_replacement = FillWithCapturedTexts( result_regex.capturedTexts(), replacement );
        cursor.insertText( final_replacement );

        index = result_regex.pos() + final_replacement.length();
        ++count;
    }

    cursor.endEditBlock();

    return count;
}


QString CodeViewEditor::GetSelectedText()
{
    return textCursor().selectedText();
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
        DelayedCursorScreenCentering();
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
    m_LineNumberArea->setGeometry( QRect( contents_area.left(), 
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

// Overridden so we can block the focus out signal.
// Right clicking and calling the context menu will cause the
// editor to loose focus. When it looses focus the code is checked
// if it is well formed. If it is not a message box is shown asking
// if the user would like to auto correct. This causes the context
// menu to disappear and thus be inaccessable to the user.
void CodeViewEditor::contextMenuEvent( QContextMenuEvent *event )
{
    blockSignals( true );
    QMenu *menu = createStandardContextMenu();
    menu->exec( event->globalPos() );
    delete menu;
    blockSignals( false );
}


// Overridden so we can emit the FocusGained() signal.
void CodeViewEditor::focusInEvent( QFocusEvent *event )
{
    emit FocusGained();

    QPlainTextEdit::focusInEvent( event );
}


// Overridden so we can emit the FocusLost() signal.
void CodeViewEditor::focusOutEvent( QFocusEvent *event )
{
    emit FocusLost();

    QPlainTextEdit::focusOutEvent( event );
}


void CodeViewEditor::TextChangedFilter()
{
    if ( m_isUndoAvailable )

        emit FilteredTextChanged();
}


void CodeViewEditor::UpdateUndoAvailable( bool available )
{
    m_isUndoAvailable = available;
}


void CodeViewEditor::UpdateLineNumberAreaMargin()
{ 
    // The left margin width depends on width of the line number area
    setViewportMargins( CalculateLineNumberAreaWidth(), 0, 0, 0 );
}


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


void CodeViewEditor::ScrollOneLineUp()
{
    ScrollByLine( false );
}


void CodeViewEditor::ScrollOneLineDown()
{
    ScrollByLine( true );
}


void CodeViewEditor::ResetFont()
{
    // Let's try to use Consolas as our font
    QFont font( "Consolas", BASE_FONT_SIZE );

    // But just in case, say we want a fixed width font
    // if Consolas is not on the system
    font.setStyleHint( QFont::TypeWriter );
    setFont( font );
    setTabStopWidth( TAB_SPACES_WIDTH * QFontMetrics( font ).width( ' ' ) );

    UpdateLineNumberAreaFont( font );
}


void CodeViewEditor::UpdateLineNumberAreaFont( const QFont &font )
{
    m_LineNumberArea->setFont( font );
    m_LineNumberArea->MyUpdateGeometry();
    UpdateLineNumberAreaMargin();
}


QStack< CodeViewEditor::StackElement > CodeViewEditor::GetCaretLocationStack( int offset ) const
{
    QString source = toPlainText();
    QXmlStreamReader reader( source );

    QStack< StackElement > stack; 

    while ( !reader.atEnd() ) 
    {
        reader.readNext();

        if ( reader.isStartElement() ) 
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
        else if ( reader.isEndElement() )
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


QList< ViewEditor::ElementIndex > CodeViewEditor::ConvertStackToHierarchy( const QStack< StackElement > stack ) const
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


tuple< int, int > CodeViewEditor::ConvertHierarchyToCaretMove( const QList< ViewEditor::ElementIndex > &hierarchy ) const
{
    shared_ptr< xc::DOMDocument > dom = XhtmlDoc::LoadTextIntoDocument( toPlainText() );

    xc::DOMNode *end_node = XhtmlDoc::GetNodeFromHierarchy( *dom, hierarchy );
    QTextCursor cursor( document() );

    if ( end_node )
    
        return make_tuple( XhtmlDoc::NodeLineNumber( *end_node ) - cursor.blockNumber(), 
                           XhtmlDoc::NodeColumnNumber( *end_node ) ); 
    
    else
    
        return make_tuple( 0, 0 );
}


bool CodeViewEditor::ExecuteCaretUpdate()
{
    // If there's a cursor/caret update waiting (from BookView),
    // we update the caret location and reset the update variable
    if ( m_CaretUpdate.count() == 0 )
    {
        return false;
    }

    QTextCursor cursor( document() );

    int vertical_lines_move = 0;
    int horizontal_chars_move = 0;

    // We *have* to do the conversion on-demand since the 
    // conversion uses toPlainText(), and the text needs to up-to-date.
    tie( vertical_lines_move, horizontal_chars_move ) = ConvertHierarchyToCaretMove( m_CaretUpdate );

    cursor.movePosition( QTextCursor::NextBlock, QTextCursor::MoveAnchor, vertical_lines_move - 1 );

    for( int i = 1 ; i < horizontal_chars_move ; i++ )
    {
        cursor.movePosition( QTextCursor::NextCharacter , QTextCursor::MoveAnchor );
        // TODO: cursor.movePosition( QTextCursor::Left, ...) is badly bugged in Qt 4.7.
        // Test whether it's fixed when the next version of Qt comes out.
        // cursor.movePosition( QTextCursor::Left, QTextCursor::MoveAnchor, horizontal_chars_move );
    }

    m_CaretUpdate.clear();
    setTextCursor( cursor );

    m_DelayedCursorScreenCenteringRequired = true;

    return true;
}

// Center the screen on the cursor/caret location.
// Centering requires fresh information about the
// visible viewport, so we usually call this after
// the paint event has been processed.
void CodeViewEditor::DelayedCursorScreenCentering()
{
     if ( m_DelayedCursorScreenCenteringRequired )
    {
        centerCursor();

        m_DelayedCursorScreenCenteringRequired = false;
    }
}


int CodeViewEditor::GetSelectionOffset( Searchable::Direction search_direction, bool ignore_selection_offset ) const
{
    if ( search_direction == Searchable::Direction_Down ||
         search_direction == Searchable::Direction_All
       )
    {
        return !ignore_selection_offset ? textCursor().selectionEnd() : 0;
    }

    else
    {
        return !ignore_selection_offset ? textCursor().selectionStart() : toPlainText().count() - 1;
    }
}


void CodeViewEditor::ScrollByLine( bool down )
{
    int current_scroll_value = verticalScrollBar()->value();
    int move_delta = down ? 1 : -1;

    verticalScrollBar()->setValue( current_scroll_value + move_delta );

    if ( !contentsRect().contains( cursorRect() ) )
    {
        if ( move_delta > 0 )

            moveCursor( QTextCursor::Down );

        else

            moveCursor( QTextCursor::Up );
    }
       
}


void CodeViewEditor::ConnectSignalsToSlots()
{
    connect( this, SIGNAL( blockCountChanged( int )           ), this, SLOT( UpdateLineNumberAreaMargin()              ) );
    connect( this, SIGNAL( updateRequest( const QRect&, int ) ), this, SLOT( UpdateLineNumberArea( const QRect&, int ) ) );
    connect( this, SIGNAL( cursorPositionChanged()            ), this, SLOT( HighlightCurrentLine()                    ) );
    connect( this, SIGNAL( textChanged()                      ), this, SLOT( TextChangedFilter()                       ) );
    connect( this, SIGNAL( undoAvailable( bool )              ), this, SLOT( UpdateUndoAvailable( bool )               ) );

    connect( &m_ScrollOneLineUp,   SIGNAL( activated() ), this, SLOT( ScrollOneLineUp()   ) );
    connect( &m_ScrollOneLineDown, SIGNAL( activated() ), this, SLOT( ScrollOneLineDown() ) );
}


