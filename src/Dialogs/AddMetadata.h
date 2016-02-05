/************************************************************************
**
**  Copyright (C) 2009, 2010, 2011  Strahinja Markovic  <strahinja.markovic@gmail.com>
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
#ifndef ADDMETADATA_H
#define ADDMETADATA_H

#include <QtWidgets/QDialog>

#include "ui_AddMetadata.h"
#include "BookManipulation/Metadata.h"


/**
 * The dialog for adding a single metadata element.
 * Displays a list of available metadata entries with a
 * detailed description for each. Used for adding basic
 * Dublin Core metadata elements and also all versions
 * of creators and contributors.
 */
class AddMetadata : public QDialog
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param metadata The metadata list that this dialog displays. \see Metadata
     * @param parent The dialog's parent.
     */
  AddMetadata(Metadata* mdp, QString infotype, QWidget *parent = 0);

    /**
     * Returns the list of names selected by user.
     */
    QStringList GetSelectedEntries();

signals:

    /**
     * Emitted when the user has selected what he wants to insert.
     *
     * @param name The name of the metadata the user
     *             wants inserted into the table.
     */
    void MetadataToAdd(QStringList metadata);

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
     * stores metadata instance for workign with basic and roles
     */
    Metadata* m_Metadata;
    QString   m_InfoType;

    /**
     * Holds the names of the selected entries
     */
    QStringList m_SelectedEntries;

    /**
     * Holds all the widgets Qt Designer created for us.
     */
    Ui::AddMetadata ui;
};

#endif // ADDMETADATA_H


