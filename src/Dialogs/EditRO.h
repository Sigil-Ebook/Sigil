/************************************************************************
**
**  Copyright (C) 2016-2026 Kevin B. Hendricks, Stratford, Ontario, Canada
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
#ifndef EDITRO_H
#define EDITRO_H

#include <QString>
#include <QStringList>
#include <QDialog>
#include <QListWidget>
#include <QListWidgetItem>
#include <QAction>



#include "ui_EditRO.h"

class EditRO : public QDialog
{
    Q_OBJECT

public:

    EditRO(QStringList bookpaths, QString selected = QString(), QWidget *parent = 0);

    ~EditRO();

    QStringList GetNewReadingOrder();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void Save();
    void ToTop();
    void ToBottom();
    void MoveUp();
    void MoveDown();
    void SetSelectedRow(int row);
    
private:
    void ReadSettings();
    void WriteSettings();

    void ConnectSignalsToSlots();

    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    QStringList m_BookPaths;
    bool m_updated = false;

    Ui::EditRO ui;
};

#endif // EDITRO_H
