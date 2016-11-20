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

#include <QtCore/QLocale>
#include <QtCore/QCoreApplication>
#include <QFile>
#include <QDir>

#include "Misc/SettingsStore.h"
#include "Misc/PluginDB.h"
#include "Misc/Utility.h"

#include "sigil_constants.h"

static QString SETTINGS_GROUP = "user_preferences";
static QString KEY_DEFAULT_METADATA_LANGUAGE = SETTINGS_GROUP + "/" + "default_metadata_lang";
static QString KEY_UI_LANGUAGE = SETTINGS_GROUP + "/" + "ui_language";
static QString KEY_ZOOM_IMAGE = SETTINGS_GROUP + "/" + "zoom_image";
static QString KEY_ZOOM_TEXT = SETTINGS_GROUP + "/" + "zoom_text";
static QString KEY_ZOOM_WEB = SETTINGS_GROUP + "/" + "zoom_web";
static QString KEY_ZOOM_PREVIEW = SETTINGS_GROUP + "/" + "zoom_preview";
static QString KEY_RENAME_TEMPLATE = SETTINGS_GROUP + "/" + "rename_template";
static QString KEY_DICTIONARY_NAME = SETTINGS_GROUP + "/" + "dictionary_name";
static QString KEY_VIEW_STATE = SETTINGS_GROUP + "/" + "view_state";
static QString KEY_SPELL_CHECK = SETTINGS_GROUP + "/" + "spell_check";
static QString KEY_LOAD_DICTS_FROM_LAST_SESSION = SETTINGS_GROUP + "/" + "load_dictionaries_from_last_session";
static QString KEY_UNLOAD_CURR_DICTS = SETTINGS_GROUP + "/" + "unload_current_dictionaries";
static QString KEY_LOAD_MAIN_LANG_DICT = SETTINGS_GROUP + "/" + "load_main_language_dictionary";
static QString KEY_LOAD_ALL_LANG_DICTS = SETTINGS_GROUP + "/" + "load_all_languages_dictionaries";
static QString KEY_DEFAULT_USER_DICTIONARY = SETTINGS_GROUP + "/" + "user_dictionary_name";
static QString KEY_ENABLED_USER_DICTIONARIES = SETTINGS_GROUP + "/" + "enabled_user_dictionaries";
static QString KEY_PLUGIN_USER_MAP = SETTINGS_GROUP + "/" + "plugin_user_map";
static QString KEY_CLEAN_ON = SETTINGS_GROUP + "/" + "clean_on";
static QString KEY_REMOTE_ON = SETTINGS_GROUP + "/" + "remote_on";
static QString KEY_DEFAULT_VERSION = SETTINGS_GROUP + "/" + "default_version";
static QString KEY_PRESERVE_ENTITY_NAMES = SETTINGS_GROUP + "/" + "preserve_entity_names";
static QString KEY_PRESERVE_ENTITY_CODES = SETTINGS_GROUP + "/" + "preserve_entity_codes";

static QString KEY_PLUGIN_INFO = SETTINGS_GROUP + "/" + "plugin_info";
static QString KEY_PLUGIN_ENGINE_PATHS = SETTINGS_GROUP + "/" + "plugin_engine_paths";
static QString KEY_PLUGIN_LAST_FOLDER = SETTINGS_GROUP + "/" + "plugin_add_last_folder";
static QString KEY_PLUGIN_USE_BUNDLED_INTERP = SETTINGS_GROUP + "/" + "plugin_use_bundled_interp";

static QString KEY_CSS_EPUB2_VALIDATION_SPEC = SETTINGS_GROUP + "/" + "css_epub2_validation_spec";
static QString KEY_CSS_EPUB3_VALIDATION_SPEC = SETTINGS_GROUP + "/" + "css_epub3_validation_spec";

static QString KEY_TEMP_FOLDER = SETTINGS_GROUP + "/" + "temp_folder_path";

