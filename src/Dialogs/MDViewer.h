/************************************************************************
**
**  Copyright (C) 2024-2026 Kevin B. Hendricks, Stratford Ontario Canada
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
#ifndef MDVIEWER_H
#define MDVIEWER_H

#include <QDialog>
#include <QString>
#include <QTextEdit>
#include "ui_MDViewer.h"

class MDViewer: public QDialog
{
    Q_OBJECT

public:
    MDViewer(const QString & mdsrc,QWidget *parent = 0);

private slots:
    void WriteSettings();

private:
    void ReadSettings();
    void connectSignalsSlots();
    Ui::MDViewer ui;
};

#endif // MDVIEWER_H
