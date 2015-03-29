/************************************************************************
**
**  Copyright (C) 2011  John Schember <john@nachtimwald.com>
**  Copyright (C) 2012  Dave Heiland
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
#ifndef CLEANSOURCEWIDGET_H
#define CLEANSOURCEWIDGET_H

#include "PreferencesWidget.h"
#include "Misc/SettingsStore.h"
#include "ui_PCleanSourceWidget.h"

/**
 * Preferences widget for clean source code related items
 */
class CleanSourceWidget : public PreferencesWidget
{
public:
    CleanSourceWidget();
    PreferencesWidget::ResultAction saveSettings();

private:
    void readSettings();

    Ui::CleanSourceWidget ui;
};

#endif // CLEANSOURCEWIDGET_H
