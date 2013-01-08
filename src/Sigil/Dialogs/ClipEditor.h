/************************************************************************
**
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
#ifndef CLIPEDITOR_H
#define CLIPEDITOR_H

#include <QtWidgets/QDialog>
#include <QtGui/QStandardItemModel>

#include "Misc/SettingsStore.h"
#include "MiscEditors/ClipEditorModel.h"
#include "MiscEditors/ClipEditorTreeView.h"

#include "ui_ClipEditor.h"

/**
 * The editor used to create and modify saved clip entries
 */
class ClipEditor : public QDialog
{
    Q_OBJECT

public:
    ClipEditor(QWidget *parent);
    void ForceClose();

public slots:
    QStandardItem *AddEntry(bool is_group = false, ClipEditorModel::clipEntry *clip_entry = NULL, bool insert_after = true);

signals:
    void PasteSelectedClipRequest(QList<ClipEditorModel::clipEntry *> clip_entries);
    void ShowStatusMessageRequest(const QString &message);
    void ClipsUpdated();

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

    void Apply();
    bool Save();

    void MoveUp();
    void MoveDown();
    void MoveLeft();
    void MoveRight();

    void PasteIntoDocument();

    void FilterEditTextChangedSlot(const QString &text);

    void OpenContextMenu(const QPoint &point);

    void SettingsFileModelUpdated();

    void ModelItemDropped(const QModelIndex &index);

private:
    bool MaybeSaveDialogSaysProceed(bool is_forced);
    void MoveVertical(bool move_down);
    void MoveHorizontal(bool move_left);

    void ExportItems(QList<QStandardItem *> items);

    void SetupClipEditorTree();

    int SelectedRowsCount();
    QList<ClipEditorModel::clipEntry *> GetSelectedEntries();

    QList<QStandardItem *> GetSelectedItems();

    bool ItemsAreUnique(QList<QStandardItem *> items);

    bool SaveData(QList<ClipEditorModel::clipEntry *> entries = QList<ClipEditorModel::clipEntry *>() , QString filename = QString());

    bool FilterEntries(const QString &text, QStandardItem *item = NULL);
    bool SelectFirstVisibleNonGroup(QStandardItem *item);

    void ReadSettings();
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

    ClipEditorModel *m_ClipEditorModel;

    QString m_LastFolderOpen;

    QMenu *m_ContextMenu;

    QList<ClipEditorModel::clipEntry *> m_SavedClipEntries;

    Ui::ClipEditor ui;
};

#endif // CLIPEDITOR_H

