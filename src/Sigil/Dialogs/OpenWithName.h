/************************************************************************
**
**  Copyright (C) 2012 John Schember <john@nachtimwald.com>
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
#ifndef OPENWITHNAME_H
#define OPENWITHNAME_H

#include <QtGui/QDialog>

#include "ResourceObjects/Resource.h"
#include "BookManipulation/Book.h"

#include "ui_OpenWithName.h"

class OpenWithName: public QDialog
{
    Q_OBJECT

public:
    OpenWithName(QString filename, QWidget *parent = 0);

    QString GetName();

private slots:
    void SetName();

private:

    void connectSignalsSlots();

    QString m_Filename;
    QString m_MenuName;


    Ui::OpenWithName ui;
};

#endif // OPENWITHNAME_H
