/************************************************************************
**
**  Copyright (C) 2009, 2010, 2011  Strahinja Markovic  <strahinja.markovic@gmail.com>
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

class QString;

// These enable us to use constants defined
// in one CPP file to be used in another
extern const QString BODY_START;
extern const QString BODY_END;
extern const QString HEAD_END;
extern const QString BREAK_TAG_INSERT;
extern const QString HEADING;
extern const QString STYLE_TAG;
extern const QString VERSION_NUMBERS;
extern const QString SIGIL_VERSION;
extern const int PROGRESS_BAR_MINIMUM_DURATION;
extern const QString IMAGE_FOLDER_NAME;
extern const QString FONT_FOLDER_NAME;
extern const QString TEXT_FOLDER_NAME;
extern const QString STYLE_FOLDER_NAME;
extern const QString MISC_FOLDER_NAME;
extern const QString NOT_IN_TOC_CLASS;
extern const QString FIRST_CHAPTER_NAME; 
extern const QString FIRST_CHAPTER_PREFIX;
extern const QString OPF_FILE_NAME; 
extern const QString NCX_FILE_NAME;
extern const QString CONTAINER_XML_FILE_NAME;
extern const QStringList TEXT_EXTENSIONS;
extern const QStringList FONT_EXTENSIONS;
extern const QStringList IMAGE_EXTENSIONS;
extern const QString ENCODING_ATTRIBUTE;
extern const QString STANDALONE_ATTRIBUTE;
extern const QString VERSION_ATTRIBUTE;
extern const QString ADOBE_FONT_ALGO_ID;
extern const QString IDPF_FONT_ALGO_ID; 
extern const QString DUBLIN_CORE_NS;
extern const int XML_DECLARATION_SEARCH_PREFIX_SIZE;

extern const QString NCX_MIMETYPE;

extern const char*         XHTML_ENTITIES_DTD_ID; 
extern const unsigned int  XHTML_ENTITIES_DTD_LEN;
extern const unsigned char XHTML_ENTITIES_DTD[];  

#endif // SG_CONSTANTS_H
