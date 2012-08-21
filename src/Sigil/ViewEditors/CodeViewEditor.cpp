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
#include "Dialogs/ClipEditor.h"
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
static const QString NEXT_TAG_LOCATION      = "<[^>]+>";
static const QString TAG_NAME_SEARCH        = "<\\s*([^\\s>]+)";

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
    m_CaretUpdate( QList< ViewEditor::ElementIndex >() ),
    m_checkSpelling( check_spelling ),
    m_spellingMapper( new QSignalMapper( this ) ),
    m_addSpellingMapper( new QSignalMapper( this ) ),
    m_ignoreSpellingMapper( new QSignalMapper( this ) ),
    m_clipMapper( new QSignalMapper( this ) ),
    m_pendingClipEntryRequest( NULL ),
    m_pendingGoToStyleDefinitionRequest( false )
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

bool CodeViewEditor::IsGoToStyleDefinitionAllowed()
{
    return IsPositionInOpeningTag();
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

    m_CaretUpdate.clear();
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
    ScrollToPosition(index);
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
                               bool misspelled_words,
                               bool ignore_selection_offset,
                               bool wrap )
{
    SPCRE *spcre = PCRECache::instance()->getObject(search_regex);

    int selection_offset = GetSelectionOffset( search_direction, ignore_selection_offset );
    SPCRE::MatchInfo match_info;
    int start_offset = 0;

    if ( search_direction == Searchable::Direction_Up )
    {
        if ( misspelled_words )
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
        if ( misspelled_words )
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
        if ( FindNext( search_regex, search_direction, misspelled_words, true, false ) )
        {
            ShowWrapIndicator(this);
            return true;
        }
    }

    return false;
}


int CodeViewEditor::Count( const QString &search_regex )
{
    SPCRE *spcre = PCRECache::instance()->getObject( search_regex );
    return spcre->getEveryMatchInfo( toPlainText() ).count();
}


bool CodeViewEditor::ReplaceSelected( const QString &search_regex, const QString &replacement, Searchable::Direction direction )
{
    SPCRE *spcre = PCRECache::instance()->getObject( search_regex );

    // Convert to plain text or \s won't get newlines
    int selection_start = textCursor().selectionStart();
    int selection_end = textCursor().selectionEnd();
    QString selected_text = Utility::Substring(selection_start, selection_end - selection_start, toPlainText() );

    SPCRE::MatchInfo match_info;

    // Check if current selection is a match as well to handle highlighted text that is a
    // match, new files when replacing in all HTML, and misspelled words
    if ( !( m_lastMatch.offset.first == selection_start && m_lastMatch.offset.second == selection_start + selected_text.length() ) )
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
                                const QString &replacement )
{
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
    // When a right-click occurs, move the caret location if this is performed.
    // outside the currently selected text.
    if (event->button() == Qt::RightButton) {
        QTextCursor cursor = cursorForPosition(event->pos());
        if (cursor.position() < textCursor().selectionStart() || cursor.position() > textCursor().selectionEnd()) {
            setTextCursor(cursor);
        }
    }

    // Propagate to base class
    QPlainTextEdit::mousePressEvent( event );   

    // Allow open link with Ctrl-mouseclick - after propagation sets cursor position
    bool isCtrl = QApplication::keyboardModifiers() & Qt::ControlModifier;
    if (isCtrl) {
        OpenLink();
    }
}


