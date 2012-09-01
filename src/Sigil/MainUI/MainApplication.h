/************************************************************************
**
**  Copyright (C) 2012 John Schember <john@nachtimwald.com>
**  Copyright (C) 2012 Grant Drake
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
#ifndef MAINAPPLICATION_H
#define MAINAPPLICATION_H

#include <QEvent>
#include <QtGui/QApplication>

class MainApplication : public QApplication
{
    Q_OBJECT

public:
    MainApplication( int argc, char * argv[] );

signals:
    void applicationActivated();
    void applicationDeactivated();

protected:
    bool event( QEvent *pEvent );
};

#endif // MAINAPPLICATION_H


