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
#include <QScrollArea>
#include "Preferences.h"
#include "PreferenceWidgets/LanguageWidget.h"
#include "PreferenceWidgets/AppearanceWidget.h"

static const QString SETTINGS_GROUP = "preferences_dialog";

Preferences::Preferences(QWidget *parent) :
    QDialog(parent)
{
    ui.setupUi(this);

    // Create and load all of our preference widgets.;
    m_pWidgets.append(new AppearanceWidget);
    m_pWidgets.append(new LanguageWidget);

    // Populate the list of avaliable preference groups with the names
    // of each widgets.
    foreach (PreferencesWidget *w, m_pWidgets) {
        ui.availableWidgets->addItem(w->windowTitle());
    }

    // Select the first item in the preferences list.
    if (m_pWidgets.length() > 0) {
        ui.availableWidgets->item(0)->setSelected(true);
        selectPWidget(ui.availableWidgets->item(0));
    }

    readSettings();
    connectSignalsSlots();
}

Preferences::~Preferences()
{
    // Clean up all of the preference widgets we created earlier.
    foreach(PreferencesWidget *pw, m_pWidgets) {
        if (pw != 0) {
            delete pw;
        }
    }
}

void Preferences::selectPWidget(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous)

    int index = ui.availableWidgets->row(current);

    // Ensure the selected index exists in our list of widgets.
    if (index >= 0 && index < m_pWidgets.length()) {
        PreferencesWidget *pw = m_pWidgets.at(index);

        // Remove the current widget from the scroll area so it's not deleted
        // when we display the new widget.
        PreferencesWidget *old_pw = qobject_cast<PreferencesWidget*>(ui.pWidget->takeWidget());
        if (old_pw != 0) {
            old_pw->hide();
        }

        // Add the preference widget to the dialog and show it.
        ui.pWidget->setWidget(pw);
        pw->show();
    }
}

void Preferences::saveSettings()
{
    QSettings settings;
    settings.beginGroup( SETTINGS_GROUP );

    settings.setValue("geometry", saveGeometry());

    foreach (PreferencesWidget *pw, m_pWidgets) {
        pw->saveSettings();
    }
}

void Preferences::readSettings()
{
    QSettings settings;
    settings.beginGroup( SETTINGS_GROUP );

    QByteArray geometry = settings.value("geometry").toByteArray();
    if (!geometry.isNull()) {
        restoreGeometry(geometry);
    }
}

void Preferences::connectSignalsSlots()
{
    connect(ui.availableWidgets, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(selectPWidget(QListWidgetItem*, QListWidgetItem*)));
    connect(this, SIGNAL(finished(int)), this, SLOT(saveSettings()));
}
