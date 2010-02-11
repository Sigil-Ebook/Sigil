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

#include <stdafx.h>
#include "Resource.h"
#include "../Misc/Utility.h"


Resource::Resource( const QString &fullfilepath, QHash< QString, Resource* > *hash_owner, QObject *parent )
    : 
    QObject( parent ),
    m_Identifier( Utility::CreateUUID() ),
    m_FullFilePath( fullfilepath ),
    m_HashOwner( *hash_owner )
{

}


QString Resource::GetIdentifier() const
{
    return m_Identifier;
}


QString Resource::Filename() const
{
    return QFileInfo( m_FullFilePath ).fileName();
}


QString Resource::GetRelativePathToOEBPS() const
{
    return QFileInfo( m_FullFilePath ).dir().dirName() + "/" + Filename();
}


QUrl Resource::GetBaseUrl() const
{
    return QUrl::fromLocalFile( QFileInfo( m_FullFilePath ).absolutePath() + "/" );
}


QIcon Resource::Icon() const
{
    return QFileIconProvider().icon( QFileInfo( m_FullFilePath ) );
}


bool Resource::RenameTo( const QString &new_filename )
{
    QWriteLocker locker( &m_ReadWriteLock );

    bool successful = Utility::RenameFile( m_FullFilePath, QFileInfo( m_FullFilePath ).absolutePath() + "/" + new_filename );

    if ( successful )
    {
        emit RenamedTo( new_filename );
    }

    return successful;
}

bool Resource::Delete()
{
    QWriteLocker locker( &m_ReadWriteLock );

    bool successful = Utility::DeleteFile( m_FullFilePath );

    if ( successful )
    {
        m_HashOwner.remove( m_Identifier );

        emit Deleted();

        deleteLater();
    }

    return successful;
}


Resource::ResourceType Resource::Type() const
{
    return Resource::GenericResource;
}

