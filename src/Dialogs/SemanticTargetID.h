/************************************************************************
**
**  Copyright (C) 2024 Kevin B. Hendricks, Stratford Ontario Canada
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
#ifndef SEMANTICTARGETID_H
#define SEMANTICTARGETID_H

#include <QDialog>

#include "ResourceObjects/Resource.h"
#include "ResourceObjects/HTMLResource.h"
#include "ui_SemanticTargetID.h"

class SemanticTargetID: public QDialog
{
    Q_OBJECT

public:
    SemanticTargetID(HTMLResource *html_resource, QWidget *parent = 0);

    void SetList();

    QString GetID();

private slots:
    void WriteSettings();

private:
    void SetSelectedText();

    void ReadSettings();
    void connectSignalsSlots();

    QString m_SelectedText;

    HTMLResource *m_HTMLResource;

    Ui::SemanticTargetID ui;
};

#endif // SEMANTICTARGETID_H
