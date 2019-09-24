/************************************************************************
**
**  Copyright (C) 2019  Kevin B. Hendricks, Stratford, ON, Canada
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
#ifndef MEDIATYPES_H
#define MEDIATYPES_H

#include <QCoreApplication>
#include <QHash>
class QString;
class QStringList;


/**
 * Singleton.
 *
 * MediaTypes
 */
 

class MediaTypes
{
    Q_DECLARE_TR_FUNCTIONS(MediaTypes)

public:

    static MediaTypes *instance();
    QString GetMediaTypeFromExtension(const QString &extension, const QString &fallback = "");
    QString GetGroupFromMediaType(const QString &mediatype, const QString &fallback = "");
    QString GetResourceDescFromMediaType(const QString &mediatype, const QString &fallback = "");

private:

    MediaTypes();

    void SetExtToMTypeMap();

    void SetMTypeToGroupMap();

    void SetMTypeToRDescMap();

    QHash<QString, QString> m_ExtToMType;
    
    QHash<QString, QString> m_MTypeToGroup;

    QHash<QString, QString>m_MTypeToRDesc;
    
    static MediaTypes *m_instance;
};

#endif // MEDIATYPES_H
