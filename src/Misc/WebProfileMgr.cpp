/************************************************************************
**
**  Copyright (C) 2023 Kevin B. Hendricks, Stratford Ontario Canada
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
    web_settings->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);
    web_settings->setAttribute(QWebEngineSettings::AllowRunningInsecureContent, false);
    web_settings->setAttribute(QWebEngineSettings::XSSAuditingEnabled, true);
    web_settings->setAttribute(QWebEngineSettings::AllowGeolocationOnInsecureOrigins, false);
    web_settings->setAttribute(QWebEngineSettings::ScreenCaptureEnabled, false);
    web_settings->setAttribute(QWebEngineSettings::LocalStorageEnabled, false);
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    web_settings->setAttribute(QWebEngineSettings::AllowWindowActivationFromJavaScript, false);
#endif
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    web_settings->setUnknownUrlSchemePolicy(QWebEngineSettings::DisallowUnknownUrlSchemes);
    web_settings->setAttribute(QWebEngineSettings::PlaybackRequiresUserGesture, true);
    web_settings->setAttribute(QWebEngineSettings::JavascriptCanPaste, false);
#endif
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    web_settings->setAttribute(QWebEngineSettings::DnsPrefetchEnabled, false);
#endif
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
    web_settings->setAttribute(QWebEngineSettings::PdfViewerEnabled, false);
#endif
}


WebProfileMgr::WebProfileMgr()
{
    // Create URLSchemeHandler and URLInterceptor
    m_URLhandler = new URLSchemeHandler();
    m_URLint = new URLInterceptor();
    
    // initialize the defaultProfile to be restrictive for security
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QWebEngineSettings *web_settings = QWebEngineSettings::defaultSettings();
#else
    QWebEngineSettings *web_settings = QWebEngineProfile::defaultProfile()->settings();
#endif
    InitializeDefaultSettings(web_settings);
    // Use URLInterceptor for protection
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
    QWebEngineProfile::defaultProfile()->setUrlRequestInterceptor(m_URLint);
#else
    QWebEngineProfile::defaultProfile()->setRequestInterceptor(m_URLint);
#endif

    // create the profile for Preview
    SettingsStore ss;
    m_preview_profile = new QWebEngineProfile("Preview", nullptr);
    InitializeDefaultSettings(m_preview_profile->settings());
    m_preview_profile->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, true);
    m_preview_profile->settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);
    m_preview_profile->settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
    m_preview_profile->settings()->setAttribute(QWebEngineSettings::PdfViewerEnabled, true);
#endif
    m_preview_profile->settings()->setDefaultTextEncoding("UTF-8");  
    m_preview_profile->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, (ss.javascriptOn() == 1));
    m_preview_profile->settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, (ss.javascriptOn() == 1));
    m_preview_profile->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, (ss.remoteOn() == 1));
    m_preview_profile->settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, false);
    m_preview_profile->settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    // Enable local-storage for epub3
    QString localStorePath = Utility::DefinePrefsDir() + "/local-storage/";
    QDir storageDir(localStorePath);
    if (!storageDir.exists()) {
        storageDir.mkpath(localStorePath);
    }
    m_preview_profile->setPersistentStoragePath(localStorePath);
    // Use both our URLInterceptor and our URLSchemeHandler
    m_preview_profile->installUrlSchemeHandler("sigil", m_URLhandler);
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
    m_preview_profile->setUrlRequestInterceptor(m_URLint);
#else
    m_preview_profile->setRequestInterceptor(m_URLint);
#endif

    // create the profile for OneTime
    m_onetime_profile = new QWebEngineProfile();
    InitializeDefaultSettings(m_onetime_profile->settings());
    m_onetime_profile->settings()->setDefaultTextEncoding("UTF-8");  
    m_onetime_profile->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, true);
    m_onetime_profile->settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
    m_onetime_profile->settings()->setAttribute(QWebEngineSettings::PdfViewerEnabled, true);
#endif
    // Use URLInterceptor for protection
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
    m_onetime_profile->setUrlRequestInterceptor(m_URLint);
#else
    m_onetime_profile->setRequestInterceptor(m_URLint);
#endif

}

