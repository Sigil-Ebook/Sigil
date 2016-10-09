/************************************************************************
**
**  Copyright (C) 2015 Kevin B. Hendricks Stratford, ON Canada
**  Copyright (C) 2012 John Schember <john@nachtimwald.com>
**  Copyright (C) 2012, 2013 Dave Heiland
**  Copyright (C) 2012 Grant Drake
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

#include <memory>

#include <QtCore/QFileInfo>
#include <QtGui/QContextMenuEvent>
#include <QtCore/QSignalMapper>
#include <QtWidgets/QAction>
#include <QtWidgets/QMenu>
#include <QtGui/QPainter>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QShortcut>
#include <QtCore/QXmlStreamReader>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>

#include "BookManipulation/Book.h"
#include "BookManipulation/CleanSource.h"
#include "BookManipulation/XhtmlDoc.h"
#include "MainUI/MainWindow.h"
#include "Misc/GumboInterface.h"
#include "Misc/XHTMLHighlighter.h"
#include "Dialogs/ClipEditor.h"
#include "Misc/CSSHighlighter.h"
#include "Misc/Language.h"
#include "Misc/SettingsStore.h"
#include "Misc/SpellCheck.h"
#include "Misc/HTMLSpellCheck.h"
#include "Misc/QuickSerialHtmlParser.h"
#include "Misc/Utility.h"
#include "PCRE/PCRECache.h"
#include "ViewEditors/CodeViewEditor.h"
#include "ViewEditors/LineNumberArea.h"
#include "sigil_constants.h"

static const int TAB_SPACES_WIDTH        = 4;
static const int LINE_NUMBER_MARGIN      = 5;

static const QString XML_OPENING_TAG        = "(<[^>/][^>]*[^>/]>|<[^>/]>)";
static const QString NEXT_CLOSE_TAG_LOCATION = "</\\s*[^>]+>";
static const QString NEXT_TAG_LOCATION      = "<[^!>]+>";
static const QString TAG_NAME_SEARCH        = "<\\s*([^\\s>]+)";
static const QString STYLE_ATTRIBUTE_SEARCH = "style\\s*=\\s*\"[^\"]*\"";
static const QString ATTRIBUTE_NAME_POSTFIX_SEARCH = "\\s*=\\s*\"[^\"]*\"";
static const QString ATTRIB_VALUES_SEARCH   = "\"([^\"]*)";

static const int MAX_SPELLING_SUGGESTIONS = 10;


CodeViewEditor::CodeViewEditor(HighlighterType high_type, bool check_spelling, QWidget *parent)
    :
    QPlainTextEdit(parent),
    m_isUndoAvailable(false),
    m_LastBlockCount(0),
    m_LineNumberAreaBlockNumber(-1),
    m_LineNumberArea(new LineNumberArea(this)),
    m_ScrollOneLineUp(new QShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_Up), this, 0, 0, Qt::WidgetShortcut)),
    m_ScrollOneLineDown(new QShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_Down), this, 0, 0, Qt::WidgetShortcut)),
    m_isLoadFinished(false),
    m_DelayedCursorScreenCenteringRequired(false),
    m_CaretUpdate(QList<ViewEditor::ElementIndex>()),
    m_checkSpelling(check_spelling),
    m_reformatCSSEnabled(false),
    m_reformatHTMLEnabled(false),
    m_lastFindRegex(QString()),
    m_spellingMapper(new QSignalMapper(this)),
    m_addSpellingMapper(new QSignalMapper(this)),
    m_addDictMapper(new QSignalMapper(this)),
    m_ignoreSpellingMapper(new QSignalMapper(this)),
    m_clipMapper(new QSignalMapper(this)),
    m_MarkedTextStart(-1),
    m_MarkedTextEnd(-1),
    m_ReplacingInMarkedText(false),
    m_QSHParser(nullptr),
    //DEBUG
    m_DEBUG(false)
{
    if (high_type == CodeViewEditor::Highlight_XHTML) {
        m_Highlighter = new XHTMLHighlighter(check_spelling, this);
        //need this to build language map
        //cannot use the XHTMLHighlighter instance, alas
        //so we parse text double - waiting for better idea
        m_QSHParser = new QuickSerialHtmlParser();
    } else if (high_type == CodeViewEditor::Highlight_CSS) {
        m_Highlighter = new CSSHighlighter(this);
    } else {
        m_Highlighter = NULL;
    }

    setFocusPolicy(Qt::StrongFocus);
    ConnectSignalsToSlots();
    SettingsStore settings;
    m_codeViewAppearance = settings.codeViewAppearance();
    SetAppearanceColors();
    UpdateLineNumberAreaMargin();
    HighlightCurrentLine();
    setFrameStyle(QFrame::NoFrame);
    // Set the Zoom factor but be sure no signals are set because of this.
    m_CurrentZoomFactor = settings.zoomText();
    Zoom();   
}

CodeViewEditor::~CodeViewEditor()
{
    m_ScrollOneLineUp->deleteLater();
    m_ScrollOneLineDown->deleteLater();
    if(m_QSHParser) {
        delete m_QSHParser;
        m_QSHParser=nullptr;
    }
}

QSize CodeViewEditor::sizeHint() const
{
    return QSize(16777215, 16777215);
}


void CodeViewEditor::CustomSetDocument(QTextDocument &document)
{
    setDocument(&document);
    document.setModified(false);

    if (m_Highlighter) {
        m_Highlighter->setDocument(&document);
        // The QSyntaxHighlighter will setup a singleShot timer to do the highlighting
        // in response to setDocument being called. This causes a problem because we
        // cannot control at what point it finishes, and the textChanged signal of the
        // QTextDocument gets fired by the syntax highlighting. This in turn causes issues
        // with our own logic trying to do stuff in response to genunine document changes.
        // So we will synchronously highlight now, and block signals while doing so.
        RehighlightDocument();
    }

    ResetFont();
    m_isLoadFinished = true;
    updateLangMap();
    emit DocumentSet();
}

void CodeViewEditor::DeleteLine()
{
    if (document()->isEmpty()) {
        return;
    }

    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();
    cursor.select(QTextCursor::LineUnderCursor);
    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
    cursor.removeSelectedText();
    cursor.endEditBlock();
    emit selectionChanged();
}

bool CodeViewEditor::MarkSelection()
{
    if (textCursor().hasSelection()) {
        m_MarkedTextStart = textCursor().selectionStart();
        m_MarkedTextEnd = textCursor().selectionEnd();
        HighlightCurrentLine();
        return true;
    }
    ClearMarkedText();
    return false;
}

bool CodeViewEditor::ClearMarkedText()
{
    bool marked = IsMarkedText();
    m_MarkedTextStart = -1;
    m_MarkedTextEnd = -1;
    HighlightCurrentLine();
    return marked;
}

void CodeViewEditor::HighlightMarkedText()
{
    QTextCharFormat format;

    QList<QTextEdit::ExtraSelection> extraSelections;
    QTextEdit::ExtraSelection selection;
    selection.cursor = textCursor();

    selection.format.setFontUnderline(QTextCharFormat::DotLine);
    selection.cursor.clearSelection();
    selection.cursor.setPosition(0);
    selection.cursor.setPosition(toPlainText().length());
    extraSelections.append(selection);
    setExtraSelections(extraSelections);
    extraSelections.clear();

    if (m_MarkedTextStart < 0 || m_MarkedTextEnd > toPlainText().length()) {
        return;
    }
    selection.format.setFontUnderline(QTextCharFormat::DotLine);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();
    selection.cursor.setPosition(m_MarkedTextStart);
    selection.cursor.setPosition(m_MarkedTextEnd, QTextCursor::KeepAnchor);
    extraSelections.append(selection);
    setExtraSelections(extraSelections);
}

bool CodeViewEditor::IsMarkedText()
{
    return m_MarkedTextStart >= 0 && m_MarkedTextEnd > 0;
}

bool CodeViewEditor::MoveToMarkedText(Searchable::Direction direction, bool wrap)
{
    if (!IsMarkedText()) {
        return false;
    }
    int pos = textCursor().position();
    if (pos >= m_MarkedTextStart && pos <= m_MarkedTextEnd) {
        return true;
    }

    bool moved = false;

    if (direction == Searchable::Direction_Up) {
        if (wrap || pos > m_MarkedTextEnd) {
            pos = m_MarkedTextEnd;
            moved = true;
        }
    } else {
        if (wrap || pos < m_MarkedTextStart) {
            pos = m_MarkedTextStart;
            moved = true;
        }
    }

    if (moved) {
        QTextCursor cursor = textCursor();
        cursor.setPosition(pos);
        setTextCursor(cursor);
    }
    return moved;
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

bool CodeViewEditor::TextIsSelected()
{
    return textCursor().hasSelection();
}

bool CodeViewEditor::TextIsSelectedAndNotInStartOrEndTag()
{
    if (!textCursor().hasSelection()) {
        return false;
    }

    if (IsPositionInTag(textCursor().selectionStart()) || IsPositionInTag(textCursor().selectionEnd())) {
        return false;
    }

    return true;
}

bool CodeViewEditor::IsCutCodeTagsAllowed()
{
    return TextIsSelectedAndNotInStartOrEndTag();
}

bool CodeViewEditor::IsInsertClosingTagAllowed()
{
    return !IsPositionInTag();
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

QString CodeViewEditor::SplitSection()
{
    QString text = toPlainText();
    int split_position = textCursor().position();

    // Abort splitting the section if user is within a tag - MainWindow will display a status message
    if (IsPositionInTag(split_position, text)) {
        return QString();
    }

    QRegularExpression body_search(BODY_START, QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch body_search_mo = body_search.match(text);
    int body_tag_start = body_search_mo.capturedStart();
    int body_tag_end   = body_tag_start + body_search_mo.capturedLength();
    QRegularExpression body_end_search(BODY_END, QRegularExpression::CaseInsensitiveOption);
    int body_contents_end = text.indexOf(body_end_search);
    QString head = text.left(body_tag_start);

    if (split_position < body_tag_end) {
        // Cursor is before the start of the body
        split_position = body_tag_end;
    } else {
        int next_close_tag_index = text.indexOf(QRegularExpression(NEXT_CLOSE_TAG_LOCATION), split_position);

        if (next_close_tag_index == -1) {
            // Cursor is at end of file
            split_position = body_contents_end;
        }
    }

    const QString &text_segment = split_position != body_tag_end
                                  ? Utility::Substring(body_tag_start, split_position, text)
                                  : QString("<p>&#160;</p>");
    // Remove the text that will be in the new section from the View.
    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();
    cursor.setPosition(body_tag_end);
    cursor.setPosition(split_position, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();

    // We add a newline if the next tag is sitting right next to the end of the body tag.
    if (toPlainText().at(body_tag_end) == QChar('<')) {
        cursor.insertBlock();
    }

    // There is a scenario which can cause disaster - if our split point is next to
    // a <br/> tag then parsing may discard all content after the end of the current block.
    // e.g. If split point is: <br/></p><p>More text</p> then anything after first </p> is lost
    //      from within AnchorUpdates.cpp when it replaces the resource text by re-serializing.
    // An additional problem occurs if the user splits inside the last block in the body,
    // as previous logic would always give you a blank new page without the split off text.
    // So instead we will identify any open tags for the current caret position, and repeat
    // those at the caret position to ensure we have valid html 
    const QStringList &opening_tags = GetUnmatchedTagsForBlock(split_position, text);

    if (!opening_tags.isEmpty()) {
        cursor.insertText(opening_tags.join(""));
    }

    cursor.endEditBlock();

    cursor.beginEditBlock();

    // Prevent current file from having an empty body which
    // causes Book View to insert text outside the body.
    text = toPlainText();
    QRegularExpression empty_body_search("<body>\\s</body>");
    QRegularExpressionMatch empty_body_search_mo = empty_body_search.match(text);
    int empty_body_tag_start = empty_body_search_mo.capturedStart();
    if (empty_body_tag_start != - 1) {
        int empty_body_tag_end = empty_body_tag_start + QString("<body>").length();
        cursor.setPosition(empty_body_tag_end);
        cursor.insertText("\n  <p>&#160;</p>");
    };

    cursor.endEditBlock();

    return QString()
           .append(head)
           .append(text_segment)
           .append("\n</body>\n</html>");
}


void CodeViewEditor::InsertSGFSectionMarker()
{
    textCursor().insertText(BREAK_TAG_INSERT);
}


void CodeViewEditor::InsertClosingTag()
{
    if (!IsInsertClosingTagAllowed()) {
        emit ShowStatusMessageRequest(tr("Cannot insert closing tag at this position."));
        return;
    }

    int pos = textCursor().position() - 1;
    QString text = toPlainText();
    const QStringList unmatched_tags = GetUnmatchedTagsForBlock(pos, text);

    if (unmatched_tags.isEmpty()) {
        emit ShowStatusMessageRequest(tr("No open tags found at this position."));
        return;
    }

    QString tag = unmatched_tags.last();
    QRegularExpression tag_name_search(TAG_NAME_SEARCH);
    QRegularExpressionMatch mo = tag_name_search.match(tag);
    int tag_name_index = mo.capturedStart();

    if (tag_name_index >= 0) {
        const QString closing_tag = "</" %  mo.captured(1) % ">";
        textCursor().insertText(closing_tag);
    }
}


void CodeViewEditor::LineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(m_LineNumberArea);
    // Paint the background first
    painter.fillRect(event->rect(), m_codeViewAppearance.line_number_background_color);
    // A "block" represents a line of text
    QTextBlock block = firstVisibleBlock();
    // Blocks are numbered from zero,
    // but we count lines of text from one
    int blockNumber  = block.blockNumber() + 1;

    // We loop through all the visible and
    // unobscured blocks and paint line numbers for each
    while (block.isValid()) {
        // Getting the Y coordinates for the top of a block.
        int topY = (int) blockBoundingGeometry(block).translated(contentOffset()).top();

        // Ignore blocks that are not visible.
        if (!block.isVisible() || (topY > event->rect().bottom())) {
            break;
        }

        // Draw the number in the line number area.
        painter.setPen(m_codeViewAppearance.line_number_foreground_color);
        QString number_to_paint = QString::number(blockNumber);
        painter.drawText(- LINE_NUMBER_MARGIN,
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


void CodeViewEditor::LineNumberAreaMouseEvent(QMouseEvent *event)
{
    QTextCursor cursor = cursorForPosition(QPoint(0, event->pos().y()));

    if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick) {
        if (event->button() == Qt::LeftButton) {
            QTextCursor selection = cursor;
            selection.setVisualNavigation(true);
            m_LineNumberAreaBlockNumber = selection.blockNumber();
            selection.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
            selection.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
            setTextCursor(selection);
        }
    } else if (m_LineNumberAreaBlockNumber >= 0) {
        QTextCursor selection = cursor;
        selection.setVisualNavigation(true);

        if (event->type() == QEvent::MouseMove) {
            QTextBlock anchorBlock = document()->findBlockByNumber(m_LineNumberAreaBlockNumber);
            selection.setPosition(anchorBlock.position());

            if (cursor.blockNumber() < m_LineNumberAreaBlockNumber) {
                selection.movePosition(QTextCursor::EndOfBlock);
                selection.movePosition(QTextCursor::Right);
            }

            selection.setPosition(cursor.block().position(), QTextCursor::KeepAnchor);

            if (cursor.blockNumber() >= m_LineNumberAreaBlockNumber) {
                selection.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
                selection.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
            }
        } else {
            m_LineNumberAreaBlockNumber = -1;
            return;
        }

        setTextCursor(selection);
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
    while (last_line_number >= 10) {
        last_line_number /= 10;
        num_digits++;
    }

    return LINE_NUMBER_MARGIN * 2 + fontMetrics().width(QChar('0')) * num_digits;
}


void CodeViewEditor::ReplaceDocumentText(const QString &new_text)
{
    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();
    cursor.select(QTextCursor::Document);
    cursor.removeSelectedText();
    cursor.insertText(new_text);
    cursor.endEditBlock();
}


void CodeViewEditor::ScrollToTop()
{
    verticalScrollBar()->setValue(0);
}

void CodeViewEditor::ScrollToPosition(int cursor_position, bool center_screen)
{
    if (cursor_position < 0) {
        return;
    }

    QTextCursor cursor(document());
    cursor.setPosition(cursor_position);
    setTextCursor(cursor);

    // If height is 0, then the widget is still collapsed
    // and centering the screen will do squat.
    if (center_screen) {
        if (height() > 0) {
            centerCursor();
        } else {
            m_DelayedCursorScreenCenteringRequired = true;
        }

        m_CaretUpdate.clear();
    }
}

void CodeViewEditor::ScrollToLine(int line)
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
    } else {
        m_DelayedCursorScreenCenteringRequired = true;
    }
}

void CodeViewEditor::ScrollToFragment(const QString &fragment)
{
    if (fragment.isEmpty()) {
        ScrollToLine(1);
        return;
    }

    QRegularExpression fragment_search("id=\"" % fragment % "\"");
    int index = toPlainText().indexOf(fragment_search);
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


void CodeViewEditor::SetZoomFactor(float factor)
{
    SettingsStore settings;
    settings.setZoomText(factor);
    m_CurrentZoomFactor = factor;
    Zoom();
    emit ZoomFactorChanged(factor);
}


float CodeViewEditor::GetZoomFactor() const
{
    SettingsStore settings;
    return settings.zoomText();
}


void CodeViewEditor::Zoom()
{
    QFont current_font = font();
    current_font.setPointSizeF(m_codeViewAppearance.font_size * m_CurrentZoomFactor);
    setFont(current_font);
    UpdateLineNumberAreaFont(current_font);
}


void CodeViewEditor::UpdateDisplay()
{
    SettingsStore settings;
    float stored_factor = settings.zoomText();

    if (stored_factor != m_CurrentZoomFactor) {
        m_CurrentZoomFactor = stored_factor;
        Zoom();
    }
}

SPCRE::MatchInfo CodeViewEditor::GetMisspelledWord(const QString &text, int start_offset, int end_offset, const QString &search_regex, Searchable::Direction search_direction)
{
    SPCRE::MatchInfo match_info;
    HTMLSpellCheck::MisspelledWord misspelled_word;

    if (search_direction == Searchable::Direction_Up) {
        if (end_offset > 0) {
            end_offset -= 1;
        }

        misspelled_word =  HTMLSpellCheck::GetLastMisspelledWord(text, start_offset, end_offset, search_regex);
    } else {
        misspelled_word =  HTMLSpellCheck::GetFirstMisspelledWord(text, start_offset, end_offset, search_regex);
    }

    if (!misspelled_word.text.isEmpty()) {
        match_info.offset.first = misspelled_word.offset - start_offset;
        match_info.offset.second = match_info.offset.first + misspelled_word.length;
    }

    return match_info;
}

bool CodeViewEditor::FindNext(const QString &search_regex,
                              Searchable::Direction search_direction,
                              bool misspelled_words,
                              bool ignore_selection_offset,
                              bool wrap,
                              bool marked_text)
{
    SPCRE *spcre = PCRECache::instance()->getObject(search_regex);
    SPCRE::MatchInfo match_info;
    int start_offset = 0;
    int start = 0;
    int end = toPlainText().length();
    if (marked_text) {
        if (!MoveToMarkedText(search_direction, wrap)) {
            return false;
        }
        start = m_MarkedTextStart;
        end = m_MarkedTextEnd;
        start_offset = m_MarkedTextStart;
    }
    int selection_offset = GetSelectionOffset(search_direction, ignore_selection_offset, marked_text);

    if (search_direction == Searchable::Direction_Up) {
        if (misspelled_words) {
            match_info = GetMisspelledWord(toPlainText(), 0, selection_offset, search_regex, search_direction);
        } else {
            match_info = spcre->getLastMatchInfo(Utility::Substring(start, selection_offset, toPlainText()));
        }
    } else {
        if (misspelled_words) {
            match_info = GetMisspelledWord(toPlainText(), selection_offset, toPlainText().count(), search_regex, search_direction);
        } else {
            match_info = spcre->getFirstMatchInfo(Utility::Substring(selection_offset, end, toPlainText()));
        }

        start_offset = selection_offset;
    }

    if (marked_text) {
        // If not in marked text it's not a real match.
        if (match_info.offset.second + start_offset > m_MarkedTextEnd ||
            match_info.offset.first + start_offset < m_MarkedTextStart) {
            match_info.offset.first = -1;
        }
    }

    if (match_info.offset.first != -1) {
        // We will scroll the position on screen in order to ensure the entire block is visible
        // and if not, then center the match.
        SelectAndScrollIntoView(match_info.offset.first + start_offset, match_info.offset.second + start_offset,
                                search_direction, ignore_selection_offset);
        // We store our offset after the selection changing event which would otherwise reset it
        m_lastFindRegex = search_regex;
        m_lastMatch = match_info;
        m_lastMatch.offset.first += start_offset;
        m_lastMatch.offset.second += start_offset;
        return true;
    } else if (wrap) {
        if (FindNext(search_regex, search_direction, misspelled_words, true, false, marked_text)) {
            ShowWrapIndicator(this);
            return true;
        }
    }

    return false;
}


int CodeViewEditor::Count(const QString &search_regex, Searchable::Direction direction, bool wrap, bool marked_text)
{
    SPCRE *spcre = PCRECache::instance()->getObject(search_regex);
    QString text;
    text = toPlainText();
    int start = 0;
    int end = text.length();

    if (marked_text) {
        if (!MoveToMarkedText(direction, wrap)) {
            return 0;
        }
        start = m_MarkedTextStart;
        end = m_MarkedTextEnd;
    }
    if (!wrap) {
        if (direction == Searchable::Direction_Up) {
            text = Utility::Substring(start, textCursor().position(), text);
        } else {
            text = Utility::Substring(textCursor().position(), end, text);
        }
    } else if (marked_text) {
        text = Utility::Substring(start, end, text);
    }
    return spcre->getEveryMatchInfo(text).count();
}


bool CodeViewEditor::ReplaceSelected(const QString &search_regex, const QString &replacement, Searchable::Direction direction, bool replace_current)
{
    SPCRE *spcre = PCRECache::instance()->getObject(search_regex);
    int selection_start = textCursor().selectionStart();
    int selection_end = textCursor().selectionEnd();

    // It is only safe to do a replace if we have not changed the selection or find text
    // since we last did a Find.
    if ((search_regex != m_lastFindRegex) || (m_lastMatch.offset.first == -1)) {
        return false;
    }

    // Convert to plain text or \s won't get newlines
    const QString &document_text = toPlainText();
    QString selected_text = Utility::Substring(selection_start, selection_end, document_text);
    QString replaced_text;
    bool replacement_made = false;
    bool in_marked_text = selection_start >= m_MarkedTextStart && selection_end <= m_MarkedTextEnd;
    if (in_marked_text) {
        m_ReplacingInMarkedText = true;
    }
    int original_text_length = toPlainText().length();
    replacement_made = spcre->replaceText(selected_text, m_lastMatch.capture_groups_offsets, replacement, replaced_text);

    if (replacement_made) {
        QTextCursor cursor = textCursor();
        int start = cursor.position();
        // Replace the selected text with our replacement text.
        cursor.beginEditBlock();
        cursor.removeSelectedText();
        cursor.insertText(replaced_text);
        cursor.clearSelection();
        cursor.endEditBlock();

        // Select the new text
        if (replace_current) {
            if (direction == Searchable::Direction_Up) {
                cursor.setPosition(start + replaced_text.length());
                cursor.setPosition(start, QTextCursor::KeepAnchor);
            } else {
                cursor.setPosition(selection_start);
                cursor.setPosition(selection_start + replaced_text.length(), QTextCursor::KeepAnchor);
            }
        } else if (direction == Searchable::Direction_Up) {
            // Find for next match done after will set selection
            cursor.setPosition(selection_start);
        }

        setTextCursor(cursor);

        // Adjust size of marked text.
        if (in_marked_text) {
            m_ReplacingInMarkedText = false;
            m_MarkedTextEnd += toPlainText().length() - original_text_length;
        }

        if (!hasFocus()) {
            // The replace operation is being performed where focus is elsewhere (like in the F&R combos)
            // If the user does not click back into the tab, these changes will not be saved yet, which
            // means if the switch to another tab (such as a BV tab after doing a F&R in CSS) they will
            // not see the result of those changes. So we will emit a FocusLost event, which will trigger
            // the saving of the tab content and all associated ResourceModified signals to fire.
            emit FocusLost(this);
        }
    }

    HighlightCurrentLine();
    return replacement_made;
}


int CodeViewEditor::ReplaceAll(const QString &search_regex,
                               const QString &replacement,
                               Searchable::Direction direction,
                               bool wrap,
                               bool marked_text)
{
    int count = 0;
    QString text = toPlainText();
    int original_position = textCursor().position();
    int position = original_position;
    if (marked_text) {
        m_ReplacingInMarkedText = true;
        if (!MoveToMarkedText(direction, wrap)) {
            return 0;
        }
        // Restrict replace to the marked area.
        text = Utility::Substring(m_MarkedTextStart, m_MarkedTextEnd, text);
        position = original_position - m_MarkedTextStart;
    }
    int marked_text_length = text.length();

    SPCRE *spcre = PCRECache::instance()->getObject(search_regex);
    QList<SPCRE::MatchInfo> match_info = spcre->getEveryMatchInfo(text);

    // Run though all match offsets making the replacement in reverse order.
    // This way changes in text length won't change the offsets as we make
    // our changes.
    for (int i = match_info.count() - 1; i >= 0; i--) {
        QString replaced_text;
        if (!wrap) {
            if (direction == Searchable::Direction_Up) {
                if (match_info.at(i).offset.first > position) {
                    break;
                }
            } else if (!wrap) {
                if (match_info.at(i).offset.second < position) {
                    break;
                }
            }
        }

        bool replacement_made = spcre->replaceText(Utility::Substring(match_info.at(i).offset.first, match_info.at(i).offset.second, text), match_info.at(i).capture_groups_offsets, replacement, replaced_text);

        if (replacement_made) {
            // Replace the text.
            text = text.replace(match_info.at(i).offset.first, match_info.at(i).offset.second - match_info.at(i).offset.first, replaced_text);
            count++;
        }
    }
    if (marked_text) {
        // Merge the replaced marked text into the original text and adjust the marker.
        QString replaced_text = toPlainText();
        replaced_text.replace(m_MarkedTextStart, m_MarkedTextEnd - m_MarkedTextStart, text);
        m_MarkedTextEnd += text.length() - marked_text_length;
        text = replaced_text;
    }

    QTextCursor cursor = textCursor();
    // Store the cursor position
    int cursor_position = cursor.selectionStart();
    cursor.beginEditBlock();
    // Replace all text in the document with the new text.
    cursor.setPosition(cursor_position);
    cursor.select(QTextCursor::Document);
    cursor.insertText(text);
    cursor.endEditBlock();

    // Restore the cursor position
    cursor.setPosition(cursor_position);
    setTextCursor(cursor);

    HighlightCurrentLine();

    if (marked_text) {
        m_ReplacingInMarkedText = false;
    }

    if (!hasFocus()) {
        // The replace operation is being performed where focus is elsewhere (like in the F&R combos)
        // If the user does not click back into the tab, these changes will not be saved yet, which
        // means if the switch to another tab (such as a BV tab after doing a F&R in CSS) they will
        // not see the result of those changes. So we will emit a FocusLost event, which will trigger
        // the saving of the tab content and all associated ResourceModified signals to fire.
        emit FocusLost(this);
    }

    return count;
}

void CodeViewEditor::ResetLastFindMatch()
{
    m_lastMatch.offset.first = -1;
}

QString CodeViewEditor::GetSelectedText()
{
    return textCursor().selectedText();
}

void CodeViewEditor::SetUpFindForSelectedText(const QString &search_regex)
{
    // When a user hits Ctrl+F to load up Find text for the selection, they will want to be
    // able to follow that up with a Replace. Since ReplaceSelected() requires the PCRE
    // lastMatchInfo to be setup, we must effectively do a Find for the selected text.
    // However this is complicated by the fact that the auto-tokenise and other F&R options
    // may mean that the search_regex when ReplaceSelected() is called can be different
    // to what regex would minimally match the selection. So instead we require this function
    // to be passed the regex that is derived from the selected text with current options.
    QTextCursor cursor = textCursor();
    const int selection_start = cursor.selectionStart();
    const int selection_end = cursor.selectionEnd();
    cursor.setPosition(selection_start);
    setTextCursor(cursor);
    bool found = FindNext(search_regex, Searchable::Direction_Down, false, false, false);

    if (!found) {
        // We have an edge case where the text selected is not a match for this regex text.
        cursor.setPosition(selection_end);
        cursor.setPosition(selection_start, QTextCursor::KeepAnchor);
        setTextCursor(cursor);
    }
}

// The base class implementation of the print()
// method is not a slot, and we need it as a slot
// for print preview support; so this is just
// a slot wrapper around that function
void CodeViewEditor::print(QPagedPaintDevice *printer)
{
    QPlainTextEdit::print(printer);
}

// Overridden because we need to update the cursor
// location if a cursor update (from BookView)
// is waiting to be processed
bool CodeViewEditor::event(QEvent *event)
{
    // We just return whatever the "real" event handler returns
    bool real_return = QPlainTextEdit::event(event);

    // Executing the caret update inside the paint event
    // handler causes artifacts on mac. So we do it after
    // the event is processed and accepted.
    if (event->type() == QEvent::Paint) {
        DelayedCursorScreenCentering();
    }

    return real_return;
}


// Overridden because after updating itself on a resize event,
// the editor needs to update its line number area too
void CodeViewEditor::resizeEvent(QResizeEvent *event)
{
    // Update self normally
    QPlainTextEdit::resizeEvent(event);
    QRect contents_area = contentsRect();
    // Now update the line number area
    m_LineNumberArea->setGeometry(QRect(contents_area.left(),
                                        contents_area.top(),
                                        CalculateLineNumberAreaWidth(),
                                        contents_area.height()
                                       )
                                 );
}


void CodeViewEditor::mousePressEvent(QMouseEvent *event)
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
    QPlainTextEdit::mousePressEvent(event);
    // Allow open link with Ctrl-mouseclick - after propagation sets cursor position
    bool isCtrl = QApplication::keyboardModifiers() & Qt::ControlModifier;

    if (isCtrl) {
        GoToLinkOrStyle();
    }
}

void CodeViewEditor::mouseReleaseEvent(QMouseEvent *event)
{
    emit PageClicked();
    QPlainTextEdit::mouseReleaseEvent(event);
}


// Overridden so we can block the focus out signal for Windows.
// Right clicking and calling the context menu will cause the
// editor to loose focus. When it looses focus the code is checked
// if it is well formed. If it is not a message box is shown asking
// if the user would like to auto correct. This causes the context
// menu to disappear and thus be inaccessible to the user.
void CodeViewEditor::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = createStandardContextMenu();

    if (m_reformatCSSEnabled) {
        AddReformatCSSContextMenu(menu);
    }

    if (m_reformatHTMLEnabled) {
        AddReformatHTMLContextMenu(menu);
    }

    AddMarkSelectionMenu(menu);
    AddGoToLinkOrStyleContextMenu(menu);
    AddClipContextMenu(menu);

    if (m_checkSpelling) {
        AddSpellCheckContextMenu(menu);
    }

    if (InViewableImage()) {
        AddViewImageContextMenu(menu);
    }
    //DEBUG
    if(m_DEBUG){
        AddPosInTxtContextMenu(menu);
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
        const QString &selected_word = GetCurrentWordAtCaret(true);
        offer_spelling = !selected_word.isNull() && !selected_word.isEmpty();

        // If a misspelled word is selected try to offer spelling suggestions.
        if (offer_spelling) {
            SpellCheck *sc = SpellCheck::instance();
            QStringList suggestions = sc->suggestML(selected_word);
            QAction *suggestAction = 0;

            // We want to limit the number of suggestions so we don't
            // get a huge context menu.
            QString lang{selected_word.split(',').first()};
            QString l{Language::instance()->GetLanguageName(lang)};
            lang=l.isEmpty()?lang:l;

            QMenu *sub=menu->addMenu(tr("Spelling suggestons: ")+lang);
            QFont f=sub->menuAction()->font();
            f.setBold(true);
            sub->menuAction()->setFont(f);
            for (int i = 0; i < std::min(suggestions.length(), MAX_SPELLING_SUGGESTIONS); ++i) {
                suggestAction = new QAction(suggestions.at(i), menu);
                f=suggestAction->font();
                f.setBold(true);
                suggestAction->setFont(f);

                sub->addAction(suggestAction);
                connect(suggestAction, SIGNAL(triggered()), m_spellingMapper, SLOT(map()));
                m_spellingMapper->setMapping(suggestAction, suggestions.at(i));

//                // If the menu is empty we need to append rather than insert our actions.
//                if (!topAction) {
//                    menu->addAction(suggestAction);
//                } else {
//                    menu->insertAction(topAction, suggestAction);
//                }

            }
            if(topAction) menu->insertMenu(topAction,sub);
            // Add a separator to keep our spelling actions differentiated from
            // the default menu actions.
            if (!suggestions.isEmpty() && topAction) {
                menu->insertSeparator(topAction);
            }

            // Allow the user to add the misspelled word to their default user dictionary.
            QAction *addToDictAction = new QAction(tr("Add To Default Dictionary"), menu);
            connect(addToDictAction, SIGNAL(triggered()), m_addSpellingMapper, SLOT(map()));
            m_addSpellingMapper->setMapping(addToDictAction, selected_word);

            if (topAction) {
                menu->insertAction(topAction, addToDictAction);
            } else {
                menu->addAction(addToDictAction);
            }

            // Allow the user to select a dictionary
            QStringList dictionaries = sc->userDictionaries();
            QMenu *dictionary_menu = new QMenu(this);
            dictionary_menu->setTitle(tr("Add To Dictionary"));

            if (topAction) {
                menu->insertMenu(topAction, dictionary_menu);
            } else {
                menu->addMenu(dictionary_menu);
            }

            foreach (QString dict_name, dictionaries) {
                QAction *dictAction = new QAction(dict_name, dictionary_menu);
                connect(dictAction, SIGNAL(triggered()), m_addDictMapper, SLOT(map()));
                QString key = selected_word + "\t" + dict_name;
                m_addDictMapper->setMapping(dictAction, key);
                dictionary_menu->addAction(dictAction);
            }

            // Allow the user to ignore misspelled words until the program exits
            QAction *ignoreWordAction = new QAction(tr("Ignore"), menu);
            connect(ignoreWordAction, SIGNAL(triggered()), m_ignoreSpellingMapper, SLOT(map()));
            m_ignoreSpellingMapper->setMapping(ignoreWordAction, selected_word);

            if (topAction) {
                menu->insertAction(topAction, ignoreWordAction);
                menu->insertSeparator(topAction);
            } else {
                menu->addAction(ignoreWordAction);
            }
        }
    }

    return offer_spelling;
}

QString CodeViewEditor::GetCurrentWordAtCaret(bool select_word)
{
    QTextCursor c = textCursor();
    QString lang=m_QSHParser->getLanguage(c.anchor());
    // See if we are close to or inside of a misspelled word. If so select it.
    if (!c.hasSelection()) {
        // We cannot use QTextCursor::charFormat because the format is not set directly in
        // the document. The QSyntaxHighlighter sets the format in the block layout's
        // additionalFormats property. Thus we have to check if the cursor is within
        // an additionalFormat for the block and if that format is for a misspelled word.
        int pos = c.positionInBlock();
        foreach(QTextLayout::FormatRange r, textCursor().block().layout()->additionalFormats()) {
            if (pos > r.start && pos < r.start + r.length && r.format.underlineStyle() == QTextCharFormat::WaveUnderline/*QTextCharFormat::SpellCheckUnderline*/) {
                if (select_word) {
                    c.setPosition(c.block().position() + r.start);
                    c.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, r.length);
                    setTextCursor(c);
                    QString lword=(lang.isNull())?c.selectedText():lang+QChar(',')+c.selectedText();
                    return lword;
                } else {
                    return lang+QChar(',')+toPlainText().mid(c.block().position() + r.start, r.length);
                }
            }
        }
    }
    // Check if our selection is a misspelled word.
    else {
        int selStart = c.selectionStart() - c.block().position();
        int selLen = c.selectionEnd() - c.block().position() - selStart;
        foreach(QTextLayout::FormatRange r, textCursor().block().layout()->additionalFormats()) {
            if (r.start == selStart && selLen == r.length && r.format.underlineStyle() == QTextCharFormat::WaveUnderline/*QTextCharFormat::SpellCheckUnderline*/) {
                QString lword=(lang.isNull())?c.selectedText():lang+QChar(',')+c.selectedText();
                return lword;
            }
        }
    }

    return QString();
}

