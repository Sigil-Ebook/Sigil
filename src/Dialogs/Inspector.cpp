/************************************************************************
 **
 **  Copyright (C) 2019  Kevin B. Hendricks, Stratford Ontario Canada
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
#include <QtWebEngineWidgets/QWebEngineView>
#include <QtWebEngineWidgets/QWebEnginePage>
#include <QApplication>
#include <QDebug>

#include "Misc/SettingsStore.h"
#include "Dialogs/Inspector.h"

static const QString SETTINGS_GROUP = "inspect_dialog";


Inspector::Inspector(QWidget *parent) :
    QDialog(parent),
    m_Layout(new QVBoxLayout(this)),
    m_inspectView(new QWebEngineView(this)),
    m_page(nullptr)
{
    m_Layout->addWidget(m_inspectView);
    LoadSettings();
}

Inspector::~Inspector()
{
  if (m_inspectView) {
      m_inspectView->close();
      m_inspectView->page()->setInspectedPage(nullptr);
      m_page = nullptr;
      delete m_inspectView;
      m_inspectView = nullptr;
  }
}

void Inspector::InspectPage(QWebEnginePage* page)
{
    m_page = page;
    m_inspectView->page()->setInspectedPage(m_page);
}

void Inspector::StopInspection()
{
    SaveSettings();
    m_page = nullptr;
    m_inspectView->page()->setInspectedPage(m_page);
}


QSize Inspector::sizeHint()
{
  return QSize(400,200);
}

void Inspector::closeEvent(QCloseEvent* event)
{
    qDebug() << "Inspector Close Event";
    StopInspection();
    QDialog::closeEvent(event);
}

void Inspector::LoadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    QByteArray geometry = settings.value("geometry").toByteArray();

    if (!geometry.isNull()) {
        restoreGeometry(geometry);
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

