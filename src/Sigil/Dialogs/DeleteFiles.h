/************************************************************************
**
**  Copyright (C) 2012 Dave Heiland
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
#ifndef DELETEFILES_H
#define DELETEFILES_H

#include <QtWidgets/QDialog>
#include <QtGui/QStandardItemModel>

#include "ui_DeleteFiles.h"

class DeleteFiles: public QDialog
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param opf The OPF whose metadata we want to edit.
     * @param parent The object's parent.
     */
    DeleteFiles(QStringList files_to_delete, QWidget *parent = 0);
    ~DeleteFiles();

    QStringList GetFilesToDelete();

signals:
    void OpenFileRequest(QString, int);

private slots:
    void SaveFilesToDelete();
    void WriteSettings();
    void DoubleClick(const QModelIndex index);

private:
    void SetUpTable();

    void ReadSettings();

    void ConnectSignals();

    QStandardItemModel m_Model;

    QStringList m_FilesToDelete;

    Ui::DeleteFiles ui;
};

#endif // DELETEFILES_H


