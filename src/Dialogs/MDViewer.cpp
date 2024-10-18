/************************************************************************
**
**  Copyright (C) 2024 Kevin B. Hendricks, Stratford Ontario Canada
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

#include <QLineEdit>

#include "Dialogs/MDViewer.h"
#include "ResourceObjects/HTMLResource.h"
#include "Misc/SettingsStore.h"

static QString SETTINGS_GROUP = "markdown_viewer";

MDViewer::MDViewer(const QString & mdsrc, QWidget *parent)
    :
    QDialog(parent)
{
    ui.setupUi(this);
    connectSignalsSlots();
    ReadSettings();
    ui.viewer->setMarkdown(mdsrc);
    ui.viewer->setReadOnly(true);
}

void MDViewer::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    QByteArray geometry = settings.value("geometry").toByteArray();
    if (!geometry.isNull()) {
        restoreGeometry(geometry);
    }
    settings.endGroup();
}

void MDViewer::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
}

void MDViewer::connectSignalsSlots()
{
    connect(this, SIGNAL(accepted()), this, SLOT(WriteSettings()));
}
