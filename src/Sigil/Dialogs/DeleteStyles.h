/************************************************************************
**
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
#ifndef DELETESTYLES_H
#define DELETESTYLES_H

#include <QtGui/QDialog>
#include <QtGui/QStandardItemModel>
#include "Misc/CSSInfo.h"

#include "ui_DeleteStyles.h"

class DeleteStyles: public QDialog
{
    Q_OBJECT

public:

    /**
     * Constructor.
     * 
     * @param opf The OPF whose metadata we want to edit.
     * @param parent The object's parent.
     */
    DeleteStyles(QHash< QString, QList<CSSInfo::CSSSelector *> > css_styles_to_delete, QWidget *parent = 0 );
    ~DeleteStyles();

    QHash< QString, QList<CSSInfo::CSSSelector *> > GetStylesToDelete();

signals:
    void OpenFileRequest(QString, int);

private slots:
    void SaveStylesToDelete();
    void WriteSettings();
    void DoubleClick();

private:
    void SetUpTable();	

    void ReadSettings();

    void ConnectSignals();

    QStandardItemModel m_Model;

    QHash< QString, QList<CSSInfo::CSSSelector *> > m_CSSStylesToDelete;

    Ui::DeleteStyles ui;
};

#endif // DELETESTYLES_H


