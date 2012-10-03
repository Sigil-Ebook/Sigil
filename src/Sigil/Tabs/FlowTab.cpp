/************************************************************************
**
**  Copyright (C) 2009, 2010, 2011  Strahinja Markovic  <strahinja.markovic@gmail.com>
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

#include <QtCore/QTimer>
#include <QtCore/QUrl>
#include <QtGui/QApplication>
#include <QtGui/QAction>
#include <QtGui/QDialog>
#include <QtGui/QLayout>
#include <QtGui/QPrinter>
#include <QtGui/QPrintDialog>
#include <QtGui/QPrintPreviewDialog>
#include <QtGui/QSplitter>
#include <QtGui/QStackedWidget>
#include <QtWebKit/QWebInspector>
#include <QtWebKit/QWebSettings>

#include "BookManipulation/CleanSource.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "ResourceObjects/HTMLResource.h"
#include "sigil_constants.h"
#include "Tabs/FlowTab.h"
#include "Tabs/WellFormedCheckComponent.h"
#include "ViewEditors/BookViewEditor.h"
#include "ViewEditors/BookViewPreview.h"
#include "ViewEditors/CodeViewEditor.h"

// These correspond to the order the views are added to the stacked widget.
#define BV_INDEX 0
#define PV_INDEX 1
#define CV_INDEX 2

static const QString SETTINGS_GROUP = "flowtab";

FlowTab::FlowTab(HTMLResource& resource,
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
    m_pvVSplitter(new QSplitter(this)),
    m_wBookView(new BookViewEditor(this)),
    m_wBookPreview(new BookViewPreview(this)),
    m_wCodeView(new CodeViewEditor(CodeViewEditor::Highlight_XHTML, true, this)),
    m_inspector(new QWebInspector(this)),
    m_ViewState(view_state),
    m_previousViewState(view_state),
    m_WellFormedCheckComponent(*new WellFormedCheckComponent(*this)),
    m_safeToLoad(false),
    m_initialLoad(true),
    m_BookPreviewNeedReload(false),
    m_grabFocus(grab_focus),
    m_suspendTabReloading(false),
    m_defaultCaretLocationToTop(false)
{
    // Loading a flow tab can take a while. We set the wait
    // cursor and clear it at the end of the delayed initialization.
    QApplication::setOverrideCursor(Qt::WaitCursor);

    m_inspector->setPage(m_wBookPreview->page());

    m_pvVSplitter->setOrientation(Qt::Vertical);
    m_pvVSplitter->addWidget(m_wBookPreview);
    m_pvVSplitter->addWidget(m_inspector);
    m_pvVSplitter->setSizes(QList<int>() << 400 << 200);

    m_views->addWidget(m_wBookView);
    m_views->addWidget(m_pvVSplitter);
    m_views->addWidget(m_wCodeView);

    m_Layout.addWidget(m_views);

    LoadSettings();

    // We need to set this in the constructor too,
    // so that the ContentTab focus handlers don't
    // get called when the tab is created.
    if (view_state == MainWindow::ViewState_BookView) {
        setFocusProxy(m_wBookView);
    }
    else {
        setFocusProxy(m_wCodeView);
    }

    ConnectSignalsToSlots();

    // We perform delayed initialization after the widget is on
    // the screen. This way, the user perceives less load time.
    QTimer::singleShot( 0, this, SLOT( DelayedInitialization() ) );
}

FlowTab::~FlowTab()
{
    // Explicitly disconnect this signal because it's causing the ResourceModified
    // function to be called after we delete BV and PV later in this destructor.
    // No idea how that's possible but this prevents a segfault...
    disconnect(&m_HTMLResource, SIGNAL(Modified()), this, SLOT(ResourceModified()));

    m_WellFormedCheckComponent.deleteLater();
    if (m_wBookView) {
        delete m_wBookView;
        m_wBookView = 0;
    }
    if (m_wCodeView) {
        delete m_wCodeView;
        m_wCodeView = 0;
    }
    // BookViewPreview must be deleted before QWebInspector.
    // BookViewPreview's QWebPage is linked to the QWebInspector
    // and when deleted it will send a message to the linked QWebInspector
    // to remove the assoication. If QWebInspector is deleted before
    // BookViewPReview, BookViewPreview will try to access the deleted
    // QWebInspector and the application will SegFault. This is an issue
    // with how QWebPages interface wtih QWebInspector.
    if (m_wBookPreview) {
        delete m_wBookPreview;
        m_wBookPreview = 0;
    }
    if (m_inspector) {
        delete m_inspector;
        m_inspector = 0;
    }
    if (m_pvVSplitter) {
        delete m_pvVSplitter;
        m_pvVSplitter = 0;
    }
    if (m_views) {
        delete(m_views);
        m_views = 0;
    }
}

MainWindow::ViewState FlowTab::GetViewState()
{
    return m_ViewState;
}

bool FlowTab::IsModified()
{
    return m_wBookView->isModified() || m_wCodeView->document()->isModified();
}

bool FlowTab::CutEnabled()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView->pageAction(QWebPage::Cut)->isEnabled();
    }
    else if (m_ViewState == MainWindow::ViewState_CodeView) {
        return m_wCodeView->textCursor().hasSelection();
    }

    return false;
}

bool FlowTab::CopyEnabled()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView->pageAction(QWebPage::Copy)->isEnabled();
    }
    else if (m_ViewState == MainWindow::ViewState_PreviewView) {
        return m_wBookPreview->pageAction(QWebPage::Copy)->isEnabled();
    }
    else if (m_ViewState == MainWindow::ViewState_CodeView) {
        return m_wCodeView->textCursor().hasSelection();
    }

    return false;
}

bool FlowTab::PasteEnabled()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView->pageAction(QWebPage::Paste)->isEnabled();
    }
    else if (m_ViewState == MainWindow::ViewState_CodeView) {
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
    }

    return false;
}

bool FlowTab::MarkForIndexEnabled()
{
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        return true;
    }
    else if (m_ViewState == MainWindow::ViewState_BookView) {
        return true;
    }

    return false;
}

bool FlowTab::InsertIdEnabled()
{
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        return m_wCodeView->IsInsertIdAllowed();
    }
    else if (m_ViewState == MainWindow::ViewState_BookView) {
        return true;
    }

    return false;
}

bool FlowTab::InsertHyperlinkEnabled()
{
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        return m_wCodeView->IsInsertHyperlinkAllowed();
    }
    else if (m_ViewState == MainWindow::ViewState_BookView) {
        return true;
    }

    return false;
}

bool FlowTab::InsertSpecialCharacterEnabled()
{
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        return true;
    }
    else if (m_ViewState == MainWindow::ViewState_BookView) {
        return true;
    }

    return false;
}

bool FlowTab::InsertImageEnabled()
{
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        return m_wCodeView->IsInsertImageAllowed();
    }
    else if (m_ViewState == MainWindow::ViewState_BookView) {
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
        m_ViewState == MainWindow::ViewState_PreviewView ||
        m_ViewState == MainWindow::ViewState_CodeView ) {
        return true;
    }

    return false;
}

QString FlowTab::GetCaretLocationUpdate() const
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView->GetCaretLocationUpdate();
    }
    else if (m_ViewState == MainWindow::ViewState_PreviewView) {
        return m_wBookPreview->GetCaretLocationUpdate();
    }

    return QString();
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
    }
    else if (m_ViewState == MainWindow::ViewState_PreviewView) {
            return m_wBookPreview->GetZoomFactor();
    }
    else if (m_ViewState == MainWindow::ViewState_CodeView) {
        return m_wCodeView->GetZoomFactor();
    }

    return 1;
}

void FlowTab::SetZoomFactor(float new_zoom_factor)
{
    // We need to set a wait cursor for the Book View
    // since zoom operations take some time in it.
    if (m_ViewState == MainWindow::ViewState_BookView || m_ViewState == MainWindow::ViewState_PreviewView) {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        m_wBookView->SetZoomFactor(new_zoom_factor);
        m_wBookPreview->SetZoomFactor(new_zoom_factor);
        QApplication::restoreOverrideCursor();
    }
    else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->SetZoomFactor(new_zoom_factor);
    }
}

void FlowTab::UpdateDisplay()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);

    m_pvVSplitter->restoreState(settings.value("pv_splitter").toByteArray());

    settings.endGroup();

    m_wBookView->UpdateDisplay();
    m_wBookPreview->UpdateDisplay();
    m_wCodeView->UpdateDisplay();
}

Searchable* FlowTab::GetSearchableContent()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView;
    }
    else if (m_ViewState == MainWindow::ViewState_PreviewView) {
        return m_wBookPreview;
    }
    else if (m_ViewState == MainWindow::ViewState_CodeView) {
        return m_wCodeView;
    }

    return NULL;
}


bool FlowTab::SetViewState(MainWindow::ViewState new_view_state)
{
    // Do we really need to do anything?
    // Ignore this function if we are in the middle of doing an initial load
    // of the content. We don't want it to save over the content with nothing
    // if this is called before the delayed initialization function is called.
    if (m_initialLoad) {
        return false;
    }
    if (new_view_state == m_ViewState) {
        return false;
    }
    if (new_view_state == MainWindow::ViewState_BookView) { 
        if ( !m_wBookView->IsLoadingFinished() || !IsDataWellFormed() ) {
            return false;
        }
    }

    // We do a save before changing to ensure we don't lose any unsaved data
    // in the previous view.
    SaveTabContent();
    m_previousViewState = m_ViewState;
    m_ViewState = new_view_state;
    LoadTabContent();

    if (new_view_state == MainWindow::ViewState_PreviewView) {
        // Since we have just loaded the tab content, clear the reload flag to prevent
        // the preview from being loaded again when EnterEditor() is called.
        m_BookPreviewNeedReload = false;
        SplitView();
    }
    else if (new_view_state == MainWindow::ViewState_CodeView) {
        CodeView();
    }
    else {
        m_BookPreviewNeedReload = false;
        BookView();
    }

    return true;
}

bool FlowTab::IsLoadingFinished()
{
    return m_wBookView->IsLoadingFinished() && m_wCodeView->IsLoadingFinished();
}

void FlowTab::ScrollToFragment(const QString &fragment)
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->ScrollToFragment(fragment);
    }
    else if (m_ViewState == MainWindow::ViewState_PreviewView) {
        m_wBookPreview->ScrollToFragment(fragment);
    }
    else if (m_ViewState == MainWindow::ViewState_CodeView) {
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
    else if (m_ViewState == MainWindow::ViewState_PreviewView) {
        m_wBookPreview->ScrollToTop();
    }
}

void FlowTab::ScrollToPosition(int cursor_position)
{
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->ScrollToPosition(cursor_position);
    }
}

void FlowTab::ScrollToCaretLocation( QString caret_location_update )
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->ExecuteCaretUpdate(caret_location_update);
    }
    else if (m_ViewState == MainWindow::ViewState_PreviewView) {
        m_wBookPreview->ExecuteCaretUpdate(caret_location_update);
    }
}

void FlowTab::ScrollToTop()
{
   m_wBookView->ScrollToTop();
   m_wBookPreview->ScrollToTop();
   m_wCodeView->ScrollToTop();
}

void FlowTab::AutoFixWellFormedErrors()
{
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        int pos = m_wCodeView->GetCursorPosition();
        m_wCodeView->ReplaceDocumentText(CleanSource::PrettyPrint(CleanSource::Clean(m_wCodeView->toPlainText())));
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
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_safeToLoad = true;
        return true;
    }

    bool well_formed = m_safeToLoad;
    if (!m_safeToLoad) {
        XhtmlDoc::WellFormedError error = XhtmlDoc::WellFormedErrorForSource(m_wCodeView->toPlainText());
        well_formed = error.line == -1;

        if (!well_formed) {
            m_safeToLoad = false;
            if (m_ViewState != MainWindow::ViewState_CodeView) {
                CodeView();
            }
            m_WellFormedCheckComponent.DemandAttentionIfAllowed(error);
        }
        else {
            m_safeToLoad = true;
        }
    }

    return well_formed;
}

void FlowTab::Undo()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->Undo();
    }
    else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->undo();
    }
}

void FlowTab::Redo()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->Redo();
    }
    else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->redo();
    }
}

void FlowTab::Cut()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->page()->triggerAction(QWebPage::Cut);
    }
    else if (m_ViewState == MainWindow::ViewState_PreviewView) {
        m_wBookPreview->page()->triggerAction(QWebPage::Cut);
    }
    else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->cut();
    }
}

void FlowTab::Copy()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->page()->triggerAction( QWebPage::Copy );
    }
    else if (m_ViewState == MainWindow::ViewState_PreviewView) {
        m_wBookPreview->page()->triggerAction(QWebPage::Copy);
    }
    else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->copy();
    }
}

void FlowTab::Paste()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->paste();
    }
    else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->paste();
    }
}

void FlowTab::DeleteLine()
{
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->DeleteLine();
    }
}

void FlowTab::SplitSection()
{
    if (!IsDataWellFormed()) {
        return;
    }

    if (m_ViewState == MainWindow::ViewState_BookView) {
        emit OldTabRequest( m_wBookView->SplitSection(), m_HTMLResource );
    }
    else if (m_ViewState == MainWindow::ViewState_CodeView) {
        emit OldTabRequest( m_wCodeView->SplitSection(), m_HTMLResource );
    }
}

void FlowTab::InsertSGFSectionMarker()
{
    if (!IsDataWellFormed()) {
        return;
    }

    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->InsertHtml(BREAK_TAG_INSERT);
    }
    else if (m_ViewState == MainWindow::ViewState_CodeView) {
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
    }
}

bool FlowTab::MarkForIndex(const QString &title)
{
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        return m_wCodeView->MarkForIndex(title);
    }
    else if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView->MarkForIndex(title);
    }

    return false;
}

QString FlowTab::GetAttributeId()
{
    QString attribute_value;
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        // We are only interested in ids on <a> anchor elements
        attribute_value = m_wCodeView->GetAttribute("id", ANCHOR_TAGS, false, true);
    }
    else if (m_ViewState == MainWindow::ViewState_BookView) {
        attribute_value = m_wBookView->GetAncestorTagAttributeValue("id", ANCHOR_TAGS);
    }

    return attribute_value;
}

QString FlowTab::GetAttributeHref()
{
    QString attribute_value;
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        attribute_value = m_wCodeView->GetAttribute("href", ANCHOR_TAGS, false, true);
    }
    else if (m_ViewState == MainWindow::ViewState_BookView) {
        attribute_value = m_wBookView->GetAncestorTagAttributeValue("href", ANCHOR_TAGS);
    }

    return attribute_value;
}

QString FlowTab::GetAttributeIndexTitle()
{
    QString attribute_value;
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        attribute_value = m_wCodeView->GetAttribute("title", ANCHOR_TAGS, false, true);
    }
    else if (m_ViewState == MainWindow::ViewState_BookView) {
        attribute_value = m_wBookView->GetAncestorTagAttributeValue("title", ANCHOR_TAGS);
    }

    return attribute_value;
}


bool FlowTab::InsertId(const QString &id)
{
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        return m_wCodeView->InsertId(id);
    }
    else if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView->InsertId(id);
    }
    return false;
}

bool FlowTab::InsertHyperlink(const QString &href)
{
    if (m_ViewState == MainWindow::ViewState_CodeView) {
        return m_wCodeView->InsertHyperlink(href);
    }
    else if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView->InsertHyperlink(href);
    }
    return false;
}

void FlowTab::InsertImage( const QString &image_path )
{
    QString html = QString("<img src=\"%1\"/>").arg(image_path);

    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->InsertHtml(html);
    }
    else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->insertPlainText(html);
    }
}

void FlowTab::PrintPreview()
{
    QPrintPreviewDialog *print_preview = new QPrintPreviewDialog(this);

    if (m_ViewState == MainWindow::ViewState_BookView) {
        connect(print_preview, SIGNAL(paintRequested(QPrinter *)), m_wBookView, SLOT(print(QPrinter *)));
    }
    else if (m_ViewState == MainWindow::ViewState_PreviewView) {
        connect(print_preview, SIGNAL(paintRequested(QPrinter *)), m_wBookPreview, SLOT(print(QPrinter *)));
    }
    else if (m_ViewState == MainWindow::ViewState_CodeView) {
        connect(print_preview, SIGNAL(paintRequested(QPrinter *)), m_wCodeView, SLOT(print(QPrinter *)));
    }
    else {
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
        }
        else if (m_ViewState == MainWindow::ViewState_PreviewView) {
            m_wBookPreview->print(&printer);
        }
        else if (m_ViewState == MainWindow::ViewState_CodeView) {
            m_wCodeView->print(&printer);
        }
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
    // Reload BV/PV if the resource was marked as changed outside of the editor.
    if (m_BookPreviewNeedReload && (m_ViewState == MainWindow::ViewState_PreviewView || m_ViewState == MainWindow::ViewState_BookView)) {
        LoadTabContent();
        m_BookPreviewNeedReload = false;
    }
}

void FlowTab::BookView()
{
    if (!IsDataWellFormed()) {
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    EnterBookView();

    m_views->setCurrentIndex(BV_INDEX);

    setFocusProxy(m_wBookView);
    // When opening this tab as a preceding tab such as when splitting a section
    // we do not want focus grabbed from the currently selected tab.
    if (m_grabFocus) {
        m_wBookView->GrabFocus();
    }
    m_grabFocus = true;

    if (m_previousViewState == MainWindow::ViewState_CodeView) {
        m_wBookView->StoreCaretLocationUpdate( m_wCodeView->GetCaretLocation() );
    }
    else if (m_previousViewState == MainWindow::ViewState_PreviewView) {
        m_wBookView->StoreCaretLocationUpdate( m_wBookPreview->GetCaretLocation() );
    }
    m_wBookView->ExecuteCaretUpdate();

    QApplication::restoreOverrideCursor();
}

void FlowTab::SplitView()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    m_views->setCurrentIndex(PV_INDEX);
    if (m_grabFocus) {
        m_wBookPreview->GrabFocus();
    }
    m_grabFocus = true;

    if (m_previousViewState == MainWindow::ViewState_BookView) {
        m_wBookPreview->StoreCaretLocationUpdate( m_wBookView->GetCaretLocation() );
    }
    else if (m_previousViewState == MainWindow::ViewState_CodeView) {
        m_wBookPreview->StoreCaretLocationUpdate( m_wCodeView->GetCaretLocation() );
    }
    m_wBookPreview->ExecuteCaretUpdate();

    QApplication::restoreOverrideCursor();
}

void FlowTab::CodeView()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    m_ViewState = MainWindow::ViewState_CodeView;

    m_views->setCurrentIndex(CV_INDEX);
    m_wCodeView->SetDelayedCursorScreenCenteringRequired();

    setFocusProxy(m_wCodeView);

    // Make sure the cursor is properly displayed
    if (m_grabFocus) {
        m_wCodeView->setFocus();
    }
    m_grabFocus = true;

    if (m_previousViewState == MainWindow::ViewState_BookView) {
        m_wCodeView->StoreCaretLocationUpdate( m_wBookView->GetCaretLocation() );
    }
    else if (m_previousViewState == MainWindow::ViewState_PreviewView) {
        m_wCodeView->StoreCaretLocationUpdate( m_wBookPreview->GetCaretLocation() );
    }
    m_wCodeView->ExecuteCaretUpdate();

    QApplication::restoreOverrideCursor();
}

void FlowTab::SaveTabContent()
{
    if (m_ViewState == MainWindow::ViewState_BookView && m_wBookView->IsModified()) {
        m_HTMLResource.SetText(m_wBookView->GetHtml());
        m_BookPreviewNeedReload = true;
        m_wBookView->ResetModified();
    }

    m_HTMLResource.GetTextDocumentForWriting().setModified(false);
    // Just because we have saved the tab content doesn't mean that it is valid
    // unless it was BookView, because it might not be well formed and the save
    // could have been just triggered by losing focus/switching tabs
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_safeToLoad = true;
    }
}

void FlowTab::LoadTabContent()
{
    if (!m_safeToLoad) {
        return;
    }

    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->CustomSetDocument(m_HTMLResource.GetFullPath(), m_HTMLResource.GetText());
    }
    else if (m_ViewState == MainWindow::ViewState_PreviewView) {
        m_wBookPreview->CustomSetDocument(m_HTMLResource.GetFullPath(), m_HTMLResource.GetText());
    }
}


void FlowTab::LoadSettings()
{
    UpdateDisplay();
    m_wCodeView->LoadSettings();
}

void FlowTab::ResourceModified()
{
    m_BookPreviewNeedReload = true;
    if ( m_ViewState == MainWindow::ViewState_CodeView ) {
        m_wCodeView->ExecuteCaretUpdate(m_defaultCaretLocationToTop);
        m_defaultCaretLocationToTop = false;
    }
}

void FlowTab::LinkedResourceModified()
{
    QWebSettings::clearMemoryCaches();
    ResourceModified();
    ReloadTabIfPending();
}

void FlowTab::ResourceTextChanging()
{
    if ( m_ViewState == MainWindow::ViewState_CodeView ) {
        // We need to store the caret location so it can be restored later
        m_wCodeView->StoreCaretLocationUpdate( m_wCodeView->GetCaretLocation() );
        // If the caret is at the very top of the document then our stored location 
        // will be empty. The problem is that when ResourceModified() fires next
        // it will have effectively moved the caret to the bottom of the document.
        // The default behaviour of CodeView is to "do nothing" if no location
        // is stored, in this one situation we want to override that to force
        // it to place the cursor at the top of the document.
        m_defaultCaretLocationToTop = true;
    }
}

void FlowTab::PVSplitterMoved(int pos, int index)
{
    Q_UNUSED(pos);
    Q_UNUSED(index);

    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);

    settings.setValue("pv_splitter", m_pvVSplitter->saveState());

    settings.endGroup();
}

void FlowTab::LeaveEditor(QWidget *editor)
{
    SaveTabContent();
}

void FlowTab::EnterEditor(QWidget *editor)
{
    // We don't want to do anything if we haven't already done an
    // initial (delayed) load. We especially don't want to do a save
    // over a valid file.
    if (!m_safeToLoad) {
        return;
    }

    // BookPreview is left out of this because we always want to reload with any current changes
    // from CodeView.
    if ((m_ViewState == MainWindow::ViewState_BookView && editor == m_wBookView) ||
         ((m_ViewState == MainWindow::ViewState_PreviewView || m_ViewState == MainWindow::ViewState_CodeView) && editor == m_wCodeView))
    {
        // Nothing to do because the view state matches the current view.
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    if (m_ViewState == MainWindow::ViewState_BookView) {
        EnterBookView();
    }
    else if (m_ViewState == MainWindow::ViewState_PreviewView) {
        EnterBookPreview();
    }
    else if (m_ViewState == MainWindow::ViewState_CodeView) {
        EnterCodeView();
    }
    QApplication::restoreOverrideCursor();

    EmitUpdateCursorPosition();
}

void FlowTab::DelayedInitialization()
{
    m_wBookView->CustomSetDocument(m_HTMLResource.GetFullPath(), m_HTMLResource.GetText());
    m_wBookPreview->CustomSetDocument(m_HTMLResource.GetFullPath(), m_HTMLResource.GetText());
    m_wCodeView->CustomSetDocument(m_HTMLResource.GetTextDocumentForWriting());

    // m_lastViewState in this instance is actually used a the initial view
    // state because there is no last view state.
    switch(m_ViewState) {
        case MainWindow::ViewState_CodeView:
        {
            CodeView();

            if (m_PositionToScrollTo > 0) {
                m_wCodeView->ScrollToPosition(m_PositionToScrollTo);
            }
            else if (m_LineToScrollTo > 0) {
                m_wCodeView->ScrollToLine(m_LineToScrollTo);
            }
            else {
                m_wCodeView->ScrollToFragment(m_FragmentToScroll.toString());
            }

            break;
        }
        case MainWindow::ViewState_PreviewView:
        {
            SplitView();
            if (!m_CaretLocationToScrollTo.isEmpty()) {
                m_wBookPreview->ExecuteCaretUpdate(m_CaretLocationToScrollTo);
            }
            else {
                m_wBookPreview->ScrollToFragment(m_FragmentToScroll.toString());
            }
            break;
        }
        // Don't care about these so ignore them.
        case MainWindow::ViewState_RawView:
        case MainWindow::ViewState_StaticView:
        default:
            BookView();
            if (!m_CaretLocationToScrollTo.isEmpty()) {
                m_wBookView->ExecuteCaretUpdate(m_CaretLocationToScrollTo);
            }
            else {
                m_wBookView->ScrollToFragment(m_FragmentToScroll.toString());
            }
            break;
    }

    m_wBookView->Zoom();
    m_wBookPreview->Zoom();
    m_wCodeView->Zoom();

    m_safeToLoad = true;
    m_initialLoad = false;

    // Only now will we wire up monitoring of ResourceChanged, to prevent
    // unnecessary saving and marking of the resource for reloading.
    QTimer::singleShot(0, this, SLOT(DelayedConnectSignalsToSlots()));

    // Cursor set in constructor
    QApplication::restoreOverrideCursor();
}

void FlowTab::EmitContentChanged()
{
    m_safeToLoad = false;
    emit ContentChanged();
}

void FlowTab::EmitUpdateCursorPosition()
{
    emit UpdateCursorPosition(GetCursorLine(), GetCursorColumn());
}

void FlowTab::EnterBookView()
{
    emit EnteringBookView();
}

void FlowTab::EnterBookPreview()
{
    emit EnteringBookPreview();
}

void FlowTab::EnterCodeView()
{
    emit EnteringCodeView();
}

void FlowTab::ReadSettings()
{
    // TODO: fill this... with what?
}

void FlowTab::WriteSettings()
{
    // TODO: fill this... with what?
}

void FlowTab::Bold()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->page()->triggerAction( QWebPage::ToggleBold );
    }
    else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->ToggleFormatSelection("b", "font-weight", "bold");
    }
}

void FlowTab::Italic()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->page()->triggerAction( QWebPage::ToggleItalic );
    }    
    else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->ToggleFormatSelection("i", "font-style", "italic");
    }
}

void FlowTab::Underline()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->page()->triggerAction( QWebPage::ToggleUnderline );
    }    
    else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->ToggleFormatSelection("u", "text-decoration", "underline");
    }
}

void FlowTab::Strikethrough()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->ExecCommand( "strikeThrough" );
    }    
    else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->ToggleFormatSelection("strike", "text-decoration", "line-through");
    }
}

void FlowTab::Subscript()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->page()->triggerAction( QWebPage::ToggleSubscript );
    }
    else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->ToggleFormatSelection("sub");
    }
}

void FlowTab::Superscript()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->page()->triggerAction( QWebPage::ToggleSuperscript );
    }
    else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->ToggleFormatSelection("sup");
    }
}

void FlowTab::AlignLeft()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->ExecCommand( "justifyLeft" );
    }
    else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->FormatStyle( "text-align", "left" );
    }    
}

void FlowTab::AlignCenter()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->ExecCommand( "justifyCenter" );
    }    
    else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->FormatStyle( "text-align", "center" );
    }    
}

void FlowTab::AlignRight()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->ExecCommand( "justifyRight" );
    }    
    else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->FormatStyle( "text-align", "right" );
    }    
}

void FlowTab::AlignJustify()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->ExecCommand( "justifyFull" );
    }    
    else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->FormatStyle( "text-align", "justify" );
    }    
}

void FlowTab::InsertBulletedList()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->ExecCommand( "insertUnorderedList" );
    }
}

void FlowTab::InsertNumberedList()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->ExecCommand( "insertOrderedList" );
    }
}

void FlowTab::DecreaseIndent()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->page()->triggerAction( QWebPage::Outdent );
    }
}

void FlowTab::IncreaseIndent()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->page()->triggerAction( QWebPage::Indent );
    }
}

void FlowTab::TextDirectionLeftToRight()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->page()->triggerAction( QWebPage::SetTextDirectionLeftToRight );
    }
    else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->FormatStyle( "direction", "ltr" );
    }    
}

void FlowTab::TextDirectionRightToLeft()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->page()->triggerAction( QWebPage::SetTextDirectionRightToLeft );
    }
    else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->FormatStyle( "direction", "rtl" );
    }    
}

void FlowTab::TextDirectionDefault()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->page()->triggerAction( QWebPage::SetTextDirectionDefault );
    }
    else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->FormatStyle( "direction", "inherit" );
    }    
}

void FlowTab::ShowTag()
{
    if (m_ViewState == MainWindow::ViewState_BookView)
        m_wBookView->ShowTag();
    else if (m_ViewState == MainWindow::ViewState_PreviewView)
        m_wBookPreview->ShowTag();
}

void FlowTab::RemoveFormatting()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->page()->triggerAction( QWebPage::RemoveFormat );
    }
    else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->CutCodeTags();
    }
}

void FlowTab::ChangeCasing( const Utility::Casing casing )
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->ApplyCaseChangeToSelection( casing );
    }
    else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->ApplyCaseChangeToSelection( casing );
    }
}

void FlowTab::HeadingStyle( const QString& heading_type, bool preserve_attributes )
{
    if (m_ViewState == MainWindow::ViewState_BookView)
    {
        QChar last_char = heading_type[ heading_type.count() - 1 ];

        // For heading_type == "Heading #"
        if ( last_char.isDigit() ) {
            m_wBookView->FormatBlock( "h" % QString( last_char ), preserve_attributes );
        }
        else if ( heading_type == "Normal" ) {
            m_wBookView->FormatBlock( "p", preserve_attributes );
        }
    }
    else if (m_ViewState == MainWindow::ViewState_CodeView)
    {
        QChar last_char = heading_type[ heading_type.count() - 1 ];

        // For heading_type == "Heading #"
        if ( last_char.isDigit() ) {
            m_wCodeView->FormatBlock( "h" % QString( last_char ), preserve_attributes );
        }
        else if ( heading_type == "Normal" ) {
            m_wCodeView->FormatBlock( "p", preserve_attributes );
        }
    }
}

void FlowTab::GoToLinkOrStyle()
{
    if (m_ViewState == MainWindow::ViewState_CodeView)
        m_wCodeView->GoToLinkOrStyle();
}

void FlowTab::AddMisspelledWord()
{
    if (m_ViewState == MainWindow::ViewState_CodeView)
        m_wCodeView->AddMisspelledWord();
}

void FlowTab::IgnoreMisspelledWord()
{
    if (m_ViewState == MainWindow::ViewState_CodeView)
        m_wCodeView->IgnoreMisspelledWord();
}

void FlowTab::RefreshSpellingHighlighting()
{
    // We always want this to happen, regardless of what the current view is.
    m_wCodeView->RefreshSpellingHighlighting();
}


bool FlowTab::BoldChecked()
{
    if (m_ViewState == MainWindow::ViewState_BookView)
        return m_wBookView->pageAction( QWebPage::ToggleBold )->isChecked();

    else
        return ContentTab::BoldChecked();
}


bool FlowTab::ItalicChecked()
{
    if (m_ViewState == MainWindow::ViewState_BookView)
        return m_wBookView->pageAction( QWebPage::ToggleItalic )->isChecked();

    else
        return ContentTab::ItalicChecked();
}


bool FlowTab::UnderlineChecked()
{
    if (m_ViewState == MainWindow::ViewState_BookView)
        return m_wBookView->pageAction( QWebPage::ToggleUnderline )->isChecked(); 

    else
        return ContentTab::UnderlineChecked();
}


bool FlowTab::StrikethroughChecked()
{
    if (m_ViewState == MainWindow::ViewState_BookView)
        return m_wBookView->QueryCommandState( "strikeThrough" );

    else
        return ContentTab::StrikethroughChecked();
}


bool FlowTab::SubscriptChecked()
{
    if (m_ViewState == MainWindow::ViewState_BookView)
        return m_wBookView->QueryCommandState( "subscript" );

    else
        return ContentTab::SubscriptChecked();
}


bool FlowTab::SuperscriptChecked()
{
    if (m_ViewState == MainWindow::ViewState_BookView)
        return m_wBookView->QueryCommandState( "superscript" );

    else
        return ContentTab::SuperscriptChecked();
}


bool FlowTab::AlignLeftChecked()
{
    if (m_ViewState == MainWindow::ViewState_BookView)
        return m_wBookView->QueryCommandState( "justifyLeft" );
    else
        return ContentTab::AlignLeftChecked();
}

bool FlowTab::AlignRightChecked()
{
    if (m_ViewState == MainWindow::ViewState_BookView)
        return m_wBookView->QueryCommandState( "justifyRight" );
    else
        return ContentTab::AlignRightChecked();
}

bool FlowTab::AlignCenterChecked()
{
    if (m_ViewState == MainWindow::ViewState_BookView)
        return m_wBookView->QueryCommandState( "justifyCenter" );
    else
        return ContentTab::AlignCenterChecked();
}

bool FlowTab::AlignJustifyChecked()
{
    if (m_ViewState == MainWindow::ViewState_BookView)
        return m_wBookView->QueryCommandState( "justifyFull" );
    else
        return ContentTab::AlignJustifyChecked();
}

bool FlowTab::BulletListChecked()
{
    if (m_ViewState == MainWindow::ViewState_BookView)
        return m_wBookView->QueryCommandState( "insertUnorderedList" );

    else
        return ContentTab::BulletListChecked();
}

bool FlowTab::NumberListChecked()
{
    if (m_ViewState == MainWindow::ViewState_BookView)
        return m_wBookView->QueryCommandState( "insertOrderedList" );

    else
        return ContentTab::NumberListChecked();
}

QString FlowTab::GetCaretElementName()
{
    if (m_ViewState == MainWindow::ViewState_BookView)
        return m_wBookView->GetCaretElementName();
    else
        return ContentTab::GetCaretElementName();
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
    if (m_BookPreviewNeedReload) {
        // Must save tab content first or else reload may not take place.
        SaveTabContent();
        ReloadTabIfPending();
    }
}

void FlowTab::DelayedConnectSignalsToSlots()
{
    connect(m_wBookView, SIGNAL(textChanged()), this, SLOT(EmitContentChanged()));
    connect(m_wCodeView, SIGNAL(FilteredTextChanged()), this, SLOT(EmitContentChanged()));

    connect(&m_HTMLResource, SIGNAL(TextChanging()), this, SLOT(ResourceTextChanging()));
    connect(&m_HTMLResource, SIGNAL(LinkedResourceUpdated()), this, SLOT(LinkedResourceModified()));
    connect(&m_HTMLResource, SIGNAL(Modified()), this, SLOT(ResourceModified()));
    connect(&m_HTMLResource, SIGNAL(LoadedFromDisk()), this, SLOT(ReloadTabIfPending()));
}

void FlowTab::ConnectSignalsToSlots()
{
    connect(m_wCodeView, SIGNAL(cursorPositionChanged()), this, SLOT(EmitUpdateCursorPosition()));

    connect(m_wBookView, SIGNAL(ZoomFactorChanged(float)), this, SIGNAL(ZoomFactorChanged(float)));
    connect(m_wBookPreview, SIGNAL(ZoomFactorChanged(float)), this, SIGNAL(ZoomFactorChanged(float)));
    connect(m_wCodeView, SIGNAL(ZoomFactorChanged(float)), this, SIGNAL(ZoomFactorChanged(float)));

    connect(m_wBookView, SIGNAL(selectionChanged()), this, SIGNAL(SelectionChanged()));
    connect(m_wBookPreview, SIGNAL(selectionChanged()), this, SIGNAL(SelectionChanged()));
    connect(m_wCodeView, SIGNAL(selectionChanged()), this, SIGNAL(SelectionChanged()));

    connect(m_wBookView, SIGNAL(FocusGained(QWidget *)), this, SLOT(EnterEditor(QWidget *)));
    connect(m_wBookView, SIGNAL(FocusLost(QWidget *)), this, SLOT(LeaveEditor(QWidget *)));

    connect(m_wBookPreview, SIGNAL(FocusGained(QWidget *)), this, SLOT(EnterEditor(QWidget *)));

    connect(m_wCodeView, SIGNAL(FocusGained(QWidget *)), this, SLOT(EnterEditor(QWidget *)));
    connect(m_wCodeView, SIGNAL(FocusLost(QWidget *)), this, SLOT(LeaveEditor(QWidget *)));

    connect(m_wBookView, SIGNAL(InsertImage()), this, SIGNAL(InsertImageRequest()));
    connect(m_wBookView, SIGNAL(LinkClicked(const QUrl&)), this, SIGNAL(LinkClicked(const QUrl&)));
    connect(m_wCodeView, SIGNAL(LinkClicked(const QUrl&)), this, SIGNAL(LinkClicked(const QUrl&)));
    connect(m_wBookPreview, SIGNAL(LinkClicked(const QUrl&)), this, SIGNAL(LinkClicked(const QUrl&)));

    connect(m_pvVSplitter, SIGNAL(splitterMoved(int, int)), this, SLOT(PVSplitterMoved(int, int)));

    connect(m_wCodeView, SIGNAL(OpenClipEditorRequest(ClipEditorModel::clipEntry *)), this, SIGNAL(OpenClipEditorRequest(ClipEditorModel::clipEntry *)));

    connect(m_wCodeView, SIGNAL(OpenIndexEditorRequest(IndexEditorModel::indexEntry *)), this, SIGNAL(OpenIndexEditorRequest(IndexEditorModel::indexEntry *)));

    connect(m_wCodeView, SIGNAL(GoToLinkedStyleDefinitionRequest(const QString&, const QString&)), this, SIGNAL(GoToLinkedStyleDefinitionRequest(const QString&, const QString&)));

    connect(m_wBookView, SIGNAL(ClipboardSaveRequest()),    this, SIGNAL(ClipboardSaveRequest()));
    connect(m_wBookView, SIGNAL(ClipboardRestoreRequest()), this, SIGNAL(ClipboardRestoreRequest()));

    connect(m_wCodeView, SIGNAL(BookmarkLinkOrStyleLocationRequest()), this, SIGNAL(BookmarkLinkOrStyleLocationRequest()));

    connect(m_wCodeView, SIGNAL(SpellingHighlightRefreshRequest()), this, SIGNAL(SpellingHighlightRefreshRequest()));

    connect(m_wBookView, SIGNAL(ImageOpenedExternally(const QString &)), this, SIGNAL(ImageOpenedExternally(const QString &)));

    connect(m_wBookView, SIGNAL(ImageSaveAs(const QUrl&)), this, SIGNAL(ImageSaveAs(const QUrl&)));

    connect(m_wBookView, SIGNAL(ShowStatusMessageRequest(const QString&, int)), this, SIGNAL(ShowStatusMessageRequest(const QString&, int)));
    connect(m_wBookPreview, SIGNAL(ShowStatusMessageRequest(const QString&, int)), this, SIGNAL(ShowStatusMessageRequest(const QString&, int)));
    connect(m_wCodeView, SIGNAL(ShowStatusMessageRequest(const QString&, int)), this, SIGNAL(ShowStatusMessageRequest(const QString&, int)));
}
