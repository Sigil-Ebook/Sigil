/************************************************************************
**
**  Copyright (C) 2020 Kevin B. Hendricks, Stratford, ON, Canada
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
#ifndef SELECTCHECKPOINT_H
#define SELECTCHECKPOINT_H

#include <QString>
#include <QStringList>
#include <QtWidgets/QDialog>

#include "ui_SelectCheckpoint.h"

/**
 * The dialog for selecting from a list of available 
 * checkpoint tags.  Each tag includes some descriptive text
 */
class SelectCheckpoint : public QDialog
{
    Q_OBJECT

public:

    SelectCheckpoint(const QStringList & checkpointlst, QWidget *parent = 0);

    QStringList GetSelectedEntries();

signals:

    /**
     * Emitted when the user has selected what he wants to insert.
     * @param name The name of the semantics the user has selected
     */
    void CheckpointSelected(QStringList checkpoints);

private slots:

    void UpdateDescription(QListWidgetItem *current);
    void ClearDescription();
    void SaveSelection();

    void WriteSettings();

private:

    void ReadSettings();

    QHash<QString, QString> m_CheckpointInfo;

    QStringList m_SelectedEntries;

    Ui::SelectCheckpoint ui;
};

#endif // SELECTCHECKPOINT_H
