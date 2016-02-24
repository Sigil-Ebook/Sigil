/************************************************************************
**
**  Copyright (C) 2016 Kevin B. Hendricks, Stratford, ON, Canada
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
#ifndef ADDSEMANTICS_H
#define ADDSEMANTICS_H

#include <QString>
#include <QStringList>
#include <QtWidgets/QDialog>

#include "ui_AddSemantics.h"
#include "Misc/DescriptiveInfo.h"

/**
 * The dialog for selecting from a list of available 
 * Semantic properties.  Used for Nav Landmarks, OPF Guide Entries
 * and for adding semantics epub:type attributes to section and other elements
 */
class AddSemantics : public QDialog
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param infomap  QHash map used to generate the list of properties this dialog displays
     * @param parent The dialog's parent.
     */
  AddSemantics(const QHash<QString, DescriptiveInfo> & infomap, QWidget *parent = 0);

    /**
     * Returns the list of names selected by user.
     */
    QStringList GetSelectedEntries();

signals:

    /**
     * Emitted when the user has selected what he wants to insert.
     *
     * @param name The name of the semantics the user has selected
     *             
     */
    void SemanticsToAdd(QStringList semantics);

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
     * Represents the semantics property  information list that this dialog displays
     * and generates a reverse mapping
     */
    const QHash<QString, DescriptiveInfo> & m_SemanticsInfo;
    QHash<QString, QString> m_Name2Code;

    /**
     * Holds the names of the selected entries
     */
    QStringList m_SelectedEntries;

    /**
     * Holds all the widgets Qt Designer created for us.
     */
    Ui::AddSemantics ui;
};

#endif // ADDSEMANTICS_H


