/************************************************************************
**
**  Copyright (C) 2019 Kevin B. Hendricks, Stratford Ontario, Canada
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
#ifndef RETABLE_H
#define RETABLE_H

#include <QDialog>
#include <QList>
#include "ui_RETable.h"

class Resource;

class RETable: public QDialog
{
    Q_OBJECT

public:
    RETable(const QList<Resource *> &resources, QString &retext, QString &replacetext, QWidget *parent = 0);

    void SetTable(const QList<Resource *> &resources);

    QStringList GetNewNames();

private slots:
    void WriteSettings();

private:
    void SetResults();
    void ReadSettings();
    void connectSignalsSlots();
    QString m_REText;
    QString m_ReplaceText;
    QStringList m_NewNames;
    QStringList newnames;

    Ui::RETable ui;
};

#endif // RETABLE_H
