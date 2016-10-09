/************************************************************************
**
**  Copyright (C) 2013 Dave Heiland
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
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QMessageBox>

#include "SpellCheckWidget.h"
#include "Misc/Language.h"
#include "Misc/SettingsStore.h"
#include "Misc/SpellCheck.h"
#include "Misc/Utility.h"

const QString DEFAULT_DICTIONARY_NAME = "default";

SpellCheckWidget::SpellCheckWidget()
    :
    m_isDirty(false)
{
    ui.setupUi(this);
    setUpTable();
    readSettings();
    connectSignalsToSlots();
}

void SpellCheckWidget::setUpTable()
{
    QStringList header;
    header.append(tr("Enable"));
    header.append(tr("Dictionary"));
    m_Model.setHorizontalHeaderLabels(header);
    ui.userDictList->setModel(&m_Model);
    // Make the header fill all the available space
    ui.userDictList->horizontalHeader()->setStretchLastSection(true);
    ui.userDictList->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    ui.userDictList->resizeColumnToContents(0);
    ui.userDictList->resizeColumnToContents(1);
    ui.userDictList->verticalHeader()->setVisible(false);
    ui.userDictList->setSortingEnabled(false);
    ui.userDictList->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.userDictList->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.userDictList->setAlternatingRowColors(true);
}

PreferencesWidget::ResultAction SpellCheckWidget::saveSettings()
{
    if (!m_isDirty) {
        return PreferencesWidget::ResultAction_None;
    }

    // Save the current dictionary's word list
    if (ui.userDictList->selectionModel()->hasSelection()) {
        int row = ui.userDictList->selectionModel()->selectedIndexes().first().row();
        QStandardItem *item = m_Model.item(row, 1);
        QString name = item->text();
        saveUserDictionaryWordList(name);
    }

    // Save dictionary information
    SettingsStore settings;
    settings.setEnabledUserDictionaries(EnabledDictionaries());
    settings.setDefaultUserDictionary(ui.defaultUserDictionary->text());
    //settings.setDictionary(ui.dictionaries->itemData(ui.dictionaries->currentIndex()).toString());
    settings.setSpellCheck(ui.HighlightMisspelled->checkState() == Qt::Checked);
    settings.setLoadLastSessionDictionaries(ui.cB_loadDicFromLastSession->checkState() == Qt::Checked);
    settings.setUnloadCurrentDIctionaries(ui.cB_UnloadCurrDIcs->checkState() == Qt::Checked);
    settings.setLoadMainLanguageDictionary(ui.cB_loadBookMainLangDic->checkState() == Qt::Checked);
    settings.setLoadAllLanguagesDIctionaries(ui.cB_loadAllMetaLangDic->checkState() == Qt::Checked);

    SpellCheck *sc = SpellCheck::instance();
    sc->setDictionary(settings.dictionary(), true);

    return PreferencesWidget::ResultAction_RefreshSpelling;
}

QStringList SpellCheckWidget::EnabledDictionaries()
{
    QStringList enabled_dicts;
    for (int row = 0; row < m_Model.rowCount(); ++row) {
        QStandardItem *item = m_Model.itemFromIndex(m_Model.index(row, 0));
        if (item->checkState() == Qt::Checked) {
            QStandardItem *name_item = m_Model.itemFromIndex(m_Model.index(row, 1));
            enabled_dicts.append(name_item->text());
        }
    }
    return enabled_dicts;
}

void SpellCheckWidget::addUserDict()
{
    QString name = QInputDialog::getText(this, tr("Add Dictionary"), tr("Name:"));

    if (name.isEmpty()) {
        return;
    }

    QStringList currentDicts;

    for (int row = 0; row < m_Model.rowCount(); ++row) {
        QStandardItem *item = m_Model.itemFromIndex(m_Model.index(row, 1));
        currentDicts << item->text();
    }

    if (currentDicts.contains(name, Qt::CaseInsensitive)) {
        QMessageBox::critical(this, tr("Error"), tr("A user dictionary already exists with this name!"));
        return;
    }

    createUserDict(name);
}

void SpellCheckWidget::addUserWords()
{
    QString list = QInputDialog::getText(this, tr("Add Words"), tr("Words:"));

    if (list.isEmpty()) {
        return;
    }

    list.replace(" ", ",");
    list.replace(",", "\n");
    QStringList words = list.split("\n");

    // Add the words to the dictionary
    foreach(QString word, words) {
        if (!word.isEmpty()) {
            QListWidgetItem *item = new QListWidgetItem(word, ui.userWordList);
            item->setFlags(item->flags() | Qt::ItemIsEditable);
            ui.userWordList->addItem(item);
        }
    }
    ui.userWordList->sortItems(Qt::AscendingOrder);

    m_isDirty = true;
}

bool SpellCheckWidget::createUserDict(QString dict_name)
{
    QString path = SpellCheck::userDictionaryDirectory() + "/" + dict_name;
    QFile dict_file(path);

    if (dict_file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        dict_file.close();
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Could not create file!"));
        return false;
    }

    addNewItem(true, dict_name);
    ui.userDictList->sortByColumn(1, Qt::AscendingOrder);

    return true;
}

void SpellCheckWidget::addNewItem(bool enabled, QString dict_name)
{
    QList<QStandardItem *> rowItems;
    // Checkbox
    QStandardItem *checkbox_item = new QStandardItem();
    checkbox_item->setCheckable(true);
    checkbox_item->setCheckState(Qt::Checked);
    if (enabled) {
        checkbox_item->setCheckState(Qt::Checked);
    } else {
        checkbox_item->setCheckState(Qt::Unchecked);
    }
    rowItems << checkbox_item;
    // Filename
    QStandardItem *file_item = new QStandardItem();
    file_item->setText(dict_name);
    file_item->setToolTip(dict_name);
    rowItems << file_item;

    for (int i = 0; i < rowItems.count(); i++) {
        rowItems[i]->setEditable(false);
    }
    m_Model.appendRow(rowItems);

    ui.userDictList->setCurrentIndex(file_item->index());

    m_isDirty = true;
}

void SpellCheckWidget::renameUserDict()
{
    if (!ui.userDictList->selectionModel()->hasSelection()) {
        return;
    }

    int row = ui.userDictList->selectionModel()->selectedIndexes().first().row();
    QStandardItem *item = m_Model.item(row, 1);
    QString orig_name = item->text();

    QString new_name = QInputDialog::getText(this, tr("Rename"), tr("Name:"), QLineEdit::Normal, orig_name);

    if (new_name == orig_name || new_name.isEmpty()) {
        return;
    }

    QStringList currentDicts;
    for (int row = 0; row < m_Model.rowCount(); ++row) {
        QStandardItem *item= m_Model.itemFromIndex(m_Model.index(row, 1));
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
    setDefaultUserDictionary(new_name);

    ui.userDictList->sortByColumn(1, Qt::AscendingOrder);
    m_isDirty = true;
}

void SpellCheckWidget::removeUserDict()
{
    if (!ui.userDictList->selectionModel()->hasSelection()) {
        return;
    }

    // Don't remove the last dictionary
    if (m_Model.rowCount() == 1) {
        QMessageBox::warning(this, tr("Error"), tr("You cannot delete the last dictionary."));
        return;
    }

    int row = ui.userDictList->selectionModel()->selectedIndexes().first().row();
    QStandardItem *item = m_Model.item(row, 1);

    if (item) {
        // Delete the dictionary and remove it from the list.
        QString dict_name = item->text();
        m_Model.removeRow(row);
        Utility::SDeleteFile(SpellCheck::userDictionaryDirectory() + "/" + dict_name);
    }

    m_isDirty = true;
}

void SpellCheckWidget::copyUserDict()
{
    // Get the current dictionary.
    if (!ui.userDictList->selectionModel()->hasSelection()) {
        return;
    }

    int row = ui.userDictList->selectionModel()->selectedIndexes().first().row();
    QStandardItem *item = m_Model.item(row, 1);

    if (!item) {
        return;
    }

    // Get the current words, before creating so list doesn't change
    QStringList words;
    for (int i = 0; i < ui.userWordList->count(); ++i) {
        QString word = ui.userWordList->item(i)->text();
        words.append(word);
    }

    // Create a new dictionary
    QStringList current_dicts;
    for (int row = 0; row < m_Model.rowCount(); ++row) {
        QStandardItem *item = m_Model.itemFromIndex(m_Model.index(row, 1));
        current_dicts.append(item->text());
    }
    QString dict_name = item->text();
    while (current_dicts.contains(dict_name)) {
        dict_name += "_copy";
    }

    if (!createUserDict(dict_name)) {
        return;
    }

    // Add the words to the dictionary
    foreach(QString word, words) {
        QListWidgetItem *item = new QListWidgetItem(word, ui.userWordList);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ui.userWordList->addItem(item);
    }

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
    foreach(QListWidgetItem * item, ui.userWordList->selectedItems()) {
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
    //QStringList dicts = sc->dictionaries();
    QStringList dicts = sc->alreadyLoadedDics();
    //ui.dictionaries->clear();
    foreach(QString dict, dicts) {
        QString name = lang->GetLanguageName(dict);

        if (name.isEmpty()) {
            name = dict;
        }

       // ui.dictionaries->addItem(name, dict);
    }
    // Select the current dictionary.
    QString currentDict = sc->currentDictionary();
    SettingsStore settings;

//    if (!currentDict.isEmpty()) {
//        int index = ui.dictionaries->findData(currentDict);

//        if (index > -1) {
//            ui.dictionaries->setCurrentIndex(index);
//        }
//    }

    // Load the list of user dictionaries.
    QDir userDictDir(SpellCheck::userDictionaryDirectory());
    QStringList userDicts = userDictDir.entryList(QDir::Files | QDir::NoDotAndDotDot);
    userDicts.sort();

    // Make sure at least one dictionary exists
    // Should never happen since spellcheck creates one
    if (userDicts.count() < 1) {
        QFile defaultFile(SpellCheck::userDictionaryDirectory() + "/" + DEFAULT_DICTIONARY_NAME);

        if (defaultFile.open(QIODevice::WriteOnly)) {
            defaultFile.close();
        }
        // Add and enable a default dictionary
        addNewItem(true, DEFAULT_DICTIONARY_NAME);
    }

    // Load the list of dictionary files into the UI, marking if enabled
    QStringList enabled_dicts = settings.enabledUserDictionaries();
    foreach(QString dict_name, userDicts) {
        addNewItem(enabled_dicts.contains(dict_name), dict_name);
    }

    // Get the default dictionary - it should always exist
    setDefaultUserDictionary(SettingsStore().defaultUserDictionary());

    loadUserDictionaryWordList();

    // Set whether mispelled words are highlighted or not
    ui.HighlightMisspelled->setChecked(settings.spellCheck());
    ui.cB_loadDicFromLastSession->setChecked(settings.setLoadLastSessionDictionaries());
    ui.cB_UnloadCurrDIcs->setChecked(settings.setUnloadCurrentDIctionaries());
    ui.cB_loadBookMainLangDic->setChecked(settings.setLoadMainLanguageDictionary());
    ui.cB_loadAllMetaLangDic->setChecked(settings.setLoadAllLanguagesDIctionaries());

    m_isDirty = false;
}

void SpellCheckWidget::loadUserDictionaryWordList(QString dict_name)
{
    ui.userWordList->clear();

    if (dict_name.isEmpty()) {
        if (ui.userDictList->selectionModel()->hasSelection()) {
            int row = ui.userDictList->selectionModel()->selectedIndexes().first().row();
            QStandardItem *item = m_Model.item(row, 1);
            dict_name = item->text();
        }
    }

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
    foreach(QString word, words) {
        QListWidgetItem *item = new QListWidgetItem(word, ui.userWordList);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ui.userWordList->addItem(item);
    }
}

void SpellCheckWidget::saveUserDictionaryWordList(QString dict_name)
{
    SettingsStore ss;
    QString dict_path;

    if (dict_name.isEmpty()) {
        return;
    }

    dict_path = SpellCheck::userDictionaryDirectory() + "/" + dict_name;
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
        foreach(QString word, words) {
            userDictStream << word << "\n";
        }
        userDictFile.close();
    }
}

void SpellCheckWidget::SelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    if (deselected.indexes().count() > 1) {
        QStandardItem *previous_item = m_Model.item(m_Model.itemFromIndex(deselected.indexes().first())->row(), 1);
        if (previous_item) {
            saveUserDictionaryWordList(previous_item->text());
        }
    }

    if (selected.indexes().count() > 1) {
        QStandardItem *current_item = m_Model.item(m_Model.itemFromIndex(selected.indexes().first())->row(), 1);
        if (current_item) {
            loadUserDictionaryWordList(current_item->text());
        }
    }

    setDefaultUserDictionary();

    m_isDirty = true;
}

void SpellCheckWidget::setDefaultUserDictionary(QString dict_name)
{
    if (m_Model.rowCount() < 1) {
        return;
    }

    // Highlight the dictionary if a specific name was given
    if (!dict_name.isEmpty()) {
        for (int row = 0; row < m_Model.rowCount(); ++row) {
            QStandardItem *item = m_Model.itemFromIndex(m_Model.index(row, 1));
            if (dict_name == item->text()) {
                ui.userDictList->setCurrentIndex(item->index());
            }
        }
    }

    // Update the dictionary label to match the highlighted entry
    // Default to the first entry if name is not matched
    QStandardItem *item = m_Model.item(0, 1);
    if (ui.userDictList->selectionModel()->hasSelection()) {
        int row = ui.userDictList->selectionModel()->selectedIndexes().first().row();
        item = m_Model.item(row, 1);
    }
    ui.defaultUserDictionary->setText(item->text());
    SettingsStore settings;
    settings.setDefaultUserDictionary(ui.defaultUserDictionary->text());
}

void SpellCheckWidget::dictionariesCurrentIndexChanged(int index)
{
    m_isDirty = true;
}

void SpellCheckWidget::checkBoxChanged(int state)
{
    m_isDirty = true;
}

void SpellCheckWidget::ItemChanged(QStandardItem *item)
{
    m_isDirty = true;
}


void SpellCheckWidget::connectSignalsToSlots()
{
    // User dict list.
    connect(ui.addUserDict, SIGNAL(clicked()), this, SLOT(addUserDict()));
    connect(ui.addUserWords, SIGNAL(clicked()), this, SLOT(addUserWords()));
    connect(ui.renameUserDict, SIGNAL(clicked()), this, SLOT(renameUserDict()));
    connect(ui.copyUserDict, SIGNAL(clicked()), this, SLOT(copyUserDict()));
    connect(ui.removeUserDict, SIGNAL(clicked()), this, SLOT(removeUserDict()));
    connect(ui.userWordList, SIGNAL(itemChanged(QListWidgetItem *)), this, SLOT(userWordChanged(QListWidgetItem *)));
    // Word list.
    connect(ui.editWord, SIGNAL(clicked()), this, SLOT(editWord()));
    connect(ui.removeWord, SIGNAL(clicked()), this, SLOT(removeWord()));
    connect(ui.removeAll, SIGNAL(clicked()), this, SLOT(removeAll()));
    //connect(ui.dictionaries, SIGNAL(currentIndexChanged(int)), this, SLOT(dictionariesCurrentIndexChanged(int)));
    connect(ui.HighlightMisspelled, SIGNAL(stateChanged(int)), this, SLOT(checkBoxChanged(int)));
    connect(ui.cB_loadDicFromLastSession, SIGNAL(stateChanged(int)),this, SLOT(checkBoxChanged(int)));
    connect(ui.cB_UnloadCurrDIcs, SIGNAL(stateChanged(int)),this, SLOT(checkBoxChanged(int)));
    connect(ui.cB_loadBookMainLangDic, SIGNAL(stateChanged(int)),this, SLOT(checkBoxChanged(int)));
    connect(ui.cB_loadAllMetaLangDic, SIGNAL(stateChanged(int)),this, SLOT(checkBoxChanged(int)));
    QItemSelectionModel *selectionModel = ui.userDictList->selectionModel();
    connect(selectionModel,     SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
            this,               SLOT(SelectionChanged(const QItemSelection &, const QItemSelection &)));
    connect(&m_Model, SIGNAL(itemChanged(QStandardItem *)),
            this,               SLOT(ItemChanged(QStandardItem *))
           );
}
