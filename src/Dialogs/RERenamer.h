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
#ifndef RERENAMER_H
#define RERENAMER_H

#include <QString>
#include <QtWidgets/QDialog>
#include "ui_RERenamer.h"

class RERenamer: public QDialog
{
    Q_OBJECT

public:
    RERenamer(QWidget *parent = 0);
    QString GetREText();
    QString GetReplaceText();

private slots:
    void WriteSettings();

private:
    void SetREText();
    void SetReplaceText();

    void ReadSettings();
    void connectSignalsSlots();

    QString m_REText;
    QString m_ReplaceText;

    Ui::RERenamer ui;
};

#endif // RERENAMER_H