static QString KEY_BOOK_VIEW_FONT_FAMILY_STANDARD = SETTINGS_GROUP + "/" + "book_view_font_family_standard";
static QString KEY_BOOK_VIEW_FONT_FAMILY_SERIF = SETTINGS_GROUP + "/" + "book_view_font_family_serif";
static QString KEY_BOOK_VIEW_FONT_FAMILY_SANS_SERIF = SETTINGS_GROUP + "/" + "book_view_font_family_sans_serif";
static QString KEY_BOOK_VIEW_FONT_SIZE = SETTINGS_GROUP + "/" + "book_view_font_size";

static QString KEY_CODE_VIEW_BACKGROUND_COLOR = SETTINGS_GROUP + "/" + "code_view_background_color";
static QString KEY_CODE_VIEW_FOREGROUND_COLOR = SETTINGS_GROUP + "/" + "code_view_foreground_color";
static QString KEY_CODE_VIEW_CSS_COMMENT_COLOR = SETTINGS_GROUP + "/" + "code_view_css_comment_color";
static QString KEY_CODE_VIEW_CSS_PROPERTY_COLOR = SETTINGS_GROUP + "/" + "code_view_css_property_color";
static QString KEY_CODE_VIEW_CSS_QUOTE_COLOR = SETTINGS_GROUP + "/" + "code_view_css_quote_color";
static QString KEY_CODE_VIEW_CSS_SELECTOR_COLOR = SETTINGS_GROUP + "/" + "code_view_css_selector_color";
static QString KEY_CODE_VIEW_CSS_VALUE_COLOR = SETTINGS_GROUP + "/" + "code_view_css_value_color";
static QString KEY_CODE_VIEW_FONT_FAMILY = SETTINGS_GROUP + "/" + "code_view_font_family_standard";
static QString KEY_CODE_VIEW_FONT_SIZE = SETTINGS_GROUP + "/" + "code_view_font_size";
static QString KEY_CODE_VIEW_LINE_HIGHLIGHT_COLOR = SETTINGS_GROUP + "/" + "code_view_line_highlight_color";
static QString KEY_CODE_VIEW_LINE_NUMBER_BACKGROUND_COLOR = SETTINGS_GROUP + "/" + "code_view_line_number_background_color";
static QString KEY_CODE_VIEW_LINE_NUMBER_FOREGROUND_COLOR = SETTINGS_GROUP + "/" + "code_view_line_number_foreground_color";
static QString KEY_CODE_VIEW_SELECTION_BACKGROUND_COLOR = SETTINGS_GROUP + "/" + "code_view_selection_background_color";
static QString KEY_CODE_VIEW_SELECTION_FOREGROUND_COLOR = SETTINGS_GROUP + "/" + "code_view_selection_foreground_color";
static QString KEY_CODE_VIEW_SPELLING_UNDERLINE_COLOR = SETTINGS_GROUP + "/" + "code_view_spelling_underline_color";
static QString KEY_CODE_VIEW_XHTML_ATTRIBUTE_NAME_COLOR = SETTINGS_GROUP + "/" + "code_view_xhtml_attribute_name_color";
static QString KEY_CODE_VIEW_XHTML_ATTRIBUTE_VALUE_COLOR = SETTINGS_GROUP + "/" + "code_view_xhtml_attribute_value_color";
static QString KEY_CODE_VIEW_XHTML_CSS_COLOR = SETTINGS_GROUP + "/" + "code_view_xhtml_css_color";
static QString KEY_CODE_VIEW_XHTML_CSS_COMMENT_COLOR = SETTINGS_GROUP + "/" + "code_view_xhtml_css_comment_color";
static QString KEY_CODE_VIEW_XHTML_DOCTYPE_COLOR = SETTINGS_GROUP + "/" + "code_view_xhtml_doctype_color";
static QString KEY_CODE_VIEW_XHTML_ENTITY_COLOR = SETTINGS_GROUP + "/" + "code_view_xhtml_entity_color";
static QString KEY_CODE_VIEW_XHTML_HTML_COLOR = SETTINGS_GROUP + "/" + "code_view_xhtml_html_color";
static QString KEY_CODE_VIEW_XHTML_HTML_COMMENT_COLOR = SETTINGS_GROUP + "/" + "code_view_xhtml_html_comment_color";

