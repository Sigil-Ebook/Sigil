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

#include "Dialogs/SpellcheckEditor.h"
#include "Misc/NumericItem.h"
#include "Misc/SettingsStore.h"
#include "Misc/SpellCheck.h"
#include "Misc/Utility.h"
#include "ResourceObjects/Resource.h"

static const QString SETTINGS_GROUP = "spellcheck_editor";
static const QString SELECTED_DICTIONARY = "selected_dictionary";
static const QString FILE_EXTENSION = "ini";

SpellcheckEditor::SpellcheckEditor(QWidget *parent)
    :
    QDialog(parent),
    m_Book(NULL),
    m_SpellcheckEditorModel(new QStandardItemModel(this)),
    m_ContextMenu(new QMenu(this)),
    m_MultipleSelection(false)
{
    ui.setupUi(this);
    ui.FilterText->installEventFilter(this);

    SetupSpellcheckEditorTree();
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

int SpellcheckEditor::SelectedRowsCount()
{
    int count = 0;

    if (ui.SpellcheckEditorTree->selectionModel()->hasSelection()) {
        count = ui.SpellcheckEditorTree->selectionModel()->selectedRows(0).count();
    }

    return count;
}

QList<QStandardItem *> SpellcheckEditor::GetSelectedItems()
{
    QList<QStandardItem *> selected_items;
    if (SelectedRowsCount() < 1) {
        return selected_items;
    }

    // Shift-click order is top to bottom regardless of starting position
    // Ctrl-click order is first clicked to last clicked (included shift-clicks stay ordered as is)
    QModelIndexList selected_indexes = ui.SpellcheckEditorTree->selectionModel()->selectedRows(0);
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

    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_MultipleSelection = SelectedRowsCount() > 1;

    SpellCheck *sc = SpellCheck::instance();
    foreach (QStandardItem *item, GetSelectedItems()) {
        sc->ignoreWord(Utility::getSpellingSafeText(item->text()));
        MarkSpelledOkay(item->row());
    }

    if (m_MultipleSelection) {
        m_MultipleSelection = false;
        FindSelectedWord();
    }
    emit ShowStatusMessageRequest(tr("Ignored word(s)."));
    emit SpellingHighlightRefreshRequest();
    QApplication::restoreOverrideCursor();
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

    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_MultipleSelection = SelectedRowsCount() > 1;

    SpellCheck *sc = SpellCheck::instance();
    SettingsStore settings;
    QStringList enabled_dicts = settings.enabledUserDictionaries();
    foreach (QStandardItem *item, GetSelectedItems()) {
        sc->addToUserDictionary(Utility::getSpellingSafeText(item->text()), dict_name);
        if (enabled_dicts.contains(dict_name)) {
            MarkSpelledOkay(item->row());
        }
    }

    if (m_MultipleSelection) {
        m_MultipleSelection = false;
        FindSelectedWord();
    }
    emit ShowStatusMessageRequest(tr("Added word(s) to dictionary."));
    emit SpellingHighlightRefreshRequest();
    QApplication::restoreOverrideCursor();
}

void SpellcheckEditor::MarkSpelledOkay(int row)
{
    m_SpellcheckEditorModel->invisibleRootItem()->child(row, 1)->setText(tr("No"));
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

void SpellcheckEditor::CreateModel()
{
    m_SpellcheckEditorModel->clear();
    QStringList header;
    header.append(tr("Word"));
    header.append(tr("Count"));
    header.append(tr("Misspelled?"));
    m_SpellcheckEditorModel->setHorizontalHeaderLabels(header);
    ui.SpellcheckEditorTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui.SpellcheckEditorTree->resizeColumnToContents(1);
    ui.SpellcheckEditorTree->resizeColumnToContents(2);

    QHash<QString, int> unique_words = m_Book->GetUniqueWordsInHTMLFiles();

    int total_misspelled_words = 0;
    SpellCheck *sc = SpellCheck::instance();

    QHashIterator<QString, int> i(unique_words);
    while (i.hasNext()) {
        i.next();
        QString word = i.key();
        int count = unique_words.value(word);
        
        bool misspelled = !sc->spell(word);
        if (misspelled) {
            total_misspelled_words++;
        }

        if (ui.ShowAllWords->checkState() == Qt::Unchecked && !misspelled) {
            continue;
        }

        QStandardItem *word_item = new QStandardItem(word);
        NumericItem *count_item = new NumericItem();
        count_item->setText(QString::number(count));
        QStandardItem *misspelled_item = new QStandardItem();
        word_item->setEditable(false);
        misspelled_item->setEditable(false);
        if (misspelled) {
            misspelled_item->setText(tr("Yes"));
        }
        else {
            misspelled_item->setText(tr("No"));
        }

        QList<QStandardItem *> row_items;
        row_items << word_item << count_item << misspelled_item ;
        m_SpellcheckEditorModel->invisibleRootItem()->appendRow(row_items);
    }
    ui.SpellcheckEditorTree->sortByColumn(0, Qt::AscendingOrder);

    ui.SpellcheckEditorTree->header()->setToolTip("<table><tr><td>" % tr("Misspelled Words") % ":</td><td>" % QString::number(total_misspelled_words) % "</td></tr><tr><td>" % tr("Total Unique Words") % ":</td><td>" % QString::number(unique_words.count()) % "</td></tr></table>");
}

void SpellcheckEditor::Refresh()
{
    WriteSettings();
    CreateModel();
    UpdateDictionaries();

    ReadSettings();

    ui.FilterText->clear();
    ui.FilterText->setFocus();
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

void SpellcheckEditor::ChangeDisplayType(int state)
{
    Refresh();
}

void SpellcheckEditor::SelectAll()
{
    ui.SpellcheckEditorTree->selectAll();
}

void SpellcheckEditor::FindSelectedWord()
{
    if (SelectedRowsCount() != 1 || m_MultipleSelection) {
        return;
    }

    QModelIndex index = ui.SpellcheckEditorTree->selectionModel()->selectedRows(0).first();
    QString word = m_SpellcheckEditorModel->itemFromIndex(index)->text();

    emit FindWordRequest(word);
}

void SpellcheckEditor::SelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
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

    settings.endGroup();
}

void SpellcheckEditor::CreateContextMenuActions()
{
    m_Ignore    = new QAction(tr("Ignore"),            this);
    m_Add       = new QAction(tr("Add to Dictionary"), this);
    m_Find      = new QAction(tr("Find in Text"),      this);
    m_SelectAll = new QAction(tr("Select All"),        this);
    m_Ignore->setShortcut(QKeySequence(Qt::Key_F1));
    m_Add->setShortcut(QKeySequence(Qt::Key_F2));
    m_Find->setShortcut(QKeySequence(Qt::Key_F3));
    // Has to be added to the dialog itself for the keyboard shortcut to work.
    addAction(m_Ignore);
    addAction(m_Add);
    addAction(m_Find);
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
    connect(ui.SpellcheckEditorTree, SIGNAL(customContextMenuRequested(const QPoint &)),
            this,        SLOT(OpenContextMenu(const QPoint &)));
    connect(m_Ignore,       SIGNAL(triggered()), this, SLOT(Ignore()));
    connect(m_Add,      SIGNAL(triggered()), this, SLOT(Add()));
    connect(m_Find,      SIGNAL(triggered()), this, SLOT(FindSelectedWord()));
    connect(m_SelectAll, SIGNAL(triggered()), this, SLOT(SelectAll()));
    connect(ui.SpellcheckEditorTree, SIGNAL(doubleClicked(const QModelIndex &)),
            this,         SLOT(FindSelectedWord()));
    connect(ui.ShowAllWords,  SIGNAL(stateChanged(int)),
            this,               SLOT(ChangeDisplayType(int))
           );
    connect(ui.Dictionaries, SIGNAL(activated(const QString &)),
            this,            SLOT(DictionaryChanged(const QString &))
           );
}
