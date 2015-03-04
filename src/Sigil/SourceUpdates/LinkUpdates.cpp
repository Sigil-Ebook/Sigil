/************************************************************************
**
**  Copyright (C) 2012 Dave Heiland
**  Copyright (C) 2012 John Schember <john@nachtimwald.com>
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
#include "Misc/GumboInterface.h"
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
    // build the new stylesheet links
    foreach(QString stylesheet, new_stylesheets) {
        newcsslinks += "<link href=\"" + stylesheet + "\" type=\"text/css\" rel=\"stylesheet\"/>\n";
    }
    QWriteLocker locker(&html_resource->GetLock());
    QString newsource = html_resource->GetText();
    GumboInterface gi = GumboInterface(newsource);
    gi.parse();
    newsource = gi.perform_link_updates(newcsslinks);
    html_resource->SetText(newsource);
}
