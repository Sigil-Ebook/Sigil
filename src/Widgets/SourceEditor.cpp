/************************************************************************
**
**  Copyright (C) 2025 Kevin B. Hendricks, Stratford Ontario Canada
**
**  Based on CodeViewEditor.cpp portions of which were:
**    Copyright (C) 2012      John Schember <john@nachtimwald.com>
**    Copyright (C) 2012-2013 Dave Heiland
**    Copyright (C) 2012      Grant Drake
**    Copyright (C) 2009-2011 Strahinja Markovic  <strahinja.markovic@gmail.com>, Nokia Corporation
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
#include <QApplication>
#include <QFileInfo>
#include <QContextMenuEvent>
#include <QSignalMapper>
#include <QAction>
#include <QMenu>
#include <QPointer>
#include <QColor>
#include <QPainter>
#include <QScrollBar>
#include <QTextBlock>
#include <QPlainTextEdit>
#include <QSyntaxHighlighter>
#include <QDebug>

#include "Misc/XHTMLHighlighter.h"
#include "Misc/CSSHighlighter.h"
#include "Misc/PythonHighlighter.h"
#include "MainUI/MainWindow.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "Widgets/SELineNumberArea.h"
#include "Widgets/SourceEditor.h"

static const int TAB_SPACES_WIDTH     = 4;
static const int LINE_NUMBER_MARGIN   = 5;

static const QChar _PAD               = QChar(0x2007); // "use a 'figure space' 8199

SourceEditor::SourceEditor(QWidget *parent)
    :
    QPlainTextEdit(parent),
    m_LineNumberArea(new SELineNumberArea(this)),
    m_Highlighter(nullptr)
{
    UpdateLineNumberAreaMargin();
    setReadOnly(false);
    setTextInteractionFlags(textInteractionFlags() | Qt::TextSelectableByKeyboard);
    setLineWrapMode(QPlainTextEdit::WidgetWidth);
    QFont cf = font();
    cf.setFamily("Courier New");
    cf.setStyleHint(QFont::TypeWriter);
    setFont(cf);
    m_verticalScrollBar = verticalScrollBar();
    ConnectSignalsToSlots();
    SetAppearance();
}

SourceEditor::~SourceEditor()
{
}

bool SourceEditor::event(QEvent* e)
{
    return QPlainTextEdit::event(e);
}

void SourceEditor::setBlockMap(const QStringList& blockmap)
{
    m_blockmap = blockmap;
    UpdateLineNumberAreaMargin();
}

int SourceEditor::CalculateLineNumberAreaWidth()
{
    int max_width = 0;
    foreach(const QString& aval, m_blockmap) {
        if (aval.length() > max_width) {
            max_width = aval.length();
        }
    }
    if (max_width == 0) {
        return 0;
    }
    int num_digits = 1;
    int max_value = std::max(1, blockCount());
    // We count the number of digits
    // for the line number of the last line
    while (max_value >= 10) {
        max_value /= 10;
        num_digits++;
    }
    return LINE_NUMBER_MARGIN * 2 + fontMetrics().horizontalAdvance(QChar('0')) * num_digits;

}

void SourceEditor::UpdateLineNumberAreaFont(const QFont &font)
{
    m_LineNumberArea->setFont(font);
    UpdateLineNumberAreaMargin();
}

void SourceEditor::UpdateLineNumberAreaMargin()
{
    // The left margin width depends on width of the line number area
    setViewportMargins(CalculateLineNumberAreaWidth(), 0, 0, 0);
}

void SourceEditor::UpdateLineNumberArea(const QRect &area_to_update, int vertically_scrolled)
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

void SourceEditor::LineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(m_LineNumberArea);
    // Paint the background first
    painter.fillRect(event->rect(), m_codeViewAppearance.line_number_background_color);
    // painter.fillRect(event->rect(), Qt::lightGray);
    QTextBlock block = firstVisibleBlock();
    int blockNumber  = block.blockNumber();

    int top = blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + blockBoundingRect(block).height();
    int height = fontMetrics().height();

    // We loop through all the visible and
    // unobscured blocks and paint line numbers for each
    while (block.isValid() && (top <= event->rect().bottom())) {
        if (block.isVisible() && (bottom >= event->rect().top())) {
            QString numlbl = "";
            if (blockNumber < m_blockmap.count()) {
                numlbl = m_blockmap.at(blockNumber);
            }
            // Draw the number in the line number area.
            painter.setPen(m_codeViewAppearance.line_number_foreground_color);
            // painter.setPen(Qt::black);
            painter.drawText(0, top, m_LineNumberArea->width(), height, Qt::AlignRight, numlbl);
        }
        block = block.next();
        top = bottom;
        bottom = top + blockBoundingRect(block).height();
        blockNumber++;
    }
}

void SourceEditor::DoHighlightDocument(HighlighterType high_type)
{
    if (m_Highlighter) {
        delete m_Highlighter;
        m_Highlighter = nullptr;
    }
    if (!m_Highlighter) {
        if (high_type == SourceEditor::Highlight_XHTML) {
            m_Highlighter = new XHTMLHighlighter(false, this);
        } else if (high_type == SourceEditor::Highlight_CSS) {
            m_Highlighter = new CSSHighlighter(this);
        } else if (high_type == SourceEditor::Highlight_PYTHON) {
            m_Highlighter = new PythonHighlighter(this);
        }
    }
    if (m_Highlighter) {
        m_Highlighter->setDocument(document());
        RehighlightDocument();
    }
}

void SourceEditor::Refresh(HighlighterType high_type)
{
    SetAppearance();
    DoHighlightDocument(high_type);
}

void SourceEditor::RehighlightDocument()
{
    if (!isVisible()) {
        return;
    }

    if (m_Highlighter) {
        // We block signals from the document while highlighting takes place,
        // because we do not want the contentsChanged() signal to be fired
        // which would mark the underlying resource as needing saving.
        document()->blockSignals(true);
        CSSHighlighter* cssh = qobject_cast<CSSHighlighter*>(m_Highlighter);
        if (cssh) cssh->do_rehighlight();

        XHTMLHighlighter* xmlh = qobject_cast<XHTMLHighlighter*>(m_Highlighter);
        if (xmlh) xmlh->do_rehighlight();

        PythonHighlighter* pyh = qobject_cast<PythonHighlighter*>(m_Highlighter);
        if (pyh) pyh->do_rehighlight();
        document()->blockSignals(false);
    }
}


void SourceEditor::SetAppearance()
{
    // follow the codeViewAppearance here
    SettingsStore settings;
    if (Utility::IsDarkMode()) {
        m_codeViewAppearance = settings.codeViewDarkAppearance();
    } else {
        m_codeViewAppearance = settings.codeViewAppearance();
    }
    SetAppearanceColors();
    UpdateLineNumberAreaMargin();
}


QSize SourceEditor::sizeHint() const
{
    return QSize(16777215, 16777215);
}

void SourceEditor::ScrollToTop()
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

void SourceEditor::ScrollToPosition(int cursor_position, bool center_screen)
{
    if (cursor_position < 0) {
        return;
    }

    QTextCursor cursor(document());
    cursor.setPosition(cursor_position);
    setTextCursor(cursor);
}

void SourceEditor::ScrollToBlock(int bnum)
{
    if (bnum <= 0) {
        return;
    }

    QTextCursor cursor(document());
    cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, bnum);
    setTextCursor(cursor);
}

QString SourceEditor::rstrip_pad(const QString& str) 
{
    int n = str.size() - 1;
    for (; n >= 0; --n) {
        if (str.at(n) != _PAD) {
            return str.left(n + 1);
        }
    }
    return "";
}

QString SourceEditor::selected_text_from_cursor(const QTextCursor& cursor) const
{
    QString txt = cursor.selectedText();
    QChar *uc = txt.data();
    QChar *e = uc + txt.size();
    for (; uc != e; ++uc) {
        switch (uc->unicode()) {
            case 0xfdd0: // QTextBeginningOfFrame
            case 0xfdd1: // QTextEndOfFrame
            case QChar::ParagraphSeparator:
            case QChar::LineSeparator:
                *uc = QLatin1Char('\n');
                break;
            default:
            ;
        }
    }
    QStringList result;
    foreach(QString line, txt.split('\n')) {
        result << rstrip_pad(line); 
    }
    txt = result.join('\n');
    txt = Utility::UseNFC(txt);
    return txt;
}

// overrides document toPlainText to prevent loss of nbsp
// and to handle any line end padding chars
QString SourceEditor::toPlainText() const
{
    QString txt;
    // if the TextDocument itself is empty just return an empty string
    if (document()->isEmpty()) return txt;

    // Use text cursors to get the TextDocument's contents
    QTextCursor cursor = textCursor();
    cursor.select(QTextCursor::Document);
    txt = selected_text_from_cursor(cursor);
    return txt;
}

// overrides createMimeDataFromSelection()
QMimeData *SourceEditor::createMimeDataFromSelection() const
{
    QString txt = selected_text_from_cursor(textCursor());
    QMimeData* md = new QMimeData();
    md->setText(txt);
    return md;
}

int SourceEditor::GetCursorPosition() const
{
    const int position = textCursor().position();
    return position;
}

int SourceEditor::GetCursorBlockNumber() const
{
    return textCursor().block().blockNumber();
}

int SourceEditor::GetCursorColumn() const
{
    const QTextCursor cursor = textCursor();
    const QTextBlock block = cursor.block();
    const int column = cursor.position() - block.position();
    return column;
}

QString SourceEditor::GetSelectedText()
{
    return selected_text_from_cursor(textCursor());
}


// Overridden because after updating itself on a resize event,
// the editor needs to update its line number area too
void SourceEditor::resizeEvent(QResizeEvent *event)
{
    // Update self normally
    QPlainTextEdit::resizeEvent(event);
    QRect contents_area = contentsRect();
    // Now update the line number area
    m_LineNumberArea->setGeometry(QRect(contents_area.left(),
                                        contents_area.top(),
                                        CalculateLineNumberAreaWidth(),
                                        contents_area.height()
                                       ));
}


void SourceEditor::mousePressEvent(QMouseEvent *event)
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
        // GoToLinkOrStyle();
    }
}

void SourceEditor::mouseReleaseEvent(QMouseEvent *event)
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
void SourceEditor::contextMenuEvent(QContextMenuEvent *event)
{
    QPointer<QMenu> menu = createStandardContextMenu();
    
    menu->exec(event->globalPos());
    if (!menu.isNull()) {
        delete menu.data();
    }
}


// Overridden so we can emit the FocusGained() signal.
void SourceEditor::focusInEvent(QFocusEvent *event)
{
    RehighlightDocument();
    emit FocusGained(this);
    QPlainTextEdit::focusInEvent(event);
}


// Overridden so we can emit the FocusLost() signal.
void SourceEditor::focusOutEvent(QFocusEvent *event)
{
    emit FocusLost(this);
    QPlainTextEdit::focusOutEvent(event);
}


void SourceEditor::SetAppearanceColors()
{
    QPalette app_pal = qApp->palette();
    setPalette(app_pal);
    return;
}


void SourceEditor::ScrollByLine(bool down)
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

QScrollBar* SourceEditor::GetVerticalScrollBar()
{
    return m_verticalScrollBar;
}


void SourceEditor::ConnectSignalsToSlots()
{
    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(UpdateLineNumberAreaMargin()));
    connect(this, SIGNAL(updateRequest(const QRect &, int)), this, SLOT(UpdateLineNumberArea(const QRect &, int)));
}
