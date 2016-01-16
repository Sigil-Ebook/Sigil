/************************************************************************
**
**  Copyright (C) 2016, Kevin B. Hendricks, Stratford, Ontario, CA
**  Copyright (C) 2009, 2010, 2011  Strahinja Markovic  <strahinja.markovic@gmail.com>
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
#ifndef PERFORMXMLUPDATES_H
#define PERFORMXMLUPDATES_H

#include <QtCore/QHash>
#include <QtCore/QStringList>

class QString;
class QStringList;

/**
 * Performs path updates on XML documents.
 */
class PerformXMLUpdates
{

public:

    PerformXMLUpdates(const QString &source,
                      const QHash<QString, QString> &xml_updates,
                      const QString& currentpath,
                      const QString& mtype);


    QString operator()();


private:
    const QString &m_Source;
    const QHash<QString, QString> &m_XMLUpdates;
    const QString &m_CurrentPath;
    const QString & m_MediaType;
};

#endif // PERFORMXMLUPDATES_H