static QString KEY_SPECIAL_CHARACTER_FONT_FAMILY = SETTINGS_GROUP + "/" + "special_character_font_family";
static QString KEY_SPECIAL_CHARACTER_FONT_SIZE = SETTINGS_GROUP + "/" + "special_character_font_size";


SettingsStore::SettingsStore()
    : QSettings(Utility::DefinePrefsDir() + "/sigil.ini", QSettings::IniFormat)
{
}

SettingsStore::SettingsStore(QString filename)
    : QSettings(filename, QSettings::IniFormat)
{
}

QString SettingsStore::uiLanguage()
{
    clearSettingsGroup();
    return value(KEY_UI_LANGUAGE, QLocale::system().name()).toString();
}

QString SettingsStore::defaultMetadataLang()
{
    clearSettingsGroup();
    return value(KEY_DEFAULT_METADATA_LANGUAGE, "en").toString();
}

float SettingsStore::zoomImage()
{
    clearSettingsGroup();
    return value(KEY_ZOOM_IMAGE, ZOOM_NORMAL).toFloat();;
}

float SettingsStore::zoomText()
{
    clearSettingsGroup();
    return value(KEY_ZOOM_TEXT, ZOOM_NORMAL).toFloat();
}

float SettingsStore::zoomWeb()
{
    clearSettingsGroup();
    return value(KEY_ZOOM_WEB, ZOOM_NORMAL).toFloat();
}

float SettingsStore::zoomPreview()
{
    clearSettingsGroup();
    return value(KEY_ZOOM_PREVIEW, ZOOM_NORMAL).toFloat();
}

QString SettingsStore::dictionary()
{
    clearSettingsGroup();
    return value(KEY_DICTIONARY_NAME, "en_US").toString();
}

QStringList SettingsStore::enabledUserDictionaries()
{
    clearSettingsGroup();
    return value(KEY_ENABLED_USER_DICTIONARIES, defaultUserDictionary()).toStringList();
}

int SettingsStore::viewState()
{
    clearSettingsGroup();
    return value(KEY_VIEW_STATE, -1).toInt();
}

bool SettingsStore::spellCheck()
{
    clearSettingsGroup();
    return static_cast<bool>(value(KEY_SPELL_CHECK, true).toBool());
}
bool SettingsStore::setLoadLastSessionDictionaries()
{
    clearSettingsGroup();
    return static_cast<bool>(value(KEY_LOAD_DICTS_FROM_LAST_SESSION, true).toBool());
}

bool SettingsStore::setUnloadCurrentDIctionaries()
{
    clearSettingsGroup();
    return static_cast<bool>(value(KEY_UNLOAD_CURR_DICTS, true).toBool());
}

bool SettingsStore::setLoadMainLanguageDictionary()
{
    clearSettingsGroup();
    return static_cast<bool>(value(KEY_LOAD_MAIN_LANG_DICT, true).toBool());
}

bool SettingsStore::setLoadAllLanguagesDictionaries()
{
    clearSettingsGroup();
    return static_cast<bool>(value(KEY_LOAD_ALL_LANG_DICTS, true).toBool());
}

QString SettingsStore::defaultUserDictionary()
{
    clearSettingsGroup();
    return value(KEY_DEFAULT_USER_DICTIONARY, "default").toString();
}

QString SettingsStore::renameTemplate()
{
    clearSettingsGroup();
    return value(KEY_RENAME_TEMPLATE, "").toString();
}

int SettingsStore::remoteOn()
{
    clearSettingsGroup();
    return value(KEY_REMOTE_ON, 0).toInt();
}

int SettingsStore::cleanOn()
{
    clearSettingsGroup();
    return value(KEY_CLEAN_ON, (CLEANON_OPEN | CLEANON_SAVE)).toInt();
}

