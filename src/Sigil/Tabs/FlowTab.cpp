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
    m_Splitter(*new QSplitter(this)),
    m_wBookView(*new BookViewEditor(this)),
    m_wBookPreview(*new BookViewPreview(this)),
    m_wCodeView(*new CodeViewEditor(CodeViewEditor::Highlight_XHTML, true, this)),
    m_ViewState(view_state),
    m_WellFormedCheckComponent(*new WellFormedCheckComponent(*this)),
    m_safeToLoad(false),
    m_initialLoad(true)
{
    // Loading a flow tab can take a while. We set the wait
    // cursor and clear it at the end of the delayed initialization.
    QApplication::setOverrideCursor(Qt::WaitCursor);

    m_Layout.addWidget(&m_Splitter);

    LoadSettings();

    // We need to set this in the constructor too,
    // so that the ContentTab focus handlers don't 
    // get called when the tab is created.
    if (view_state == MainWindow::ViewState_BookView) {
        setFocusProxy(&m_wBookView);
    }
    else {
        setFocusProxy(&m_wCodeView);
    }

    // We perform delayed initialization after the widget is on
    // the screen. This way, the user perceives less load time.
    QTimer::singleShot( 0, this, SLOT( DelayedInitialization() ) );
}


FlowTab::~FlowTab()
{
    m_WellFormedCheckComponent.deleteLater();
}

void FlowTab::RestoreCaret()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView.RestoreCaret();
    }
    else {
        m_wCodeView.RestoreCaret();
    }
}

bool FlowTab::IsModified()
{
    return m_wBookView.isModified() || m_wCodeView.document()->isModified();
}


bool FlowTab::PrintEnabled()
{
    return true;
}


bool FlowTab::CutEnabled()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView.pageAction( QWebPage::Cut )->isEnabled();
    }
    else {
        return m_wCodeView.textCursor().hasSelection();
    }
}


bool FlowTab::CopyEnabled()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView.pageAction(QWebPage::Copy)->isEnabled();
    }
    else {
        return m_wCodeView.textCursor().hasSelection();
    }
}

bool FlowTab::PasteEnabled()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView.pageAction(QWebPage::Paste)->isEnabled();
    }
    else {
        return m_wCodeView.canPaste();
    }
}


int FlowTab::GetCursorLine() const
{
    if (m_ViewState == MainWindow::ViewState_CodeView || m_wCodeView.hasFocus()) {
        return m_wCodeView.GetCursorLine();
    }

    return -1;
}


int FlowTab::GetCursorColumn() const
{
    if (m_ViewState == MainWindow::ViewState_CodeView || m_wCodeView.hasFocus()) {
        return m_wCodeView.GetCursorColumn();
    }

    return -1;
}


float FlowTab::GetZoomFactor() const
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView.GetZoomFactor();
    }
    else if (m_ViewState == MainWindow::ViewState_SplitView) {
        if (m_wBookPreview.hasFocus()) {
            return m_wBookPreview.GetZoomFactor();
        }
    }

    // None of the above matched so we are in CodeView
    return m_wCodeView.GetZoomFactor();
}


void FlowTab::SetZoomFactor(float new_zoom_factor)
{
    // We need to set a wait cursor for the Book View
    // since zoom operations take some time in it.
    if (m_ViewState == MainWindow::ViewState_BookView) {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        m_wBookView.SetZoomFactor(new_zoom_factor);
        m_wBookPreview.SetZoomFactor(new_zoom_factor);
        QApplication::restoreOverrideCursor();
    }
    else if (m_ViewState == MainWindow::ViewState_SplitView) {
        if (m_wBookPreview.hasFocus()) {
            QApplication::setOverrideCursor(Qt::WaitCursor);
            m_wBookView.SetZoomFactor(new_zoom_factor);
            m_wBookPreview.SetZoomFactor(new_zoom_factor);
            QApplication::restoreOverrideCursor();
        }
        else {
            m_wCodeView.SetZoomFactor(new_zoom_factor);
        }
    }
    else {
        m_wCodeView.SetZoomFactor(new_zoom_factor);
    } 
}


