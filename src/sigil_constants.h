/************************************************************************
**
**  Copyright (C) 2015-2025 Kevin B. Hendricks, Stratford Ontario Canada
**  Copyright (C) 2015-2025 Doug Massay
**  Copyright (C) 2009-2011 Strahinja Markovic  <strahinja.markovic@gmail.com>
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
#ifndef SG_CONSTANTS_H
#define SG_CONSTANTS_H

// Workaround compiler issue with LTO and static initializers from
// const QStrings defined in a different compilation unit by using defines:
#define CLIPS_SETTINGS_FILE       "sigil_clips.ini"
#define INDEX_SETTINGS_FILE       "sigil_index.ini"
#define SEARCHES_SETTINGS_FILE    "sigil_searches.ini"
#define SEARCHES_V2_SETTINGS_FILE "sigil_searches_v2.ini"
#define SIGIL_SETTINGS_FILE       "sigil.ini"
#define CLIPS_V6_SETTINGS_FILE    "sigil_clips_v6.ini"
#define INDEX_V6_SETTINGS_FILE    "sigil_index_v6.ini"
#define SEARCHES_V6_SETTINGS_FILE "sigil_searches_v6.ini"
#define SIGIL_V6_SETTINGS_FILE    "sigil_v6.ini"
#define SIGIL_FUNCTION_REPLACE_JSON_FILE "replace_functions.json"

#include <QStringList>

// QL1SV() Macro to handle renaming of QLatin1String to QLatin1StringView in Qt6.4+
#include <QLatin1StringView>
#define QL1SV(t) QLatin1StringView(t)

class QString;

// These enable us to use constants defined
// in one CPP file to be used in another
extern const QString BODY_START;
extern const QString BODY_END;
extern const QString BREAK_TAG_INSERT;
extern const QString HEADING;
extern const QString STYLE_TAG;
extern const QStringList BLOCK_LEVEL_TAGS;
extern const QStringList ID_TAGS;
extern const QStringList IMAGE_TAGS;
extern const QStringList AUDIO_TAGS;
extern const QStringList VIDEO_TAGS;
extern const QStringList ANCHOR_TAGS;
extern const QStringList SRC_TAGS;
extern const QString VERSION_NUMBERS;
extern const QString SIGIL_VERSION;
extern const int PROGRESS_BAR_MINIMUM_DURATION;
extern const QString IMAGE_FOLDER_NAME;
extern const QString FONT_FOLDER_NAME;
extern const QString TEXT_FOLDER_NAME;
extern const QString STYLE_FOLDER_NAME;
extern const QString MISC_FOLDER_NAME;
extern const QStringList IMAGE_MIMEYPES;
extern const QStringList TEXT_MIMETYPES;
extern const QStringList STYLE_MIMETYPES;
extern const QString SIGIL_TOC_ID_PREFIX;
extern const QStringList HEADING_TAGS;
extern const QString SIGIL_NOT_IN_TOC_CLASS;
extern const QString OLD_SIGIL_NOT_IN_TOC_CLASS;
extern const QString FIRST_SECTION_PREFIX;
extern const QString FIRST_SECTION_NAME;
extern const QString OPF_FILE_NAME;
extern const QString NCX_FILE_NAME;
extern const QString CONTAINER_XML_FILE_NAME;
extern const QStringList TEXT_EXTENSIONS;
extern const QStringList FONT_EXTENSIONS;
extern const QStringList IMAGE_EXTENSIONS;
extern const QStringList SVG_EXTENSIONS;
extern const QStringList JPG_EXTENSIONS;
extern const QStringList TIFF_EXTENSIONS;
extern const QStringList VIDEO_EXTENSIONS;
extern const QStringList AUDIO_EXTENSIONS;
extern const QStringList MISC_XML_MIMETYPES;
extern const QString ENCODING_ATTRIBUTE;
extern const QString STANDALONE_ATTRIBUTE;
extern const QString VERSION_ATTRIBUTE;
extern const QString ADOBE_FONT_ALGO_ID;
extern const QString IDPF_FONT_ALGO_ID;
extern const QString DUBLIN_CORE_NS;
extern const int XML_DECLARATION_SEARCH_PREFIX_SIZE;

extern const QString NCX_MIMETYPE;

extern const char         *XHTML_ENTITIES_DTD_ID;
extern const unsigned int  XHTML_ENTITIES_DTD_LEN;
extern const unsigned char XHTML_ENTITIES_DTD[];

extern const char         *NCX_2005_1_DTD_ID;
extern const unsigned int  NCX_2005_1_DTD_LEN;
extern const unsigned char NCX_2005_1_DTD[];

extern const int PCRE_MAX_CAPTURE_GROUPS;

extern const float ZOOM_STEP;
extern const float ZOOM_MIN;
extern const float ZOOM_MAX;
extern const float ZOOM_NORMAL;

extern const QString SET_CURSOR_JS;

extern const QString SIGIL_INDEX_CLASS;
extern const QString SIGIL_INDEX_ID_PREFIX;

extern const QString IMAGE_HTML_BASE_PREVIEW;
extern const QString VIDEO_HTML_BASE;
extern const QString AUDIO_HTML_BASE;

extern const QString SGC_TOC_CSS_FILENAME;
extern const QString SGC_INDEX_CSS_FILENAME;
extern const QString CUSTOM_ICON_THEME_FILENAME;

extern const QString HTML_NAV_FILENAME;
extern const QString EMPTY_NAV_FILE_START;
extern const QString EMPTY_NAV_FILE_TOC;
extern const QString EMPTY_NAV_FILE_LANDMARKS;
extern const QString EMPTY_NAV_FILE_END;
extern const QString SGC_NAV_CSS_FILENAME;

extern const QString HTML_COVER_FILENAME;
extern const QString HTML_COVER_SOURCE;
extern const QString HTML5_COVER_SOURCE;

extern const int CLIPBOARD_HISTORY_MAX;

extern const QString SIGIL_PREFS_DIR;
extern const QString PATH_LIST_DELIM;
extern const QString PYTHON_MAIN_PATH;
extern const QString PYTHON_MAIN_BIN_PATH;
extern const bool DONT_CHECK_FOR_UPDATES;

#if defined(__APPLE__)
extern const QString PYTHON_SITE_PACKAGES;
extern const QString PYTHON_MAIN_PREFIX;
extern const QString PYTHON_MAIN_BIN_PATH;
extern const QString PYTHON_LIB_PATH;
#endif
extern const QStringList PYTHON_SYS_PATHS;

#if !defined(_WIN32) && !defined(__APPLE__)
extern const QString sigil_extra_root;
extern const QString hunspell_dicts_override;
extern const QString force_sigil_darkmode_palette;
extern const QString sigil_share_root;
extern const bool dicts_are_bundled;
extern const QString extra_dict_dirs;
extern const QString mathjax3_dir;
extern const QString virt_python_bin;
extern const QString PYTHON_MAIN_PREFIX;
extern const QString PYTHON_LIB_PATH;
extern const bool APPIMAGE_BUILD;
#endif

#endif // SG_CONSTANTS_H