void CodeViewEditor::AddReformatCSSContextMenu(QMenu *menu)
{
    QAction *topAction = 0;

    if (!menu->actions().isEmpty()) {
        topAction = menu->actions().at(0);
    }

    QMenu *reformatCSSMenu = new QMenu(tr("Reformat CSS"), menu);
    QAction *multiLineCSSAction = new QAction(tr("Multiple Lines Per Style"), reformatCSSMenu);
    QAction *singleLineCSSAction = new QAction(tr("Single Line Per Style"), reformatCSSMenu);
    connect(multiLineCSSAction, SIGNAL(triggered()), this, SLOT(ReformatCSSMultiLineAction()));
    connect(singleLineCSSAction, SIGNAL(triggered()), this, SLOT(ReformatCSSSingleLineAction()));
    reformatCSSMenu->addAction(multiLineCSSAction);
    reformatCSSMenu->addAction(singleLineCSSAction);

    if (!topAction) {
        menu->addMenu(reformatCSSMenu);
    } else {
        menu->insertMenu(topAction, reformatCSSMenu);
    }

    if (topAction) {
        menu->insertSeparator(topAction);
    }
}

void CodeViewEditor::AddReformatHTMLContextMenu(QMenu *menu)
{
    QAction *topAction = 0;

    if (!menu->actions().isEmpty()) {
        topAction = menu->actions().at(0);
    }

    QMenu *reformatMenu = new QMenu(tr("Reformat HTML"), menu);
    QAction *cleanAction = new QAction(tr("Mend and Prettify Code"), menu);
    QAction *cleanAllAction = new QAction(tr("Mend and Prettify Code - All HTML Files"), menu);
    QAction *toValidAction = new QAction(tr("Mend Code"), menu);
    QAction *toValidAllAction = new QAction(tr("Mend Code - All HTML Files"), menu);
    connect(cleanAction, SIGNAL(triggered()), this, SLOT(ReformatHTMLCleanAction()));
    connect(cleanAllAction, SIGNAL(triggered()), this, SLOT(ReformatHTMLCleanAllAction()));
    connect(toValidAction, SIGNAL(triggered()), this, SLOT(ReformatHTMLToValidAction()));
    connect(toValidAllAction, SIGNAL(triggered()), this, SLOT(ReformatHTMLToValidAllAction()));
    reformatMenu->addAction(cleanAction);
    reformatMenu->addAction(cleanAllAction);
    reformatMenu->addSeparator();
    reformatMenu->addAction(toValidAction);
    reformatMenu->addAction(toValidAllAction);

    if (!topAction) {
        menu->addMenu(reformatMenu);
    } else {
        menu->insertMenu(topAction, reformatMenu);
    }

    if (topAction) {
        menu->insertSeparator(topAction);
    }
}

