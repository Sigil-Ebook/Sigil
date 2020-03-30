/************************************************************************
**
**  Copyright (C) 2020 Kevin B. Hendricks, Stratford Ontario Canada
**  Copyright (C) 2020 Doug Massay
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
#include <QFileInfo>
#include <QContextMenuEvent>
#include <QSignalMapper>
#include <QAction>
#include <QMenu>
#include <QColor>
#include <QPainter>
#include <QScrollBar>
#include <QTextBlock>
#include <QPlainTextEdit>
#include <QKeyEvent>
#include <QSyntaxHighlighter>
#include <QDebug>

#include "Misc/XHTMLHighlighter.h"
#include "Misc/CSSHighlighter.h"
#include "MainUI/MainWindow.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "Widgets/TVLineNumberArea.h"
#include "Widgets/TextView.h"

static const int TAB_SPACES_WIDTH     = 4;
static const int LINE_NUMBER_MARGIN   = 5;

static const QChar _PAD               = QChar(0x2007); // "use a 'figure space' 8199

TextView::TextView(QWidget *parent)
    :
    QPlainTextEdit(parent),
    m_LineNumberArea(new TVLineNumberArea(this)),
    m_Highlighter(nullptr)
{
    UpdateLineNumberAreaMargin();
    setReadOnly(true);
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

TextView::~TextView()
{
}

void TextView::setBlockMap(const QStringList& blockmap)
{
    m_blockmap = blockmap;
    UpdateLineNumberAreaMargin();
}

int TextView::CalculateLineNumberAreaWidth()
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
    return LINE_NUMBER_MARGIN * 2 + fontMetrics().width(QChar('0')) * num_digits;
}

void TextView::UpdateLineNumberAreaFont(const QFont &font)
{
    m_LineNumberArea->setFont(font);
    UpdateLineNumberAreaMargin();
}

void TextView::UpdateLineNumberAreaMargin()
{
    // The left margin width depends on width of the line number area
    setViewportMargins(CalculateLineNumberAreaWidth(), 0, 0, 0);
}

void TextView::UpdateLineNumberArea(const QRect &area_to_update, int vertically_scrolled)
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

void TextView::LineNumberAreaPaintEvent(QPaintEvent *event)
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

void TextView::DoHighlightDocument(HighlighterType high_type)
{
    if (m_Highlighter) {
	delete m_Highlighter;
	m_Highlighter = nullptr;
    }
    if (!m_Highlighter) {
	if (high_type == TextView::Highlight_XHTML) {
	    m_Highlighter = new XHTMLHighlighter(false, this);
	} else if (high_type == TextView::Highlight_CSS) {
	    m_Highlighter = new CSSHighlighter(this);
	}
    }
    if (m_Highlighter) {
        m_Highlighter->setDocument(document());
	RehighlightDocument();
    }
}

void TextView::Refresh(HighlighterType high_type)
{
    SetAppearance();
    DoHighlightDocument(high_type);
}

void TextView::RehighlightDocument()
{
    if (!isVisible()) {
        return;
    }

    if (m_Highlighter) {
	// We block signals from the document while highlighting takes place,
	// because we do not want the contentsChanged() signal to be fired
	// which would mark the underlying resource as needing saving.
	document()->blockSignals(true);
	m_Highlighter->rehighlight();
	document()->blockSignals(false);
    }
}


void TextView::SetAppearance()
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


QSize TextView::sizeHint() const
{
    return QSize(16777215, 16777215);
}

void TextView::ScrollToTop()
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

void TextView::ScrollToPosition(int cursor_position, bool center_screen)
{
    if (cursor_position < 0) {
        return;
    }

    QTextCursor cursor(document());
    cursor.setPosition(cursor_position);
    setTextCursor(cursor);
}

void TextView::ScrollToBlock(int bnum)
{
    if (bnum <= 0) {
        return;
    }

    QTextCursor cursor(document());
    cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, bnum);
    setTextCursor(cursor);
}

QString TextView::rstrip_pad(const QString& str) 
{
    int n = str.size() - 1;
    for (; n >= 0; --n) {
        if (str.at(n) != _PAD) {
            return str.left(n + 1);
        }
    }
    return "";
}

QString TextView::selected_text_from_cursor(const QTextCursor& cursor) const
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
    return txt;
}

// overrides document toPlainText to prevent loss of nbsp
// and to handle any line end padding chars
QString TextView::toPlainText() const
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
QMimeData *TextView::createMimeDataFromSelection() const
{
    QString txt = selected_text_from_cursor(textCursor());
    QMimeData* md = new QMimeData();
    md->setText(txt);
    return md;
}

int TextView::GetCursorPosition() const
{
    const int position = textCursor().position();
    return position;
}

int TextView::GetCursorBlockNumber() const
{
    return textCursor().block().blockNumber();
}

int TextView::GetCursorColumn() const
{
    const QTextCursor cursor = textCursor();
    const QTextBlock block = cursor.block();
    const int column = cursor.position() - block.position();
    return column;
}

QString TextView::GetSelectedText()
{
    return selected_text_from_cursor(textCursor());
}


// Overridden because after updating itself on a resize event,
// the editor needs to update its line number area too
void TextView::resizeEvent(QResizeEvent *event)
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


void TextView::mousePressEvent(QMouseEvent *event)
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

void TextView::mouseReleaseEvent(QMouseEvent *event)
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
void TextView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = createStandardContextMenu();
    
    menu->exec(event->globalPos());
    delete menu;
}

#if 0
void TextView::AddReformatHTMLContextMenu(QMenu *menu)
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
#endif

#if 0
void TextView::GoToLinkOrStyle()
{
    // emit LinkClicked(QUrl(url_name));
}
#endif

// Overridden so we can emit the FocusGained() signal.
void TextView::focusInEvent(QFocusEvent *event)
{
    RehighlightDocument();
    emit FocusGained(this);
    QPlainTextEdit::focusInEvent(event);
}


// Overridden so we can emit the FocusLost() signal.
void TextView::focusOutEvent(QFocusEvent *event)
{
    emit FocusLost(this);
    QPlainTextEdit::focusOutEvent(event);
}

#if 0
void TextView::ScrollOneLineUp()
{
    ScrollByLine(false);
}

void TextView::ScrollOneLineDown()
{
    ScrollByLine(true);
}
#endif

#if 0
void TextView::ResetFont()
{
    // Let's try to use our user specified value as our font (default Courier New)
    QFont font(m_textViewAppearance.font_family, m_textViewAppearance.font_size);
    // But just in case, say we want a fixed width font if font is not present
    font.setStyleHint(QFont::TypeWriter);
    setFont(font);
    setTabStopWidth(TAB_SPACES_WIDTH * QFontMetrics(font).width(' '));
    UpdateLineNumberAreaFont(font);
}
#endif


void TextView::SetAppearanceColors()
{

    QPalette app_pal = qApp->palette();
    setPalette(app_pal);
    return;
}


void TextView::ScrollByLine(bool down)
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

QScrollBar* TextView::GetVerticalScrollBar()
{
    return m_verticalScrollBar;
}


void TextView::keyPressEvent(QKeyEvent* ev)
{
    int amount = 0;
    int d = 1;
    int key = ev->key();
    if ((key == Qt::Key_Up) || (key == Qt::Key_Down) || (key == Qt::Key_J) || (key ==  Qt::Key_K)) {
        amount = m_verticalScrollBar->singleStep();
	if ((key == Qt::Key_Up) || (key ==  Qt::Key_K)) d = -1;

    } else if ((key == Qt::Key_PageUp) || (key == Qt::Key_PageDown)) {
	amount = m_verticalScrollBar->pageStep();
	if (key == Qt::Key_PageUp) d = -1;

    } else if ((key == Qt::Key_Home) || (key == Qt::Key_End)) {
	if (key == Qt::Key_Home) {
	    m_verticalScrollBar->setValue(0);
	} else {
	    m_verticalScrollBar->maximum();
	}
        return;

    } else if ((key == Qt::Key_N) || (key == Qt::Key_P)) {
	if (key == Qt::Key_N) emit NextChange(1);
	if (key == Qt::Key_P) emit NextChange(-1);
        return;
    }

    if (amount != 0) {
        m_verticalScrollBar->setValue(m_verticalScrollBar->value() + d * amount);
        return;
    }
    return QPlainTextEdit::keyPressEvent(ev);
}

void TextView::ConnectSignalsToSlots()
{
    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(UpdateLineNumberAreaMargin()));
    connect(this, SIGNAL(updateRequest(const QRect &, int)), this, SLOT(UpdateLineNumberArea(const QRect &, int)));
}
