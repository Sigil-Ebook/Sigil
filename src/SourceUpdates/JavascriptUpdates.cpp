/************************************************************************
**
**  Copyright (C) 2021 Kevin B. Hendricks, Stratford, Ontario Canada
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

#include <functional>

#include <QtCore/QtCore>
#include <QtCore/QString>
#include <QtConcurrent/QtConcurrent>

#include "ResourceObjects/HTMLResource.h"
#include "Misc/Utility.h"
#include "Parsers/GumboInterface.h"
#include "BookManipulation/CleanSource.h"
#include "sigil_constants.h"
#include "SourceUpdates/JavascriptUpdates.h"

void JavascriptUpdates::UpdateJavascriptsInAllFiles(const QList<HTMLResource *> &html_resources, const QList<QString> new_javascripts)
{
    QtConcurrent::blockingMap(html_resources, std::bind(UpdateJavascriptsInOneFile, std::placeholders::_1, new_javascripts));
}

void JavascriptUpdates::UpdateJavascriptsInOneFile(HTMLResource *html_resource, QList<QString> new_javascripts)
{
    Q_ASSERT(html_resource);

    QString newjslinks;
    // build the new javascript links, new_javascripts is a list of javascript bookpaths
    foreach(QString javascript, new_javascripts) {
        QString ahref = Utility::buildRelativePath(html_resource->GetRelativePath(), javascript);
        ahref = Utility::URLEncodePath(ahref);
        newjslinks += "  <script type=\"text/javascript\" src=\"" + ahref + "\" ></script>\n";
    }
    QWriteLocker locker(&html_resource->GetLock());
    QString newsource = html_resource->GetText();
    QString version = html_resource->GetEpubVersion();
    GumboInterface gi = GumboInterface(newsource, version);
    gi.parse();
    newsource = gi.perform_javascript_updates(newjslinks);
    html_resource->SetText(CleanSource::CharToEntity(newsource, version));
}
