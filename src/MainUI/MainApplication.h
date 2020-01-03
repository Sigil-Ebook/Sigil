/************************************************************************
**
**  Copyright (C) 2019 Kevin B. Hendricks, Stratford, Ontario Canada
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
#include <QtWidgets/QApplication>

class QStyle;

class MainApplication : public QApplication
{
    Q_OBJECT

public:
    MainApplication(int &argc, char **argv);

    bool isDarkMode() { return m_isDark; }

signals:
    void applicationActivated();
    void applicationDeactivated();
    void applicationPaletteChanged();

public slots:
    void EmitPaletteChanged();

protected:
    bool event(QEvent *pEvent);

private:
    QStyle * m_Style;
    bool m_isDark;
};

#endif // MAINAPPLICATION_H


