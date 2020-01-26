/************************************************************************
**
**  Copyright (C) 2019-2020 Kevin B. Hendricks, Stratford Ontario Canada
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
#ifndef SIMPLEPAGE_H
#define SIMPLEPAGE_H

#include <QObject>
#include <QUrl>
#include <QtWebEngineWidgets/QWebEnginePage>

class SimplePage : public QWebEnginePage
{
    Q_OBJECT

public:
    SimplePage(QObject *parent = 0);

};

#endif // SIMPLEPAGE_H

