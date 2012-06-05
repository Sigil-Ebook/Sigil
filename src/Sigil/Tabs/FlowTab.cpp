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
                  QWidget *parent)
    :
    ContentTab(resource, parent),
    m_FragmentToScroll(fragment),
    m_LineToScrollTo(line_to_scroll_to),
    m_HTMLResource(resource),
    m_views(new QStackedWidget(this)),
    m_pvVSplitter(new QSplitter(this)),
    m_wBookView(new BookViewEditor(this)),
    m_wBookPreview(new BookViewPreview(this)),
    m_wCodeView(new CodeViewEditor(CodeViewEditor::Highlight_XHTML, true, this)),
    m_inspector(new QWebInspector(this)),
    m_ViewState(view_state),
    m_WellFormedCheckComponent(*new WellFormedCheckComponent(*this)),
    m_safeToLoad(false),
    m_initialLoad(true),
    m_BookPreviewNeedReload(false)
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

bool FlowTab::IsModified()
{
    return m_wBookView->isModified() || m_wCodeView->document()->isModified();
}

bool FlowTab::PrintEnabled()
{
    return true;
}

bool FlowTab::CutEnabled()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView->pageAction(QWebPage::Cut)->isEnabled();
    }
    else if (m_ViewState == MainWindow::ViewState_PreviewView) {
        return m_wBookPreview->pageAction(QWebPage::Cut)->isEnabled();
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
    else if (m_ViewState == MainWindow::ViewState_BookView) {
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
    else if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookPreview->pageAction(QWebPage::Paste)->isEnabled();
    }
    else if (m_ViewState == MainWindow::ViewState_CodeView) {
        return m_wCodeView->canPaste();
    }

    return false;
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
    if (new_view_state == MainWindow::ViewState_BookView && !m_wBookView->IsLoadingFinished()) {
        return false;
    }
    if (!IsDataWellFormed()) {
        return false;
    }

    // We do a save before changing to ensure we don't lose any unsaved data
    // in the previous view.
    SaveTabContent();
    m_ViewState = new_view_state;
    LoadTabContent();

    if (new_view_state == MainWindow::ViewState_PreviewView) {
        SplitView();
    }
    else if (new_view_state == MainWindow::ViewState_CodeView) {
        CodeView();
    }
    else {
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
        m_wCodeView->ReplaceDocumentText(CleanSource::PrettyPrint(CleanSource::Clean(m_wCodeView->toPlainText())));
    }
}

bool FlowTab::GetCheckWellFormedErrors()
{
    return m_WellFormedCheckComponent.GetCheckWellFormedErrors();
}

void FlowTab::SetWellFormedDialogsEnabledState(bool enabled)
{
    m_WellFormedCheckComponent.SetWellFormedDialogsEnabledState(enabled);
}

void FlowTab::SetCheckWellFormedErrorsState(bool enabled)
{
    m_WellFormedCheckComponent.SetCheckWellFormedErrorsState(enabled);
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
    if (m_ViewState == MainWindow::ViewState_BookView || m_ViewState == MainWindow::ViewState_PreviewView || !GetCheckWellFormedErrors()) {
        m_safeToLoad = true;
        return true;
    }

    if (m_ViewState == MainWindow::ViewState_CodeView) {
        XhtmlDoc::WellFormedError error = XhtmlDoc::WellFormedErrorForSource(m_wCodeView->toPlainText());
        bool well_formed = error.line == -1;

        if (!well_formed) {
            m_safeToLoad = false;
            m_WellFormedCheckComponent.DemandAttentionIfAllowed(error);
        }
        else {
            m_safeToLoad = true;
        }

        return well_formed;
    }

    return true;
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
        m_wBookView->page()->triggerAction(QWebPage::Paste);
    }
    else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->paste();
    }
}

void FlowTab::SplitChapter()
{
    if (!IsDataWellFormed()) {
        return;
    }

    if (m_ViewState == MainWindow::ViewState_BookView) {
        emit OldTabRequest( m_wBookView->SplitChapter(), m_HTMLResource );
    }
    else if (m_ViewState == MainWindow::ViewState_CodeView) {
        emit OldTabRequest( m_wCodeView->SplitChapter(), m_HTMLResource );
    }
}

void FlowTab::InsertSGFChapterMarker()
{
    if (!IsDataWellFormed()) {
        return;
    }

    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView->InsertHtml(BREAK_TAG_INSERT);
    }
    else if (m_ViewState == MainWindow::ViewState_CodeView) {
        m_wCodeView->InsertSGFChapterMarker();
    }
}

