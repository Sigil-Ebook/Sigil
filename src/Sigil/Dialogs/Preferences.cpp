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

#include <QtGui/QScrollArea>

#include "Dialogs/Preferences.h"
#include "Misc/SettingsStore.h"
#include "PreferenceWidgets/AppearanceWidget.h"
#include "PreferenceWidgets/KeyboardShortcutsWidget.h"
#include "PreferenceWidgets/LanguageWidget.h"
#include "PreferenceWidgets/SpellCheckWidget.h"

static const QString SETTINGS_GROUP = "preferences_dialog";

Preferences::Preferences(QWidget *parent) :
    QDialog(parent)
{
    ui.setupUi(this);

    // Create and load all of our preference widgets.;
    appendPreferenceWidget(new AppearanceWidget);
    appendPreferenceWidget(new KeyboardShortcutsWidget);
    appendPreferenceWidget(new LanguageWidget);
    appendPreferenceWidget(new SpellCheckWidget);

    // Ensure the first item in the avaliable preferences widgets list
    // is highlighted.
    ui.availableWidgets->item(0)->setSelected(true);

    readSettings();
    connectSignalsSlots();
}

void Preferences::selectPWidget(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous)

    int index = ui.availableWidgets->row(current);
    ui.pWidget->setCurrentIndex(index);
}

void Preferences::saveSettings()
{
    SettingsStore settings;
    settings.beginGroup( SETTINGS_GROUP );

    settings.setValue("geometry", saveGeometry());

    for (int i = 0; i < ui.pWidget->count(); ++i) {
        PreferencesWidget *pw = qobject_cast<PreferencesWidget*>(ui.pWidget->widget(i));
        if (pw != 0) {
            pw->saveSettings();
        }
    }

    settings.endGroup();
}

void Preferences::readSettings()
{
    SettingsStore settings;
    settings.beginGroup( SETTINGS_GROUP );

    QByteArray geometry = settings.value("geometry").toByteArray();
    if (!geometry.isNull()) {
        restoreGeometry(geometry);
    }

    settings.endGroup();
}

void Preferences::appendPreferenceWidget(PreferencesWidget *widget)
{
    // Add the PreferencesWidget to the stack view area.
    ui.pWidget->addWidget(widget);

    // Add an entry to the list of avaliable preference widgets.
    ui.availableWidgets->addItem(widget->windowTitle());
}

void Preferences::connectSignalsSlots()
{
    connect(ui.availableWidgets, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(selectPWidget(QListWidgetItem*, QListWidgetItem*)));
    connect(this, SIGNAL(finished(int)), this, SLOT(saveSettings()));
}
