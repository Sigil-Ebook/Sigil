/************************************************************************
**
**  Copyright (C) 2023-2025 Kevin B. Hendricks, Stratford Ontario Canada
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

#include <QDir>
#include <QString>
#include <QStringList>
#include <QWebEngineProfile>
#include <QWebEngineSettings>

#include "Misc/Utility.h"
#include "Misc/SettingsStore.h"
#include "Misc/URLSchemeHandler.h"
#include "Misc/URLInterceptor.h"
#include "Misc/WebProfileMgr.h"

WebProfileMgr *WebProfileMgr::m_instance = 0;

WebProfileMgr *WebProfileMgr::instance()
{
    if (m_instance == 0) {
        m_instance = new WebProfileMgr();
    }

    return m_instance;
}

#if 0
void WebProfileMgr::FlushDiskCaches()
{
    if (m_preview_profile) {
        m_preview_profile->removeAllUrlSchemeHandlers();
        m_preview_profile->deleteLater();
    }
    if (m_onetime_profile) {
        m_onetime_profile->deleteLater();
    }
}

void WebProfileMgr::CleanUpForExit()
{
    QDir dcp(m_disk_cache_path);
    if (dcp.exists()) {
        dcp.removeRecursively();
    }
    QDir ecp(m_extra_cache_path);
    if (ecp.exists()) {
        ecp.removeRecursively();
    }
}
#endif

QWebEngineProfile*  WebProfileMgr::GetPreviewProfile()
{
    return m_preview_profile;
}

QWebEngineProfile* WebProfileMgr::GetOneTimeProfile()
{
    return m_onetime_profile;
}


void WebProfileMgr::InitializeDefaultSettings(QWebEngineSettings* web_settings)
{
    SettingsStore ss;
    SettingsStore::PreviewAppearance PVAppearance = ss.previewAppearance();
    web_settings->setFontSize(QWebEngineSettings::DefaultFontSize, PVAppearance.font_size);
    web_settings->setFontFamily(QWebEngineSettings::StandardFont, PVAppearance.font_family_standard);
    web_settings->setFontFamily(QWebEngineSettings::SerifFont, PVAppearance.font_family_serif);
    web_settings->setFontFamily(QWebEngineSettings::SansSerifFont, PVAppearance.font_family_sans_serif);
    
    web_settings->setAttribute(QWebEngineSettings::AutoLoadImages, true);
    web_settings->setAttribute(QWebEngineSettings::JavascriptEnabled, false);
    web_settings->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, false);
    web_settings->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, false);
    web_settings->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, false);
    web_settings->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, false);
    web_settings->setAttribute(QWebEngineSettings::PluginsEnabled, false);
    web_settings->setAttribute(QWebEngineSettings::AutoLoadIconsForPage, false);
    web_settings->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, false);
    web_settings->setAttribute(QWebEngineSettings::AllowRunningInsecureContent, false);
    web_settings->setAttribute(QWebEngineSettings::XSSAuditingEnabled, true);
    web_settings->setAttribute(QWebEngineSettings::AllowGeolocationOnInsecureOrigins, false);
    web_settings->setAttribute(QWebEngineSettings::ScreenCaptureEnabled, false);
    web_settings->setAttribute(QWebEngineSettings::LocalStorageEnabled, false);
    web_settings->setAttribute(QWebEngineSettings::AllowWindowActivationFromJavaScript, false);
    web_settings->setUnknownUrlSchemePolicy(QWebEngineSettings::DisallowUnknownUrlSchemes);
    web_settings->setAttribute(QWebEngineSettings::PlaybackRequiresUserGesture, true);
    web_settings->setAttribute(QWebEngineSettings::JavascriptCanPaste, false);
    web_settings->setAttribute(QWebEngineSettings::DnsPrefetchEnabled, false);
    web_settings->setAttribute(QWebEngineSettings::PdfViewerEnabled, false);
}


WebProfileMgr::WebProfileMgr()
{
    // Create URLSchemeHandler and URLInterceptor
    m_URLhandler = new URLSchemeHandler();
    m_URLint = new URLInterceptor();

    SettingsStore ss;

    // create local storage path if needed
    QString localStorePath = Utility::DefinePrefsDir() + "/local-storage/";
    QDir storageDir(localStorePath);
    if (!storageDir.exists()) {
        storageDir.mkpath(localStorePath);
    }

#if 0
    // create a place for Caches if needed
    // disable as this is very broken
    QString PreviewCachePath = Utility::DefinePrefsDir() + "/Preview-Cache/";
    QDir cacheDir(PreviewCachePath);
    if (!cacheDir.exists()) {
        cacheDir.mkpath(PreviewCachePath);
    }
#endif

    // Preview Profile
    // ---------------
    // create the profile for Preview

    // we may need to give this profile a unique storage name otherwise cache
    // is never cleared on Windows by a second or third instance of Sigil
    
    // m_preview_profile = new QWebEngineProfile(QString("Preview-") + Utility::CreateUUID(), nullptr);
    // m_preview_profile->setCachePath(PreviewCachePath);
    // m_preview_profile->setHttpCacheType(QWebEngineProfile::DiskHttpCache);
    m_preview_profile = new QWebEngineProfile();
    m_preview_profile->setPersistentStoragePath(localStorePath);
    // qDebug() << "WebProfileMgr - StorageName: " << m_preview_profile->storageName();
    // qDebug() << "WebProfileMgr - CachePath: " << m_preview_profile->cachePath();
    // m_disk_cache_path = m_preview_profile->cachePath();
    // m_extra_cache_path = Utility::DefinePrefsDir() + "/QtWebEngine/" + m_preview_profile->storageName();
    // m_preview_profile->setSpellCheckEnabled(false); // setting to false actually generates warnings!
    
    InitializeDefaultSettings(m_preview_profile->settings());
    m_preview_profile->settings()->setDefaultTextEncoding("UTF-8");  
    m_preview_profile->settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    m_preview_profile->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, true);
    m_preview_profile->settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);
    m_preview_profile->settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    m_preview_profile->settings()->setAttribute(QWebEngineSettings::PdfViewerEnabled, true);
    m_preview_profile->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, (ss.javascriptOn() == 1));
    m_preview_profile->settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, (ss.javascriptOn() == 1));
    m_preview_profile->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, (ss.remoteOn() == 1));
    m_preview_profile->settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, false);
    
    // Use both our URLInterceptor and our URLSchemeHandler
    m_preview_profile->installUrlSchemeHandler("sigil", m_URLhandler);
    m_preview_profile->setUrlRequestInterceptor(m_URLint);


    // OneTime Profile
    // ---------------
    // create the profile for OneTime
    m_onetime_profile = new QWebEngineProfile();
    InitializeDefaultSettings(m_onetime_profile->settings());
    // m_onetime_profile->setSpellCheckEnabled(false); // setting to false actually generates warnings!
    m_onetime_profile->settings()->setDefaultTextEncoding("UTF-8");  
    m_onetime_profile->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, true);
    m_onetime_profile->settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    m_onetime_profile->settings()->setAttribute(QWebEngineSettings::PdfViewerEnabled, true);
    
    // Unfortunately the PdfView used for PdfTab now requires both java and LocalStorage work
    m_onetime_profile->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, (ss.javascriptOn() == 1));
    m_onetime_profile->settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    m_onetime_profile->setPersistentStoragePath(localStorePath);

    // Use URLInterceptor for protection
    m_onetime_profile->setUrlRequestInterceptor(m_URLint);


    // Default Profile
    // ---------------
    // initialize the defaultProfile to be restrictive for security
    QWebEngineSettings *web_settings = QWebEngineProfile::defaultProfile()->settings();
    InitializeDefaultSettings(web_settings);
    // QWebEngineProfile::defaultProfile()->setSpellCheckEnabled(false); // setting to false actually generates warnings!
    // Use URLInterceptor for protection
    QWebEngineProfile::defaultProfile()->setUrlRequestInterceptor(m_URLint);

}

