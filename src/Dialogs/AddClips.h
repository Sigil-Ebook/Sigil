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

#ifndef ADDCLIPS_H
#define ADDCLIPS_H

#include <QString>
#include <QStringList>
#include <QDialog>

#include "ui_AddClips.h"

/**
 * The dialog for selecting from a list of available 
 * Role properties that can be applied to the current tag
 * Used to improve Accessibility by adding Aria roles (and mapped epub:types)
 */

class AddClips : public QDialog
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param infomap  QHash map used to generate the list of properties this dialog displays
     * @param parent The dialog's parent.
     */
    AddClips(const QString& selected_text, const QString& book_lang, QWidget *parent = 0);

    /**
     * Returns the list of clips selected by user.
     */
    QString GetSelectedClip();

private slots:

    /**
     * Updates the description of the currently selected item
     * whenever the user selects a new item.
     *
     * @param current The currently selected item.
     */
    void UpdateDescription(QListWidgetItem *current);

    /**
     * Saves the selected clips for later retrieval
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
     * Holds the clips of the selected entries
     */
    QStringList m_SelectedEntries;

    QString m_selected_text;

    QString m_book_lang;

    /**
     * Holds all the widgets Qt Designer created for us.
     */
    Ui::AddClips ui;
};

#endif // ADDCLIPS_H
