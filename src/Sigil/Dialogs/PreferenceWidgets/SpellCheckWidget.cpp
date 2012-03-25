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
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QUrl>
#include <QtGui/QDesktopServices>
#include <QtGui/QInputDialog>
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
    saveUserDictionaryWordList(ui.userDictList->currentItem());

    SettingsStore settings;
    settings.setDictionary(ui.dictionaries->currentText());
    settings.setSpellCheck(ui.enableSpellCheck->isChecked());

    SpellCheck *sc = SpellCheck::instance();
    sc->setDictionary(settings.dictionary(), true);
}

void SpellCheckWidget::addUserDict()
{
    QString name = QInputDialog::getText(this, tr("Word List Name"), tr("Name:"));
    if (name.isEmpty()) {
        return;
    }

    QStringList currentDicts;
    for (int i = 0; i < ui.userDictList->count(); ++i) {
        QListWidgetItem *item = ui.userDictList->item(i);
        currentDicts << item->text();
    }

    if (currentDicts.contains(name, Qt::CaseInsensitive)) {
        QMessageBox::critical(this, tr("Error"), tr("A user dictionary already exists with this name!"));
        return;
    }

    QString path = SpellCheck::userDictionaryDirectory() + "/" + name;
    QFile dict_file(path);
    if (dict_file.open(QIODevice::WriteOnly|QIODevice::Truncate)) {
        dict_file.close();
    }
    else {
        QMessageBox::critical(this, tr("Error"), tr("Could not create file!"));
        return;
    }

    QListWidgetItem *item = new QListWidgetItem(ui.userDictList);
    ui.userDictList->addItem(item);
}

void SpellCheckWidget::renameUserDict()
{
    QList<QListWidgetItem *> items = ui.userDictList->selectedItems();
    QListWidgetItem *item;
    if (items.empty()) {
        return;
    }
    item = items.at(0);
    QString orig_name = item->text();

    QString new_name = QInputDialog::getText(this, tr("Rename"), tr("Name:"), QLineEdit::Normal, orig_name);

    if (new_name == orig_name) {
        return;
    }

    QStringList currentDicts;
    for (int i = 0; i < ui.userDictList->count(); ++i) {
        QListWidgetItem *item = ui.userDictList->item(i);
        currentDicts << item->text();
    }

    if (currentDicts.contains(new_name, Qt::CaseInsensitive)) {
        QMessageBox::critical(this, tr("Error"), tr("A user dictionary already exists with this name!"));
        return;
    }

    QString orig_path = SpellCheck::userDictionaryDirectory() + "/" + orig_name;
    QString new_path = SpellCheck::userDictionaryDirectory() + "/" + new_name;
    if (!QFile::rename(orig_path, new_path)) {
        QMessageBox::critical(this, tr("Error"), tr("Could not rename file!"));
        return;
    }
    item->setText(new_name);
}

void SpellCheckWidget::removeUserDict()
{
    QListWidgetItem *dict_item;

    // Get the current dictionary.
    QList<QListWidgetItem *> items = ui.userDictList->selectedItems();
    if (items.empty()) {
        return;
    }
    dict_item = items.at(0);

    // Delete the dictionary and remove it from the list.
    QFile::remove(SpellCheck::userDictionaryDirectory() + "/" + dict_item->text());
    if (dict_item) {
        delete dict_item;
        dict_item = 0;
    }
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
    QList<QListWidgetItem *> items = ui.userWordList->selectedItems();
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

    // Load the list of user dictionaries.
    QDir userDictDir(SpellCheck::userDictionaryDirectory());
    QStringList userDicts = userDictDir.entryList(QDir::Files|QDir::NoDotAndDotDot);
    // Get the dict that should be in use.
    QString confUserDict = SettingsStore().userDictionaryName();

    // Ensure at least one user dictionary is avaliable.
    if (userDicts.isEmpty()) {
        userDicts << confUserDict;
        // Create the file.
        QFile confUserDictFile(SpellCheck::userDictionaryDirectory() + "/" + confUserDict);
        if (confUserDictFile.open(QIODevice::WriteOnly)) {
            confUserDictFile.close();
        }
    }

    // Load the list of files into the UI.
    foreach (QString ud, userDicts) {
        QListWidgetItem *item = new QListWidgetItem(ud, ui.userDictList);
        ui.userDictList->addItem(item);
        if (confUserDict == ud) {
            item->setSelected(true);
            ui.userDictList->setCurrentItem(item);
        }
    }

    loadUserDictionaryWordList();
}