void CodeViewEditor::AddGoToLinkOrStyleContextMenu(QMenu *menu)
{
    QAction *topAction = 0;

    if (!menu->actions().isEmpty()) {
        topAction = menu->actions().at(0);
    }

    QAction *goToLinkOrStyleAction = new QAction(tr("Go To Link Or Style"), menu);

    if (!topAction) {
        menu->addAction(goToLinkOrStyleAction);
    } else {
        menu->insertAction(topAction, goToLinkOrStyleAction);
    }

    connect(goToLinkOrStyleAction, SIGNAL(triggered()), this, SLOT(GoToLinkOrStyleAction()));

    if (topAction) {
        menu->insertSeparator(topAction);
    }
}

void CodeViewEditor::AddViewImageContextMenu(QMenu *menu)
{
    QAction *topAction = 0;

    if (!menu->actions().isEmpty()) {
        topAction = menu->actions().at(0);
    }

    QAction *viewImageAction = new QAction(tr("View Image"), menu);
    QAction *openImageAction = new QAction(tr("Open Tab For Image"), menu);

    if (!topAction) {
        menu->addAction(viewImageAction);
        menu->addAction(openImageAction);
    } else {
        menu->insertAction(topAction, viewImageAction);
        menu->insertAction(topAction, openImageAction);
    }

    connect(viewImageAction, SIGNAL(triggered()), this, SLOT(GoToLinkOrStyle()));
    connect(openImageAction, SIGNAL(triggered()), this, SLOT(OpenImageAction()));

    if (topAction) {
        menu->insertSeparator(topAction);
    }
}

