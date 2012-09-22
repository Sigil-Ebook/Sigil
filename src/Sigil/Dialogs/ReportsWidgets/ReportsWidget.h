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
#include <QString>

/**
 * Base Interface for reports widgets.
 */
class ReportsWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * Describes the result actions to present to the user as a result
     * of saving any changes made in the preferences widgets.
     * Results are in order of increasing priority of result to display.
     */
    struct Results
    {
        QString filename;
        int line;
        QStringList files_to_delete;
    };

    /**
     * Save settings made available by the widget.
     */
    virtual Results saveSettings() = 0;
};

#endif // REPORTSWIDGET_H
