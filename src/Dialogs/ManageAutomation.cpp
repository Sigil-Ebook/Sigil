/************************************************************************
**
**  Copyright (C) 2015-2025 Kevin Hendricks, Stratford, Ontario Canada
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

#include <QInputDialog>
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "Dialogs/AutomateEditor.h"
#include "Dialogs/ManageAutomation.h"

static const QString SETTINGS_GROUP   = "manage_automation";

ManageAutomation::ManageAutomation(QStringList automationList)
: m_automateList(automationList)
{
    ui.setupUi(this);
    readSettings();
    m_automateList.sort(Qt::CaseInsensitive);
    foreach(QString lname, m_automateList) {
        QListWidgetItem *item = new QListWidgetItem(lname, ui.autoList);
        ui.autoList->addItem(item);
    }
    updateItemsInComboBoxes();    
    connectSignalsToSlots();
}

void ManageAutomation::accept() {
    saveSettings();
    QDialog::accept();
}

void ManageAutomation::reject() {
    saveSettings();
    QDialog::reject();
}

void ManageAutomation::newList()
{
    QString listid = QInputDialog::getText(this, tr("New List"), tr("List Identifier"));
    // remove anything the user entered that is stupid for file names
    listid.remove("&");
    listid.replace(" ", "_");
    listid.replace("\n", "");
    listid.remove("/");
    listid.remove("\\");
    if (listid.isEmpty()) {
        return;
    }
    if (m_automateList.contains(listid)) return;
    // create the empty file first
    QString automateFile = Utility::DefinePrefsDir() + "/automate" + listid + ".txt";
    Utility::WriteUnicodeTextFile(QString(""), automateFile);
    if (Utility::IsFileReadable(automateFile)) {
        // Add the new list identifier to our list of available automation list ids
        QListWidgetItem * item = new QListWidgetItem(listid, ui.autoList);
        ui.autoList->addItem(item);
        ui.autoList->sortItems(Qt::AscendingOrder);
        m_automateList << listid;
    }
    m_automateList.sort(Qt::CaseInsensitive);
    updateItemsInComboBoxes();
}

void ManageAutomation::renameList()
{
    QString listid = QInputDialog::getText(this, tr("New Name"), tr("List Identifier"));
    // remove anything the user entered that is stupid for file names
    listid.remove("&");
    listid.replace(" ", "_");
    listid.replace("\n", "");
    listid.remove("/");
    listid.remove("\\");
    if (listid.isEmpty()) {
        return;
    }
    if (m_automateList.contains(listid)) return;
    QString new_name = listid;
    QString new_file_name = Utility::DefinePrefsDir() + "/automate" + listid + ".txt";
    QString old_name;
    QList<QListWidgetItem*> selected = ui.autoList->selectedItems();
    QListWidgetItem* item = nullptr;
    if (selected.count() > 0) {
        item = selected.at(0);
    }
    if (!item) return;
    old_name = item->text();
    QString old_file_name = Utility::DefinePrefsDir() + "/automate" + old_name + ".txt";
    bool result = Utility::RenameFile(old_file_name, new_file_name);
    if (result) {
        ui.autoList->removeItemWidget(item);
        m_automateList.removeOne(old_name);
        delete item;
        QListWidgetItem * nitem = new QListWidgetItem(listid, ui.autoList);
        ui.autoList->addItem(nitem);
        ui.autoList->sortItems(Qt::AscendingOrder);
        m_automateList << listid;
        m_automateList.sort(Qt::CaseInsensitive);
        // first update the m_automMap to reflect the new name if needed
        for(int n=0; n < m_autoMap.count(); n++) {
            if (m_autoMap.at(n) == old_name) {
                m_autoMap.replace(n, new_name);
            }
        }
        // we need to update any quick launch associations that are impacted be the rename
        if (ui.comboBox1->currentText() == old_name) {
            ui.comboBox1->setCurrentText(new_name);
        }
        if (ui.comboBox2->currentText() == old_name) {
            ui.comboBox2->setCurrentText(new_name);
        }
        if (ui.comboBox3->currentText() == old_name) {
            ui.comboBox3->setCurrentText(new_name);
        }
    }
    updateItemsInComboBoxes();
}

void ManageAutomation::removeList()
{
    foreach(QListWidgetItem * item, ui.autoList->selectedItems()) {
        QString listid = item->text();
        // first remove the file
        QString automateFile = Utility::DefinePrefsDir() + "/automate" + listid + ".txt";
        bool result = Utility::SDeleteFile(automateFile);
        if (result) {
            ui.autoList->removeItemWidget(item);
            m_automateList.removeOne(listid);
            delete item;
        }
    }
    m_automateList.sort(Qt::CaseInsensitive);
    updateItemsInComboBoxes();
}

void ManageAutomation::editList()
{
    QList<QListWidgetItem*> selected = ui.autoList->selectedItems();
    QListWidgetItem* item = nullptr;
    if (selected.count() > 0) {
        item = selected.at(0);
    }
    if (!item) return;
    QString listid = item->text();
    QString automateFile = Utility::DefinePrefsDir() + "/automate" + listid + ".txt";
    AutomateEditor aedit(automateFile, this);
    aedit.exec();
}

void ManageAutomation::updateItemsInComboBoxes()
{
    QStringList cbitems = QStringList() << " ";
    cbitems.append(m_automateList);
    ui.comboBox1->clear();
    ui.comboBox2->clear();
    ui.comboBox3->clear();
    ui.comboBox1->addItems(cbitems);
    ui.comboBox2->addItems(cbitems);
    ui.comboBox3->addItems(cbitems);
    ui.comboBox1->setEditable(false);
    ui.comboBox2->setEditable(false);
    ui.comboBox3->setEditable(false);
    ui.comboBox1->setCurrentText(m_autoMap.at(0));
    ui.comboBox2->setCurrentText(m_autoMap.at(1));
    ui.comboBox3->setCurrentText(m_autoMap.at(2));
    ui.comboBox1->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    ui.comboBox2->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    ui.comboBox3->setSizeAdjustPolicy(QComboBox::AdjustToContents);
}

void ManageAutomation::readSettings()
{
    // Load the current automation Map
    SettingsStore settings;
    m_autoMap = settings.automateMap();
    while(m_autoMap.count() < 3) m_autoMap.append(QString(""));

    settings.beginGroup(SETTINGS_GROUP);
    QByteArray geometry = settings.value("geometry").toByteArray();
    if (!geometry.isNull()) {
        restoreGeometry(geometry);
    }
    settings.endGroup();

}


void ManageAutomation:: saveSettings()
{
    SettingsStore settings;
    QStringList autoMap;
    if (m_automateList.contains(ui.comboBox1->currentText())) {
        autoMap << ui.comboBox1->currentText();
    } else { 
        autoMap << QString("");
    }
    if (m_automateList.contains(ui.comboBox2->currentText())) {
        autoMap << ui.comboBox2->currentText();
    } else { 
        autoMap << QString("");
    }
    if (m_automateList.contains(ui.comboBox3->currentText())) {
        autoMap << ui.comboBox3->currentText();
    } else { 
        autoMap << QString("");
    }
    settings.setAutomateMap(autoMap);

    settings.beginGroup(SETTINGS_GROUP);
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();

}


void ManageAutomation::ItemChanged(QStandardItem *item)
{
    // unused
}


void ManageAutomation::connectSignalsToSlots()
{
    connect(ui.newList, SIGNAL(clicked()), this, SLOT(newList()));
    connect(ui.removeList, SIGNAL(clicked()), this, SLOT(removeList()));
    connect(ui.renameList, SIGNAL(clicked()), this, SLOT(renameList()));
    connect(ui.editList, SIGNAL(clicked()), this, SLOT(editList()));
    connect(&m_Model, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(ItemChanged(QStandardItem *)));
}
