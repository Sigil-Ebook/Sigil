/************************************************************************
**
**  Copyright (C) 2015-2025 Kevin B. Hendricks, Startford Ontario Canada
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
#ifndef MANAGEAUTOMATION_H
#define MANAGEAUTOMATION_H

#include <QStandardItemModel>

#include "ui_ManageAutomation.h"

class ManageAutomation : public QDialog
{
    Q_OBJECT

public:
    ManageAutomation(QStringList automateList);

public slots:
    virtual void accept();
    virtual void reject();

private slots:
    void newList();
    void removeList();
    void renameList();
    void editList();
    void ItemChanged(QStandardItem *item);

private:
    void readSettings();
    void saveSettings();
    void updateItemsInComboBoxes();
    void connectSignalsToSlots();

    QStringList m_automateList;
    QStringList m_autoMap;
    QStandardItemModel m_Model;

    Ui::ManageAutomation ui;

};

#endif // MANAGE_AUTOMATION_H
