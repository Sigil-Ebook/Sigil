/************************************************************************
**
**  Copyright (C) 2012 John Schember <john@nachtimwald.com>
**  Copyright (C) 2012  Dave Heiland
**  Copyright (C) 2012  Grant Drake
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
#include <QtWidgets/QStackedWidget>

#include "BookManipulation/CleanSource.h"
#include "MiscEditors/ClipEditorModel.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "ResourceObjects/HTMLResource.h"
#include "sigil_constants.h"
#include "Tabs/FlowTab.h"
#include "Tabs/WellFormedCheckComponent.h"
#include "ViewEditors/BookViewEditor.h"
#include "ViewEditors/BookViewPreview.h"
#include "ViewEditors/CodeViewEditor.h"

static const QString SETTINGS_GROUP = "flowtab";

FlowTab::FlowTab(HTMLResource &resource,
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
    m_views(new QStackedWidget(this)),
    m_wBookView(NULL),
    m_wCodeView(NULL),
    m_ViewState(view_state),
    m_previousViewState(view_state),
    m_WellFormedCheckComponent(*new WellFormedCheckComponent(*this, parent)),
    m_safeToLoad(false),
    m_initialLoad(true),
    m_bookViewNeedsReload(false),
    m_grabFocus(grab_focus),
    m_suspendTabReloading(false),
    m_defaultCaretLocationToTop(false)
{
    // Loading a flow tab can take a while. We set the wait
    // cursor and clear it at the end of the delayed initialization.
    QApplication::setOverrideCursor(Qt::WaitCursor);

    if (view_state == MainWindow::ViewState_BookView) {
        CreateBookViewIfRequired(false);
    } else {
        CreateCodeViewIfRequired(false);
    }

    m_Layout.addWidget(m_views);
    LoadSettings();

    // We need to set this in the constructor too,
    // so that the ContentTab focus handlers don't
    // get called when the tab is created.
    if (view_state == MainWindow::ViewState_BookView) {
        setFocusProxy(m_wBookView);
        ConnectBookViewSignalsToSlots();
    } else {
        setFocusProxy(m_wCodeView);
        ConnectCodeViewSignalsToSlots();
    }

    // We perform delayed initialization after the widget is on
    // the screen. This way, the user perceives less load time.
    QTimer::singleShot(0, this, SLOT(DelayedInitialization()));
}

FlowTab::~FlowTab()
{
    // Explicitly disconnect signals because Modified is causing the ResourceModified
    // function to be called after we delete BV and PV later in this destructor.
    // No idea how that's possible but this prevents a segfault...
    disconnect();
    m_WellFormedCheckComponent.deleteLater();

    if (m_wBookView) {
        delete m_wBookView;
        m_wBookView = 0;
    }

    if (m_wCodeView) {
        delete m_wCodeView;
        m_wCodeView = 0;
    }

    if (m_views) {
        delete(m_views);
        m_views = 0;
    }
}

void FlowTab::CreateBookViewIfRequired(bool is_delayed_load)
{
    if (m_wBookView) {
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_wBookView = new BookViewEditor(this);
    m_views->addWidget(m_wBookView);
    m_bookViewNeedsReload = true;

    if (is_delayed_load) {
        ConnectBookViewSignalsToSlots();
    }

    m_wBookView->Zoom();
    QApplication::restoreOverrideCursor();
}

void FlowTab::CreateCodeViewIfRequired(bool is_delayed_load)
{
    if (m_wCodeView) {
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_wCodeView = new CodeViewEditor(CodeViewEditor::Highlight_XHTML, true, this);
    m_wCodeView->SetReformatHTMLEnabled(true);
    m_views->addWidget(m_wCodeView);

    if (is_delayed_load) {
        ConnectCodeViewSignalsToSlots();
        // CodeView (if already loaded) will be directly hooked into the TextResource and
        // will not need reloading. However if the tab was not in Code View at tab opening
        // then we must populate it now.
        m_wCodeView->CustomSetDocument(m_HTMLResource.GetTextDocumentForWriting());
        // Zoom assignment only works after the document has been loaded
        m_wCodeView->Zoom();
    }

    QApplication::restoreOverrideCursor();
}

void FlowTab::DelayedInitialization()
{
    if (m_wBookView) {
        m_safeToLoad = true;
        LoadTabContent();
    } else if (m_wCodeView) {
        m_wCodeView->CustomSetDocument(m_HTMLResource.GetTextDocumentForWriting());
        // Zoom factor for CodeView can only be set when document has been loaded.
        m_wCodeView->Zoom();
    }

    switch (m_ViewState) {
        case MainWindow::ViewState_CodeView: {
            CodeView();

            if (m_PositionToScrollTo > 0) {
                m_wCodeView->ScrollToPosition(m_PositionToScrollTo);
            } else if (m_LineToScrollTo > 0) {
                m_wCodeView->ScrollToLine(m_LineToScrollTo);
            } else {
                m_wCodeView->ScrollToFragment(m_FragmentToScroll.toString());
            }

            break;
        }

        case MainWindow::ViewState_BookView:
        default:
            BookView();

            if (!m_CaretLocationToScrollTo.isEmpty()) {
                m_wBookView->ExecuteCaretUpdate(m_CaretLocationToScrollTo);
            } else {
                m_wBookView->ScrollToFragment(m_FragmentToScroll.toString());
            }

            break;
    }

    m_initialLoad = false;
    // Only now will we wire up monitoring of ResourceChanged, to prevent
    // unnecessary saving and marking of the resource for reloading.
    DelayedConnectSignalsToSlots();
    // Cursor set in constructor
    QApplication::restoreOverrideCursor();
}

MainWindow::ViewState FlowTab::GetViewState()
{
    return m_ViewState;
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

    // Our well formed check will be run if switching to BV. If this check fails,
    // that check will set our tab viewstate to be in CV.
    if (new_view_state == MainWindow::ViewState_BookView && !IsDataWellFormed()) {
        return false;
    }

    // We do a save (if pending changes) before switching to ensure we don't lose
    // any unsaved data in the current view, as cannot rely on lost focus happened.
    SaveTabContent();
    // Track our previous view state for the purposes of caret syncing
    m_previousViewState = m_ViewState;
    m_ViewState = new_view_state;

    if (new_view_state == MainWindow::ViewState_CodeView) {
        // As CV is directly hooked to the QTextDocument, we never have to "Load" content
        // except for when creating the CV control for the very first time.
        CreateCodeViewIfRequired();
        CodeView();
    } else {
        CreateBookViewIfRequired();
        LoadTabContent();
        BookView();
    }

    return true;
}

bool FlowTab::IsLoadingFinished()
{
    bool is_finished = true;

    if (m_wCodeView) {
        is_finished = m_wCodeView->IsLoadingFinished();
    }

    if (is_finished && m_wBookView) {
        is_finished = m_wBookView->IsLoadingFinished();
    }

    return is_finished;
}

bool FlowTab::IsModified()
{
    bool is_modified = false;

    if (m_wCodeView) {
        is_modified = is_modified || m_wCodeView->document()->isModified();
    }

    if (!is_modified && m_wBookView) {
        is_modified = is_modified || m_wBookView->isModified();
    }

    return is_modified;
}

void FlowTab::BookView()
{
    if (!IsDataWellFormed()) {
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    CreateBookViewIfRequired();
    m_views->setCurrentIndex(m_views->indexOf(m_wBookView));
    setFocusProxy(m_wBookView);

    // We will usually want focus in the tab, except when splitting opens this as a preceding tab.
    if (m_grabFocus) {
        m_wBookView->GrabFocus();
    }

    m_grabFocus = true;

    // Ensure the caret is positioned corresponding to previous view of this tab
    if (m_previousViewState == MainWindow::ViewState_CodeView) {
        m_wBookView->StoreCaretLocationUpdate(m_wCodeView->GetCaretLocation());
    }

    m_wBookView->ExecuteCaretUpdate();
    QApplication::restoreOverrideCursor();
}

void FlowTab::CodeView()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_ViewState = MainWindow::ViewState_CodeView;
    CreateCodeViewIfRequired();
    m_views->setCurrentIndex(m_views->indexOf(m_wCodeView));
    m_wCodeView->SetDelayedCursorScreenCenteringRequired();
    setFocusProxy(m_wCodeView);

    // We will usually want focus in the tab, except when splitting opens this as a preceding tab.
    if (m_grabFocus) {
        m_wCodeView->setFocus();
    }

    m_grabFocus = true;

    // Ensure the caret is positioned corresponding to previous view of this tab
    if (m_previousViewState == MainWindow::ViewState_BookView) {
        m_wCodeView->StoreCaretLocationUpdate(m_wBookView->GetCaretLocation());
    }

    m_wCodeView->ExecuteCaretUpdate();
    QApplication::restoreOverrideCursor();
}

void FlowTab::LoadTabContent()
{
    // In CV, this call has nothing to do, as the resource is connected to QTextDocument.
    //        The only exception is initial load when control is created, done elsewhere.
    // In BV, we will only allow loading if the document is well formed, since loading the
    //        resource into BV and then saving will alter badly formed sections of text.
    if (m_ViewState == MainWindow::ViewState_BookView) {
        if (m_safeToLoad && m_bookViewNeedsReload) {
            m_wBookView->CustomSetDocument(m_HTMLResource.GetFullPath(), m_HTMLResource.GetText());
            m_bookViewNeedsReload = false;
        }
    }
}

void FlowTab::SaveTabContent()
{
    // In PV, content is read-only so nothing to do
    // In CV, the connection between QPlainTextEdit and the underlying QTextDocument
    //        means the resource already is "saved". We just need to reset modified state.
    // In BV, we only need to save the BV HTML into the resource if user has modified it,
    //        which will trigger ResourceModified() to set flag to say PV needs reloading.
    if (m_ViewState == MainWindow::ViewState_BookView && m_wBookView && m_wBookView->IsModified()) {
        SettingsStore ss;
        QString html = m_wBookView->GetHtml();
        if (ss.cleanOn() & CLEANON_OPEN) {
            html = CleanSource::Clean(html);
        }
        m_HTMLResource.SetText(html);
        m_wBookView->ResetModified();
        m_safeToLoad = true;
    }

    // Either from being in CV or saving from BV above we now reset the resource to say no user changes unsaved.
    m_HTMLResource.GetTextDocumentForWriting().setModified(false);
}

void FlowTab::ResourceModified()
{
    // This slot tells us that the underlying HTML resource has been changed
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        // It could be the user has done a Replace All on underlying resource, so reset our well formed check.
        m_safeToLoad = false;
        // When the underlying resource has been modified, it replaces the whole QTextDocument which
        // causes cursor position to move to bottom of the document. We will have captured the location
        // of the caret prior to replacing in the ResourceTextChanging() slot, so now we can restore it.
        m_wCodeView->ExecuteCaretUpdate(m_defaultCaretLocationToTop);
        m_defaultCaretLocationToTop = false;
    }

    m_bookViewNeedsReload = true;

    EmitUpdatePreview();
}

void FlowTab::LinkedResourceModified()
{
    MainWindow::clearMemoryCaches();
    ResourceModified();
    ReloadTabIfPending();
}

void FlowTab::ResourceTextChanging()
{
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        // We need to store the caret location so it can be restored later
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

    // Reload BV if the resource was marked as changed outside of the editor.
    if ((m_bookViewNeedsReload && m_ViewState == MainWindow::ViewState_BookView)) {
        LoadTabContent();
    }
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
    if (m_wBookView) {
        m_wBookView->UpdateDisplay();
    }

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
    // We always want this to happen, regardless of what the current view is.
    if (m_wCodeView) {
        m_wCodeView->RefreshSpellingHighlighting();
    }
}


bool FlowTab::CutEnabled()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView->pageAction(QWebPage::Cut)->isEnabled();
    } else if (m_ViewState == MainWindow::ViewState_CodeView) {
        return m_wCodeView->textCursor().hasSelection();
    }

    return false;
}

bool FlowTab::CopyEnabled()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView->pageAction(QWebPage::Copy)->isEnabled();
    } else if (m_ViewState == MainWindow::ViewState_CodeView) {
        return m_wCodeView->textCursor().hasSelection();
    }

    return false;
}

bool FlowTab::PasteEnabled()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView->pageAction(QWebPage::Paste)->isEnabled();
    } else if (m_ViewState == MainWindow::ViewState_CodeView) {
        return m_wCodeView->canPaste();
    }

    return false;
}

bool FlowTab::DeleteLineEnabled()
{
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        return !m_wCodeView->document()->isEmpty();
    }

    return false;
}

bool FlowTab::RemoveFormattingEnabled()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView->pageAction(QWebPage::RemoveFormat)->isEnabled();
    }

    if (m_ViewState == MainWindow::ViewState_CodeView) {
        return m_wCodeView->IsCutCodeTagsAllowed();
    }

    return false;
}

bool FlowTab::InsertClosingTagEnabled()
{
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        return m_wCodeView->IsInsertClosingTagAllowed();
    }

    return false;
}

bool FlowTab::AddToIndexEnabled()
{
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        return m_wCodeView->IsAddToIndexAllowed();
    } else if (m_ViewState == MainWindow::ViewState_BookView) {
        return true;
    }

    return false;
}

bool FlowTab::MarkForIndexEnabled()
{
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        return true;
    } else if (m_ViewState == MainWindow::ViewState_BookView) {
        return true;
    }

    return false;
}

bool FlowTab::InsertIdEnabled()
{
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        return m_wCodeView->IsInsertIdAllowed();
    } else if (m_ViewState == MainWindow::ViewState_BookView) {
        return true;
    }

    return false;
}

bool FlowTab::InsertHyperlinkEnabled()
{
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        return m_wCodeView->IsInsertHyperlinkAllowed();
    } else if (m_ViewState == MainWindow::ViewState_BookView) {
        return true;
    }

    return false;
}

bool FlowTab::InsertSpecialCharacterEnabled()
{
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        return true;
    } else if (m_ViewState == MainWindow::ViewState_BookView) {
        return true;
    }

    return false;
}

bool FlowTab::InsertFileEnabled()
{
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        return m_wCodeView->IsInsertFileAllowed();
    } else if (m_ViewState == MainWindow::ViewState_BookView) {
        return true;
    }

    return false;
}

bool FlowTab::ToggleAutoSpellcheckEnabled()
{
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        return true;
    }

    return false;
}

bool FlowTab::ViewStatesEnabled()
{
    if (m_ViewState == MainWindow::ViewState_BookView ||
        m_ViewState == MainWindow::ViewState_CodeView) {
        return true;
    }

    return false;
}

void FlowTab::GoToCaretLocation(QList<ViewEditor::ElementIndex> location)
{
    if (location.isEmpty()) {
        return;
    }
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->StoreCaretLocationUpdate(location);
        m_wBookView->ExecuteCaretUpdate();
    } else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->StoreCaretLocationUpdate(location);
        m_wCodeView->ExecuteCaretUpdate();
    }
}

QList<ViewEditor::ElementIndex> FlowTab::GetCaretLocation()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView->GetCaretLocation();
    } else if (m_ViewState == MainWindow::ViewState_CodeView) {
        return m_wCodeView->GetCaretLocation();
    }

    return QList<ViewEditor::ElementIndex>();
}

QString FlowTab::GetCaretLocationUpdate() const
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView->GetCaretLocationUpdate();
    }

    return QString();
}

QString FlowTab::GetDisplayedCharacters()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView->GetDisplayedCharacters();
    }

    return "";
}

QString FlowTab::GetText()
{
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        return m_wCodeView->toPlainText();
    } else if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView->GetHtml();
    }

    return "";
}

int FlowTab::GetCursorPosition() const
{
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        return m_wCodeView->GetCursorPosition();
    }

    return -1;
}

int FlowTab::GetCursorLine() const
{
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        return m_wCodeView->GetCursorLine();
    }

    return -1;
}

int FlowTab::GetCursorColumn() const
{
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        return m_wCodeView->GetCursorColumn();
    }

    return -1;
}

float FlowTab::GetZoomFactor() const
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView->GetZoomFactor();
    } else if (m_ViewState == MainWindow::ViewState_CodeView) {
        return m_wCodeView->GetZoomFactor();
    }

    return 1;
}

void FlowTab::SetZoomFactor(float new_zoom_factor)
{
    // We need to set a wait cursor for the Book View
    // since zoom operations take some time in it.
    if (m_ViewState == MainWindow::ViewState_BookView) {
        QApplication::setOverrideCursor(Qt::WaitCursor);

        if (m_wBookView) {
            m_wBookView->SetZoomFactor(new_zoom_factor);
        }

        QApplication::restoreOverrideCursor();
    } else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->SetZoomFactor(new_zoom_factor);
    }
}

Searchable *FlowTab::GetSearchableContent()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView;
    } else if (m_ViewState == MainWindow::ViewState_CodeView) {
        return m_wCodeView;
    }

    return NULL;
}


void FlowTab::ScrollToFragment(const QString &fragment)
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->ScrollToFragment(fragment);
    } else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->ScrollToFragment(fragment);
    }
}

void FlowTab::ScrollToLine(int line)
{
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->ScrollToLine(line);
    }
    // Scrolling to top if BV/CV requests a line allows
    // view to be reset to top for links
    else if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->ScrollToTop();
    }
}

void FlowTab::ScrollToPosition(int cursor_position)
{
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->ScrollToPosition(cursor_position);
    }
}

void FlowTab::ScrollToCaretLocation(QString caret_location_update)
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->ExecuteCaretUpdate(caret_location_update);
    }
}

void FlowTab::ScrollToTop()
{
    if (m_wBookView) {
        m_wBookView->ScrollToTop();
    }

    if (m_wCodeView) {
        m_wCodeView->ScrollToTop();
    }
}

void FlowTab::AutoFixWellFormedErrors()
{
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        int pos = m_wCodeView->GetCursorPosition();
        m_wCodeView->ReplaceDocumentText(CleanSource::ToValidXHTML(m_wCodeView->toPlainText()));
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
    if (m_ViewState == MainWindow::ViewState_BookView) {
        // If we are in BookView, then we know the data must be well formed, as even if edits have
        // taken place QWebView will have retained the XHTML integrity.
        m_safeToLoad = true;
    } else {
        // We are in PV or CV. In either situation the xhtml from CV could be invalid as the user may
        // have switched to PV to preview it (as they are allowed to do).
        // It is also possible that they opened the tab in PV as the initial load and CV is not loaded.
        // We are doing a well formed check, but we can only do it on the CV text if CV has been loaded.
        // So lets play safe and have a fallback to use the resource text if CV is not loaded yet.
        XhtmlDoc::WellFormedError error = (m_wCodeView != NULL)
                                          ? XhtmlDoc::WellFormedErrorForSource(m_wCodeView->toPlainText())
                                          : XhtmlDoc::WellFormedErrorForSource(m_HTMLResource.GetText());
        m_safeToLoad = error.line == -1;

        if (!m_safeToLoad) {
            if (m_ViewState != MainWindow::ViewState_CodeView) {
                CodeView();
            }

            m_WellFormedCheckComponent.DemandAttentionIfAllowed(error);
        }
    }

    return m_safeToLoad;
}

void FlowTab::Undo()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->Undo();
    } else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->undo();
    }
}

void FlowTab::Redo()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->Redo();
    } else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->redo();
    }
}

void FlowTab::Cut()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->page()->triggerAction(QWebPage::Cut);
    } else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->cut();
    }
}

void FlowTab::Copy()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->page()->triggerAction(QWebPage::Copy);
    } else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->copy();
    }
}

void FlowTab::Paste()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->paste();
    } else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->paste();
    }
}

void FlowTab::DeleteLine()
{
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->DeleteLine();
    }
}

bool FlowTab::MarkSelection()
{
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        return m_wCodeView->MarkSelection();
    }
    return false;
}

bool FlowTab::ClearMarkedText()
{
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        return m_wCodeView->ClearMarkedText();
    }
    return false;
}

void FlowTab::SplitSection()
{
    if (!IsDataWellFormed()) {
        return;
    }

    if (m_ViewState == MainWindow::ViewState_BookView) {
        const QString &content = m_wBookView->SplitSection();
        // The webview visually has split off the text, but not yet saved to the underlying resource
        SaveTabContent();
        emit OldTabRequest(content, m_HTMLResource);
    } else if (m_ViewState == MainWindow::ViewState_CodeView && m_wCodeView) {
        emit OldTabRequest(m_wCodeView->SplitSection(), m_HTMLResource);
    }
}

void FlowTab::InsertSGFSectionMarker()
{
    if (!IsDataWellFormed()) {
        return;
    }

    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->InsertHtml(BREAK_TAG_INSERT);
    } else if (m_ViewState == MainWindow::ViewState_CodeView && m_wCodeView) {
        m_wCodeView->InsertSGFSectionMarker();
    }
}

void FlowTab::InsertClosingTag()
{
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->InsertClosingTag();
    }
}

void FlowTab::AddToIndex()
{
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->AddToIndex();
    } else if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->AddToIndex();
    }
}

bool FlowTab::MarkForIndex(const QString &title)
{
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        return m_wCodeView->MarkForIndex(title);
    } else if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView->MarkForIndex(title);
    }

    return false;
}

QString FlowTab::GetAttributeId()
{
    QString attribute_value;

    if (m_ViewState == MainWindow::ViewState_CodeView) {
        // We are only interested in ids on <a> anchor elements
        attribute_value = m_wCodeView->GetAttributeId();
    } else if (m_ViewState == MainWindow::ViewState_BookView) {
        attribute_value = m_wBookView->GetAncestorTagAttributeValue("id", ANCHOR_TAGS);
    }

    return attribute_value;
}

QString FlowTab::GetAttributeHref()
{
    QString attribute_value;

    if (m_ViewState == MainWindow::ViewState_CodeView) {
        attribute_value = m_wCodeView->GetAttribute("href", ANCHOR_TAGS, false, true);
    } else if (m_ViewState == MainWindow::ViewState_BookView) {
        attribute_value = m_wBookView->GetAncestorTagAttributeValue("href", ANCHOR_TAGS);
    }

    return attribute_value;
}

QString FlowTab::GetAttributeIndexTitle()
{
    QString attribute_value;

    if (m_ViewState == MainWindow::ViewState_CodeView) {
        attribute_value = m_wCodeView->GetAttribute("title", ANCHOR_TAGS, false, true);

        if (attribute_value.isEmpty()) {
            attribute_value = m_wCodeView->GetSelectedText();
        }
    } else if (m_ViewState == MainWindow::ViewState_BookView) {
        attribute_value = m_wBookView->GetAncestorTagAttributeValue("title", ANCHOR_TAGS);

        if (attribute_value.isEmpty()) {
            attribute_value = m_wBookView->GetSelectedText();
        }
    }

    return attribute_value;
}

QString FlowTab::GetSelectedText()
{
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        return m_wCodeView->GetSelectedText();
    } else if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView->GetSelectedText();
    }

    return "";
}

bool FlowTab::InsertId(const QString &id)
{
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        return m_wCodeView->InsertId(id);
    } else if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView->InsertId(id);
    }

    return false;
}

bool FlowTab::InsertHyperlink(const QString &href)
{
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        return m_wCodeView->InsertHyperlink(href);
    } else if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView->InsertHyperlink(href);
    }

    return false;
}

void FlowTab::InsertFile(QString html)
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->InsertHtml(html);
    } else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->insertPlainText(html);
    }
}

void FlowTab::PrintPreview()
{
    QPrintPreviewDialog *print_preview = new QPrintPreviewDialog(this);

    if (m_ViewState == MainWindow::ViewState_BookView) {
        connect(print_preview, SIGNAL(paintRequested(QPrinter *)), m_wBookView, SLOT(print(QPrinter *)));
    } else if (m_ViewState == MainWindow::ViewState_CodeView) {
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
        if (m_ViewState == MainWindow::ViewState_BookView) {
            m_wBookView->print(&printer);
        } else if (m_ViewState == MainWindow::ViewState_CodeView) {
            m_wCodeView->print(&printer);
        }
    }
}

void FlowTab::Bold()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->page()->triggerAction(QWebPage::ToggleBold);
    } else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->ToggleFormatSelection("b", "font-weight", "bold");
    }
}

void FlowTab::Italic()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->page()->triggerAction(QWebPage::ToggleItalic);
    } else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->ToggleFormatSelection("i", "font-style", "italic");
    }
}

void FlowTab::Underline()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->page()->triggerAction(QWebPage::ToggleUnderline);
    } else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->ToggleFormatSelection("u", "text-decoration", "underline");
    }
}

// the strike tag has been deprecated, the del tag is still okay
void FlowTab::Strikethrough()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->ExecCommand("strikeThrough");
    } else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->ToggleFormatSelection("del", "text-decoration", "line-through");
    }
}

void FlowTab::Subscript()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->page()->triggerAction(QWebPage::ToggleSubscript);
    } else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->ToggleFormatSelection("sub");
    }
}

void FlowTab::Superscript()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->page()->triggerAction(QWebPage::ToggleSuperscript);
    } else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->ToggleFormatSelection("sup");
    }
}

void FlowTab::AlignLeft()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->ExecCommand("justifyLeft");
    } else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->FormatStyle("text-align", "left");
    }
}

void FlowTab::AlignCenter()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->ExecCommand("justifyCenter");
    } else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->FormatStyle("text-align", "center");
    }
}

void FlowTab::AlignRight()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->ExecCommand("justifyRight");
    } else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->FormatStyle("text-align", "right");
    }
}

void FlowTab::AlignJustify()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->ExecCommand("justifyFull");
    } else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->FormatStyle("text-align", "justify");
    }
}

void FlowTab::InsertBulletedList()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->ExecCommand("insertUnorderedList");
    }
}

void FlowTab::InsertNumberedList()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->ExecCommand("insertOrderedList");
    }
}

void FlowTab::DecreaseIndent()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->page()->triggerAction(QWebPage::Outdent);
    }
}

void FlowTab::IncreaseIndent()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->page()->triggerAction(QWebPage::Indent);
    }
}

void FlowTab::TextDirectionLeftToRight()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->page()->triggerAction(QWebPage::SetTextDirectionLeftToRight);
    } else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->FormatStyle("direction", "ltr");
    }
}

void FlowTab::TextDirectionRightToLeft()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->page()->triggerAction(QWebPage::SetTextDirectionRightToLeft);
    } else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->FormatStyle("direction", "rtl");
    }
}

void FlowTab::TextDirectionDefault()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->page()->triggerAction(QWebPage::SetTextDirectionDefault);
    } else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->FormatStyle("direction", "inherit");
    }
}

void FlowTab::ShowTag()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->ShowTag();
    }
}

void FlowTab::RemoveFormatting()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->page()->triggerAction(QWebPage::RemoveFormat);
    } else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->CutCodeTags();
    }
}

void FlowTab::ChangeCasing(const Utility::Casing casing)
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->ApplyCaseChangeToSelection(casing);
    } else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->ApplyCaseChangeToSelection(casing);
    }
}

void FlowTab::HeadingStyle(const QString &heading_type, bool preserve_attributes)
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        QChar last_char = heading_type[ heading_type.count() - 1 ];

        // For heading_type == "Heading #"
        if (last_char.isDigit()) {
            m_wBookView->FormatBlock("h" % QString(last_char), preserve_attributes);
        } else if (heading_type == "Normal") {
            m_wBookView->FormatBlock("p", preserve_attributes);
        }
    } else if (m_ViewState == MainWindow::ViewState_CodeView) {
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
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->GoToLinkOrStyle();
    }
}

void FlowTab::AddMisspelledWord()
{
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->AddMisspelledWord();
    }
}

void FlowTab::IgnoreMisspelledWord()
{
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->IgnoreMisspelledWord();
    }
}

bool FlowTab::BoldChecked()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView->pageAction(QWebPage::ToggleBold)->isChecked();
    } else {
        return ContentTab::BoldChecked();
    }
}

bool FlowTab::ItalicChecked()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView->pageAction(QWebPage::ToggleItalic)->isChecked();
    } else {
        return ContentTab::ItalicChecked();
    }
}

bool FlowTab::UnderlineChecked()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView->pageAction(QWebPage::ToggleUnderline)->isChecked();
    } else {
        return ContentTab::UnderlineChecked();
    }
}

bool FlowTab::StrikethroughChecked()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView->QueryCommandState("strikeThrough");
    } else {
        return ContentTab::StrikethroughChecked();
    }
}

bool FlowTab::SubscriptChecked()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView->QueryCommandState("subscript");
    } else {
        return ContentTab::SubscriptChecked();
    }
}

bool FlowTab::SuperscriptChecked()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView->QueryCommandState("superscript");
    } else {
        return ContentTab::SuperscriptChecked();
    }
}

bool FlowTab::AlignLeftChecked()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView->QueryCommandState("justifyLeft");
    } else {
        return ContentTab::AlignLeftChecked();
    }
}

bool FlowTab::AlignRightChecked()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView->QueryCommandState("justifyRight");
    } else {
        return ContentTab::AlignRightChecked();
    }
}

bool FlowTab::AlignCenterChecked()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView->QueryCommandState("justifyCenter");
    } else {
        return ContentTab::AlignCenterChecked();
    }
}

bool FlowTab::AlignJustifyChecked()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView->QueryCommandState("justifyFull");
    } else {
        return ContentTab::AlignJustifyChecked();
    }
}

bool FlowTab::BulletListChecked()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView->QueryCommandState("insertUnorderedList");
    } else {
        return ContentTab::BulletListChecked();
    }
}

bool FlowTab::NumberListChecked()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView->QueryCommandState("insertOrderedList");
    } else {
        return ContentTab::NumberListChecked();
    }
}

bool FlowTab::PasteClipNumber(int clip_number)
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView->PasteClipNumber(clip_number);
    } else if (m_ViewState == MainWindow::ViewState_CodeView) {
        return m_wCodeView->PasteClipNumber(clip_number);
    }
    return false;
}

bool FlowTab::PasteClipEntries(QList<ClipEditorModel::clipEntry *>clips)
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView->PasteClipEntries(clips);
    } else if (m_ViewState == MainWindow::ViewState_CodeView) {
        return m_wCodeView->PasteClipEntries(clips);
    }
    return false;
}

QString FlowTab::GetCaretElementName()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView->GetCaretElementName();
    } else {
        return ContentTab::GetCaretElementName();
    }
}

void FlowTab::SuspendTabReloading()
{
    // Call this function to prevent the currently displayed BV/PV from being
    // reloaded if a linked resource is changed. Automatic reloading can cause
    // issues if your code is attempting to manipulate the tab content concurrently.
    m_suspendTabReloading = true;
}

void FlowTab::ResumeTabReloading()
{
    // Call this function to resume reloading of BV/PV in response to linked
    // resources changing. If a reload tab request is pending it is executed now.
    m_suspendTabReloading = false;

    // Force an immediate reload if there is one pending
    if (m_bookViewNeedsReload) {
        // Must save tab content first or else reload may not take place.
        SaveTabContent();
        ReloadTabIfPending();
    }
}

void FlowTab::DelayedConnectSignalsToSlots()
{
    connect(&m_HTMLResource, SIGNAL(TextChanging()), this, SLOT(ResourceTextChanging()));
    connect(&m_HTMLResource, SIGNAL(LinkedResourceUpdated()), this, SLOT(LinkedResourceModified()));
    connect(&m_HTMLResource, SIGNAL(Modified()), this, SLOT(ResourceModified()));
    connect(&m_HTMLResource, SIGNAL(LoadedFromDisk()), this, SLOT(ReloadTabIfPending()));
}

void FlowTab::ConnectBookViewSignalsToSlots()
{
    connect(m_wBookView, SIGNAL(ZoomFactorChanged(float)), this, SIGNAL(ZoomFactorChanged(float)));
    connect(m_wBookView, SIGNAL(selectionChanged()), this, SIGNAL(SelectionChanged()));
    connect(m_wBookView, SIGNAL(FocusLost(QWidget *)), this, SLOT(LeaveEditor(QWidget *)));
    connect(m_wBookView, SIGNAL(InsertFile()), this, SIGNAL(InsertFileRequest()));
    connect(m_wBookView, SIGNAL(LinkClicked(const QUrl &)), this, SIGNAL(LinkClicked(const QUrl &)));
    connect(m_wBookView, SIGNAL(ClipboardSaveRequest()),    this, SIGNAL(ClipboardSaveRequest()));
    connect(m_wBookView, SIGNAL(ClipboardRestoreRequest()), this, SIGNAL(ClipboardRestoreRequest()));
    connect(m_wBookView, SIGNAL(InsertedFileOpenedExternally(const QString &)), this, SIGNAL(InsertedFileOpenedExternally(const QString &)));
    connect(m_wBookView, SIGNAL(InsertedFileSaveAs(const QUrl &)), this, SIGNAL(InsertedFileSaveAs(const QUrl &)));
    connect(m_wBookView, SIGNAL(ShowStatusMessageRequest(const QString &)), this, SIGNAL(ShowStatusMessageRequest(const QString &)));
    connect(m_wBookView, SIGNAL(OpenClipEditorRequest(ClipEditorModel::clipEntry *)), this, SIGNAL(OpenClipEditorRequest(ClipEditorModel::clipEntry *)));
    connect(m_wBookView, SIGNAL(OpenIndexEditorRequest(IndexEditorModel::indexEntry *)), this, SIGNAL(OpenIndexEditorRequest(IndexEditorModel::indexEntry *)));
    connect(m_wBookView, SIGNAL(textChanged()), this, SLOT(EmitContentChanged()));
    connect(m_wBookView, SIGNAL(BVInspectElement()), this, SIGNAL(InspectElement()));
    connect(m_wBookView, SIGNAL(PageUpdated()), this, SLOT(EmitUpdatePreview()));
    connect(m_wBookView, SIGNAL(PageClicked()), this, SLOT(EmitUpdatePreviewImmediately()));
    connect(m_wBookView, SIGNAL(PageOpened()), this, SLOT(EmitUpdatePreviewImmediately()));
    connect(m_wBookView, SIGNAL(DocumentLoaded()), this, SLOT(EmitUpdatePreviewImmediately()));
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
    connect(m_wCodeView, SIGNAL(PageClicked()), this, SLOT(EmitUpdatePreviewImmediately()));
    connect(m_wCodeView, SIGNAL(DocumentSet()), this, SLOT(EmitUpdatePreviewImmediately()));
    connect(m_wCodeView, SIGNAL(MarkSelectionRequest()), this, SIGNAL(MarkSelectionRequest()));
    connect(m_wCodeView, SIGNAL(ClearMarkedTextRequest()), this, SIGNAL(ClearMarkedTextRequest()));
}
