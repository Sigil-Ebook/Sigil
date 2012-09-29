/************************************************************************
**
**  Copyright (C) 2012 John Schember <john@nachtimwald.com>
**  Copyright (C) 2012 Dave Heiland
**  Copyright (C) 2012 Grant Drake
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

#ifdef DEBUG
#include <QDebug>
#endif

#include "ViewEditors/ViewWebPage.h"

ViewWebPage::ViewWebPage( QObject *parent )
    : QWebPage(parent)
{
}

void ViewWebPage::javaScriptConsoleMessage( const QString &message, int lineNumber, const QString &sourceID )
{
#ifdef DEBUG
    const QString logEntry = message +" on line:" % QString::number(lineNumber) % " Source:"+ sourceID;
    qDebug() << "Javascript error: " << logEntry;
#endif
}
