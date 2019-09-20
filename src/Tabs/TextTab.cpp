/************************************************************************
**
**  Copyright (C) 2015-2019 Kevin B. Hendricks, Stratford Ontario Canada
**  Copyright (C) 2009-2011 Strahinja Markovic  <strahinja.markovic@gmail.com>
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

#include <QtCore/QTimer>
#include <QtWidgets/QLayout>
#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QPrintDialog>
#include <QtPrintSupport/QPrintPreviewDialog>

#include "ResourceObjects/TextResource.h"
#include "Tabs/TextTab.h"

TextTab::TextTab(TextResource *resource,
                 CodeViewEditor::HighlighterType type,
                 int line_to_scroll_to,
                 int position_to_scroll_to,
                 QWidget *parent)
    :
    ContentTab(resource, parent),
    m_wCodeView(new CodeViewEditor(type, false, this)),
    m_TextResource(resource),
    m_LineToScrollTo(line_to_scroll_to),
    m_PositionToScrollTo(position_to_scroll_to)
{
    m_Layout->addWidget(m_wCodeView);
    setFocusProxy(m_wCodeView);
    ConnectSignalsToSlots();
    // Make sure the resource is loaded as its file doesn't seem
    // to exist when the resource tries to do an initial load.
    m_TextResource->InitialLoad();
    // We perform delayed initialization after the widget is on
    // the screen. This way, the user perceives less load time.
    QTimer::singleShot(0, this, SLOT(DelayedInitialization()));
}

TextTab::~TextTab()
{
    if (m_wCodeView) {
        delete m_wCodeView;
        m_wCodeView = 0;
    }
}


void TextTab::ScrollToLine(int line)
{
    m_wCodeView->ScrollToLine(line);
}

void TextTab::ScrollToPosition(int cursor_position)
{
    m_wCodeView->ScrollToPosition(cursor_position);
}

bool TextTab::IsModified()
{
    return m_wCodeView->document()->isModified();
}


bool TextTab::CutEnabled()
{
    return m_wCodeView->textCursor().hasSelection();
}


bool TextTab::CopyEnabled()
{
    return m_wCodeView->textCursor().hasSelection();
}


bool TextTab::PasteEnabled()
{
    return m_wCodeView->canPaste();
}

bool TextTab::DeleteLineEnabled()
{
    return !m_wCodeView->document()->isEmpty();
}

bool TextTab::CutCodeTagsEnabled()
{
    return false;
}


int TextTab::GetCursorLine() const
{
    return m_wCodeView->GetCursorLine();
}


int TextTab::GetCursorColumn() const
{
    return m_wCodeView->GetCursorColumn();
}


float TextTab::GetZoomFactor() const
{
    return m_wCodeView->GetZoomFactor();
}


void TextTab::SetZoomFactor(float new_zoom_factor)
{
    m_wCodeView->SetZoomFactor(new_zoom_factor);
}


void TextTab::UpdateDisplay()
{
    m_wCodeView->UpdateDisplay();
}


Searchable *TextTab::GetSearchableContent()
{
    return m_wCodeView;
}


void TextTab::Undo()
{
    if (m_wCodeView->hasFocus()) {
        m_wCodeView->undo();
    }
}


void TextTab::Redo()
{
    if (m_wCodeView->hasFocus()) {
        m_wCodeView->redo();
    }
}


void TextTab::Cut()
{
    if (m_wCodeView->hasFocus()) {
        m_wCodeView->cut();
    }
}


void TextTab::Copy()
{
    if (m_wCodeView->hasFocus()) {
        m_wCodeView->copy();
    }
}


void TextTab::Paste()
{
    m_wCodeView->paste();
}


void TextTab::DeleteLine()
{
    if (m_wCodeView->hasFocus()) {
        m_wCodeView->DeleteLine();
    }
}

bool TextTab::MarkSelection()
{
    return m_wCodeView->MarkSelection();
}

bool TextTab::ClearMarkedText()
{
    return m_wCodeView->ClearMarkedText();
}

void TextTab::CutCodeTags()
{
}

void TextTab::ChangeCasing(const Utility::Casing casing)
{
    if (m_wCodeView->hasFocus()) {
        m_wCodeView->ApplyCaseChangeToSelection(casing);
    }
}


void TextTab::SaveTabContent()
{
    // We can't perform the document modified check
    // here because that causes problems with epub export
    // when the user has not changed the text file.
    // (some text files have placeholder text on disk)
    if (!m_wCodeView->document()->isModified()) {
        ContentTab::SaveTabContent();
        return;
    }

    // Losing focus will invoke SaveTabContent
    // which may run SaveToDisk on the underlying resource 
    // that will reset the cursor to the end of file
    // so save the cursor position and then put it
    // back after the SaveToDisk
    int cursorPosition = m_wCodeView->GetCursorPosition();

    // The call to SaveToDisk **IS** needed here even though the
    // QTextEdit is tied directly to the QtQPlainTextDocument.
    // Webkit is passed in an xhtml file but all supporting files
    // such as CSS, svg, and etc are only ever read from Disk,
    // so any time focus is lost and the contents of the TextTab 
    // have changed, we should save them to disk
    m_TextResource->SaveToDisk();

    // Some systems (Linux/Windows) may lose text highlighting when
    // ScrollToPosition is called unnecessarily. Only reposition
    // if the cursor has actually moved.
    int newcursorPosition = m_wCodeView->GetCursorPosition();
    if (newcursorPosition != cursorPosition) m_wCodeView->ScrollToPosition(cursorPosition, false);
    ContentTab::SaveTabContent();
}


void TextTab::SaveTabContent(QWidget *editor)
{
    Q_UNUSED(editor);
    SaveTabContent();
}


void TextTab::LoadTabContent()
{
}


void TextTab::LoadTabContent(QWidget *editor)
{
    Q_UNUSED(editor);
    LoadTabContent();
}


void TextTab::EmitUpdateCursorPosition()
{
    emit UpdateCursorPosition(GetCursorLine(), GetCursorColumn());
}

void TextTab::DelayedInitialization()
{
    m_wCodeView->CustomSetDocument(m_TextResource->GetTextDocumentForWriting());
    m_wCodeView->Zoom();
    if (m_PositionToScrollTo > 0) {
        m_wCodeView->ScrollToPosition(m_PositionToScrollTo);
    } else {
        m_wCodeView->ScrollToLine(m_LineToScrollTo);
    }
}


void TextTab::ConnectSignalsToSlots()
{
    // We set the Code View as the focus proxy for the tab,
    // so the ContentTab focusIn/Out handlers are not called.
    connect(m_wCodeView, SIGNAL(FocusGained(QWidget *)),    this, SLOT(LoadTabContent(QWidget *)));
    connect(m_wCodeView, SIGNAL(FocusLost(QWidget *)),      this, SLOT(SaveTabContent(QWidget *)));
    connect(m_wCodeView, SIGNAL(FilteredTextChanged()),      this, SIGNAL(ContentChanged()));
    connect(m_wCodeView, SIGNAL(cursorPositionChanged()),     this, SLOT(EmitUpdateCursorPosition()));
    connect(m_wCodeView, SIGNAL(ZoomFactorChanged(float)), this, SIGNAL(ZoomFactorChanged(float)));
    connect(m_wCodeView, SIGNAL(selectionChanged()),         this, SIGNAL(SelectionChanged()));
    connect(m_wCodeView, SIGNAL(OpenClipEditorRequest(ClipEditorModel::clipEntry *)), this, SIGNAL(OpenClipEditorRequest(ClipEditorModel::clipEntry *)));
    connect(m_wCodeView, SIGNAL(MarkSelectionRequest()),         this, SIGNAL(MarkSelectionRequest()));
    connect(m_wCodeView, SIGNAL(ClearMarkedTextRequest()),              this, SIGNAL(ClearMarkedTextRequest()));
}


void TextTab::PrintPreview()
{
    QPrintPreviewDialog *print_preview = new QPrintPreviewDialog(this);
    connect(print_preview, SIGNAL(paintRequested(QPrinter *)), m_wCodeView, SLOT(print(QPrinter *)));
    print_preview->exec();
    print_preview->deleteLater();
}

void TextTab::Print()
{
    QPrinter printer;
    QPrintDialog print_dialog(&printer, this);
    print_dialog.setWindowTitle(tr("Print %1").arg(GetFilename()));

    if (print_dialog.exec() == QDialog::Accepted) {
        m_wCodeView->print(&printer);
    }
}