// Overridden so we can block the focus out signal for Windows.
// Right clicking and calling the context menu will cause the
// editor to loose focus. When it looses focus the code is checked
// if it is well formed. If it is not a message box is shown asking
// if the user would like to auto correct. This causes the context
// menu to disappear and thus be inaccessible to the user.
void CodeViewEditor::contextMenuEvent( QContextMenuEvent *event )
{
    // We block signals while the menu is executed because we don't want the LostFocus/GainedFocus
    // events to be triggered which will cause the selection to be changed
    blockSignals( true );

    QMenu *menu = createStandardContextMenu();

    bool offered_spelling = false;
    if (m_checkSpelling) {
        offered_spelling = AddSpellCheckContextMenu(menu);
    }

    if (!offered_spelling) {
        AddGoToStyleContextMenu(menu);
        AddClipContextMenu(menu);
    }

    menu->exec(event->globalPos());
    delete menu;

    blockSignals(false);

    // Now that we are no longer blocking signals we can execute any pending signal emits
    if (m_pendingClipEntryRequest) {
        emit OpenClipEditorRequest(m_pendingClipEntryRequest);
        m_pendingClipEntryRequest = NULL;
    }
    if (m_pendingGoToStyleDefinitionRequest) {
        m_pendingGoToStyleDefinitionRequest = false;
        GoToStyleDefinition();
    }
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

void CodeViewEditor::AddGoToStyleContextMenu(QMenu *menu)
{
    QAction *topAction = 0;
    if (!menu->actions().isEmpty()) {
        topAction = menu->actions().at(0);
    }

    QAction *goToStyleDefinitionAction = new QAction(tr("Go To Style Definition"), menu);
    if (!topAction) {
        menu->addAction(goToStyleDefinitionAction);
    }
    else {
        menu->insertAction(topAction, goToStyleDefinitionAction);
    }
    connect(goToStyleDefinitionAction, SIGNAL(triggered()), this, SLOT(GoToStyleDefinitionAction()));
    goToStyleDefinitionAction->setEnabled( IsGoToStyleDefinitionAllowed() );

    if (topAction) {
        menu->insertSeparator(topAction);
    }
}

void CodeViewEditor::AddClipContextMenu(QMenu *menu)
{
    QAction *topAction = 0;
    if (!menu->actions().isEmpty()) {
        topAction = menu->actions().at(0);
    }

    if (CreateMenuEntries(menu, topAction, ClipEditorModel::instance()->invisibleRootItem())) {
        if (topAction) {
            menu->insertSeparator(topAction);
        }
        else {
            menu->addSeparator();
        }
    }

    QAction *saveClipAction = new QAction(tr("Add To Clip Editor"), menu);
    if (!topAction) {
        menu->addAction(saveClipAction);
    }
    else {
        menu->insertAction(topAction, saveClipAction);
    }
    connect(saveClipAction, SIGNAL(triggered()), this , SLOT(SaveClipAction()));
    saveClipAction->setEnabled(textCursor().hasSelection());

    if (topAction) {
        menu->insertSeparator(topAction);
    }
}

bool CodeViewEditor::CreateMenuEntries(QMenu *parent_menu, QAction *topAction, QStandardItem *item)
{
    QAction *clipAction = 0;
    QMenu *group_menu = parent_menu;

    if (!item) {
        return false;
    }

    if (!item->text().isEmpty()) {
        // If item has no children, add entry to the menu, else create menu
        if (!item->data().toBool()) {
            clipAction = new QAction(item->text(), this);
            connect(clipAction, SIGNAL(triggered()), m_clipMapper, SLOT(map()));
            m_clipMapper->setMapping(clipAction, ClipEditorModel::instance()->GetFullName(item));
            if (!topAction) {
                parent_menu->addAction(clipAction);
            }
            else {
                parent_menu->insertAction(topAction, clipAction);
            }
        }
        else {
            group_menu = new QMenu(this);
            group_menu->setTitle(item->text());

            if (topAction) {
                parent_menu->insertMenu(topAction, group_menu);
            }
            else {
                parent_menu->addMenu(group_menu);
            }
            topAction = 0;
        }
    }

    // Recursively add entries for children
    for (int row = 0; row < item->rowCount(); row++) {
        CreateMenuEntries(group_menu, topAction, item->child(row,0));
    }
    return item->rowCount() > 0;
}

void CodeViewEditor::SaveClipAction()
{
    m_pendingClipEntryRequest = new ClipEditorModel::clipEntry();
    m_pendingClipEntryRequest->name = "Unnamed Entry";
    m_pendingClipEntryRequest->is_group = false;

    QTextCursor cursor = textCursor();
    m_pendingClipEntryRequest->text = cursor.selectedText();
}

void CodeViewEditor::GoToStyleDefinitionAction()
{
    // We don't do anything at this point - wait for the context menu
    // to be closed and signals are no longer blocked.
    m_pendingGoToStyleDefinitionRequest = true;
}

void CodeViewEditor::GoToStyleDefinition()
{
    // Begin by identifying the tag name and selected class style attribute if any
    CodeViewEditor::StyleTagElement element = GetSelectedStyleTagElement();
    if ( element.name.isEmpty() )
        return;

    // Emit a signal to bookmark our code location, enabling the "Back to" feature
    emit BookmarkStyleUsageLocationRequest(GetCursorPosition());
    
    // Look to see whether we have an inline style matching this.
    const QString text = toPlainText();

    // First look to see whether there is a <style type="text/css"> block
    QRegExp inline_styles_search("<\\s*style\\s[^>]+>", Qt::CaseInsensitive);
    inline_styles_search.setMinimal(true);
    int start = inline_styles_search.indexIn(text);
    if ( start > 0 ) 
    {
        start += inline_styles_search.matchedLength() - 1;
        int end = text.indexOf(QRegExp("<\\s*/\\s*style\\s*>"));
        if ( end >= 0 )
        {
            // First look for "element.class {"
            // Then look for just ".class {"
            // Finally look for "element {"
            if ( !element.classStyle.isEmpty() ) {
                if ( ScrollToInlineStyleDefinition( element.name + "\\." + element.classStyle, text, start, end ) ) { 
                    return;
                }
                if ( ScrollToInlineStyleDefinition( "\\." + element.classStyle, text, start, end ) ) { 
                    return;
                }
            }
            else if ( ScrollToInlineStyleDefinition( element.name, text, start, end ) ) { 
                return;
            }
        }
    }

    // We didn't find the style - escalate as an event to look in linked stylesheets
    emit GoToLinkedStyleDefinitionRequest( element.name, element.classStyle );
}

bool CodeViewEditor::ScrollToInlineStyleDefinition( const QString &style_name, const QString &text, const int &start_pos, const int &end_pos )
{
    QRegExp inline_style_search("[>\\};\\s]" + style_name + "\\s*\\{");

    int inline_index = inline_style_search.indexIn(text, start_pos);
    if ( inline_index > 0 && inline_index < end_pos ) {
        QStringList lines = text.left( inline_index + 1 ).split( QChar('\n') );
        ScrollToLine(lines.count());
        return true;
    }

    return false;
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

bool CodeViewEditor::IsOpenLinkAllowed()
{
    return !GetInternalLinkInTag().isEmpty();
}

void CodeViewEditor::OpenLink()
{
    QUrl url = GetInternalLinkInTag();

    if (!url.isEmpty()) {
        emit LinkClicked(url);
    }

    return;
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

void CodeViewEditor::AddToIndex()
{
    if (!IsNotInTagTextSelected()) {
        return;
    }

    IndexEditorModel::indexEntry *index = new IndexEditorModel::indexEntry();

    QTextCursor cursor = textCursor();
    index->pattern = cursor.selectedText();

    emit OpenIndexEditorRequest(index);
}

bool CodeViewEditor::IsAddToIndexAllowed()
{
    return !IsNotInTagTextSelected();
}

bool CodeViewEditor::IsMarkForIndexAllowed()
{
    return !IsNotInTagTextSelected();
}

void CodeViewEditor::MarkForIndex()
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

void CodeViewEditor::PasteClipEntryFromName(QString name)
{
    ClipEditorModel::clipEntry *clip = ClipEditorModel::instance()->GetEntryFromName(name);
    PasteClipEntry(clip);
}

void CodeViewEditor::PasteClipEntries(QList<ClipEditorModel::clipEntry *> clips)
{
    foreach(ClipEditorModel::clipEntry *clip, clips) {
        PasteClipEntry(clip);
    }
}

void CodeViewEditor::PasteClipEntry(ClipEditorModel::clipEntry *clip)
{
    if (!clip || clip->text.isEmpty()) {
        return;
    }

    // Remove any existing tags before adding in clip (save them if Shift not depressed)
    bool isShift = QApplication::keyboardModifiers() & Qt::ControlModifier;
    if (!isShift) {
        CutCodeTags();
    }

    QTextCursor cursor = textCursor();
    QString selected_text = cursor.selectedText();

    if (selected_text.isEmpty()) {

        // Allow users to use the same entry for insert/replace
        // Will not handle complicated regex, but good for tags like <p>\0</p>
        QString replacement_text = clip->text;
        replacement_text.remove(QString("\\0"));

        cursor.beginEditBlock();
        cursor.removeSelectedText();
        cursor.insertText(replacement_text);
        cursor.endEditBlock();
        setTextCursor( cursor );
    }
    else {
        QString search_regex = "(?s).*";
        ReplaceSelected(search_regex, clip->text, Searchable::Direction_Down );
    }
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


void CodeViewEditor::FormatBlock( const QString &element_name, bool preserve_attributes )
{
    if ( element_name.isEmpty() ) {
        return;
    }

    // Emit a selection changed event, so we can make sure the style buttons are updated
    // to uncheck any heading buttons check states.
    emit selectionChanged();

    // Going to assume that the user is allowed to click anywhere within or just after the block 
    // Also makes assumptions about being well formed, or else crazy things may happen...
    int pos = textCursor().selectionStart();
    QString text = toPlainText();

    if ( !IsPositionInBody(pos, text) ) {
        // User is outside the body so not allowed to change or insert a block tag
        return;
    }

    QString tag_name;
    QString opening_tag_text;
    QString opening_tag_attributes;
    QString closing_tag_text;
    int opening_tag_start = -1;
    int opening_tag_end = -1;
    int closing_tag_start = -1;
    int closing_tag_end = -1;

    QRegExp tag_search( NEXT_TAG_LOCATION );
    QRegExp tag_name_search( TAG_NAME_SEARCH );

    // Search backwards for the next block tag
    while (true) {
        int previous_tag_index = text.lastIndexOf( tag_search, pos );
        if (previous_tag_index < 0) {
            return;
        }
        // We found a tag. It could be opening or closing.
        int tag_name_index = tag_name_search.indexIn(text, previous_tag_index);
        if (tag_name_index < 0) {
            pos = previous_tag_index - 1;
            continue;
        }
        tag_name = tag_name_search.cap(1).toLower();

        // Isolate whether it was opening or closing tag.
        bool is_closing_tag = false;
        if ( tag_name.startsWith('/') ) {
            is_closing_tag = true;
            tag_name = tag_name.right(tag_name.length() - 1);
        }

        // Is this a block level tag? If not, keep searching...
        if ( !BLOCK_LEVEL_TAGS.contains( tag_name) ) {
            pos = previous_tag_index - 1;
            continue;
        }

        // Special case for the body tag or a previous closing block tag
        // In these situations we just insert html around our selection.
        if ( tag_name == "body" || is_closing_tag ) {
            InsertHTMLTagAroundSelection( element_name, "/" + element_name );
            return;
        }

        // If we got to here we know we have an opening block tag we shall replace.
        opening_tag_start = previous_tag_index;
        // Grab any attributes applied to this opening tag
        int attribute_start_index = tag_name_index + tag_name_search.matchedLength();
        opening_tag_end = text.indexOf('>', attribute_start_index) + 1;
        opening_tag_attributes = text.mid(attribute_start_index, opening_tag_end - attribute_start_index - 1).trimmed();

        // Now find the closing tag for this block.
        QRegExp closing_tag_search("</\\s*" + tag_name + "\\s*>", Qt::CaseInsensitive);
        closing_tag_start = text.indexOf(closing_tag_search, opening_tag_end);
        if (closing_tag_start < 0) {
            // Could not find a closing tag for this block name. Invalid HTML.
            return;
        }
        closing_tag_end = closing_tag_start + closing_tag_search.matchedLength();
        // Success
        break;
    }

    if ( preserve_attributes && opening_tag_attributes.length() > 0 ) {
        opening_tag_text = "<" + element_name + " " + opening_tag_attributes + ">";
    }
    else {
        opening_tag_text = "<" + element_name + ">";
    }
    closing_tag_text = "</" + element_name + ">";

    ReplaceTags( opening_tag_start, opening_tag_end, opening_tag_text,
                 closing_tag_start, closing_tag_end, closing_tag_text );
}

void CodeViewEditor::InsertHTMLTagAroundSelection( const QString &left_element_name, const QString &right_element_name )
{
    QTextCursor cursor = textCursor();

    const QString selected_text = cursor.selectedText();
    const QString replacement_text = "<" + left_element_name + ">" + selected_text + "<" + right_element_name + ">";
    int selection_start = cursor.selectionStart() + left_element_name.length() + 2;

    cursor.beginEditBlock();

    cursor.removeSelectedText();
    cursor.insertText( replacement_text );

    cursor.endEditBlock();
    cursor.setPosition( selection_start + selected_text.length() );
	cursor.setPosition( selection_start, QTextCursor::KeepAnchor );

    setTextCursor(cursor);
}

bool CodeViewEditor::IsPositionInBody( const int &pos, const QString &text )
{
    QRegExp body_search( BODY_START, Qt::CaseInsensitive );
    QRegExp body_end_search( BODY_END, Qt::CaseInsensitive );

    int body_tag_start = text.indexOf( body_search );
    int body_tag_end   = body_tag_start + body_search.matchedLength();
    int body_contents_end = text.indexOf( body_end_search );

    if ( (pos < body_tag_end) || (pos > body_contents_end) ) {
        return false;
    }
    return true;
}

bool CodeViewEditor::IsPositionInTag( const int &pos, QString &text)
{
    QRegExp tag_search( NEXT_TAG_LOCATION );

    int tag_start = text.lastIndexOf( tag_search, pos - 1 );
    int tag_end   = tag_start + tag_search.matchedLength();

    if ( ( pos >= tag_start ) && ( pos < tag_end ) ) {
        return true;
    }
    return false;
}

bool CodeViewEditor::IsPositionInOpeningTag( const int &pos, const QString &text)
{
    int search_pos = pos;
    if (pos == -1) {
        search_pos = textCursor().selectionStart();
    }
    QString search_text = text;
    if ( text.isEmpty() ) {
        search_text = toPlainText();
    }

    QRegExp tag_search( "<\\s*[^/][^>]*>" );
    int tag_start = search_text.lastIndexOf( tag_search, search_pos - 1 );
    int tag_end   = tag_start + tag_search.matchedLength();

    if ( ( search_pos >= tag_start ) && ( search_pos < tag_end ) ) {
        return true;
    }
    return false;
}

void CodeViewEditor::ToggleFormatSelection( const QString &element_name )
{
    if ( element_name.isEmpty() ) {
        return;
    }

    // Emit a selection changed event, so we can make sure the style buttons are updated
    // to uncheck any style buttons check states.
    emit selectionChanged();

    // Going to assume that the user is allowed to click anywhere within or just after the block 
    // Also makes assumptions about being well formed, or else crazy things may happen...
    int pos = textCursor().selectionStart();
    QString text = toPlainText();

    if ( !IsPositionInBody(pos, text) ) {
        // User is outside the body so not allowed to change or insert a style tag
        return;
    }
    // We might have a selection that begins or ends in a tag < > itself
    if ( IsPositionInTag( textCursor().selectionStart(), text ) || 
         IsPositionInTag( textCursor().selectionEnd(), text ) ) {
        // Not allowed to toggle style if caret placed on a tag
        return;
    }
    QString tag_name;
    QRegExp tag_search( NEXT_TAG_LOCATION );
    QRegExp tag_name_search( TAG_NAME_SEARCH );

    bool in_existing_tag_occurrence = false;
    int previous_tag_index = -1;
    pos--;

    // Look backwards from the current selection to find whether we are in an open occurrence
    // of this tag already within this block.
    while (true) {
        previous_tag_index = text.lastIndexOf( tag_search, pos );
        if (previous_tag_index < 0) {
            return;
        }
        // We found a tag. It could be opening or closing.
        int tag_name_index = tag_name_search.indexIn(text, previous_tag_index);
        if (tag_name_index < 0) {
            pos = previous_tag_index - 1;
            continue;
        }
        tag_name = tag_name_search.cap(1).toLower();

        // Isolate whether it was opening or closing tag.
        bool is_closing_tag = false;
        if ( tag_name.startsWith('/') ) {
            is_closing_tag = true;
            tag_name = tag_name.right(tag_name.length() - 1);
        }

        // Is this tag the element we are looking for?
        if ( element_name == tag_name ) {
            if ( !is_closing_tag ) {
                in_existing_tag_occurrence = true;
            }
            break;
        }
        else {
            // Is this a block level tag?
            if ( BLOCK_LEVEL_TAGS.contains( tag_name) ) {
                // No point in searching any further - we reached the block parent
                // without finding an open occurrence of this tag.
                break;
            }
            else {
                // Not a tag of interest - keep searching.
                pos = previous_tag_index - 1;
                continue;
            }
        }
    }

    if ( in_existing_tag_occurrence ) {
        FormatSelectionWithinElement( element_name, previous_tag_index, text );
    }
    else {
        // Otherwise assume we are in a safe place to add a wrapper tag.
        InsertHTMLTagAroundSelection( element_name, "/" + element_name );
    }
}

void CodeViewEditor::FormatSelectionWithinElement(const QString &element_name, const int &previous_tag_index, const QString &text)
{
    // We are inside an existing occurrence. Are we immediately adjacent to it?
    // e.g. "<b>selected text</b>" should result in "selected text" losing the tags.
    // but  "<b>XXXselected textYYY</b> should result in "<b>XXX</b>selected text<b>YYY</b>"
    // plus the variations where XXX or YYY may be non-existent to make tag adjacent.

    int previous_tag_end_index = text.indexOf(">", previous_tag_index);
    QRegExp closing_tag_search("</\\s*" + element_name + "\\s*>", Qt::CaseInsensitive);
    int closing_tag_index = text.indexOf(closing_tag_search, previous_tag_end_index);
    if ( closing_tag_index < 0 ) {
        // There is no closing tag for this style (not well formed). Give up.
        return;
    }

    QTextCursor cursor = textCursor();
    int selection_start = cursor.selectionStart();
    int selection_end = cursor.selectionEnd();

    bool adjacent_to_start = (previous_tag_end_index + 1) == selection_start;
    bool adjacent_to_end = closing_tag_index == selection_end;

    if ( !adjacent_to_start && !adjacent_to_end ) {
        // We want to put a closing tag at the start and an opening tag after (copying attributes)
        QString opening_tag_text = text.mid(previous_tag_index + 1, previous_tag_end_index - previous_tag_index - 1).trimmed();
        InsertHTMLTagAroundSelection( "/" + element_name, opening_tag_text );
    }
    else if ( ( selection_end == selection_start) && ( adjacent_to_start || adjacent_to_end ) ) {
        // User is just inside the opening or closing tag with no selection. Nothing to do
        return;
    }
    else {
        QString opening_tag = text.mid( previous_tag_index, previous_tag_end_index - previous_tag_index + 1 );
        QString closing_tag = text.mid( closing_tag_index, closing_tag_search.matchedLength() );

        QString replacement_text = cursor.selectedText();
        cursor.beginEditBlock();
        int new_selection_end = selection_end;
        int new_selection_start = selection_start;

        if ( adjacent_to_start ) {
            selection_start -= opening_tag.length();
            new_selection_start -= opening_tag.length();
            new_selection_end -= opening_tag.length();
        }
        else {
            replacement_text = closing_tag + replacement_text;
            new_selection_start += closing_tag.length();
            new_selection_end += closing_tag.length();
        }
        if ( adjacent_to_end ) {
            selection_end += closing_tag.length();
        }
        else {
            replacement_text.append( opening_tag );
        }

        cursor.setPosition( selection_end );
	    cursor.setPosition( selection_start, QTextCursor::KeepAnchor );
        cursor.removeSelectedText();
        cursor.insertText( replacement_text );

        cursor.setPosition( new_selection_end );
	    cursor.setPosition( new_selection_start, QTextCursor::KeepAnchor );

        cursor.endEditBlock();
        setTextCursor(cursor);
    }
}

void CodeViewEditor::ReplaceTags( const int &opening_tag_start, const int &opening_tag_end, const QString &opening_tag_text,
                                  const int &closing_tag_start, const int &closing_tag_end, const QString &closing_tag_text )
{
    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();

    // Replace the end block tag first since that does not affect positions
    cursor.setPosition( closing_tag_end );
    cursor.setPosition( closing_tag_start, QTextCursor::KeepAnchor );
    cursor.removeSelectedText();
    cursor.insertText( closing_tag_text );

    // Now replace the opening block tag
    cursor.setPosition( opening_tag_end );
    cursor.setPosition( opening_tag_start, QTextCursor::KeepAnchor );
    cursor.removeSelectedText();
    cursor.insertText( opening_tag_text );

    cursor.endEditBlock();

    setTextCursor(cursor);
}

CodeViewEditor::StyleTagElement CodeViewEditor::GetSelectedStyleTagElement()
{
    // Look at the current cursor position, and return a struct representing the 
    // name of the element, and the style class name if any under the caret.
    // If caret not on a style name, returns the first style class name.
    // If no style class specified, only the element tag name will be populated.
    CodeViewEditor::StyleTagElement element;
    QString text = toPlainText();
    int pos = textCursor().selectionStart() - 1;

    QRegExp tag_name_search(TAG_NAME_SEARCH);

    int tag_name_index = tag_name_search.lastIndexIn(text, pos);
    if ( tag_name_index >= 0 ) 
    {
        element.name = tag_name_search.cap(1);

        int closing_tag_index = text.indexOf(QChar('>'), tag_name_index + tag_name_search.matchedLength());
        text = text.left(closing_tag_index);
        QRegExp style_names_search("\\s+class\\s*=\\s*\"([^\"]+)\"");
        int style_names_index = style_names_search.indexIn( text, tag_name_index + tag_name_search.matchedLength() );
        if ( style_names_index >= 0 )
        {
            // We have one or more styles. Figure out which one the cursor is in if any.
            QString style_names_text = style_names_search.cap(1);
            int styles_end_index = style_names_index + style_names_search.matchedLength() - 1;
            int styles_start_index = text.indexOf( style_names_text, style_names_index );
            if ( ( pos >= styles_start_index ) && ( pos < styles_end_index ) ) {
                // Get the name under the cursor.
                QString style_name = QString();
                while (pos > 0 && text[pos] != QChar('"') && text[pos] != QChar(' ')) {
                    pos--;
                }
                pos++;
                while (text[pos] != QChar('"') && text[pos] != QChar(' ')) {
                    style_name.append(text[pos]);
                    pos++;
                }
                element.classStyle = style_name;
            }
            if ( element.classStyle.isEmpty() ) {
                // User has clicked outside of the style class names or somewhere strange - default to the first name.
                QStringList style_names = style_names_text.trimmed().split(' ');
                element.classStyle = style_names[0];
            }
        }
    }

    return element;
}

void CodeViewEditor::PasteFromClipboard()
{
    paste();
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

    connect(m_clipMapper, SIGNAL(mapped(const QString&)), this, SLOT(PasteClipEntryFromName(const QString&)));
}
