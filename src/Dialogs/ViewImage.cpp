/************************************************************************
**
**  Copyright (C) 2019-2020 Kevin B. Hendricks
**  Copyright (C) 2013      Dave Heiland
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
#include <QImage>
#include <QToolButton>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QApplication>
#include <QGuiApplication>
#include <QtWidgets/QLayout>

#include "ResourceObjects/ImageResource.h"
#include "ViewEditors/SimplePage.h"
#include "Misc/Utility.h"
#include "MainUI/MainApplication.h"
#include "Widgets/ImageView.h"
#include "Dialogs/ViewImage.h"

static QString SETTINGS_GROUP = "view_image";

ViewImage::ViewImage(QWidget *parent)
    :
    QDialog(parent),
    m_iv(new ImageView(this)),
    m_bp(new QToolButton(this)),
    m_layout(new QVBoxLayout(this))
{
    m_layout->addWidget(m_iv);
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

ViewImage::~ViewImage()
{
    WriteSettings();
}

QSize ViewImage::sizeHint()
{
    return QSize(450,450);
}

void ViewImage::ShowImage(QString path)
{
    m_iv->ShowImage(path);
    QApplication::processEvents();
}

void ViewImage::ReloadViewer()
{
    m_iv->ReloadViewer();
}

void ViewImage::ReadSettings()
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

void ViewImage::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
}

void ViewImage::ConnectSignalsToSlots()
{
#ifdef Q_OS_MAC
    MainApplication *mainApplication = qobject_cast<MainApplication *>(qApp);
    connect(mainApplication, SIGNAL(applicationPaletteChanged()), this, SLOT(ReloadViewer()));
#endif
    connect(m_bp, SIGNAL(clicked()), this, SLOT(accept()));
}