QStringList SettingsStore::pluginMap()
{
    clearSettingsGroup();
    QStringList EmptyMap = QStringList() << "" << "" << "";
    return value(KEY_PLUGIN_USER_MAP, EmptyMap).toStringList();
}

QString SettingsStore::defaultVersion()
{
    clearSettingsGroup();
    return value(KEY_DEFAULT_VERSION, "2.0").toString();
}

QList <std::pair <ushort, QString>>  SettingsStore::preserveEntityCodeNames()
{
    clearSettingsGroup();
    QList <std::pair <ushort, QString>> codenames;
    QStringList names = value(KEY_PRESERVE_ENTITY_NAMES, "&nbsp;").toStringList();
    QString codes = value(KEY_PRESERVE_ENTITY_CODES, QChar(160)).toString();
    int i = 0;
    foreach(QString name, names) {
        std::pair <ushort, QString> epair;
        epair.first = (ushort) codes.at(i++).unicode();
        epair.second = name;
        codenames.append(epair);
    }
    return codenames;
}

QHash <QString, QString> SettingsStore::pluginEnginePaths()
{
    QHash <QString, QVariant> ep;
    QHash <QString, QString>  enginepath;

    clearSettingsGroup();

    ep = value(KEY_PLUGIN_ENGINE_PATHS).toHash();
    foreach (QString k, ep.keys()) {
        enginepath.insert(k, ep.value(k).toString());
    }

    return enginepath;
}

QString SettingsStore::pluginLastFolder()
{
    clearSettingsGroup();
    return value(KEY_PLUGIN_LAST_FOLDER, QDir::homePath()).toString();
}

bool SettingsStore::useBundledInterp()
{
    clearSettingsGroup();
    // Defaults to true.
    return static_cast<bool>(value(KEY_PLUGIN_USE_BUNDLED_INTERP, true).toBool());
}

QString SettingsStore::cssEpub2ValidationSpec()
{
    clearSettingsGroup();
    return value(KEY_CSS_EPUB2_VALIDATION_SPEC, "css21").toString();
}

QString SettingsStore::cssEpub3ValidationSpec()
{
    clearSettingsGroup();
    return value(KEY_CSS_EPUB3_VALIDATION_SPEC, "css30").toString();
}

QString SettingsStore::tempFolderHome()
{
    clearSettingsGroup();
    return value(KEY_TEMP_FOLDER, QDir::tempPath()).toString();
}

SettingsStore::BookViewAppearance SettingsStore::bookViewAppearance()
{
    clearSettingsGroup();
    SettingsStore::BookViewAppearance appearance;
    appearance.font_family_standard = value(KEY_BOOK_VIEW_FONT_FAMILY_STANDARD, "Arial").toString();
    appearance.font_family_serif = value(KEY_BOOK_VIEW_FONT_FAMILY_SERIF, "Times New Roman").toString();
    appearance.font_family_sans_serif = value(KEY_BOOK_VIEW_FONT_FAMILY_SANS_SERIF, "Arial").toString();
    appearance.font_size = value(KEY_BOOK_VIEW_FONT_SIZE, 16).toInt();
    return appearance;
}

