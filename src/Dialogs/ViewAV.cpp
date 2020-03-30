/************************************************************************
 **
 **  Copyright (C) 2020 Kevin B. Hendricks
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

#include <QString>
#include <QToolButton>
#include <QFileInfo>
#include <QApplication>
#include <QGuiApplication>
#include <QUrl>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QtWebEngineWidgets/QWebEngineProfile>
#include <QtWebEngineWidgets/QWebEngineView>
#include <QGuiApplication>
#include <QApplication>
#include "ViewEditors/SimplePage.h"
#include "Misc/Utility.h"
#include "Misc/SettingsStore.h"
#include "MainUI/MainApplication.h"
#include "Widgets/AVView.h"
#include "Dialogs/ViewAV.h"

static QString SETTINGS_GROUP = "view_av";

ViewAV::ViewAV(QWidget *parent)
    : QDialog(parent),
      m_av(new AVView(this)),
      m_bp(new QToolButton(this)),
      m_layout(new QVBoxLayout(this))
{
    m_layout->addWidget(m_av);
    m_bp->setToolTip(tr("Close this window"));
    m_bp->setText(tr("Done"));
    m_bp->setToolButtonStyle(Qt::ToolButtonTextOnly);
    QHBoxLayout* hl = new QHBoxLayout();
    hl->addStretch(0);
    hl->addWidget(m_bp);
    m_layout->addLayout(hl);
    ReadSettings();
    ConnectSignalsToSlots();
}

ViewAV::~ViewAV()
{
    WriteSettings();
}


QSize ViewAV::sizeHint()
{
    return QSize(450,250);
}

void ViewAV::ShowAV(QString path)
{
    m_av->ShowAV(path);
    QApplication::processEvents();
}

void ViewAV::ReloadViewer()
{
    m_av->ReloadViewer();
}

void ViewAV::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    QByteArray geometry = settings.value("geometry").toByteArray();
    if (!geometry.isNull()) {
        restoreGeometry(geometry);
    } else {
	resize(sizeHint());
    }
    settings.endGroup();
}

void ViewAV::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
}

void ViewAV::ConnectSignalsToSlots()
{
#ifdef Q_OS_MAC
    MainApplication *mainApplication = qobject_cast<MainApplication *>(qApp);
    connect(mainApplication, SIGNAL(applicationPaletteChanged()), this, SLOT(ReloadViewer()));
#endif
    connect(m_bp, SIGNAL(clicked()), this, SLOT(accept()));
}
