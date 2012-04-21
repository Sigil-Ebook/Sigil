/************************************************************************
**
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

#include <QtCore/QFile>
#include <QtCore/QString>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtGui/QPlainTextDocumentLayout>
#include <QtGui/QTextDocument>

#include "Misc/Utility.h"
#include "ResourceObjects/TextResource.h"

TextResource::TextResource( const QString &fullfilepath, QObject *parent )
    : 
    Resource( fullfilepath, parent ),
    m_CacheInUse( false ),
    m_TextDocument( new QTextDocument( this ) )
{
    m_TextDocument->setDocumentLayout( new QPlainTextDocumentLayout( m_TextDocument ) );
 
    connect( m_TextDocument, SIGNAL( contentsChanged() ), this, SIGNAL( Modified() ) );
}


QString TextResource::GetText() const
{
    QMutexLocker locker( &m_CacheAccessMutex );

    if ( m_CacheInUse )

        return m_Cache; 

    return m_TextDocument->toPlainText();
}


void TextResource::SetText( const QString& text )
{
    //   We need to delay updating the QTextDocument if SetText has
    // been called from something other than the main GUI thread. Why?
    // Because a CodeView is probably connected to the text document,
    // and if we update it from a non-GUI thread, it will notify the 
    // CodeView base class to update as well and that will crash us since
    // the base class derives from QWidget (and those can only be updated
    // in the GUI thread).
    //   So we cache the text update into m_Cache and update the QTextDocument
    // when we return to the GUI thread. The single-shot timer makes sure
    // of that.
    if ( QThread::currentThread() == QApplication::instance()->thread() )
    {
        SetTextInternal( text );          
    }

    else
    {
        QMutexLocker locker( &m_CacheAccessMutex );

        m_Cache = text;

        // We want to make sure we schedule only one delayed update
        if ( !m_CacheInUse )
        {
            m_CacheInUse = true;        
            QTimer::singleShot( 0, this, SLOT( DelayedUpdateToTextDocument() ) );  
        }
    }
}


QTextDocument& TextResource::GetTextDocumentForWriting()
{
    Q_ASSERT( m_TextDocument );

    return *m_TextDocument;
}


void TextResource::SaveToDisk( bool book_wide_save )
{
    // We can't perform the document modified check
    // here because that causes problems with epub export
    // when the user has not changed the text file.
    // (some text files have placeholder text on disk)

    // Just in case there was no initial load until now.
    InitialLoad();

    {
        QWriteLocker locker( &GetLock() );

        Utility::WriteUnicodeTextFile( GetText(), GetFullPath() );
    }

    if ( !book_wide_save )

        emit ResourceUpdatedOnDisk();

    m_TextDocument->setModified( false );
}


void TextResource::InitialLoad()
{
    QWriteLocker locker( &GetLock() );

    Q_ASSERT( m_TextDocument );

    if ( m_TextDocument->toPlainText().isEmpty() && QFile::exists( GetFullPath() ) )

        m_TextDocument->setPlainText( Utility::ReadUnicodeTextFile( GetFullPath() ) );
}


Resource::ResourceType TextResource::Type() const
{
    return Resource::TextResourceType;
}


void TextResource::DelayedUpdateToTextDocument()
{
    QMutexLocker locker( &m_CacheAccessMutex );

    if ( !m_CacheInUse )

        return;
    
    SetTextInternal( m_Cache );
}


void TextResource::SetTextInternal( const QString &text )
{
    m_TextDocument->setPlainText( text );
    m_TextDocument->setModified( false );

    // Clear anything left in the cache
    m_Cache = "";
    m_CacheInUse = false;
}