SettingsStore::CodeViewAppearance SettingsStore::codeViewAppearance()
{
    clearSettingsGroup();
    SettingsStore::CodeViewAppearance appearance;
    appearance.background_color = value(KEY_CODE_VIEW_BACKGROUND_COLOR, QColor()).value<QColor>();
    appearance.foreground_color = value(KEY_CODE_VIEW_FOREGROUND_COLOR, QColor()).value<QColor>();
    appearance.css_comment_color = value(KEY_CODE_VIEW_CSS_COMMENT_COLOR, QColor(Qt::darkGreen)).value<QColor>();
    appearance.css_property_color = value(KEY_CODE_VIEW_CSS_PROPERTY_COLOR, QColor(Qt::darkBlue)).value<QColor>();
    appearance.css_quote_color = value(KEY_CODE_VIEW_CSS_QUOTE_COLOR, QColor(Qt::darkMagenta)).value<QColor>();
    appearance.css_selector_color = value(KEY_CODE_VIEW_CSS_SELECTOR_COLOR, QColor(Qt::darkRed)).value<QColor>();
    appearance.css_value_color = value(KEY_CODE_VIEW_CSS_VALUE_COLOR, QColor(Qt::black)).value<QColor>();
    appearance.font_family = value(KEY_CODE_VIEW_FONT_FAMILY, "Courier New").toString();
    appearance.font_size = value(KEY_CODE_VIEW_FONT_SIZE, 10).toInt();
    QColor defaultlineColor = QColor(Qt::yellow).lighter(175);
    appearance.line_highlight_color = value(KEY_CODE_VIEW_LINE_HIGHLIGHT_COLOR, defaultlineColor).value<QColor>();
    appearance.line_number_background_color = value(KEY_CODE_VIEW_LINE_NUMBER_BACKGROUND_COLOR, QColor(225, 225, 225)).value<QColor>();
    appearance.line_number_foreground_color = value(KEY_CODE_VIEW_LINE_NUMBER_FOREGROUND_COLOR, QColor(125, 125, 125)).value<QColor>();
    appearance.selection_background_color = value(KEY_CODE_VIEW_SELECTION_BACKGROUND_COLOR, QColor()).value<QColor>();
    appearance.selection_foreground_color = value(KEY_CODE_VIEW_SELECTION_FOREGROUND_COLOR, QColor()).value<QColor>();
    appearance.spelling_underline_color = value(KEY_CODE_VIEW_SPELLING_UNDERLINE_COLOR, QColor(Qt::red)).value<QColor>();
    appearance.xhtml_attribute_name_color = value(KEY_CODE_VIEW_XHTML_ATTRIBUTE_NAME_COLOR, QColor(Qt::darkRed)).value<QColor>();
    appearance.xhtml_attribute_value_color = value(KEY_CODE_VIEW_XHTML_ATTRIBUTE_VALUE_COLOR, QColor(Qt::darkCyan)).value<QColor>();
    appearance.xhtml_css_color = value(KEY_CODE_VIEW_XHTML_CSS_COLOR, QColor(Qt::darkYellow)).value<QColor>();
    appearance.xhtml_css_comment_color = value(KEY_CODE_VIEW_XHTML_CSS_COMMENT_COLOR, QColor(Qt::darkGreen)).value<QColor>();
    appearance.xhtml_doctype_color = value(KEY_CODE_VIEW_XHTML_DOCTYPE_COLOR, QColor(Qt::darkBlue)).value<QColor>();
    appearance.xhtml_entity_color = value(KEY_CODE_VIEW_XHTML_ENTITY_COLOR, QColor(Qt::darkMagenta)).value<QColor>();
    appearance.xhtml_html_color = value(KEY_CODE_VIEW_XHTML_HTML_COLOR, QColor(Qt::blue)).value<QColor>();
    appearance.xhtml_html_comment_color = value(KEY_CODE_VIEW_XHTML_HTML_COMMENT_COLOR, QColor(Qt::darkGreen)).value<QColor>();
    return appearance;
}

SettingsStore::SpecialCharacterAppearance SettingsStore::specialCharacterAppearance()
{
    clearSettingsGroup();
    SettingsStore::SpecialCharacterAppearance appearance;
    appearance.font_family = value(KEY_SPECIAL_CHARACTER_FONT_FAMILY, "Arial").toString();
    appearance.font_size = value(KEY_SPECIAL_CHARACTER_FONT_SIZE, 14).toInt();
    return appearance;
}

void SettingsStore::setDefaultMetadataLang(const QString &lang)
{
    clearSettingsGroup();
    setValue(KEY_DEFAULT_METADATA_LANGUAGE, lang);
}

void SettingsStore::setUILanguage(const QString &language_code)
{
    clearSettingsGroup();
    setValue(KEY_UI_LANGUAGE, language_code);
}

