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
#ifndef PREFERENCESWIDGET_H
#define PREFERENCESWIDGET_H

#include <QWidget>

class QString;

/**
 * Base Interface for preferences widgets.
 */
class PreferencesWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * Describes the result actions to present to the user as a result
     * of saving any changes made in the preferences widgets.
     * Results are in order of increasing priority of result to display.
     */
    enum ResultAction
    {
        ResultAction_None = 0,       /**< Default, no further action required */
        ResultAction_ReloadTabs,     /**< All tabs need to be reloaded. */
        ResultAction_RestartSigil    /**< Warn user that Sigil needs to be restarted. */
    };

    /**
     * Save settings made available by the widget.
     */
    virtual ResultAction saveSettings() = 0;
};

#endif // PREFERENCESWIDGET_H
