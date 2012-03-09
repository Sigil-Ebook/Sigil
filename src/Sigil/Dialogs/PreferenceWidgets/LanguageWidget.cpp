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
#include "Misc/Language.h"
#include "Misc/UILanguage.h"
#include "QtGui/QMessageBox"

#include <QString>
#include <QStringList>

LanguageWidget::LanguageWidget()
{
    ui.setupUi(this);


    // Metadata language combobox
    foreach( QString lang, Language::instance()->GetSortedPrimaryLanguageNames() )
    {
        ui.cbMetadataLanguage->addItem(lang);
    }

    // UI language combobox - smaller subset of available languages
    QStringList ui_language_names;
    foreach( QString language_code, UILanguage::GetUILanguages() )
    {
        ui_language_names.append( Language::instance()->GetLanguageName( language_code ) );
    }
    ui_language_names.sort();

    foreach( QString ui_language_name, ui_language_names )
    {
        ui.cbUILanguage->addItem( ui_language_name );
    }

    readSettings();
}

void LanguageWidget::saveSettings()
{
    SettingsStore settings;

    settings.setDefaultMetadataLang( Language::instance()->GetLanguageCode( ui.cbMetadataLanguage->currentText() ) );
    settings.setUILanguage( Language::instance()->GetLanguageCode( ui.cbUILanguage->currentText() ) );

    if ( ui.cbUILanguage->currentText() != m_UILanguage )
    {
        QMessageBox::warning( this, tr( "Restart Required" ), tr( "You must restart Sigil to show the User Interface in a different language." ) );
    }
}

void LanguageWidget::readSettings()
{
    SettingsStore settings;

    // Metadata Language
    int index = ui.cbMetadataLanguage->findText( Language::instance()->GetLanguageName( settings.defaultMetadataLang() ) );
    if ( index == -1 ) {
        index = ui.cbMetadataLanguage->findText( Language::instance()->GetLanguageName( "en" ) );
        if (index == -1) {
            index = 0;
        }
    }
    ui.cbMetadataLanguage->setCurrentIndex( index );

    // UI Language
    index = ui.cbUILanguage->findText( Language::instance()->GetLanguageName( settings.uiLanguage() ) );
    if ( index == -1 ) {
        index = ui.cbUILanguage->findText( Language::instance()->GetLanguageName( "en" ) );
        if ( index == -1 ) {
            index = 0;
        }
    }
    ui.cbUILanguage->setCurrentIndex( index );
    m_UILanguage= ui.cbUILanguage->currentText();
}