void SettingsStore::setZoomImage(float zoom)
{
    clearSettingsGroup();
    setValue(KEY_ZOOM_IMAGE, zoom);
}

void SettingsStore::setZoomText(float zoom)
{
    clearSettingsGroup();
    setValue(KEY_ZOOM_TEXT, zoom);
}

void SettingsStore::setZoomWeb(float zoom)
{
    clearSettingsGroup();
    setValue(KEY_ZOOM_WEB, zoom);
}

void SettingsStore::setZoomPreview(float zoom)
{
    clearSettingsGroup();
    setValue(KEY_ZOOM_PREVIEW, zoom);
}

void SettingsStore::setDictionary(const QString &name)
{
    clearSettingsGroup();
    setValue(KEY_DICTIONARY_NAME, name);
}

void SettingsStore::setEnabledUserDictionaries(const QStringList names)
{
    clearSettingsGroup();
    setValue(KEY_ENABLED_USER_DICTIONARIES, names);
}

void SettingsStore::setViewState(int state)
{
    clearSettingsGroup();
    setValue(KEY_VIEW_STATE, state);
}

void SettingsStore::setSpellCheck(bool enabled)
{
    clearSettingsGroup();
    setValue(KEY_SPELL_CHECK, enabled);
}

void SettingsStore::setLoadLastSessionDictionaries(bool enabled)
{
    clearSettingsGroup();
    setValue(KEY_LOAD_DICTS_FROM_LAST_SESSION, enabled);
}

void SettingsStore::setUnloadCurrentDIctionaries(bool enabled)
{
    clearSettingsGroup();
    setValue(KEY_UNLOAD_CURR_DICTS, enabled);
}

void SettingsStore::setLoadMainLanguageDictionary(bool enabled)
{
    clearSettingsGroup();
    setValue(KEY_LOAD_MAIN_LANG_DICT, enabled);
}

void SettingsStore::setLoadAllLanguagesDictionaries(bool enabled)
{
    clearSettingsGroup();
    setValue(KEY_LOAD_ALL_LANG_DICTS, enabled);
}

void SettingsStore::setDefaultUserDictionary(const QString &name)
{
    clearSettingsGroup();
    setValue(KEY_DEFAULT_USER_DICTIONARY, name);
}

void SettingsStore::setRenameTemplate(const QString &name)
{
    clearSettingsGroup();
    setValue(KEY_RENAME_TEMPLATE, name);
}

void SettingsStore::setRemoteOn(int on)
{
    clearSettingsGroup();
    setValue(KEY_REMOTE_ON, on);
}

void SettingsStore::setCleanOn(int on)
{
    clearSettingsGroup();
    setValue(KEY_CLEAN_ON, on);
}

void SettingsStore::setPluginMap(QStringList &map)
{
    clearSettingsGroup();
    setValue(KEY_PLUGIN_USER_MAP, map);
}

void SettingsStore::setDefaultVersion(const QString &version)
{
    clearSettingsGroup();
    setValue(KEY_DEFAULT_VERSION, version);
}

void SettingsStore::setPreserveEntityCodeNames(const QList<std::pair <ushort, QString>> codenames)
{
    clearSettingsGroup();
    QStringList names;
    QString codes;
    std::pair <ushort, QString> epair;
    foreach (epair, codenames) {
        names.append(epair.second);
        codes.append(QChar(epair.first));
    }
    setValue(KEY_PRESERVE_ENTITY_NAMES, names);
    setValue(KEY_PRESERVE_ENTITY_CODES, codes);
}

void SettingsStore::setPluginEnginePaths(const QHash <QString, QString> &enginepaths)
{
    clearSettingsGroup();
    QHash<QString, QVariant> ep;

    foreach (QString k, enginepaths.keys()) {
        ep.insert(k, enginepaths.value(k));
    }
    setValue(KEY_PLUGIN_ENGINE_PATHS, ep);
}

