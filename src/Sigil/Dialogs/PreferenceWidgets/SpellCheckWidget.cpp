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
#include "Misc/Language.h"
#include "Misc/SettingsStore.h"
#include "Misc/SpellCheck.h"
#include "Misc/Utility.h"

SpellCheckWidget::SpellCheckWidget()
    :
    m_isDirty(false)
{
    ui.setupUi(this);

    connectSignalsToSlots();
    readSettings();
}

PreferencesWidget::ResultAction SpellCheckWidget::saveSettings()
{
    if (!m_isDirty)
        return PreferencesWidget::ResultAction_None;

    saveUserDictionaryWordList(ui.userDictList->currentItem());

    SettingsStore settings;
    settings.setDictionary(ui.dictionaries->itemData(ui.dictionaries->currentIndex()).toString());

    SpellCheck *sc = SpellCheck::instance();
    sc->setDictionary(settings.dictionary(), true);

    return PreferencesWidget::ResultAction_RefreshSpelling;
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
    item->setText(name);
    item->setSelected(true);
    ui.userDictList->addItem(item);
    ui.userDictList->setCurrentItem(item);
    m_isDirty = true;
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

    if (new_name == orig_name || new_name.isEmpty()) {
        return;
    }

    QStringList currentDicts;
    for (int i = 0; i < ui.userDictList->count(); ++i) {
        QListWidgetItem *item = ui.userDictList->item(i);
        currentDicts << item->text();
    }

    if (currentDicts.contains(new_name)) {
        QMessageBox::critical(this, tr("Error"), tr("A user dictionary already exists with this name!"));
        return;
    }

    QString orig_path = SpellCheck::userDictionaryDirectory() + "/" + orig_name;
    QString new_path = SpellCheck::userDictionaryDirectory() + "/" + new_name;
    if (!Utility::RenameFile(orig_path, new_path)) {
        QMessageBox::critical(this, tr("Error"), tr("Could not rename file!"));
        return;
    }
    item->setText(new_name);
    m_isDirty = true;
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

    if (dict_item) {
        // Delete the dictionary and remove it from the list.
        QString dict_name = dict_item->text();
        delete dict_item;
        dict_item = 0;
        Utility::DeleteFile(SpellCheck::userDictionaryDirectory() + "/" + dict_name);
    }

    // We have to have at least one user dict.
    if (ui.userDictList->count() < 1) {
        QFile defaultFile(SpellCheck::userDictionaryDirectory() + "/default");
        if (defaultFile.open(QIODevice::WriteOnly)) {
            defaultFile.close();

            QListWidgetItem *item = new QListWidgetItem(ui.userDictList);
            item->setText("default");
            item->setSelected(true);
            ui.userDictList->addItem(item);
            ui.userDictList->setCurrentItem(item);
            loadUserDictionaryWordList(item);
        }
    }
    m_isDirty = true;
}

void SpellCheckWidget::addWord()
{
    QListWidgetItem *item = new QListWidgetItem(ui.userWordList);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    ui.userWordList->addItem(item);
    ui.userWordList->scrollToBottom();
    ui.userWordList->editItem(item);
    m_isDirty = true;
}

void SpellCheckWidget::editWord()
{
    QList<QListWidgetItem *> items = ui.userWordList->selectedItems();
    if (!items.empty()) {
        ui.userWordList->editItem(items.at(0));
    }
    m_isDirty = true;
}

void SpellCheckWidget::removeWord()
{
    foreach (QListWidgetItem *item, ui.userWordList->selectedItems()) {
        ui.userWordList->removeItemWidget(item);
        delete item;
        item = 0;
    }
    m_isDirty = true;
}

void SpellCheckWidget::removeAll()
{
    ui.userWordList->clear();
    m_isDirty = true;
}

void SpellCheckWidget::userWordChanged(QListWidgetItem *item)
{
    if (item) {
        item->setText(item->text().replace(QChar(0x2019), QChar('\'')));
    }
}

void SpellCheckWidget::readSettings()
{
    // Load the available dictionary names.
    Language *lang = Language::instance();
    SpellCheck *sc = SpellCheck::instance();
    QStringList dicts = sc->dictionaries();
    ui.dictionaries->clear();
    foreach (QString dict, dicts) {
        QString name = lang->GetLanguageName(dict);
        if (name.isEmpty()) {
            name = dict;
        }
        ui.dictionaries->addItem(name, dict);
    }

    // Select the current dictionary.
    QString currentDict = sc->currentDictionary();

    SettingsStore settings;

    if (!currentDict.isEmpty()) {
        int index = ui.dictionaries->findData(currentDict);
        if (index > -1) {
            ui.dictionaries->setCurrentIndex(index);
        }
    }

    // Load the list of user dictionaries.
    QDir userDictDir(SpellCheck::userDictionaryDirectory());
    QStringList userDicts = userDictDir.entryList(QDir::Files|QDir::NoDotAndDotDot);
    // Get the dict that should be in use.
    QString confUserDict = SettingsStore().userDictionaryName();

    // Ensure at least one user dictionary is available.
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
    m_isDirty = false;
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
        userDictStream.setCodec("UTF-8");
        QString line;
        do {
            line = userDictStream.readLine();
            if (!line.isEmpty()) {
                words << line;
            }
        } while (!line.isNull());
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
    QSet<QString> unique_words;
    for (int i = 0; i < ui.userWordList->count(); ++i) {
        QString word = ui.userWordList->item(i)->text();
        if (!word.isEmpty()) {
            unique_words << word;
        }
    }
    QStringList words = unique_words.toList();
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
    m_isDirty = true;
}

void SpellCheckWidget::dictionariesCurrentIndexChanged(int index)
{
    m_isDirty = true;
}

void SpellCheckWidget::connectSignalsToSlots()
{
    // User dict list.
    connect(ui.addUserDict, SIGNAL(clicked()), this, SLOT(addUserDict()));
    connect(ui.renameUserDict, SIGNAL(clicked()), this, SLOT(renameUserDict()));
    connect(ui.removeUserDict, SIGNAL(clicked()), this, SLOT(removeUserDict()));
    connect(ui.userDictList, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this, SLOT(userDictionaryChanged(QListWidgetItem *, QListWidgetItem *)));
    connect(ui.userWordList, SIGNAL(itemChanged(QListWidgetItem *)), this, SLOT(userWordChanged(QListWidgetItem *)));

    // Word list.
    connect(ui.addWord, SIGNAL(clicked()), this, SLOT(addWord()));
    connect(ui.editWord, SIGNAL(clicked()), this, SLOT(editWord()));
    connect(ui.removeWord, SIGNAL(clicked()), this, SLOT(removeWord()));
    connect(ui.removeAll, SIGNAL(clicked()), this, SLOT(removeAll()));

    connect(ui.dictionaries, SIGNAL(currentIndexChanged(int)), this, SLOT(dictionariesCurrentIndexChanged(int)));
}
