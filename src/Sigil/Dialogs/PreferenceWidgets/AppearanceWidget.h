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
#ifndef APPEARANCEWIDGET_H
#define APPEARANCEWIDGET_H

#include <QWidget>
#include "PreferencesWidget.h"
#include "ui_PAppearanceWidget.h"

class QString;

/**
 * Preferences widget for appearance related preferences.
 */
class AppearanceWidget : public PreferencesWidget
{
public:
    AppearanceWidget();
    void saveSettings();

private:
    void readSettings();

    Ui::AppearanceWidget ui;
};

#endif // APPEARANCEWIDGET_H
