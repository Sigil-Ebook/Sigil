/************************************************************************
**
**  Copyright (C) 2016-2019  Kevin B Hendricks, Stratford, Ontario, Canada
**  Copyright (C) 2012       John Schember <john@nachtimwald.com>
**  Copyright (C) 2012       Dave Heiland
**  Copyright (C) 2012       Grant Drake
**  Copyright (C) 2009, 2010, 2011  Strahinja Markovic  <strahinja.markovic@gmail.com>
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
#include <QtCore/QUrl>
#include <QtWidgets/QApplication>
#include <QtWidgets/QAction>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLayout>
#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QPrintDialog>
#include <QtPrintSupport/QPrintPreviewDialog>

#include "BookManipulation/CleanSource.h"
#include "MiscEditors/ClipEditorModel.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "ResourceObjects/HTMLResource.h"
#include "sigil_constants.h"
#include "Tabs/FlowTab.h"
#include "Tabs/WellFormedCheckComponent.h"
#include "ViewEditors/CodeViewEditor.h"

static const QString SETTINGS_GROUP = "flowtab";

FlowTab::FlowTab(HTMLResource *resource,
                 const QUrl &fragment,
                 MainWindow::ViewState view_state,
                 int line_to_scroll_to,
                 int position_to_scroll_to,
                 QString caret_location_to_scroll_to,
                 bool grab_focus,
                 QWidget *parent)
    :
    ContentTab(resource, parent),
    m_FragmentToScroll(fragment),
    m_LineToScrollTo(line_to_scroll_to),
    m_PositionToScrollTo(position_to_scroll_to),
    m_CaretLocationToScrollTo(caret_location_to_scroll_to),
    m_HTMLResource(resource),
    m_wCodeView(NULL),
    m_ViewState(view_state),
    m_previousViewState(view_state),
    m_WellFormedCheckComponent(new WellFormedCheckComponent(this, parent)),
    m_safeToLoad(false),
    m_initialLoad(true),
    m_grabFocus(grab_focus),
    m_suspendTabReloading(false),
    m_defaultCaretLocationToTop(false),
    m_LastPosition(-1)
{
    // Loading a flow tab can take a while. We set the wait
    // cursor and clear it at the end of the delayed initialization.
    QApplication::setOverrideCursor(Qt::WaitCursor);
    CreateCodeViewIfRequired(false);
    m_Layout->addWidget(m_wCodeView);
    LoadSettings();
    setFocusProxy(m_wCodeView);
    ConnectCodeViewSignalsToSlots();

    // We perform delayed initialization after the widget is on
    // the screen. This way, the user perceives less load time.
    QTimer::singleShot(0, this, SLOT(DelayedInitialization()));
}

FlowTab::~FlowTab()
{
    // Explicitly disconnect signals because Modified is causing the ResourceModified
    // function to be called after we delete other things later in this destructor.
    // No idea how that's possible but this prevents a segfault...

    disconnect(this, 0, 0, 0);

    // or at least it used to, as this signal Modified still fires so try an explicit
    // disconnect

    disconnect(m_HTMLResource, SIGNAL(Modified()), this, SLOT(ResourceModified()));

    m_WellFormedCheckComponent->deleteLater();

    if (m_wCodeView) {
        delete m_wCodeView;
        m_wCodeView = 0;
    }

    m_HTMLResource = NULL;

}

void FlowTab::CreateCodeViewIfRequired(bool is_delayed_load)
{
    if (m_wCodeView) {
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_wCodeView = new CodeViewEditor(CodeViewEditor::Highlight_XHTML, true, this);
    m_wCodeView->SetReformatHTMLEnabled(true);
    // m_views->addWidget(m_wCodeView);

    if (is_delayed_load) {
        ConnectCodeViewSignalsToSlots();
        // CodeView (if already loaded) will be directly hooked into the TextResource and
        // will not need reloading. However if the tab was not in Code View at tab opening
        // then we must populate it now.
        m_wCodeView->CustomSetDocument(m_HTMLResource->GetTextDocumentForWriting());
        // Zoom assignment only works after the document has been loaded
        m_wCodeView->Zoom();
    }

    QApplication::restoreOverrideCursor();
}

void FlowTab::DelayedInitialization()
{
    if (m_wCodeView) {
        m_wCodeView->CustomSetDocument(m_HTMLResource->GetTextDocumentForWriting());
        // Zoom factor for CodeView can only be set when document has been loaded.
        m_wCodeView->Zoom();
    }

    CodeView();

    if (m_PositionToScrollTo > 0) {
        m_wCodeView->ScrollToPosition(m_PositionToScrollTo);
    } else if (m_LineToScrollTo > 0) {
        m_wCodeView->ScrollToLine(m_LineToScrollTo);
    } else {
        m_wCodeView->ScrollToFragment(m_FragmentToScroll.toString());
    }

    m_initialLoad = false;
    // Only now will we wire up monitoring of ResourceChanged, to prevent
    // unnecessary saving and marking of the resource for reloading.
    DelayedConnectSignalsToSlots();
    // Cursor set in constructor
    QApplication::restoreOverrideCursor();
}

// we only support codeview now
MainWindow::ViewState FlowTab::GetViewState()
{
    return MainWindow::ViewState_CodeView;
}

bool FlowTab::SetViewState(MainWindow::ViewState new_view_state)
{
    // There are only two ways a tab can get routed into a particular viewstate.
    // At FlowTab construction time, all initialisation is done via a timer to
    // call DelayedInitialization(). So that needs to handle first time tab state.
    // The second route is via this function, which is invoked from MainWindow
    // any time a tab is switched to.

    // Do we really need to do anything? Not if we are already in this state.
    if (new_view_state == m_ViewState) {
        return false;
    }

    // Ignore this function if we are in the middle of doing an initial load
    // of the content. We don't want it to save over the content with nothing
    // if this is called before the delayed initialization function is called.
    if (m_initialLoad || !IsLoadingFinished()) {
        return false;
    }

    // We do a save (if pending changes) before switching to ensure we don't lose
    // any unsaved data in the current view, as cannot rely on lost focus happened.
    SaveTabContent();
    // Track our previous view state for the purposes of caret syncing
    m_previousViewState = m_ViewState;
    m_ViewState = MainWindow::ViewState_CodeView;
    // if (new_view_state == MainWindow::ViewState_CodeView) {
        // As CV is directly hooked to the QTextDocument, we never have to "Load" content
        // except for when creating the CV control for the very first time.
        // CreateCodeViewIfRequired();
        // CodeView();
    // }

    return true;
}

bool FlowTab::IsLoadingFinished()
{
    bool is_finished = true;

    if (m_wCodeView) {
        is_finished = m_wCodeView->IsLoadingFinished();
    }

    return is_finished;
}

bool FlowTab::IsModified()
{
    bool is_modified = false;

    if (m_wCodeView) {
        is_modified = is_modified || m_wCodeView->document()->isModified();
    }

    return is_modified;
}

void FlowTab::CodeView()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_ViewState = MainWindow::ViewState_CodeView;
    CreateCodeViewIfRequired();
    // m_views->setCurrentIndex(m_views->indexOf(m_wCodeView));
    m_wCodeView->SetDelayedCursorScreenCenteringRequired();
    setFocusProxy(m_wCodeView);

    // We will usually want focus in the tab, except when splitting opens this as a preceding tab.
    if (m_grabFocus) {
        m_wCodeView->setFocus();
    }

    m_grabFocus = true;

    m_wCodeView->ExecuteCaretUpdate();
    
    QApplication::restoreOverrideCursor();
}

void FlowTab::LoadTabContent()
{
    // In CV, this call has nothing to do, as the resource is connected to QTextDocument.
    //        The only exception is initial load when control is created, done elsewhere.
}

void FlowTab::SaveTabContent()
{
    // In CV, the connection between QPlainTextEdit and the underlying QTextDocument
    //        means the resource already is "saved". We just need to reset modified state.
    m_HTMLResource->GetTextDocumentForWriting().setModified(false);
}

void FlowTab::ResourceModified()
{
    // This slot tells us that the underlying HTML resource has been changed
    // It could be the user has done a Replace All on underlying resource, so reset our well formed check.
    m_safeToLoad = false;
    // When the underlying resource has been modified, it replaces the whole QTextDocument which
    // causes cursor position to move to bottom of the document. We will have captured the location
    // of the caret prior to replacing in the ResourceTextChanging() slot, so now we can restore it.
    // First try to get to the enclosing block and if possible the exact position
    m_wCodeView->ExecuteCaretUpdate(m_defaultCaretLocationToTop);
    m_defaultCaretLocationToTop = false;
    if (m_LastPosition > 0) {
        m_wCodeView->ScrollToPosition(m_LastPosition);
        m_LastPosition = -1;
    }

    EmitUpdatePreview();
}

void FlowTab::LinkedResourceModified()
{
    // MainWindow::clearMemoryCaches();
    ResourceModified();
    ReloadTabIfPending();
}

void FlowTab::ResourceTextChanging()
{
    // We need to store caret (cursor) position so it can be restored later
    // Store an exact position as well as the tag hierarchy
    m_LastPosition = m_wCodeView->GetCursorPosition();
    m_wCodeView->StoreCaretLocationUpdate(m_wCodeView->GetCaretLocation());
    // If the caret happened to be at the very top of the document then our location
    // will be empty. The problem is that when ResourceModified() fires next
    // it will have effectively moved the caret to the bottom of the document.
    // At which point we will call the CodeView to restore caret location.
    // However the default behaviour of CodeView is to "do nothing" if no location
    // is stored. So in this one situation we want to override that to force
    // it to place the cursor at the top of the document.
    m_defaultCaretLocationToTop = true;
}

void FlowTab::ReloadTabIfPending()
{
    if (!isVisible()) {
        return;
    }

    if (m_suspendTabReloading) {
        return;
    }

    setFocus();
}

void FlowTab::LeaveEditor(QWidget *editor)
{
    SaveTabContent();
}

void FlowTab::LoadSettings()
{
    UpdateDisplay();

    // SettingsChanged can fire for wanting the spelling highlighting to be refreshed on the tab.
    if (m_wCodeView) {
        m_wCodeView->RefreshSpellingHighlighting();
    }
}

void FlowTab::UpdateDisplay()
{
    if (m_wCodeView) {
        m_wCodeView->UpdateDisplay();
    }
}

void FlowTab::EmitContentChanged()
{
    m_safeToLoad = false;
    emit ContentChanged();
}

void FlowTab::EmitUpdatePreview()
{
    emit UpdatePreview();
}

void FlowTab::EmitUpdatePreviewImmediately()
{
      emit UpdatePreviewImmediately();
}

void FlowTab::EmitScrollPreviewImmediately()
{
  if (!m_wCodeView->document()->isModified()) {
      emit ScrollPreviewImmediately();
  } else {
      emit UpdatePreviewImmediately();
  }
}

void FlowTab::EmitUpdateCursorPosition()
{
    emit UpdateCursorPosition(GetCursorLine(), GetCursorColumn());
}

void FlowTab::HighlightWord(QString word, int pos)
{
    if (m_wCodeView) {
        m_wCodeView->HighlightWord(word, pos);
    }
}

void FlowTab::RefreshSpellingHighlighting()
{
    if (m_wCodeView) {
        m_wCodeView->RefreshSpellingHighlighting();
    }
}


bool FlowTab::CutEnabled()
{
    if (m_wCodeView) {
        return m_wCodeView->textCursor().hasSelection();
    }
    return false;
}

bool FlowTab::CopyEnabled()
{
    if (m_wCodeView) {
        return m_wCodeView->textCursor().hasSelection();
    }
    return false;
}

bool FlowTab::PasteEnabled()
{
    if (m_wCodeView) {
        return m_wCodeView->canPaste();
    }
    return false;
}

bool FlowTab::DeleteLineEnabled()
{
    if (m_wCodeView) {
        return !m_wCodeView->document()->isEmpty();
    }
    return false;
}

bool FlowTab::RemoveFormattingEnabled()
{
    if (m_wCodeView) {
        return m_wCodeView->IsCutCodeTagsAllowed();
    }
    return false;
}

bool FlowTab::InsertClosingTagEnabled()
{
    if (m_wCodeView) {
        return m_wCodeView->IsInsertClosingTagAllowed();
    }
    return false;
}

bool FlowTab::AddToIndexEnabled()
{
    if (m_wCodeView) {
        return m_wCodeView->IsAddToIndexAllowed();
    }
    return false;
}

bool FlowTab::MarkForIndexEnabled()
{
    if (m_wCodeView) {
        return true;
    }
    return false;
}

bool FlowTab::InsertIdEnabled()
{
    if (m_wCodeView) {
        return m_wCodeView->IsInsertIdAllowed();
    }
    return false;
}

bool FlowTab::InsertHyperlinkEnabled()
{
    if (m_wCodeView) {
        return m_wCodeView->IsInsertHyperlinkAllowed();
    } 
    return false;
}

bool FlowTab::InsertSpecialCharacterEnabled()
{
    if (m_wCodeView) {
        return true;
    }
    return false;
}

bool FlowTab::InsertFileEnabled()
{
    if (m_wCodeView) {
        return m_wCodeView->IsInsertFileAllowed();
    } 
    return false;
}

bool FlowTab::ToggleAutoSpellcheckEnabled()
{
    if (m_wCodeView) {
        return true;
    }
    return false;
}

bool FlowTab::ViewStatesEnabled()
{
    if (m_wCodeView) {
        return true;
    }
    return false;
}

void FlowTab::GoToCaretLocation(QList<ElementIndex> location)
{
    if (location.isEmpty()) {
        return;
    }
    if (m_wCodeView) {
        m_wCodeView->StoreCaretLocationUpdate(location);
        m_wCodeView->ExecuteCaretUpdate();
    }
}

QList<ElementIndex> FlowTab::GetCaretLocation()
{
    if (m_wCodeView) {
        return m_wCodeView->GetCaretLocation();
    }
    return QList<ElementIndex>();
}

QString FlowTab::GetCaretLocationUpdate() const
{
    return QString();
}

QString FlowTab::GetDisplayedCharacters()
{
    return "";
}

QString FlowTab::GetText()
{
    if (m_wCodeView) {
        return m_wCodeView->toPlainText();
    } 
    return "";
}

int FlowTab::GetCursorPosition() const
{
    if (m_wCodeView) {
        return m_wCodeView->GetCursorPosition();
    }
    return -1;
}

int FlowTab::GetCursorLine() const
{
    if (m_wCodeView) {
        return m_wCodeView->GetCursorLine();
    }
    return -1;
}

int FlowTab::GetCursorColumn() const
{
    if (m_wCodeView) {
        return m_wCodeView->GetCursorColumn();
    }
    return -1;
}

float FlowTab::GetZoomFactor() const
{
    if (m_wCodeView) {
        return m_wCodeView->GetZoomFactor();
    }
    return 1;
}

void FlowTab::SetZoomFactor(float new_zoom_factor)
{
    if (m_wCodeView) {
        m_wCodeView->SetZoomFactor(new_zoom_factor);
    }
}

Searchable *FlowTab::GetSearchableContent()
{
    if (m_wCodeView) {
        return m_wCodeView;
    }
    return NULL;
}


void FlowTab::ScrollToFragment(const QString &fragment)
{
    if (m_wCodeView) {
        m_wCodeView->ScrollToFragment(fragment);
    }
}

void FlowTab::ScrollToLine(int line)
{
    if (m_wCodeView) {
        m_wCodeView->ScrollToLine(line);
    }
}

void FlowTab::ScrollToPosition(int cursor_position)
{
    if (m_wCodeView) {
        m_wCodeView->ScrollToPosition(cursor_position);
    }
}

void FlowTab::ScrollToCaretLocation(QString caret_location_update)
{
}

void FlowTab::ScrollToTop()
{
    if (m_wCodeView) {
        m_wCodeView->ScrollToTop();
    }
}

void FlowTab::AutoFixWellFormedErrors()
{
    if (m_wCodeView) {
        int pos = m_wCodeView->GetCursorPosition();
        QString version = m_HTMLResource->GetEpubVersion();
        m_wCodeView->ReplaceDocumentText(CleanSource::ToValidXHTML(m_wCodeView->toPlainText(), version));
        m_wCodeView->ScrollToPosition(pos);
    }
}

void FlowTab::TakeControlOfUI()
{
    EmitCentralTabRequest();
    setFocus();
}

QString FlowTab::GetFilename()
{
    return ContentTab::GetFilename();
}

bool FlowTab::IsDataWellFormed()
{
    // The content has been changed or was in a not well formed state when last checked.
    QString version = m_HTMLResource->GetEpubVersion();

    // So lets play safe and have a fallback to use the resource text if CV is not loaded yet.
    XhtmlDoc::WellFormedError error = (m_wCodeView != NULL)
        ? XhtmlDoc::WellFormedErrorForSource(m_wCodeView->toPlainText(),version)
        : XhtmlDoc::WellFormedErrorForSource(m_HTMLResource->GetText(),version);
    m_safeToLoad = error.line == -1;
    if (!m_safeToLoad) {
          m_WellFormedCheckComponent->DemandAttentionIfAllowed(error);
    }
    return m_safeToLoad;
}

void FlowTab::Undo()
{
    if (m_wCodeView) {
        m_wCodeView->undo();
    }
}

void FlowTab::Redo()
{
    if (m_wCodeView) {
        m_wCodeView->redo();
    }
}

void FlowTab::Cut()
{
    if (m_wCodeView) {
        m_wCodeView->cut();
    }
}

void FlowTab::Copy()
{
    if (m_wCodeView) {
        m_wCodeView->copy();
    }
}

void FlowTab::Paste()
{
    if (m_wCodeView) {
        m_wCodeView->paste();
    }
}

void FlowTab::DeleteLine()
{
    if (m_wCodeView) {
        m_wCodeView->DeleteLine();
    }
}

bool FlowTab::MarkSelection()
{
    if (m_wCodeView) {
        return m_wCodeView->MarkSelection();
    }
    return false;
}

bool FlowTab::ClearMarkedText()
{
    if (m_wCodeView) {
        return m_wCodeView->ClearMarkedText();
    }
    return false;
}

void FlowTab::SplitSection()
{
    if (!IsDataWellFormed()) {
        return;
    }

    QWidget *mainWindow_w = Utility::GetMainWindow();
    MainWindow *mainWindow = dynamic_cast<MainWindow *>(mainWindow_w);
    if (!mainWindow) {
        Utility::DisplayStdErrorDialog("Could not determine main window.");
        return;
    }
    HTMLResource * nav_resource = mainWindow->GetCurrentBook()->GetConstOPF()->GetNavResource();
    if (nav_resource && (nav_resource == m_HTMLResource)) {
        Utility::DisplayStdErrorDialog("The Nav file can not be split");
        return;
    }

    // Handle warning the user about undefined url fragments.
    if (!mainWindow->ProceedWithUndefinedUrlFragments()) {
        return;
    }

    if (m_wCodeView) {
        emit OldTabRequest(m_wCodeView->SplitSection(), m_HTMLResource);
    }
}

void FlowTab::InsertSGFSectionMarker()
{
    if (!IsDataWellFormed()) {
        return;
    }

    QWidget *mainWindow_w = Utility::GetMainWindow();
    MainWindow *mainWindow = dynamic_cast<MainWindow *>(mainWindow_w);
    if (!mainWindow) {
        Utility::DisplayStdErrorDialog("Could not determine main window.");
        return;
    }
    HTMLResource * nav_resource = mainWindow->GetCurrentBook()->GetConstOPF()->GetNavResource();
    if (nav_resource && (nav_resource == m_HTMLResource)) {
        Utility::DisplayStdErrorDialog("The Nav file can not be split");
        return;
    }

    if (m_wCodeView) {
        m_wCodeView->InsertSGFSectionMarker();
    }
}

void FlowTab::InsertClosingTag()
{
    if (m_wCodeView) {
        m_wCodeView->InsertClosingTag();
    }
}

void FlowTab::AddToIndex()
{
    if (m_wCodeView) {
        m_wCodeView->AddToIndex();
    }
}

bool FlowTab::MarkForIndex(const QString &title)
{
    if (m_wCodeView) {
        return m_wCodeView->MarkForIndex(title);
    } 
    return false;
}

QString FlowTab::GetAttributeId()
{
    QString attribute_value;

    if (m_wCodeView) {
        // We are only interested in ids on <a> anchor elements
        attribute_value = m_wCodeView->GetAttributeId();
    } 
    return attribute_value;
}

QString FlowTab::GetAttributeHref()
{
    QString attribute_value;

    if (m_wCodeView) {
        attribute_value = m_wCodeView->GetAttribute("href", ANCHOR_TAGS, false, true);
    }
    return attribute_value;
}

QString FlowTab::GetAttributeIndexTitle()
{
    QString attribute_value;

    if (m_wCodeView) {
        attribute_value = m_wCodeView->GetAttribute("title", ANCHOR_TAGS, false, true);
        if (attribute_value.isEmpty()) {
            attribute_value = m_wCodeView->GetSelectedText();
        }
    }
    return attribute_value;
}

QString FlowTab::GetSelectedText()
{
    if (m_wCodeView) {
        return m_wCodeView->GetSelectedText();
    } 
    return "";
}

bool FlowTab::InsertId(const QString &id)
{
    if (m_wCodeView) {
        return m_wCodeView->InsertId(id);
    } 
    return false;
}

bool FlowTab::InsertHyperlink(const QString &href)
{
    if (m_wCodeView) {
        return m_wCodeView->InsertHyperlink(href);
    } 
    return false;
}

void FlowTab::InsertFile(QString html)
{
    if (m_wCodeView) {
        m_wCodeView->insertPlainText(html);
    }
}

void FlowTab::PrintPreview()
{
    QPrintPreviewDialog *print_preview = new QPrintPreviewDialog(this);

    if (m_wCodeView) {
        connect(print_preview, SIGNAL(paintRequested(QPrinter *)), m_wCodeView, SLOT(print(QPrinter *)));
    } else {
        return;
    }

    print_preview->exec();
    print_preview->deleteLater();
}

void FlowTab::Print()
{
    QPrinter printer;
    QPrintDialog print_dialog(&printer, this);
    print_dialog.setWindowTitle(tr("Print %1").arg(GetFilename()));

    if (print_dialog.exec() == QDialog::Accepted) {
        if (m_wCodeView) {
            m_wCodeView->print(&printer);
        }
    }
}

void FlowTab::Bold()
{
    if (m_wCodeView) {
        m_wCodeView->ToggleFormatSelection("b", "font-weight", "bold");
    }
}

void FlowTab::Italic()
{
    if (m_wCodeView) {
        m_wCodeView->ToggleFormatSelection("i", "font-style", "italic");
    }
}

void FlowTab::Underline()
{
    if (m_wCodeView) {
        m_wCodeView->ToggleFormatSelection("u", "text-decoration", "underline");
    }
}

// the strike tag has been deprecated, the del tag is still okay
void FlowTab::Strikethrough()
{
    if (m_wCodeView) {
        m_wCodeView->ToggleFormatSelection("del", "text-decoration", "line-through");
    }
}

void FlowTab::Subscript()
{
    if (m_wCodeView) {
        m_wCodeView->ToggleFormatSelection("sub");
    }
}

void FlowTab::Superscript()
{
    if (m_wCodeView) {
        m_wCodeView->ToggleFormatSelection("sup");
    }
}

void FlowTab::AlignLeft()
{
    if (m_wCodeView) {
        m_wCodeView->FormatStyle("text-align", "left");
    }
}

void FlowTab::AlignCenter()
{
    if (m_wCodeView) {
        m_wCodeView->FormatStyle("text-align", "center");
    }
}

void FlowTab::AlignRight()
{
    if (m_wCodeView) {
        m_wCodeView->FormatStyle("text-align", "right");
    }
}

void FlowTab::AlignJustify()
{
    if (m_wCodeView) {
        m_wCodeView->FormatStyle("text-align", "justify");
    }
}

void FlowTab::InsertBulletedList()
{
  // should we put something here
}

void FlowTab::InsertNumberedList()
{
  // should we put something here
}

void FlowTab::DecreaseIndent()
{
  // should we put something here
}

void FlowTab::IncreaseIndent()
{
  // should we put something here
}

void FlowTab::TextDirectionLeftToRight()
{
    QString version = m_HTMLResource->GetEpubVersion();
    if (m_wCodeView) {
        if (version.startsWith("3")) {
            m_wCodeView->FormatTextDir("ltr"); 
        } else {
            m_wCodeView->FormatStyle("direction", "ltr");
        }
   }
}

void FlowTab::TextDirectionRightToLeft()
{
    QString version = m_HTMLResource->GetEpubVersion();
    if (m_wCodeView) {
        if (version.startsWith("3")) {
            m_wCodeView->FormatTextDir("rtl"); 
        } else {
            m_wCodeView->FormatStyle("direction", "rtl");
        }
    }
}

void FlowTab::TextDirectionDefault()
{
    QString version = m_HTMLResource->GetEpubVersion();
    if (m_wCodeView) {
        if (version.startsWith("3")) {
            m_wCodeView->FormatTextDir(QString()); 
        } else {
            m_wCodeView->FormatStyle("direction", "inherit");
        }
    }
}

void FlowTab::ShowTag()
{
}

void FlowTab::RemoveFormatting()
{
    if (m_wCodeView) {
        m_wCodeView->CutCodeTags();
    }
}

void FlowTab::ChangeCasing(const Utility::Casing casing)
{
    if (m_wCodeView) {
        m_wCodeView->ApplyCaseChangeToSelection(casing);
    }
}

void FlowTab::HeadingStyle(const QString &heading_type, bool preserve_attributes)
{
    if (m_wCodeView) {
        QChar last_char = heading_type[ heading_type.count() - 1 ];

        // For heading_type == "Heading #"
        if (last_char.isDigit()) {
            m_wCodeView->FormatBlock("h" % QString(last_char), preserve_attributes);
        } else if (heading_type == "Normal") {
            m_wCodeView->FormatBlock("p", preserve_attributes);
        }
    }
}

void FlowTab::GoToLinkOrStyle()
{
    if (m_wCodeView) {
        m_wCodeView->GoToLinkOrStyle();
    }
}

void FlowTab::AddMisspelledWord()
{
    if (m_wCodeView) {
        m_wCodeView->AddMisspelledWord();
    }
}

void FlowTab::IgnoreMisspelledWord()
{
    if (m_wCodeView) {
        m_wCodeView->IgnoreMisspelledWord();
    }
}

bool FlowTab::BoldChecked()
{
    return ContentTab::BoldChecked();
}

bool FlowTab::ItalicChecked()
{
    return ContentTab::ItalicChecked();
}

bool FlowTab::UnderlineChecked()
{
    return ContentTab::UnderlineChecked();
}

bool FlowTab::StrikethroughChecked()
{
    return ContentTab::StrikethroughChecked();
}

bool FlowTab::SubscriptChecked()
{
    return ContentTab::SubscriptChecked();
}

bool FlowTab::SuperscriptChecked()
{
    return ContentTab::SuperscriptChecked();
}

bool FlowTab::AlignLeftChecked()
{
    return ContentTab::AlignLeftChecked();
}

bool FlowTab::AlignRightChecked()
{
    return ContentTab::AlignRightChecked();
}

bool FlowTab::AlignCenterChecked()
{
    return ContentTab::AlignCenterChecked();
}

bool FlowTab::AlignJustifyChecked()
{
    return ContentTab::AlignJustifyChecked();
}

bool FlowTab::BulletListChecked()
{
    return ContentTab::BulletListChecked();
}

bool FlowTab::NumberListChecked()
{
    return ContentTab::NumberListChecked();
}

bool FlowTab::PasteClipNumber(int clip_number)
{
    if (m_wCodeView) {
        return m_wCodeView->PasteClipNumber(clip_number);
    }
    return false;
}

bool FlowTab::PasteClipEntries(QList<ClipEditorModel::clipEntry *>clips)
{
    if (m_wCodeView) {
        return m_wCodeView->PasteClipEntries(clips);
    }
    return false;
}

QString FlowTab::GetCaretElementName()
{
    return ContentTab::GetCaretElementName();
}

void FlowTab::SuspendTabReloading()
{
    // Call this function to prevent the currently displayed PV from being
    // reloaded if a linked resource is changed. Automatic reloading can cause
    // issues if your code is attempting to manipulate the tab content concurrently.
    m_suspendTabReloading = true;
}

void FlowTab::ResumeTabReloading()
{
    // Call this function to resume reloading of PV in response to linked
    // resources changing. If a reload tab request is pending it is executed now.
    m_suspendTabReloading = false;
}

void FlowTab::DelayedConnectSignalsToSlots()
{
    connect(m_HTMLResource, SIGNAL(TextChanging()), this, SLOT(ResourceTextChanging()));
    connect(m_HTMLResource, SIGNAL(LinkedResourceUpdated()), this, SLOT(LinkedResourceModified()));
    connect(m_HTMLResource, SIGNAL(Modified()), this, SLOT(ResourceModified()));
    connect(m_HTMLResource, SIGNAL(LoadedFromDisk()), this, SLOT(ReloadTabIfPending()));
}

void FlowTab::ConnectCodeViewSignalsToSlots()
{
    connect(m_wCodeView, SIGNAL(cursorPositionChanged()), this, SLOT(EmitUpdateCursorPosition()));
    connect(m_wCodeView, SIGNAL(ZoomFactorChanged(float)), this, SIGNAL(ZoomFactorChanged(float)));
    connect(m_wCodeView, SIGNAL(selectionChanged()), this, SIGNAL(SelectionChanged()));
    connect(m_wCodeView, SIGNAL(FocusLost(QWidget *)), this, SLOT(LeaveEditor(QWidget *)));
    connect(m_wCodeView, SIGNAL(LinkClicked(const QUrl &)), this, SIGNAL(LinkClicked(const QUrl &)));
    connect(m_wCodeView, SIGNAL(ViewImage(const QUrl &)), this, SIGNAL(ViewImageRequest(const QUrl &)));
    connect(m_wCodeView, SIGNAL(OpenClipEditorRequest(ClipEditorModel::clipEntry *)), this, SIGNAL(OpenClipEditorRequest(ClipEditorModel::clipEntry *)));
    connect(m_wCodeView, SIGNAL(OpenIndexEditorRequest(IndexEditorModel::indexEntry *)), this, SIGNAL(OpenIndexEditorRequest(IndexEditorModel::indexEntry *)));
    connect(m_wCodeView, SIGNAL(GoToLinkedStyleDefinitionRequest(const QString &, const QString &)), this, SIGNAL(GoToLinkedStyleDefinitionRequest(const QString &, const QString &)));
    connect(m_wCodeView, SIGNAL(BookmarkLinkOrStyleLocationRequest()), this, SIGNAL(BookmarkLinkOrStyleLocationRequest()));
    connect(m_wCodeView, SIGNAL(SpellingHighlightRefreshRequest()), this, SIGNAL(SpellingHighlightRefreshRequest()));
    connect(m_wCodeView, SIGNAL(ShowStatusMessageRequest(const QString &)), this, SIGNAL(ShowStatusMessageRequest(const QString &)));
    connect(m_wCodeView, SIGNAL(FilteredTextChanged()), this, SLOT(EmitContentChanged()));
    connect(m_wCodeView, SIGNAL(FilteredCursorMoved()), this, SLOT(EmitUpdatePreview()));
    connect(m_wCodeView, SIGNAL(PageUpdated()), this, SLOT(EmitUpdatePreview()));
    connect(m_wCodeView, SIGNAL(PageClicked()), this, SLOT(EmitScrollPreviewImmediately()));
    connect(m_wCodeView, SIGNAL(DocumentSet()), this, SLOT(EmitUpdatePreviewImmediately()));
    connect(m_wCodeView, SIGNAL(MarkSelectionRequest()), this, SIGNAL(MarkSelectionRequest()));
    connect(m_wCodeView, SIGNAL(ClearMarkedTextRequest()), this, SIGNAL(ClearMarkedTextRequest()));
}
