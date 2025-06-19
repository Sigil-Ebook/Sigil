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

#ifndef ADDROLES_H
#define ADDROLES_H

#include <QString>
#include <QStringList>
#include <QDialog>

#include "ui_AddRoles.h"

/**
 * The dialog for selecting from a list of available 
 * Role properties that can be applied to the current tag
 * Used to improve Accessibility by adding Aria roles (and mapped epub:types)
 */

class AddRoles : public QDialog
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param infomap  QHash map used to generate the list of properties this dialog displays
     * @param parent The dialog's parent.
     */
    AddRoles(const QString& current_tag, QWidget *parent = 0);

    /**
     * Returns the list of names selected by user.
     */
    QStringList GetSelectedEntries();

private slots:

    /**
     * Updates the description of the currently selected item
     * whenever the user selects a new item.
     *
     * @param current The currently selected item.
     */
    void UpdateDescription(QListWidgetItem *current);

    /**
     * Saves the selected names for later retrieval
     */
    void SaveSelection();

    /**
     * Writes all the stored application settings like
     * window position, geometry etc.
     */
    void WriteSettings();

private:

    /**
     * Reads all the stored application settings like
     * window position, geometry etc.
     */
    void ReadSettings();

    /** 
     * Generates a reverse mapping from Name to Code
     */
    QHash<QString, QString> m_Name2Code;

    /**
     * Holds the names of the selected entries
     */
    QStringList m_SelectedEntries;

    QString m_currentTag;

    /**
     * Holds all the widgets Qt Designer created for us.
     */
    Ui::AddRoles ui;
};

#endif // ADDROLES_H


