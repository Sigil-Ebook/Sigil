/************************************************************************
**
**  Copyright (C) 2011  John Schember <john@nachtimwald.com>
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
#ifndef REPORTSWIDGET_H
#define REPORTSWIDGET_H

#include <QWidget>

#include "ResourceObjects/Resource.h"
#include "BookManipulation/Book.h"

/**
 * Base Interface for reports widgets.
 */
class ReportsWidget : public QWidget
{
    Q_OBJECT

public:
    virtual void CreateTable(QList<Resource*> html_resources, QList<Resource*> image_resources, QList<Resource*> css_resources, QSharedPointer< Book > book) = 0;
};

#endif // REPORTSWIDGET_H
