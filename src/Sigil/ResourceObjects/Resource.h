/************************************************************************
**
**  Copyright (C) 2009  Strahinja Markovic
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
#ifndef RESOURCE_H
#define RESOURCE_H

#include <QObject>
#include <QMutex>
#include <QIcon>
#include <QUrl>

class Resource : public QObject 
{
    Q_OBJECT

public:

    enum ResourceType
    {
        GenericResource,
        TextResource,
        HTMLResource,
        CSSResource,
        XPGTResource,
        ImageResource,
        FontResource
    };

    Resource( const QString &fullfilepath, QHash< QString, Resource* > *hash_owner, QObject *parent = NULL );

    QString GetIdentifier() const;

    QString Filename() const;

    QString GetRelativePathToOEBPS() const;

    QUrl GetBaseUrl() const;

    QIcon Icon() const;

    bool RenameTo( const QString &new_filename );

    bool Delete();    

    virtual ResourceType Type() const;

signals:

    void RenamedTo( QString );

    void Deleted();

protected:

    QString m_Identifier;

    QString m_FullFilePath;

    QMutex m_AccessMutex;

private:

    QHash< QString, Resource* > &m_HashOwner;
};

#endif // RESOURCE_H
