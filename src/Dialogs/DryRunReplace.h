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
#ifndef DRYRUNREPLACE_H
#define DRYRUNREPLACE_H

#include <QDialog>
#include <QList>
#include <QStandardItemModel>
#include <ResourceObjects/Resource.h>
#include "ui_DryRunReplace.h"

class QString;

class DryRunReplace: public QDialog
{
    Q_OBJECT

public:
    DryRunReplace(QWidget* parent=NULL);
    ~DryRunReplace();

    void CreateReport(const QString& search_regex, const QString& replace_text,
                      const QList<Resource*> resources);

    void SetupTable(int sort_column = 1, Qt::SortOrder sort_order = Qt::AscendingOrder);

public slots:
    
private slots:
    void Sort(int logicalindex, Qt::SortOrder order);
    void FilterEditTextChangedSlot(const QString &text);

private:
    QString GetPriorContext(int start, const QString &text, int amt);
    QString GetPostContext(int end, const QString &text, int amt);

    void ReadSettings();
    void WriteSettings();

    void connectSignalsSlots();

    QStandardItemModel *m_ItemModel;

    QList<Resource*> m_resources;
    QString m_search_regex;
    QString m_replace_text;

    Ui::DryRunReplace ui;
};

#endif // DRYRUNREPLACE_H
