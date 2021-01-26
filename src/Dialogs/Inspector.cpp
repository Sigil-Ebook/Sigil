/************************************************************************
 **
 **  Copyright (C) 2019-2021 Kevin B. Hendricks, Stratford Ontario Canada
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

#include <QByteArray>
#include <QKeySequence>
#include <QtWebEngineWidgets/QWebEngineView>
#include <QtWebEngineWidgets/QWebEnginePage>
#include <QtWebEngineWidgets/QWebEngineSettings>
#include <QtWebEngineWidgets/QWebEngineProfile>
#include <QApplication>
#include <QDir>

#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "Dialogs/Inspector.h"

static const QString SETTINGS_GROUP = "inspect_dialog";

const float ZOOM_STEP               = 0.1f;
const float ZOOM_MIN                = 0.09f;
const float ZOOM_MAX                = 5.0f;
const float ZOOM_NORMAL             = 1.0f;

Inspector::Inspector(QWidget *parent, Qt::WindowFlags flags) :
    QDialog(parent, flags),
    m_Layout(new QVBoxLayout(this)),
    m_inspectView(new QWebEngineView(this)),
    m_view(nullptr),
    m_LoadingFinished(false),
    m_LoadOkay(false),
    m_ZoomIn(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Plus), this)),
    m_ZoomOut(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Minus), this)),
    m_ZoomReset(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_0), this))
{
    setAttribute(Qt::WA_DeleteOnClose, false);
    setWindowTitle(tr("Inspect Page or Element"));
    setMinimumSize(QSize(200, 200));
    m_Layout->addWidget(m_inspectView);
    // QtWebEngine WebInspector needs to run javascript in MainWorld
    // so override the app default but just for this inspector
    m_inspectView->page()->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);

    // The Inspector also needs access to local-storage to save its dark mode settings between launches
    m_inspectView->page()->settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    QString inspectorStorePath = Utility::DefinePrefsDir() + "/local-devtools";
    QDir storageDir(inspectorStorePath);
    if (!storageDir.exists()) {
        storageDir.mkpath(inspectorStorePath);
    }
    m_inspectView->page()->profile()->setPersistentStoragePath(inspectorStorePath);

    LoadSettings();
    connect(m_inspectView->page(), SIGNAL(loadFinished(bool)), this, SLOT(UpdateFinishedState(bool)));
    connect(m_inspectView->page(), SIGNAL(loadStarted()),      this, SLOT(LoadingStarted()));
    connect(m_ZoomIn,              SIGNAL(activated()),        this, SLOT(ZoomIn()));
    connect(m_ZoomOut,             SIGNAL(activated()),        this, SLOT(ZoomOut()));
    connect(m_ZoomReset,           SIGNAL(activated()),        this, SLOT(ZoomReset()));
}
       

Inspector::~Inspector()
{
    if (m_inspectView) {
        m_inspectView->close();
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        m_inspectView->page()->setInspectedPage(nullptr);
#endif
        m_view = nullptr;
        delete m_inspectView;
        m_inspectView = nullptr;
    }
}

// Start of Zoom Related Routines
void Inspector::SetZoomFactor(float factor)
{
    if (factor > ZOOM_MAX) factor = ZOOM_MAX;
    if (factor < ZOOM_MIN) factor = ZOOM_MIN;
    SettingsStore settings;
    settings.setZoomInspector(factor);
    SetCurrentZoomFactor(factor);
    Zoom();
}

void  Inspector::ZoomIn()    { ZoomByStep(true);           }
void  Inspector::ZoomOut()   { ZoomByStep(false);          }
void  Inspector::ZoomReset() { SetZoomFactor(ZOOM_NORMAL); }

void  Inspector::SetCurrentZoomFactor(float factor) { m_CurrentZoomFactor = factor; }
float Inspector::GetZoomFactor() const              { return m_CurrentZoomFactor;   }

void Inspector::Zoom()
{
    m_inspectView->setZoomFactor(m_CurrentZoomFactor);
}

void Inspector::ZoomByStep(bool zoom_in)
{
    // zoom out - neg. zoom step, round down; zoom in  - pos. zoom step, round UP
    float zoom_stepping       = zoom_in ? ZOOM_STEP : - ZOOM_STEP;
    float rounding_helper     = zoom_in ? 0.05f : - 0.05f;
    float current_zoom_factor = GetZoomFactor();
    float rounded_zoom_factor = Utility::RoundToOneDecimal(current_zoom_factor + rounding_helper);
    if (qAbs(current_zoom_factor - rounded_zoom_factor) < 0.01f) {
        SetZoomFactor(Utility::RoundToOneDecimal(current_zoom_factor + zoom_stepping));
    } else {
        SetZoomFactor(rounded_zoom_factor);
    }
}
// End of Zoom Related Routines


void Inspector::LoadingStarted()
{
    m_LoadingFinished = false;
    m_LoadOkay = false;
}

void Inspector::UpdateFinishedState(bool okay)
{
    m_LoadingFinished = true;
    m_LoadOkay = okay;
}

void Inspector::InspectPageofView(QWebEngineView* view)
{
    m_view = view;

#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    if (m_view) {
        m_inspectView->page()->setInspectedPage(m_view->page());
    }
#else
    if (m_view) {
        QString not_supported = tr("The Inspector functionality is not supported before Qt 5.11");
        QString response = "<html><head><title>Warning</title></head><body><p>" + 
                    not_supported + "</p></body></html>";
        m_inspectView->setHtml(response);
        show();
    }
#endif
}

void Inspector::StopInspection()
{
    SaveSettings();
    m_view = nullptr;
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    m_inspectView->page()->setInspectedPage(nullptr);
#else
    m_inspectView->setHtml("<html><head><title></title></head><body></body></html>");
#endif
}

QSize Inspector::sizeHint()
{
  return QSize(450,250);
}

void Inspector::closeEvent(QCloseEvent* event)
{
    StopInspection();
    QDialog::closeEvent(event);
}

void Inspector::LoadSettings()
{
    SettingsStore settings;

    // The zoom inspector setting is in the user preference group
    SetCurrentZoomFactor(settings.zoomInspector());

    settings.beginGroup(SETTINGS_GROUP);
    QByteArray geometry = settings.value("geometry").toByteArray();
    if (!geometry.isNull()) {
        restoreGeometry(geometry);
    } else {
        resize(sizeHint());
    }
    settings.endGroup();
}

void Inspector::SaveSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    settings.setValue("geometry", saveGeometry());
    QApplication::restoreOverrideCursor();
    settings.endGroup();
}
