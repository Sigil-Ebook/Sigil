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

#include "Dialogs/PreferenceWidgets/CleanSourceWidget.h"

#include <QString>
#include <QStringList>

CleanSourceWidget::CleanSourceWidget()
{
    ui.setupUi(this);

    readSettings();
}

PreferencesWidget::ResultAction CleanSourceWidget::saveSettings()
{
    SettingsStore::CleanLevel new_clean_level = SettingsStore::CleanLevel_Off;
    int new_clean_on_level = 0;

    if (ui.CleanLevelTidy->isChecked()) {
        new_clean_level = SettingsStore::CleanLevel_Tidy;
    }
    else if (ui.CleanLevelPrettyPrint->isChecked()) {
        new_clean_level = SettingsStore::CleanLevel_PrettyPrint;
    }
    else if (ui.CleanLevelPrettyPrintTidy->isChecked()) {
        new_clean_level = SettingsStore::CleanLevel_PrettyPrintTidy;
    }
    else {
        new_clean_level = SettingsStore::CleanLevel_Off;
    }

    if (ui.CleanOnOpen->isChecked()) {
        new_clean_on_level |= CLEANON_OPEN;
    }
    if (ui.CleanOnSave->isChecked()) {
        new_clean_on_level |= CLEANON_SAVE;
    }
    if (ui.CleanOnReplaceInAll->isChecked()) {
        new_clean_on_level |= CLEANON_REPLACEINALL;
    }

    SettingsStore settings;

    settings.setCleanLevel(new_clean_level);
    settings.setCleanOn(new_clean_on_level);

    return PreferencesWidget::ResultAction_None;
}

void CleanSourceWidget::readSettings()
{
    SettingsStore settings;

    SettingsStore::CleanLevel level = settings.cleanLevel();
    ui.CleanLevelOff->setChecked(level == SettingsStore::CleanLevel_Off);
    ui.CleanLevelPrettyPrint->setChecked(level == SettingsStore::CleanLevel_PrettyPrint);
    ui.CleanLevelPrettyPrintTidy->setChecked(level == SettingsStore::CleanLevel_PrettyPrintTidy);
    ui.CleanLevelTidy->setChecked(level == SettingsStore::CleanLevel_Tidy);

    int cleanOn = settings.cleanOn();
    ui.CleanOnOpen->setChecked(cleanOn & CLEANON_OPEN);
    ui.CleanOnSave->setChecked(cleanOn & CLEANON_SAVE);
    ui.CleanOnReplaceInAll->setChecked(cleanOn & CLEANON_REPLACEINALL);
}

