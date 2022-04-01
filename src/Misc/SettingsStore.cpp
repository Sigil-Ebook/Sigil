/************************************************************************
**
**  Copyright (C) 2016-2022  Kevin B. Hendricks, Stratford, ON
**  Copyright (C) 2016-2020  Doug Massay
**  Copyright (C) 2011-2013  John Schember <john@nachtimwald.com>
**  Copyright (C) 2012-2013  Dave Heiland
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
#include <QPalette>
#include <QFile>
#include <QDir>

#include "Misc/SettingsStore.h"
#include "Misc/PluginDB.h"
#include "Misc/Utility.h"

#include "sigil_constants.h"

#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
static const QString SETTINGS_FILE = SIGIL_SETTINGS_FILE;
#else
static const QString SETTINGS_FILE = SIGIL_V6_SETTINGS_FILE;
#endif

static QString SETTINGS_GROUP = "user_preferences";
static QString KEY_DEFAULT_METADATA_LANGUAGE = SETTINGS_GROUP + "/" + "default_metadata_lang";
static QString KEY_UI_LANGUAGE = SETTINGS_GROUP + "/" + "ui_language";
static QString KEY_UI_FONT = SETTINGS_GROUP + "/" + "ui_font";
static QString KEY_ORIGINAL_UI_FONT = SETTINGS_GROUP + "/" + "original_ui_font";
static QString KEY_UI_ICON_THEME = SETTINGS_GROUP + "/" + "ui_icon_theme";
static QString KEY_DRAG_DISTANCE_TWEAK = SETTINGS_GROUP + "/" + "drag_distance_tweak";
static QString KEY_ZOOM_IMAGE = SETTINGS_GROUP + "/" + "zoom_image";
static QString KEY_ZOOM_TEXT = SETTINGS_GROUP + "/" + "zoom_text";
static QString KEY_ZOOM_WEB = SETTINGS_GROUP + "/" + "zoom_web";
static QString KEY_ZOOM_PREVIEW = SETTINGS_GROUP + "/" + "zoom_preview";
static QString KEY_ZOOM_INSPECTOR = SETTINGS_GROUP + "/" + "zoom_inspector";
static QString KEY_RENAME_TEMPLATE = SETTINGS_GROUP + "/" + "rename_template";
static QString KEY_DICTIONARY_NAME = SETTINGS_GROUP + "/" + "dictionary_name";
static QString KEY_SECONDARY_DICTIONARY_NAME = SETTINGS_GROUP + "/" + "secondary_dictionary_name";
static QString KEY_SPELL_CHECK = SETTINGS_GROUP + "/" + "spell_check";
static QString KEY_SPELL_CHECK_NUMBERS = SETTINGS_GROUP + "/" + "spell_check_numbers";
static QString KEY_DEFAULT_USER_DICTIONARY = SETTINGS_GROUP + "/" + "user_dictionary_name";
static QString KEY_ENABLED_USER_DICTIONARIES = SETTINGS_GROUP + "/" + "enabled_user_dictionaries";
static QString KEY_PLUGIN_USER_MAP = SETTINGS_GROUP + "/" + "plugin_user_map";
static QString KEY_CLEAN_ON = SETTINGS_GROUP + "/" + "clean_on";
static QString KEY_REMOTE_ON = SETTINGS_GROUP + "/" + "remote_on";
static QString KEY_JAVASCRIPT_ON = SETTINGS_GROUP + "/" + "javascript_on";
static QString KEY_SHOWFULLPATH_ON = SETTINGS_GROUP + "/" + "showfullpath_on";
static QString KEY_HIGHDPI_SETTING = SETTINGS_GROUP + "/" + "high_dpi";
static QString KEY_DISABLEGPU_SETTING = SETTINGS_GROUP + "/" + "disable_gpu";
static QString KEY_PREVIEW_DARK_IN_DM = SETTINGS_GROUP + "/" + "preview_dark_in_dm";
static QString KEY_DEFAULT_VERSION = SETTINGS_GROUP + "/" + "default_version";
static QString KEY_PRESERVE_ENTITY_NAMES = SETTINGS_GROUP + "/" + "preserve_entity_names";
static QString KEY_PRESERVE_ENTITY_CODES = SETTINGS_GROUP + "/" + "preserve_entity_codes";
static QString KEY_EXTERNAL_XHTML_EDITOR = SETTINGS_GROUP + "/" + "external_xhtml_editor"; 
static QString KEY_ENABLE_ALTGR = SETTINGS_GROUP + "/" + "enable_altgr";

static QString KEY_PLUGIN_INFO = SETTINGS_GROUP + "/" + "plugin_info";
static QString KEY_PLUGIN_ENGINE_PATHS = SETTINGS_GROUP + "/" + "plugin_engine_paths";
static QString KEY_PLUGIN_LAST_FOLDER = SETTINGS_GROUP + "/" + "plugin_add_last_folder";
static QString KEY_PLUGIN_USE_BUNDLED_INTERP = SETTINGS_GROUP + "/" + "plugin_use_bundled_interp";

static QString KEY_CSS_EPUB2_VALIDATION_SPEC = SETTINGS_GROUP + "/" + "css_epub2_validation_spec";
static QString KEY_CSS_EPUB3_VALIDATION_SPEC = SETTINGS_GROUP + "/" + "css_epub3_validation_spec";

static QString KEY_TEMP_FOLDER = SETTINGS_GROUP + "/" + "temp_folder_path";

static QString KEY_APPEARANCE_PREFS_TAB_INDEX = SETTINGS_GROUP + "/" + "appearance_prefs_tab_index";
static QString KEY_PREVIEW_FONT_FAMILY_STANDARD = SETTINGS_GROUP + "/" + "preview_font_family_standard";
static QString KEY_PREVIEW_FONT_FAMILY_SERIF = SETTINGS_GROUP + "/" + "preview_font_family_serif";
static QString KEY_PREVIEW_FONT_FAMILY_SANS_SERIF = SETTINGS_GROUP + "/" + "preview_font_family_sans_serif";
static QString KEY_PREVIEW_FONT_SIZE = SETTINGS_GROUP + "/" + "preview_font_size";

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
static QString KEY_CODE_VIEW_SPELLING_UNDERLINE_COLOR = SETTINGS_GROUP + "/" + "code_view_spelling_underline_color";
static QString KEY_CODE_VIEW_XHTML_ATTRIBUTE_NAME_COLOR = SETTINGS_GROUP + "/" + "code_view_xhtml_attribute_name_color";
static QString KEY_CODE_VIEW_XHTML_ATTRIBUTE_VALUE_COLOR = SETTINGS_GROUP + "/" + "code_view_xhtml_attribute_value_color";
static QString KEY_CODE_VIEW_XHTML_CSS_COLOR = SETTINGS_GROUP + "/" + "code_view_xhtml_css_color";
static QString KEY_CODE_VIEW_XHTML_CSS_COMMENT_COLOR = SETTINGS_GROUP + "/" + "code_view_xhtml_css_comment_color";
static QString KEY_CODE_VIEW_XHTML_DOCTYPE_COLOR = SETTINGS_GROUP + "/" + "code_view_xhtml_doctype_color";
static QString KEY_CODE_VIEW_XHTML_ENTITY_COLOR = SETTINGS_GROUP + "/" + "code_view_xhtml_entity_color";
static QString KEY_CODE_VIEW_XHTML_HTML_COLOR = SETTINGS_GROUP + "/" + "code_view_xhtml_html_color";
static QString KEY_CODE_VIEW_XHTML_HTML_COMMENT_COLOR = SETTINGS_GROUP + "/" + "code_view_xhtml_html_comment_color";

static QString KEY_CODE_VIEW_HIGHLIGHT_OPEN_CLOSE_TAGS = SETTINGS_GROUP + "/" + "code_view_highlight_open_close_tags";

// Dark Appearance
static QString KEY_CV_DARK_CSS_COMMENT_COLOR = SETTINGS_GROUP + "/" + "cv_dark_css_comment_color";
static QString KEY_CV_DARK_CSS_PROPERTY_COLOR = SETTINGS_GROUP + "/" + "cv_dark_css_property_color";
static QString KEY_CV_DARK_CSS_QUOTE_COLOR = SETTINGS_GROUP + "/" + "cv_dark_css_quote_color";
static QString KEY_CV_DARK_CSS_SELECTOR_COLOR = SETTINGS_GROUP + "/" + "cv_dark_css_selector_color";
static QString KEY_CV_DARK_CSS_VALUE_COLOR = SETTINGS_GROUP + "/" + "cv_dark_css_value_color";
static QString KEY_CV_DARK_FONT_FAMILY = SETTINGS_GROUP + "/" + "cv_dark_font_family_standard";
static QString KEY_CV_DARK_FONT_SIZE = SETTINGS_GROUP + "/" + "cv_dark_font_size";
static QString KEY_CV_DARK_LINE_HIGHLIGHT_COLOR = SETTINGS_GROUP + "/" + "cv_dark_line_highlight_color";
static QString KEY_CV_DARK_LINE_NUMBER_BACKGROUND_COLOR = SETTINGS_GROUP + "/" + "cv_dark_line_number_background_color";
static QString KEY_CV_DARK_LINE_NUMBER_FOREGROUND_COLOR = SETTINGS_GROUP + "/" + "cv_dark_line_number_foreground_color";
static QString KEY_CV_DARK_SPELLING_UNDERLINE_COLOR = SETTINGS_GROUP + "/" + "cv_dark_spelling_underline_color";
static QString KEY_CV_DARK_XHTML_ATTRIBUTE_NAME_COLOR = SETTINGS_GROUP + "/" + "cv_dark_xhtml_attribute_name_color";
static QString KEY_CV_DARK_XHTML_ATTRIBUTE_VALUE_COLOR = SETTINGS_GROUP + "/" + "cv_dark_xhtml_attribute_value_color";
static QString KEY_CV_DARK_XHTML_CSS_COLOR = SETTINGS_GROUP + "/" + "cv_dark_xhtml_css_color";
static QString KEY_CV_DARK_XHTML_CSS_COMMENT_COLOR = SETTINGS_GROUP + "/" + "cv_dark_xhtml_css_comment_color";
static QString KEY_CV_DARK_XHTML_DOCTYPE_COLOR = SETTINGS_GROUP + "/" + "cv_dark_xhtml_doctype_color";
static QString KEY_CV_DARK_XHTML_ENTITY_COLOR = SETTINGS_GROUP + "/" + "cv_dark_xhtml_entity_color";
static QString KEY_CV_DARK_XHTML_HTML_COLOR = SETTINGS_GROUP + "/" + "cv_dark_xhtml_html_color";
static QString KEY_CV_DARK_XHTML_HTML_COMMENT_COLOR = SETTINGS_GROUP + "/" + "cv_dark_xhtml_html_comment_color";
// End Dark appearance

static QString KEY_SPECIAL_CHARACTER_FONT_FAMILY = SETTINGS_GROUP + "/" + "special_character_font_family";
static QString KEY_SPECIAL_CHARACTER_FONT_SIZE = SETTINGS_GROUP + "/" + "special_character_font_size";
static QString KEY_MAIN_MENU_ICON_SIZE = SETTINGS_GROUP + "/" + "main_menu_icon_size";
static QString KEY_CLIPBOARD_HISTORY_LIMIT = SETTINGS_GROUP + "/" + "clipboard_history_limit";

SettingsStore::SettingsStore()
    : QSettings(Utility::DefinePrefsDir() + "/" + SETTINGS_FILE, QSettings::IniFormat)
{  
    // See QTBUG-40796 and QTBUG-54510 as using UTF-8 as a codec for ini files is very broken
    // setIniCodec("UTF-8");
}

SettingsStore::SettingsStore(QString filename)
    : QSettings(filename, QSettings::IniFormat)
{
    // See QTBUG-40796 and QTBUG-54510 as using UTF-8 as a codec for ini files is very broken
    // setIniCodec("UTF-8");
}

QString SettingsStore::uiLanguage()
{
    clearSettingsGroup();
    return value(KEY_UI_LANGUAGE, QLocale::system().name()).toString();
}

QString SettingsStore::uiFont()
{
    clearSettingsGroup();
    return value(KEY_UI_FONT, "").toString();
}

QString SettingsStore::originalUIFont()
{
    clearSettingsGroup();
    return value(KEY_ORIGINAL_UI_FONT, "").toString();
}

QString SettingsStore::uiIconTheme()
{
    clearSettingsGroup();
    return value(KEY_UI_ICON_THEME, "main").toString();
}

int SettingsStore::uiDragDistanceTweak()
{
    clearSettingsGroup();
    return value(KEY_DRAG_DISTANCE_TWEAK, 0).toInt();
}

QString SettingsStore::defaultMetadataLang()
{
    clearSettingsGroup();
    return value(KEY_DEFAULT_METADATA_LANGUAGE, "en").toString();
}

QString SettingsStore::externalXEditorPath()
{
    clearSettingsGroup();
    return value(KEY_EXTERNAL_XHTML_EDITOR, "").toString();
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

float SettingsStore::zoomInspector()
{
    clearSettingsGroup();
    return value(KEY_ZOOM_INSPECTOR, ZOOM_NORMAL).toFloat();
}

QString SettingsStore::dictionary()
{
    clearSettingsGroup();
    return value(KEY_DICTIONARY_NAME, "en_US").toString();
}

QString SettingsStore::secondary_dictionary()
{
    clearSettingsGroup();
    return value(KEY_SECONDARY_DICTIONARY_NAME, "").toString();
}

QStringList SettingsStore::enabledUserDictionaries()
{
    clearSettingsGroup();
    return value(KEY_ENABLED_USER_DICTIONARIES, defaultUserDictionary()).toStringList();
}

bool SettingsStore::spellCheck()
{
    clearSettingsGroup();
    return static_cast<bool>(value(KEY_SPELL_CHECK, false).toBool());
}

bool SettingsStore::spellCheckNumbers()
{
    clearSettingsGroup();
    return static_cast<bool>(value(KEY_SPELL_CHECK_NUMBERS, false).toBool());
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

int SettingsStore::javascriptOn()
{
    clearSettingsGroup();
    return value(KEY_JAVASCRIPT_ON, 0).toInt();
}

int SettingsStore::showFullPathOn()
{
    clearSettingsGroup();
    return value(KEY_SHOWFULLPATH_ON, 1).toInt();
}

int SettingsStore::highDPI()
{
    clearSettingsGroup();
    return value(KEY_HIGHDPI_SETTING, 0).toInt();
}

bool SettingsStore::disableGPU()
{
    clearSettingsGroup();
    return static_cast<bool>(value(KEY_DISABLEGPU_SETTING, false).toBool());
}

int SettingsStore::previewDark()
{
    clearSettingsGroup();
    return value(KEY_PREVIEW_DARK_IN_DM, 1).toInt();
}

int SettingsStore::cleanOn()
{
    clearSettingsGroup();
    return value(KEY_CLEAN_ON, (CLEANON_OPEN | CLEANON_SAVE)).toInt();
}

QStringList SettingsStore::pluginMap()
{
    clearSettingsGroup();
    QStringList EmptyMap = QStringList();
    for (int i = 0; i < 10; i++) {
        EmptyMap.append("");
    }
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
    QStringList names = value(KEY_PRESERVE_ENTITY_NAMES, "&#160;").toStringList();
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
    QString temp_path = value(KEY_TEMP_FOLDER, "<SIGIL_DEFAULT_TEMP_HOME>").toString();
    if (temp_path != "<SIGIL_DEFAULT_TEMP_HOME>") {
        QDir tdir = QDir(temp_path);
        if ( !tdir.exists() || !tdir.isReadable()) {
            temp_path = "<SIGIL_DEFAULT_TEMP_HOME>";
        }
    }
    return temp_path;
}

int SettingsStore::appearancePrefsTabIndex() {
    clearSettingsGroup();
    return value(KEY_APPEARANCE_PREFS_TAB_INDEX, 0).toInt();
}

SettingsStore::PreviewAppearance SettingsStore::previewAppearance()
{
    clearSettingsGroup();
    SettingsStore::PreviewAppearance appearance;
    appearance.font_family_standard = value(KEY_PREVIEW_FONT_FAMILY_STANDARD, "Arial").toString();
    appearance.font_family_serif = value(KEY_PREVIEW_FONT_FAMILY_SERIF, "Times New Roman").toString();
    appearance.font_family_sans_serif = value(KEY_PREVIEW_FONT_FAMILY_SANS_SERIF, "Arial").toString();
    appearance.font_size = value(KEY_PREVIEW_FONT_SIZE, 16).toInt();
    return appearance;
}

SettingsStore::CodeViewAppearance SettingsStore::codeViewAppearance()
{
    clearSettingsGroup();
    SettingsStore::CodeViewAppearance appearance;
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

bool SettingsStore::highlightOpenCloseTags()
{
    clearSettingsGroup();
    return static_cast<bool>(value(KEY_CODE_VIEW_HIGHLIGHT_OPEN_CLOSE_TAGS, true).toBool());
}

SettingsStore::CodeViewAppearance SettingsStore::codeViewDarkAppearance()
{
    clearSettingsGroup();
    SettingsStore::CodeViewAppearance appearance;
    appearance.css_comment_color = value(KEY_CV_DARK_CSS_COMMENT_COLOR, QColor(112, 109, 91)).value<QColor>();
    appearance.css_property_color = value(KEY_CV_DARK_CSS_PROPERTY_COLOR, QColor(159, 194, 138)).value<QColor>();
    appearance.css_quote_color = value(KEY_CV_DARK_CSS_QUOTE_COLOR, QColor(235, 147, 154)).value<QColor>();
    appearance.css_selector_color = value(KEY_CV_DARK_CSS_SELECTOR_COLOR, QColor(239, 239, 143)).value<QColor>();
    appearance.css_value_color = value(KEY_CV_DARK_CSS_VALUE_COLOR, QColor(252, 255, 224)).value<QColor>();
    appearance.font_family = value(KEY_CV_DARK_FONT_FAMILY, "Courier New").toString();
    appearance.font_size = value(KEY_CV_DARK_FONT_SIZE, 10).toInt();
    appearance.line_highlight_color = value(KEY_CV_DARK_LINE_HIGHLIGHT_COLOR, QColor(81, 81, 81)).value<QColor>();
    appearance.line_number_background_color = value(KEY_CV_DARK_LINE_NUMBER_BACKGROUND_COLOR, QPalette().color(QPalette::AlternateBase)).value<QColor>();
    appearance.line_number_foreground_color = value(KEY_CV_DARK_LINE_NUMBER_FOREGROUND_COLOR, QColor(229, 229, 229)).value<QColor>();
    appearance.spelling_underline_color = value(KEY_CV_DARK_SPELLING_UNDERLINE_COLOR, QColor(255, 55, 55)).value<QColor>();
    appearance.xhtml_attribute_name_color = value(KEY_CV_DARK_XHTML_ATTRIBUTE_NAME_COLOR, QColor(159, 194, 138)).value<QColor>();
    appearance.xhtml_attribute_value_color = value(KEY_CV_DARK_XHTML_ATTRIBUTE_VALUE_COLOR, QColor(232, 145, 152)).value<QColor>();
    appearance.xhtml_css_color = value(KEY_CV_DARK_XHTML_CSS_COLOR, QColor(128, 128, 0)).value<QColor>();
    appearance.xhtml_css_comment_color = value(KEY_CV_DARK_XHTML_CSS_COMMENT_COLOR, QColor(112, 109, 91)).value<QColor>();
    appearance.xhtml_doctype_color = value(KEY_CV_DARK_XHTML_DOCTYPE_COLOR, QColor(126, 252, 255)).value<QColor>();
    appearance.xhtml_entity_color = value(KEY_CV_DARK_XHTML_ENTITY_COLOR, QColor(235, 255, 196)).value<QColor>();
    appearance.xhtml_html_color = value(KEY_CV_DARK_XHTML_HTML_COLOR, QColor(239, 239, 143)).value<QColor>();
    appearance.xhtml_html_comment_color = value(KEY_CV_DARK_XHTML_HTML_COMMENT_COLOR, QColor(112, 109, 91)).value<QColor>();
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

double SettingsStore::mainMenuIconSize()
{
    clearSettingsGroup();
    return value(KEY_MAIN_MENU_ICON_SIZE, 1.8).toDouble();
}

int SettingsStore::clipboardHistoryLimit()
{
    clearSettingsGroup();
    int tmpLimit = value(KEY_CLIPBOARD_HISTORY_LIMIT, CLIPBOARD_HISTORY_MAX).toInt();
    // Ensure that any clipboard history limit gleaned from the ini file is between 0 and CLIPBOARD_HISTORY_MAX
    return ((tmpLimit >= 0 && tmpLimit <= CLIPBOARD_HISTORY_MAX) ? tmpLimit : CLIPBOARD_HISTORY_MAX);
    //return value(KEY_CLIPBOARD_HISTORY_LIMIT, CLIPBOARD_HISTORY_MAX).toInt();
}

bool SettingsStore::enableAltGr()
{
    clearSettingsGroup();
    return static_cast<bool>(value(KEY_ENABLE_ALTGR, false).toBool());
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

void SettingsStore::setUIFont(const QString &font_data)
{
    clearSettingsGroup();
    setValue(KEY_UI_FONT, font_data);
}

void SettingsStore::setOriginalUIFont(const QString &font_data)
{
    clearSettingsGroup();
    setValue(KEY_ORIGINAL_UI_FONT, font_data);
}

void SettingsStore::setUIIconTheme(const QString &iconthemename)
{
    clearSettingsGroup();
    setValue(KEY_UI_ICON_THEME, iconthemename);
}

void SettingsStore::setUiDragDistanceTweak(int tweak)
{
    clearSettingsGroup();
    setValue(KEY_DRAG_DISTANCE_TWEAK, tweak);
}

void SettingsStore::setExternalXEditorPath(const QString &path)
{
    clearSettingsGroup();
    setValue(KEY_EXTERNAL_XHTML_EDITOR, path);
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

void SettingsStore::setZoomInspector(float zoom)
{
    clearSettingsGroup();
    setValue(KEY_ZOOM_INSPECTOR, zoom);
}

void SettingsStore::setDictionary(const QString &name)
{
    clearSettingsGroup();
    setValue(KEY_DICTIONARY_NAME, name);
}

void SettingsStore::setSecondaryDictionary(const QString &name)
{
    clearSettingsGroup();
    setValue(KEY_SECONDARY_DICTIONARY_NAME, name);
}

void SettingsStore::setEnabledUserDictionaries(const QStringList names)
{
    clearSettingsGroup();
    setValue(KEY_ENABLED_USER_DICTIONARIES, names);
}

void SettingsStore::setSpellCheck(bool enabled)
{
    clearSettingsGroup();
    setValue(KEY_SPELL_CHECK, enabled);
}

void SettingsStore::setSpellCheckNumbers(bool enabled)
{
    clearSettingsGroup();
    setValue(KEY_SPELL_CHECK_NUMBERS, enabled);
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

void SettingsStore::setJavascriptOn(int on)
{
    clearSettingsGroup();
    setValue(KEY_JAVASCRIPT_ON, on);
}

void SettingsStore::setShowFullPathOn(int on)
{
    clearSettingsGroup();
    setValue(KEY_SHOWFULLPATH_ON, on);
}

void SettingsStore::setHighDPI(int value)
{
    clearSettingsGroup();
    setValue(KEY_HIGHDPI_SETTING, value);
}

void SettingsStore::setDisableGPU(bool value)
{
    clearSettingsGroup();
    setValue(KEY_DISABLEGPU_SETTING, value);
}

void SettingsStore::setPreviewDark(int enabled)
{
    clearSettingsGroup();
    setValue(KEY_PREVIEW_DARK_IN_DM, enabled);
}


void SettingsStore::setCleanOn(int on)
{
    clearSettingsGroup();
    setValue(KEY_CLEAN_ON, on);
}

void SettingsStore::setPluginMap(const QStringList &map)
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
    if (QDir(path).exists()) {
        setValue(KEY_TEMP_FOLDER, path);
    } else {
        setValue(KEY_TEMP_FOLDER, "<SIGIL_DEFAULT_TEMP_HOME>" );
    }
}

void SettingsStore::setAppearancePrefsTabIndex(int index) {
    clearSettingsGroup();
    setValue(KEY_APPEARANCE_PREFS_TAB_INDEX, index);
}

void SettingsStore::setPreviewAppearance(const SettingsStore::PreviewAppearance &preview_appearance)
{
    clearSettingsGroup();
    setValue(KEY_PREVIEW_FONT_FAMILY_STANDARD, preview_appearance.font_family_standard);
    setValue(KEY_PREVIEW_FONT_FAMILY_SERIF, preview_appearance.font_family_serif);
    setValue(KEY_PREVIEW_FONT_FAMILY_SANS_SERIF, preview_appearance.font_family_sans_serif);
    setValue(KEY_PREVIEW_FONT_SIZE, preview_appearance.font_size);
}

void SettingsStore::setCodeViewAppearance(const SettingsStore::CodeViewAppearance &code_view_appearance)
{
    clearSettingsGroup();
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

void SettingsStore::setHighlightOpenCloseTags(bool enabled)
{
    clearSettingsGroup();
    setValue(KEY_CODE_VIEW_HIGHLIGHT_OPEN_CLOSE_TAGS, enabled);
}

void SettingsStore::setCodeViewDarkAppearance(const SettingsStore::CodeViewAppearance &code_view_appearance)
{
    clearSettingsGroup();
    setValue(KEY_CV_DARK_CSS_COMMENT_COLOR, code_view_appearance.css_comment_color);
    setValue(KEY_CV_DARK_CSS_PROPERTY_COLOR, code_view_appearance.css_property_color);
    setValue(KEY_CV_DARK_CSS_QUOTE_COLOR, code_view_appearance.css_quote_color);
    setValue(KEY_CV_DARK_CSS_SELECTOR_COLOR, code_view_appearance.css_selector_color);
    setValue(KEY_CV_DARK_CSS_VALUE_COLOR, code_view_appearance.css_value_color);
    setValue(KEY_CV_DARK_FONT_FAMILY, code_view_appearance.font_family);
    setValue(KEY_CV_DARK_FONT_SIZE, code_view_appearance.font_size);
    setValue(KEY_CV_DARK_LINE_HIGHLIGHT_COLOR, code_view_appearance.line_highlight_color);
    setValue(KEY_CV_DARK_LINE_NUMBER_BACKGROUND_COLOR, code_view_appearance.line_number_background_color);
    setValue(KEY_CV_DARK_LINE_NUMBER_FOREGROUND_COLOR, code_view_appearance.line_number_foreground_color);
    setValue(KEY_CV_DARK_SPELLING_UNDERLINE_COLOR, code_view_appearance.spelling_underline_color);
    setValue(KEY_CV_DARK_XHTML_ATTRIBUTE_NAME_COLOR, code_view_appearance.xhtml_attribute_name_color);
    setValue(KEY_CV_DARK_XHTML_ATTRIBUTE_VALUE_COLOR, code_view_appearance.xhtml_attribute_value_color);
    setValue(KEY_CV_DARK_XHTML_CSS_COLOR, code_view_appearance.xhtml_css_color);
    setValue(KEY_CV_DARK_XHTML_CSS_COMMENT_COLOR, code_view_appearance.xhtml_css_comment_color);
    setValue(KEY_CV_DARK_XHTML_DOCTYPE_COLOR, code_view_appearance.xhtml_doctype_color);
    setValue(KEY_CV_DARK_XHTML_ENTITY_COLOR, code_view_appearance.xhtml_entity_color);
    setValue(KEY_CV_DARK_XHTML_HTML_COLOR, code_view_appearance.xhtml_html_color);
    setValue(KEY_CV_DARK_XHTML_HTML_COMMENT_COLOR, code_view_appearance.xhtml_html_comment_color);
}

void SettingsStore::setSpecialCharacterAppearance(const SettingsStore::SpecialCharacterAppearance &special_character_appearance)
{
    clearSettingsGroup();
    setValue(KEY_SPECIAL_CHARACTER_FONT_FAMILY, special_character_appearance.font_family);
    setValue(KEY_SPECIAL_CHARACTER_FONT_SIZE, special_character_appearance.font_size);
}

void SettingsStore::setMainMenuIconSize(double icon_size)
{
    clearSettingsGroup();
    setValue(KEY_MAIN_MENU_ICON_SIZE, icon_size);
}

void SettingsStore::setClipboardHistoryLimit(int limit)
{
    clearSettingsGroup();
    setValue(KEY_CLIPBOARD_HISTORY_LIMIT, limit);
}

void SettingsStore::setEnableAltGr(bool enabled)
{
    clearSettingsGroup();
    setValue(KEY_ENABLE_ALTGR, enabled);
}

void SettingsStore::clearAppearanceSettings()
{
    clearSettingsGroup();
    remove(KEY_PREVIEW_FONT_FAMILY_STANDARD);
    remove(KEY_PREVIEW_FONT_FAMILY_SERIF);
    remove(KEY_PREVIEW_FONT_FAMILY_SANS_SERIF);
    remove(KEY_PREVIEW_FONT_SIZE);
    if (!Utility::IsDarkMode()) {
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
        remove(KEY_CODE_VIEW_SPELLING_UNDERLINE_COLOR);
        remove(KEY_CODE_VIEW_XHTML_ATTRIBUTE_NAME_COLOR);
        remove(KEY_CODE_VIEW_XHTML_ATTRIBUTE_VALUE_COLOR);
        remove(KEY_CODE_VIEW_XHTML_CSS_COLOR);
        remove(KEY_CODE_VIEW_XHTML_CSS_COMMENT_COLOR);
        remove(KEY_CODE_VIEW_XHTML_DOCTYPE_COLOR);
        remove(KEY_CODE_VIEW_XHTML_ENTITY_COLOR);
        remove(KEY_CODE_VIEW_XHTML_HTML_COLOR);
        remove(KEY_CODE_VIEW_XHTML_HTML_COMMENT_COLOR);
    } else {
        // Dark Appearance
        remove(KEY_CV_DARK_CSS_COMMENT_COLOR);
        remove(KEY_CV_DARK_CSS_PROPERTY_COLOR);
        remove(KEY_CV_DARK_CSS_QUOTE_COLOR);
        remove(KEY_CV_DARK_CSS_SELECTOR_COLOR);
        remove(KEY_CV_DARK_CSS_VALUE_COLOR);
        remove(KEY_CV_DARK_FONT_FAMILY);
        remove(KEY_CV_DARK_FONT_SIZE);
        remove(KEY_CV_DARK_LINE_HIGHLIGHT_COLOR);
        remove(KEY_CV_DARK_LINE_NUMBER_BACKGROUND_COLOR);
        remove(KEY_CV_DARK_LINE_NUMBER_FOREGROUND_COLOR);
        remove(KEY_CV_DARK_SPELLING_UNDERLINE_COLOR);
        remove(KEY_CV_DARK_XHTML_ATTRIBUTE_NAME_COLOR);
        remove(KEY_CV_DARK_XHTML_ATTRIBUTE_VALUE_COLOR);
        remove(KEY_CV_DARK_XHTML_CSS_COLOR);
        remove(KEY_CV_DARK_XHTML_CSS_COMMENT_COLOR);
        remove(KEY_CV_DARK_XHTML_DOCTYPE_COLOR);
        remove(KEY_CV_DARK_XHTML_ENTITY_COLOR);
        remove(KEY_CV_DARK_XHTML_HTML_COLOR);
        remove(KEY_CV_DARK_XHTML_HTML_COMMENT_COLOR);
    }

    remove(KEY_CODE_VIEW_HIGHLIGHT_OPEN_CLOSE_TAGS);
    remove(KEY_SPECIAL_CHARACTER_FONT_FAMILY);
    remove(KEY_SPECIAL_CHARACTER_FONT_SIZE);
    remove(KEY_MAIN_MENU_ICON_SIZE);
    remove(KEY_SHOWFULLPATH_ON);
    remove(KEY_HIGHDPI_SETTING);
    remove(KEY_UI_FONT);
    remove(KEY_UI_ICON_THEME);
    remove(KEY_DRAG_DISTANCE_TWEAK);
    remove(KEY_PREVIEW_DARK_IN_DM);
    ;
}

void SettingsStore::clearSettingsGroup()
{
    while (!group().isEmpty()) {
        endGroup();
    }
}
