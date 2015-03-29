/************************************************************************
**
**  Copyright (C) 2013  Dave Heiland
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
#ifndef XMLENTITIES_H
#define XMLENTITIES_H

#include <QtCore/QCoreApplication>
#include <QtCore/QHash>

class QString;
class QStringList;

/**
 * Singleton.
 *
 * XMLEntities routines
 */
class XMLEntities
{
    Q_DECLARE_TR_FUNCTIONS(XMLEntities)

public:
    static XMLEntities *instance();

    QString GetEntityName(ushort code);
    QString GetEntityDescription(ushort code);
    ushort GetEntityCode(const QString name);

private:
    XMLEntities();

    void SetXMLEntities();

    QHash<ushort, QString> m_EntityName;
    QHash<ushort, QString> m_EntityDescription;
    QHash<QString, ushort> m_EntityCode;

    static XMLEntities *m_instance;
};

#endif // XMLENTITIES_H
