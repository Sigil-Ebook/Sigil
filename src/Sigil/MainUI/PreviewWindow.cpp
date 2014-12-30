/************************************************************************
**
**  Copyright (C) 2012 Dave Heiland, John Schember
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

#include <QtWidgets/QSplitter>
#include <QtWidgets/QStackedWidget>
#include <QtWebKitWidgets/QWebInspector>

#include "MainUI/PreviewWindow.h"
#include "Misc/SleepFunctions.h"
#include "ResourceObjects/HTMLResource.h"
#include "ViewEditors/BookViewEditor.h"

static const QString SETTINGS_GROUP = "previewwindow";

PreviewWindow::PreviewWindow(QWidget *parent)
    :
    QDockWidget(tr("Preview"), parent),
    m_MainWidget(*new QWidget(this)),
    m_Layout(*new QVBoxLayout(&m_MainWidget)),
    m_Preview(new BookViewPreview(this)),
    m_Inspector(new QWebInspector(this)),
    m_Splitter(new QSplitter(this)),
    m_StackedViews(new QStackedWidget(this))
{
    SetupView();
    LoadSettings();
    ConnectSignalsToSlots();
}

PreviewWindow::~PreviewWindow()
{
    // BookViewPreview must be deleted before QWebInspector.
    // BookViewPreview's QWebPage is linked to the QWebInspector
    // and when deleted it will send a message to the linked QWebInspector
    // to remove the association. If QWebInspector is deleted before
    // BookViewPreview, BookViewPreview will try to access the deleted
    // QWebInspector and the application will SegFault. This is an issue
    // with how QWebPages interface with QWebInspector.

    if (m_Inspector) {
        m_Inspector->setPage(0);
        m_Inspector->close();
    }

    if (m_Preview) {
        delete m_Preview;
        m_Preview = 0;
    }

    if (m_Inspector) {
        delete m_Inspector;
        m_Inspector = 0;
    }

    if (m_Splitter) {
        delete m_Splitter;
        m_Splitter = 0;
    }

    if (m_StackedViews) {
        delete(m_StackedViews);
        m_StackedViews= 0;
    }
}

void PreviewWindow::hideEvent(QHideEvent * event)
{
    if (m_Inspector) {
        // break the link between the inspector and the page it is inspecting
        // to prevent memory corruption from Qt modified after free issue
        m_Inspector->setPage(0);
        if (m_Inspector->isVisible()) {
            m_Inspector->hide();
        }
    }
    if ((m_Preview) && m_Preview->isVisible()) {
        m_Preview->hide();
    }
    if ((m_Splitter) && m_Splitter->isVisible()) {
        m_Splitter->hide();
    }
    if ((m_StackedViews) && m_StackedViews->isVisible()) {
        m_StackedViews->hide();
    }
}

void PreviewWindow::showEvent(QShowEvent * event)
{
    // restablish the link between the inspector and its page
    if ((m_Inspector) && (m_Preview)) {
        m_Inspector->setPage(m_Preview->page());
    }
    // perform the show for all children of this widget
    if ((m_Preview) && !m_Preview->isVisible()) {
        m_Preview->show();
    }
    if ((m_Inspector) && !m_Inspector->isVisible()) {
        m_Inspector->show();
    }
    if ((m_Splitter) && !m_Splitter->isVisible()) {
        m_Splitter->show();
    }
    if ((m_StackedViews) && !m_StackedViews->isVisible()) {
        m_StackedViews->show();
    }
    QDockWidget::showEvent(event);
    raise();
    emit Shown();
}

bool PreviewWindow::IsVisible()
{
    return m_Preview->isVisible();
}

bool PreviewWindow::HasFocus()
{
    if (!m_Preview->isVisible()) {
        return false;
    }
    return m_Preview->hasFocus();
}

float PreviewWindow::GetZoomFactor()
{
    return m_Preview->GetZoomFactor();
}

void PreviewWindow::SetupView()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    m_Inspector->setPage(m_Preview->page());

    m_Layout.setContentsMargins(0, 0, 0, 0);
#ifdef Q_OS_MAC
    m_Layout.setSpacing(4);
#endif

    m_Layout.addWidget(m_StackedViews);

    m_Splitter->setOrientation(Qt::Vertical);
    m_Splitter->addWidget(m_Preview);
    m_Splitter->addWidget(m_Inspector);
    m_Splitter->setSizes(QList<int>() << 400 << 200);
    m_StackedViews->addWidget(m_Splitter);

    m_MainWidget.setLayout(&m_Layout);
    setWidget(&m_MainWidget);

    m_Preview->Zoom();

    QApplication::restoreOverrideCursor();
}

void PreviewWindow::UpdatePage(QString filename, QString text, QList<ViewEditor::ElementIndex> location)
{
    if (!m_Preview->isVisible()) {
        return;
    }

    m_Preview->CustomSetDocument(filename, text);

    // Wait until the preview is loaded before moving cursor.
    while (!m_Preview->IsLoadingFinished()) {
        qApp->processEvents();
        SleepFunctions::msleep(100);
    }

    m_Preview->StoreCaretLocationUpdate(location);
    m_Preview->ExecuteCaretUpdate();
    m_Preview->InspectElement();
}

QList<ViewEditor::ElementIndex> PreviewWindow::GetCaretLocation()
{
    return m_Preview->GetCaretLocation();
}

void PreviewWindow::SetZoomFactor(float factor)
{
    m_Preview->SetZoomFactor(factor);
}

void PreviewWindow::SplitterMoved(int pos, int index)
{
    Q_UNUSED(pos);
    Q_UNUSED(index);
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    settings.setValue("splitter", m_Splitter->saveState());
    settings.endGroup();
}

void PreviewWindow::LoadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    m_Splitter->restoreState(settings.value("splitter").toByteArray());
    settings.endGroup();
}

void PreviewWindow::ConnectSignalsToSlots()
{
    connect(m_Splitter,  SIGNAL(splitterMoved(int, int)), this, SLOT(SplitterMoved(int, int)));
    connect(m_Preview,   SIGNAL(GoToPreviewLocationRequest()), this, SIGNAL(GoToPreviewLocationRequest()));
    connect(m_Preview,   SIGNAL(ZoomFactorChanged(float)), this, SIGNAL(ZoomFactorChanged(float)));
}