void FlowTab::SplitOnSGFChapterMarkers()
{
    if (!IsDataWellFormed()) {
        return;
    }

    SaveTabContent();
    emit NewChaptersRequest(m_HTMLResource.SplitOnSGFChapterMarkers(), m_HTMLResource);
    LoadTabContent();
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

void FlowTab::BookView()
{
    if (!IsDataWellFormed()) {
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    EnterBookView();

    m_views->setCurrentIndex(BV_INDEX);

    setFocusProxy(m_wBookView);
    m_wBookView->GrabFocus();

    QApplication::restoreOverrideCursor();
}

void FlowTab::SplitView()
{
    if (!IsDataWellFormed()) {
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    m_views->setCurrentIndex(PV_INDEX);
    m_wCodeView->SetDelayedCursorScreenCenteringRequired();

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
    m_wCodeView->setFocus();
    m_wCodeView->RestoreCaretLocation();

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
    m_safeToLoad = true;
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

    // Reload BV if the resource was marked as changed outside of the editor.
    if (m_BookPreviewNeedReload && m_ViewState == MainWindow::ViewState_PreviewView) {
        LoadTabContent();
    }
    if (m_BookPreviewNeedReload) {
        m_BookPreviewNeedReload = false;
    }

    // BookPreview is left out of this because we always want to reload with any current changes
    // from CodeView.
    if ((m_ViewState == MainWindow::ViewState_BookView && editor == m_wBookView) ||
         ((m_ViewState == MainWindow::ViewState_PreviewView || m_ViewState == MainWindow::ViewState_CodeView) && editor == m_wCodeView))
    {
        // Nothing to de because the view state matches the current view.
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    if (m_ViewState == MainWindow::ViewState_BookView) {
        EnterBookView();
    }
    else if (m_ViewState == MainWindow::ViewState_PreviewView) {
        EnteringBookPreview();
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
    // state becuase there is no last view state.
    switch(m_ViewState) {
        case MainWindow::ViewState_CodeView:
        {
            CodeView();

            if (m_LineToScrollTo > 0) {
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
            m_wBookPreview->ScrollToFragmentAfterLoad(m_FragmentToScroll.toString());
            break;
        }
        // Don't care about these so ignore them.
        case MainWindow::ViewState_RawView:
        case MainWindow::ViewState_StaticView:
        default:
            BookView();
            m_wBookView->ScrollToFragmentAfterLoad(m_FragmentToScroll.toString());
            break;
    }

    m_wBookView->Zoom();
    m_wCodeView->Zoom();

    m_safeToLoad = true;
    m_initialLoad = false;

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

void FlowTab::ConnectSignalsToSlots()
{
    connect(m_wCodeView, SIGNAL(cursorPositionChanged()), this, SLOT(EmitUpdateCursorPosition()));

    connect(m_wBookView, SIGNAL(ZoomFactorChanged(float)), this, SIGNAL(ZoomFactorChanged(float)));
    connect(m_wBookPreview, SIGNAL(ZoomFactorChanged(float)), this, SIGNAL(ZoomFactorChanged(float)));
    connect(m_wCodeView, SIGNAL(ZoomFactorChanged(float)), this, SIGNAL(ZoomFactorChanged(float)));

    connect(m_wBookView, SIGNAL(selectionChanged()), this, SIGNAL(SelectionChanged()));
    connect(m_wCodeView, SIGNAL(selectionChanged()), this, SIGNAL(SelectionChanged()));

    connect(m_wBookView, SIGNAL(FocusGained(QWidget *)), this, SLOT(EnterEditor(QWidget *)));
    connect(m_wBookView, SIGNAL(FocusLost(QWidget *)), this, SLOT(LeaveEditor(QWidget *)));
    connect(m_wBookView, SIGNAL(FilteredLinkClicked(const QUrl&)), this, SIGNAL(LinkClicked(const QUrl&)));

    connect(m_wBookPreview, SIGNAL(FocusGained(QWidget *)), this, SLOT(EnterEditor(QWidget *)));
    connect(m_wBookPreview, SIGNAL(FilteredLinkClicked(const QUrl&)), this, SIGNAL(LinkClicked(const QUrl&)));

    connect(m_wCodeView, SIGNAL(FocusGained(QWidget *)), this, SLOT(EnterEditor(QWidget *)));
    connect(m_wCodeView, SIGNAL(FocusLost(QWidget *)), this, SLOT(LeaveEditor(QWidget *)));

    connect(m_wBookView, SIGNAL(textChanged()), this, SLOT(EmitContentChanged()));
    connect(m_wCodeView, SIGNAL(FilteredTextChanged()), this, SLOT(EmitContentChanged()));

    connect(&m_HTMLResource, SIGNAL(Modified()), this, SLOT(ResourceModified()));
    connect(&m_HTMLResource, SIGNAL(LinkedResourceUpdated()), this, SLOT(ResourceModified()));

    connect(m_pvVSplitter, SIGNAL(splitterMoved(int, int)), this, SLOT(PVSplitterMoved(int, int)));
}
