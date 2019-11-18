/************************************************************************
**
**  Copyright (C) 2019 Kevin B. Hendricks, Stratford Ontario Canada
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

#include "Dialogs/RERenamer.h"
#include "Misc/SettingsStore.h"

static QString SETTINGS_GROUP = "re_renamer";

RERenamer::RERenamer(QWidget *parent)
    :
    QDialog(parent),
    m_REText(QString()),
    m_ReplaceText(QString())
{
    ui.setupUi(this);
    connectSignalsSlots();
    ReadSettings();
}

QString RERenamer::GetREText()
{
    return m_REText;
}


QString RERenamer::GetReplaceText()
{
    return m_ReplaceText;
}


void RERenamer::SetREText()
{
    m_REText = ui.rexLineEdit->text();
}

void RERenamer::SetReplaceText()
{
    m_ReplaceText = ui.repLineEdit->text();
}

void RERenamer::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    QByteArray geometry = settings.value("geometry").toByteArray();

    if (!geometry.isNull()) {
        restoreGeometry(geometry);
    }

    settings.endGroup();
}

void RERenamer::WriteSettings()
{
    SetREText();
    SetReplaceText();
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
}

void RERenamer::connectSignalsSlots()
{
    connect(this, SIGNAL(accepted()), this, SLOT(WriteSettings()));
}
