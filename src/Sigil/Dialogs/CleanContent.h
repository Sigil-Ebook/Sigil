/************************************************************************
**
**  Copyright (C) 2014 Marek Gibek
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
#ifndef CLEANCONTENT_H
#define CLEANCONTENT_H

#include <QtWidgets/QDialog>

#include "ui_CleanContent.h"
#include "BookManipulation/Book.h"
#include "MainUI/MainWindow.h"
#include "SourceUpdates/CleanContentUpdates.h"

class MainWindow;

/**
 * The dialog used to setup and run content cleaning process.
 */
class CleanContent : public QDialog
{
    Q_OBJECT

public:
    CleanContent(MainWindow &main_window);

    /**
     * Defines possible areas where the search can be performed.
     */
    enum LookWhere {
        LookWhere_CurrentFile = 0,
        LookWhere_AllHTMLFiles = 10,
        LookWhere_SelectedHTMLFiles = 20,
    };

    void ForceClose();
    void SetBook(QSharedPointer <Book> book);

    CleanContentParams GetParams();

public slots:

signals:

protected:

protected slots:
    void showEvent(QShowEvent *event);

private slots:
    void Execute();
    void Save();

private:
    Ui::CleanContent ui;

    MainWindow &m_MainWindow;

    QSharedPointer<Book> m_Book;

    void ConnectSignalsSlots();

    void SetLookWhere(int look_where);
    CleanContent::LookWhere GetLookWhere();

    QList <HTMLResource *> GetHTMLFiles();
};

#endif // CLEANCONTENT_H

