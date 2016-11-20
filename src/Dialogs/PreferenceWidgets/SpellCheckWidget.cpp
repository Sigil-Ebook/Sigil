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
#include <QtWidgets/QTreeView>
#include <QCryptographicHash>
#include <QDebug>

#include "SpellCheckWidget.h"
#include "Misc/Language.h"
#include "Misc/SettingsStore.h"
#include "Misc/SpellCheck.h"
#include "Misc/Utility.h"

const QString DEFAULT_DICTIONARY_NAME = "default";

SpellCheckWidget::SpellCheckWidget()
    :
    m_isDirty {false},
    m_hash {},
    m_dirtyDicts {},
    m_Model {new QStandardItemModel(this)}
{
    ui.setupUi(this);
    setUpTable();
    readSettings();
    connectSignalsToSlots();
}

void SpellCheckWidget::setUpTable()
{
    QStringList header;
    header.append(tr("Dictionary"));
    header.append(tr("Enable"));

    m_Model->setHorizontalHeaderLabels(header);
    ui.userDictList->setModel(m_Model);
    ui.userDictList->header()->setSectionsMovable(false);
    ui.userDictList->resizeColumnToContents(0);
    ui.userDictList->resizeColumnToContents(1);
    ui.userDictList->setUniformRowHeights(true);
    ui.userDictList->setSortingEnabled(false);
    ui.userDictList->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.userDictList->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.userDictList->setAlternatingRowColors(true);
    ui.userDictList->setIndentation(14);
}

PreferencesWidget::ResultAction SpellCheckWidget::saveSettings()
{
    if (!m_isDirty) {
        return PreferencesWidget::ResultAction_None;
    }

    // Save the current dictionary's word list
    if (ui.userDictList->selectionModel()->hasSelection()) {
        QString name = getSelectedDictName(ui.userDictList->selectionModel()->selectedIndexes().first());
        if(!name.isEmpty()){
            saveUserDictionaryWordList(name);
        }
    }

    // Save dictionary information
    SettingsStore settings;
    settings.setValue("SCWgeometry", saveGeometry());
    settings.setValue("SCWtab", ui.tabWidget->currentIndex());

    QStringList previousyEnabledUserDicts {settings.enabledUserDictionaries()};
    QStringList currentlyEnabledUserDicts {EnabledDictionaries()};

    settings.setEnabledUserDictionaries(currentlyEnabledUserDicts);

    //determine dirty dictionaries
    //all dirtied in SpellCheckWidget dialog are in m_dirtyDicts
    //but we have to account for disabled and enabled
    previousyEnabledUserDicts.sort();
    SpellCheck *sc {SpellCheck::instance()};
    if(previousyEnabledUserDicts!=currentlyEnabledUserDicts){
        //find disabled and enabled in this session
        for(auto dict:currentlyEnabledUserDicts){
            if(previousyEnabledUserDicts.contains(dict)){
                previousyEnabledUserDicts.removeOne(dict);
                currentlyEnabledUserDicts.removeOne(dict);
            }
        }
        QStringList dirtyDicts {previousyEnabledUserDicts+currentlyEnabledUserDicts};
        bool haveMUL {false};
        for(auto dict:dirtyDicts){
            if(haveMUL) break;
            QStringList list {sc->userDictLaunguages(dict)}; //TODO get rid of doppel call
            if(list.contains("mul")){
                m_dirtyDicts<<"mul";
                haveMUL=true;
            }else{
                for(auto item:list){
                    //in case of deselected we have to force the issue
                    dictionaryDirty(dict+QChar('.')+item, true);
                }
            }
        }

    }

    //reload dirty dictionaries

    if(m_dirtyDicts.contains("mul")){
        sc->reloadAllDictionaries();
    }else{
        for(auto dict:m_dirtyDicts){
            sc->unloadDictionary(dict);
            sc->loadDictionary(dict);
        }
    }

    settings.setDefaultUserDictionary(ui.defaultUserDictionary->text());

    settings.setSpellCheck(ui.HighlightMisspelled->checkState() == Qt::Checked);
    settings.setLoadLastSessionDictionaries(ui.cB_loadDicFromLastSession->checkState() == Qt::Checked);
    settings.setUnloadCurrentDIctionaries(ui.cB_UnloadCurrDIcs->checkState() == Qt::Checked);
    settings.setLoadMainLanguageDictionary(ui.cB_loadBookMainLangDic->checkState() == Qt::Checked);
    settings.setLoadAllLanguagesDictionaries(ui.cB_loadAllMetaLangDic->checkState() == Qt::Checked);

    return PreferencesWidget::ResultAction_RefreshSpelling;
}

