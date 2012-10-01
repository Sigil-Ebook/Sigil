/************************************************************************
**
**  Copyright (C) 2012 John Schember <john@nachtimwald.com>
**  Copyright (C) 2012 Dave Heiland
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

#include "Dialogs/SelectIndexTitle.h"
#include "Misc/SettingsStore.h"

static QString SETTINGS_GROUP = "select_index_title";

SelectIndexTitle::SelectIndexTitle(QString title, QWidget *parent)
    :
    QDialog(parent),
    m_SelectedTitle(title)
{
    ui.setupUi(this);
    connectSignalsSlots();

    ReadSettings();

    // Set default title
    ui.title->setText(m_SelectedTitle);
}

QString SelectIndexTitle::GetTitle()
{
    return m_SelectedTitle;
}

void SelectIndexTitle::SetSelectedTitle()
{
    m_SelectedTitle = ui.title->text();
}

void SelectIndexTitle::ReadSettings()
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

void SelectIndexTitle::WriteSettings()
{
    SetSelectedTitle();

    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);

    // The size of the window and it's full screen status
    settings.setValue("geometry", saveGeometry());

    settings.endGroup();
}

void SelectIndexTitle::connectSignalsSlots()
{
    connect(this,         SIGNAL(accepted()),
            this,         SLOT(WriteSettings()));
}
