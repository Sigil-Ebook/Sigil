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

    QString css_epub2_spec = "css21";

    if (ui.Epub2css20->isChecked()) {
        css_epub2_spec = "css2";
    } else if (ui.Epub2css30->isChecked()) {
        css_epub2_spec = "css3";
    }

    QString css_epub3_spec = "css3";

    if (ui.Epub3css20->isChecked()) {
        css_epub3_spec = "css2";
    } else if (ui.Epub3css21->isChecked()) {
        css_epub3_spec = "css21";
    }

    int new_remote_on_level = 0;

    if (ui.AllowRemote->isChecked()) {
        new_remote_on_level = 1;
    }

    SettingsStore settings;
    settings.setCleanOn(new_clean_on_level);
    settings.setDefaultVersion(new_epub_version);
    settings.setCssEpub2ValidationSpec(css_epub2_spec);
    settings.setCssEpub3ValidationSpec(css_epub3_spec);
    settings.setRemoteOn(new_remote_on_level);
    return PreferencesWidget::ResultAction_None;
}

void GeneralSettingsWidget::readSettings()
{
    SettingsStore settings;
    QString version = settings.defaultVersion();
    ui.EpubVersion2->setChecked(version == "2.0");
    ui.EpubVersion3->setChecked(version == "3.0");
    QString css_epub2_spec = settings.cssEpub2ValidationSpec();
    ui.Epub2css20->setChecked(css_epub2_spec == "css2");
    ui.Epub2css21->setChecked(css_epub2_spec == "css21");
    ui.Epub2css30->setChecked(css_epub2_spec == "css3");
    QString css_epub3_spec = settings.cssEpub3ValidationSpec();
    ui.Epub3css20->setChecked(css_epub3_spec == "css2");
    ui.Epub3css21->setChecked(css_epub3_spec == "css21");
    ui.Epub3css30->setChecked(css_epub3_spec == "css3");
    int cleanOn = settings.cleanOn();
    ui.MendOnOpen->setChecked(cleanOn & CLEANON_OPEN);
    ui.MendOnSave->setChecked(cleanOn & CLEANON_SAVE);
    int remoteOn = settings.remoteOn();
    ui.AllowRemote->setChecked(remoteOn);
}