void SettingsStore::setPluginLastFolder(const QString &lastfolder)
{
    clearSettingsGroup();
    setValue(KEY_PLUGIN_LAST_FOLDER, lastfolder);
}

void SettingsStore::setUseBundledInterp(bool use)
{
    clearSettingsGroup();
    setValue(KEY_PLUGIN_USE_BUNDLED_INTERP, use);
}

void SettingsStore::setCssEpub2ValidationSpec(const QString &spec)
{
    clearSettingsGroup();
    setValue(KEY_CSS_EPUB2_VALIDATION_SPEC, spec);
}

void SettingsStore::setCssEpub3ValidationSpec(const QString &spec)
{
    clearSettingsGroup();
    setValue(KEY_CSS_EPUB3_VALIDATION_SPEC, spec);
}

void SettingsStore::setTempFolderHome(const QString &path)
{
    clearSettingsGroup();
    setValue(KEY_TEMP_FOLDER, path);
}

void SettingsStore::setBookViewAppearance(const SettingsStore::BookViewAppearance &book_view_appearance)
{
    clearSettingsGroup();
    setValue(KEY_BOOK_VIEW_FONT_FAMILY_STANDARD, book_view_appearance.font_family_standard);
    setValue(KEY_BOOK_VIEW_FONT_FAMILY_SERIF, book_view_appearance.font_family_serif);
    setValue(KEY_BOOK_VIEW_FONT_FAMILY_SANS_SERIF, book_view_appearance.font_family_sans_serif);
    setValue(KEY_BOOK_VIEW_FONT_SIZE, book_view_appearance.font_size);
}

void SettingsStore::setCodeViewAppearance(const SettingsStore::CodeViewAppearance &code_view_appearance)
{
    clearSettingsGroup();
    setValue(KEY_CODE_VIEW_BACKGROUND_COLOR, code_view_appearance.background_color);
    setValue(KEY_CODE_VIEW_FOREGROUND_COLOR, code_view_appearance.foreground_color);
    setValue(KEY_CODE_VIEW_CSS_COMMENT_COLOR, code_view_appearance.css_comment_color);
    setValue(KEY_CODE_VIEW_CSS_PROPERTY_COLOR, code_view_appearance.css_property_color);
    setValue(KEY_CODE_VIEW_CSS_QUOTE_COLOR, code_view_appearance.css_quote_color);
    setValue(KEY_CODE_VIEW_CSS_SELECTOR_COLOR, code_view_appearance.css_selector_color);
    setValue(KEY_CODE_VIEW_CSS_VALUE_COLOR, code_view_appearance.css_value_color);
    setValue(KEY_CODE_VIEW_FONT_FAMILY, code_view_appearance.font_family);
    setValue(KEY_CODE_VIEW_FONT_SIZE, code_view_appearance.font_size);
    setValue(KEY_CODE_VIEW_LINE_HIGHLIGHT_COLOR, code_view_appearance.line_highlight_color);
    setValue(KEY_CODE_VIEW_LINE_NUMBER_BACKGROUND_COLOR, code_view_appearance.line_number_background_color);
    setValue(KEY_CODE_VIEW_LINE_NUMBER_FOREGROUND_COLOR, code_view_appearance.line_number_foreground_color);
    setValue(KEY_CODE_VIEW_SELECTION_BACKGROUND_COLOR, code_view_appearance.selection_background_color);
    setValue(KEY_CODE_VIEW_SELECTION_FOREGROUND_COLOR, code_view_appearance.selection_foreground_color);
    setValue(KEY_CODE_VIEW_SPELLING_UNDERLINE_COLOR, code_view_appearance.spelling_underline_color);
    setValue(KEY_CODE_VIEW_XHTML_ATTRIBUTE_NAME_COLOR, code_view_appearance.xhtml_attribute_name_color);
    setValue(KEY_CODE_VIEW_XHTML_ATTRIBUTE_VALUE_COLOR, code_view_appearance.xhtml_attribute_value_color);
    setValue(KEY_CODE_VIEW_XHTML_CSS_COLOR, code_view_appearance.xhtml_css_color);
    setValue(KEY_CODE_VIEW_XHTML_CSS_COMMENT_COLOR, code_view_appearance.xhtml_css_comment_color);
    setValue(KEY_CODE_VIEW_XHTML_DOCTYPE_COLOR, code_view_appearance.xhtml_doctype_color);
    setValue(KEY_CODE_VIEW_XHTML_ENTITY_COLOR, code_view_appearance.xhtml_entity_color);
    setValue(KEY_CODE_VIEW_XHTML_HTML_COLOR, code_view_appearance.xhtml_html_color);
    setValue(KEY_CODE_VIEW_XHTML_HTML_COMMENT_COLOR, code_view_appearance.xhtml_html_comment_color);
}