void SpellCheckWidget::openDictionaryDirectory()
{
    QString dictDir = SpellCheck::dictionaryDirectory();

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

void SpellCheckWidget::loadUserDictionaryWordList(QListWidgetItem *item)
{
    ui.userWordList->clear();
    QString dict_name;

    if (!item) {
        // Get the current dictionary.
        QList<QListWidgetItem *> items = ui.userDictList->selectedItems();
        if (!items.empty()) {
            item = items.at(0);
        }
    }
    dict_name = item->text();

    // This shouldn't happen but we want to prevent crashes just in case.
    if (dict_name.isEmpty()) {
        return;
    }

    // We store the words in a list instead of loading them directly because
    // we want to sort the list before loading the words.
    QStringList words;
    // Read each word from the user dictionary.
    QFile userDictFile(SpellCheck::userDictionaryDirectory() + "/" + dict_name);
    if (userDictFile.open(QIODevice::ReadOnly)) {
        QTextStream userDictStream(&userDictFile);
        for (QString line = userDictStream.readLine(); !line.isEmpty(); line = userDictStream.readLine()) {
            words << line;
        }
        userDictFile.close();
    }
    words.sort();

    // Load the words into the list.
    foreach (QString word, words) {
        QListWidgetItem *item = new QListWidgetItem(word, ui.userWordList);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ui.userWordList->addItem(item);
    }

    SettingsStore ss;
    ss.setUserDictionaryName(dict_name);
}

void SpellCheckWidget::saveUserDictionaryWordList(QListWidgetItem *item)
{
    SettingsStore ss;
    QString dict_name;
    QString dict_path;

    if (!item) {
        // Get the selected user dictionary.
        QList<QListWidgetItem *> items = ui.userWordList->selectedItems();
        if (items.empty()) {
            return;
        }
        item = items.at(0);
    }
    dict_name = item->text();
    dict_path = SpellCheck::userDictionaryDirectory() + "/" + dict_name;
    ss.setUserDictionaryName(dict_name);

    // Get the word list
    QStringList words;
    for (int i = 0; i < ui.userWordList->count(); ++i) {
        words << ui.userWordList->item(i)->text();
    }
    words.sort();

    // Replace words in the user dictionary.
    QFile userDictFile(dict_path);
    if (userDictFile.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream userDictStream(&userDictFile);
        userDictStream.setCodec("UTF-8");
        foreach (QString word, words) {
            userDictStream << word << "\n";
        }
        userDictFile.close();
    }
}

void SpellCheckWidget::userDictionaryChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (previous) {
        saveUserDictionaryWordList(previous);
    }
    if (current) {
        loadUserDictionaryWordList(current);
    }
}

void SpellCheckWidget::connectSignalsSlots()
{
    // User dict list.
    connect(ui.addUserDict, SIGNAL(clicked()), this, SLOT(addUserDict()));
    connect(ui.renameUserDict, SIGNAL(clicked()), this, SLOT(renameUserDict()));
    connect(ui.removeUserDict, SIGNAL(clicked()), this, SLOT(removeUserDict()));
    connect(ui.userDictList, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this, SLOT(userDictionaryChanged(QListWidgetItem *, QListWidgetItem *)));

    // Word list.
    connect(ui.addWord, SIGNAL(clicked()), this, SLOT(addWord()));
    connect(ui.editWord, SIGNAL(clicked()), this, SLOT(editWord()));
    connect(ui.removeWord, SIGNAL(clicked()), this, SLOT(removeWord()));
    connect(ui.removeAll, SIGNAL(clicked()), this, SLOT(removeAll()));

    connect(ui.pbDictionaryDirectory, SIGNAL(clicked()), this, SLOT(openDictionaryDirectory()));
}
