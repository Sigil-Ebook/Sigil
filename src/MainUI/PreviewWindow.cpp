/************************************************************************
**
**  Copyright (C) 2019 Kevin Hendricks, Doug Massay
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

#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QtWebEngineWidgets/QWebEngineView>
#include <QDir>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QTimer>
#include <QDebug>

#include "MainUI/PreviewWindow.h"
#include "Dialogs/Inspector.h"
#include "Misc/SleepFunctions.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "ViewEditors/ViewPreview.h"
#include "sigil_constants.h"

static const QString SETTINGS_GROUP = "previewwindow";

#define DBG if(0)

PreviewWindow::PreviewWindow(QWidget *parent)
    :
    QDockWidget(tr("Preview"), parent),
    m_MainWidget(new QWidget(this)),
    m_Layout(new QVBoxLayout(m_MainWidget)),
    m_buttons(new QHBoxLayout(m_MainWidget)),
    m_Preview(new ViewPreview(this)),
    m_Inspector(new Inspector(this)),
    m_Filepath(QString()),
    m_GoToRequestPending(false)
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

    if (m_Preview) {
        delete m_Preview;
        m_Preview = nullptr;
    }

    if (m_Inspector) {
        if (m_Inspector->isVisible()) {
            m_Inspector->StopInspection();
	    m_Inspector->close();
	}
        delete m_Inspector;
        m_Inspector = nullptr;
    }
}


void PreviewWindow::resizeEvent(QResizeEvent *event)
{
    // Update self normally
    QDockWidget::resizeEvent(event);
    UpdateWindowTitle();
}


void PreviewWindow::hideEvent(QHideEvent * event)
{
    if (m_Inspector) {
        m_Inspector->StopInspection();
	m_Inspector->close();
	m_Inspector->hide();
    }

    if ((m_Preview) && m_Preview->isVisible()) {
        m_Preview->hide();
    }
}

void PreviewWindow::showEvent(QShowEvent * event)
{
    // perform the show for all children of this widget
    if ((m_Preview) && !m_Preview->isVisible()) {
        m_Preview->show();
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

    // QWebEngineView events are routed to their parent
    m_Preview->installEventFilter(this);

    m_Layout->setContentsMargins(0, 0, 0, 0);
#ifdef Q_OS_MAC
    // m_Layout->setSpacing(4);
#endif
    m_Layout->addWidget(m_Preview);

    // ToolBar for Inspect, Select-All, Copy, Reload
    m_inspectButton = new QPushButton(QIcon(":main/inspect_22px.png"),"", this);
    m_inspectButton->setToolTip(tr("Inspect Page"));
    m_selectButton  = new QPushButton(QIcon(":main/edit-select-all_22px.png"),"", this);
    m_selectButton->setToolTip(tr("Select-All"));
    m_copyButton    = new QPushButton(QIcon(":main/edit-copy_22px.png"),"", this);
    m_copyButton->setToolTip(tr("Copy Selection To ClipBoard"));
    m_reloadButton  = new QPushButton(QIcon(":main/reload-page_22px.png"),"", this);
    m_reloadButton->setToolTip(tr("Update Preview Window"));
    m_buttons->addWidget(m_inspectButton);
    m_buttons->addWidget(m_selectButton);
    m_buttons->addWidget(m_copyButton);
    m_buttons->addWidget(m_reloadButton);
    m_Layout->addLayout(m_buttons);

    m_MainWidget->setLayout(m_Layout);
    setWidget(m_MainWidget);

    m_Preview->Zoom();

    QApplication::restoreOverrideCursor();
}

void PreviewWindow::UpdatePage(QString filename_url, QString text, QList<ElementIndex> location)
{
    if (!m_Preview->isVisible()) {
        return;
    }

    DBG qDebug() << "PV UpdatePage " << filename_url;
    DBG foreach(ElementIndex ei, location) qDebug()<< "PV name: " << ei.name << " index: " << ei.index;

    // If the user has set a default stylesheet inject it
    if (!m_usercssurl.isEmpty()) {
        int endheadpos = text.indexOf("</head>");
        if (endheadpos > 1) {
            QString inject_userstyles = 
              "<link rel=\"stylesheet\" type=\"text/css\" "
	      "href=\"" + m_usercssurl + "\" />\n";
	    DBG qDebug() << "Preview injecting stylesheet: " << inject_userstyles;
            text.insert(endheadpos, inject_userstyles);
	}
    }

    // If this page uses mathml tags, inject a polyfill
    // MathJax.js so that the mathml appears in the Preview Window
    QRegularExpression mathused("<\\s*math [^>]*>");
    QRegularExpressionMatch mo = mathused.match(text);
    if (mo.hasMatch()) {
        int endheadpos = text.indexOf("</head>");
        if (endheadpos > 1) {
            QString inject_mathjax = 
              "<script type=\"text/javascript\" async=\"async\" "
              "src=\"" + m_mathjaxurl + "\"></script>\n";
            text.insert(endheadpos, inject_mathjax);
        }
    }

    m_Filepath = filename_url;
    m_Preview->CustomSetDocument(filename_url, text);

    // this next bit is allowing javascript to run before
    // the page is finished loading somehow? 
    // but we explicitly prevent that

    // Wait until the preview is loaded before moving cursor.
    while (!m_Preview->IsLoadingFinished()) {
        qApp->processEvents();
    }
    
    if (!m_Preview->WasLoadOkay()) qDebug() << "PV loadFinished with okay set to false!";
 
    DBG qDebug() << "PreviewWindow UpdatePage load is Finished";
    DBG qDebug() << "PreviewWindow UpdatePage final step scroll to location";

    m_Preview->StoreCaretLocationUpdate(location);
    m_Preview->ExecuteCaretUpdate();
    UpdateWindowTitle();
}

void PreviewWindow::ScrollTo(QList<ElementIndex> location)
{
    DBG qDebug() << "received a PreviewWindow ScrollTo event";
    if (!m_Preview->isVisible()) {
        return;
    }
    m_Preview->StoreCaretLocationUpdate(location);
    m_Preview->ExecuteCaretUpdate();
}

void PreviewWindow::UpdateWindowTitle()
{
    if ((m_Preview) && m_Preview->isVisible()) {
        int height = m_Preview->height();
        int width = m_Preview->width();
        setWindowTitle(tr("Preview") + " (" + QString::number(width) + "x" + QString::number(height) + ")");
    }
}

QList<ElementIndex> PreviewWindow::GetCaretLocation()
{
    DBG qDebug() << "PreviewWindow in GetCaretLocation";
    QList<ElementIndex> hierarchy = m_Preview->GetCaretLocation();
    DBG foreach(ElementIndex ei, hierarchy) qDebug() << "name: " << ei.name << " index: " << ei.index;
    return hierarchy;
}

void PreviewWindow::SetZoomFactor(float factor)
{
    m_Preview->SetZoomFactor(factor);
}

void PreviewWindow::EmitGoToPreviewLocationRequest()
{
    DBG qDebug() << "EmitGoToPreviewLocationRequest request: " << m_GoToRequestPending;
    if (m_GoToRequestPending) {
        m_GoToRequestPending = false;
        emit GoToPreviewLocationRequest();
    }
}

bool PreviewWindow::eventFilter(QObject *object, QEvent *event)
{
  switch (event->type()) {
    case QEvent::ChildAdded:
      if (object == m_Preview) {
	  DBG qDebug() << "child add event";
	  const QChildEvent *childEvent(static_cast<QChildEvent*>(event));
	  if (childEvent->child()) {
	      childEvent->child()->installEventFilter(this);
	  }
      }
      break;
    case QEvent::MouseButtonPress:
      {
	  DBG qDebug() << "Preview mouse button press event " << object;
	  const QMouseEvent *mouseEvent(static_cast<QMouseEvent*>(event));
	  if (mouseEvent) {
	      if (mouseEvent->button() == Qt::LeftButton) {
		  DBG qDebug() << "Detected Left Mouse Button Press Event";
 		  DBG qDebug() << "emitting GoToPreviewLocationRequest";
	          m_GoToRequestPending = true;
		  // we must delay long enough to separate out LinksClicked from scroll sync clicks
	          QTimer::singleShot(100, this, SLOT(EmitGoToPreviewLocationRequest()));
	      }
	  }
      }
      break;
    case QEvent::MouseButtonRelease:
      {
	  DBG qDebug() << "Preview mouse button release event " << object;
	  const QMouseEvent *mouseEvent(static_cast<QMouseEvent*>(event));
	  if (mouseEvent) {
	      if (mouseEvent->button() == Qt::LeftButton) {
	          DBG qDebug() << "Detected Left Mouse Button Release Event";
	      }
	  }
      }
      break;
    default:
      break;
  }
  return QObject::eventFilter(object, event);
}

void PreviewWindow::LinkClicked(const QUrl &url)
{
    if (m_GoToRequestPending) m_GoToRequestPending = false;

    DBG qDebug() << "in PreviewWindow LinkClicked with url :" << url.toString();

    if (url.toString().isEmpty()) {
        return;
    }

    QFileInfo finfo(m_Filepath);
    QString url_string = url.toString();

    // Convert fragments to full filename/fragments
    if (url_string.startsWith("#")) {
        url_string.prepend(finfo.fileName());
    } else if (url.scheme() == "file") {
        if (url_string.contains("/#")) {
            url_string.insert(url_string.indexOf("/#") + 1, finfo.fileName());
        }
    }
    emit OpenUrlRequest(QUrl(url_string));
}

void PreviewWindow::InspectorClosed(int i)
{
    qDebug() << "received finished with argument: " << i;
}

void PreviewWindow::InspectPreviewPage()
{
    qDebug() << "InspectPreviewPage()";
    // non-modal dialog
    if (!m_Inspector->isVisible()) {
        qDebug() << "inspecting";
        m_Inspector->InspectPage(m_Preview->page());
        m_Inspector->show();
        m_Inspector->raise();
        m_Inspector->activateWindow();
	return;
    }
    qDebug() << "stopping inspection()";
    m_Inspector->close();
}

void PreviewWindow::SelectAllPreview()
{
    m_Preview->triggerPageAction(QWebEnginePage::SelectAll);
}

void PreviewWindow::CopyPreview()
{
    m_Preview->triggerPageAction(QWebEnginePage::Copy);
}

void PreviewWindow::ReloadPreview()
{
    m_Preview->triggerPageAction(QWebEnginePage::ReloadAndBypassCache);
    // m_Preview->triggerPageAction(QWebEnginePage::Reload);
}

void PreviewWindow::LoadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // m_Layout->restoreState(settings.value("layout").toByteArray());
    settings.endGroup();
}

void PreviewWindow::ConnectSignalsToSlots()
{
    connect(m_Preview,   SIGNAL(ZoomFactorChanged(float)), this, SIGNAL(ZoomFactorChanged(float)));
    connect(m_Preview,   SIGNAL(LinkClicked(const QUrl &)), this, SLOT(LinkClicked(const QUrl &)));
    connect(m_inspectButton, SIGNAL(clicked()),     this, SLOT(InspectPreviewPage()));
    connect(m_selectButton,  SIGNAL(clicked()),     this, SLOT(SelectAllPreview()));
    connect(m_copyButton,    SIGNAL(clicked()),     this, SLOT(CopyPreview()));
    connect(m_reloadButton,  SIGNAL(clicked()),     this, SLOT(ReloadPreview()));
    connect(m_Inspector,     SIGNAL(finished(int)), this, SLOT(InspectorClosed(int)));
}

