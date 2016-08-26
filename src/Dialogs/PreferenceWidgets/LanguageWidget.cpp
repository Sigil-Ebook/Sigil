/************************************************************************
**
**  Copyright (C) 2011  John Schember <john@nachtimwald.com>
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

#include "LanguageWidget.h"
#include "Misc/SettingsStore.h"
#include "Misc/Language.h"
#include "Misc/UILanguage.h"

#include <QString>
#include <QStringList>

LanguageWidget::LanguageWidget()
{
    ui.setupUi(this);
    // Metadata language combobox
    foreach(QString lang, Language::instance()->GetSortedPrimaryLanguageNames()) {
        ui.cbMetadataLanguage->addItem(lang);
    }
    // UI language combobox - smaller subset of available languages
    QStringList ui_language_names;
    foreach(QString language_code, UILanguage::GetUILanguages()) {
        // Convert standard language codes to those used for translations.
        QString std_language_code = language_code;
        std_language_code.replace("_", "-");
        QString language_name = Language::instance()->GetLanguageName(std_language_code);

        if (language_name.isEmpty()) {
            language_name = language_code;
        }

        ui_language_names.append(language_name);
    }
    ui_language_names.sort();
    foreach(QString ui_language_name, ui_language_names) {
        ui.cbUILanguage->addItem(ui_language_name);
    }
    readSettings();
}

PreferencesWidget::ResultAction LanguageWidget::saveSettings()
{
    SettingsStore settings;
    settings.setDefaultMetadataLang(Language::instance()->GetLanguageCode(ui.cbMetadataLanguage->currentText()));
    settings.setUILanguage(Language::instance()->GetLanguageCode(ui.cbUILanguage->currentText()).replace("-", "_"));

    if (ui.cbUILanguage->currentText() != m_UILanguage) {
        return PreferencesWidget::ResultAction_RestartSigil;
    }

    return PreferencesWidget::ResultAction_None;
}

void LanguageWidget::readSettings()
{
    SettingsStore settings;
    // Metadata Language
    int index = ui.cbMetadataLanguage->findText(Language::instance()->GetLanguageName(settings.defaultMetadataLang()));

    if (index == -1) {
        index = ui.cbMetadataLanguage->findText(Language::instance()->GetLanguageName("en"));

        if (index == -1) {
            index = 0;
        }
    }

    ui.cbMetadataLanguage->setCurrentIndex(index);
    // UI Language
    index = ui.cbUILanguage->findText(Language::instance()->GetLanguageName(settings.uiLanguage().replace("_", "-")));

    if (index == -1) {
        index = ui.cbUILanguage->findText(Language::instance()->GetLanguageName("en"));

        if (index == -1) {
            index = 0;
        }
    }

    ui.cbUILanguage->setCurrentIndex(index);
    m_UILanguage = ui.cbUILanguage->currentText();
}
