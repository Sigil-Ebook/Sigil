/************************************************************************
**
**  Copyright (C) 2012, 2013 John Schember <john@nachtimwald.com>
**  Copyright (C) 2012, 2013 Dave Heiland
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

#include <QtCore/QHashIterator>
#include <QtCore/QSignalMapper>
#include <QtGui/QContextMenuEvent>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QListView>

#include "Dialogs/SpellcheckEditor.h"
#include "Misc/CaseInsensitiveItem.h"
#include "Misc/NumericItem.h"
#include "Misc/SettingsStore.h"
#include "Misc/SpellCheck.h"
#include "Misc/Utility.h"
#include "ResourceObjects/Resource.h"
#include "Misc/Language.h"

static const QString SETTINGS_GROUP = "spellcheck_editor";
static const QString SELECTED_DICTIONARY = "selected_dictionary";
static const QString SHOW_ALL_WORDS = "show_all_words";
static const QString CASE_INSENSITIVE_SORT = "case_insensitive_sort";
static const QString SORT_COLUMN = "sort_column";
static const QString SORT_ORDER = "sort_order";
static const QString FILE_EXTENSION = "ini";

SpellcheckEditor::SpellcheckEditor(QWidget *parent)
    :
    QDialog(parent),
    m_Book(NULL),
    m_SpellcheckEditorModel(new QStandardItemModel(this)),
    m_ContextMenu(new QMenu(this)),
    m_MultipleSelection(false),
    m_SelectRow(-1),
    m_FilterSC(new QShortcut(QKeySequence(tr("f", "Filter")), this)),
    m_ShowAllSC(new QShortcut(QKeySequence(tr("s", "ShowAllWords")), this)),
    m_NoCaseSC(new QShortcut(QKeySequence(tr("c", "Case-InsensitiveSort")), this)),
    m_RefreshSC(new QShortcut(QKeySequence(tr("r", "Refresh")), this)),
    m_SpellCheck(SpellCheck::instance())


{
    ui.setupUi(this);
    ui.FilterText->installEventFilter(this);

    SetupSpellcheckEditorTree();
    setupLoadedDicsTable();
    CreateContextMenuActions();
    ConnectSignalsSlots();
    UpdateDictionaries();
    ReadSettings();
}

SpellcheckEditor::~SpellcheckEditor()
{
    WriteSettings();
}

void SpellcheckEditor::SetBook(QSharedPointer <Book> book)
{
    m_Book = book;
    // TODO: new book, let's do somemething usefull
    //hide this
    this->hide();
}

void SpellcheckEditor::SetupSpellcheckEditorTree()
{
    ui.SpellcheckEditorTree->setModel(m_SpellcheckEditorModel);
    ui.SpellcheckEditorTree->setContextMenuPolicy(Qt::CustomContextMenu);
    ui.SpellcheckEditorTree->setSortingEnabled(true);
    ui.SpellcheckEditorTree->setWordWrap(true);
    ui.SpellcheckEditorTree->setAlternatingRowColors(true);
    ui.SpellcheckEditorTree->header()->setStretchLastSection(false);
}

void SpellcheckEditor::showEvent(QShowEvent *event)
{
    ui.FilterText->clear();
    Refresh();
}

bool SpellcheckEditor::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui.FilterText) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            int key = keyEvent->key();

            if (key == Qt::Key_Down) {
                ui.SpellcheckEditorTree->setFocus();
                return true;
            }
        }
    }

    // pass the event on to the parent class
    return QDialog::eventFilter(obj, event);
}

void SpellcheckEditor::toggleShowAllWords()
{
  ui.ShowAllWords->click();
  ui.SpellcheckEditorTree->setFocus();
}

void SpellcheckEditor::toggleCaseInsensitiveSort()
{
  ui.CaseInsensitiveSort->click();
  ui.SpellcheckEditorTree->setFocus();
}

int SpellcheckEditor::SelectedRowsCount()
{
    int count = 0;

    if (ui.SpellcheckEditorTree->selectionModel()->hasSelection()) {
        count = ui.SpellcheckEditorTree->selectionModel()->selectedRows(0).count();
    }

    return count;
}

QList<QStandardItem *> SpellcheckEditor::GetSelectedItems(const int column)
{
    QList<QStandardItem *> selected_items;
    if (SelectedRowsCount() < 1) {
        return selected_items;
    }

    // Shift-click order is top to bottom regardless of starting position
    // Ctrl-click order is first clicked to last clicked (included shift-clicks stay ordered as is)
    QModelIndexList selected_indexes = ui.SpellcheckEditorTree->selectionModel()->selectedRows(column);
    foreach(QModelIndex index, selected_indexes) {
        selected_items.append(m_SpellcheckEditorModel->itemFromIndex(index));
    }
    return selected_items;
}

void SpellcheckEditor::Ignore()
{
    if (SelectedRowsCount() < 1) {
        emit ShowStatusMessageRequest(tr("No words selected."));
        return;
    }

    m_MultipleSelection = SelectedRowsCount() > 1;

    foreach (QStandardItem *item, GetSelectedItems(0)) {       
        int r{item->row()};
        QString lang{m_SpellcheckEditorModel->item(r,1)->text()};
        if(m_SpellCheck->ignoreWord(item->text(),lang)){
            MarkSpelledOkay(item->row());
        }
    }

    if (m_MultipleSelection) {
        m_MultipleSelection = false;
        FindSelectedWord();
    }
    emit ShowStatusMessageRequest(tr("Ignored word(s)."));
    emit SpellingHighlightRefreshRequest();
}

void SpellcheckEditor::Add()
{
    if (SelectedRowsCount() < 1) {
        emit ShowStatusMessageRequest(tr("No words selected."));
        return;
    }

    QString dict_name = ui.Dictionaries->currentText();
    if (dict_name.isEmpty()) {
        return;
    }

    m_MultipleSelection = SelectedRowsCount() > 1;

    SpellCheck *sc = SpellCheck::instance();
    SettingsStore settings;
    QStringList enabled_dicts = settings.enabledUserDictionaries();
    bool enabled = false;
    foreach (QStandardItem *item, GetSelectedItems(0)) {
        sc->addToUserDictionary(item->text(), dict_name);
        if (enabled_dicts.contains(dict_name)) {
            enabled = true;
            MarkSpelledOkay(item->row());
        }
    }

    if (m_MultipleSelection) {
        m_MultipleSelection = false;
        FindSelectedWord();
    }
    if (enabled) {
        emit ShowStatusMessageRequest(tr("Added word(s) to dictionary."));
    } else {
        emit ShowStatusMessageRequest(tr("Added word(s) to dictionary. The dictionary is not enabled in Preferences."));
    }
    emit SpellingHighlightRefreshRequest();
}

void SpellcheckEditor::ChangeAll()
{
    QString old_word = GetSelectedWord();
    if (old_word.isEmpty()) {
        emit ShowStatusMessageRequest(tr("No words selected."));
        return;
    }
    QString new_word = ui.cbChangeAll->currentText();

    if (new_word.contains("<") || new_word.contains(">") || new_word.contains("&")) {
        Utility::DisplayStdErrorDialog(tr("The new word cannot contain \"<\", \">\", or \"&\"."));
        return;
    }

    m_SelectRow = GetSelectedRow();

    emit UpdateWordRequest(old_word, new_word);
}

void SpellcheckEditor::MarkSpelledOkay(int row)
{
    m_SpellcheckEditorModel->invisibleRootItem()->child(row, 3)->setText(tr("No"));
    if (ui.ShowAllWords->checkState() == Qt::Unchecked) {
        m_SpellcheckEditorModel->removeRows(row, 1);
        if (row >= m_SpellcheckEditorModel->rowCount()) {
            row--;
        }
        if (row >= 0) {
            ui.SpellcheckEditorTree->selectionModel()->clear();
            QModelIndex index = m_SpellcheckEditorModel->index(row, 0);
            ui.SpellcheckEditorTree->setCurrentIndex(index);
            ui.SpellcheckEditorTree->selectionModel()->select(index, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
        }
    }
}

void SpellcheckEditor::CreateModel(int sort_column, Qt::SortOrder sort_order)
{
    m_SpellcheckEditorModel->clear();
    QStringList header;
    header.append(tr("Word"));
    header.append(tr("Language"));
    header.append(tr("Count"));
    header.append(tr("Misspelled?"));
    m_SpellcheckEditorModel->setHorizontalHeaderLabels(header);
    ui.SpellcheckEditorTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui.SpellcheckEditorTree->resizeColumnToContents(1);
    ui.SpellcheckEditorTree->resizeColumnToContents(2);
    ui.SpellcheckEditorTree->resizeColumnToContents(3);

    //the QString has format "language,word"
    QHash<QString, int> unique_words = m_Book->GetUniqueWordsInHTMLFiles();
    int wc=unique_words.size();
    int total_misspelled_words = 0;


    QHashIterator<QString, int> i(unique_words);
    while (i.hasNext()) {
        i.next();
        QStringList w=i.key().split(",");
        QString word = w.at(1);
        QString lang = w.at(0);
        int count = unique_words.value(i.key());
        wc+=count-1;

        bool misspelled = !m_SpellCheck->spell(word,lang);
        if (misspelled) {
            total_misspelled_words++;
        }

        if (ui.ShowAllWords->checkState() == Qt::Unchecked && !misspelled) {
            continue;
        }

        QList<QStandardItem *> row_items;

        if (ui.CaseInsensitiveSort->checkState() == Qt::Unchecked) {
            QStandardItem *word_item = new QStandardItem(word);
            word_item->setEditable(false);
            row_items << word_item;
        } else {
            CaseInsensitiveItem *word_item = new CaseInsensitiveItem();
            word_item->setText(word);
            word_item->setEditable(false);
            row_items << word_item;
        }

        QStandardItem *lang_item=new QStandardItem();
        lang_item->setText((lang.isEmpty())?"?":lang);
        row_items << lang_item;

        NumericItem *count_item = new NumericItem();
        count_item->setText(QString::number(count));
        row_items << count_item;

        QStandardItem *misspelled_item = new QStandardItem();
        misspelled_item->setEditable(false);
        if (misspelled) {
            misspelled_item->setText(tr("Yes"));
        } else {
            misspelled_item->setText(tr("No"));
        }
        if(lang.isEmpty()||
                //somebody could have defined dectionary for "" TODO?
                 !m_SpellCheck->alreadyLoadedDics().contains(m_SpellCheck->codeToAlias(lang)))
                    misspelled_item->setText("?");
        row_items << misspelled_item ;
        ui.le_wordc->setText(QString::number((wc))+
                             QString("(")+
                             QString::number((total_misspelled_words))+QChar(')'));
        m_SpellcheckEditorModel->invisibleRootItem()->appendRow(row_items);
    }

    // Since sortIndicator calls this routine, must disconnect/reconnect while resorting
    disconnect(ui.SpellcheckEditorTree->header(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)), this, SLOT(Sort(int, Qt::SortOrder)));
    ui.SpellcheckEditorTree->header()->setSortIndicator(sort_column, sort_order);
    connect(ui.SpellcheckEditorTree->header(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)), this, SLOT(Sort(int, Qt::SortOrder)));


    ui.SpellcheckEditorTree->header()->setToolTip("<table><tr><td>" % tr("Misspelled Words") % ":</td><td>" % QString::number(total_misspelled_words) % "</td></tr><tr><td>" % tr("Total Unique Words") % ":</td><td>" % QString::number(unique_words.count()) % "</td></tr></table>");
}

void SpellcheckEditor::Refresh(int sort_column, Qt::SortOrder sort_order)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    WriteSettings();
    setupMultiLanguageUi(); //must be before CreateModel
    CreateModel(sort_column, sort_order);
    UpdateDictionaries();

    ReadSettings();

    ui.FilterText->setFocus();
    FilterEditTextChangedSlot(ui.FilterText->text());

    SelectRow(m_SelectRow);
    UpdateSuggestions();

    QApplication::restoreOverrideCursor();
}

void SpellcheckEditor::UpdateDictionaries()
{
    ui.Dictionaries->clear();
    SpellCheck *sc = SpellCheck::instance();
    QStringList dicts = sc->userDictionaries();
    if (dicts.count() > 0) {
        ui.Dictionaries->addItems(dicts);
    }
}

void SpellcheckEditor::DictionaryChanged(QString dictionary)
{
    ui.Dictionaries->setToolTip(dictionary);
}

void SpellcheckEditor::ChangeState(int state)
{
    Refresh();
}

void SpellcheckEditor::SelectAll()
{
    ui.SpellcheckEditorTree->selectAll();
}

//this function return string "language code,word"
//all clients have to account for it
QString SpellcheckEditor::GetSelectedWord()
{
    QString word;
    QString lang;

    if (SelectedRowsCount() != 1 || m_MultipleSelection) {
        return word;
    }

    QModelIndex index = ui.SpellcheckEditorTree->selectionModel()->selectedRows(0).first();
    word = m_SpellcheckEditorModel->itemFromIndex(index)->text();
    index = ui.SpellcheckEditorTree->selectionModel()->selectedRows(1).first();
    lang = m_SpellcheckEditorModel->itemFromIndex(index)->text();
    return lang + QChar(',') + word;
}

int SpellcheckEditor::GetSelectedRow()
{
    int row = -1;

    if (SelectedRowsCount() != 1 || m_MultipleSelection) {
        return row;
    }

    return ui.SpellcheckEditorTree->selectionModel()->selectedRows(0).first().row();
}

void SpellcheckEditor::SelectRow(int row)
{
    QStandardItem *root_item = m_SpellcheckEditorModel->invisibleRootItem();

    if (root_item->rowCount() > 0 && row >= 0) {
        if (row >= root_item->rowCount()) {
            row = root_item->rowCount() - 1;
        }

        QStandardItem *child = root_item->child(row, 0);
        if (child) {
            QModelIndex index = child->index();
            ui.SpellcheckEditorTree->setFocus();
            ui.SpellcheckEditorTree->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
            ui.SpellcheckEditorTree->setCurrentIndex(child->index());
        }
    }

    m_SelectRow = -1;
}

void SpellcheckEditor::UpdateSuggestions()
{
    ui.cbChangeAll->clear();
    QString lw{GetSelectedWord()};

    if (!lw.isEmpty()) {
        ui.cbChangeAll->addItems(m_SpellCheck->suggestML(lw));
        ui.cbChangeAll->setToolTip("<b>"+ui.cbChangeAll->currentText()+"</b>");
        ui.cbChangeAll->setToolTipDuration(5000);
    }
}

void SpellcheckEditor::FindSelectedWord()
{
    QString word = GetSelectedWord();
    if (!word.isEmpty()) {
        emit FindWordRequest(word);
    }
}

void SpellcheckEditor::SelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    UpdateSuggestions();
    FindSelectedWord();
}

void SpellcheckEditor::FilterEditTextChangedSlot(const QString &text)
{
    const QString lowercaseText = text.toLower();
    QModelIndex root_index = m_SpellcheckEditorModel->indexFromItem(m_SpellcheckEditorModel->invisibleRootItem());

    for (int row = 0; row < m_SpellcheckEditorModel->invisibleRootItem()->rowCount(); row++) {
        QStandardItem *item = m_SpellcheckEditorModel->item(row, 0);
        bool hidden = !(text.isEmpty() || item->text().toLower().contains(lowercaseText));
        ui.SpellcheckEditorTree->setRowHidden(item->row(), root_index, hidden);
    }
}

void SpellcheckEditor::Sort(int logicalindex, Qt::SortOrder order)
{
    Refresh(logicalindex, order);
}

void SpellcheckEditor::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    QByteArray geometry = settings.value("geometry").toByteArray();

    if (!geometry.isNull()) {
        restoreGeometry(geometry);
    }

    // Selected Dictionary
    int index = -1;
    QString dictionary;
    if (settings.contains(SELECTED_DICTIONARY)) {
        dictionary = settings.value(SELECTED_DICTIONARY).toString();
        index = ui.Dictionaries->findText(dictionary);
    }
    if (index < 0) {
        index = 0;
    }
    ui.Dictionaries->setCurrentIndex(index);
    if (dictionary.isEmpty()) {
        dictionary = ui.Dictionaries->currentText();
    }
    ui.Dictionaries->setToolTip(dictionary);

    // Checkboxes
    // Disconnect signals to avoid refresh when setting.
    if (settings.contains(SHOW_ALL_WORDS)) {
        if (settings.value(SHOW_ALL_WORDS).toBool()) {
            disconnect(ui.ShowAllWords, SIGNAL(stateChanged(int)),
                       this, SLOT(ChangeState(int)));
            ui.ShowAllWords->setCheckState(Qt::Checked);
            connect(ui.ShowAllWords, SIGNAL(stateChanged(int)),
                    this, SLOT(ChangeState(int)));
        }
    }
    if (settings.contains(CASE_INSENSITIVE_SORT)) {
        if (settings.value(CASE_INSENSITIVE_SORT).toBool()) {
            disconnect(ui.CaseInsensitiveSort, SIGNAL(stateChanged(int)),
                       this, SLOT(ChangeState(int)));
            ui.CaseInsensitiveSort->setCheckState(Qt::Checked);
            connect(ui.CaseInsensitiveSort, SIGNAL(stateChanged(int)),
                    this, SLOT(ChangeState(int)));
        }
    }

    // Sort Order.
    if (settings.contains(SORT_COLUMN) && settings.contains(SORT_ORDER)) {
        int sort_column = settings.value(SORT_COLUMN).toInt();
        Qt::SortOrder sort_order = Qt::AscendingOrder;
        if (!settings.value(SORT_ORDER).toBool()) {
            sort_order = Qt::DescendingOrder;
        }
        // Since sortIndicator calls this routine, must disconnect/reconnect while resorting
        disconnect(ui.SpellcheckEditorTree->header(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)), this, SLOT(Sort(int, Qt::SortOrder)));
        ui.SpellcheckEditorTree->header()->setSortIndicator(sort_column, sort_order);
        connect(ui.SpellcheckEditorTree->header(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)), this, SLOT(Sort(int, Qt::SortOrder)));
    }

    settings.endGroup();
}

void SpellcheckEditor::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    settings.setValue("geometry", saveGeometry());

    // Selected Dictionary
    settings.setValue(SELECTED_DICTIONARY, ui.Dictionaries->currentText());

    // Checkboxes
    settings.setValue(SHOW_ALL_WORDS, ui.ShowAllWords->checkState() == Qt::Checked);
    settings.setValue(CASE_INSENSITIVE_SORT, ui.CaseInsensitiveSort->checkState() == Qt::Checked);
    settings.setValue(SORT_COLUMN, ui.SpellcheckEditorTree->header()->sortIndicatorSection());
    settings.setValue(SORT_ORDER, ui.SpellcheckEditorTree->header()->sortIndicatorOrder() == Qt::AscendingOrder);

    settings.endGroup();
}

//***varlogs multilanguage

void SpellcheckEditor::setupMultiLanguageUi()
{

//   QStringList haveAlready(m_SpellCheck->alreadyLoadedDics());
//    QStringList langs{};
//    foreach (QString l, haveAlready) {
//       QString ln=Language::instance()->GetLanguageName(toCodeName(l));
//       if (ln.isEmpty()) ln=toCodeName(l);
//       langs.append(ln);
//    }

      populateLoadedDicsTable();
}

void SpellcheckEditor::setupLoadedDicsTable(){
    ui.twLoadedDics->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui.twLoadedDics->setColumnCount(3);
    ui.twLoadedDics->setHorizontalHeaderLabels({tr("Code"),tr("Alias"),tr("Language")});
    ui.twLoadedDics->verticalHeader()->hide();
    ui.twLoadedDics->horizontalHeader()->setStretchLastSection(true);
    ui.twLoadedDics->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
}

void SpellcheckEditor::populateLoadedDicsTable(){

        QStringList dicts{ m_SpellCheck->alreadyLoadedDics()};
        ui.twLoadedDics->setRowCount(0);
        int i=0;

        for(auto dic:dicts){
            QString name=Language::instance()->GetLanguageName(toCodeName(dic));
            if(name.isEmpty()){
                name=toCodeName(dic);
            }
            QStringList codes(m_SpellCheck->aliasToCode(dic));
            for(auto code:codes){
                QTableWidgetItem *item1= new QTableWidgetItem(code);
                QTableWidgetItem *item2= new QTableWidgetItem(dic);
                QTableWidgetItem *item3= new QTableWidgetItem(name);
                int flags(Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsUserCheckable);
                item1->setFlags(item1->flags() & ~flags);
                item2->setFlags(item2->flags() & ~flags);
                item3->setFlags(item3->flags() & ~flags);
                item1->setToolTip(tr("Double click to change"));
                item2->setToolTip(tr("Double click to change"));
                ui.twLoadedDics->setRowCount(i+1);
                ui.twLoadedDics->setItem(i,0,item1);
                ui.twLoadedDics->setItem(i,1,item2);
                ui.twLoadedDics->setItem(i,2,item3);
                i++;
            }
        }

        ui.twLoadedDics->resizeColumnToContents(0);
        ui.twLoadedDics->resizeColumnToContents(1);
}

void SpellcheckEditor::changeCodeOrAlias(const int row, const int column){
    if(column==0){
        bool ok;
        QString orig_code{ui.twLoadedDics->item(row,column)->text()};
        QString code = QInputDialog::getText(this, tr("Change language code for dictionary"),
                                   tr("Code:"), QLineEdit::Normal,orig_code,&ok);
        if(ok && !code.isEmpty()){
            ui.twLoadedDics->item(row,column)->setText(code);
            m_SpellCheck->removeDictionaryAlias(orig_code);
            m_SpellCheck->setDictionaryAlias(code,ui.twLoadedDics->item(row,1)->text());
            m_SpellCheck->loadDictionaryForLang(code);
            Refresh();
        }
        return;
    }
    if(column==1){
        QString origCode{ui.twLoadedDics->item(row,0)->text()};
        QString code{choseDictionary()};
        if(!code.isEmpty() && !code.isNull()){
            QString dic{origCode};
            QString alias{toDicName(code)};
            m_SpellCheck->unloadDictionary(dic);
            m_SpellCheck->setDictionaryAlias(origCode,alias);
            m_SpellCheck->loadDictionaryForLang(origCode);
            Refresh();
        }

    }
}

const QString SpellcheckEditor::getSelectedWordLanguage(){

    if (SelectedRowsCount() != 1 || m_MultipleSelection) {
        return "";
    }
    QStandardItem *item {GetSelectedItems(1).first()};
    return item->text();
}

//slots
void SpellcheckEditor::loadDictionary(const QString lang){
    m_SpellCheck->loadDictionaryForLang(lang);
}
//returns language code
const QString SpellcheckEditor::choseDictionary() {
    QStringList dicts(m_SpellCheck->dictionaries());
    QStringList dics{};

    foreach (QString l, dicts) {
       QString ln=Language::instance()->GetLanguageName(toCodeName(l));
       if (ln.isEmpty()) ln=l;
       dics.append(ln);
    }
    dics.sort();
    bool ok;
    QString c=QInputDialog::getItem(this,
                            tr("Dictionaries present on system"),
                            tr("Chose dictionary to load:"),dics,0,false,&ok);;
    if(ok && !c.isEmpty()){
        QString langCode(Language::instance()->GetLanguageCode(c));
        if(!langCode.isEmpty()) c=langCode;
        return c;
    }
    return QString();
}
void SpellcheckEditor::loadDictionary()
{
   QString c{choseDictionary()};
   if(!c.isEmpty()){
        m_SpellCheck->loadDictionaryForLang(c);
        setupMultiLanguageUi();
        Refresh();
    }

}

void SpellcheckEditor::unloadDictionary()
{
    QStringList loadedDics(m_SpellCheck->alreadyLoadedDics());
    if(loadedDics.isEmpty()) return;

    bool ok;
    QString c= QInputDialog::getItem(this,
                           tr("Loaded Dictionaries"),
                           tr("Chose dictionary to unload:"),loadedDics,0,false,&ok);
    if(ok && !c.isEmpty()){
        m_SpellCheck->unloadDictionary(c);
        setupMultiLanguageUi();
        Refresh();
    }
}

void SpellcheckEditor::getDictionary(){

    if (SelectedRowsCount() < 1) {
        emit ShowStatusMessageRequest(tr("No language selected."));
        return;
    }

    m_MultipleSelection = SelectedRowsCount() > 1;
    if(m_MultipleSelection){
        emit ShowStatusMessageRequest(tr("No multiple selection allowed."));
        return;
    }
    //TODO: get rid of loop
    foreach (QStandardItem *item, GetSelectedItems(1)) {
        QString lang(item->text());
        QString l=m_SpellCheck->findDictionary(lang);
        if(!l.isEmpty()){
            m_SpellCheck->loadDictionaryForLang(lang);
            Refresh();
            emit SpellingHighlightRefreshRequest();
        }
    }
}
void SpellcheckEditor::changeDictionary(){

    if (SelectedRowsCount() < 1) {
        emit ShowStatusMessageRequest(tr("No item selected."));
        return;
    }

    m_MultipleSelection = SelectedRowsCount() > 1;
    if(m_MultipleSelection){
        emit ShowStatusMessageRequest(tr("No multiple selection allowed."));
        return;
    }
    //TODO: get rid of loop
    foreach (QStandardItem *item, GetSelectedItems(1)) {
        QString code(item->text());
        if(!m_SpellCheck->isLoaded(code)){
            emit ShowStatusMessageRequest(tr("No Dictionary to change."));
            return;
        }
        QString dic=choseDictionary();
        if(!dic.isEmpty()){
            m_SpellCheck->setDictionaryAlias(code,toDicName(dic));
            m_SpellCheck->loadDictionaryForLang(code);
            Refresh();
            emit SpellingHighlightRefreshRequest();
        }
    }
}
QString SpellcheckEditor::toCodeName(QString code){
    return(code.replace('_','-'));
}
QString SpellcheckEditor::toDicName(QString code){
    return(code.replace('-','_'));
}

void SpellcheckEditor::update_cbCAToolTipp(const QString &word){
    //help for words too long for box
    ui.cbChangeAll->setToolTip("<b>"+word+"</b>");
}

//***varlogs multilanguage end

void SpellcheckEditor::CreateContextMenuActions()
{
    m_Ignore    = new QAction(tr("Ignore"),            this);
    m_Add       = new QAction(tr("Add to Dictionary"), this);
    m_Find      = new QAction(tr("Find in Text"),      this);
    m_SelectAll = new QAction(tr("Select All"),        this);
    m_loadDic   = new QAction(tr("Load Dictionary for Language"),this);
    m_changeDic = new QAction(tr("Change Dictionary for Language"),this);
    m_Ignore->setShortcut(QKeySequence(Qt::Key_F1));
    m_Add->setShortcut(QKeySequence(Qt::Key_F2));
    m_Find->setShortcut(QKeySequence(Qt::Key_F3));
    // Has to be added to the dialog itself for the keyboard shortcut to work.
    addAction(m_Ignore);
    addAction(m_Add);
    addAction(m_Find);
    addAction(m_loadDic);
    addAction(m_changeDic);
}

void SpellcheckEditor::OpenContextMenu(const QPoint &point)
{
    SetupContextMenu(point);
    m_ContextMenu->exec(ui.SpellcheckEditorTree->viewport()->mapToGlobal(point));
    m_ContextMenu->clear();
    // Make sure every action is enabled - in case shortcut is used after context menu disables some.
    m_Ignore->setEnabled(true);
    m_Add->setEnabled(true);
    m_Find->setEnabled(true);
    m_SelectAll->setEnabled(true);
    m_loadDic->setEnabled(true);
    m_changeDic->setEnabled(true);
}

void SpellcheckEditor::SetupContextMenu(const QPoint &point)
{
    int selected_rows_count = SelectedRowsCount();
    m_ContextMenu->addAction(m_Ignore);
    m_Ignore->setEnabled(selected_rows_count > 0);
    m_ContextMenu->addAction(m_Add);
    m_Add->setEnabled(selected_rows_count > 0);
    m_ContextMenu->addAction(m_Find);
    m_Find->setEnabled(selected_rows_count > 0);
    m_ContextMenu->addSeparator();
    m_ContextMenu->addAction(m_SelectAll);
    m_ContextMenu->addSeparator();
    m_ContextMenu->addAction(m_loadDic);
    m_ContextMenu->addAction(m_changeDic);
}

void SpellcheckEditor::ForceClose()
{
    close();
}

void SpellcheckEditor::ConnectSignalsSlots()
{
    QItemSelectionModel *selectionModel = ui.SpellcheckEditorTree->selectionModel();
    connect(selectionModel,     SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
            this,               SLOT(SelectionChanged(const QItemSelection &, const QItemSelection &)));

    connect(ui.FilterText,  SIGNAL(textChanged(QString)), this, SLOT(FilterEditTextChangedSlot(QString)));
    connect(ui.Refresh, SIGNAL(clicked()), this, SLOT(Refresh()));
    connect(ui.Ignore, SIGNAL(clicked()), this, SLOT(Ignore()));
    connect(ui.Add, SIGNAL(clicked()), this, SLOT(Add()));
    connect(ui.ChangeAll, SIGNAL(clicked()), this, SLOT(ChangeAll()));
    connect(ui.SpellcheckEditorTree, SIGNAL(customContextMenuRequested(const QPoint &)),
            this,        SLOT(OpenContextMenu(const QPoint &)));
    connect(ui.SpellcheckEditorTree->header(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)),
            this, SLOT(Sort(int, Qt::SortOrder)));
    connect(m_Ignore,       SIGNAL(triggered()), this, SLOT(Ignore()));
    connect(m_Add,      SIGNAL(triggered()), this, SLOT(Add()));
    connect(m_Find,      SIGNAL(triggered()), this, SLOT(FindSelectedWord()));
    connect(m_SelectAll, SIGNAL(triggered()), this, SLOT(SelectAll()));

    connect(ui.SpellcheckEditorTree, SIGNAL(doubleClicked(const QModelIndex &)),
            this,         SLOT(FindSelectedWord()));
    connect(ui.ShowAllWords,  SIGNAL(stateChanged(int)),
            this,               SLOT(ChangeState(int)));
    connect(ui.CaseInsensitiveSort,  SIGNAL(stateChanged(int)),
            this,               SLOT(ChangeState(int)));
    connect(ui.Dictionaries, SIGNAL(activated(const QString &)),
            this,            SLOT(DictionaryChanged(const QString &)));

    connect(m_FilterSC, SIGNAL(activated()), ui.FilterText, SLOT(setFocus()));
    connect(m_ShowAllSC, SIGNAL(activated()), this, SLOT(toggleShowAllWords()));
    connect(m_NoCaseSC, SIGNAL(activated()), this, SLOT(toggleCaseInsensitiveSort()));
    connect(m_RefreshSC, SIGNAL(activated()), this, SLOT(Refresh()));

    //***varlogs
    connect(ui.pbAddDictionary, SIGNAL(clicked(bool)), this, SLOT(loadDictionary()));
    connect(ui.pbRemoveDictionary, SIGNAL(clicked(bool)),this,SLOT(unloadDictionary()));
    connect(m_loadDic, SIGNAL(triggered()), this, SLOT(getDictionary()));
    connect(m_changeDic,SIGNAL(triggered()),this, SLOT(changeDictionary()));
    connect(ui.cbChangeAll, SIGNAL(currentTextChanged(QString)),this,SLOT(update_cbCAToolTipp(QString)));
    connect(ui.twLoadedDics,SIGNAL(cellDoubleClicked(int,int)),this,SLOT(changeCodeOrAlias(int,int)));
}
