/************************************************************************
**
**  Copyright (C) 2011, 2012, 2013  John Schember <john@nachtimwald.com>
**  Copyright (C) 2012, 2013  Dave Heiland
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

#include <QColor>
#include <QtCore/QSettings>
#include <QtCore/QString>
#include <utility>

#define CLEANON_OPEN         (1 << 0)
#define CLEANON_SAVE         (1 << 1)

class QColor;

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
        CleanLevel_Off             = 0,
        CleanLevel_PrettyPrintBS4  = 100,
        CleanLevel_PrettyPrint     = 160,
        CleanLevel_BS4             = 200
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
    float zoomPreview();

    /**
     * The name of the dictionary to use for spell check.
     *
     * @return The dictionary name.
     */
    QString dictionary();

    QStringList enabledUserDictionaries();

    /**
     * Get the list of entities/code pairs to preserve
     */

    QList<std::pair <ushort, QString>>  preserveEntityCodeNames();

    /**
     * Support for Plugins
     */
    QHash <QString, QString> pluginEnginePaths();
    QString pluginLastFolder();


    /**
     * Whether automatic Spellcheck is enabled or not
     *
     * @return if spell check is enabled
     */
    bool spellCheck();

    int viewState();

    /**
     * The name of the file containing user words
     *
     * @return The dictionary name.
     */
    QString defaultUserDictionary();

    /**
     * The template name for renaming selections in book browser
     *
     * @return The template name.
     */
    QString renameTemplate();

    SettingsStore::CleanLevel cleanLevel();

    int cleanOn();

    /**
     * All appearance settings related to BookView.
     */
    struct BookViewAppearance {
        QString font_family_standard;
        QString font_family_serif;
        QString font_family_sans_serif;
        int font_size;
    };

    /**
     * All appearance settings related to CodeView.
     */
    struct CodeViewAppearance {
        QString font_family;
        int font_size;

        QColor background_color;
        QColor foreground_color;

        QColor selection_background_color;
        QColor selection_foreground_color;

        QColor css_comment_color;
        QColor css_property_color;
        QColor css_quote_color;
        QColor css_selector_color;
        QColor css_value_color;

        QColor line_highlight_color;
        QColor line_number_background_color;
        QColor line_number_foreground_color;

        QColor spelling_underline_color;

        QColor xhtml_attribute_name_color;
        QColor xhtml_attribute_value_color;
        QColor xhtml_css_color;
        QColor xhtml_css_comment_color;
        QColor xhtml_doctype_color;
        QColor xhtml_entity_color;
        QColor xhtml_html_color;
        QColor xhtml_html_comment_color;
    };

    /**
     * All appearance settings related to Special Characters.
     */
    struct SpecialCharacterAppearance {
        QString font_family;
        int font_size;
    };


    /**
     * The default font to use for rendering Book View/Preview.
     */
    BookViewAppearance bookViewAppearance();

    /**
     * The appearance settings to use for editing in Code View.
     */
    CodeViewAppearance codeViewAppearance();

    /**
     * The appearance settings to use for editing in Code View.
     */
    SpecialCharacterAppearance specialCharacterAppearance();

    /**
     * Clear all Book View, Code View and Special Characters settings back to their defaults.
     */
    void clearAppearanceSettings();

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
    void setZoomPreview(float zoom);

    /**
     * Set the name of the dictionary the user has selected.
     *
     * @param name The name of the dictionary.
     */
    void setDictionary(const QString &name);

    void setEnabledUserDictionaries(const QStringList name);

    /**
     * Set the list of paired code, entity strings to preserve.
     */

    void setPreserveEntityCodeNames(const QList<std::pair <ushort, QString>>  codenames);


    /**
     * Support for Plugins
     */

    void setPluginEnginePaths(const QHash <QString, QString> &enginepaths);
    void setPluginLastFolder(const QString &lastfolder);

    /**
     * Set whether automatic Spellcheck is enabled
     *
     * @param name The name of the dictionary.
     */
    void setSpellCheck(bool enabled);

    void setViewState(int state);

    /**
     * Set the name of the dictionary file to store user words.
     *
     * @param name The name of the dictionary file.
     */
    void setDefaultUserDictionary(const QString &name);

    /**
     * Set the name of the dictionary the user has selected.
     *
     * @param name The name of the dictionary.
     */
    void setRenameTemplate(const QString &name);

    void setCleanLevel(SettingsStore::CleanLevel level);

    void setCleanOn(int on);

    /**
     * Set the default font settings to use for rendering Book View/Preview
     */
    void setBookViewAppearance(const BookViewAppearance &book_view_appearance);
    /**
     * Set the appearance settings to use for editing in Code View
     */
    void setCodeViewAppearance(const CodeViewAppearance &code_view_appearance);
    /**
    * Set the default font settings to use for Special Characters popup window
    */
    void setSpecialCharacterAppearance(const SpecialCharacterAppearance &special_character_appearance);

private:
    /**
     * Ensures there is not an open settings group which will cause the settings
     * this class implements to be set in the wrong place.
     */
    void clearSettingsGroup();
};

#endif // SETTINGSSTORE_H
