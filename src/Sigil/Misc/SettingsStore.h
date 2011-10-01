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
#ifndef SETTINGSSTORE_H
#define SETTINGSSTORE_H

#include <QObject>
#include <QString>

/**
 * Singleton. Provides access for reading and writing user configurable
 * settings.
 */
class SettingsStore : public QObject
{
    Q_OBJECT

public:
    /**
     * The accessor function to access the store.
     */
    static SettingsStore *instance();
    ~SettingsStore();

    /**
     * The default langauge to use when creating new books.
     *
     * @return The language as a string.
     */
    QString defaultMetadataLang();
    /**
     * The orientation to use for the split view.
     *
     * @return The orientation.
     */
    Qt::Orientation splitViewOrientation();

    /**
     * Set the default language to use when creating new books.
     *
     * @param lang The language to set.
     */
    void setDefaultMetadataLang(const QString &lang);
    /**
     * Set the orientation of the split view.
     *
     * @param orientation The orientation to set.
     */
    void setSplitViewOrientation(Qt::Orientation orientation);

    /**
     * Causes the store to emit its settingsChanged signal.
     *
     * This is to be used to cause listening objects to reread the settings
     * they use from the cache.
     */
    void triggerSettingsChanged();
    /**
     * Write the stored settings to disk.
     */
    void writeSettings();

signals:
    /**
     * Signals that settings have changed.
     *
     * This is not emitted after a set* function call. This is only emitted
     * by a call from triggerSettingsChanged. This is because we do not want
     * this signal to emit multiple times when a varity of settings are changed
     * via the preferences dialog.
     */
    void settingsChanged();

private:
    /**
     * Private constructor.
     */
    SettingsStore();
    /**
     * Reads settings from disk into the store.
     */
    void readSettings();

    static SettingsStore *m_instance;

    // Cached settings.
    QString m_defaultMetadataLang;
    Qt::Orientation m_splitViewOrientation;
};

#endif // SETTINGSSTORE_H
