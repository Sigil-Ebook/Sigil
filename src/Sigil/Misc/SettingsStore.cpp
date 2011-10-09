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
#include "constants.h"
#include <QSettings>
#include "SettingsStore.h"

SettingsStore *SettingsStore::m_instance = 0;

static const QString SETTINGS_GROUP = "user_preferences";
static const QString KEY_DEFAULT_METADATA_LANGUAGE = "default_metadata_lang";
static const QString KEY_SPLIT_VIEW_ORIENTATION = "split_view_orientation";
static const QString KEY_ZOOM_IMAGE = "zoom_image";
static const QString KEY_ZOOM_TEXT = "zoom_text";
static const QString KEY_ZOOM_WEB = "zoom_web";

SettingsStore *SettingsStore::instance()
{
    if (m_instance == 0) {
        m_instance = new SettingsStore();
    }

    return m_instance;
}

SettingsStore::~SettingsStore()
{
    writeSettings();
}

QString SettingsStore::defaultMetadataLang()
{
    return m_defaultMetadataLang;
}

Qt::Orientation SettingsStore::splitViewOrientation()
{
    return m_splitViewOrientation;
}

float SettingsStore::zoomImage()
{
    return m_zoomImage;
}

float SettingsStore::zoomText()
{
    return m_zoomText;
}

float SettingsStore::zoomWeb()
{
    return m_zoomWeb;
}

void SettingsStore::setDefaultMetadataLang(const QString &lang)
{
    m_defaultMetadataLang = lang;
}

void SettingsStore::setSplitViewOrientation(Qt::Orientation orientation)
{
    m_splitViewOrientation = orientation;
}

void SettingsStore::setZoomImage(float zoom)
{
    m_zoomImage = zoom;
}

void SettingsStore::setZoomText(float zoom)
{
    m_zoomText = zoom;
}

void SettingsStore::setZoomWeb(float zoom)
{
    m_zoomWeb = zoom;
}

void SettingsStore::triggerSettingsChanged()
{
    emit settingsChanged();
}

void SettingsStore::writeSettings()
{
    QSettings settings;
    settings.beginGroup( SETTINGS_GROUP );

    settings.setValue(KEY_DEFAULT_METADATA_LANGUAGE, m_defaultMetadataLang);
    settings.setValue(KEY_SPLIT_VIEW_ORIENTATION, m_splitViewOrientation);
    settings.setValue(KEY_ZOOM_IMAGE, m_zoomImage);
    settings.setValue(KEY_ZOOM_TEXT, m_zoomText);
    settings.setValue(KEY_ZOOM_WEB, m_zoomWeb);
}

SettingsStore::SettingsStore()
{
    readSettings();
}

void SettingsStore::readSettings()
{
    QSettings settings;
    settings.beginGroup( SETTINGS_GROUP );

    m_defaultMetadataLang = settings.value(KEY_DEFAULT_METADATA_LANGUAGE, "English").toString();
    m_splitViewOrientation = static_cast<Qt::Orientation>(settings.value(KEY_SPLIT_VIEW_ORIENTATION, Qt::Vertical).toInt());
    m_zoomImage = settings.value(KEY_ZOOM_IMAGE, ZOOM_NORMAL).toFloat();
    m_zoomText = settings.value(KEY_ZOOM_TEXT, ZOOM_NORMAL).toFloat();
    m_zoomWeb = settings.value(KEY_ZOOM_WEB, ZOOM_NORMAL).toFloat();
}
