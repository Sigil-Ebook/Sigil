/************************************************************************
**
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
#ifndef LINKSTYLESHEETS_H
#define LINKSTYLESHEETS_H

#include <QtCore/QList>
#include <QtCore/QStringList>
#include <QtGui/QDialog>
#include <QtGui/QStandardItemModel>

#include "ui_LinkStylesheets.h"

class LinkStylesheets : public QDialog
{
    Q_OBJECT

public:

    // Constructor;
    // The first parameter is the list of included/excluded stylesheets
    LinkStylesheets(QList<std::pair<QString, bool> > stylesheet_map, QWidget *parent = 0);

    QStringList GetStylesheets();

private slots:

    // Writes all the stored dialog settings
    void WriteSettings();

    void MoveUp();
    void MoveDown();
    void UpdateStylesheets();

private:

    // Updates the display of the tree view (resizes columns etc.)
    void UpdateTreeViewDisplay();

    // Creates the model that is displayed in the tree view
    void CreateStylesheetsModel();

    // Inserts the specified stylesheet into the model
    void InsertStylesheetIntoModel(std::pair<QString, bool> stylesheet);

    // Reads all the stored dialog settings like window position, size, etc.
    void ReadSettings();

    void ConnectSignalsToSlots();


    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    // The model displayed and edited in the tree view
    QStandardItemModel m_StylesheetsModel;

    // The list of stylesheets to include/exclude
    QList< std::pair<QString, bool> > m_StylesheetsMap;

    // The new list of stylesheets to include
    QStringList m_Stylesheets;

    // Holds all the widgets Qt Designer created for us
    Ui::LinkStylesheets ui;
};


#endif // LINKSTYLESHEETS_H