void FlowTab::UpdateDisplay()
{
    m_wBookView.UpdateDisplay();
    m_wBookPreview.UpdateDisplay();
    m_wCodeView.UpdateDisplay();
}


Searchable* FlowTab::GetSearchableContent()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        return &m_wBookView;
    }
    else {
        return &m_wCodeView;
    }
}


void FlowTab::SetViewState(MainWindow::ViewState new_view_state)
{
    // Do we really need to do anything?
    // Ignore this function if we are in the middle of doing an initial load
    // of the content. We don't want it to save over the content with nothing
    // if this is called before the delayed initialization function is called.
    if (m_initialLoad) {
        return;
    }
    if (new_view_state == m_ViewState) {
        return;
    }
    if (new_view_state == MainWindow::ViewState_BookView && !m_wBookView.IsLoadingFinished()) {
        return;
    }

    // We do a save before changing to ensure we don't lose any unsaved data
    // in the previous view.
    SaveTabContent();
    m_ViewState = new_view_state;
    LoadTabContent();

    if (new_view_state == MainWindow::ViewState_SplitView) {
        SplitView();
    }
    else if (new_view_state == MainWindow::ViewState_CodeView) {
        CodeView();
    }
    else {
       BookView();
    }
}

bool FlowTab::IsLoadingFinished()
{
    return m_wBookView.IsLoadingFinished() && m_wBookPreview.IsLoadingFinished() && m_wCodeView.IsLoadingFinished();
}

void FlowTab::ScrollToFragment(const QString &fragment)
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView.ScrollToFragment(fragment);
    }
    else if (m_ViewState == MainWindow::ViewState_SplitView) {
        m_wBookPreview.ScrollToFragment(fragment);
        m_wCodeView.ScrollToFragment(fragment);
    }
    else {
        m_wCodeView.ScrollToFragment(fragment);
    }
}


void FlowTab::ScrollToLine(int line)
{
    if (m_wCodeView.isVisible()) {
        m_wCodeView.ScrollToLine(line);
    }
}


void FlowTab::ScrollToTop()
{
   m_wBookView.ScrollToTop();
   m_wBookPreview.ScrollToTop();
   m_wCodeView.ScrollToTop();
}


void FlowTab::AutoFixWellFormedErrors()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        return;
    }

    m_wCodeView.ReplaceDocumentText(CleanSource::PrettyPrint(CleanSource::Clean(m_wCodeView.toPlainText())));
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
    if (m_ViewState == MainWindow::ViewState_BookView || !GetCheckWellFormedErrors()) {
        m_safeToLoad = true;
        return true;
    }

    XhtmlDoc::WellFormedError error = XhtmlDoc::WellFormedErrorForSource(m_wCodeView.toPlainText());
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


void FlowTab::Undo()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        //m_wBookView.page()->triggerAction( QWebPage::Undo );
    }
    else {
        m_wCodeView.undo();
    }
}


void FlowTab::Redo()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        //m_wBookView.page()->triggerAction( QWebPage::Redo );
    }
    else {
        m_wCodeView.redo();
    }
}


void FlowTab::Cut()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView.page()->triggerAction( QWebPage::Cut );
    }
    else {
        m_wCodeView.cut();
    }
}


void FlowTab::Copy()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView.page()->triggerAction( QWebPage::Copy );
    }
    else {
        m_wCodeView.copy();
    }
}


void FlowTab::Paste()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView.page()->triggerAction( QWebPage::Paste );
    }
    else {
        m_wCodeView.paste();
    }
}


void FlowTab::SplitChapter()
{
    if (!IsDataWellFormed()) {
        return;
    }

    if (m_ViewState == MainWindow::ViewState_BookView) {
        emit OldTabRequest( m_wBookView.SplitChapter(), m_HTMLResource );
    }
    else {
        emit OldTabRequest( m_wCodeView.SplitChapter(), m_HTMLResource );
    }
}


void FlowTab::InsertSGFChapterMarker()
{
    if (!IsDataWellFormed()) {
        return;
    }

    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_wBookView.InsertHtml(BREAK_TAG_INSERT);
    }
    else {
        m_wCodeView.InsertSGFChapterMarker();
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
        m_wBookView.InsertHtml(html);
    }
    else {
        m_wCodeView.insertPlainText(html);
    }
}


