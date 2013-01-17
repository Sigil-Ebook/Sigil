/************************************************************************
**
**  Copyright (C) 2012 John Schember <john@nachtimwald.com>
**  Copyright (C) 2012 Dave Heiland
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

#include <QtCore/QSignalMapper>
#include <QtGui/QContextMenuEvent>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>

#include "Dialogs/SpellcheckEditor.h"
#include "Misc/SpellCheck.h"
#include "Misc/Utility.h"
#include "ResourceObjects/Resource.h"

static const QString SETTINGS_GROUP = "spellcheck_editor";
static const QString FILE_EXTENSION = "ini";

SpellcheckEditor::SpellcheckEditor(QWidget *parent)
    :
    QDialog(parent),
    m_Book(NULL),
    m_SpellcheckEditorModel(new QStandardItemModel(this)),
    m_ContextMenu(new QMenu(this))
{
    ui.setupUi(this);
    ui.FilterText->installEventFilter(this);

    SetupSpellcheckEditorTree();
    CreateContextMenuActions();
    ConnectSignalsSlots();
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
    ui.SpellcheckEditorTree->setSortingEnabled(false);
    ui.SpellcheckEditorTree->setWordWrap(true);
    ui.SpellcheckEditorTree->setAlternatingRowColors(true);
    ui.SpellcheckEditorTree->header()->setStretchLastSection(true);
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
    // Shift-click order is top to bottom regardless of starting position
    // Ctrl-click order is first clicked to last clicked (included shift-clicks stay ordered as is)
    QModelIndexList selected_indexes = ui.SpellcheckEditorTree->selectionModel()->selectedRows(0);
    QList<QStandardItem *> selected_items;
    foreach(QModelIndex index, selected_indexes) {
        selected_items.append(m_SpellcheckEditorModel->itemFromIndex(index));
    }
    return selected_items;
}

void SpellcheckEditor::Ignore()
{
    if (SelectedRowsCount() < 1) {
        return;
    }

    SpellCheck *sc = SpellCheck::instance();
    foreach (QStandardItem *item, GetSelectedItems()) {
        sc->ignoreWord(Utility::getSpellingSafeText(item->text()));
        m_SpellcheckEditorModel->invisibleRootItem()->child(item->row(), 1)->setCheckState(Qt::Unchecked);
    }

    emit SpellingHighlightRefreshRequest();
}

void SpellcheckEditor::Add()
{
    if (SelectedRowsCount() < 1) {
        return;
    }
    SpellCheck *sc = SpellCheck::instance();
    foreach (QStandardItem *item, GetSelectedItems()) {
        sc->addToUserDictionary(Utility::getSpellingSafeText(item->text()));
        m_SpellcheckEditorModel->invisibleRootItem()->child(item->row(), 1)->setCheckState(Qt::Unchecked);
    }

    emit SpellingHighlightRefreshRequest();
}

void SpellcheckEditor::CreateModel()
{
    m_SpellcheckEditorModel->clear();
    QStringList header;
    header.append(tr("Word"));
    header.append(tr("Misspelled?"));
    m_SpellcheckEditorModel->setHorizontalHeaderLabels(header);

    QSet<QString> unique_words = m_Book->GetWordsInHTMLFiles();
    QSet<QString> misspelled_words = m_Book->GetMisspelledWordsInHTMLFiles();
    ui.SpellcheckEditorTree->header()->setToolTip("<table><tr><td>" % tr("Misspelled Words") % ":</td><td>" % QString::number(misspelled_words.count()) % "</td></tr><tr><td>" % tr("Total Words") % ":</td><td>" % QString::number(unique_words.count()) % "</td></tr></table>");

    foreach(QString word, unique_words) {
        bool misspelled = false;
        if (misspelled_words.contains(word)) {
            misspelled = true;
        }

        if (ui.ShowAllWords->checkState() == Qt::Unchecked && !misspelled) {
            continue;
        }

        QStandardItem *word_item = new QStandardItem(word);
        QStandardItem *misspelled_item = new QStandardItem();
        word_item->setEditable(false);
        misspelled_item->setEditable(false);
        misspelled_item->setCheckable(false);
        if (misspelled) {
            misspelled_item->setCheckState(Qt::Checked);
        }
        else {
            misspelled_item->setCheckState(Qt::Unchecked);
        }

        QList<QStandardItem *> row_items;
        row_items << word_item << misspelled_item ;
        m_SpellcheckEditorModel->invisibleRootItem()->appendRow(row_items);
    }
    ui.SpellcheckEditorTree->sortByColumn(0, Qt::AscendingOrder);
}

void SpellcheckEditor::Refresh()
{
    WriteSettings();
    CreateModel();
    ReadSettings();

    ui.FilterText->setFocus();
}

void SpellcheckEditor::ChangeDisplayType(int state)
{
    Refresh();
}

void SpellcheckEditor::SelectAll()
{
    ui.SpellcheckEditorTree->selectAll();
}

void SpellcheckEditor::DoubleClick()
{
    QModelIndex index = ui.SpellcheckEditorTree->selectionModel()->selectedRows(0).first();
    QString word = m_SpellcheckEditorModel->itemFromIndex(index)->text();

    emit FindWordRequest(word);
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

    // Column widths
    int size = settings.beginReadArray("column_data");

    for (int column = 0; column < size && column < ui.SpellcheckEditorTree->header()->count(); column++) {
        settings.setArrayIndex(column);
        int column_width = settings.value("width").toInt();

        if (column_width) {
            ui.SpellcheckEditorTree->setColumnWidth(column, column_width);
        }
    }

}

void SpellcheckEditor::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    settings.setValue("geometry", saveGeometry());
    // Column widths
    settings.beginWriteArray("column_data");

    for (int column = 0; column < ui.SpellcheckEditorTree->header()->count(); column++) {
        settings.setArrayIndex(column);
        settings.setValue("width", ui.SpellcheckEditorTree->columnWidth(column));
    }

    settings.endArray();
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
    connect(ui.FilterText,  SIGNAL(textChanged(QString)), this, SLOT(FilterEditTextChangedSlot(QString)));
    connect(ui.Refresh, SIGNAL(clicked()), this, SLOT(Refresh()));
    connect(ui.Ignore, SIGNAL(clicked()), this, SLOT(Ignore()));
    connect(ui.Add, SIGNAL(clicked()), this, SLOT(Add()));
    connect(ui.SpellcheckEditorTree, SIGNAL(customContextMenuRequested(const QPoint &)),
            this,        SLOT(OpenContextMenu(const QPoint &)));
    connect(m_Ignore,       SIGNAL(triggered()), this, SLOT(Ignore()));
    connect(m_Add,      SIGNAL(triggered()), this, SLOT(Add()));
    connect(m_Find,      SIGNAL(triggered()), this, SLOT(DoubleClick()));
    connect(m_SelectAll, SIGNAL(triggered()), this, SLOT(SelectAll()));
    connect(ui.SpellcheckEditorTree, SIGNAL(doubleClicked(const QModelIndex &)),
            this,         SLOT(DoubleClick()));
    connect(ui.ShowAllWords,  SIGNAL(stateChanged(int)),
            this,               SLOT(ChangeDisplayType(int))
           );
}
