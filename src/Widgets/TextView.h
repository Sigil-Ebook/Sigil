/************************************************************************
**
**  Copyright (C) 2020 Kevin B. Hendricks Stratford, ON Canada 
**    Based on CodeViewEditor portions of which are Copyright 
**    (C) 2009-2011  Strahinja Markovic  <strahinja.markovic@gmail.com>
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
#ifndef TEXTVIEW_H
#define TEXTVIEW_H

#include <QList>
#include <QPlainTextEdit>
#include <QUrl>

#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"

class QResizeEvent;
class QSize;
class QWidget;
class QShortcut;
class TVLineNumberArea;
class QContextMenuEvent;
class QSyntaxHighlighter;

/**
 * A text viewer for source code and general text.
 */
class TextView : public QPlainTextEdit
{
    Q_OBJECT

public:

    enum HighlighterType {
        Highlight_NONE,  /**< No source code highlighting */
        Highlight_XHTML, /**< XHTML source code highlighting */
        Highlight_CSS    /**< CSS source code highlighting */
    };

    TextView(QWidget *parent = 0);
    ~TextView();

    void SetAppearance();
    void SetAppearanceColors();

    QSize sizeHint() const;

    void LineNumberAreaPaintEvent(QPaintEvent *event);
    void LineNumberAreaMouseEvent(QMouseEvent *event);
    int CalculateLineNumberAreaWidth();

    void ScrollToTop();
    void ScrollToPosition(int cursor_position, bool center_screen = true);
    void ScrollToBlock(int bnum);

    void DoHighlightDocument(HighlighterType highlighter_type);
    void RehighlightDocument();

    static QString rstrip_pad(const QString& str);
    QString selected_text_from_cursor(const QTextCursor& cursor) const;

    // override and hide the toPlainText() call to prevent
    // issues with lost non-breaking spaces (improperly
    // converted to normal spaces)
    QString toPlainText() const;
    
    // override the createMimeDataFromSelection() to 
    // prevent copy and cut from losing nbsp
    // ala Kovid's solution in calibre PlainTextEdit
    virtual QMimeData *createMimeDataFromSelection() const;

    int GetCursorPosition() const;
    int GetCursorBlockNumber() const;
    int GetCursorColumn() const;

    QScrollBar* GetVerticalScrollBar();

    QString GetSelectedText();

    void setBlockMap(const QStringList& blockmap);

public slots:
    void Refresh(HighlighterType hightype);

signals:

    void FocusLost(QWidget *editor);
    void FocusGained(QWidget *editor);

    void PageClicked();
    void PageUpdated();

    void NextChange(int);

    void LinkClicked(const QUrl &url);

protected:

    void keyPressEvent(QKeyEvent* ev);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    void resizeEvent(QResizeEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);

    void focusInEvent(QFocusEvent *event);
    void focusOutEvent(QFocusEvent *event);

private slots:

    void UpdateLineNumberAreaMargin();
    void UpdateLineNumberArea(const QRect &rectangle, int vertical_delta);

private:

    void UpdateLineNumberAreaFont(const QFont &font);

    // void SetAppearanceColors();

    void ScrollByLine(bool down);

    void ConnectSignalsToSlots();

    SettingsStore::CodeViewAppearance m_codeViewAppearance;
    TVLineNumberArea *m_LineNumberArea;
    QStringList  m_blockmap;
    QScrollBar* m_verticalScrollBar;
    QSyntaxHighlighter * m_Highlighter;
};

#endif // TEXTVIEW_H
