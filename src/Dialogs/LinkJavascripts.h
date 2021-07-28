/************************************************************************
**
**  Copyright (C) 2021 Kevin B. Hendricks, Stratford, Ontario Canada
**  Copyright (C) 2012 Dave Heiland
**  Copyright (C) 2012 John Schember <john@nachtimwald.com>
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
#ifndef LINKJAVASCRIPTS_H
#define LINKJAVASCRIPTS_H

#include <QtCore/QList>
#include <QtCore/QStringList>
#include <QtWidgets/QDialog>
#include <QtGui/QStandardItemModel>

#include "ui_LinkJavascripts.h"

class LinkJavascripts : public QDialog
{
    Q_OBJECT

public:

    // Constructor;
    // The first parameter is the list of included/excluded stylesheets
    LinkJavascripts(QList<std::pair<QString, bool>> stylesheet_map, QWidget *parent = 0);

    QStringList GetJavascripts();

private slots:

    // Writes all the stored dialog settings
    void WriteSettings();

    void MoveUp();
    void MoveDown();
    void UpdateJavascripts();

private:

    // Updates the display of the tree view (resizes columns etc.)
    void UpdateTreeViewDisplay();

    // Creates the model that is displayed in the tree view
    void CreateJavascriptsModel();

    // Inserts the specified stylesheet into the model
    void InsertJavascriptIntoModel(std::pair<QString, bool> stylesheet);

    // Reads all the stored dialog settings like window position, size, etc.
    void ReadSettings();

    void ConnectSignalsToSlots();


    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    // The model displayed and edited in the tree view
    QStandardItemModel m_JavascriptsModel;

    // The list of javascripts to include/exclude
    QList<std::pair<QString, bool>> m_JavascriptsMap;

    // The new list of javascripts to include
    QStringList m_Javascripts;

    // Holds all the widgets Qt Designer created for us
    Ui::LinkJavascripts ui;
};


#endif // LINKJAVASCRIPTS_H
