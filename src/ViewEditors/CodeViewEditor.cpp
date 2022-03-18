/************************************************************************
**
**  Copyright (C) 2019-2022 Doug Massay
**  Copyright (C) 2015-2022 Kevin B. Hendricks, Stratford Ontario Canada
**  Copyright (C) 2012      John Schember <john@nachtimwald.com>
**  Copyright (C) 2012-2013 Dave Heiland
**  Copyright (C) 2012      Grant Drake
**  Copyright (C) 2009-2011 Strahinja Markovic  <strahinja.markovic@gmail.com>, Nokia Corporation
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

#include <QFileInfo>
#include <QContextMenuEvent>
#include <QSignalMapper>
#include <QAction>
#include <QMenu>
#include <QPainter>
#include <QPalette>
#include <QColor>
#include <QScrollBar>
#include <QShortcut>
#include <QXmlStreamReader>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
#include <QString>
#include <QStringRef>
#include <QPointer>
#include <QApplication>
#include <QInputDialog>
#include <QTimer>
#include <QDebug>

#include "BookManipulation/Book.h"
#include "BookManipulation/CleanSource.h"
#include "BookManipulation/XhtmlDoc.h"
#include "MainUI/MainWindow.h"
#include "Parsers/GumboInterface.h"
#include "Parsers/CSSToolbox.h"
#include "Misc/XHTMLHighlighter2.h"
#include "Dialogs/ClipEditor.h"
#include "Misc/CSSHighlighter.h"
#include "Misc/SettingsStore.h"
#include "Misc/SpellCheck.h"
#include "Misc/HTMLSpellCheck.h"
#include "Misc/Utility.h"
#include "Parsers/HTMLStyleInfo.h"
#include "PCRE2/PCRECache.h"
#include "ViewEditors/CodeViewEditor.h"
#include "ViewEditors/LineNumberArea.h"
#include "sigil_constants.h"

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    #define QT_ENUM_SKIPEMPTYPARTS Qt::SkipEmptyParts
    #define QT_ENUM_KEEPEMPTYPARTS Qt::KeepEmptyParts
#else
    #define QT_ENUM_SKIPEMPTYPARTS QString::SkipEmptyParts
    #define QT_ENUM_KEEPEMPTYPARTS QString::KeepEmptyParts
#endif

const int PROGRESS_BAR_MINIMUM_DURATION = 1000;
const QString BREAK_TAG_INSERT    = "<hr class=\"sigil_split_marker\" />";

static const int TAB_SPACES_WIDTH        = 4;
static const int LINE_NUMBER_MARGIN      = 5;

static const QString XML_OPENING_TAG        = "(<[^>/][^>]*[^>/]>|<[^>/]>)";
static const QString NEXT_CLOSE_TAG_LOCATION = "</\\s*[^>]+>";
static const QString NEXT_TAG_LOCATION      = "<[^!>]+>";
static const QString TAG_NAME_SEARCH        = "<\\s*([^\\s>]+)";
static const QString STYLE_ATTRIBUTE_SEARCH = "style\\s*=\\s*\"[^\"]*\"";
static const QString ATTRIBUTE_NAME_POSTFIX_SEARCH = "\\s*=\\s*\"[^\"]*\"";
static const QString ATTRIB_VALUES_SEARCH   = "\"([^\"]*)";

static const QString OPEN_TAG_STARTS_SELECTION = "^\\s*(<\\s*([a-zA-Z0-9]+)[^>]*>)";
static const QString STARTING_INDENT_USED = "(^\\s*)[^\\s]";

static const uint MAX_SPELLING_SUGGESTIONS = 10;


CodeViewEditor::CodeViewEditor(HighlighterType high_type, bool check_spelling, QWidget *parent)
    :
    QPlainTextEdit(parent),
    m_isUndoAvailable(false),
    m_LastBlockCount(0),
    m_LineNumberAreaBlockNumber(-1),
    m_LineNumberArea(new LineNumberArea(this)),
    m_ScrollOneLineUp(new QShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_Up), this, 0, 0, Qt::WidgetShortcut)),
    m_ScrollOneLineDown(new QShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_Down), this, 0, 0, Qt::WidgetShortcut)),
    m_isLoadFinished(false),
    m_DelayedCursorScreenCenteringRequired(false),
    m_CaretUpdate(QList<ElementIndex>()),
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
    m_regen_taglist(true)
{
    if (high_type == CodeViewEditor::Highlight_XHTML) {
        // m_Highlighter = new XHTMLHighlighter(check_spelling, this);
        m_Highlighter = new XHTMLHighlighter2(check_spelling, this);
    } else if (high_type == CodeViewEditor::Highlight_CSS) {
        m_Highlighter = new CSSHighlighter(this);
    } else {
        m_Highlighter = NULL;
    }

    setFocusPolicy(Qt::StrongFocus);
    ConnectSignalsToSlots();
    SetAppearance();
}

CodeViewEditor::~CodeViewEditor()
{
    m_ScrollOneLineUp->deleteLater();
    m_ScrollOneLineDown->deleteLater();
}

void CodeViewEditor::SetAppearance()
{
    SettingsStore settings;
    if (Utility::IsDarkMode()) {
        // qDebug() << "IsDarkMode returned: true";
        m_codeViewAppearance = settings.codeViewDarkAppearance();
    } else {
        // qDebug() << "IsDarkMode returned: false";
        m_codeViewAppearance = settings.codeViewAppearance();
    }

    SetAppearanceColors();
    UpdateLineNumberAreaMargin();
    HighlightCurrentLine(false);
    setFrameStyle(QFrame::NoFrame);
    // Set the Zoom factor but be sure no signals are set because of this.
    m_CurrentZoomFactor = settings.zoomText();
    Zoom();
}


QSize CodeViewEditor::sizeHint() const
{
    return QSize(16777215, 16777215);
}


void CodeViewEditor::CustomSetDocument(TextDocument &ndocument)
{
    setDocument(&ndocument);
    ndocument.setModified(false);
    if (m_Highlighter) {
        m_Highlighter->setDocument(&ndocument);
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
    m_regen_taglist = true;
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
    HighlightCurrentLine(false);
    return marked;
}

void CodeViewEditor::HighlightMarkedText()
{
    QTextCharFormat format;

    QList<QTextEdit::ExtraSelection> extraSelections;
    QTextEdit::ExtraSelection selection;
    selection.cursor = textCursor();
    selection.format.setUnderlineStyle(QTextCharFormat::DotLine);
    selection.format.setFontUnderline(true);
    selection.cursor.clearSelection();
    selection.cursor.setPosition(0);
    int textlen = textLength();
    selection.cursor.setPosition(textlen);
    extraSelections.append(selection);
    setExtraSelections(extraSelections);
    extraSelections.clear();

    if (m_MarkedTextStart < 0 || m_MarkedTextEnd > textlen) {
        return;
    }
    selection.format.setUnderlineStyle(QTextCharFormat::DotLine);
    selection.format.setFontUnderline(true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();
    selection.cursor.setPosition(m_MarkedTextStart);
    selection.cursor.setPosition(m_MarkedTextEnd, QTextCursor::KeepAnchor);
    extraSelections.append(selection);
    setExtraSelections(extraSelections);
}


// mimic the impact of wrap around to search either above or below where the
// original search started in that file
bool CodeViewEditor::MoveToSplitText(Searchable::Direction direction, int start, int end)
{
    if (start < 0 || end <=0 || start >= end) {
        return false;
    }

    int pos = textCursor().position();
    int start_pos = textCursor().selectionStart();
    int end_pos = textCursor().selectionEnd();

    bool moved = false;
    if (direction == Searchable::Direction_Up) {
        if (end_pos > start && end_pos <= end) {
            return true;
        }
        if (pos <= start) {
            pos = end;
            moved = true;
        }
    } else {
        if (start_pos >= start && start_pos < end) {
            return true;
        }
        if (pos >= end) {
            pos = start;
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

void CodeViewEditor::CutTagPair()
{
    // cursor must be in a tag, with no selection active at all
    if (!IsCutTagPairAllowed()) return;

    QTextCursor cursor = textCursor();
    int pos = cursor.selectionStart();
    int newpos = pos;
    int open_tag_pos = -1;
    int open_tag_len = -1;
    int close_tag_pos = -1;
    int close_tag_len = -1;
    int i = m_TagList.findFirstTagOnOrAfter(pos);
    TagLister::TagInfo ti = m_TagList.at(i);
    if ((pos >= ti.pos) && (pos < ti.pos + ti.len)) {
        // removing the body or html tags freaks out QWebEnginePage in Preview
        if (ti.tname == "body" || ti.tname == "html") return;
        if(ti.ttype == "end") {
            newpos = ti.pos - ti.open_len;
            open_tag_pos = ti.open_pos;
            open_tag_len = ti.open_len;
            close_tag_pos = ti.pos;
            close_tag_len = ti.len;
        } else { // all others single, begin, xmlheader, doctype, comment, etc
             newpos = ti.pos;
             open_tag_pos = ti.pos;
             open_tag_len = ti.len;
        }
        if (ti.ttype == "begin") {
            int j = m_TagList.findCloseTagForOpen(i);
            if (j >= 0) {
                if (m_TagList.at(j).len != -1) {
                    close_tag_pos = m_TagList.at(j).pos;
                    close_tag_len = m_TagList.at(j).len;
                }
            }
        }
    }
    if (open_tag_len != -1 || close_tag_len != -1) {
        // handle close tag removal first to not mess up position info
        cursor.beginEditBlock();
        if (close_tag_len != -1) {
            cursor.setPosition(close_tag_pos + close_tag_len);
            cursor.setPosition(close_tag_pos, QTextCursor::KeepAnchor);
            cursor.removeSelectedText();
        }
        if (open_tag_len != -1) {
            cursor.setPosition(open_tag_pos + open_tag_len);
            cursor.setPosition(open_tag_pos, QTextCursor::KeepAnchor);
            cursor.removeSelectedText();
        }
        cursor.endEditBlock();
        cursor.setPosition(newpos);
        setTextCursor(cursor);
    }
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

    MaybeRegenerateTagList();
    QString text = m_TagList.getSource();
    int pos = textCursor().selectionStart();
    int end = textCursor().selectionEnd()-1;

    if ((text[pos] == '<') && (text[end] == '>')) return true;
    
    if (IsPositionInTag(pos) || IsPositionInTag(end)) {
        return false;
    }

    return true;
}

bool CodeViewEditor::IsCutCodeTagsAllowed()
{
    return TextIsSelectedAndNotInStartOrEndTag();
}

bool CodeViewEditor::IsCutTagPairAllowed()
{
    if (textCursor().hasSelection()) return false;
    return IsPositionInTag(textCursor().selectionStart());
}

bool CodeViewEditor::IsInsertClosingTagAllowed()
{
    int pos = textCursor().selectionStart();
    if (IsPositionInTag(pos)) {
        // special case of cursor |<tag>
        QString text = m_TagList.getSource();
        if (text[pos] != '<') return false;
    }
    return true;
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
    int split_position = textCursor().position();

    MaybeRegenerateTagList();
    QString text = m_TagList.getSource();
    
    // Abort splitting the section if user is within a tag - MainWindow will display a status message
    if (IsPositionInTag(split_position)) {
        // exempt the case of cursor |<tag>
        if (text[split_position]!= '<') return QString();
    }

    // abort if no body tags exist
    int bo = m_TagList.findBodyOpenTag();
    int bc = m_TagList.findBodyCloseTag();
    if (bo == -1 || bc == -1) return QString();
    
    int body_tag_start = m_TagList.at(bo).pos;
    int body_tag_end   = body_tag_start + m_TagList.at(bo).len;
    int body_contents_end = m_TagList.at(bc).pos;
    QString head = text.left(body_tag_start);

    if (split_position < body_tag_end) {
        // Cursor is before the start of the body
        split_position = body_tag_end;
    }
    if (split_position > body_contents_end) {
        // Cursor is after or in the closing body tag
        split_position = body_contents_end;
    }
    
    const QStringList &opening_tags = GetUnmatchedTagsForBlock(split_position);
 
    QString text_segment = "<p>&#160;</p>";
    if (split_position != body_tag_end) {
        text_segment = Utility::Substring(body_tag_start, split_position, text);
    }
    
    // This splits off from contents of body from top to the split position
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

    // We identify any open tags for the current caret position, and repeat
    // those at the caret position to ensure we have valid html 
    if (!opening_tags.isEmpty()) {
        cursor.insertText(opening_tags.join(""));
    }

    cursor.endEditBlock();

    QString new_section = head + text_segment + "\n</body>\n</html>";
    return new_section;
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

    int pos = textCursor().position(); // was -1 but that is not right
    const QStringList unmatched_tags = GetUnmatchedTagsForBlock(pos);

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

#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    return LINE_NUMBER_MARGIN * 2 + fontMetrics().horizontalAdvance(QChar('0')) * num_digits;
#else
    return LINE_NUMBER_MARGIN * 2 + fontMetrics().width(QChar('0')) * num_digits;
#endif

}


void CodeViewEditor::ReplaceDocumentText(const QString &new_text)
{
    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();
    cursor.select(QTextCursor::Document);
    cursor.removeSelectedText();
    cursor.insertText(new_text);
    cursor.endEditBlock();
    m_regen_taglist = true; // just in case
}


void CodeViewEditor::ScrollToTop()
{
    verticalScrollBar()->setValue(0);
}

// Note: using
//     QTextCursor cursor(document()); 
// instead of:
//     QTextCursor cursor = textCursor();
// creates a new text coursor that points to the document top
// and so loses the state of the current textCursor if one exists
// and this includes any existing highlighting asscoiated with it

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

// return the length in QChars in plain text inside
int CodeViewEditor::textLength() const
{
    TextDocument * doc = qobject_cast<TextDocument *> (document());
    return doc->textLength();
}

// overrides document toPlainText to prevent loss of nbsp
QString CodeViewEditor::toPlainText() const
{
    if (!m_isLoadFinished) return QString();
    TextDocument * doc = qobject_cast<TextDocument *> (document());
    return doc->toText();
}

// overrides createMimeDataFromSelection()
QMimeData *CodeViewEditor::createMimeDataFromSelection() const
{ 
  QString selected_text = textCursor().selectedText();
  selected_text = selected_text.replace(QChar::ParagraphSeparator, '\n');
  QMimeData* md = new QMimeData();
  md->setText(selected_text);
  return md;
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
                              bool marked_text,
                              int split_at)
{
    SPCRE *spcre = PCRECache::instance()->getObject(search_regex);
    SPCRE::MatchInfo match_info;
    QString txt = toPlainText();
    int start_offset = 0;
    int start = 0;
    int end = txt.length();

    if (marked_text) {
        if (!MoveToMarkedText(search_direction, wrap)) {
            return false;
        }
        start = m_MarkedTextStart;
        end = m_MarkedTextEnd;
        start_offset = m_MarkedTextStart;
    }

    // treat split_at as if it was the marked region to mimic
    // the impact of wrap around, split will always be -1 if
    // marked text is being used and in all Current File Mode searches
    if (split_at != -1) {
        if (search_direction == Searchable::Direction_Up) {
            start = split_at;
        } else {
            end = split_at;
        }
        start_offset = start;
        if (!MoveToSplitText(search_direction, start, end)) {
            return false;
        }
    }

    int selection_offset = GetSelectionOffset(search_direction, ignore_selection_offset, marked_text);

    if (search_direction == Searchable::Direction_Up) {
        if (misspelled_words) {
            match_info = GetMisspelledWord(txt, 0, selection_offset, search_regex, search_direction);
        } else {
            match_info = spcre->getLastMatchInfo(Utility::Substring(start, selection_offset, txt));
        }
    } else {
        if (misspelled_words) {
            match_info = GetMisspelledWord(txt, selection_offset, txt.count(), search_regex, search_direction);
        } else {
            match_info = spcre->getFirstMatchInfo(Utility::Substring(selection_offset, end, txt));
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

    if (split_at != -1) {
        // If not in split range it's not a real match.
        if (match_info.offset.second + start_offset > end ||
            match_info.offset.first + start_offset < start) {
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
    QString text= toPlainText();
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
    int original_text_length = textLength();
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
            m_MarkedTextEnd += textLength() - original_text_length;
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
            } else { 
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


void CodeViewEditor::mouseDoubleClickEvent(QMouseEvent *event)
{
    // Propagate to base class first then handle locally
    QPlainTextEdit::mouseDoubleClickEvent(event);

    // record the initial position in case later changed by doubleclick event
    QTextCursor cursor = textCursor();
    int pos = cursor.selectionStart();
    bool isShift = QApplication::keyboardModifiers() & Qt::ShiftModifier;
    bool isAlt = QApplication::keyboardModifiers() & Qt::AltModifier;
    // qDebug() << "Modifiers: " << QApplication::keyboardModifiers();

    if (!isShift && !isAlt) return;

    if (!IsPositionInTag(pos)){
        return;
    }

    // if Shift is used select just the tag's contents, but not the tag itself
    // if Alt (option key on macOS) is used select the tag's contents and that tag itself
    int open_tag_pos = -1;
    int open_tag_len = -1;
    int close_tag_pos = -1;
    int close_tag_len = -1;
    int i = m_TagList.findFirstTagOnOrAfter(pos);
    TagLister::TagInfo ti = m_TagList.at(i);
    if ((pos >= ti.pos) && (pos < ti.pos + ti.len)) {
        if(ti.ttype == "end") {
            open_tag_pos = ti.open_pos;
            open_tag_len = ti.open_len;
            close_tag_pos = ti.pos;
            close_tag_len = ti.len;
        } else { // all others single, begin, doctype, xmlheader, cdata, etc
             open_tag_pos = ti.pos;
             open_tag_len = ti.len;
        }
        if (ti.ttype == "begin") {
            int j = m_TagList.findCloseTagForOpen(i);
            if (j >= 0) {
                if (m_TagList.at(j).len != -1) {
                    close_tag_pos = m_TagList.at(j).pos;
                    close_tag_len = m_TagList.at(j).len;
                }
            }
        }
    }
    if (open_tag_len != -1 || close_tag_len != -1) {
        int selstart;
        int selend;
        if (isShift) {
            selstart = open_tag_pos + open_tag_len;
            selend = selstart;
            if (close_tag_len != -1) selend = close_tag_pos;
        } else {
            selstart = open_tag_pos;
            selend = open_tag_pos + open_tag_len;
            if (close_tag_len != -1) selend = close_tag_pos + close_tag_len;
        }
        cursor.setPosition(selstart);
        cursor.setPosition(selend, QTextCursor::KeepAnchor);
        setTextCursor(cursor);
    }
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
    // Need to use QPointer to prevent crashes on macOS when closing 
    // parent during qmenu exec.  See discussion at:
    // https://www.qtcentre.org/threads/65046-closing-parent-widget-during-QMenu-exec()
    QPointer<QMenu> menu = createStandardContextMenu();
    
    if (m_reformatCSSEnabled) {
        AddReformatCSSContextMenu(menu);
        AddCSSClassContextMenu(menu);
    }

    if (m_reformatHTMLEnabled) {
        AddReformatHTMLContextMenu(menu);
    }

    AddMarkSelectionMenu(menu);
    AddGoToLinkOrStyleContextMenu(menu);
    AddToggleLineWrapModeContextMenu(menu);
    AddClipContextMenu(menu);

    if (m_checkSpelling) {
        AddSpellCheckContextMenu(menu);
    }

    if (InViewableImage()) {
        AddViewImageContextMenu(menu);
    }

    menu->exec(event->globalPos());
    if (!menu.isNull()) {
        delete menu.data();
    }
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
            QStringList suggestions = sc->suggestPS(selected_word);
            QAction *suggestAction = 0;

            // We want to limit the number of suggestions so we don't
            // get a huge context menu.
            for (int i = 0; i < std::min(static_cast<uint>(suggestions.length()), MAX_SPELLING_SUGGESTIONS); ++i) {
                suggestAction = new QAction(suggestions.at(i), menu);
                connect(suggestAction, SIGNAL(triggered()), m_spellingMapper, SLOT(map()));
                m_spellingMapper->setMapping(suggestAction, suggestions.at(i));

                // If the menu is empty we need to append rather than insert our actions.
                if (!topAction) {
                    menu->addAction(suggestAction);
                } else {
                    menu->insertAction(topAction, suggestAction);
                }
            }

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
            QMenu *dictionary_menu = new QMenu(menu);
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

    // See if we are close to or inside of a misspelled word. If so select it.
    if (!c.hasSelection()) {
        // We cannot use QTextCursor::charFormat because the format is not set directly in
        // the document. The QSyntaxHighlighter sets the format in the block layout's
        // additionalFormats property. Thus we have to check if the cursor is within
        // an additionalFormat for the block and if that format is for a misspelled word.
        int pos = c.positionInBlock();
        foreach(QTextLayout::FormatRange r, textCursor().block().layout()->formats()) {
            if (pos > r.start && pos < r.start + r.length && r.format.underlineStyle() == QTextCharFormat::WaveUnderline/*QTextCharFormat::SpellCheckUnderline*/) {
                if (select_word) {
                    c.setPosition(c.block().position() + r.start);
                    c.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, r.length);
                    setTextCursor(c);
                    return c.selectedText();
                } else {
                    return toPlainText().mid(c.block().position() + r.start, r.length);
                }
            }
        }
    }
    // Check if our selection is a misspelled word.
    else {
        int selStart = c.selectionStart() - c.block().position();
        int selLen = c.selectionEnd() - c.block().position() - selStart;
        foreach(QTextLayout::FormatRange r, textCursor().block().layout()->formats()) {
            if (r.start == selStart && selLen == r.length && r.format.underlineStyle() == QTextCharFormat::WaveUnderline/*QTextCharFormat::SpellCheckUnderline*/) {
                return c.selectedText();
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


void CodeViewEditor::AddCSSClassContextMenu(QMenu *menu)
{
    QAction *topAction = 0;

    if (!menu->actions().isEmpty()) {
        topAction = menu->actions().at(0);
    }

    QString text = tr("Rename Selected Class");
    QAction *renameClassAction = new QAction(text, menu);

    if (textCursor().hasSelection() && textCursor().selectedText().startsWith(".")) {
        if (!topAction) {
            menu->addAction(renameClassAction);
        } else {
	       menu->insertAction(topAction, renameClassAction);
        }
    }
    connect(renameClassAction, SIGNAL(triggered()), this, SLOT(RenameClassClicked()));
}


void CodeViewEditor::AddReformatHTMLContextMenu(QMenu *menu)
{
    QAction *topAction = 0;

    if (!menu->actions().isEmpty()) {
        topAction = menu->actions().at(0);
    }

    QMenu *reformatMenu = new QMenu(tr("Reformat HTML"), menu);

    QAction *cleanAction = new QAction(tr("Mend and Prettify Code"), reformatMenu);
    QAction *cleanAllAction = new QAction(tr("Mend and Prettify Code - All HTML Files"), reformatMenu);
    QAction *toValidAction = new QAction(tr("Mend Code"), reformatMenu);
    QAction *toValidAllAction = new QAction(tr("Mend Code - All HTML Files"), reformatMenu);
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

void CodeViewEditor::AddToggleLineWrapModeContextMenu(QMenu *menu)
{
    QAction *topAction = 0;

    if (!menu->actions().isEmpty()) {
        topAction = menu->actions().at(0);
    }

    QAction *toggleLineWrapModeAction = new QAction(tr("Toggle Line Wrap Mode"), menu);
    if (!topAction) {
        menu->addAction(toggleLineWrapModeAction);
    } else {
        menu->insertAction(topAction, toggleLineWrapModeAction);
    }

    connect(toggleLineWrapModeAction, SIGNAL(triggered()), this, SLOT(ToggleLineWrapMode()));

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

    QMenu *clips_menu = new QMenu(menu);
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
            clipAction = new QAction(item->text(), parent_menu);
            connect(clipAction, SIGNAL(triggered()), m_clipMapper, SLOT(map()));
            m_clipMapper->setMapping(clipAction, ClipEditorModel::instance()->GetFullName(item));

            if (!topAction) {
                parent_menu->addAction(clipAction);
            } else {
                parent_menu->insertAction(topAction, clipAction);
            }
        } else {
            group_menu = new QMenu(parent_menu);
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
    QString url_name = GetAttribute("src", SRC_TAGS, true);

    if (url_name.isEmpty()) {
        // We do not know what namespace may have been used
        url_name = GetAttribute("xlink:href", IMAGE_TAGS, true);
    }

    return !url_name.isEmpty();
}

void CodeViewEditor::OpenImageAction()
{
    QString url_name = GetAttribute("src", SRC_TAGS, true);

    if (url_name.isEmpty()) {
        // We do not know what namespace may have been used
        url_name = GetAttribute("xlink:href", IMAGE_TAGS, true);
    }

    emit LinkClicked(QUrl(url_name));
}

void CodeViewEditor::GoToLinkOrStyleAction()
{
    GoToLinkOrStyle();
}

void CodeViewEditor::GoToLinkOrStyle()
{
    QString url_name = GetAttribute("href", ANCHOR_TAGS, true);
    
    if (url_name.isEmpty()) {
        QStringList LINK_TAGS = QStringList() << "link";
        url_name = GetAttribute("href", LINK_TAGS, true, false, false);
    }
    
    if (url_name.isEmpty()) {
        url_name = GetAttribute("src", SRC_TAGS, true);
    }

    if (url_name.isEmpty()) {
        // We do not know what namespace may have been used
        url_name = GetAttribute("xlink:href", IMAGE_TAGS, true);
    }

    if (!url_name.isEmpty()) {
        
        QUrl url = QUrl(url_name);
        QString extension = url_name.right(url_name.length() - url_name.lastIndexOf('.') - 1).toLower();

        if (IMAGE_EXTENSIONS.contains(extension)) {
            emit ViewImage(QUrl(url_name));
        } else {
            emit LinkClicked(QUrl(url_name));
        }
    } else if (IsPositionInOpeningTag(textCursor().selectionStart())) {
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

    HTMLStyleInfo htmlcss_info(toPlainText());
    CSSInfo::CSSSelector *selector = htmlcss_info.getCSSSelectorForElementClass(element.name, element.classStyle);

    if (!selector) {
        // We didn't find the style - escalate as an event to look in linked stylesheets
        emit GoToLinkedStyleDefinitionRequest(element.name, element.classStyle);
    } else {
        // Emit a signal to bookmark our code location, enabling the "Back to" feature
        emit BookmarkLinkOrStyleLocationRequest();
        // Scroll to the line after bookmarking or we lose our place
        ScrollToPosition(selector->pos);
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

void CodeViewEditor::RenameClassClicked()
{
    QTimer::singleShot(20,this,SLOT(RenameClass()));
}

void CodeViewEditor::RenameClass()
{
    QWidget *mainWindow_w = Utility::GetMainWindow();
    MainWindow *mainWindow = qobject_cast<MainWindow *>(mainWindow_w);
    if (!mainWindow) {
	    Utility::DisplayStdErrorDialog("Could not determine main window.");
        return;
    }

    int lineno = textCursor().blockNumber() + 1;

    QString aclassname = textCursor().selectedText();
    if (aclassname.startsWith('.')) aclassname = aclassname.mid(1,-1);
    QString cssdata = toPlainText();
    CSSToolbox tb;
    QSet<QString> classnames = tb.generate_class_set(cssdata);
    if (aclassname.isEmpty() || !classnames.contains(aclassname)) {
        emit ShowStatusMessageRequest(tr("Selected Text is not a valid class name."));
        return;
    }

    // get the new name
    QString newname;
    QInputDialog dinput;
    dinput.setWindowTitle(tr("Rename Class"));
    dinput.setLabelText(tr("Enter new class name"));
    if (dinput.exec()) {
        newname = dinput.textValue().trimmed();
    }
    if (newname.startsWith('.')) newname = newname.mid(1, -1);
    if (newname.isEmpty()) return;

    // rename the class in all xhtml files that use this css file
    // MainWindow can get current tab to get css resource currently loaded
    if (mainWindow->RenameClassInHtml(aclassname, newname)) {
        // rename the class in this css file
        QString newcssdata = tb.rename_class_in_css(aclassname, newname, cssdata);
        if (cssdata != newcssdata) {
            QTextCursor cursor = textCursor();
            cursor.beginEditBlock();
            cursor.select(QTextCursor::Document);
            cursor.insertText(newcssdata);
            cursor.endEditBlock();
            ScrollToLine(lineno);
        }
        emit ShowStatusMessageRequest(tr("Class renamed."));
    } else {
        emit ShowStatusMessageRequest(tr("Class rename aborted."));
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

    if (!IsPositionInBody(pos)) {
        return false;
    }

    // Only allow if the closing tag we're in is an "a" tag
    QString closing_tag_name = GetClosingTagName(pos);
    // special case of cursor immediately before ending tag |</tag>
    if (!closing_tag_name.isEmpty()) {
        if (m_TagList.getSource().at(pos) == '<') {
            closing_tag_name = "";
        }
    }
    if (!closing_tag_name.isEmpty() && !ANCHOR_TAGS.contains(closing_tag_name)) {
        return false;
    }

    // Only allow if the opening tag we're in is valid for id attribute
    QString tag_name = GetOpeningTagName(pos);

    if (!tag_name.isEmpty() && !ID_TAGS.contains(tag_name)) {
        return false;
    }

    return true;
}

bool CodeViewEditor::IsInsertHyperlinkAllowed()
{
    int pos = textCursor().selectionStart();

    if (!IsPositionInBody(pos)) {
        return false;
    }

    // Only allow if the closing tag we're in is an "a" tag
    QString closing_tag_name = GetClosingTagName(pos);
    // special case of cursor immediately before ending tag |</tag>
    if (!closing_tag_name.isEmpty()) {
        if (m_TagList.getSource().at(pos) == '<') {
            closing_tag_name = "";
        }
    }

    if (!closing_tag_name.isEmpty() && !ANCHOR_TAGS.contains(closing_tag_name)) {
        return false;
    }

    // Only allow if the opening tag we're in is an "a" tag
    QString tag_name = GetOpeningTagName(pos);

    if (!tag_name.isEmpty() && !ANCHOR_TAGS.contains(tag_name)) {
        return false;
    }

    return true;
}

bool CodeViewEditor::IsInsertFileAllowed()
{
    int pos = textCursor().selectionStart();
    if (!IsPositionInBody(pos)) return false;
    if (IsPositionInTag(pos)) {
        // special case of cursor |<tag>
        QString text = m_TagList.getSource();
        if (text[pos] != '<') return false;
    }
    return true;
}

bool CodeViewEditor::InsertId(const QString &attribute_value)
{
    int pos = textCursor().selectionStart();
    const QString &element_name = "a";
    const QString &attribute_name = "id";
    // If we're in an "a" tag we can update the id even if not in the opening tag
    QStringList tag_list = ID_TAGS;

    if (GetOpeningTagName(pos).isEmpty()) {
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

bool CodeViewEditor::InsertTagAttribute(const QString &element_name,
                                        const QString &attribute_name,
                                        const QString &attribute_value,
                                        const QStringList &tag_list,
                                        bool ignore_selection)
{
    bool inserted = false;

    // Add or update the attribute within the start tag and return if ok
    if (!SetAttribute(attribute_name, tag_list, attribute_value, false, true).isEmpty()) {
        return true;
    }

    // If nothing was inserted, then just insert a new tag with no text as long as we aren't in a tag
    int pos = textCursor().position();
    bool in_tag_not_before = IsPositionInTag(pos) && (m_TagList.getSource().at(pos) != '<');
    if (!textCursor().hasSelection() && !in_tag_not_before) {
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

    // first see if an achor is the immediate parent of selected text
    // and if so get any existing attribute class values to append
    // so we do not overwrite them
    QString existing_class = GetAttribute("class",ANCHOR_TAGS, false);
    QString new_class = SIGIL_INDEX_CLASS;
    if (!existing_class.isEmpty()) new_class = new_class + " " + existing_class;

    if (!InsertTagAttribute(element_name, attribute_name, new_class, ANCHOR_TAGS)) {
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
    // Why is this needed?
    // RehighlightDocument();
    emit FocusGained(this);
    QPlainTextEdit::focusInEvent(event);
    HighlightCurrentLine(false);
}


// Overridden so we can emit the FocusLost() signal.
void CodeViewEditor::focusOutEvent(QFocusEvent *event)
{
    emit FocusLost(this);
    QPlainTextEdit::focusOutEvent(event);
    HighlightCurrentLine(false);
}

void CodeViewEditor::EmitFilteredCursorMoved()
{
    // Avoid slowdown while selecting text
    if (QApplication::mouseButtons() == Qt::NoButton) {
        if (m_isLoadFinished) {
            emit FilteredCursorMoved();
        }
    }
}

void CodeViewEditor::TextChangedFilter()
{
    m_regen_taglist = true;

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

    // Is this needed,  Why not let it work asynchronously
    if (m_Highlighter) {
        // We block signals from the document while highlighting takes place,
        // because we do not want the contentsChanged() signal to be fired
        // which would mark the underlying resource as needing saving.
        XHTMLHighlighter2* xhl = qobject_cast<XHTMLHighlighter2*>(m_Highlighter);
        // XHTMLHighlighter* xhl = qobject_cast<XHTMLHighlighter*>(m_Highlighter);
        CSSHighlighter* chl = qobject_cast<CSSHighlighter*>(m_Highlighter);
        document()->blockSignals(true);
        if (xhl) {
            xhl->do_rehighlight();
        } else if (chl) {
            chl->do_rehighlight();
        }
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


void CodeViewEditor::MaybeRegenerateTagList()
{
    // calling toPlainText before the initial load is finished causes
    // a segfault deep inside QTextDocument
    // if (!m_isLoadFinished) return;
    
    if (m_regen_taglist) {
        // qDebug() << "regenerating tag list";
        m_TagList.reloadLister(toPlainText());
        m_regen_taglist = false;
    }
}


void CodeViewEditor::HighlightCurrentLine(bool highlight_tags)
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    SettingsStore settings;

    // Draw the full width line color.
    QTextEdit::ExtraSelection selection_line;
    if (hasFocus()) {
        selection_line.format.setBackground(m_codeViewAppearance.line_highlight_color);
    } else {
        selection_line.format.setBackground(m_codeViewAppearance.line_number_background_color);
    }
    selection_line.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection_line.cursor = textCursor();
    selection_line.cursor.clearSelection();
    extraSelections.append(selection_line);

    if (highlight_tags && settings.highlightOpenCloseTags()) {

        // If and only if cursor is inside a tag, highlight open and matching close
        // current cursor position is just before this char at position pos in text
        QString text = toPlainText();
        int pos = textCursor().selectionStart();

        // find previous begin tag marker
        int pb = text.lastIndexOf('<', pos);

        // find next end tag marker
        int ne = text.indexOf('>', pos);

        // find next begin tag marker *after* this char
        // and handle case if missing
        int nb = text.indexOf('<', pos+1);   
        if (nb == -1) nb = text.length()+1;

        // in tag if '<' is closer than '>' when search backwards
        // and if '>' is closer but than '<' (if it exists) but >= pos  when search forward
        if ((pb > text.lastIndexOf('>', pos-1)) && (ne >= pos) && (nb > ne)) {

            // if text has changed since last time regenerate the tag list
            MaybeRegenerateTagList();

            // in a tag
            int open_tag_pos = -1;
            int open_tag_len = -1;
            int close_tag_pos = -1;
            int close_tag_len = -1;
            int i = m_TagList.findFirstTagOnOrAfter(pos);
            TagLister::TagInfo ti = m_TagList.at(i);
            if ((pos >= ti.pos) && (pos < ti.pos + ti.len)) {
                if(ti.ttype == "end") {
                    open_tag_pos = ti.open_pos;
                    open_tag_len = ti.open_len;
                    close_tag_pos = ti.pos;
                    close_tag_len = ti.len;
                } else { // all others: single, begin, xmlheader, doctype, comment, cdata, etc 
                    open_tag_pos = ti.pos;
                    open_tag_len = ti.len;
                }
                if (ti.ttype == "begin") {
                    int j = m_TagList.findCloseTagForOpen(i);
                    if (j >= 0) {
                        if (m_TagList.at(j).len != -1) {
                            close_tag_pos = m_TagList.at(j).pos;
                            close_tag_len = m_TagList.at(j).len;
                        }
                    }
                }
            }
            if (open_tag_len != -1) {
                QTextEdit::ExtraSelection selection_open;
                selection_open.format.setBackground(m_codeViewAppearance.line_number_background_color);
                selection_open.cursor = textCursor();
                selection_open.cursor.clearSelection();
                selection_open.cursor.setPosition(open_tag_pos);
                selection_open.cursor.setPosition(open_tag_pos + open_tag_len, QTextCursor::KeepAnchor);
                extraSelections.append(selection_open);
            }
            if (close_tag_len != -1) {
                QTextEdit::ExtraSelection selection_close;
                selection_close.format.setBackground(m_codeViewAppearance.line_number_background_color);
                selection_close.cursor = textCursor();
                selection_close.cursor.clearSelection();
                selection_close.cursor.setPosition(close_tag_pos);
                selection_close.cursor.setPosition(close_tag_pos + close_tag_len, QTextCursor::KeepAnchor);
                extraSelections.append(selection_close);
            } 
        }
    }

    // Add highlighting of the marked text
    if (IsMarkedText()) {
        if (m_MarkedTextEnd > textLength()) {
            m_MarkedTextEnd = textLength();
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
    // still want to force a UI update in mainWindow but do not want to reset
    // any last match info as that would invalidate search settings
    // so disconnect this locally,  emit,  and then reconnect
    disconnect(this, SIGNAL(selectionChanged()), this, SLOT(ResetLastFindMatch()));
    emit selectionChanged();
    connect(this, SIGNAL(selectionChanged()), this, SLOT(ResetLastFindMatch()));
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
        bool res = PasteClipEntry(clip);
        applied = applied | res;
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
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    setTabStopDistance(TAB_SPACES_WIDTH * QFontMetrics(font).horizontalAdvance(' '));
#else
    setTabStopWidth(TAB_SPACES_WIDTH * QFontMetrics(font).width(' '));
#endif
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
    QPalette our_pal = qApp->palette();
    QColor active_highlight = our_pal.color(QPalette::Active, QPalette::Highlight);
    QColor active_highlightedtext = our_pal.color(QPalette::Active, QPalette::HighlightedText);
    our_pal.setColor(QPalette::Inactive, QPalette::Highlight, active_highlight);
    our_pal.setColor(QPalette::Inactive, QPalette::HighlightedText, active_highlightedtext);
    setPalette(our_pal);
    return;
}


// Center the screen on the cursor/caret location.
// Centering requires fresh information about the
// visible viewport, so we usually call this after
// the paint event has been processed.
void CodeViewEditor::DelayedCursorScreenCentering()
{
    if (m_DelayedCursorScreenCenteringRequired) {
        centerCursor();
#if !defined(Q_OS_WIN32) && !defined(Q_OS_MAC)
        // The Code View viewport stopped updating on Linux somewhere
        // around Qt5.12.0 in this delayed call to center the cursor.
        viewport()->update();
#endif
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
                offset = textLength();
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

QString CodeViewEditor::GetCaretElementName() 
{
    return m_element_name;
}

QList<ElementIndex> CodeViewEditor::GetCaretLocation()
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
    QList<ElementIndex> hierarchy = ConvertStackToHierarchy(GetCaretLocationStack(offset + len));

    // determine last block element containing caret
    QString element_name;
    foreach(ElementIndex ei, hierarchy) {
        if (BLOCK_LEVEL_TAGS.contains(ei.name)) {
        element_name = ei.name;
        }
    }
    m_element_name = element_name;

    return hierarchy;
}


void CodeViewEditor::StoreCaretLocationUpdate(const QList<ElementIndex> &hierarchy)
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


QString CodeViewEditor::ConvertHierarchyToQWebPath(const QList<ElementIndex>& hierarchy) const
{
    QStringList pathparts;
    for (int i=0; i < hierarchy.count(); i++) {
        QString part = hierarchy.at(i).name + " " + QString::number(hierarchy.at(i).index);
        pathparts.append(part);
    }
    return pathparts.join(",");
}


QList<ElementIndex> CodeViewEditor::ConvertStackToHierarchy(const QStack<StackElement> stack) const
{
    QList<ElementIndex> hierarchy;
    foreach(StackElement stack_element, stack) {
        ElementIndex new_element;
        new_element.name  = stack_element.name;
        new_element.index = stack_element.num_children - 1;
        hierarchy.append(new_element);
    }
    return hierarchy;
}


std::tuple<int, int> CodeViewEditor::ConvertHierarchyToCaretMove(const QList<ElementIndex> &hierarchy) const
{
    QString source = toPlainText();
    QString version = "any_version";
    GumboInterface gi = GumboInterface(source, version);
    gi.parse();
    QString webpath = ConvertHierarchyToQWebPath(hierarchy);
    GumboNode* end_node = gi.get_node_from_qwebpath(webpath);
    if (!end_node) {
      return std::make_tuple(0, 0);
    }
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
    return std::make_tuple(line - cursor.blockNumber(), col);
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

    // create a default selection when no user selection is provided
    if (!textCursor().hasSelection()) {
        QTextCursor newcursor(textCursor());
        newcursor.select(QTextCursor::LineUnderCursor);
        QString newtxt = newcursor.selectedText();
        int startpos = newcursor.selectionStart();
        QString cleantxt(newtxt);
        cleantxt = cleantxt.trimmed();
        if (cleantxt.startsWith("<")) {
             int p = cleantxt.indexOf(">");
             if (p > -1) cleantxt = cleantxt.mid(p+1, -1);
        }
        if (cleantxt.endsWith(">")) {
            int p = cleantxt.lastIndexOf("<");
            if (p > -1) cleantxt = cleantxt.mid(0, p);
        }
        int pos =  newtxt.indexOf(cleantxt, 0);
        if (pos > 0) startpos = startpos + pos;
        newcursor.clearSelection();
        newcursor.setPosition(startpos);
        newcursor.setPosition(startpos + cleantxt.length(), QTextCursor::KeepAnchor); 
        setTextCursor(newcursor);
    }

    // Emit a selection changed event, so we can make sure the style buttons are updated
    // to uncheck any heading buttons check states.
    emit selectionChanged();

    // Going to assume that the user is allowed to click anywhere within or just after the block
    // Also makes assumptions about being well formed, or else crazy things may happen...
    int pos = textCursor().selectionStart();
    MaybeRegenerateTagList();
    QString text = m_TagList.getSource();


    if (!IsPositionInBody(pos)) return;

    // find that tag that starts immediately **after** pos and then
    // then use its predecessor when working backwards
    int i = m_TagList.findLastTagOnOrBefore(pos);
    TagLister::TagInfo ti = m_TagList.at(i);
    while(i >= 0) {
        ti = m_TagList.at(i);

        if (BLOCK_LEVEL_TAGS.contains(ti.tname)) {

            // we do not want a closing block tag if that is where the cursor is now, look earlier
            if ((ti.ttype == "end") && ((pos >= ti.pos) && (pos < ti.pos + ti.len))) {
                i--;
                continue;
            }

            // special case for body tag or closing tag that we did not start in
            // just insert the element around the current selection
            if ((ti.tname == "body") || (ti.ttype == "end") || (ti.ttype == "single")) {
                InsertHTMLTagAroundSelection(element_name, "/" % element_name);
                return;
            }

            // if we reached here we have an opening block tag we need to replace
            QStringRef opening_tag_text(&text, ti.pos, ti.len);
            QString all_attributes = TagLister::extractAllAttributes(opening_tag_text);
            
            // look for matching closing tag from here to the end
            int j = i+1;
            TagLister::TagInfo et = m_TagList.at(j);
            while((et.len != -1) && (et.open_pos != ti.pos)) {
                j++;
                et = m_TagList.at(j);
            }
            if (et.len == -1) return; // no matching closing tag found

            // ready to now format this block
            QString new_opening_tag_text;
            if (preserve_attributes && (all_attributes.length() > 0)) {
                new_opening_tag_text = "<" + element_name + " " + all_attributes + ">";
            } else {
                new_opening_tag_text = "<" + element_name + ">";
            }

            QString new_closing_tag_text = "</" + element_name + ">";
            ReplaceTags(ti.pos, ti.pos + ti.len, new_opening_tag_text,
                        et.pos, et.pos + et.len, new_closing_tag_text);
            return;
        }
        i--;
    }
    return;
}


void CodeViewEditor::InsertHTMLTagAroundText(const QString &left_element_name,
                                             const QString &right_element_name,
                                             const QString &attributes,
                                             const QString &text)
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

void CodeViewEditor::InsertHTMLTagAroundSelection(const QString &left_element_name,
                                                  const QString &right_element_name,
                                                  const QString &attributes)
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

bool CodeViewEditor::IsPositionInBody(int pos)
{
    MaybeRegenerateTagList();
    return m_TagList.isPositionInBody(pos);
}

// This routine is time critical as it is called a lot
bool CodeViewEditor::IsPositionInTag(int pos)
{
    QString text = toPlainText();
    // find previous begin tag marker
    int pb = text.lastIndexOf('<', pos);
    // find next end tag marker
    int ne = text.indexOf('>', pos);

    // find next begin tag marker *after* this char
    // and handle case if missing
    int nb = text.indexOf('<', pos+1);
    if (nb == -1) nb = text.length()+1;

    // in tag if '<' is closer than '>' when search backwards
    // and if '>' is closer but than '<' (if it exists) but >= pos  when search forward
    if ((pb > text.lastIndexOf('>', pos-1)) && (ne >= pos) && (nb > ne)) {
        MaybeRegenerateTagList();
        return m_TagList.isPositionInTag(pos);
    }
    return false;
}

// OpeningTag is can be a begin tag or a single tag
bool CodeViewEditor::IsPositionInOpeningTag(int pos)
{
    MaybeRegenerateTagList();
    return m_TagList.isPositionInOpenTag(pos);
}

bool CodeViewEditor::IsPositionInClosingTag(int pos)
{
    MaybeRegenerateTagList();
    return m_TagList.isPositionInCloseTag(pos);
}

QString CodeViewEditor::GetOpeningTagName(int pos)
{
    QString tag_name;
    MaybeRegenerateTagList();
    int i = m_TagList.findFirstTagOnOrAfter(pos);
    TagLister::TagInfo ti = m_TagList.at(i);
    if ((pos >= ti.pos) && (pos < ti.pos + ti.len)) {
        if ((ti.ttype == "begin") || (ti.ttype == "single")) tag_name = ti.tname.toLower();
    }
    return tag_name;
}

QString CodeViewEditor::GetClosingTagName(int pos)
{
    QString tag_name;
    MaybeRegenerateTagList();
    int i = m_TagList.findFirstTagOnOrAfter(pos);
    TagLister::TagInfo ti = m_TagList.at(i);
    if ((pos >= ti.pos) && (pos < ti.pos + ti.len)) {
        if (ti.ttype == "end") tag_name = ti.tname.toLower();
    }
    return tag_name;
}


void CodeViewEditor::ToggleFormatSelection(const QString &element_name, const QString property_name, const QString property_value)
{
    if (element_name.isEmpty()) {
        return;
    }

    // Going to assume that the user is allowed to click anywhere within or just after the block
    // Also makes assumptions about being well formed, or else crazy things may happen...
    if (!textCursor().hasSelection()) {
        QTextCursor newcursor(textCursor());
        newcursor.select(QTextCursor::WordUnderCursor);
        setTextCursor(newcursor);
    }

    // Emit a selection changed event, so we can make sure the style buttons are updated
    // to uncheck any style buttons check states.
    emit selectionChanged();

    int pos = textCursor().selectionStart();

    MaybeRegenerateTagList();

    if (!IsPositionInBody(pos)) {
        // We are in an HTML file outside the body element. We might possibly be in an
        // inline CSS style so attempt to format that.
        if (!property_name.isEmpty()) {
            FormatCSSStyle(property_name, property_value);
        }
        return;
    }

    if (IsPositionInTag(textCursor().selectionStart()) ||
        IsPositionInTag(textCursor().selectionEnd()-1)) {
        // Not allowed to toggle style if caret placed on a tag
        return;
    }


    QString text = m_TagList.getSource();
    bool in_existing_tag_occurrence = false;

    // Look backwards from the current selection to find whether we are in an open occurrence
    // of this tag already within this block.
    int i = m_TagList.findLastTagOnOrBefore(pos);
    
    TagLister::TagInfo ti;
    while (i >= 0) {
        ti = m_TagList.at(i);
        if (ti.len == -1) return;
        
        if (element_name == ti.tname) {
            if (ti.ttype != "end") in_existing_tag_occurrence = true;
            break;
        } else if (BLOCK_LEVEL_TAGS.contains(ti.tname)) {
            // No point in searching any further - we reached the block parent
            // without finding an open occurrence of this tag.
            break;
        }
        // Not a tag of interest - keep searching.
        i--;
    }
    if (i < 0) return;

    if (in_existing_tag_occurrence) {
        FormatSelectionWithinElement(element_name, i, text);
    } else {
        // Otherwise assume we are in a safe place to add a wrapper tag.
        InsertHTMLTagAroundSelection(element_name, "/" % element_name);
    }
}

// This routine is used solely from within ToggleFormatSelection
void CodeViewEditor::FormatSelectionWithinElement(const QString &element_name, int tagno, const QString &text)
{
    // We are inside an existing occurrence. Are we immediately adjacent to it?
    // e.g. "<b>selected text</b>" should result in "selected text" losing the tags.
    // but  "<b>XXXselected textYYY</b> should result in "<b>XXX</b>selected text<b>YYY</b>"
    // plus the variations where XXX or YYY may be non-existent to make tag adjacent.
    int j = m_TagList.findCloseTagForOpen(tagno);
    if (j < 0) return; // no closing tag for this style then not well formed so abort

    TagLister::TagInfo tb = m_TagList.at(tagno);
    TagLister::TagInfo te = m_TagList.at(j);

    QTextCursor cursor = textCursor();
    int selection_start = cursor.selectionStart();
    int selection_end = cursor.selectionEnd();
    bool adjacent_to_start = tb.pos + tb.len == selection_start;
    bool adjacent_to_end = te.pos == selection_end;

    if (!adjacent_to_start && !adjacent_to_end) {
        // We want to put a closing tag at the start and an opening tag after (copying attributes)
        // Do NOT copy the '<' or the '>' of the opening tag!
        QString opening_tag_text = text.mid(tb.pos+1, tb.len-2).trimmed();
        InsertHTMLTagAroundSelection("/" % element_name, opening_tag_text);
    } else if ((selection_end == selection_start) && (adjacent_to_start || adjacent_to_end) &&
               (te.pos > tb.pos + tb.len)) {
        // User is just inside the opening or closing tag with no selection and there is text within
        // The tags. Nothing to do in this situation.
        // If there is no text e.g. <b></b> and caret between then we will toggle off as per following else.
        return;
    } else {
        QString opening_tag = text.mid(tb.pos, tb.len);
        QString closing_tag = text.mid(te.pos, te.len);
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
    MaybeRegenerateTagList();

    int pos = textCursor().selectionStart();

    if (!IsPositionInOpeningTag(pos)) return element;
    
    QString text = m_TagList.getSource();
    
    int i = m_TagList.findLastTagOnOrBefore(pos);
    TagLister::TagInfo ti = m_TagList.at(i);
    QStringRef tagstring(&text, ti.pos, ti.len);
    element.name = ti.tname;

    TagLister::AttInfo attr;
    TagLister::parseAttribute(tagstring, "class", attr);
    if (attr.pos < 0) {
        return element;
    }

    // if caret not in a class attribute search only for element
    int cstart = ti.pos + attr.pos;
    if (pos < cstart || pos >= cstart + attr.len) return element;

    QString avalue = attr.avalue.trimmed();
    QStringList vals = avalue.split(' ');
    if (vals.length() == 1) {
        //just one class value provided use it
        element.classStyle = avalue;
        return element;
    }
    
    // multiple values present, see if the original cursor as in one of them
    // if so use that one, if not return the first
    int vstart = ti.pos + attr.vpos;
    if (pos < vstart || (pos  >= vstart + attr.vlen )) {
        element.classStyle = vals[0];
        return element;
    }

    // cursor is someplace inside the class value area and multiple values present
    int offset = pos - vstart;
    avalue = attr.avalue; // untrimmed so it matches vpos and vlen
    int k = 0;
    int p = avalue.indexOf(' ', 0);
    while ((p > 0) && p < offset) {
        k++;
        p = avalue.indexOf(' ', p+1);
    }
    if (i >= 0 && k < vals.size()) {
        element.classStyle = vals[k];
    } else {
        element.classStyle = vals[0];
    }
    return element;
}


QString CodeViewEditor::GetAttributeId()
{
    int pos = textCursor().selectionStart();
    QString tag_name = GetOpeningTagName(pos);
    // If we're in an opening tag use it for the id, else use a
    QStringList tag_list = ID_TAGS;

    if (tag_name.isEmpty()) {
        tag_list = ANCHOR_TAGS;
    }

    return GetAttribute("id", tag_list, false, true);
}

QString CodeViewEditor::GetAttribute(const QString &attribute_name,
                                     QStringList tag_list,
                                     bool must_be_in_attribute,
                                     bool skip_paired_tags,
                                     bool must_be_in_body)
{
    return ProcessAttribute(attribute_name, tag_list, QString(), 
                            false, must_be_in_attribute, skip_paired_tags, must_be_in_body);
}


QString CodeViewEditor::SetAttribute(const QString &attribute_name,
                                     QStringList tag_list,
                                     const QString &attribute_value,
                                     bool must_be_in_attribute,
                                     bool skip_paired_tags)
{
    return ProcessAttribute(attribute_name, tag_list, attribute_value, 
                            true, must_be_in_attribute, skip_paired_tags);
}


QString CodeViewEditor::ProcessAttribute(const QString &attribute_name,
                                         QStringList tag_list,
                                         const QString &attribute_value,
                                         bool set_attribute,
                                         bool must_be_in_attribute,
                                         bool skip_paired_tags,
                                         bool must_be_in_body)
{

    if (attribute_name.isEmpty()) {
        return QString();
    }

    if (tag_list.count() == 0) {
        tag_list = BLOCK_LEVEL_TAGS;
    }

    // Makes assumptions about being well formed, or else crazy things may happen...
    // Given the code <p>abc</p>, users can click between first < and > and
    // one character to the left of the first <.
    int pos = textCursor().selectionStart();
    int original_position = textCursor().position();
    
    MaybeRegenerateTagList();
    QString text = m_TagList.getSource();

    // The old implementation did not properly handle pi, multi-line comments, cdata
    // nor attribute values delimited by single quotes

    if (must_be_in_body && !IsPositionInBody(pos)) return QString();

    // If we're in a closing tag, move to the text between tags to make parsing easier.
    if (IsPositionInClosingTag(pos)) {
        while (pos > 0 && text[pos] != QChar('<')) {
            pos--;
        }
    }
    if (pos < 0) return QString();

    // now find the tag that starts immediately *after* position pos
    int i = m_TagList.findLastTagOnOrBefore(pos);
    TagLister::TagInfo ti = m_TagList.at(i);
    QList<int> paired_tags;
    while((i >= 0) && (m_TagList.at(i).tname != "body")) {
        ti = m_TagList.at(i);
        // qDebug() << " checking the tag: " << ti.tname << ti.ttype << ti.pos;
        if (ti.ttype == "end") {
            if (skip_paired_tags && !BLOCK_LEVEL_TAGS.contains(ti.tname)) {
                paired_tags << ti.open_pos;
            }
            if (tag_list.contains(ti.tname) || BLOCK_LEVEL_TAGS.contains(ti.tname)) {
                return QString();
            }
        } else if ((ti.ttype == "begin") || (ti.ttype == "single")) {
            if (skip_paired_tags && paired_tags.contains(ti.pos)) {
                paired_tags.removeOne(ti.pos);
            } else {
                // did we found what we want
                if (tag_list.contains(ti.tname) || BLOCK_LEVEL_TAGS.contains(ti.tname)) break; 
            }
        }
        // skip all special tags like doctype, cdata, pi, xmlheaders, and comments
        i--;
    }     
    if ((i < 0) || !tag_list.contains(ti.tname) || (ti.tname == "body")) return QString();
    QStringRef opening_tag_text(&text, ti.pos, ti.len);

    // Now look for the attribute, which may or may not already exist
    TagLister::AttInfo ainfo;
    TagLister::parseAttribute(opening_tag_text, attribute_name, ainfo);

    // qDebug() << " in tag: " << opening_tag_text;
    // qDebug() << " found attribute: " << ainfo.aname << ainfo.avalue << ainfo.pos << ainfo.len;
    // set absolute attribute start and end locations in text
    int attribute_start = ti.pos + ti.len - 1;  // right before the tag '>'
    int attribute_end = attribute_start;
    if (ainfo.pos != -1) {
        // attribute exists
        attribute_start = ti.pos + ainfo.pos - 1; // include single leading space as part of attribute 
        attribute_end = attribute_start + 1 + ainfo.len; // and compensate when setting the end position
        if (must_be_in_attribute && (original_position <= attribute_start  || original_position >= attribute_end)) {
            return "";
        }
    }

    if (!set_attribute) return ainfo.avalue;

    // setting an attribute value to a empty or null value deletes the attribute
    // if no attribute found and doing a delete then just return since nopthing to delete
    if ((ainfo.pos == -1) && attribute_value.isEmpty()) return QString();

    QString attribute_text;
    if (!attribute_value.isEmpty()) {
        attribute_text = " " + TagLister::serializeAttribute(attribute_name, attribute_value);
    }

    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();
    cursor.setPosition(attribute_end);
    cursor.setPosition(attribute_start, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    if (!attribute_text.isEmpty()) {
        cursor.insertText(attribute_text);
    }

    // Now place the cursor at the end of this opening tag, taking into account difference in attributes.
    cursor.setPosition(ti.pos + ti.len - (attribute_end - attribute_start) + attribute_text.length());
    cursor.endEditBlock();
    setTextCursor(cursor);
    return attribute_value;
}


void CodeViewEditor::FormatTextDir(const QString &attribute_value)
{
    // Going to assume that the user is allowed to click anywhere within or just after the block
    // Also makes assumptions about being well formed, or else crazy things may happen...
    int pos = textCursor().selectionStart();
    if (!IsPositionInBody(pos)) {
        return;
    }
    // Apply the modified attribute.
    SetAttribute("dir", ID_TAGS, attribute_value);
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

    if (!IsPositionInBody(pos)) {
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
        QList<HTMLStyleInfo::CSSProperty> css_properties = HTMLStyleInfo::getCSSProperties(style_attribute_value, 0, style_attribute_value.length());
        // Apply our property value, adding if not present currently, toggling if it is.
        ApplyChangeToProperties(css_properties, property_name, property_value);

        if (css_properties.count() == 0) {
            style_attribute_value = QString();
        } else {
            QStringList property_values;
            foreach(HTMLStyleInfo::CSSProperty css_property, css_properties) {
                if (css_property.value.isNull()) {
                    property_values.append(css_property.name);
                } else {
                    property_values.append(QString("%1: %2").arg(css_property.name).arg(css_property.value));
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

    // Now parse the HTML style content
    QList<HTMLStyleInfo::CSSProperty> css_properties = HTMLStyleInfo::getCSSProperties(text, bracket_start + 1, bracket_end);
    // Apply our property value, adding if not present currently, toggling if it is.
    ApplyChangeToProperties(css_properties, property_name, property_value);
    // Figure out the formatting to be applied to these style properties to write prettily
    // preserving any multi-line/single line style the CSS had before we changed things.
    bool is_single_line_format = (block.position() < bracket_start) && (bracket_end <= (block.position() + block.length()));
    const QString &style_attribute_text = HTMLStyleInfo::formatCSSProperties(css_properties, !is_single_line_format);
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

void CodeViewEditor::ApplyChangeToProperties(QList<HTMLStyleInfo::CSSProperty > &css_properties, const QString &property_name, const QString &property_value)
{
    // Apply our property value, adding if not present currently, toggling if it is.
    bool has_property = false;

    for (int i = css_properties.length() - 1; i >= 0; i--) {
        HTMLStyleInfo::CSSProperty css_property = css_properties.at(i);

        if (css_property.name.toLower() == property_name) {
            has_property = true;

            // We will treat this as a toggle - if we already have the value then remove it
            if (css_property.value.toLower() == property_value) {
                css_properties.removeAt(i);
                continue;
            } else {
                css_property.value = property_value;
            }
        }
    }

    if (!has_property) {
        HTMLStyleInfo::CSSProperty new_property;
        new_property.name = property_name;
        new_property.value = property_value;
        css_properties.append(new_property);
    }
}

// FIXME: Detect the type of document we are editing and handle both
// internal xhtml style elements and external css stylesheets.
// This routine is now only enabled when editing a CSS Stylesheet
void CodeViewEditor::ReformatCSS(bool multiple_line_format)
{
    const QString original_text = toPlainText();
    // Currently this feature is only enabled for CSS content, no inline HTML
    CSSInfo css_info(original_text);
    QString new_text = css_info.getReformattedCSSText(multiple_line_format);

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
    MainWindow *mainWindow = qobject_cast<MainWindow *>(mainWindow_w);
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
            StoreCaretLocationUpdate(GetCaretLocation());
            QTextCursor cursor = textCursor();
            cursor.beginEditBlock();
            cursor.select(QTextCursor::Document);
            cursor.insertText(new_text);
            cursor.endEditBlock();
            ExecuteCaretUpdate();
        }
    }
}

QString CodeViewEditor::RemoveFirstTag(const QString &text, const QString &tagname)
{
    QString result = text;
    int p = result.indexOf(">");
    if (p > -1) {
        QString tag = result.mid(0,p+1);
        if (tag.contains(tagname)) {
            result = result.mid(p+1,-1);
        }
    }
    return result;
}

QString CodeViewEditor::RemoveLastTag(const QString &text, const QString &tagname)
{
    QString result = text;
    int p = result.lastIndexOf("<");
    if (p > -1) {
        QString tag = result.mid(p,-1);
        if (tag.contains(tagname)) {
        result = result.mid(0,p);
    }
    }
    return result;
}

bool CodeViewEditor::IsSelectionValid(const QString & text) 
{
    // whatever is selected must either be pure text or have balanced
    // opening and closing tags (ie. well-formed).
    // fastest way to check this is parse the fragment
    GumboInterface gi = GumboInterface(text, "any_version");
    QList<GumboWellFormedError> results = gi.fragment_error_check();
    if (!results.isEmpty()) {
      return false;
    }
    return true;
}

void CodeViewEditor::WrapSelectionInElement(const QString& element, bool unwrap)
{
    QTextCursor cursor = textCursor();
    const QString selected_text = cursor.selectedText();

    if (selected_text.isEmpty()) {
        return;
    }

    QString new_text = selected_text;
 
    if (!IsSelectionValid(new_text)) return;

    QRegularExpression start_indent(STARTING_INDENT_USED);
    QRegularExpressionMatch indent_mo = start_indent.match(new_text);
    QString indent;
    if (indent_mo.hasMatch()) {
        indent = indent_mo.captured(1);
    indent = indent.replace("\t","    ");
    }
 
    QRegularExpression open_tag_at_start(OPEN_TAG_STARTS_SELECTION);
    QRegularExpressionMatch open_mo = open_tag_at_start.match(new_text);
    QString tagname;
    if (open_mo.hasMatch()) {
        tagname = open_mo.captured(2);
    }

    if (unwrap) {
        if (tagname == element) {
            new_text = RemoveFirstTag(new_text, element);
        new_text = RemoveLastTag(new_text, element);
        new_text = new_text.trimmed();
        new_text = indent + new_text;
    }
    }
    else {
        new_text = new_text.trimmed();
        QStringList result;
    result.append(indent + "<" + element + ">");
    result.append(indent + "    " + new_text);
    result.append(indent + "</" + element + ">\n");
    new_text = result.join('\n');
    }
    
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


void CodeViewEditor::ApplyListToSelection(const QString &element)
{
    QTextCursor cursor = textCursor();
    const QString selected_text = cursor.selectedText();

    if (selected_text.isEmpty()) {
        return;
    }

    QString new_text = selected_text;

    QRegularExpression start_indent(STARTING_INDENT_USED);
    QRegularExpressionMatch indent_mo = start_indent.match(new_text);
    QString indent;
    if (indent_mo.hasMatch()) {
        indent = indent_mo.captured(1);
        indent = indent.replace("\t","    ");
    }
 
    QRegularExpression open_tag_at_start(OPEN_TAG_STARTS_SELECTION);
    QRegularExpressionMatch open_mo = open_tag_at_start.match(new_text);
    QString tagname;
    if (open_mo.hasMatch()) {
        tagname = open_mo.captured(2);
    }

    if (((tagname == "ol") && (element == "ol")) || 
        ((tagname == "ul") && (element == "ul"))) {
        new_text = RemoveFirstTag(new_text, element);
        new_text = RemoveLastTag(new_text, element);
        new_text = new_text.trimmed();
        // now split remaining text by new lines and 
        // remove any beginning and ending li tags
        QStringList alist = new_text.split(QChar::ParagraphSeparator, QT_ENUM_SKIPEMPTYPARTS);
        QStringList result;
        foreach(QString aitem, alist) {
            result.append(indent + RemoveLastTag(RemoveFirstTag(aitem,"li"), "li"));
        }
        result.append("");
        new_text = result.join("\n");
    }
    else if ((tagname == "p") || tagname.isEmpty()) {
        QStringList alist = new_text.split(QChar::ParagraphSeparator, QT_ENUM_SKIPEMPTYPARTS);
        QStringList result;
        result.append(indent + "<" + element + ">");
        foreach(QString aitem, alist) {
            result.append(indent + "    " + "<li>" + aitem.trimmed() + "</li>"); 
        }
        result.append(indent + "</" + element + ">\n");
        new_text = result.join('\n');
    }
    
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



QStringList CodeViewEditor::GetUnmatchedTagsForBlock(int pos)
{
    // Given the specified position within the text, keep looking backwards finding
    // any tags until we hit all open block tags within the body. Append all the opening tags
    // that do not have closing tags together (ignoring self-closing tags)
    // and return the opening tags list complete with their attributes contiguously.
    // Note: this should *never* include the html, head, or the body opening tags
    QStringList opening_tags;
    QList<int> paired_tags;
    MaybeRegenerateTagList();
    QString text = m_TagList.getSource();
    int i = m_TagList.findFirstTagOnOrAfter(pos);
    // so start looking for unmatched tags starting at i - 1
    i--;
    if (i < 0) return opening_tags;
    while((i >= 0) && (m_TagList.at(i).tname != "body")) {
        TagLister::TagInfo ti = m_TagList.at(i);
        if (ti.ttype == "end") {
            paired_tags << ti.open_pos;
        } else if (ti.ttype == "begin") {
            if (paired_tags.contains(ti.pos)) {
                paired_tags.removeOne(ti.pos);
            } else {
                opening_tags.prepend(text.mid(ti.pos, ti.len));
            }
        }
        // ignore single, and all special tags like doctype, cdata, pi, xmlheaders, and comments
        i--;
    }     
    return opening_tags;
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

    // Tell FlowTab to Tell Preview to Sync to this Location
    emit PageClicked();
}

void CodeViewEditor::ToggleLineWrapMode()
{
    if (lineWrapMode() == QPlainTextEdit::NoWrap) {
        setLineWrapMode(QPlainTextEdit::WidgetWidth);
    } else {
        setLineWrapMode(QPlainTextEdit::NoWrap);
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
    connect(this, SIGNAL(undoAvailable(bool)), this, SLOT(UpdateUndoAvailable(bool)));
    connect(this, SIGNAL(selectionChanged()), this, SLOT(ResetLastFindMatch()));
    connect(m_ScrollOneLineUp,   SIGNAL(activated()), this, SLOT(ScrollOneLineUp()));
    connect(m_ScrollOneLineDown, SIGNAL(activated()), this, SLOT(ScrollOneLineDown()));
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0) 
    connect(m_spellingMapper, SIGNAL(mapped(const QString &)), this, SLOT(InsertText(const QString &)));
    connect(m_addSpellingMapper, SIGNAL(mapped(const QString &)), this, SLOT(addToDefaultDictionary(const QString &)));
    connect(m_addDictMapper, SIGNAL(mapped(const QString &)), this, SLOT(addToUserDictionary(const QString &)));
    connect(m_ignoreSpellingMapper, SIGNAL(mapped(const QString &)), this, SLOT(ignoreWord(const QString &)));
    connect(m_clipMapper, SIGNAL(mapped(const QString &)), this, SLOT(PasteClipEntryFromName(const QString &)));
#else
    connect(m_spellingMapper, SIGNAL(mappedString(const QString &)), this, SLOT(InsertText(const QString &)));
    connect(m_addSpellingMapper, SIGNAL(mappedString(const QString &)), this, SLOT(addToDefaultDictionary(const QString &)));
    connect(m_addDictMapper, SIGNAL(mappedString(const QString &)), this, SLOT(addToUserDictionary(const QString &)));
    connect(m_ignoreSpellingMapper, SIGNAL(mappedString(const QString &)), this, SLOT(ignoreWord(const QString &)));
    connect(m_clipMapper, SIGNAL(mappedString(const QString &)), this, SLOT(PasteClipEntryFromName(const QString &)));
#endif
}
