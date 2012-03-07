/************************************************************************
**
**  Copyright (C) 2009, 2010, 2011  Strahinja Markovic  <strahinja.markovic@gmail.com>
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

#include <QtCore/QDir>
#include <QtCore/QUrl>
#include <QtGui/QDesktopServices>
#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>

#include "SpellCheckWidget.h"
#include "Misc/SettingsStore.h"
#include "Misc/SpellCheck.h"

SpellCheckWidget::SpellCheckWidget()
{
    ui.setupUi(this);

    connectSignalsSlots();
    readSettings();
}

void SpellCheckWidget::saveSettings()
{
    SpellCheck *sc = SpellCheck::instance();
    sc->setDictionary(ui.dictionaries->currentText());

    QStringList words;
    for (int i = 0; i < ui.userWordList->count(); ++i) {
        words << ui.userWordList->item(i)->text();
    }

    sc->replaceUserDictionaryWords(words);

    SettingsStore settings;
    settings.setDictionary(ui.dictionaries->currentText());
    settings.setSpellCheck(ui.enableSpellCheck->isChecked());
}

void SpellCheckWidget::addWord()
{
    QListWidgetItem *item = new QListWidgetItem(ui.userWordList);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    ui.userWordList->addItem(item);
    ui.userWordList->editItem(item);
}

void SpellCheckWidget::editWord()
{
    QList<QListWidgetItem*> items = ui.userWordList->selectedItems();
    if (!items.empty()) {
        ui.userWordList->editItem(items.at(0));
    }
}

void SpellCheckWidget::removeWord()
{
    foreach (QListWidgetItem *item, ui.userWordList->selectedItems()) {
        ui.userWordList->removeItemWidget(item);
        delete item;
        item = 0;
    }
}

void SpellCheckWidget::removeAll()
{
    ui.userWordList->clear();
}

void SpellCheckWidget::readSettings()
{
    // Load the available dictionary names.
    SpellCheck *sc = SpellCheck::instance();
    QStringList dicts = sc->dictionaries();
    ui.dictionaries->clear();
    ui.dictionaries->addItems(dicts);

    // Select the current dictionary.
    QString currentDict = sc->currentDictionary();

    SettingsStore settings;
	ui.enableSpellCheck->setChecked(settings.spellCheck());

    if (!currentDict.isEmpty()) {
        int index = ui.dictionaries->findText(currentDict);
        if (index > -1) {
            ui.dictionaries->setCurrentIndex(index);
        }
    }

    // Load the words in the user dictionary word list.
    ui.userWordList->clear();
    QStringList words = sc->userDictionaryWords();
    foreach (QString word, words) {
        QListWidgetItem *item = new QListWidgetItem(word, ui.userWordList);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ui.userWordList->addItem(item);
    }

    // Update the description of the user dictionary file
    QString tooltip_file = tr ( "Choose the file that contains your word list.<P>Current location: " ) %
                           settings.userDictionaryFile();
                         
    ui.pbUserDictionaryFile->setToolTip( tooltip_file );


    // Update the description of the directory location
    QString tooltip_directory = tr  ( "Choose the directory where Sigil should look for additional dictionaries."
                                      "<P>Dictionaries in this location will be added to the list of dictionaries you can choose from "
                                      "and will override default dictionaries that have the same names.<P>Current location: " ) %
                                settings.userDictionaryFile();

    ui.pbDictionaryDirectory->setToolTip( tooltip_directory );
}

void SpellCheckWidget::selectDictionaryDirectory()
{
    SettingsStore settings;

    QString directory = QFileDialog::getExistingDirectory(  this,
                                                            tr( "Select Additional Dictionaries Directory" ),
                                                            settings.dictionaryDirectory()
                                                         );

    if ( !directory.isEmpty() )
    {
        settings.setDictionaryDirectory(directory);
        readSettings();
    }
}

void SpellCheckWidget::selectUserDictionaryFile()
{
    if ( isWordListModified() )
    {
        QMessageBox::critical( this, tr( "Unable to change User Dictionary File" ), 
                                     tr( "The User Word List has been edited but not saved.<P>"
                                         "You must save your current word list before changing the User Dictionary File." ) );
        return;
    }

    SettingsStore settings;
    QString filter_string = "";
    QString default_filter = "";
    QString filepath = QFileDialog::getSaveFileName(    this,
                                                        tr( "Select User Dictionary File" ),
                                                        settings.userDictionaryFile(),
                                                        filter_string,
                                                        &default_filter,
                                                        QFileDialog::DontConfirmOverwrite
                                                   );

    if ( !filepath.isEmpty() )
    {
        settings.setUserDictionaryFile(filepath);
        readSettings();
    }
}

bool SpellCheckWidget::isWordListModified()
{
    SpellCheck *sc = SpellCheck::instance();
    QStringList saved_words = sc->userDictionaryWords();

    bool word_list_modified = saved_words.count() != ui.userWordList->count();
    int i = 0;
    if ( !word_list_modified )
    {
        foreach( QString word, saved_words )
        {
            if ( ui.userWordList->item(i++)->text() != word )
            {
                word_list_modified = true;
                break;
            }
        }
    }

    return word_list_modified;
}

void SpellCheckWidget::connectSignalsSlots()
{
    connect(ui.addWord, SIGNAL(clicked()), this, SLOT(addWord()));
    connect(ui.editWord, SIGNAL(clicked()), this, SLOT(editWord()));
    connect(ui.removeWord, SIGNAL(clicked()), this, SLOT(removeWord()));
    connect(ui.removeAll, SIGNAL(clicked()), this, SLOT(removeAll()));
    connect(ui.pbDictionaryDirectory, SIGNAL(clicked()), this, SLOT(selectDictionaryDirectory()));
    connect(ui.pbUserDictionaryFile, SIGNAL(clicked()), this, SLOT(selectUserDictionaryFile()));
}
