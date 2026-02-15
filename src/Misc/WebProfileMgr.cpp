/************************************************************************
**
**  Copyright (C) 2023-2026 Kevin B. Hendricks, Stratford Ontario Canada
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
#include <QApplication>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
#include <QWebEngineProfileBuilder>
#endif

#include "MainUI/MainApplication.h"
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

QWebEngineProfile*  WebProfileMgr::GetPreviewProfile()
{
    // Create Dynamic Preview Profile - Off The Record
    // ---------------
    QWebEngineProfile * preview_profile = nullptr; 
    SettingsStore ss;

#if QT_VERSION < QT_VERSION_CHECK(6, 9, 0)
    preview_profile = new QWebEngineProfile();
    preview_profile->setPersistentStoragePath(m_local_storage_path);
#else
    preview_profile = QWebEngineProfileBuilder::createOffTheRecordProfile(nullptr);
    preview_profile->setHttpCacheMaximumSize(0);
    preview_profile->setHttpCacheType(QWebEngineProfile::MemoryHttpCache);
#endif
    InitializeDefaultSettings(preview_profile->settings());
    preview_profile->settings()->setDefaultTextEncoding("UTF-8");  
    preview_profile->settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    preview_profile->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, true);
    preview_profile->settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);
    preview_profile->settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    preview_profile->settings()->setAttribute(QWebEngineSettings::PdfViewerEnabled, true);
    preview_profile->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, (ss.javascriptOn() == 1));
    preview_profile->settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, (ss.javascriptOn() == 1));
    preview_profile->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, (ss.remoteOn() == 1));
    preview_profile->settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, false);
    
    // Use both our URLInterceptor and our URLSchemeHandler
    preview_profile->installUrlSchemeHandler("sigil", m_URLhandler);
    preview_profile->setUrlRequestInterceptor(m_URLint);
    preview_profile->clearHttpCache();

    return preview_profile;
}

QWebEngineProfile*  WebProfileMgr::GetInspectorProfile()
{
    return m_inspector_profile;
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

    // determine if another instance of Sigil is already running
    bool first_instance = false;
    MainApplication *mainApplication = qobject_cast<MainApplication *>(qApp);
    if (mainApplication) first_instance = mainApplication->isFirstInstance();
    
    SettingsStore ss;
    
    // create devtools storage path if needed
    m_devtools_storage_path = Utility::DefinePrefsDir() + "/local-devtools/";
    QDir devstorageDir(m_devtools_storage_path);
    if (!devstorageDir.exists()) {
        devstorageDir.mkpath(m_devtools_storage_path);
    }

    // create a place for Inspector Disk Cache if needed
    QString InspectorCachePath = Utility::DefinePrefsDir() + "/Inspector-Cache/";
    QDir inspector_cacheDir(InspectorCachePath);
    if (!inspector_cacheDir.exists()) {
        inspector_cacheDir.mkpath(InspectorCachePath);
    }

    // Create Shared Default Profile
    // -----------------------------
    // initialize the defaultProfile to be restrictive for security
    QWebEngineSettings *web_settings = QWebEngineProfile::defaultProfile()->settings();
    InitializeDefaultSettings(web_settings);
    // Use URLInterceptor for protection
    QWebEngineProfile::defaultProfile()->setUrlRequestInterceptor(m_URLint);


    // Create Shared OneTime Profile
    // -----------------------------
#if QT_VERSION < QT_VERSION_CHECK(6, 9, 0)
    m_onetime_profile = new QWebEngineProfile();
#else
    m_onetime_profile = QWebEngineProfileBuilder::createOffTheRecordProfile(nullptr);
#endif
    InitializeDefaultSettings(m_onetime_profile->settings());
    // onetime_profile->setSpellCheckEnabled(false); // setting to false actually generates warnings!
    m_onetime_profile->settings()->setDefaultTextEncoding("UTF-8");  
    m_onetime_profile->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, true);
    m_onetime_profile->settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    m_onetime_profile->settings()->setAttribute(QWebEngineSettings::PdfViewerEnabled, true);
    
    // Unfortunately the PdfView used for PdfTab now requires both java and LocalStorage work
    m_onetime_profile->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, (ss.javascriptOn() == 1));
    m_onetime_profile->settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);

    // Use URLInterceptor for protection
    m_onetime_profile->setUrlRequestInterceptor(m_URLint);


    // Create Shared Inspector Profile
    // -------------------------------
#if QT_VERSION < QT_VERSION_CHECK(6, 9, 0)
    m_inspector_profile = new QWebEngineProfile();
    m_inspector_profile->setPersistentStoragePath(m_devtools_storage_path);
#else
    QWebEngineProfileBuilder pb2;
    pb2.setCachePath(InspectorCachePath);
    pb2.setHttpCacheMaximumSize(0); // 0 - means let Qt control it
    pb2.setHttpCacheType(QWebEngineProfile::DiskHttpCache);
    pb2.setPersistentCookiesPolicy(QWebEngineProfile::NoPersistentCookies);
    pb2.setPersistentPermissionsPolicy(QWebEngineProfile::PersistentPermissionsPolicy::StoreOnDisk);
    pb2.setPersistentStoragePath(m_devtools_storage_path);
    if (first_instance) {
        m_inspector_profile = pb2.createProfile("Inspector", nullptr);
    } else {
        m_inspector_profile = QWebEngineProfileBuilder::createOffTheRecordProfile(nullptr);
    }
    // handle possible nullptr return by creating a off the record profile
    if (!m_inspector_profile) {
        m_inspector_profile = QWebEngineProfileBuilder::createOffTheRecordProfile(nullptr);
    }
#endif

    InitializeDefaultSettings(m_inspector_profile->settings());
    m_inspector_profile->settings()->setDefaultTextEncoding("UTF-8");  
    m_inspector_profile->settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    m_inspector_profile->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, true);
    m_inspector_profile->settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled,true);
    m_inspector_profile->settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    m_inspector_profile->settings()->setAttribute(QWebEngineSettings::PdfViewerEnabled, true);
    m_inspector_profile->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    m_inspector_profile->settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, true);
    m_inspector_profile->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
    m_inspector_profile->settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, false);
    m_inspector_profile->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    m_inspector_profile->settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, true);
    m_inspector_profile->settings()->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, true);
    m_inspector_profile->settings()->setAttribute(QWebEngineSettings::AllowWindowActivationFromJavaScript, true);
    m_inspector_profile->settings()->setAttribute(QWebEngineSettings::JavascriptCanPaste, true);
    // Use both our URLInterceptor and our URLSchemeHandler
    m_inspector_profile->installUrlSchemeHandler("sigil", m_URLhandler);
    m_inspector_profile->setUrlRequestInterceptor(m_URLint);
    m_inspector_profile->clearHttpCache();
}
