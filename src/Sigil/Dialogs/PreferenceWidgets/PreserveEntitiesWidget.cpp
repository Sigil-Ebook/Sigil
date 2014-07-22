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

#include <QtGui/QDesktopServices>
#include <QtWidgets/QInputDialog>
#include "PreserveEntitiesWidget.h"
#include "Misc/SettingsStore.h"
#include "Misc/XMLEntities.h"

PreserveEntitiesWidget::PreserveEntitiesWidget()
    :
    m_isDirty(false)
{
    ui.setupUi(this);
    readSettings();
    connectSignalsToSlots();
}


PreferencesWidget::ResultAction PreserveEntitiesWidget::saveSettings()
{
    if (!m_isDirty) {
        return PreferencesWidget::ResultAction_None;
    }

    // Save preserve entities information
    SettingsStore settings;
    QList< std::pair< ushort, QString > > codenames; 
    for (int i = 0; i < ui.entityList->count(); ++i) {
        QString name = ui.entityList->item(i)->text();
        ushort code = XMLEntities::instance()->GetEntityCode(name);
        if (code > 0) {
            std::pair < ushort, QString > epair( code, name );
            codenames.append(epair);
        }
    }
    settings.setPreserveEntityCodeNames(codenames);

    return PreferencesWidget::ResultAction_None;
}


void PreserveEntitiesWidget::addEntities()
{
    QString list = QInputDialog::getText(this, tr("Add Entities"), tr("Entities:"));

    if (list.isEmpty()) {
        return;
    }

    list.replace(" ", ",");
    list.replace(",", "\n");
    QStringList names = list.split("\n");

    // Add the entities to the list
    foreach(QString name, names) {
        if (!name.isEmpty()) {
          if (XMLEntities::instance()->GetEntityCode(name) > 0) {
                QListWidgetItem *item = new QListWidgetItem(name, ui.entityList);
                item->setFlags(item->flags() | Qt::ItemIsEditable);
                ui.entityList->addItem(item);
            }
        }
    }
    ui.entityList->sortItems(Qt::AscendingOrder);

    m_isDirty = true;
}

void PreserveEntitiesWidget::removeEntity()
{
    foreach(QListWidgetItem * item, ui.entityList->selectedItems()) {
        ui.entityList->removeItemWidget(item);
        delete item;
        item = 0;
    }
    m_isDirty = true;
}

void PreserveEntitiesWidget::removeAll()
{
    ui.entityList->clear();
    m_isDirty = true;
}

void PreserveEntitiesWidget::readSettings()
{
    // Load the available entities.
    SettingsStore settings;
    std::pair < ushort, QString > epair;
    QList < std::pair < ushort, QString > >  codenames = settings.preserveEntityCodeNames();
    QStringList names;
    ui.entityList->clear();
    foreach( epair, codenames) {
        names.append(epair.second);
    }
    names.sort();
    foreach(QString name, names) {
        QListWidgetItem *item = new QListWidgetItem(name, ui.entityList);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ui.entityList->addItem(item);
    }

    m_isDirty = false;
}


void PreserveEntitiesWidget::ItemChanged(QStandardItem *item)
{
    m_isDirty = true;
}


void PreserveEntitiesWidget::connectSignalsToSlots()
{
    connect(ui.addEntities, SIGNAL(clicked()), this, SLOT(addEntities()));
    connect(ui.removeEntity, SIGNAL(clicked()), this, SLOT(removeEntity()));
    connect(ui.removeAll, SIGNAL(clicked()), this, SLOT(removeAll()));
    connect(&m_Model, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(ItemChanged(QStandardItem *)));
}