void SettingsStore::setSpecialCharacterAppearance(const SettingsStore::SpecialCharacterAppearance &special_character_appearance)
{
    clearSettingsGroup();
    setValue(KEY_SPECIAL_CHARACTER_FONT_FAMILY, special_character_appearance.font_family);
    setValue(KEY_SPECIAL_CHARACTER_FONT_SIZE, special_character_appearance.font_size);
}

void SettingsStore::clearAppearanceSettings()
{
    clearSettingsGroup();
    remove(KEY_BOOK_VIEW_FONT_FAMILY_STANDARD);
    remove(KEY_BOOK_VIEW_FONT_FAMILY_SERIF);
    remove(KEY_BOOK_VIEW_FONT_FAMILY_SANS_SERIF);
    remove(KEY_BOOK_VIEW_FONT_SIZE);
    remove(KEY_CODE_VIEW_BACKGROUND_COLOR);
    remove(KEY_CODE_VIEW_FOREGROUND_COLOR);
    remove(KEY_CODE_VIEW_CSS_COMMENT_COLOR);
    remove(KEY_CODE_VIEW_CSS_PROPERTY_COLOR);
    remove(KEY_CODE_VIEW_CSS_QUOTE_COLOR);
    remove(KEY_CODE_VIEW_CSS_SELECTOR_COLOR);
    remove(KEY_CODE_VIEW_CSS_VALUE_COLOR);
    remove(KEY_CODE_VIEW_FONT_FAMILY);
    remove(KEY_CODE_VIEW_FONT_SIZE);
    remove(KEY_CODE_VIEW_LINE_HIGHLIGHT_COLOR);
    remove(KEY_CODE_VIEW_LINE_NUMBER_BACKGROUND_COLOR);
    remove(KEY_CODE_VIEW_LINE_NUMBER_FOREGROUND_COLOR);
    remove(KEY_CODE_VIEW_SELECTION_BACKGROUND_COLOR);
    remove(KEY_CODE_VIEW_SELECTION_FOREGROUND_COLOR);
    remove(KEY_CODE_VIEW_SPELLING_UNDERLINE_COLOR);
    remove(KEY_CODE_VIEW_XHTML_ATTRIBUTE_NAME_COLOR);
    remove(KEY_CODE_VIEW_XHTML_ATTRIBUTE_VALUE_COLOR);
    remove(KEY_CODE_VIEW_XHTML_CSS_COLOR);
    remove(KEY_CODE_VIEW_XHTML_CSS_COMMENT_COLOR);
    remove(KEY_CODE_VIEW_XHTML_DOCTYPE_COLOR);
    remove(KEY_CODE_VIEW_XHTML_ENTITY_COLOR);
    remove(KEY_CODE_VIEW_XHTML_HTML_COLOR);
    remove(KEY_CODE_VIEW_XHTML_HTML_COMMENT_COLOR);
    remove(KEY_SPECIAL_CHARACTER_FONT_FAMILY);
    remove(KEY_SPECIAL_CHARACTER_FONT_SIZE);
}

void SettingsStore::clearSettingsGroup()
{
    while (!group().isEmpty()) {
        endGroup();
    }
}