void CodeViewEditor::AddMarkSelectionMenu(QMenu *menu)
{
    QAction *topAction = 0;

    if (!menu->actions().isEmpty()) {
        topAction = menu->actions().at(0);
    }

    QString text = tr("Mark Selected Text");
    if (!textCursor().hasSelection() && IsMarkedText()) {
        text = tr("Unmark Marked Text");
    }
    QAction *markSelectionAction = new QAction(text, menu);

    if (!topAction) {
        menu->addAction(markSelectionAction);
    } else {
        menu->insertAction(topAction, markSelectionAction);
    }

    connect(markSelectionAction, SIGNAL(triggered()), this, SIGNAL(MarkSelectionRequest()));

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

    QMenu *clips_menu = new QMenu(this);
    clips_menu->setTitle(tr("Clips"));

    if (topAction) {
        menu->insertMenu(topAction, clips_menu);
    } else {
        menu->addMenu(clips_menu);
    }

    CreateMenuEntries(clips_menu, 0, ClipEditorModel::instance()->invisibleRootItem());

    QAction *saveClipAction = new QAction(tr("Add To Clips") + "...", menu);

    if (!topAction) {
        menu->addAction(saveClipAction);
    } else {
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
            } else {
                parent_menu->insertAction(topAction, clipAction);
            }
        } else {
            group_menu = new QMenu(this);
            group_menu->setTitle(item->text());

            if (topAction) {
                parent_menu->insertMenu(topAction, group_menu);
            } else {
                parent_menu->addMenu(group_menu);
            }

            topAction = 0;
        }
    }

    // Recursively add entries for children
    for (int row = 0; row < item->rowCount(); row++) {
        CreateMenuEntries(group_menu, topAction, item->child(row, 0));
    }

    return item->rowCount() > 0;
}

void CodeViewEditor::SaveClipAction()
{
    ClipEditorModel::clipEntry *pendingClipEntryRequest = new ClipEditorModel::clipEntry();
    pendingClipEntryRequest->name = "Unnamed Entry";
    pendingClipEntryRequest->is_group = false;
    QTextCursor cursor = textCursor();
    pendingClipEntryRequest->text = cursor.selectedText();
    emit OpenClipEditorRequest(pendingClipEntryRequest);
}

bool CodeViewEditor::InViewableImage()
{
    QString text = toPlainText();
    QString url_name = GetAttribute("src", SRC_TAGS, true);

    if (url_name.isEmpty()) {
        // We do not know what namespace may have been used
        url_name = GetAttribute(":href", IMAGE_TAGS, true);
    }

    return !url_name.isEmpty();
}

void CodeViewEditor::OpenImageAction()
{
    QString text = toPlainText();
    QString url_name = GetAttribute("src", SRC_TAGS, true);

    if (url_name.isEmpty()) {
        // We do not know what namespace may have been used
        url_name = GetAttribute(":href", IMAGE_TAGS, true);
    }

    emit LinkClicked(QUrl(url_name));
}

void CodeViewEditor::GoToLinkOrStyleAction()
{
    GoToLinkOrStyle();
}

void CodeViewEditor::GoToLinkOrStyle()
{
    QString text = toPlainText();
    QString url_name = GetAttribute("href", ANCHOR_TAGS, true);

    if (url_name.isEmpty()) {
        url_name = GetAttribute("src", SRC_TAGS, true);
    }

    if (url_name.isEmpty()) {
        // We do not know what namespace may have been used
        url_name = GetAttribute(":href", IMAGE_TAGS, true);
    }

    if (!url_name.isEmpty()) {
        QUrl url = QUrl(url_name);
        QString extension = url_name.right(url_name.length() - url_name.lastIndexOf('.') - 1).toLower();

        if (IMAGE_EXTENSIONS.contains(extension)) {
            emit ViewImage(QUrl(url_name));
        } else {
            emit LinkClicked(QUrl(url_name));
        }
    } else if (IsPositionInOpeningTag()) {
        GoToStyleDefinition();
    } else {
        emit ShowStatusMessageRequest(tr("You must be in an opening HTML tag to use this feature."));
    }
}

void CodeViewEditor::GoToStyleDefinition()
{
    // Begin by identifying the tag name and selected class style attribute if any
    CodeViewEditor::StyleTagElement element = GetSelectedStyleTagElement();

    if (element.name.isEmpty()) {
        emit ShowStatusMessageRequest(tr("You must be inside an opening HTML tag to use this feature."));
        return;
    }

    CSSInfo css_info(toPlainText(), false);
    CSSInfo::CSSSelector *selector = css_info.getCSSSelectorForElementClass(element.name, element.classStyle);

    if (!selector) {
        // We didn't find the style - escalate as an event to look in linked stylesheets
        emit GoToLinkedStyleDefinitionRequest(element.name, element.classStyle);
    } else {
        // Emit a signal to bookmark our code location, enabling the "Back to" feature
        emit BookmarkLinkOrStyleLocationRequest();
        // Scroll to the line after bookmarking or we lose our place
        ScrollToPosition(selector->position);
    }
}

void CodeViewEditor::AddMisspelledWord()
{
    if (m_checkSpelling) {
        const QString &selected_word = GetCurrentWordAtCaret(false);

        if (!selected_word.isNull() && !selected_word.isEmpty()) {
            addToDefaultDictionary(selected_word);
            emit SpellingHighlightRefreshRequest();
        }
    }
}

void CodeViewEditor::IgnoreMisspelledWord()
{
    if (m_checkSpelling) {
        const QString &selected_word = GetCurrentWordAtCaret(false);

        if (!selected_word.isNull() && !selected_word.isEmpty()) {
            ignoreWord(selected_word);
            emit SpellingHighlightRefreshRequest();
        }
    }
}

void CodeViewEditor::ReformatCSSMultiLineAction()
{
    ReformatCSS(true);
}

void CodeViewEditor::ReformatCSSSingleLineAction()
{
    ReformatCSS(false);
}

void CodeViewEditor::ReformatHTMLCleanAction()
{
    ReformatHTML(false, false);
}

void CodeViewEditor::ReformatHTMLCleanAllAction()
{
    ReformatHTML(true, false);
}

void CodeViewEditor::ReformatHTMLToValidAction()
{
    ReformatHTML(false, true);
}

