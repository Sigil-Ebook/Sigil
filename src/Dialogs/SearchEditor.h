/************************************************************************
**
**  Copyright (C) 2022 Kevin B. Hendricks, Stratford Ontario Canada
**  Copyright (C) 2012 John Schember <john@nachtimwald.com>
**  Copyright (C) 2012 Dave Heiland
**  Copyright (C) 2012 Grant Drake
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

#pragma once
#ifndef SEARCHEDITOR_H
#define SEARCHEDITOR_H

#include <QtWidgets/QDialog>
#include <QtGui/QStandardItemModel>
#include <QAction>
#include <QtWidgets/QMenu>
#include <QPointer>

#include "Misc/SettingsStore.h"
#include "MiscEditors/SearchEditorModel.h"
#include "MiscEditors/SearchEditorTreeView.h"

#include "ui_SearchEditor.h"

class SearchEditorItemDelegate;

/**
 * The editor used to create and modify saved searches
 */
class SearchEditor : public QDialog
{
    Q_OBJECT

public:
    SearchEditor(QWidget *parent);
    ~SearchEditor();

    void ForceClose();

    void RecordEntryAsCompleted(SearchEditorModel::searchEntry* entry);
    QList<SearchEditorModel::searchEntry*> GetCurrentEntries();
    int GetCurrentEntriesCount() { return m_CurrentSearchEntries.count(); }

public slots:

    QStandardItem *AddEntry(bool is_group = false, SearchEditorModel::searchEntry *search_entry = NULL, bool insert_after = true);

    void ShowMessage(const QString &message);

    void SelectionChanged();

    void SetCurrentEntriesFromFullName(const QString& name);

signals:

    void LoadSelectedSearchRequest(SearchEditorModel::searchEntry *search_entry);
    void FindSelectedSearchRequest();
    void ReplaceCurrentSelectedSearchRequest();
    void ReplaceSelectedSearchRequest();
    void CountAllSelectedSearchRequest();
    void ReplaceAllSelectedSearchRequest();
    void RestartSearch();
    void ShowStatusMessageRequest(const QString &message);
    void CountsReportCountRequest(SearchEditorModel::searchEntry* entry, int& count);

protected:
    bool eventFilter(QObject *obj, QEvent *ev);

protected slots:
    void reject();
    void showEvent(QShowEvent *event);

private slots:
    QStandardItem *AddGroup();
    void Edit();
    void Cut();
    bool Copy();
    void Paste();
    void Delete();
    void Import();
    void Reload();
    void Export();
    void ExportAll();
    void CollapseAll();
    void ExpandAll();
    void FillControls();
    
    void Apply();
    bool Save();

    void MoveUp();
    void MoveDown();
    void MoveLeft();
    void MoveRight();

    void LoadFindReplace();
    void Find();
    void ReplaceCurrent();
    void Replace();
    void CountAll();
    void ReplaceAll();

    void FilterEditTextChangedSlot(const QString &text);

    void OpenContextMenu(const QPoint &point);

    void SettingsFileModelUpdated();

    void ModelItemDropped(const QModelIndex &index);

    void MakeCountsReport();

private:

    bool MaybeSaveDialogSaysProceed(bool is_forced);
    void MoveVertical(bool move_down);
    void MoveHorizontal(bool move_left);

    void ExportItems(QList<QStandardItem *> items);

    void SetupSearchEditorTree();

    int SelectedRowsCount();

    SearchEditorModel::searchEntry *GetSelectedEntry(bool show_warning = true);
    QList<SearchEditorModel::searchEntry *> GetSelectedEntries();
    
    QList<QStandardItem *> GetSelectedItems();

    bool ItemsAreUnique(QList<QStandardItem *> items);

    bool SaveData(QList<SearchEditorModel::searchEntry *> entries = QList<SearchEditorModel::searchEntry *>() , QString filename = QString());

    bool SaveTextData(QList<SearchEditorModel::searchEntry *> entries = QList<SearchEditorModel::searchEntry *>() ,
                      QString filename = QString(), QChar sep=QChar(9));

    bool FilterEntries(const QString &text, QStandardItem *item = NULL);
    bool SelectFirstVisibleNonGroup(QStandardItem *item);

    bool ReadSettings();
    void WriteSettings();

    void CreateContextMenuActions();
    void SetupContextMenu(const QPoint &point);

    void ConnectSignalsSlots();

    QAction *m_AddEntry;
    QAction *m_AddGroup;
    QAction *m_Edit;
    QAction *m_Cut;
    QAction *m_Copy;
    QAction *m_Paste;
    QAction *m_Delete;
    QAction *m_Import;
    QAction *m_Reload;
    QAction *m_Export;
    QAction *m_ExportAll;
    QAction *m_CollapseAll;
    QAction *m_ExpandAll;
    QAction *m_FillIn;

    SearchEditorModel *m_SearchEditorModel;

    QString m_LastFolderOpen;

    QPointer<QMenu> m_ContextMenu;

    // stores result of cut/copy for later paste
    QList<SearchEditorModel::searchEntry *> m_SavedSearchEntries;

    // List of the remaining currently selected Entries updated  to remember state
    QList<SearchEditorModel::searchEntry *> m_CurrentSearchEntries;

    SearchEditorModel::searchEntry * m_SearchToLoad;

    SearchEditorItemDelegate * m_CntrlDelegate;

    Ui::SearchEditor ui;
};

#endif // SEARCHEDITOR_H
