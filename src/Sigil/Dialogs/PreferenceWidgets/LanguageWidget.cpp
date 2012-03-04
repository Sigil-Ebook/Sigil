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
#include "BookManipulation/Metadata.h"
#include "Misc/SettingsStore.h"

LanguageWidget::LanguageWidget()
{
    ui.setupUi(this);

    foreach(QString lang, Metadata::Instance().GetLanguageMap().keys()) {
        ui.metadataLang->addItem(lang);
    }

    readSettings();
}

void LanguageWidget::saveSettings()
{
    SettingsStore settings;

    settings.setDefaultMetadataLang(ui.metadataLang->currentText());
}

void LanguageWidget::readSettings()
{
    SettingsStore settings;

    int index = ui.metadataLang->findText(settings.defaultMetadataLang());
    if (index == -1) {
        index = ui.metadataLang->findText("English");
        if (index == -1) {
            index = 0;
        }
    }
    ui.metadataLang->setCurrentIndex(index);
}