QStringList SpellCheckWidget::EnabledDictionaries()
{
    QStringList enabled_dicts;
    for (int row = 0; row < m_Model->rowCount(); ++row) {
        QStandardItem *item = m_Model->itemFromIndex(m_Model->index(row, 1));
        if (item->checkState() == Qt::Checked) {
            QStandardItem *name_item = m_Model->itemFromIndex(m_Model->index(row, 0));
            enabled_dicts.append(name_item->text());
        }
    }
    enabled_dicts.sort();
    return enabled_dicts;
}

void SpellCheckWidget::addUserDict()
{
    QString name = QInputDialog::getText(this, tr("Add Dictionary"), tr("Name:"));

    if (name.isEmpty()) {
        return;
    }

    QStringList currentDicts;

    for (int row = 0; row < m_Model->rowCount(); ++row) {
        QStandardItem *item = m_Model->itemFromIndex(m_Model->index(row, 0));
        currentDicts << item->text();
    }

    if (currentDicts.contains(name, Qt::CaseInsensitive)) {
        QMessageBox::critical(this, tr("Error"), tr("A user dictionary already exists with this name!"));
        return;
    }
    if (name.contains(QChar('.'))) {
        QMessageBox::critical(this, tr("Error"), tr("Dots are not allowed in dictionary name!"));
        return;
    }
    createUserDict(QStringList()<<name);
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

bool SpellCheckWidget::createUserDict(QStringList dict_name)
{

    SpellCheck::instance()->createUserDictionary(dict_name.first());
    addNewMLItem(true, dict_name);
    ui.userDictList->sortByColumn(0, Qt::AscendingOrder);

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
    m_Model->appendRow(rowItems);

    ui.userDictList->setCurrentIndex(file_item->index());
    ui.userDictList->setExpanded(file_item->index(),true);

    m_isDirty = true;
}

void SpellCheckWidget::addNewMLItem(bool enabled, QStringList dicts)
{

    QList<QStandardItem *> rowItems;
    QStandardItem *root{m_Model->invisibleRootItem()};

    // Checkbox
    QStandardItem *checkbox_item = new QStandardItem();
    checkbox_item->setCheckable(true);
    checkbox_item->setCheckState(Qt::Checked);
    if (enabled) {
        checkbox_item->setCheckState(Qt::Checked);
    } else {
        checkbox_item->setCheckState(Qt::Unchecked);
    }

    QStandardItem *file_item = new QStandardItem(dicts.first());
    file_item->setToolTip(dicts.first());
    rowItems << file_item<<checkbox_item;

    root->appendRow(rowItems);

    for(int i=1; i<dicts.count();i++){
        QStandardItem *sub_item{new QStandardItem(dicts[i])};        
        sub_item->setToolTip(Language::instance()->GetLanguageName(sub_item->text().replace('_','-')));
        file_item->appendRow(sub_item);
    }

    for (int i = 0; i < rowItems.count(); i++) {
        rowItems[i]->setEditable(false);
    }

    ui.userDictList->setCurrentIndex(file_item->index());
    ui.userDictList->resizeColumnToContents(0);
    m_isDirty = true;
}

void SpellCheckWidget::renameUserDict()
{
    if (!ui.userDictList->selectionModel()->hasSelection()) {
        return;
    }

    QModelIndex idx{ui.userDictList->selectionModel()->selectedIndexes().first()};

    QString parent{getSelectedParentDic(idx)};
    QString selected{getSelectedDictName(idx)};
    if(!(parent==selected)){
        QMessageBox::critical(this, tr("Error"), tr("You cannot rename subdictionary!"));
        return;
    }

    QStringList subDicts{getChildrenDics(idx)};

    int row = idx.row();
    QStandardItem *item = m_Model->item(row, 0);
    QString orig_name = parent;

    QString new_name = QInputDialog::getText(this, tr("Rename"), tr("Name:"), QLineEdit::Normal, orig_name);

    if (new_name == orig_name || new_name.isEmpty()) {
        return;
    }
    if (new_name.contains(QChar('.'))) {
        QMessageBox::critical(this, tr("Error"), tr("Dots are not allowed in dictionary name!"));
        return;
    }

    QStringList currentDicts;
    for (int row = 0; row < m_Model->rowCount(); ++row) {
        QStandardItem *item= m_Model->itemFromIndex(m_Model->index(row, 0));
        currentDicts << item->text();
    }

    if (currentDicts.contains(new_name)) {
        QMessageBox::critical(this, tr("Error"), tr("A user dictionary already exists with this name!"));
        return;
    }

    QString path{SpellCheck::userDictionaryDirectory()+"/"};
    QString orig_path {path + orig_name};
    QString new_path {path + new_name};

    if (!Utility::RenameFile(orig_path, new_path)) {
        QMessageBox::critical(this, tr("Error"), tr("Could not rename file!"));
        return;
    }

    if(!subDicts.isEmpty()){
        for(auto dic:subDicts){
            orig_path=path+orig_name+QChar('.')+dic;
            new_path=path+new_name+QChar('.')+dic;
            if (!Utility::RenameFile(orig_path, new_path)) {
                QMessageBox::critical(this, tr("Error"), tr("Could not rename file!"));
                return;
            }
        }
    }


    item->setText(new_name);
    setDefaultUserDictionary(new_name);

    ui.userDictList->sortByColumn(0, Qt::AscendingOrder);
    m_isDirty = true;
}

void SpellCheckWidget::removeUserDict()
{
    if (!ui.userDictList->selectionModel()->hasSelection()) {
        return;
    }

    // Don't remove the last dictionary
    if (m_Model->rowCount() == 1) {
        QMessageBox::warning(this, tr("Error"), tr("You cannot delete the last dictionary."));
        return;
    }
    QModelIndex idx {ui.userDictList->selectionModel()->selectedIndexes().first()};
    QStandardItem * parent {getParent(idx)};
    QStandardItem * selected {getSelected(idx)};

    int row {idx.row()};

    QString selectedDic {getSelectedDictName(idx)};

    if (idx.isValid()) {
        QString path {SpellCheck::userDictionaryDirectory() + "/"};       
        // Delete the dictionary and remove it from the list.
        //Children first
        if(selected->hasChildren()){
            QStringList subDicts {getChildrenDics(idx)};
            for(auto dic:subDicts){
                QString current {selectedDic+QChar('.')+dic};
                //before removing ev. mark dictionary as dirty
                dictionaryDirty(current);
                current= path+current;
                if(!Utility::SDeleteFile(current)){
                    Utility::DisplayStdErrorDialog( tr("Delete operation failed"), current);
                    m_dirtyDicts.removeOne(dic);
                    return;
                }
            }
        }
        //now the real item
        // dirty?
        dictionaryDirty(selectedDic);
        if(!Utility::SDeleteFile(path + selectedDic)){
            Utility::DisplayStdErrorDialog( tr("Delete operation failed"), path+selectedDic);
            return;
        }
        //prevent creating dictionary again by SelectionChanged()
        //apparently Qt remembers removed item as "deselected"
        selected->setText("");
        //update model
        //perhaps it's a lonely child: it has to have parent
        if(parent){
            parent->removeRow(row);
        }
        //parent has no parent
        if(!parent){
            m_Model->removeRow(row);
        }
        m_isDirty = true;
    }
}

void SpellCheckWidget::copyUserDict()
{
    // Get the current dictionary.
    if (!ui.userDictList->selectionModel()->hasSelection()) {
        return;
    }

    QModelIndex idx {ui.userDictList->selectionModel()->selectedIndexes().first()};
    QStandardItem * parent {getParent(idx)};
    QStandardItem * selected {getSelected(idx)};

    int row {idx.row()};

    QStandardItem *item = m_Model->item(row, 0);

    if (!item) {
        return;
    }

    if(parent){
        QMessageBox::critical(this, tr("Error"),
                              tr("You cannot copy subdictionary!"));
        return;
    }

    //save current word list
    saveUserDictionaryWordList(selected->text());

    // Create a new dictionary
    QStringList current_dicts;
    for (int row = 0; row < m_Model->rowCount(); ++row) {
        QStandardItem *item = m_Model->itemFromIndex(m_Model->index(row, 0));
        current_dicts.append(item->text());
    }
    QString dict_name = selected->text();
    while (current_dicts.contains(dict_name)) {
        dict_name += "_copy";
    }
    QStringList dicList {{dict_name}};
    if(selected->hasChildren()){
        QStringList subdicts {getChildrenDics(idx)};
        for(auto suffix : subdicts){
            dicList<<suffix;
        }
    }

    if(!copyDicWithSubdics(selected->text(),dicList)){
        return;
    }

    addNewMLItem(true, dicList);
    m_isDirty = true;
}

bool SpellCheckWidget::copyDicWithSubdics(const QString &source, const QStringList &destination){

    QString sourcePath {SpellCheck::instance()->userDictionaryDirectory() + "/"+source};
    QString destPath {SpellCheck::instance()->userDictionaryDirectory() + "/" +destination.first()};

    if(!Utility::ForceCopyFile(sourcePath,destPath)){
        Utility::DisplayStdErrorDialog(tr("Copy operation failed!"), source +" -> "+ destPath);
        return false;
    }

    for(int i=1; i<destination.count();i++){
        QString orig {sourcePath+QChar('.')+destination.at(i)};
        QString copy {destPath+QChar('.')+destination.at(i)};
        if(!Utility::ForceCopyFile(orig,copy)){
            if(!Utility::ForceCopyFile(sourcePath,destPath)){
                Utility::DisplayStdErrorDialog(tr("Copy operation failed!"), orig +" -> "+copy);
                return false;
            }
        }
    }
    return true;
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

    SettingsStore settings;
    QByteArray geometry = settings.value("SCWgeometry").toByteArray();

    if (!geometry.isNull()) {
        restoreGeometry(geometry);
    }
    ui.tabWidget->setCurrentWidget(ui.tabWidget->widget(settings.value("SCWtab").toInt()));

    // Load the list of user dictionaries.
    QDir userDictDir(SpellCheck::userDictionaryDirectory());
    QStringList userDicts = userDictDir.entryList(QDir::Files | QDir::NoDotAndDotDot);
    userDicts.sort();
    QMap<QString,QStringList> mainDics{};
    for(int i=0;i<userDicts.count();i++){
        QString d{userDicts.at(i)};
        int n{d.lastIndexOf(QChar('.'))};
        QString name{d.mid(0,n)};
        QString suffix{d.mid(n+1)};
        if(mainDics.contains(name)&!suffix.isEmpty()){
            mainDics[name]<<suffix;
        }
        else{
            mainDics.insert(name,{name});
        }
    }
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

    for(auto list:mainDics){
        addNewMLItem(enabled_dicts.contains(list.first()),list);
    }
    // Get the default dictionary - it should always exist
    setDefaultUserDictionary(SettingsStore().defaultUserDictionary());

    loadUserDictionaryWordList();

    // Set whether mispelled words are highlighted or not
    ui.HighlightMisspelled->setChecked(settings.spellCheck());
    ui.cB_loadDicFromLastSession->setChecked(settings.setLoadLastSessionDictionaries());
    ui.cB_UnloadCurrDIcs->setChecked(settings.setUnloadCurrentDIctionaries());
    ui.cB_loadBookMainLangDic->setChecked(settings.setLoadMainLanguageDictionary());
    ui.cB_loadAllMetaLangDic->setChecked(settings.setLoadAllLanguagesDictionaries());

    m_isDirty = false;
}

void SpellCheckWidget::loadUserDictionaryWordList(QString dict_name)
{
    ui.userWordList->clear();

    if (dict_name.isEmpty()) {
        if (ui.userDictList->selectionModel()->hasSelection()) {
            int row = ui.userDictList->selectionModel()->selectedIndexes().first().row();
            QStandardItem *item = m_Model->item(row, 0);
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
    QByteArray forHash {};
    foreach(QString word, words) {
        forHash.append(word);
        QListWidgetItem *item = new QListWidgetItem(word, ui.userWordList);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ui.userWordList->addItem(item);
    }
    m_hash =QCryptographicHash::hash(forHash,QCryptographicHash::Md5);
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
    QByteArray forHash {};
    if (userDictFile.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream userDictStream(&userDictFile);
        userDictStream.setCodec("UTF-8");
        foreach(QString word, words) {
            forHash.append(word);
            userDictStream << word << "\n";
        }
        userDictFile.close();
    }
    if(m_hash!=QCryptographicHash::hash(forHash,QCryptographicHash::Md5)){
        dictionaryDirty(dict_name, true);
    }
}

void SpellCheckWidget::SelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{

    if (deselected.indexes().count() > 0) {
        QString name{getSelectedDictName(deselected.indexes().first())};
        //qDebug()<<"deselected: "<<name;
        if (!name.isEmpty()) {
            saveUserDictionaryWordList(name);
        }
    }

    if (selected.indexes().count() > 0) {
        QString name{getSelectedDictName(selected.indexes().first())};
        //qDebug()<<"selected: "<<name;
        if (!name.isEmpty()) {
            loadUserDictionaryWordList(name);
        }
    }


    setDefaultUserDictionary();

    m_isDirty = true;
}

void SpellCheckWidget::setDefaultUserDictionary(QString dict_name)
{
    if (m_Model->rowCount() < 1) {
        return;
    }

    // Highlight the dictionary if a specific name was given
    if (!dict_name.isEmpty()) {
        for (int row = 0; row < m_Model->rowCount(); ++row) {
            QStandardItem *item = m_Model->itemFromIndex(m_Model->index(row, 0));
            if (dict_name == item->text()) {
                ui.userDictList->setCurrentIndex(item->index());
            }
        }
    }

    // Update the dictionary label to match the highlighted entry
    // Default to the first entry if name is not matched
    QStandardItem *item = m_Model->item(0, 0);
    QString name{item->text()};

    if (ui.userDictList->selectionModel()->hasSelection()) {
        name=getSelectedParentDic(ui.userDictList->selectionModel()->selectedIndexes().first());

    }
    ui.defaultUserDictionary->setText(name);
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
    //qDebug()<<"item changed: "<<m_Model->item(item->row())->text();
    m_isDirty = true;
}

void SpellCheckWidget::tabChanged(int tab)
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
    connect(ui.HighlightMisspelled, SIGNAL(stateChanged(int)), this, SLOT(checkBoxChanged(int)));
    connect(ui.cB_loadDicFromLastSession, SIGNAL(stateChanged(int)),this, SLOT(checkBoxChanged(int)));
    connect(ui.cB_UnloadCurrDIcs, SIGNAL(stateChanged(int)),this, SLOT(checkBoxChanged(int)));
    connect(ui.cB_loadBookMainLangDic, SIGNAL(stateChanged(int)),this, SLOT(checkBoxChanged(int)));
    connect(ui.cB_loadAllMetaLangDic, SIGNAL(stateChanged(int)),this, SLOT(checkBoxChanged(int)));
    connect(ui.tabWidget,SIGNAL(currentChanged(int)),this,SLOT(tabChanged(int)));
    QItemSelectionModel *selectionModel = ui.userDictList->selectionModel();
    connect(selectionModel,     SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
            this,               SLOT(SelectionChanged(const QItemSelection &, const QItemSelection &)));
    connect(m_Model, SIGNAL(itemChanged(QStandardItem *)),
            this,               SLOT(ItemChanged(QStandardItem *))
           );
}

QStandardItem * SpellCheckWidget::getParent(const QModelIndex selected){
    return m_Model->itemFromIndex(selected.parent());
}

QStandardItem * SpellCheckWidget::getSelected(const QModelIndex selected){
    QModelIndex parent{selected.parent()};
    if(parent.isValid()){
        QModelIndex child=parent.child(m_Model->itemFromIndex(selected)->row(), 0);
        return m_Model->itemFromIndex(child);
    }
    return m_Model->itemFromIndex(selected);
}

QString SpellCheckWidget::getSelectedDictName(const QModelIndex selected){
    if(selected.isValid()){
        const QStandardItem *parent{getParent(selected)};
        const QStandardItem *child{getSelected(selected)};
        QString name{child->text()};
        if(parent && !name.isEmpty()){
            name=parent->text()+QChar('.')+name;
        }
        return name;
    }
    return QString();
}

QString SpellCheckWidget::getSelectedParentDic(const QModelIndex selected){
    const QStandardItem *parent{getParent(selected)};
    const QStandardItem *child{getSelected(selected)};
    QString name{child->text()};
    if(parent){
        name=parent->text();
    }
    return name;
}

QStringList SpellCheckWidget::getChildrenDics(const QModelIndex selected){
    QStandardItem *item{m_Model->itemFromIndex(selected)};
    if(item->hasChildren()){
        QStringList list{};
        for(int i=0;i<item->rowCount();i++){
            list<<item->child(i)->text();
        }
        return list;
    }
    return QStringList();
}

void SpellCheckWidget::dictionaryDirty(const QString userDict, const bool force){
    QString dict {(userDict.split(QChar('.')).first())};
    QString code {(userDict.contains(QChar('.')))?userDict.split(QChar('.')).last():"mul"};
    SettingsStore settings;
    if((settings.enabledUserDictionaries().contains(dict) ||
            EnabledDictionaries().contains(dict) ||
            force) &&
            (SpellCheck::instance()->userDictLaunguages(dict).contains(code)
             ) ){
        if(!m_dirtyDicts.contains(code)) {
            m_dirtyDicts<<code;
            qDebug()<<code<<" dirtied";
        }else{
            qDebug()<<code<<" already dirtied";
        }
    }
}
