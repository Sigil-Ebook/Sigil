/************************************************************************
 **
 **  Copyright (C) 2020-2021 Kevin B. Hendricks
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
#include <QFileInfo>
#include <QApplication>
#include <QGuiApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGuiApplication>
#include <QApplication>
#include "MainUI/MainApplication.h"
#include "Misc/Utility.h"
#include "Misc/SettingsStore.h"
#include "Widgets/FontView.h"
#include "Dialogs/ViewFont.h"

static QString SETTINGS_GROUP = "view_font";

ViewFont::ViewFont(QWidget *parent)
    : QDialog(parent),
      m_fv(new FontView(this)),
      m_bp(new QToolButton(this)),
      m_layout(new QVBoxLayout(this))
{
    m_layout->addWidget(m_fv);
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

ViewFont::~ViewFont()
{
    WriteSettings();
}


QSize ViewFont::sizeHint()
{
    return QSize(450,250);
}

void ViewFont::ShowFont(QString path)
{
    m_fv->ShowFont(path);
    QApplication::processEvents();
}

void ViewFont::ReloadViewer()
{
    m_fv->ReloadViewer();
}

void ViewFont::ReadSettings()
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

void ViewFont::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
}

void ViewFont::ConnectSignalsToSlots()
{
#ifdef Q_OS_MAC
    MainApplication *mainApplication = qobject_cast<MainApplication *>(qApp);
    connect(mainApplication, SIGNAL(applicationPaletteChanged()), this, SLOT(ReloadViewer()));
#endif
    connect(m_bp, SIGNAL(clicked()), this, SLOT(accept()));
}
