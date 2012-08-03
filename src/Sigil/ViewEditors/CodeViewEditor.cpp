/************************************************************************
**
**  Copyright (C) 2009, 2010, 2011  Strahinja Markovic  <strahinja.markovic@gmail.com>, Nokia Corporation
**  Copyright (C) 2012 John Schember <john@nachtimwald.com>
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

#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>

#include <QtGui/QContextMenuEvent>
#include <QtCore/QSignalMapper>
#include <QtGui/QAction>
#include <QtGui/QMenu>
#include <QtGui/QPainter>
#include <QtGui/QScrollBar>
#include <QtGui/QShortcut>
#include <QtXml/QXmlStreamReader>

#include "BookManipulation/Book.h"
#include "BookManipulation/XercesCppUse.h"
#include "BookManipulation/XhtmlDoc.h"
#include "Misc/XHTMLHighlighter.h"
#include "Misc/CSSHighlighter.h"
#include "Misc/SettingsStore.h"
#include "Misc/SpellCheck.h"
#include "Misc/HTMLSpellCheck.h"
#include "Misc/Utility.h"
#include "PCRE/PCRECache.h"
#include "ViewEditors/CodeViewEditor.h"
#include "ViewEditors/LineNumberArea.h"
#include "sigil_constants.h"

using boost::make_tuple;
using boost::shared_ptr;
using boost::tie;
using boost::tuple;

static const int COLOR_FADE_AMOUNT       = 175;
static const int TAB_SPACES_WIDTH        = 4;
static const int BASE_FONT_SIZE          = 10;
static const int LINE_NUMBER_MARGIN      = 5;
static const QColor NUMBER_AREA_BGCOLOR  = QColor( 225, 225, 225 );
static const QColor NUMBER_AREA_NUMCOLOR = QColor( 125, 125, 125 );

static const QString XML_OPENING_TAG        = "(<[^>/][^>]*[^>/]>|<[^>/]>)";
static const QString NEXT_OPEN_TAG_LOCATION = "<\\s*(?!/)";

static const int MAX_SPELLING_SUGGESTIONS = 10;


CodeViewEditor::CodeViewEditor( HighlighterType high_type, bool check_spelling, QWidget *parent )
    :
    QPlainTextEdit( parent ),
    m_isUndoAvailable( false ),
    m_LastBlockCount( 0 ),
    m_LineNumberAreaBlockNumber( -1 ),
    m_LineNumberArea( new LineNumberArea( this ) ),
    m_ScrollOneLineUp( *(   new QShortcut( QKeySequence( Qt::ControlModifier + Qt::Key_Up   ), this, 0, 0, Qt::WidgetShortcut ) ) ),
    m_ScrollOneLineDown( *( new QShortcut( QKeySequence( Qt::ControlModifier + Qt::Key_Down ), this, 0, 0, Qt::WidgetShortcut ) ) ),
    m_isLoadFinished( false ),
    m_DelayedCursorScreenCenteringRequired( false ),
    m_caretLocation(0),
    m_checkSpelling( check_spelling ),
    m_spellingMapper( new QSignalMapper( this ) ),
    m_addSpellingMapper( new QSignalMapper( this ) ),
    m_ignoreSpellingMapper( new QSignalMapper( this ) ),
    m_BackToLinkAllowed(false)
{
    if ( high_type == CodeViewEditor::Highlight_XHTML )

        m_Highlighter = new XHTMLHighlighter( check_spelling, this );

    else

        m_Highlighter = new CSSHighlighter( this );

    setFocusPolicy( Qt::StrongFocus );

    ConnectSignalsToSlots();
    UpdateLineNumberAreaMargin();
    HighlightCurrentLine();

    setFrameStyle( QFrame::NoFrame );

    // Set the Zoom factor but be sure no signals are set because of this.
    SettingsStore settings;
    m_CurrentZoomFactor = settings.zoomText();
    Zoom();
}


QSize CodeViewEditor::sizeHint() const
{
    return QSize( 16777215, 16777215 );
}


void CodeViewEditor::CustomSetDocument( QTextDocument &document )
{
    setDocument( &document );
    document.setModified( false );

    m_Highlighter->setDocument( &document );

    ResetFont();

    m_isLoadFinished = true;
}

void CodeViewEditor::CutCodeTags()
{
    // If selection starts or ends in the middle of a tag, then do nothing
    if (!IsCutCodeTagsAllowed()) {
        return;
    }

    QTextCursor cursor = textCursor();
    int start = cursor.selectionStart();

    QString selected_text = textCursor().selectedText();
    QString new_text = StripCodeTags(selected_text);

    cursor.beginEditBlock();
    cursor.removeSelectedText();
    cursor.insertText(new_text);
    cursor.endEditBlock();

    cursor.setPosition(start);
    cursor.setPosition(start + new_text.count(), QTextCursor::KeepAnchor);
    setTextCursor(cursor);

}

bool CodeViewEditor::IsNotInTagTextSelected()
{
    return textCursor().hasSelection() && !(IsPositionInTag(textCursor().selectionStart() - 1) || IsPositionInTag(textCursor().selectionEnd() - 1));
}

bool CodeViewEditor::IsCutCodeTagsAllowed()
{
    return IsNotInTagTextSelected();
}

bool CodeViewEditor::IsInsertClosingTagAllowed()
{
    return !IsPositionInTag(textCursor().position() - 1);
}

bool CodeViewEditor::IsPositionInTag(int pos)
{
    QString text = toPlainText();

    while (pos > 0 && text[pos] != QChar('<') && text[pos] != QChar('>')) {
        pos--;
    }

    if (pos >= 0 && text[pos] == QChar('<')) {
        return true;
    }

    return false;
}

QString CodeViewEditor::StripCodeTags(QString text)
{
    QString new_text;
    bool in_tag = false;

    // Remove anything between and including < and > 
    for (int i = 0; i < text.count(); i++) {
        QChar c = text.at(i);
        if (!in_tag && c != QChar('<')) {
            new_text.append(c);
        }

        if (c == QChar('<')) {
            in_tag = true;
        }
        if (in_tag && c == QChar('>')) {
            in_tag = false;
        }
    }

    return new_text;
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


void CodeViewEditor::InsertClosingTag()
{
    if (!IsInsertClosingTagAllowed()) {
        return;
    }

    int pos = textCursor().position() - 1;

    QString text = toPlainText();
    int text_length = toPlainText().count();

    QList<QString> tags;
    QString tag;

    // Search backwards for first unclosed tag
    while (true) {
        while (pos > 0 && text[pos] != QChar('<')) {
            pos--;
        }

        if (pos <= 0 || pos >= text_length - 2) {
            return;
        }

        // Save position while we get the tag name
        int lastpos = pos;
        pos++;

        // Found a tag, see if its an opening or closing tag
        bool is_closing_tag = false;
        if (text[pos] == QChar('/')) {
            is_closing_tag = true;
            pos++;
        }

        // Get the tag name
        tag = "";
        while (pos < text_length && text[pos] != QChar(' ') && text[pos] != QChar('/') && text[pos] != QChar('>')) {
            tag.append(text[pos++]);
        }
        while (pos < text_length && text[pos] != QChar('/') && text[pos] != QChar('>')) {
            pos++;
        }

        // If not a self closing tag check for matched open/close tag
        if (text[pos] != QChar('/')) {
            if (pos >= text_length) {
                return;
            }
    
            if (is_closing_tag) {
                tags.append(tag);
            }
            else {
                // If body tag, skip to avoid accidentally inserting
                if (tag == "body") {
                    return;
                }

                // If no closing tags left to check then we're done
                if (tags.isEmpty()) {
                    break;
                }
    
                // If matching closing tag then remove it from list
                if (tag == tags.last()) {
                    tags.removeLast();   
                }
            }
        }

        pos = lastpos - 1;
    }

    if (!tag.isEmpty()) {
        QString closing_tag = "</" + tag + ">";

        textCursor().insertText(closing_tag);
    }
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

    // We loop through all the visible and
    // unobscured blocks and paint line numbers for each
    while ( block.isValid() )
    {
        // Getting the Y coordinates for the top of a block.
        int topY = (int) blockBoundingGeometry( block ).translated( contentOffset() ).top();

        // Ignore blocks that are not visible.
        if ( !block.isVisible() || ( topY > event->rect().bottom() ) )
        {
            break;
        }

        // Draw the number in the line number area.
        painter.setPen( NUMBER_AREA_NUMCOLOR );
        QString number_to_paint = QString::number( blockNumber );
        painter.drawText( - LINE_NUMBER_MARGIN,
                          topY,
                          m_LineNumberArea->width(),
                          fontMetrics().height(),
                          Qt::AlignRight,
                          number_to_paint
                          );

        // Move to the next block and block number.
        block = block.next();
        blockNumber++;
    }
}


void CodeViewEditor::LineNumberAreaMouseEvent( QMouseEvent *event )
{
    QTextCursor cursor = cursorForPosition( QPoint( 0, event->pos().y() ) );

    if ( event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick )
    {
        if ( event->button() == Qt::LeftButton )
        {
            QTextCursor selection = cursor;
            selection.setVisualNavigation( true );
            m_LineNumberAreaBlockNumber = selection.blockNumber();
            selection.movePosition( QTextCursor::EndOfBlock, QTextCursor::KeepAnchor );
            selection.movePosition( QTextCursor::Right, QTextCursor::KeepAnchor );
            setTextCursor( selection );
        }
    }

    else if ( m_LineNumberAreaBlockNumber >= 0 )
    {
        QTextCursor selection = cursor;
        selection.setVisualNavigation( true );

        if ( event->type() == QEvent::MouseMove )
        {
            QTextBlock anchorBlock = document()->findBlockByNumber( m_LineNumberAreaBlockNumber );
            selection.setPosition( anchorBlock.position() );

            if ( cursor.blockNumber() < m_LineNumberAreaBlockNumber )
            {
                selection.movePosition( QTextCursor::EndOfBlock );
                selection.movePosition( QTextCursor::Right );
            }

            selection.setPosition( cursor.block().position(), QTextCursor::KeepAnchor );

            if ( cursor.blockNumber() >= m_LineNumberAreaBlockNumber )
            {
                selection.movePosition( QTextCursor::EndOfBlock, QTextCursor::KeepAnchor );
                selection.movePosition( QTextCursor::Right, QTextCursor::KeepAnchor );
            }
        }

        else
        {
            m_LineNumberAreaBlockNumber = -1;
            return;
        }

        setTextCursor( selection );
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
    verticalScrollBar()->setValue(0);
}

void CodeViewEditor::ScrollToPosition(int cursor_position)
{
    if (cursor_position < 0) {
        return;
    }

    QTextCursor cursor(document());
    cursor.setPosition(cursor_position);
    setTextCursor(cursor);

    // If height is 0, then the widget is still collapsed
    // and centering the screen will do squat.
    if (height() > 0) {
        centerCursor();
    }
    else {
        m_DelayedCursorScreenCenteringRequired = true;
    }
}

void CodeViewEditor::ScrollToLine( int line )
{
    if (line <= 0) {
        return;
    }

    QTextCursor cursor(document());
    cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, line - 1);
    // Make sure the cursor ends up within a tag so that it stays in position on switching to Book View.
    cursor.movePosition(QTextCursor::NextWord);
    setTextCursor(cursor);

    // If height is 0, then the widget is still collapsed
    // and centering the screen will do squat.
    if (height() > 0) {
        centerCursor();
    }
    else {
        m_DelayedCursorScreenCenteringRequired = true;
    }
}


void CodeViewEditor::ScrollToFragment( const QString &fragment )
{
    if ( fragment.isEmpty() )
    {
        ScrollToLine( 1 );
        return;
    }

    QRegExp fragment_search( "id=\"" % fragment % "\"");
    int index = toPlainText().indexOf( fragment_search );

    // Count the number of newlines between the start of the text and the location of the 
    // desired fragment. This will be the line to scroll to.
    int line = toPlainText().left( index ).count( '\n' ) + 1;

    ScrollToLine( line );
}


bool CodeViewEditor::IsLoadingFinished()
{
    return m_isLoadFinished;
}

int CodeViewEditor::GetCursorPosition() const
{
    const int position = textCursor().position();
    return position;
}

int CodeViewEditor::GetCursorLine() const
{
    const QTextCursor cursor = textCursor();
    const QTextBlock block = cursor.block();
    const int line = block.blockNumber() + 1;
    return line;
}


int CodeViewEditor::GetCursorColumn() const
{
    const QTextCursor cursor = textCursor();
    const QTextBlock block = cursor.block();
    const int column = cursor.position() - block.position() + 1;
    return column;
}


void CodeViewEditor::SetZoomFactor( float factor )
{
    SettingsStore settings;
    settings.setZoomText(factor);
    m_CurrentZoomFactor = factor;
    Zoom();
    emit ZoomFactorChanged( factor );
}


float CodeViewEditor::GetZoomFactor() const
{
    SettingsStore settings;
    return settings.zoomText();
}


void CodeViewEditor::Zoom()
{
    QFont current_font = font();
    current_font.setPointSizeF( BASE_FONT_SIZE * m_CurrentZoomFactor );
    setFont( current_font );

    UpdateLineNumberAreaFont( current_font );
}


void CodeViewEditor::UpdateDisplay()
{
    SettingsStore settings;
    float stored_factor = settings.zoomText();
    if ( stored_factor != m_CurrentZoomFactor )
    {
        m_CurrentZoomFactor = stored_factor;
        Zoom();
    }
}

SPCRE::MatchInfo CodeViewEditor::GetMisspelledWord( const QString &text, int start_offset, int end_offset, const QString &search_regex, Searchable::Direction search_direction )
{
    SPCRE::MatchInfo match_info;

    HTMLSpellCheck::MisspelledWord misspelled_word;
    if ( search_direction == Searchable::Direction_Up )
    {
        if ( end_offset > 0 )
        {
            end_offset -= 1;
        }
        misspelled_word =  HTMLSpellCheck::GetLastMisspelledWord( text, start_offset, end_offset, search_regex );
    }
    else
    {
        misspelled_word =  HTMLSpellCheck::GetFirstMisspelledWord( text, start_offset, end_offset, search_regex );
    }
    if ( !misspelled_word.text.isEmpty() )
    {
        match_info.offset.first = misspelled_word.offset - start_offset;
        match_info.offset.second = match_info.offset.first + misspelled_word.length;
    }

    return match_info;
}

bool CodeViewEditor::FindNext( const QString &search_regex,
                               Searchable::Direction search_direction,
                               bool check_spelling,
                               bool ignore_selection_offset,
                               bool wrap )
{
    SPCRE *spcre = PCRECache::instance()->getObject(search_regex);

    int selection_offset = GetSelectionOffset( search_direction, ignore_selection_offset );
    SPCRE::MatchInfo match_info;
    int start_offset = 0;

    if ( search_direction == Searchable::Direction_Up )
    {
        if ( check_spelling )
        {
            match_info = GetMisspelledWord( toPlainText(), 0, selection_offset, search_regex, search_direction );
        }
        else
        {
            match_info = spcre->getLastMatchInfo(Utility::Substring(0, selection_offset, toPlainText()));
        }
    }
    else
    {
        if ( check_spelling )
        {
            match_info = GetMisspelledWord( toPlainText(), selection_offset, toPlainText().count(), search_regex, search_direction );
        }
        else
        {
            match_info = spcre->getFirstMatchInfo( Utility::Substring(selection_offset, toPlainText().count(), toPlainText() ));
        }
        start_offset = selection_offset;
    }

    m_lastMatch = match_info;

    if ( match_info.offset.first != -1 )
    {
        m_lastMatch.offset.first += start_offset;
        m_lastMatch.offset.second += start_offset;

        QTextCursor cursor = textCursor();
        if ( search_direction == Searchable::Direction_Up )
        {
            // Make sure there are 10 lines above/below if possible
            cursor.setPosition( match_info.offset.second + start_offset );
            setTextCursor( cursor );
            cursor.movePosition( QTextCursor::Down, QTextCursor::KeepAnchor, 10 );
            setTextCursor( cursor );
            cursor.movePosition( QTextCursor::Up, QTextCursor::KeepAnchor, 20 );
            setTextCursor( cursor );

            cursor.setPosition( match_info.offset.second + start_offset );
            cursor.setPosition( match_info.offset.first + start_offset, QTextCursor::KeepAnchor );
        }
        else
        {
            // Make sure there are 10 lines above/below if possible
            cursor.setPosition( match_info.offset.first + start_offset );
            setTextCursor( cursor );
            cursor.movePosition( QTextCursor::Up, QTextCursor::KeepAnchor, 10 );
            setTextCursor( cursor );
            cursor.movePosition( QTextCursor::Down, QTextCursor::KeepAnchor, 20 );
            setTextCursor( cursor );

            cursor.setPosition( match_info.offset.first + start_offset );
            cursor.setPosition( match_info.offset.second + start_offset, QTextCursor::KeepAnchor );
        }

        setTextCursor( cursor );

        return true;
    }
    else if ( wrap )
    {
        if ( FindNext( search_regex, search_direction, check_spelling, true, false ) )
        {
            ShowWrapIndicator(this);
            return true;
        }
    }

    return false;
}


int CodeViewEditor::Count( const QString &search_regex, bool check_spelling )
{
    int count = 0;
    if ( check_spelling )
    {
        count =  HTMLSpellCheck::CountMisspelledWords( toPlainText(), 0, toPlainText().count(), search_regex );
    }
    else
    {
        SPCRE *spcre = PCRECache::instance()->getObject( search_regex );
        count = spcre->getEveryMatchInfo( toPlainText() ).count();
    }
    return count;
}


bool CodeViewEditor::ReplaceSelected( const QString &search_regex, const QString &replacement, Searchable::Direction direction, bool check_spelling )
{
    SPCRE *spcre = PCRECache::instance()->getObject( search_regex );

    int selection_start = textCursor().selectionStart();
    QString selected_text = textCursor().selectedText();
    SPCRE::MatchInfo match_info;

    // Check if current selection is a match as well to handle highlighted text that is a
    // match, new files when replacing in all HTML, and misspelled words
    if ( check_spelling || !( m_lastMatch.offset.first == selection_start && m_lastMatch.offset.second == selection_start + selected_text.length() ) )
    {
        match_info = spcre->getFirstMatchInfo( selected_text );
        if ( match_info.offset.first != -1 )
        {
            m_lastMatch = match_info;
            m_lastMatch.offset.first = selection_start + match_info.offset.first;
            m_lastMatch.offset.second = selection_start + match_info.offset.second;
            selected_text = selected_text.mid( match_info.offset.first, match_info.offset.second - match_info.offset.first );
        }
    }

    // Check if the currently selected text is a match.
    match_info = spcre->getFirstMatchInfo( selected_text );
    if ( m_lastMatch.offset.first == selection_start && m_lastMatch.offset.second == selection_start + selected_text.length() && match_info.offset.first != -1 )
    {
        QString replaced_text;
        bool replacement_made = false;

        replacement_made = spcre->replaceText( selected_text, m_lastMatch.capture_groups_offsets, replacement, replaced_text );

        if ( replacement_made )
        {
            QTextCursor cursor = textCursor();

            // Replace the selected text with our replacemnt text.
            cursor.beginEditBlock();
            cursor.removeSelectedText();
            cursor.insertText( replaced_text );
            cursor.clearSelection();
            cursor.endEditBlock();

            // Set the cursor to the beginning of the replaced text if the user
            // is searching backward through the text.
            if ( direction == Searchable::Direction_Up )
            {
                cursor.setPosition( selection_start );
            }

            setTextCursor( cursor );

            return true;
        }
    }

    return false;
}


int CodeViewEditor::ReplaceAll( const QString &search_regex, 
                                const QString &replacement, 
                                bool check_spelling )
{
    Q_UNUSED (check_spelling);

    int count = 0;

    QString text = toPlainText();
    SPCRE *spcre = PCRECache::instance()->getObject(search_regex);
    QList<SPCRE::MatchInfo> match_info = spcre->getEveryMatchInfo(text);

    // Run though all match offsets making the replacment in reverse order.
    // This way changes in text lengh won't change the offsets as we make
    // our changes.
    for (int i = match_info.count() - 1; i >= 0; i--) {
        QString replaced_text;
        bool replacement_made = spcre->replaceText(Utility::Substring(match_info.at(i).offset.first, match_info.at(i).offset.second, text), match_info.at(i).capture_groups_offsets, replacement, replaced_text);

        if (replacement_made) {
            // Replace the text.
            text = text.replace(match_info.at(i).offset.first, match_info.at(i).offset.second - match_info.at(i).offset.first, replaced_text);
            count++;
        }
    }

    QTextCursor cursor = textCursor();
    // Store the cursor position
    int cursor_position = cursor.selectionStart();

    cursor.beginEditBlock();

    // Replace all text in the document with the new text.
    cursor.select(QTextCursor::Document);
    cursor.insertText(text);

    cursor.endEditBlock();

    // Restore the cursor position
    cursor.setPosition(cursor_position);
    setTextCursor(cursor);

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

void CodeViewEditor::LoadSettings()
{
    m_Highlighter->rehighlight();
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
        DelayedCursorScreenCentering();
    }

    return real_return;
}


void CodeViewEditor::StoreCaretLocation()
{
    m_caretLocation = textCursor().position();
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


void CodeViewEditor::mousePressEvent( QMouseEvent *event )
{
    // Rewrite the mouse event to a left button event so the cursor is
    // moved to the location of the pointer.
    if (event->button() == Qt::RightButton) {
        event = new QMouseEvent(QEvent::MouseButtonPress, event->pos(), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    }

    // Propagate to base class
    QPlainTextEdit::mousePressEvent( event );   
}


void CodeViewEditor::contextMenuEvent( QContextMenuEvent *event )
{
    QMenu *menu = createStandardContextMenu();

    bool offered_spelling = false;
    if (m_checkSpelling) {
        offered_spelling = AddSpellCheckContextMenu(menu);
    }

    if (!offered_spelling) {
        AddOpenLinkContextMenu(menu);
        AddIndexContextMenu(menu);
    }

    menu->exec(event->globalPos());
    delete menu;
}

bool CodeViewEditor::AddSpellCheckContextMenu(QMenu *menu)
{
    // The first action in the menu.
    QAction *topAction = 0;
    if (!menu->actions().isEmpty()) {
        topAction = menu->actions().at(0);
    }

    QTextCursor c = textCursor();
    // We check if offering spelling suggestions is necessary.
    //
    // If no text is selected we check the position of the cursor and see if it
    // is within a misspelled word position range. If so we select it and
    // offer spelling suggestions.
    //
    // If text is already selected we check if it matches a misspelled word
    // position range. If so we need to offer spelling suggestions.
    bool offer_spelling = false;

    // Ignore spell check if spelling is disabled.
    //
    // Check for misspelled words by looking at the formatting set by the SyntaxHighlighter.
    // By checking the formatting we can get a precalculated list of which words are
    // misspelled. This keeps us from having to run the spell check twice. This ensures
    // the same words shown on screen (by the SyntaxHighlighter) are the only words
    // that act as spell check. Also, this reduces code duplication because we don't
    // have the same word detection code in two places (here and the SyntaxHighlighter).
    // Plus, we don't have to worry about the detection here detecting differently in
    // the situation where the SyntaxHighlighter detection code is changed but the
    // code here is not or vice versa.
    if (m_checkSpelling) {
        // See if we are close to or inside of a misspelled word. If so select it.
        if (!c.hasSelection()) {
            // We cannot use QTextCursor::charFormat because the format is not set directly in
            // the document. The QSyntaxHighlighter sets the format in the block layout's
            // additionalFormats property. Thus we have to check if the cursor is within
            // an additionalFormat for the block and if that format is for a misspelled word.
            int pos = c.positionInBlock();
            foreach (QTextLayout::FormatRange r, textCursor().block().layout()->additionalFormats()) {
                if (pos > r.start && pos < r.start + r.length && r.format.underlineStyle() == QTextCharFormat::SpellCheckUnderline) {
                    c.setPosition(c.block().position() + r.start);
                    c.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, r.length);
                    setTextCursor(c);
                    offer_spelling = true;
                    break;
                }
            }
        }
        // Check if our selection is a misspelled word.
        else {
            int selStart = c.selectionStart() - c.block().position();
            int selLen = c.selectionEnd() - c.block().position() - selStart;
            foreach (QTextLayout::FormatRange r, textCursor().block().layout()->additionalFormats()) {
                if (r.start == selStart && selLen == r.length && r.format.underlineStyle() == QTextCharFormat::SpellCheckUnderline) {
                    offer_spelling = true;
                    break;
                }
            }
        }

        // If a misspelled word is selected try to offer spelling suggestions.
        if (offer_spelling && c.hasSelection()) {
            SpellCheck *sc = SpellCheck::instance();
            QString text = c.selectedText();

            QStringList suggestions = sc->suggest(text);
            QAction *suggestAction = 0;

            // We want to limit the number of suggestions so we don't
            // get a huge context menu.
            for (int i = 0; i < std::min(suggestions.length(), MAX_SPELLING_SUGGESTIONS); ++i) {
                suggestAction = new QAction(suggestions.at(i), menu);
                connect(suggestAction, SIGNAL(triggered()), m_spellingMapper, SLOT(map()));
                m_spellingMapper->setMapping(suggestAction, suggestions.at(i));

                // If the menu is empty we need to append rather than insert our actions.
                if (!topAction) {
                    menu->addAction(suggestAction);
                }
                else {
                    menu->insertAction(topAction, suggestAction);
                }
            }

            // Add a separator to keep our spelling actions differentiated from
            // the default menu actions.
            if (!suggestions.isEmpty() && topAction) {
                menu->insertSeparator(topAction);
            }

            // Allow the user to add the misspelled word to their user dictionary.
            QAction *addToDictAction = new QAction(tr("Add to dictionary"), menu);
            connect(addToDictAction, SIGNAL(triggered()), m_addSpellingMapper, SLOT(map()));
            m_addSpellingMapper->setMapping(addToDictAction, text);
            if (topAction) {
                menu->insertAction(topAction, addToDictAction);
                menu->insertSeparator(topAction);
            }
            else {
                menu->addAction(addToDictAction);
            }

            // Allow the user to ignore misspelled words until the program exits
            QAction *ignoreWordAction = new QAction(tr("Ignore"), menu);
            connect(ignoreWordAction, SIGNAL(triggered()), m_ignoreSpellingMapper, SLOT(map()));
            m_ignoreSpellingMapper->setMapping(ignoreWordAction, text);
            if (topAction) {
                menu->insertAction(topAction, ignoreWordAction);
                menu->insertSeparator(topAction);
            }
            else {
                menu->addAction(ignoreWordAction);
            }
        }
    }

    return offer_spelling;
}

void CodeViewEditor::AddOpenLinkContextMenu(QMenu *menu)
{
    QAction *topAction = 0;
    if (!menu->actions().isEmpty()) {
        topAction = menu->actions().at(0);
    }

    QAction *openLinkAction = new QAction(tr("Open Link"), menu);
    if (!topAction) {
        menu->addAction(openLinkAction);
    }
    else {
        menu->insertAction(topAction, openLinkAction);
    }
    connect(openLinkAction, SIGNAL(triggered()), this , SLOT(OpenLinkAction()));
    openLinkAction->setEnabled(!GetInternalLinkInTag().isEmpty());

    QAction *backToLinkAction = new QAction(tr("Back to Link"), menu);
    if (!topAction) {
        menu->addAction(backToLinkAction);
    }
    else {
        menu->insertAction(topAction, backToLinkAction);
    }
    connect(backToLinkAction, SIGNAL(triggered()), this , SLOT(BackToLinkAction()));
    backToLinkAction->setEnabled(m_BackToLinkAllowed);

    if (topAction) {
        menu->insertSeparator(topAction);
    }
}

void CodeViewEditor::AddIndexContextMenu(QMenu *menu)
{
    QTextCursor cursor = textCursor();
    if (cursor.selectedText().isEmpty()) {
        return;
    }

    QAction *topAction = 0;
    if (!menu->actions().isEmpty()) {
        topAction = menu->actions().at(0);
    }

    QAction *saveIndexAction = new QAction(tr("Add To Index"), menu);
    if (!topAction) {
        menu->addAction(saveIndexAction);
    }
    else {
        menu->insertAction(topAction, saveIndexAction);
    }
    connect(saveIndexAction, SIGNAL(triggered()), this , SLOT(SaveIndexAction()));
    saveIndexAction->setEnabled(IsNotInTagTextSelected());

    QAction *markIndexAction = new QAction(tr("Mark For Index"), menu);
    if (!topAction) {
        menu->addAction(markIndexAction);
    }
    else {
        menu->insertAction(topAction, markIndexAction);
    }
    connect(markIndexAction, SIGNAL(triggered()), this , SLOT(MarkIndexAction()));
    markIndexAction->setEnabled(IsNotInTagTextSelected());

    if (topAction) {
        menu->insertSeparator(topAction);
    }
}

bool CodeViewEditor::IsOpenAllowed()
{
    if (!IsPositionInTag(textCursor().position())) {
        return false;
    }
    return true;
}

QString CodeViewEditor::GetTagText()
{
    QString tag;
    QString text = toPlainText();

    int pos = textCursor().position();

    // Find the start of the tag
    while (pos > 0 && text[pos] != QChar('<') && text[pos] != QChar('>')) {
        pos--;
    }

    // Ignore if not in a tag
    if (pos <= 0 || text[pos] == QChar('>')) {
        return tag;
    }

    pos++;
    while (pos < text.length() && text[pos] != QChar('>')) {
        tag.append(text[pos]);
        pos++;
    }

    return tag;
}

QString CodeViewEditor::GetAttributeText(QString text, QString attribute)
{
    QString attribute_value;

    QString attribute_pattern = attribute % "=\"([^\"]*)\"";
    QRegExp attribute_search(attribute_pattern);

    int pos = attribute_search.indexIn(text);
    if (pos > -1) {
        attribute_value = attribute_search.cap(1);
    }

    return attribute_value;
}

void CodeViewEditor::SetBackToLinkAllowed(bool allowed)
{
    m_BackToLinkAllowed = allowed;
}

void CodeViewEditor::BackToLinkAction()
{
    if (m_BackToLinkAllowed) {
        emit OpenLastCodeLinkOpenedRequest();
    }
}

void CodeViewEditor::OpenLinkAction()
{
    QUrl url = GetInternalLinkInTag();

    if (!url.isEmpty()) {
        if (url.toString().startsWith("#")) {
            ScrollToFragment(url.fragment());
        }
        else if (url.scheme() == "file" || url.scheme().isEmpty()) {
            emit OpenCodeLinkRequest(url);
        }
        else {
            emit OpenExternalUrl(url);
        }
    }
}

QUrl CodeViewEditor::GetInternalLinkInTag()
{
    QUrl url;

    // Get the text of the tag containing the cursor position
    QString text = GetTagText();
    if (text.isEmpty()) {
        return url;
    }

    QString link = GetAttributeText(text, "href");
    if (link.isEmpty()) {
        link = GetAttributeText(text, "src");
    }

    url = QUrl(link);

    return url;
}

void CodeViewEditor::SaveIndexAction()
{
    if (!IsNotInTagTextSelected()) {
        return;
    }

    IndexEditorModel::indexEntry *index = new IndexEditorModel::indexEntry();

    QTextCursor cursor = textCursor();
    index->pattern = cursor.selectedText();

    emit OpenIndexEditorRequest(index);
}

void CodeViewEditor::MarkIndexAction()
{
    if (!IsNotInTagTextSelected()) {
        return;
    }

    QTextCursor cursor = textCursor();
    QString selected_text = cursor.selectedText();

    cursor.beginEditBlock();
    QString new_text = "<span class=\"" % SIGIL_INDEX_CLASS % "\" title=\"" % selected_text % "\">" % selected_text % "</span>";
    cursor.insertText(new_text);
    cursor.endEditBlock();
}

// Overridden so we can emit the FocusGained() signal.
void CodeViewEditor::focusInEvent( QFocusEvent *event )
{
    emit FocusGained( this );

    QPlainTextEdit::focusInEvent( event );
}


// Overridden so we can emit the FocusLost() signal.
void CodeViewEditor::focusOutEvent( QFocusEvent *event )
{
    emit FocusLost( this );

    StoreCaretLocation();

    QPlainTextEdit::focusOutEvent( event );
}


void CodeViewEditor::TextChangedFilter()
{
    m_lastMatch = SPCRE::MatchInfo();

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


void CodeViewEditor::ReplaceSelected(const QString &text)
{
    QTextCursor c = textCursor();
    c.insertText(text);
    setTextCursor(c);
}


void CodeViewEditor::addToUserDictionary(const QString &text)
{
    SpellCheck *sc = SpellCheck::instance();
    sc->addToUserDictionary(text);
    m_Highlighter->rehighlight();
}

void CodeViewEditor::ignoreWordInDictionary(const QString &text)
{
    SpellCheck *sc = SpellCheck::instance();
    sc->ignoreWord(text);
    m_Highlighter->rehighlight();
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


void CodeViewEditor::RestoreCaretLocation()
{
    QTextCursor c = textCursor();
    c.setPosition(m_caretLocation);
    setTextCursor(c);
}


void CodeViewEditor::SetDelayedCursorScreenCenteringRequired()
{
    m_DelayedCursorScreenCenteringRequired = true;
}

int CodeViewEditor::GetSelectionOffset( Searchable::Direction search_direction, bool ignore_selection_offset ) const
{
    if ( search_direction == Searchable::Direction_Down )
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
        {
            moveCursor( QTextCursor::Down );
        }
        else
        {
            moveCursor( QTextCursor::Up );
        }
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

    connect(m_spellingMapper, SIGNAL(mapped(const QString&)), this, SLOT(ReplaceSelected(const QString&)));
    connect(m_addSpellingMapper, SIGNAL(mapped(const QString&)), this, SLOT(addToUserDictionary(const QString&)));
    connect(m_ignoreSpellingMapper, SIGNAL(mapped(const QString&)), this, SLOT(ignoreWordInDictionary(const QString&)));
}
