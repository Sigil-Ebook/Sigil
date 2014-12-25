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

/**
 * The dialog used to setup and run content cleaning process.
 */
class CleanContent : public QDialog
{
    Q_OBJECT

public:
    CleanContent(QWidget *parent);

    bool IsJoinParagraphsSelected();

public slots:

signals:

protected:

protected slots:

private slots:

private:
    Ui::CleanContent ui;
};

#endif // CLEANCONTENT_H

