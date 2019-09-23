/************************************************************************
**
**  Copyright (C) 2015-2019 Kevin B. Hendricks, Stratford Ontario Canada
**  Copyright (C) 2009-2011 Strahinja Markovic  <strahinja.markovic@gmail.com>
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
#ifndef OPFMODELITEM_H
#define OPFMODELITEM_H

#include <limits>

#include <QtCore/Qt>
#include <QtCore/QString>
#include <QtGui/QStandardItem>

static const int NO_READING_ORDER        = std::numeric_limits<int>::max();
static const int READING_ORDER_ROLE      = Qt::UserRole + 2;
static const int ALPHANUMERIC_ORDER_ROLE = Qt::UserRole + 3;

/**
 * A re-implementation of QStandardItem to
 * support sorting alphanumerically
 */
class AlphanumericItem : public QStandardItem
{

public:
    AlphanumericItem();
    AlphanumericItem(QIcon, QString);

private:
    virtual bool operator<(const QStandardItem &item) const;
};

#endif // OPFMODELITEM_H
