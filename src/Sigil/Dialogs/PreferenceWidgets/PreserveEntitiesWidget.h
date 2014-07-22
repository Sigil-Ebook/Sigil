/************************************************************************
**
**  Copyright (C) 2014 Kevin Hendricks
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
#ifndef PRESERVEENTITIESWIDGET_H
#define PRESERVEENTITIESWIDGET_H

#include "PreferencesWidget.h"
#include <QtGui/QStandardItemModel>

#include "ui_PPreserveEntitiesWidget.h"

class PreserveEntitiesWidget : public PreferencesWidget
{
    Q_OBJECT

public:
    PreserveEntitiesWidget();
    PreferencesWidget::ResultAction saveSettings();

private slots:
    void addEntities();
    void removeEntity();
    void removeAll();
    void ItemChanged(QStandardItem *item);

private:

    void readSettings();
    void connectSignalsToSlots();

    Ui::PreserveEntitiesWidget ui;
    bool m_isDirty;

    QStandardItemModel m_Model;
};

#endif // PRESERVEENTITIESWIDGET_H
