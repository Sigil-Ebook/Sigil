/************************************************************************
**
**  Copyright (C) 2015-2020  Kevin B. Hendricks, Stratford Ontario Canada
**  Copyright (C) 2016-2020  Doug Massay
**  Copyright (C) 2011-2013  John Schember <john@nachtimwald.com>
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
#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QtWidgets/QDialog>

#include "ui_Preferences.h"
#include "PreferenceWidgets/PreferencesWidget.h"

/**
 * Allows the user to change settings related to how the application functions.
 *
 * The preferences exposed are instances of PreferencesWidget. They are loaded
 * and dynamically displayed based upon which one is selected.
 */
class Preferences : public QDialog
{
    Q_OBJECT

public:

    enum AvailablePreferences {
        AppearancePrefs        = 0,
        GeneralSettingsPrefs   = 1,
        KeyboardShortcutsPrefs = 2,
        LanguagePrefs          = 3,
        SpellCheckPrefs        = 4,
        PreserveEntitiesPrefs  = 5,
        PluginsPrefs           = 6
    };



    Preferences(QWidget *parent = 0);
    /**
     * Check this after dialog closes to determine if spelling highlighting needs reapplying.
     */
    bool isRefreshSpellingHighlightingRequired();
    /**
     * Check this after dialog closes to determine if tabs need reopening.
     */
    bool isReloadTabsRequired();
    /**
     * Check this after dialog closes to determine if Sigil needs restarting.
     */
    bool isRestartRequired();
    /**
     * Check this after dialog closes to determine if clipboard history saving limit needs updated.
     */
    bool isRefreshClipHistoryLimitRequired();
    /**
     * Check this after dialog closes to determine if BookBrowser needs to be refreshed
     */
    bool isRefreshBookBrowserRequired();
    /**
     * Check this after dialog closes to determine if Preview page needs to be reloaded
     */
    bool isReloadPreviewRequired();

    void makeActive(int);

private slots:
    /**
     * Load the PreferencesWidget that the user has selected.
     */
    void selectPWidget(QListWidgetItem *current, QListWidgetItem *previous = 0);
    /**
     * Saves settings the user has selected.
     *
     * Saves the state of the dialog.
     * Also, calls saveSettings for each loaded PreferencesWidget.
     */
    void saveSettings();

    void openPreferencesLocation();

private:
    /**
     * Read settings for the dialog.
     *
     * Each PreferencesWidget upon creation will load it's own settings.
     */
    void readSettings();
    /**
     * Adds the given preferences with to the dialog.
     *
     * The widget is added to the list of available widgets and when the
     * entry in the list is selected the widget it shown in the widget display
     * area to the right of the available widget list.
     *
     * @param widget The PreferencesWidget to add to the dialog.
     */
    void appendPreferenceWidget(PreferencesWidget *widget);

    void extendUI();
    /**
     * Connect signals to slots used by this dialog.
     */
    void connectSignalsSlots();

    bool m_refreshSpellingHighlighting;
    bool m_reloadTabs;
    bool m_restartSigil;
    bool m_refreshClipHistoryLimit;
    bool m_refreshBookBrowser;
    bool m_reloadPreview;

    Ui::Preferences ui;
};

#endif // PREFERENCES_H
