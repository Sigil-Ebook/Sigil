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

#include <stdafx.h>
#include <QtCore/QDir>
#include <QtCore/QUrl>
#include <QtGui/QDesktopServices>
#include <QtGui/QMessageBox>
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

    SettingsStore *store = SettingsStore::instance();
    store->setDictionary(ui.dictionaries->currentText());
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
    SpellCheck *sc = SpellCheck::instance();

    // Load the avaliable dictionary names.
    QStringList dicts = sc->dictionaries();
    dicts.prepend(tr("None"));
    ui.dictionaries->addItems(dicts);

    // Select the current dictionary.
    QString currentDict = sc->currentDictionary();
    if (!currentDict.isEmpty()) {
        int index = ui.dictionaries->findText(currentDict);
        if (index > -1) {
            ui.dictionaries->setCurrentIndex(index);
        }
    }

    // Load the words in the user dictionary word list.
    QStringList words = sc->userDictionaryWords();
    foreach (QString word, words) {
        QListWidgetItem *item = new QListWidgetItem(word, ui.userWordList);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ui.userWordList->addItem(item);
    }
}

void SpellCheckWidget::openUserDictionaryLocation()
{
    SpellCheck *sc = SpellCheck::instance();
    QString dictDir = sc->dictionaryDirectory();

    // Check if the directory exists and create it if necessary.
    QDir loc(dictDir);
    if (!loc.exists()) {
        loc.mkpath(dictDir);
    }

    // Try to open the users file manager to the location and show
    // and error message if this is not possible.
    QUrl locUrl("file:///" + dictDir);
    if (!QDesktopServices::openUrl(locUrl)) {
        QMessageBox::warning(this, tr("Error"), tr("Could not open user dictionary location %1").arg(dictDir));
    }
}

void SpellCheckWidget::connectSignalsSlots()
{
    connect(ui.addWord, SIGNAL(clicked()), this, SLOT(addWord()));
    connect(ui.editWord, SIGNAL(clicked()), this, SLOT(editWord()));
    connect(ui.removeWord, SIGNAL(clicked()), this, SLOT(removeWord()));
    connect(ui.removeAll, SIGNAL(clicked()), this, SLOT(removeAll()));
    connect(ui.openUserDictionaryLocation, SIGNAL(clicked()), this, SLOT(openUserDictionaryLocation()));
}
