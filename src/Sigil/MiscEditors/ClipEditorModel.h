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

#pragma once
#ifndef CLIPEDITORMODEL_H
#define CLIPEDITORMODEL_H

#include <QtGui/QStandardItemModel>
#include <QDropEvent>

#include "Misc/SettingsStore.h"

class ClipEditorModel : public QStandardItemModel
{
     Q_OBJECT

public:
     ClipEditorModel(QObject *parent = 0);
     ~ClipEditorModel();

    static ClipEditorModel* instance();

    struct clipEntry {
        bool is_group;
        QString fullname;
        QString name;
        QString description;
        QString text;
    };

    bool ItemIsGroup(QStandardItem* item);

    QString GetFullName(QStandardItem* item);

    void LoadInitialData();
    void LoadData(QString filename = QString(), QStandardItem *parent_item = NULL);

    void AddFullNameEntry(ClipEditorModel::clipEntry *entry = NULL, QStandardItem *parent_item = NULL, int row = -1);

    QStandardItem* AddEntryToModel(ClipEditorModel::clipEntry *entry, bool is_group = false, QStandardItem *parent_item = NULL, int row = -1);

    QString SaveData(QList<ClipEditorModel::clipEntry*> entries = QList<ClipEditorModel::clipEntry*>(), QString filename = QString());

    QList<ClipEditorModel::clipEntry *> GetEntries(QList<QStandardItem*> items);
    ClipEditorModel::clipEntry* GetEntry(QStandardItem* item);
    ClipEditorModel::clipEntry* GetEntryFromName(QString name, QStandardItem *parent_item = NULL);


    QStandardItem* GetItemFromName(QString name, QStandardItem *item = NULL);

    QList<QStandardItem*> GetItemsForIndexes(QModelIndexList indexes);

    QList<QStandardItem*> GetNonGroupItems(QList<QStandardItem*> items);
    QList<QStandardItem*> GetNonGroupItems(QStandardItem* item);

    QList<QStandardItem*> GetNonParentItems(QList<QStandardItem*> items);
    QList<QStandardItem*> GetNonParentItems(QStandardItem* item);

    void Rename(QStandardItem *item, QString name="");

private slots:
    void ItemChangedHandler(QStandardItem *item);

private:
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    Qt::DropActions supportedDropActions() const;

    void UpdateFullName(QStandardItem *item);

    QStandardItem* GetItemFromId(qint64 id, int row, QStandardItem* item = NULL) const;

    void AddExampleEntries();

    static ClipEditorModel *m_instance;
};

#endif // CLIPEDITORMODEL_H
