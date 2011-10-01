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

#include <stdafx.h>
#include "AppearanceWidget.h"
#include "Misc/SettingsStore.h"

static const QString SETTINGS_GROUP = "user_preferences";

AppearanceWidget::AppearanceWidget()
{
    ui.setupUi(this);

    readSettings();
}

void AppearanceWidget::saveSettings()
{
    SettingsStore *store = SettingsStore::instance();

    Qt::Orientation orientation = Qt::Vertical;
    if (ui.svHorizontal->isChecked()) {
        orientation = Qt::Horizontal;
    }
    store->setSplitViewOrientation(orientation);
}

void AppearanceWidget::readSettings()
{
    SettingsStore *store = SettingsStore::instance();

    Qt::Orientation orientation = store->splitViewOrientation();
    if (orientation == Qt::Horizontal) {
        ui.svHorizontal->setChecked(true);
    }
}
