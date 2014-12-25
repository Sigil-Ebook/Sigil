/************************************************************************
**
**  Copyright (C) 2014 Marek Gibek
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

#include <boost/bind/bind.hpp>

#include <QtCore/QtCore>
#include <QtCore/QString>
#include <QtConcurrent/QtConcurrent>

#include "ResourceObjects/HTMLResource.h"
#include "SourceUpdates/CleanContentUpdates.h"

void CleanContentUpdates::JoinParagraphsInAllFiles(const QList<HTMLResource *> &html_resources)
{
    QtConcurrent::blockingMap(html_resources, JoinParagraphsInOneFile);
}

void CleanContentUpdates::JoinParagraphsInOneFile(HTMLResource *html_resource)
{
    Q_ASSERT(html_resource);
    QWriteLocker locker(&html_resource->GetLock());
    QString text = html_resource->GetText();
    html_resource->SetText(text);
}
