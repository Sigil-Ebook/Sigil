/************************************************************************
**
**  Copyright (C) 2009  Strahinja Markovic
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
#ifndef FINDREPLACE_H
#define FINDREPLACE_H

#include <QtGui/QDialog>
#include "ui_FindReplace.h"


class FindReplace : public QDialog
{
    Q_OBJECT

public:

    // Constructor;
    // the first argument is the widget's parent.
    FindReplace( QWidget *parent = 0 );

    // Destructor
    ~FindReplace();

private slots:

    void ToggleMoreLess();

    void TabChanged();

private:

    void ToFindTab();

    void ToReplaceTab();

    // Reads all the stored dialog settings like
    // window position, geometry etc.
    void ReadSettings();

    // Writes all the stored dialog settings like
    // window position, geometry etc.
    void WriteSettings();

    // Qt Designer is not able to create all the widgets
    // we want in our dialog, so we use this function
    // to extend the UI created by the Designer
    void ExtendUI();


    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    bool m_isMore;

    // Holds all the widgets Qt Designer created for us
    Ui::FindReplace ui;
};

#endif // FINDREPLACE_H


