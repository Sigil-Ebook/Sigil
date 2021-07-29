/************************************************************************
**
**  Copyright (C) 2020-2021  Kevin B. Hendricks, Stratford, ON, Canada
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

#include <QApplication>
#include <QString>
#include <QByteArray>
#include <QUrl>
#include <QWebEngineUrlRequestJob>
#include <QWebEngineUrlSchemeHandler>
#include <QBuffer>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include "MainUI/MainApplication.h"
#include "Misc/MediaTypes.h"
#include "Misc/Utility.h"
#include "Misc/URLSchemeHandler.h"


#define DBG if(0)

static const QStringList REDIRECT = QStringList() << "audio/mp4" << "video/mp4" << "audio/mpeg";

URLSchemeHandler::URLSchemeHandler(QObject *parent)
    : QWebEngineUrlSchemeHandler(parent)
{
}


void URLSchemeHandler::requestStarted(QWebEngineUrlRequestJob *request)
{
    DBG qDebug() << "In URLSchemeHandler with url: " << request->requestUrl();
    DBG qDebug() << "In URLSchemeHandler with method: " << request->requestMethod();
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    DBG qDebug() << "In URLSchemeHandler with initiator: " << request->initiator();
#endif

    QUrl url = request->requestUrl();
    QByteArray data;
    QString content_type;  // must NOT include ";charset=UTF-8"
    if (url.hasQuery() && url.query().contains("sigilpreview=")) {
        QString key = url.query();
        key = key.mid(key.indexOf("=")+1);
        content_type = QString("application/xhtml+xml");
        MainApplication *mainApplication = qobject_cast<MainApplication *>(qApp);
        QString xhtml = mainApplication->loadFromPreviewCache(key);
        data = xhtml.toUtf8();
    } else {
        QUrl fileurl("file://" + url.path());
        QString local_file =  fileurl.toLocalFile();
        QFileInfo fi(local_file);
        if (fi.exists()) {
            QString mt = MediaTypes::instance()->GetMediaTypeFromExtension(fi.suffix().toLower(), "");
            content_type = mt;

            // Work around bug in QtWebEngine when using custom schemes that load audio and video resources
            // that require proprietary codecs as WebEngine's internal ffmpeg will ask for an exact partial
            // range instead of the entire file (as if it is streaming from a webserver that supports partial ranges.)
            //
            // This results in the following failures that only happen with custom schemes:
            // In both cases FFmpeg errors are emitted to the log when attempting to play the media files that use
            // proprietary codecs:
            //
            //     MediaEvent: MEDIA_ERROR_LOG_ENTRY {"error":"FFmpegDemuxer: open context failed"}
            //     MediaEvent: PIPELINE_ERROR DEMUXER_ERROR_COULD_NOT_OPEN
            // or
            //     MediaEvent: MEDIA_ERROR_LOG_ENTRY {"error":"FFmpegDemuxer: data source error"}
            //     MediaEvent: PIPELINE_ERROR PIPELINE_ERROR_READ
            //
            // Since filling partial requests does not seem feasible in QtWebEngine without a whole
            // lot of effort, redirect them to use the url file: scheme so that the entire file gets requested
            
            if (REDIRECT.contains(mt) && url.scheme() == "sigil") {
                request->redirect(fileurl);
                return;
            }
            
            QFile file(local_file);
            if (file.open(QIODevice::ReadOnly)) {
                data = file.readAll();
                file.close();
            }
        }
    }
    if (!data.isEmpty()) {
        QBuffer *replybuffer = new QBuffer();
        replybuffer->setData(data);
        replybuffer->open(QIODevice::ReadOnly);
        connect(request, SIGNAL(destroyed()), replybuffer, SLOT(deleteLater()));
        request->reply(content_type.toUtf8(), replybuffer);
    } else {
        qDebug() << "URLSchemeHandler failed request for: " << url;
        request->fail(QWebEngineUrlRequestJob::UrlNotFound);
    }
}