void FlowTab::PrintPreview()
{
    QPrintPreviewDialog *print_preview = new QPrintPreviewDialog(this);

    if (m_ViewState == MainWindow::ViewState_BookView) {
        connect(print_preview, SIGNAL(paintRequested(QPrinter *)), &m_wBookView, SLOT(print(QPrinter *)));
    }
    else if (m_ViewState == MainWindow::ViewState_SplitView && m_wBookPreview.hasFocus()) {
        connect(print_preview, SIGNAL(paintRequested(QPrinter *)), &m_wBookPreview, SLOT(print(QPrinter *)));
    }
    else {
        connect(print_preview, SIGNAL(paintRequested(QPrinter *)), &m_wCodeView, SLOT(print(QPrinter *)));
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
            m_wBookView.print(&printer);
        }
        else if (m_ViewState == MainWindow::ViewState_SplitView && m_wBookPreview.hasFocus()) {
            m_wBookPreview.print(&printer);
        }
        else {
            m_wCodeView.print(&printer);
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

    m_wCodeView.hide();
    m_wBookPreview.hide();
    m_wBookView.show();

    m_wBookView.RestoreCaret();

    setFocusProxy(&m_wBookView);
    m_wBookView.GrabFocus();

    QApplication::restoreOverrideCursor();
}


void FlowTab::SplitView()
{
    if (!IsDataWellFormed()) {
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    m_wBookView.hide();
    m_wBookPreview.show();
    m_wCodeView.show();
    m_wCodeView.RestoreCaret();
    m_wCodeView.SetDelayedCursorScreenCenteringRequired();

    QApplication::restoreOverrideCursor();
}


void FlowTab::CodeView()
{    
    QApplication::setOverrideCursor(Qt::WaitCursor);

    m_ViewState = MainWindow::ViewState_CodeView;

    m_wBookView.hide();
    m_wBookPreview.hide();
    m_wCodeView.show();
    m_wCodeView.RestoreCaret();
    m_wCodeView.SetDelayedCursorScreenCenteringRequired();

    setFocusProxy(&m_wCodeView);

    // Make sure the cursor is properly displayed
    if (!m_wCodeView.hasFocus()) {
        m_wCodeView.setFocus();
    }

    QApplication::restoreOverrideCursor();
}


void FlowTab::SaveTabContent()
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        m_HTMLResource.SetText(m_wBookView.GetHtml());
        m_wBookView.ResetModified();
        m_wBookView.SaveCaret();
    }
    else {
        m_wCodeView.SaveCaret();
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
        m_wBookView.CustomSetDocument(m_HTMLResource.GetFullPath(), m_HTMLResource.GetText());
    } else if (m_ViewState == MainWindow::ViewState_SplitView) {
        m_wBookPreview.CustomSetDocument(m_HTMLResource.GetFullPath(), m_HTMLResource.GetText());
    }

    RestoreCaret();
}


