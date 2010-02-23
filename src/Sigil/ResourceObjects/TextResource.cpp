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
#include "TextResource.h"
#include "../Misc/Utility.h"

TextResource::TextResource( const QString &fullfilepath, QHash< QString, Resource* > *hash_owner, QObject *parent )
    : 
    Resource( fullfilepath, hash_owner, parent ),
    m_TextDocument( new QTextDocument( this ) )
{
    m_TextDocument->setDocumentLayout( new QPlainTextDocumentLayout( m_TextDocument ) );
}

void TextResource::SetText( const QString& text )
{
    m_TextDocument->setPlainText( text );
    m_TextDocument->setModified( false );
}

// Make sure to get a read lock externally before calling this function!
const QTextDocument& TextResource::GetTextDocumentForReading()
{
    Q_ASSERT( m_TextDocument );

    return *m_TextDocument;
}


// Make sure to get a write lock externally before calling this function!
QTextDocument& TextResource::GetTextDocumentForWriting()
{
    Q_ASSERT( m_TextDocument );

    return *m_TextDocument;    
}


void TextResource::SaveToDisk()
{
    QWriteLocker locker( &m_ReadWriteLock );

    Q_ASSERT( m_TextDocument );

    if ( !m_TextDocument->isModified() )

        return;

    Utility::WriteUnicodeTextFile( m_TextDocument->toPlainText(), m_FullFilePath );

    emit ResourceUpdatedOnDisk();

    m_TextDocument->setModified( false );
}


void TextResource::InitialLoad()
{
    QWriteLocker locker( &m_ReadWriteLock );

    Q_ASSERT( m_TextDocument );

    if ( m_TextDocument->toPlainText().isEmpty() )

        m_TextDocument->setPlainText( Utility::ReadUnicodeTextFile( GetFullPath() ) );
}


Resource::ResourceType TextResource::Type() const
{
    return Resource::TextResource;
}


