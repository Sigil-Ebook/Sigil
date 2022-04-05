/************************************************************************
**
**  Copyright (C) 2022 Kevin B. Hendricks, Stratford, Ontario, Canada
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
#ifndef COUNTSREPORT_H
#define COUNTSREPORT_H

#include <QDialog>
#include <QStandardItemModel>
#include "MiscEditors/SearchEditorModel.h"
#include "ui_CountsReport.h"

class QString;
class QCloseEvent;

class CountsReport: public QDialog
{
    Q_OBJECT

public:
    CountsReport(QWidget* parent=NULL);
    ~CountsReport();

    // entries are created with new and we take ownership of them and therefore
    // need to clean then up when done with them
    void CreateReport(QList<SearchEditorModel::searchEntry*> search_entries);

    void SetupTable(int sort_column = 1, Qt::SortOrder sort_order = Qt::AscendingOrder);

public slots:
    void closeEvent(QCloseEvent *e);
    void reject();
    void Save();

signals:
    // void CountRequest2(SearchEditorModel::searchEntry* entry, int& count);
    void CountRequest(SearchEditorModel::searchEntry* entry, int& count);

private slots:
    void Sort(int logicalindex, Qt::SortOrder order);
    void FilterEditTextChangedSlot(const QString &text);

private:
    void ReadSettings();
    void WriteSettings();

    void connectSignalsSlots();

    QStandardItemModel *m_ItemModel;

    QString m_LastDirSaved;
    QString m_LastFileSaved;

    QList<SearchEditorModel::searchEntry*> m_entries;
    Ui::CountsReport ui;
};

#endif // COUNTSREPORT_H
