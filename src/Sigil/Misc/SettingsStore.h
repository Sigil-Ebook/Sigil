/************************************************************************
**
**  Copyright (C) 2011, 2012  John Schember <john@nachtimwald.com>
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
#ifndef SETTINGSSTORE_H
#define SETTINGSSTORE_H

#include <QtCore/QSettings>
#include <QtCore/QString>

/**
 * Provides access for reading and writing user configurable
 * settings. This should be used instead of QSettings because it
 * sets up the settings to use INI format on all platforms except
 * OS X. Also, it implements a variety of settings that are used in
 * a large number of places throughout the application.
 */
class SettingsStore : public QSettings
{
    Q_OBJECT

public:
    SettingsStore();
    SettingsStore(QString filename);

    enum CleanLevel {
        CleanLevel_Off = 0,
        CleanLevel_PrettyPrint = 100,
        CleanLevel_Tidy = 200
    };

    /**
     * The langauge to use for the user interface
     *
     * @return The language as a string.
     */
    QString uiLanguage();

    /**
     * The default langauge to use when creating new books.
     *
     * @return The language as a string.
     */
    QString defaultMetadataLang();

    /**
     * The zoom factor used by the component.
     *
     * @return The zoom factor.
     */
    float zoomImage();
    float zoomText();
    float zoomWeb();

    /**
     * The name of the dictionary to use for spell check.
     *
     * @return The dictionary name.
     */
    QString dictionary();

    /**
     * Whether automatic Spell Check is enabled or not
     *
     * @return if spell check is enabled
     */
    bool spellCheck();

    /**
     * The name of the file containing user words
     *
     * @return The dictionary name.
     */
    QString userDictionaryName();

    /**
     * The template name for renaming selections in book browser
     *
     * @return The template name.
     */
    QString renameTemplate();

    SettingsStore::CleanLevel cleanLevel();

public slots:

    /**
     * Set the language to use for the user interface
     *
     * @param lang The language to set.
     */
    void setUILanguage(const QString &language_code);

    /**
     * Set the default language to use when creating new books.
     *
     * @param lang The language to set.
     */
    void setDefaultMetadataLang(const QString &lang);

    /**
     * Set the zoom factor used by the component.
     *
     * @param zoom The zoom factor.
     */
    void setZoomImage(float zoom);
    void setZoomText(float zoom);
    void setZoomWeb(float zoom);

    /**
     * Set the name of the dictionary the user has selected.
     *
     * @param name The name of the dictionary.
     */
    void setDictionary(const QString &name);

    /**
     * Set whether automatic Spell Check is enabled
     *
     * @param name The name of the dictionary.
     */
    void setSpellCheck(bool enabled);

    /**
     * Set the name of the dictionary file to store user words.
     *
     * @param name The name of the dictionary file.
     */
    void setUserDictionaryName(const QString &name);

    /**
     * Set the name of the dictionary the user has selected.
     *
     * @param name The name of the dictionary.
     */
    void setRenameTemplate(const QString &name);

    void setCleanLevel(SettingsStore::CleanLevel level);

private:
    /**
     * Ensures there is not an open settings group which will cause the settings
     * this class implements to be set in the wrong place.
     */
    void clearSettingsGroup();

    /**
     * Return the default directory used to store dictionary files
     */
    QString defaultDictionaryDirectory();
};

#endif // SETTINGSSTORE_H