void CodeViewEditor::ReformatHTMLToValidAllAction()
{
    ReformatHTML(true, true);
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

void CodeViewEditor::AddToIndex()
{
    if (!TextIsSelectedAndNotInStartOrEndTag()) {
        return;
    }

    IndexEditorModel::indexEntry *index = new IndexEditorModel::indexEntry();
    QTextCursor cursor = textCursor();
    index->pattern = cursor.selectedText();
    emit OpenIndexEditorRequest(index);
}

bool CodeViewEditor::IsAddToIndexAllowed()
{
    return TextIsSelectedAndNotInStartOrEndTag();
}

bool CodeViewEditor::IsInsertIdAllowed()
{
    int pos = textCursor().selectionStart();
    QString text = toPlainText();

    if (!IsPositionInBody(pos, text)) {
        return false;
    }

    // Only allow if the closing tag we're in is an "a" tag
    QString closing_tag_name = GetClosingTagName(pos, text);

    if (!closing_tag_name.isEmpty() && !ANCHOR_TAGS.contains(closing_tag_name)) {
        return false;
    }

    // Only allow if the opening tag we're in is valid for id attribute
    QString tag_name = GetOpeningTagName(pos, text);

    if (!tag_name.isEmpty() && !ID_TAGS.contains(tag_name)) {
        return false;
    }

    return true;
}

bool CodeViewEditor::IsInsertHyperlinkAllowed()
{
    int pos = textCursor().selectionStart();
    QString text = toPlainText();

    if (!IsPositionInBody(pos, text)) {
        return false;
    }

    // Only allow if the closing tag we're in is an "a" tag
    QString closing_tag_name = GetClosingTagName(pos, text);

    if (!closing_tag_name.isEmpty() && !ANCHOR_TAGS.contains(closing_tag_name)) {
        return false;
    }

    // Only allow if the opening tag we're in is an "a" tag
    QString tag_name = GetOpeningTagName(pos, text);

    if (!tag_name.isEmpty() && !ANCHOR_TAGS.contains(tag_name)) {
        return false;
    }

    return true;
}

bool CodeViewEditor::IsInsertFileAllowed()
{
    int pos = textCursor().selectionStart();
    QString text = toPlainText();
    return IsPositionInBody(pos, text) && !IsPositionInTag(pos, text);
}

bool CodeViewEditor::InsertId(const QString &attribute_value)
{
    int pos = textCursor().selectionStart();
    QString text = toPlainText();
    const QString &element_name = "a";
    const QString &attribute_name = "id";
    // If we're in an a tag we can update the id even if not in the opening tag
    QStringList tag_list = ID_TAGS;

    if (GetOpeningTagName(pos, text).isEmpty()) {
        tag_list = ANCHOR_TAGS;
    }

    return InsertTagAttribute(element_name, attribute_name, attribute_value, tag_list);
}

bool CodeViewEditor::InsertHyperlink(const QString &attribute_value)
{
    const QString &element_name = "a";
    const QString &attribute_name = "href";

    // HTML safe.
    QString safe_attribute_value = attribute_value;
    safe_attribute_value.replace("&amp;", "&");
    safe_attribute_value.replace("&", "&amp;");
    safe_attribute_value.replace("<", "&lt;");
    safe_attribute_value.replace(">", "&gt;");

    return InsertTagAttribute(element_name, attribute_name, safe_attribute_value, ANCHOR_TAGS);
}

bool CodeViewEditor::InsertTagAttribute(const QString &element_name, const QString &attribute_name, const QString &attribute_value, const QStringList &tag_list, bool ignore_selection)
{
    bool inserted = false;

    // Add or update the attribute within the start tag and return if ok
    if (!SetAttribute(attribute_name, tag_list, attribute_value, false, true).isEmpty()) {
        return true;
    }

    // If nothing was inserted, then just insert a new tag with no text as long as we aren't in a tag
    if (!textCursor().hasSelection() && !IsPositionInTag()) {
        InsertHTMLTagAroundText(element_name, "/" % element_name, attribute_name % "=\"" % attribute_value % "\"", "");
        inserted = true;
    } else if (TextIsSelectedAndNotInStartOrEndTag()) {
        // Just prepend and append the tag pairs to the text
        QString attributes = attribute_name % "=\"" % attribute_value % "\"";
        InsertHTMLTagAroundSelection(element_name, "/" % element_name, attributes);
        inserted = true;
    }

    return inserted;
}

bool CodeViewEditor::MarkForIndex(const QString &title)
{
    QString safe_title = title;
    // HTML safe.
    safe_title.replace("&amp;", "&");
    safe_title.replace("&", "&amp;");
    safe_title.replace("<", "&lt;");
    safe_title.replace(">", "&gt;");

    bool ok = true;
    QString selected_text = textCursor().selectedText();
    const QString &element_name = "a";
    const QString &attribute_name = "class";

    if (!InsertTagAttribute(element_name, attribute_name, SIGIL_INDEX_CLASS, ANCHOR_TAGS)) {
        ok = false;
    }

    const QString &second_attribute_name = "title";

    if (!InsertTagAttribute(element_name, second_attribute_name, safe_title, ANCHOR_TAGS, true)) {
        ok = false;
    }

    return ok;
}

// Overridden so we can emit the FocusGained() signal.
void CodeViewEditor::focusInEvent(QFocusEvent *event)
{
    RehighlightDocument(); //is it really neccesery?
    emit FocusGained(this);
    QPlainTextEdit::focusInEvent(event);
}


// Overridden so we can emit the FocusLost() signal.
void CodeViewEditor::focusOutEvent(QFocusEvent *event)
{
    emit FocusLost(this);
    QPlainTextEdit::focusOutEvent(event);
}

void CodeViewEditor::EmitFilteredCursorMoved()
{
    // Avoid slowdown while selecting text
    if (QApplication::mouseButtons() == Qt::NoButton) {
        emit FilteredCursorMoved();
    }
}

void CodeViewEditor::TextChangedFilter()
{
    // Clear marked text to prevent marked area not matching entered text
    // if user types text, uses Undo, etc.
    if (!m_ReplacingInMarkedText && IsMarkedText()) {
        emit ClearMarkedTextRequest();
    }

    ResetLastFindMatch();

    if (m_isUndoAvailable) {
        emit FilteredTextChanged();
    }
}

void CodeViewEditor::RehighlightDocument()
{
    if (!isVisible()) {
        return;
    }

    if (m_Highlighter) {
        // We block signals from the document while highlighting takes place,
        // because we do not want the contentsChanged() signal to be fired
        // which would mark the underlying resource as needing saving.
        document()->blockSignals(true);
        updateLangMap();
        m_Highlighter->rehighlight();
        document()->blockSignals(false);
    }
}


void CodeViewEditor::UpdateUndoAvailable(bool available)
{
    m_isUndoAvailable = available;
}


void CodeViewEditor::UpdateLineNumberAreaMargin()
{
    // The left margin width depends on width of the line number area
    setViewportMargins(CalculateLineNumberAreaWidth(), 0, 0, 0);
}


void CodeViewEditor::UpdateLineNumberArea(const QRect &area_to_update, int vertically_scrolled)
{
    // If the editor scrolled, scroll the line numbers too
    if (vertically_scrolled != 0) {
        m_LineNumberArea->scroll(0, vertically_scrolled);
    } else { // otherwise update the required portion
        m_LineNumberArea->update(0, area_to_update.y(), m_LineNumberArea->width(), area_to_update.height());
    }

    if (area_to_update.contains(viewport()->rect())) {
        UpdateLineNumberAreaMargin();
    }
}


void CodeViewEditor::HighlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    // Draw the full width line color.
    QTextEdit::ExtraSelection selection_line;
    selection_line.format.setBackground(m_codeViewAppearance.line_highlight_color);
    selection_line.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection_line.cursor = textCursor();
    selection_line.cursor.clearSelection();
    extraSelections.append(selection_line);

    // Add highlighting of the marked text
    if (IsMarkedText()) {
        if (m_MarkedTextEnd > toPlainText().length()) {
            m_MarkedTextEnd = toPlainText().length();
        }

        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(m_codeViewAppearance.line_number_background_color);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        selection.cursor.setPosition(m_MarkedTextStart);
        selection.cursor.setPosition(m_MarkedTextEnd, QTextCursor::KeepAnchor);
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}


void CodeViewEditor::ScrollOneLineUp()
{
    ScrollByLine(false);
}


void CodeViewEditor::ScrollOneLineDown()
{
    ScrollByLine(true);
}


void CodeViewEditor::InsertText(const QString &text)
{
    QTextCursor c = textCursor();
    c.insertText(text);
    setTextCursor(c);
}

void CodeViewEditor::HighlightWord(const QString &word, int pos)
{
    QTextCursor cursor = textCursor();
    cursor.clearSelection();
    cursor.setPosition(pos);
    cursor.setPosition(pos + word.length(), QTextCursor::KeepAnchor);
    setTextCursor(cursor);
}

void CodeViewEditor::RefreshSpellingHighlighting()
{
    if (hasFocus()) {
        RehighlightDocument();
    }
}

void CodeViewEditor::addToUserDictionary(const QString &text)
{
    QString word = text.split("\t")[0];
    QString dict_name = text.split("\t")[1];
    SpellCheck *sc = SpellCheck::instance();
    sc->addToUserDictionary(word, dict_name);
    emit SpellingHighlightRefreshRequest();
}

void CodeViewEditor::addToDefaultDictionary(const QString &text)
{
    SpellCheck *sc = SpellCheck::instance();
    sc->addToUserDictionary(text);
    emit SpellingHighlightRefreshRequest();
}

void CodeViewEditor::ignoreWord(const QString &text)
{
    SpellCheck *sc = SpellCheck::instance();
    sc->ignoreWord(text);
    emit SpellingHighlightRefreshRequest();
}

void CodeViewEditor::PasteText(const QString &text)
{
    InsertText(text);
}

bool CodeViewEditor::PasteClipNumber(int clip_number)
{
    ClipEditorModel::clipEntry *clip = ClipEditorModel::instance()->GetEntryFromNumber(clip_number);
    if (!clip) {
        return false;
    }

    PasteClipEntry(clip);

    return true;
}

bool CodeViewEditor::PasteClipEntries(const QList<ClipEditorModel::clipEntry *> &clips)
{
    bool applied = false;
    foreach(ClipEditorModel::clipEntry * clip, clips) {
        applied = applied || PasteClipEntry(clip);
    }
    return applied;
}

void CodeViewEditor::PasteClipEntryFromName(const QString &name)
{
    ClipEditorModel::clipEntry *clip = ClipEditorModel::instance()->GetEntryFromName(name);
    PasteClipEntry(clip);
}

bool CodeViewEditor::PasteClipEntry(ClipEditorModel::clipEntry *clip)
{
    if (!clip || clip->text.isEmpty()) {
        return false;
    }

    QTextCursor cursor = textCursor();
    // Convert to plain text or \s won't get newlines
    const QString &document_text = toPlainText();
    QString selected_text = Utility::Substring(cursor.selectionStart(), cursor.selectionEnd(), document_text);

    if (selected_text.isEmpty()) {
        // Allow users to use the same entry for insert/replace
        // Will not handle complicated regex, but good for tags like <p>\1</p>
        QString replacement_text = clip->text;
        replacement_text.remove(QString("\\1"));
        cursor.beginEditBlock();
        cursor.removeSelectedText();
        cursor.insertText(replacement_text);
        cursor.endEditBlock();
        setTextCursor(cursor);
    } else {
        QString search_regex = "(?s)(" + QRegularExpression::escape(selected_text) + ")";
        // We must do a Find before we do the Replace in order to ensure the match info that
        // ReplaceSelected relies upon is loaded.
        cursor.setPosition(cursor.selectionStart());
        setTextCursor(cursor);
        bool found = FindNext(search_regex, Searchable::Direction_Down, false, false, false);

        if (found) {
            ReplaceSelected(search_regex, clip->text, Searchable::Direction_Down, true);
        }
        cursor.setPosition(cursor.selectionEnd());
        setTextCursor(cursor);
    }

    return true;
}

void CodeViewEditor::ResetFont()
{
    // Let's try to use our user specified value as our font (default Courier New)
    QFont font(m_codeViewAppearance.font_family, m_codeViewAppearance.font_size);
    // But just in case, say we want a fixed width font if font is not present
    font.setStyleHint(QFont::TypeWriter);
    setFont(font);
    setTabStopWidth(TAB_SPACES_WIDTH * QFontMetrics(font).width(' '));
    UpdateLineNumberAreaFont(font);
}


void CodeViewEditor::UpdateLineNumberAreaFont(const QFont &font)
{
    m_LineNumberArea->setFont(font);
    m_LineNumberArea->MyUpdateGeometry();
    UpdateLineNumberAreaMargin();
}

void CodeViewEditor::SetAppearanceColors()
{
    QPalette pal = palette();

    if (m_codeViewAppearance.background_color.isValid()) {
        pal.setColor(QPalette::Base, m_codeViewAppearance.background_color);
        pal.setColor(QPalette::Window, m_codeViewAppearance.background_color);
        setBackgroundVisible(true);
    } else {
        setBackgroundVisible(false);
    }

    if (m_codeViewAppearance.foreground_color.isValid()) {
        pal.setColor(QPalette::Text, m_codeViewAppearance.foreground_color);
    }

    if (m_codeViewAppearance.selection_background_color.isValid()) {
        pal.setColor(QPalette::Highlight, m_codeViewAppearance.selection_background_color);
    }

    if (m_codeViewAppearance.selection_foreground_color.isValid()) {
        pal.setColor(QPalette::HighlightedText, m_codeViewAppearance.selection_foreground_color);
    }

    setPalette(pal);
}


// Center the screen on the cursor/caret location.
// Centering requires fresh information about the
// visible viewport, so we usually call this after
// the paint event has been processed.
void CodeViewEditor::DelayedCursorScreenCentering()
{
    if (m_DelayedCursorScreenCenteringRequired) {
        centerCursor();
        m_DelayedCursorScreenCenteringRequired = false;
    }
}


void CodeViewEditor::SetDelayedCursorScreenCenteringRequired()
{
    m_DelayedCursorScreenCenteringRequired = true;
}

int CodeViewEditor::GetSelectionOffset(Searchable::Direction search_direction, bool ignore_selection_offset, bool marked_text) const
{
    int offset = 0;
    if (search_direction == Searchable::Direction_Up) {
        if (ignore_selection_offset) {
            if (marked_text) {
                offset = m_MarkedTextEnd;
            } else {
                offset = toPlainText().length();
            }
        } else {
            offset = textCursor().selectionStart();
        }
    } else {
        if (ignore_selection_offset) {
            if (marked_text) {
                offset = m_MarkedTextStart;
            } else {
                offset = 0;
            }
        } else {
            offset = textCursor().selectionEnd();
        }
    }

    return offset;
}


void CodeViewEditor::ScrollByLine(bool down)
{
    int current_scroll_value = verticalScrollBar()->value();
    int move_delta = down ? 1 : -1;
    verticalScrollBar()->setValue(current_scroll_value + move_delta);

    if (!contentsRect().contains(cursorRect())) {
        if (move_delta > 0) {
            moveCursor(QTextCursor::Down);
        } else {
            moveCursor(QTextCursor::Up);
        }
    }
}


QList<ViewEditor::ElementIndex> CodeViewEditor::GetCaretLocation()
{
    // We search for the first opening tag *behind* the caret.
    // This specifies the element the caret is located in.
    int pos = textCursor().position();
    int offset = 0;
    int len = 0;
    QRegularExpression tag(XML_OPENING_TAG);
    QRegularExpressionMatchIterator i = tag.globalMatch(toPlainText());
    // There is no way to search for the last match (str.lastIndexOf) in a string and also have
    // the matched text length. So we search forward for every match (QRegularExpression doesn't
    // have a way to search backwards) and only use the last match's info.
    while (i.hasNext()) {
        QRegularExpressionMatch mo = i.next();
        int start = mo.capturedStart();
        if (start > pos) {
            break;
        }
        offset = start;
        len = mo.capturedLength();
    }
    return ConvertStackToHierarchy(GetCaretLocationStack(offset + len));
}


void CodeViewEditor::StoreCaretLocationUpdate(const QList<ViewEditor::ElementIndex> &hierarchy)
{
    m_CaretUpdate = hierarchy;
}


QStack<CodeViewEditor::StackElement> CodeViewEditor::GetCaretLocationStack(int offset) const
{
    QString source = toPlainText();
    QXmlStreamReader reader(source);
    QStack<StackElement> stack;

    while (!reader.atEnd()) {
        reader.readNext();

        if (reader.isStartElement()) {
            // If we detected the start of a new element, then
            // the element currently on the top of the stack
            // has one more child element
            if (!stack.isEmpty()) {
                stack.top().num_children++;
            }

            StackElement new_element;
            new_element.name         = reader.name().toString();
            new_element.num_children = 0;
            stack.push(new_element);

            // Check if this is the element start tag
            // we are looking for
            if (reader.characterOffset() == offset) {
                break;
            }
        }
        // If we detect the end tag of an element,
        // we remove it from the top of the stack
        else if (reader.isEndElement()) {
            stack.pop();
        }
    }

    if (reader.hasError()) {
        // Just return an empty location.
        // Maybe we could return the stack we currently have?
        return QStack<StackElement>();
    }

    return stack;
}


QString CodeViewEditor::ConvertHierarchyToQWebPath(const QList<ViewEditor::ElementIndex>& hierarchy) const
{
    QStringList pathparts;
    for (int i=0; i < hierarchy.count(); i++) {
        QString part = hierarchy.at(i).name + " " + QString::number(hierarchy.at(i).index);
        pathparts.append(part);
    }
    return pathparts.join(",");
}


QList<ViewEditor::ElementIndex> CodeViewEditor::ConvertStackToHierarchy(const QStack<StackElement> stack) const
{
    QList<ViewEditor::ElementIndex> hierarchy;
    foreach(StackElement stack_element, stack) {
        ViewEditor::ElementIndex new_element;
        new_element.name  = stack_element.name;
        new_element.index = stack_element.num_children - 1;
        hierarchy.append(new_element);
    }
    return hierarchy;
}


std::tuple<int, int> CodeViewEditor::ConvertHierarchyToCaretMove(const QList<ViewEditor::ElementIndex> &hierarchy) const
{
    QString source = toPlainText();
    QString version = "any_version";
    GumboInterface gi = GumboInterface(source, version);
    gi.parse();
    QString webpath = ConvertHierarchyToQWebPath(hierarchy);
    GumboNode* end_node = gi.get_node_from_qwebpath(webpath);
    unsigned int line = 0;
    unsigned int col = 0;
    if ((end_node->type == GUMBO_NODE_TEXT) || (end_node->type == GUMBO_NODE_WHITESPACE) || 
        (end_node->type == GUMBO_NODE_CDATA) || (end_node->type == GUMBO_NODE_COMMENT)) {
        line = end_node->v.text.start_pos.line + 1; // compensate for xml header removed for gumbo
        col = end_node->v.text.start_pos.column;
    } else if ((end_node->type == GUMBO_NODE_ELEMENT) || (end_node->type == GUMBO_NODE_TEMPLATE)) {
      line = end_node->v.element.start_pos.line + 1; // comprensate for xml header removed for gumbo
        col = end_node->v.element.start_pos.column;
    }
    QTextCursor cursor(document());
    if (end_node)
        return std::make_tuple(line - cursor.blockNumber(), col);
    else {
        return std::make_tuple(0, 0);
    }
}


bool CodeViewEditor::ExecuteCaretUpdate(bool default_to_top)
{
    // If there's a cursor/caret update waiting (from BookView),
    // we update the caret location and reset the update variable
    if (m_CaretUpdate.isEmpty()) {
        if (default_to_top) {
            QTextCursor cursor = textCursor();
            cursor.movePosition(QTextCursor::Start);
            setTextCursor(cursor);
        }

        return false;
    }

    QTextCursor cursor(document());
    int vertical_lines_move = 0;
    int horizontal_chars_move = 0;
    // We *have* to do the conversion on-demand since the
    // conversion uses toPlainText(), and the text needs to up-to-date.
    std::tie(vertical_lines_move, horizontal_chars_move) = ConvertHierarchyToCaretMove(m_CaretUpdate);
    cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, vertical_lines_move - 1);

    for (int i = 1 ; i < horizontal_chars_move ; i++) {
        cursor.movePosition(QTextCursor::NextCharacter , QTextCursor::MoveAnchor);
        // TODO: cursor.movePosition( QTextCursor::Left, ...) is badly bugged in Qt 4.7.
        // Test whether it's fixed when the next version of Qt comes out.
        // cursor.movePosition( QTextCursor::Left, QTextCursor::MoveAnchor, horizontal_chars_move );
    }

    m_CaretUpdate.clear();
    setTextCursor(cursor);
    m_DelayedCursorScreenCenteringRequired = true;
    return true;
}


