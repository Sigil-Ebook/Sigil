/************************************************************************
**
**  Copyright (C) 2021 Kevin B. Hendricks, Stratford, ON, Canada
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
#ifndef ADDAUTOMATETOOL_H
#define ADDAUTOMATETOOL_H

#include <QString>
#include <QStringList>
#include <QtWidgets/QDialog>

#include "ui_AddAutomateTool.h"
#include "Misc/DescriptiveInfo.h"

/**
 * The dialog for adding a single Automate Tool element.
 * Displays a list of available Automate Tool entries with a
 * detailed description for each.
 */
class AddAutomateTool : public QDialog
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param metadata The automate tool list that this dialog displays. 
     * @param parent The dialog's parent.
     */
  AddAutomateTool(const QHash<QString, DescriptiveInfo> & toolinfo, QWidget *parent = 0);

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
     * Represents the metadata information list that this dialog displays
     * and generates a reverse mapping
     */
    const QHash<QString, DescriptiveInfo> & m_ToolInfo;
    QHash<QString, QString> m_Name2Code;

    /**
     * Holds the names of the selected entries
     */
    QStringList m_SelectedEntries;

    /**
     * Holds all the widgets Qt Designer created for us.
     */
    Ui::AddAutomateTool ui;
};

#endif // ADDAUTOMATETOOL_H