void FlowTab::LoadSettings()
{
    SettingsStore settings;
    m_Splitter.setOrientation(settings.splitViewOrientation());

    // If widgets already exist, splitter will rearrange them
    if ( settings.splitViewOrder() )
    {
        m_Splitter.addWidget(&m_wBookView);
        m_Splitter.addWidget(&m_wBookPreview);
        m_Splitter.addWidget(&m_wCodeView);
    }
    else
    {
        m_Splitter.addWidget(&m_wCodeView);
        m_Splitter.addWidget(&m_wBookPreview);
        m_Splitter.addWidget(&m_wBookView);
    }

    m_wCodeView.LoadSettings();
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
    if ((m_ViewState == MainWindow::ViewState_BookView && editor == & m_wBookView) ||
         ((m_ViewState == MainWindow::ViewState_SplitView || m_ViewState == MainWindow::ViewState_CodeView) && editor == &m_wCodeView))
    {
        // Nothing to de because the view state matches the current view.
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    if (m_ViewState == MainWindow::ViewState_BookView) {
        EnterBookView();
    }
    else {
        if (editor == &m_wBookPreview) {
            m_wBookPreview.CustomSetDocument(m_HTMLResource.GetFullPath(), m_HTMLResource.GetText());
            EnteringBookPreview();
        }
        else {
            EnterCodeView();
        }
    }
    QApplication::restoreOverrideCursor();

    EmitUpdateCursorPosition();
}


void FlowTab::DelayedInitialization()
{
    // TextResource loads on demand. We want to ensure the document is loaded
    // before we start using it. Without this BV will sometimes load with
    // an empty documnet.
    m_HTMLResource.InitialLoad();

    m_wBookView.CustomSetDocument(m_HTMLResource.GetFullPath(), m_HTMLResource.GetText());
    m_wBookPreview.CustomSetDocument(m_HTMLResource.GetFullPath(), m_HTMLResource.GetText());
    m_wCodeView.CustomSetDocument(m_HTMLResource.GetTextDocumentForWriting());

    // m_lastViewState in this instance is actually used a the initial view
    // state becuase there is no last view state.
    switch(m_ViewState) {
        case MainWindow::ViewState_CodeView:
        {
            CodeView();

            if (m_LineToScrollTo > 0) {
                m_wCodeView.ScrollToLine(m_LineToScrollTo);
            }
            else {
                m_wCodeView.ScrollToFragment(m_FragmentToScroll.toString());
            }

            break;
        }
        case MainWindow::ViewState_SplitView:
        {
            SplitView();

            if (m_LineToScrollTo > 0) {
                m_wCodeView.ScrollToLine(m_LineToScrollTo);
            }
            else {
                m_wCodeView.ScrollToFragment(m_FragmentToScroll.toString());
            }

            m_wBookPreview.ScrollToFragmentAfterLoad(m_FragmentToScroll.toString());
            break;
        }
        // Don't care about these so ignore them.
        case MainWindow::ViewState_RawView:
        case MainWindow::ViewState_StaticView:
        default:
            BookView();
            m_wBookView.ScrollToFragmentAfterLoad(m_FragmentToScroll.toString());
            break;
    }

    m_wBookView.Zoom();
    m_wCodeView.Zoom();

    m_safeToLoad = true;
    m_initialLoad = false;

    ConnectSignalsToSlots();

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
    if (!IsDataWellFormed()) {
        return;
    }
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


ViewEditor& FlowTab::GetActiveViewEditor() const
{
    if (m_ViewState == MainWindow::ViewState_BookView) {
        return m_wBookView;
    }
    else {
        return m_wCodeView;
    }
}


void FlowTab::ConnectSignalsToSlots()
{
    connect(&m_wCodeView, SIGNAL(cursorPositionChanged()), this, SLOT(EmitUpdateCursorPosition()));

    connect(&m_wBookView, SIGNAL(ZoomFactorChanged(float)), this, SIGNAL(ZoomFactorChanged(float)));
    connect(&m_wBookPreview, SIGNAL(ZoomFactorChanged(float)), this, SIGNAL(ZoomFactorChanged(float)));
    connect(&m_wCodeView, SIGNAL(ZoomFactorChanged(float)), this, SIGNAL(ZoomFactorChanged(float)));

    connect(&m_wBookView, SIGNAL(selectionChanged()), this, SIGNAL(SelectionChanged()));
    connect(&m_wCodeView, SIGNAL(selectionChanged()), this, SIGNAL(SelectionChanged()));

    connect(&m_wBookView, SIGNAL(FocusGained(QWidget *)), this, SLOT(EnterEditor(QWidget *)));
    connect(&m_wBookView, SIGNAL(FocusLost(QWidget *)), this, SLOT(LeaveEditor(QWidget *)));

    connect(&m_wBookPreview, SIGNAL(FocusGained(QWidget *)), this, SLOT(EnterEditor(QWidget *)));

    connect(&m_wCodeView, SIGNAL(FocusGained(QWidget *)), this, SLOT(EnterEditor(QWidget *)));
    connect(&m_wCodeView, SIGNAL(FocusLost(QWidget *)), this, SLOT(LeaveEditor(QWidget *)));

    connect(&m_wBookView, SIGNAL(textChanged()), this, SLOT(EmitContentChanged()));
    connect(&m_wCodeView, SIGNAL(FilteredTextChanged()), this, SLOT(EmitContentChanged()));
}
