/************************************************************************
**
**  Copyright (C) 2019 Kevin B. Hendricks, Stratford, Ontario Canada
**  Copyright (C) 2013 Dave Heiland
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
#ifndef VIEWIMAGE_H
#define VIEWIMAGE_H

#include <QtWidgets/QDialog>

#include "Misc/SettingsStore.h"
#include "ResourceObjects/Resource.h"
#include "ui_ViewImage.h"

class QWebEngineView;

class ViewImage : public QDialog
{
    Q_OBJECT

public:
    ViewImage(QWidget *parent = 0);
    ~ViewImage();

    void ShowImage(QString path);

private slots:
    void WriteSettings();

private:
    void ReadSettings();

    Ui::ViewImage ui;
};

#endif // VIEWIMAGE_H