void CodeViewEditor::FormatBlock(const QString &element_name, bool preserve_attributes)
{
    if (element_name.isEmpty()) {
        return;
    }

    // Emit a selection changed event, so we can make sure the style buttons are updated
    // to uncheck any heading buttons check states.
    emit selectionChanged();
    // Going to assume that the user is allowed to click anywhere within or just after the block
    // Also makes assumptions about being well formed, or else crazy things may happen...
    int pos = textCursor().selectionStart();
    QString text = toPlainText();

    if (!IsPositionInBody(pos, text)) {
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
    QRegularExpression tag_search(NEXT_TAG_LOCATION);
    QRegularExpression tag_name_search(TAG_NAME_SEARCH);
    // Search backwards for the next block tag
    pos--;

    while (true) {
        int previous_tag_index = text.lastIndexOf(tag_search, pos);

        if (previous_tag_index < 0) {
            return;
        }

        // We found a tag. It could be opening or closing.
        QRegularExpressionMatch mo = tag_name_search.match(text, previous_tag_index);
        int tag_name_index = mo.capturedStart();

        if (tag_name_index < 0) {
            pos = previous_tag_index - 1;
            continue;
        }

        tag_name = mo.captured(1).toLower();
        // Isolate whether it was opening or closing tag.
        bool is_closing_tag = false;

        if (tag_name.startsWith('/')) {
            is_closing_tag = true;
            tag_name = tag_name.right(tag_name.length() - 1);
        }

        // Is this a block level tag? If not, keep searching...
        if (!BLOCK_LEVEL_TAGS.contains(tag_name)) {
            pos = previous_tag_index - 1;
            continue;
        }

        // Has the user clicked inside a closing block tag?
        // If so, keep searching backwards
        if (is_closing_tag) {
            closing_tag_end = text.indexOf(QChar('>'), tag_name_index);

            if (closing_tag_end >= textCursor().selectionStart()) {
                pos = previous_tag_index - 1;
                continue;
            }
        }

        // Special case for the body tag or clicked past a previous closing block tag
        // In these situations we just insert html around our selection.
        if (tag_name == "body" || is_closing_tag) {
            InsertHTMLTagAroundSelection(element_name, "/" % element_name);
            return;
        }

        // If we got to here we know we have an opening block tag we shall replace.
        opening_tag_start = previous_tag_index;
        // Grab any attributes applied to this opening tag
        int attribute_start_index = tag_name_index + mo.capturedLength();
        opening_tag_end = text.indexOf('>', attribute_start_index) + 1;
        opening_tag_attributes = text.mid(attribute_start_index, opening_tag_end - attribute_start_index - 1).trimmed();
        // Now find the closing tag for this block.
        QRegularExpression closing_tag_search("</\\s*" % tag_name % "\\s*>", QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch closing_tag_search_mo = closing_tag_search.match(text, opening_tag_end);
        closing_tag_start = closing_tag_search_mo.capturedStart();

        if (closing_tag_start < 0) {
            // Could not find a closing tag for this block name. Invalid HTML.
            return;
        }

        closing_tag_end = closing_tag_start + closing_tag_search_mo.capturedLength();
        // Success
        break;
    }

    if (preserve_attributes && opening_tag_attributes.length() > 0) {
        opening_tag_text = "<" % element_name % " " % opening_tag_attributes % ">";
    } else {
        opening_tag_text = "<" % element_name % ">";
    }

    closing_tag_text = "</" % element_name % ">";
    ReplaceTags(opening_tag_start, opening_tag_end, opening_tag_text,
                closing_tag_start, closing_tag_end, closing_tag_text);
}

void CodeViewEditor::InsertHTMLTagAroundText(const QString &left_element_name, const QString &right_element_name, const QString &attributes, const QString &text)
{
    QTextCursor cursor = textCursor();
    QString new_attributes = attributes;

    if (!new_attributes.isEmpty()) {
        new_attributes.prepend(" ");
    }

    const QString &prefix = "<" % left_element_name % new_attributes % ">";
    const QString &new_text = prefix % text % "<" % right_element_name % ">";
    int selection_start = cursor.selectionStart() + prefix.length();
    cursor.beginEditBlock();
    cursor.insertText(new_text);
    cursor.endEditBlock();
    cursor.setPosition(selection_start + text.length());
    cursor.setPosition(selection_start, QTextCursor::KeepAnchor);
    setTextCursor(cursor);
}

void CodeViewEditor::InsertHTMLTagAroundSelection(const QString &left_element_name, const QString &right_element_name, const QString &attributes)
{
    QTextCursor cursor = textCursor();
    QString new_attributes = attributes;

    if (!new_attributes.isEmpty()) {
        new_attributes.prepend(" ");
    }

    const QString &selected_text = cursor.selectedText();
    const QString &prefix_text = "<" % left_element_name % new_attributes % ">";
    const QString &replacement_text = prefix_text % selected_text % "<" % right_element_name % ">";
    int selection_start = cursor.selectionStart();
    cursor.beginEditBlock();
    cursor.insertText(replacement_text);
    cursor.endEditBlock();
    // Move caret to between the tags to allow the user to start typing/keep selection.
    cursor.setPosition(selection_start + prefix_text.length() + selected_text.length());
    cursor.setPosition(selection_start + prefix_text.length(), QTextCursor::KeepAnchor);
    setTextCursor(cursor);
}

bool CodeViewEditor::IsPositionInBody(const int &pos, const QString &text)
{
    int search_pos = pos;

    if (pos == -1) {
        search_pos = textCursor().selectionStart();
    }

    QString search_text = text;

    if (text.isEmpty()) {
        search_text = toPlainText();
    }

    QRegularExpression body_search(BODY_START, QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch mo = body_search.match(search_text);
    int body_tag_end   = mo.capturedEnd();

    QRegularExpression body_end_search(BODY_END, QRegularExpression::CaseInsensitiveOption);
    int body_contents_end = search_text.indexOf(body_end_search);

    if ((search_pos < body_tag_end) || (search_pos > body_contents_end)) {
        return false;
    }

    return true;
}

bool CodeViewEditor::IsPositionInTag(const int &pos, const QString &text)
{
    int search_pos = pos;

    if (pos == -1) {
        search_pos = textCursor().selectionStart();
    }

    QString search_text = text;

    if (text.isEmpty()) {
        search_text = toPlainText();
    }

    int tag_start = -1;
    int tag_end = -1;
    QRegularExpression tag_search(NEXT_TAG_LOCATION);
    QRegularExpressionMatchIterator i = tag_search.globalMatch(search_text);
    while (i.hasNext()) {
        QRegularExpressionMatch mo = i.next();
        int start = mo.capturedStart();
        if (start > search_pos-1) {
            break;
        }
        tag_start = start;
        tag_end = mo.capturedEnd();
    }

    if ((search_pos >= tag_start) && (search_pos < tag_end)) {
        return true;
    }

    return false;
}

bool CodeViewEditor::IsPositionInOpeningTag(const int &pos, const QString &text)
{
    int search_pos = pos;

    if (pos == -1) {
        search_pos = textCursor().selectionStart();
    }

    QString search_text = text;

    if (text.isEmpty()) {
        search_text = toPlainText();
    }

    int tag_start = -1;
    int tag_end = -1;
    QRegularExpression tag_search("<\\s*[^/][^>]*>");
    QRegularExpressionMatchIterator i = tag_search.globalMatch(search_text);
    while (i.hasNext()) {
        QRegularExpressionMatch mo = i.next();
        int start = mo.capturedStart();
        if (start > search_pos-1) {
            break;
        }
        tag_start = start;
        tag_end = mo.capturedEnd();
    }

    if ((search_pos >= tag_start) && (search_pos < tag_end)) {
        return true;
    }

    return false;
}

bool CodeViewEditor::IsPositionInClosingTag(const int &pos, const QString &text)
{
    int search_pos = pos;

    if (pos == -1) {
        search_pos = textCursor().selectionStart();
    }

    QString search_text = text;

    if (text.isEmpty()) {
        search_text = toPlainText();
    }

    int tag_start = -1;
    int tag_end = -1;
    QRegularExpression tag_search("<\\s*/[^>]*>");
    QRegularExpressionMatchIterator i = tag_search.globalMatch(search_text);
    while (i.hasNext()) {
        QRegularExpressionMatch mo = i.next();
        int start = mo.capturedStart();
        if (start > search_pos-1) {
            break;
        }
        tag_start = start;
        tag_end = mo.capturedEnd();
    }

    if ((search_pos >= tag_start) && (search_pos < tag_end)) {
        return true;
    }

    return false;
}

QString CodeViewEditor::GetOpeningTagName(const int &pos, const QString &text)
{
    QString tag_name;
    int tag_start = -1;
    int tag_end = -1;
    QRegularExpression tag_search("<\\s*[^/][^>]*>");
    QRegularExpressionMatchIterator i = tag_search.globalMatch(text);
    while (i.hasNext()) {
        QRegularExpressionMatch mo = i.next();
        int start = mo.capturedStart();
        if (start > pos-1) {
            break;
        }
        tag_start = start;
        tag_end = mo.capturedEnd();
    }

    if ((pos >= tag_start) && (pos < tag_end)) {
        QRegularExpression tag_name_search(TAG_NAME_SEARCH);
        QRegularExpressionMatch tag_name_search_mo = tag_name_search.match(text, tag_start);
        int tag_name_index = tag_name_search_mo.capturedStart();

        if (tag_name_index < 0) {
            return tag_name;
        }

        tag_name = tag_name_search_mo.captured(1).toLower();
    }

    return tag_name;
}

QString CodeViewEditor::GetClosingTagName(const int &pos, const QString &text)
{
    QString tag_name;
    int tag_start = -1;
    int tag_end = -1;
    QRegularExpression tag_search("<\\s*/[^>]*>");
    QRegularExpressionMatchIterator i = tag_search.globalMatch(text);
    while (i.hasNext()) {
        QRegularExpressionMatch mo = i.next();
        int start = mo.capturedStart();
        if (start > pos-1) {
            break;
        }
        tag_start = start;
        tag_end = mo.capturedEnd();
    }

    if ((pos >= tag_start) && (pos < tag_end)) {
        QRegularExpression tag_name_search(TAG_NAME_SEARCH);
        QRegularExpressionMatch tag_name_search_mo = tag_name_search.match(text, tag_start);
        int tag_name_index = tag_name_search_mo.capturedStart();

        if (tag_name_index < 0) {
            return tag_name;
        }

        tag_name = tag_name_search_mo.captured(1).toLower();
        tag_name = tag_name.right(tag_name.length() - 1);
    }

    return tag_name;
}

void CodeViewEditor::ToggleFormatSelection(const QString &element_name, const QString property_name, const QString property_value)
{
    if (element_name.isEmpty()) {
        return;
    }

    // Emit a selection changed event, so we can make sure the style buttons are updated
    // to uncheck any style buttons check states.
    emit selectionChanged();
    // Going to assume that the user is allowed to click anywhere within or just after the block
    // Also makes assumptions about being well formed, or else crazy things may happen...
    int pos = textCursor().selectionStart();
    QString text = toPlainText();

    if (!IsPositionInBody(pos, text)) {
        // We are in an HTML file outside the body element. We might possibly be in an
        // inline CSS style so attempt to format that.
        if (!property_name.isEmpty()) {
            FormatCSSStyle(property_name, property_value);
        }

        return;
    }

    // We might have a selection that begins or ends in a tag < > itself
    if (IsPositionInTag(textCursor().selectionStart(), text) ||
        IsPositionInTag(textCursor().selectionEnd(), text)) {
        // Not allowed to toggle style if caret placed on a tag
        return;
    }

    QString tag_name;
    QRegularExpression tag_search(NEXT_TAG_LOCATION);
    QRegularExpression tag_name_search(TAG_NAME_SEARCH);
    bool in_existing_tag_occurrence = false;
    int previous_tag_index = -1;
    pos--;

    // Look backwards from the current selection to find whether we are in an open occurrence
    // of this tag already within this block.
    while (true) {
        previous_tag_index = text.lastIndexOf(tag_search, pos);

        if (previous_tag_index < 0) {
            return;
        }

        // We found a tag. It could be opening or closing.
        QRegularExpressionMatch mo = tag_name_search.match(text, previous_tag_index);
        int tag_name_index = mo.capturedStart();

        if (tag_name_index < 0) {
            pos = previous_tag_index - 1;
            continue;
        }

        tag_name = mo.captured(1).toLower();
        // Isolate whether it was opening or closing tag.
        bool is_closing_tag = false;

        if (tag_name.startsWith('/')) {
            is_closing_tag = true;
            tag_name = tag_name.right(tag_name.length() - 1);
        }

        // Is this tag the element we are looking for?
        if (element_name == tag_name) {
            if (!is_closing_tag) {
                in_existing_tag_occurrence = true;
            }

            break;
        } else {
            // Is this a block level tag?
            if (BLOCK_LEVEL_TAGS.contains(tag_name)) {
                // No point in searching any further - we reached the block parent
                // without finding an open occurrence of this tag.
                break;
            } else {
                // Not a tag of interest - keep searching.
                pos = previous_tag_index - 1;
                continue;
            }
        }
    }

    if (in_existing_tag_occurrence) {
        FormatSelectionWithinElement(element_name, previous_tag_index, text);
    } else {
        // Otherwise assume we are in a safe place to add a wrapper tag.
        InsertHTMLTagAroundSelection(element_name, "/" % element_name);
    }
}

void CodeViewEditor::FormatSelectionWithinElement(const QString &element_name, const int &previous_tag_index, const QString &text)
{
    // We are inside an existing occurrence. Are we immediately adjacent to it?
    // e.g. "<b>selected text</b>" should result in "selected text" losing the tags.
    // but  "<b>XXXselected textYYY</b> should result in "<b>XXX</b>selected text<b>YYY</b>"
    // plus the variations where XXX or YYY may be non-existent to make tag adjacent.
    int previous_tag_end_index = text.indexOf(">", previous_tag_index);
    QRegularExpression closing_tag_search("</\\s*" % element_name % "\\s*>", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch closing_tag_search_mo = closing_tag_search.match(text, previous_tag_index);
    int closing_tag_index = closing_tag_search_mo.capturedStart();

    if (closing_tag_index < 0) {
        // There is no closing tag for this style (not well formed). Give up.
        return;
    }

    QTextCursor cursor = textCursor();
    int selection_start = cursor.selectionStart();
    int selection_end = cursor.selectionEnd();
    bool adjacent_to_start = (previous_tag_end_index + 1) == selection_start;
    bool adjacent_to_end = closing_tag_index == selection_end;

    if (!adjacent_to_start && !adjacent_to_end) {
        // We want to put a closing tag at the start and an opening tag after (copying attributes)
        QString opening_tag_text = text.mid(previous_tag_index + 1, previous_tag_end_index - previous_tag_index - 1).trimmed();
        InsertHTMLTagAroundSelection("/" % element_name, opening_tag_text);
    } else if ((selection_end == selection_start) && (adjacent_to_start || adjacent_to_end) &&
               (closing_tag_index > previous_tag_end_index + 1)) {
        // User is just inside the opening or closing tag with no selection and there is text within
        // The tags. Nothing to do in this situation.
        // If there is no text e.g. <b></b> and caret between then we will toggle off as per following else.
        return;
    } else {
        QString opening_tag = text.mid(previous_tag_index, previous_tag_end_index - previous_tag_index + 1);
        QString closing_tag = text.mid(closing_tag_index, closing_tag_search_mo.capturedLength());
        QString replacement_text = cursor.selectedText();
        cursor.beginEditBlock();
        int new_selection_end = selection_end;
        int new_selection_start = selection_start;

        if (adjacent_to_start) {
            selection_start -= opening_tag.length();
            new_selection_start -= opening_tag.length();
            new_selection_end -= opening_tag.length();
        } else {
            replacement_text = closing_tag + replacement_text;
            new_selection_start += closing_tag.length();
            new_selection_end += closing_tag.length();
        }

        if (adjacent_to_end) {
            selection_end += closing_tag.length();
        } else {
            replacement_text.append(opening_tag);
        }

        cursor.setPosition(selection_end);
        cursor.setPosition(selection_start, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
        cursor.insertText(replacement_text);
        cursor.setPosition(new_selection_end);
        cursor.setPosition(new_selection_start, QTextCursor::KeepAnchor);
        cursor.endEditBlock();
        setTextCursor(cursor);
    }
}

void CodeViewEditor::ReplaceTags(const int &opening_tag_start, const int &opening_tag_end, const QString &opening_tag_text,
                                 const int &closing_tag_start, const int &closing_tag_end, const QString &closing_tag_text)
{
    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();
    // Replace the end block tag first since that does not affect positions
    cursor.setPosition(closing_tag_end);
    cursor.setPosition(closing_tag_start, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    cursor.insertText(closing_tag_text);
    // Now replace the opening block tag
    cursor.setPosition(opening_tag_end);
    cursor.setPosition(opening_tag_start, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    cursor.insertText(opening_tag_text);
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
    int tag_name_index = -1;
    int tag_name_search_len = 0;
    QString tag_name_search_cap1;
    QRegularExpression tag_name_search(TAG_NAME_SEARCH);
    QRegularExpressionMatchIterator i = tag_name_search.globalMatch(text);
    while (i.hasNext()) {
        QRegularExpressionMatch mo = i.next();
        int start = mo.capturedStart();
        if (start > pos) {
            break;
        }
        tag_name_index = start;
        tag_name_search_len = mo.capturedLength();
        tag_name_search_cap1 = mo.captured(1);
    }

    if (tag_name_index >= 0) {
        element.name = tag_name_search_cap1;
        int closing_tag_index = text.indexOf(QChar('>'), tag_name_index + tag_name_search_len);
        text = text.left(closing_tag_index);
        QRegularExpression style_names_search("\\s+class\\s*=\\s*\"([^\"]+)\"");
        QRegularExpressionMatch style_names_search_mo = style_names_search.match(text, tag_name_index + tag_name_search_len);
        int style_names_index = style_names_search_mo.capturedStart();

        if (style_names_index >= 0) {
            // We have one or more styles. Figure out which one the cursor is in if any.
            QString style_names_text = style_names_search_mo.captured(1);
            int styles_end_index = style_names_index + style_names_search_mo.capturedLength() - 1;
            int styles_start_index = text.indexOf(style_names_text, style_names_index);

            if ((pos >= styles_start_index) && (pos < styles_end_index)) {
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

            if (element.classStyle.isEmpty()) {
                // User has clicked outside of the style class names or somewhere strange - default to the first name.
                QStringList style_names = style_names_text.trimmed().split(' ');
                element.classStyle = style_names[0];
            }
        }
    }

    return element;
}

QString CodeViewEditor::GetAttributeId()
{
    int pos = textCursor().selectionStart();
    QString text = toPlainText();
    QString tag_name = GetOpeningTagName(pos, text);
    // If we're in an opening tag use it for the id, else use a
    QStringList tag_list = ID_TAGS;

    if (tag_name.isEmpty()) {
        tag_list = ANCHOR_TAGS;
    }

    return GetAttribute("id", tag_list, false, true);
}

QString CodeViewEditor::GetAttribute(const QString &attribute_name, QStringList tag_list, bool must_be_in_attribute, bool skip_paired_tags)
{
    return ProcessAttribute(attribute_name, tag_list, QString(), false, must_be_in_attribute, skip_paired_tags);
}


QString CodeViewEditor::SetAttribute(const QString &attribute_name, QStringList tag_list, const QString &attribute_value, bool must_be_in_attribute, bool skip_paired_tags)
{
    return ProcessAttribute(attribute_name, tag_list, attribute_value, true, must_be_in_attribute, skip_paired_tags);
}

QString CodeViewEditor::ProcessAttribute(const QString &attribute_name, QStringList tag_list, const QString &attribute_value, bool set_attribute, bool must_be_in_attribute, bool skip_paired_tags)
{
    if (attribute_name.isEmpty()) {
        return QString();
    }

    if (tag_list.count() == 0) {
        tag_list = BLOCK_LEVEL_TAGS;
    }

    // Given the code <p>abc</p>, users can click between first < and > and
    // one character to the left of the first <.
    // Makes assumptions about being well formed, or else crazy things may happen...
    int pos = textCursor().selectionStart();
    int original_position = textCursor().position();
    QString text = toPlainText();

    if (!IsPositionInBody(pos, text)) {
        return QString();
    }

    QString old_attribute_value;
    QString tag_name;
    QString opening_tag_text;
    QString opening_tag_attributes;
    QString attribute_text;
    int opening_tag_start = -1;
    int opening_tag_end = -1;
    int attribute_start = -1;
    int attribute_end = -1;
    int previous_tag_index = -1;
    int tag_name_index = -1;
    QRegularExpression tag_search(NEXT_TAG_LOCATION);
    QRegularExpression tag_name_search(TAG_NAME_SEARCH);
    QRegularExpression attribute_name_search(attribute_name % ATTRIBUTE_NAME_POSTFIX_SEARCH, QRegularExpression::CaseInsensitiveOption);
    QRegularExpression attrib_values_search(ATTRIB_VALUES_SEARCH);

    // If we're in a closing tag, move to the text between tags
    // just before/at < to make parsing easier.
    if (IsPositionInClosingTag(pos, text)) {
        while (pos > 0 && text[pos] != QChar('<')) {
            pos--;
        }
    }

    pos--;

    if (pos <= 0) {
        return QString();
    }

    QStringList pairs;

    // Search backwards for the next opening tag
    while (true) {
        QRegularExpressionMatchIterator i = tag_search.globalMatch(text);
        while (i.hasNext()) {
            QRegularExpressionMatch mo = i.next();
            int start = mo.capturedStart();
            if (start > pos) {
                break;
            }
            previous_tag_index = start;
        }

        if (previous_tag_index < 0) {
            return QString();
        }

        // We found a tag. It could be opening or closing.
        QRegularExpressionMatch tag_name_search_mo = tag_name_search.match(text, previous_tag_index);
        tag_name_index = tag_name_search_mo.capturedStart();

        if (tag_name_index < 0) {
            pos = previous_tag_index - 1;
            // If no name skip
            continue;
        }

        tag_name = tag_name_search_mo.captured(1).toLower();

        // Isolate whether it was opening or closing tag.
        if (tag_name.startsWith('/')) {
            tag_name = tag_name.right(tag_name.length() - 1);

            // If we're ignoring paired tags before us and this isn't a block tag
            // then save this tag for skipping later
            if (skip_paired_tags && !BLOCK_LEVEL_TAGS.contains(tag_name)) {
                pairs.prepend(tag_name);
                pos = previous_tag_index - 1;
                continue;
            }

            // If this is the closing tag of something we're checking then skip
            // since we are outside of a relevant tag pair.
            if (tag_list.contains(tag_name) || BLOCK_LEVEL_TAGS.contains(tag_name)) {
                break;
            }

            pos = previous_tag_index - 1;
            continue;
        }

        // We have an opening tag

        // If we are skipping tag pairs and this opening tag is partner to the last
        // found closed tag then keep searching
        if (skip_paired_tags && !pairs.isEmpty() && tag_name == pairs.first()) {
            pairs.removeFirst();
            pos = previous_tag_index - 1;
            continue;
        }

        // Is this one of the tags we want - if not keep searching
        if (!tag_list.contains(tag_name) && !BLOCK_LEVEL_TAGS.contains(tag_name)) {
            pos = previous_tag_index - 1;
            continue;
        }

        // If we reached a block level tag and still haven't matched a tag we need then abort.
        // And if we've reach body, then skip it.
        if (!tag_list.contains(tag_name) || tag_name.toLower() == "body") {
            break;
        }

        // If we got to here we know we have an opening tag that we can
        // apply the attribute to. Figure out boundaries of this tag.
        opening_tag_start = previous_tag_index;
        opening_tag_end = text.indexOf('>', opening_tag_start) + 1;
        // Now look for the attribute, which may or may not already exist
        QRegularExpressionMatch attribute_name_search_mo = attribute_name_search.match(text, opening_tag_start);
        attribute_start = attribute_name_search_mo.capturedStart();

        if ((attribute_start < 0) || (attribute_start >= opening_tag_end)) {
            // There is no attribute currently on this tag.
            attribute_start = opening_tag_end - 1;
            attribute_end = attribute_start;
        } else {
            // We have an existing attribute on this tag, need to parse it to rewrite it.
            attribute_start--;
            attribute_end = attribute_start + attribute_name_search_mo.capturedLength() + 1;
            QRegularExpressionMatch attrib_values_search_mo = attrib_values_search.match(text, attribute_start);
            old_attribute_value = attrib_values_search_mo.captured(1).trimmed();

            // If not in the attribute string return nothing
            if (must_be_in_attribute && (original_position <= attribute_start || original_position >= attribute_end)) {
                return "";
            }

            // If not setting the value, just return the found value
            if (!set_attribute) {
                return old_attribute_value;
            }
        }

        // If nothing to do, stop and don't move cursor if nothing to do
        if (attribute_start == attribute_end && attribute_value.isEmpty()) {
            break;
        }

        // Define the new attribute name/values to use.
        // Delete the attribute if no attribute value was given.
        if (!attribute_value.isNull()) {
            attribute_text = QString(" %1=\"%2\"").arg(attribute_name).arg(attribute_value);
        }

        // Now perform the replacement/insertion of the style attribute on this tag
        QTextCursor cursor = textCursor();
        cursor.beginEditBlock();
        cursor.setPosition(attribute_end);
        cursor.setPosition(attribute_start, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();

        if (!attribute_text.isEmpty()) {
            cursor.insertText(attribute_text);
        }

        // Now place the cursor at the end of this opening tag, taking into account difference in attributes.
        cursor.setPosition(opening_tag_end - (attribute_end - attribute_start) + attribute_text.length());
        cursor.endEditBlock();
        setTextCursor(cursor);
        return attribute_value;
    }

    textCursor().setPosition(original_position);
    return QString();
}

void CodeViewEditor::FormatStyle(const QString &property_name, const QString &property_value)
{
    if (property_name.isEmpty() || property_value.isEmpty()) {
        return;
    }

    // Emit a selection changed event, so we can make sure the style buttons are updated
    // to uncheck any buttons check states.
    emit selectionChanged();
    // Going to assume that the user is allowed to click anywhere within or just after the block
    // Also makes assumptions about being well formed, or else crazy things may happen...
    int pos = textCursor().selectionStart();
    QString text = toPlainText();

    if (!IsPositionInBody(pos, text)) {
        // Either we are in a CSS file, or we are in an HTML file outside the body element.
        // Treat both these cases as trying to find a CSS style on the current line
        FormatCSSStyle(property_name, property_value);
        return;
    }

    // Get the existing style attribute if there is one.
    QString style_attribute_value = GetAttribute("style");

    if (style_attribute_value.isNull()) {
        // There is no style attribute currently on this tag so just set it.
        style_attribute_value = QString("%1: %2;").arg(property_name).arg(property_value);
    } else {
        // We have an existing style attribute on this tag, need to parse it to rewrite it.
        // Apply the name=value replacement getting a list of our new property pairs
        QList<CSSInfo::CSSProperty *> css_properties = CSSInfo::getCSSProperties(style_attribute_value, 0, style_attribute_value.length());
        // Apply our property value, adding if not present currently, toggling if it is.
        ApplyChangeToProperties(css_properties, property_name, property_value);

        if (css_properties.count() == 0) {
            style_attribute_value = QString();
        } else {
            QStringList property_values;
            foreach(CSSInfo::CSSProperty * css_property, css_properties) {
                if (css_property->value.isNull()) {
                    property_values.append(css_property->name);
                } else {
                    property_values.append(QString("%1: %2").arg(css_property->name).arg(css_property->value));
                }
            }
            style_attribute_value = QString("%1;").arg(property_values.join("; "));
        }
    }

    // Apply the modified attribute.
    SetAttribute("style", BLOCK_LEVEL_TAGS, style_attribute_value);
}

void CodeViewEditor::FormatCSSStyle(const QString &property_name, const QString &property_value)
{
    if (property_name.isEmpty() || property_value.isEmpty()) {
        return;
    }

    // Emit a selection changed event, so we can make sure the style buttons are updated
    // to uncheck any buttons check states.
    emit selectionChanged();
    // Going to assume that the user is allowed to click anywhere within or just after the block
    // Also makes assumptions about being well formed, or else crazy things may happen...
    int pos = textCursor().selectionStart();
    QString text = toPlainText();
    // Valid places to apply this modification are either if:
    // (a) the caret is on a line somewhere inside CSS {} parenthesis.
    // (b) the caret is on a line declaring a CSS style
    const QTextBlock block = textCursor().block();
    int bracket_end = text.indexOf(QChar('}'), pos);

    if (bracket_end < 0) {
        return;
    }

    int bracket_start = text.lastIndexOf(QChar('{'), pos);

    if (bracket_start < 0 || text.lastIndexOf(QChar('}'), pos - 1) > bracket_start) {
        // The previous opening parenthesis we found belongs to another CSS style set
        // Look for another one after the current position on the same line.
        bracket_start = block.text().indexOf('{');

        if (bracket_start >= 0) {
            bracket_start += block.position();
        } else {
            // Some CSS stylesheets put the {} on their own line following the style name.
            // Look for a non-empty current line followed by a line starting with parenthesis
            const QTextBlock next_block = block.next();

            if (block.text().trimmed().isEmpty() || !next_block.isValid() ||
                !next_block.text().trimmed().startsWith(QChar('{'))) {
                return;
            }

            bracket_start = next_block.text().indexOf(QChar('{')) + next_block.position();
        }
    }

    if (bracket_start > bracket_end) {
        // Sanity check for some really weird bracketing (perhaps invalid css)
        return;
    }

    // Now parse the CSS style content
    QList<CSSInfo::CSSProperty *> css_properties = CSSInfo::getCSSProperties(text, bracket_start + 1, bracket_end);
    // Apply our property value, adding if not present currently, toggling if it is.
    ApplyChangeToProperties(css_properties, property_name, property_value);
    // Figure out the formatting to be applied to these style properties to write prettily
    // preserving any multi-line/single line style the CSS had before we changed things.
    bool is_single_line_format = (block.position() < bracket_start) && (bracket_end <= (block.position() + block.length()));
    const QString &style_attribute_text = CSSInfo::formatCSSProperties(css_properties, !is_single_line_format);
    // Now perform the replacement/insertion of the style properties into the CSS
    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();
    // Replace the end block tag first since that does not affect positions
    cursor.setPosition(bracket_end);
    cursor.setPosition(bracket_start + 1, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    cursor.insertText(style_attribute_text);
    cursor.setPosition(bracket_start);
    cursor.endEditBlock();
    setTextCursor(cursor);
}

void CodeViewEditor::ApplyChangeToProperties(QList<CSSInfo::CSSProperty *> &css_properties, const QString &property_name, const QString &property_value)
{
    // Apply our property value, adding if not present currently, toggling if it is.
    bool has_property = false;

    for (int i = css_properties.length() - 1; i >= 0; i--) {
        CSSInfo::CSSProperty *css_property = css_properties.at(i);

        if (css_property->name.toLower() == property_name) {
            has_property = true;

            // We will treat this as a toggle - if we already have the value then remove it
            if (css_property->value.toLower() == property_value) {
                css_properties.removeAt(i);
                continue;
            } else {
                css_property->value = property_value;
            }
        }
    }

    if (!has_property) {
        CSSInfo::CSSProperty *new_property = new CSSInfo::CSSProperty();
        new_property->name = property_name;
        new_property->value = property_value;
        css_properties.append(new_property);
    }
}

void CodeViewEditor::ReformatCSS(bool multiple_line_format)
{
    const QString &original_text = toPlainText();
    // Currently this feature is only enabled for CSS content, no inline HTML
    CSSInfo css_info(original_text, true);
    const QString &new_text = css_info.getReformattedCSSText(multiple_line_format);

    if (original_text != new_text) {
        QTextCursor cursor = textCursor();
        cursor.beginEditBlock();
        cursor.select(QTextCursor::Document);
        cursor.insertText(new_text);
        cursor.endEditBlock();
    }
}

void CodeViewEditor::ReformatHTML(bool all, bool to_valid)
{
    QString original_text;
    QString new_text;
    QWidget *mainWindow_w = Utility::GetMainWindow();
    MainWindow *mainWindow = dynamic_cast<MainWindow *>(mainWindow_w);
    if (!mainWindow) {
        Utility::DisplayStdErrorDialog("Could not determine main window.");
        return;
    }
    QString version = mainWindow->GetCurrentBook()->GetConstOPF()->GetEpubVersion();

    if (all) {
        mainWindow->GetCurrentBook()->ReformatAllHTML(to_valid);

    } else {
        original_text = toPlainText();

        if (to_valid) {
            new_text = CleanSource::Mend(original_text, version);
        } else {
            new_text = CleanSource::MendPrettify(original_text, version);
        }

        if (original_text != new_text) {
            QTextCursor cursor = textCursor();
            cursor.beginEditBlock();
            cursor.select(QTextCursor::Document);
            cursor.insertText(new_text);
            cursor.endEditBlock();
        }
    }
}

void CodeViewEditor::ApplyCaseChangeToSelection(const Utility::Casing &casing)
{
    QTextCursor cursor = textCursor();
    const QString selected_text = cursor.selectedText();

    if (selected_text.isEmpty()) {
        return;
    }

    // Do not allow user to try to capitalize where selection contains an html tag.
    if (selected_text.contains(QChar('<')) || selected_text.contains(QChar('>'))) {
        return;
    }

    const QString new_text = Utility::ChangeCase(selected_text, casing);

    if (new_text == selected_text) {
        return;
    }

    const int pos = cursor.selectionStart();
    cursor.beginEditBlock();
    cursor.removeSelectedText();
    cursor.insertText(new_text);
    cursor.setPosition(pos + new_text.length());
    cursor.setPosition(pos, QTextCursor::KeepAnchor);
    cursor.endEditBlock();
    setTextCursor(cursor);
}

QStringList CodeViewEditor::GetUnmatchedTagsForBlock(const int &start_pos, const QString &text) const
{
    // Given the specified position within the text, keep looking backwards finding
    // any tags until we hit all open block tags within the body. Append all the opening tags
    // that do not have closing tags together (ignoring self-closing tags)
    // and return the opening tags complete with their attributes contiguously.
    QStringList opening_tags;
    int closing_tag_count = 0;
    int pos = start_pos;
    QString tag_name;
    QRegularExpression tag_search(NEXT_TAG_LOCATION);
    QRegularExpression tag_name_search(TAG_NAME_SEARCH);
    pos--;

    while (true) {
        int previous_tag_index = -1;
        int tag_search_len = 0;
        QRegularExpressionMatchIterator i = tag_search.globalMatch(text);
        while (i.hasNext()) {
            QRegularExpressionMatch mo = i.next();
            int start = mo.capturedStart();
            if (start > pos) {
                break;
            }
            previous_tag_index = start;
            tag_search_len = mo.capturedLength();
        }

        if (previous_tag_index < 0) {
            break;
        }

        // We found a tag. Is it self-closing? If so, ignore it.
        const QString &full_tag_text = text.mid(previous_tag_index, tag_search_len);

        if (full_tag_text.endsWith("/>")) {
            pos = previous_tag_index - 1;
            continue;
        }

        // Is it valid with a name? If not, ignore it
        QRegularExpressionMatch tag_name_search_mo = tag_name_search.match(full_tag_text);
        int tag_name_index = tag_name_search_mo.capturedStart();

        if (tag_name_index < 0) {
            pos = previous_tag_index - 1;
            continue;
        }

        tag_name = tag_name_search_mo.captured(1).toLower();

        if (tag_name == "body") {
            break;
        }

        // Isolate whether it was opening or closing tag.
        if (tag_name.startsWith('/')) {
            tag_name = tag_name.right(tag_name.length() - 1);
            closing_tag_count++;
        } else {
            // Add the whole tag text to our opening tags if we haven't found a closing tag for it.
            if (closing_tag_count > 0) {
                closing_tag_count--;
            } else {
                opening_tags.insert(0, full_tag_text);
            }
        }

        pos = previous_tag_index - 1;
        continue;
    }

    if (opening_tags.count() > 0) {
        return opening_tags;
    }

    return QStringList();
}

bool CodeViewEditor::ReformatCSSEnabled()
{
    return m_reformatCSSEnabled;
}

void CodeViewEditor::SetReformatCSSEnabled(bool value)
{
    m_reformatCSSEnabled = value;
}

bool CodeViewEditor::ReformatHTMLEnabled()
{
    return m_reformatHTMLEnabled;
}

void CodeViewEditor::SetReformatHTMLEnabled(bool value)
{
    m_reformatHTMLEnabled = value;
}

void CodeViewEditor::SelectAndScrollIntoView(int start_position, int end_position, Searchable::Direction direction, bool wrapped)
{
    // We will scroll the position on screen if necessary in order to ensure that there is a block visible
    // before and after the text that will be selected by these positions.
    QTextBlock start_block = document()->findBlock(start_position);
    QTextBlock end_block = document()->findBlock(end_position);
    bool scroll_to_center = false;
    QTextCursor cursor = textCursor();

    if (wrapped) {
        // Set an initial cursor position at the top or bottom of the screen as appropriate
        if (direction == Searchable::Direction_Up) {
            cursor.movePosition(QTextCursor::End);
            setTextCursor(cursor);
        } else {
            cursor.movePosition(QTextCursor::Start);
            setTextCursor(cursor);
        }
    }

    if (direction == Searchable::Direction_Up || start_block.blockNumber() < end_block.blockNumber()) {
        QTextBlock first_visible_block = firstVisibleBlock();
        QTextBlock previous_block = start_block.previous();

        while (previous_block.blockNumber() > 0 && previous_block.text().isEmpty()) {
            previous_block = previous_block.previous();
        }

        if (!previous_block.isValid()) {
            previous_block = start_block;
        }

        if (previous_block.blockNumber() < first_visible_block.blockNumber()) {
            scroll_to_center = true;
        }
    }

    if (direction == Searchable::Direction_Down || start_block.blockNumber() < end_block.blockNumber()) {
        QTextBlock last_visible_block =  cursorForPosition(QPoint(viewport()->width(), viewport()->height())).block();
        QTextBlock next_block = end_block.next();

        while (next_block.blockNumber() > 0 && next_block.blockNumber() < blockCount() - 1 && next_block.text().isEmpty()) {
            next_block = next_block.next();
        }

        if (!next_block.isValid()) {
            next_block = end_block;
        }

        if (next_block.blockNumber() > last_visible_block.blockNumber()) {
            scroll_to_center = true;
        }
    }

    if (direction == Searchable::Direction_Up) {
        cursor.setPosition(end_position);
        cursor.setPosition(start_position, QTextCursor::KeepAnchor);
    } else {
        cursor.setPosition(start_position);
        cursor.setPosition(end_position, QTextCursor::KeepAnchor);
    }

    setTextCursor(cursor);

    if (scroll_to_center) {
        centerCursor();
    }
}

void CodeViewEditor::ConnectSignalsToSlots()
{
    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(UpdateLineNumberAreaMargin()));
    connect(this, SIGNAL(updateRequest(const QRect &, int)), this, SLOT(UpdateLineNumberArea(const QRect &, int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(HighlightCurrentLine()));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(EmitFilteredCursorMoved()));
    connect(this, SIGNAL(textChanged()), this, SIGNAL(PageUpdated()));
    connect(this, SIGNAL(textChanged()), this, SLOT(TextChangedFilter()));
    connect(this, SIGNAL(textChanged()), this, SLOT(updateLangMap()));
    connect(this, SIGNAL(undoAvailable(bool)), this, SLOT(UpdateUndoAvailable(bool)));
    connect(this, SIGNAL(selectionChanged()), this, SLOT(ResetLastFindMatch()));
    connect(m_ScrollOneLineUp,   SIGNAL(activated()), this, SLOT(ScrollOneLineUp()));
    connect(m_ScrollOneLineDown, SIGNAL(activated()), this, SLOT(ScrollOneLineDown()));
    connect(m_spellingMapper, SIGNAL(mapped(const QString &)), this, SLOT(InsertText(const QString &)));
    connect(m_addSpellingMapper, SIGNAL(mapped(const QString &)), this, SLOT(addToDefaultDictionary(const QString &)));
    connect(m_addDictMapper, SIGNAL(mapped(const QString &)), this, SLOT(addToUserDictionary(const QString &)));
    connect(m_ignoreSpellingMapper, SIGNAL(mapped(const QString &)), this, SLOT(ignoreWord(const QString &)));
    connect(m_clipMapper, SIGNAL(mapped(const QString &)), this, SLOT(PasteClipEntryFromName(const QString &)));
}

void CodeViewEditor::updateLangMap(){
    m_QSHParser->parseText(this->document()->toPlainText());
}

//DEBUG
void CodeViewEditor::AddPosInTxtContextMenu(QMenu *menu)
{
    QAction *topAction = 0;

    if (!menu->actions().isEmpty()) {
        topAction = menu->actions().at(0);
    }

    QString text = tr("Position in Text: ")+QString::number(textCursor().position());
    QAction *showPosition = new QAction(text, menu);

    if (!topAction) {
        menu->addAction(showPosition);
    } else {
        menu->insertAction(topAction, showPosition);
    }

    if (topAction) {
        menu->insertSeparator(topAction);
    }
}
