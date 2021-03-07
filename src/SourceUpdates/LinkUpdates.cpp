/************************************************************************
**
**  Copyright (C) 2015-2020 Kevin B. Hendricks, Stratford, Ontario Canada
**  Copyright (C) 2012      Dave Heiland
**  Copyright (C) 2012      John Schember <john@nachtimwald.com>
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
#include <QRegularExpression>

#include "ResourceObjects/HTMLResource.h"
#include "Misc/Utility.h"
#include "Parsers/GumboInterface.h"
#include "BookManipulation/CleanSource.h"
#include "sigil_constants.h"
#include "SourceUpdates/LinkUpdates.h"

void LinkUpdates::UpdateLinksInAllFiles(const QList<HTMLResource *> &html_resources, const QList<QString> new_stylesheets)
{
    QtConcurrent::blockingMap(html_resources, std::bind(UpdateLinksInOneFile, std::placeholders::_1, new_stylesheets));
}

void LinkUpdates::UpdateLinksInOneFile(HTMLResource *html_resource, QList<QString> new_stylesheets)
{
    Q_ASSERT(html_resource);

    QString newcsslinks;
    // build the new stylesheet links new_stylesheets is a list of stylesheet bookpaths
    foreach(QString stylesheet, new_stylesheets) {
        QString ahref = Utility::buildRelativePath(html_resource->GetRelativePath(), stylesheet);
        ahref = Utility::URLEncodePath(ahref);
        newcsslinks += "  <link href=\"" + ahref + "\" type=\"text/css\" rel=\"stylesheet\"/>\n";
    }
    QWriteLocker locker(&html_resource->GetLock());
    QString newsource = html_resource->GetText();
    QString version = html_resource->GetEpubVersion();
    newsource.replace(QRegularExpression("</title>(\\s)+"), "</title>");
    GumboInterface gi = GumboInterface(newsource, version);
    gi.parse();
    newsource = gi.perform_link_updates(newcsslinks);
    html_resource->SetText(CleanSource::CharToEntity(newsource, version));
}
