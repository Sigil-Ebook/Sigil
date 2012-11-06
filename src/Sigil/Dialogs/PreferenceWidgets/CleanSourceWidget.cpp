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

    SettingsStore settings;

    settings.setCleanLevel(new_clean_level);

    if (new_clean_level != m_CleanLevel) {
        return PreferencesWidget::ResultAction_RestartSigil;
    }

    return PreferencesWidget::ResultAction_None;
}

void CleanSourceWidget::readSettings()
{
    SettingsStore settings;

    m_CleanLevel = settings.cleanLevel();

    ui.CleanLevelOff->setChecked(m_CleanLevel == SettingsStore::CleanLevel_Off);
    ui.CleanLevelPrettyPrint->setChecked(m_CleanLevel == SettingsStore::CleanLevel_PrettyPrint);
    ui.CleanLevelPrettyPrintTidy->setChecked(m_CleanLevel == SettingsStore::CleanLevel_PrettyPrintTidy);
    ui.CleanLevelTidy->setChecked(m_CleanLevel == SettingsStore::CleanLevel_Tidy);
}
