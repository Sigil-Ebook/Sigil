/************************************************************************
**
**  Copyright (C) 2025 Kevin B. Hendricks, Stratford, ON, Canada
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

#include <QDate>
#include <QModelIndex>

#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "Misc/AriaRoles.h"
#include "Dialogs/AddRoles.h"

static const QString SETTINGS_GROUP = "add_roles";

AddRoles::AddRoles(const QString& current_tag, QWidget *parent)
    :
    QDialog(parent),
    m_currentTag(current_tag)
{
    ui.setupUi(this);

    connect(ui.lwProperties, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
            this, SLOT(UpdateDescription(QListWidgetItem *)));
    connect(this, SIGNAL(accepted()), this, SLOT(WriteSettings()));
    connect(ui.lwProperties, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(accept()));

    // Fill the dialog with sorted translated role names
    QStringList names;
    foreach (QString code, AriaRoles::instance()->GetAllCodes()) {
        QStringList allowed_tags = AriaRoles::instance()->AllowedTags(code);
        QString name = AriaRoles::instance()->GetName(code);
        // Some translations are broken, add code in parentheses after translated name
        // To force them to be visually unique without having to read the entire Description
        name = name + " (" + code + ")";
        // only include roles if allowed on the current tag
        if (allowed_tags.contains(m_currentTag)) {
            m_Name2Code[name] = code;
            names.append(name);
        }
    }
    names = Utility::LocaleAwareSort(names);

    foreach(QString name, names) {
        ui.lwProperties->addItem(name);
    }
    ReadSettings();
}

void AddRoles::UpdateDescription(QListWidgetItem *current)
{
    QString text;
    QString code = m_Name2Code.value(current->text(), QString());
    if (!code.isEmpty()) {
        text = AriaRoles::instance()->GetDescriptionByCode(code);
    }
    if (!text.isEmpty()) {
        ui.lbDescription->setText(text);
    }
}

QStringList AddRoles::GetSelectedEntries()
{
    return m_SelectedEntries;
}

void AddRoles::SaveSelection()
{
    m_SelectedEntries.clear();
    foreach(QListWidgetItem * item, ui.lwProperties->selectedItems()) {
        QString code = m_Name2Code.value(item->text(), QString() );
        m_SelectedEntries.append(code);
    }
}


void AddRoles::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    QByteArray geometry = settings.value("geometry").toByteArray();

    if (!geometry.isNull()) {
        restoreGeometry(geometry);
    }

    QByteArray splitter_position = settings.value("splitter").toByteArray();

    if (!splitter_position.isNull()) {
        ui.splitter->restoreState(splitter_position);
    }

    settings.endGroup();
}


void AddRoles::WriteSettings()
{
    SaveSelection();
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    settings.setValue("geometry", saveGeometry());
    // The position of the splitter handle
    settings.setValue("splitter", ui.splitter->saveState());
    settings.endGroup();
}
