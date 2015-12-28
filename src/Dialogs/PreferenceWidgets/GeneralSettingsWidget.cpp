/************************************************************************
**
**  Copyright (C) 2011  John Schember <john@nachtimwald.com>
**  Copyright (C) 2012  Dave Heiland
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

#include "Dialogs/PreferenceWidgets/GeneralSettingsWidget.h"

#include <QString>
#include <QStringList>

GeneralSettingsWidget::GeneralSettingsWidget()
{
    ui.setupUi(this);
    readSettings();
}

PreferencesWidget::ResultAction GeneralSettingsWidget::saveSettings()
{

    int new_clean_on_level = 0;
    QString new_epub_version = "2.0";

    if (ui.EpubVersion3->isChecked()) {
        new_epub_version = "3.0";
    }
    if (ui.MendOnOpen->isChecked()) {
        new_clean_on_level |= CLEANON_OPEN;
    }
    if (ui.MendOnSave->isChecked()) {
        new_clean_on_level |= CLEANON_SAVE;
    }

    SettingsStore settings;
    settings.setCleanOn(new_clean_on_level);
    settings.setDefaultVersion(new_epub_version);
    return PreferencesWidget::ResultAction_None;
}

void GeneralSettingsWidget::readSettings()
{
    SettingsStore settings;
    QString version = settings.defaultVersion();
    ui.EpubVersion2->setChecked(version == "2.0");
    ui.EpubVersion3->setChecked(version == "3.0");
    int cleanOn = settings.cleanOn();
    ui.MendOnOpen->setChecked(cleanOn & CLEANON_OPEN);
    ui.MendOnSave->setChecked(cleanOn & CLEANON_SAVE);
}

