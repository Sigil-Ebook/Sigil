/************************************************************************
**
**  Copyright (C) 2023  Kevin B. Hendricks, Stratford, ON, Canada
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
#ifndef WEBPROFILEMGR_H
#define WEBPROFILEMGR_H

#include <QCoreApplication>

/**
 * Singleton.
 *
 * WebProfileMgr
 */

class URLInterceptor;
class URLSchemeHandler;
class QWebEngineProfile;

class WebProfileMgr
{

public:

    static WebProfileMgr *instance();
    QWebEngineProfile* GetPreviewProfile();
    QWebEngineProfile* GetOneTimeProfile();
    
private:

    WebProfileMgr();
    void InitializeDefaultSettings(QWebEngineSettings* web_settings);
    URLInterceptor* m_URLint;
    URLSchemeHandler* m_URLhandler;

    QWebEngineProfile* m_preview_profile;
    QWebEngineProfile* m_onetime_profile;
    static WebProfileMgr *m_instance;
};

#endif // WEBPROFILEMGR_H
