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

#include <stdafx.h>
#include "NCXModel.h"
#include "ResourceObjects/NCXResource.h"
#include "Misc/Utility.h"


NCXModel::NCXModel( QWidget *parent )
    : 
    QStandardItemModel( parent ),
    m_Book( NULL ),
    m_RefreshInProgress( false ),
    m_NcxRootWatcher( *new QFutureWatcher< NCXModel::NCXEntry >( this ) )
{
    connect( &m_NcxRootWatcher, SIGNAL( finished() ), this, SLOT( RefreshEnd() ) );
}

void NCXModel::SetBook( QSharedPointer< Book > book )
{
    {
        // We need to make sure we don't step on the toes of GetNCXText
        QMutexLocker book_lock( &m_UsingBookMutex );

        m_Book = book;
    }

    Refresh();
}


// Should only ever be called from the main thread!
// This is because access to m_RefreshInProgress is not guarded.
// We *could* guard it, but there's no need to call this func
// from several threads so we just disallow it with the assert.
void NCXModel::Refresh()
{
    Q_ASSERT( QThread::currentThread() == QApplication::instance()->thread() );

    if ( m_RefreshInProgress )

        return;

    m_RefreshInProgress = true;

    m_NcxRootWatcher.setFuture( QtConcurrent::run( this, &NCXModel::GetRootNCXEntry ) );
}


void NCXModel::RefreshEnd()
{
    BuildModel( m_NcxRootWatcher.result() );

    m_RefreshInProgress = false;
}


NCXModel::NCXEntry NCXModel::GetRootNCXEntry()
{
    return ParseNCX( GetNCXText() );
}


QString NCXModel::GetNCXText()
{
    QMutexLocker book_lock( &m_UsingBookMutex );

    NCXResource &ncx = m_Book->GetNCX();

    QReadLocker locker( &ncx.GetLock() );

    return ncx.GetTextDocumentForReading().toPlainText();
}


NCXModel::NCXEntry NCXModel::ParseNCX( const QString &ncx_source )
{
    QXmlStreamReader ncx( ncx_source );

    bool in_navmap = false;

    NCXModel::NCXEntry root;
    root.is_root = true;

    while ( !ncx.atEnd() ) 
    {
        ncx.readNext();

        if ( ncx.isStartElement() ) 
        {
            if ( !in_navmap )
            { 
                if ( ncx.name() == "navMap" )

                    in_navmap = true;

                continue;
            }

            if ( ncx.name() == "navPoint" )
            
                root.children.append( ParseNavPoint( ncx ) );            
        }

        else if ( ncx.isEndElement() && 
                  ncx.name() == "navMap" )
        {
            break;
        }
    }

    if ( ncx.hasError() )
    {
        NCXModel::NCXEntry empty;
        empty.is_root = true;

        return empty;
    }

    return root;
}


NCXModel::NCXEntry NCXModel::ParseNavPoint( QXmlStreamReader &ncx )
{
    NCXModel::NCXEntry current;

    while ( !ncx.atEnd() ) 
    {
        ncx.readNext();

        // TODO: use isStartElement etc everywhere
        if ( ncx.isStartElement() ) 
        {
            if ( ncx.name() == "text" )
            {                
                while ( !ncx.isCharacters() )
                {
                    ncx.readNext();
                }

                // TODO: check whether this text() string is unescaped
                current.text = ncx.text().toString();
            }

            else if ( ncx.name() == "content" )
            {
                current.target = Utility::URLDecodePath( ncx.attributes().value( "", "src" ).toString() );
            }

            else if ( ncx.name() == "navPoint" )
            {
                current.children.append( ParseNavPoint( ncx ) );    
            }
        }

        else if ( ncx.isEndElement() && 
                  ncx.name() == "navPoint" )
        {
            break;
        }
    }

    return current;
}


void NCXModel::BuildModel( const NCXModel::NCXEntry &root_entry )
{
    if ( root_entry.children.isEmpty() )

        return;

    clear();

    foreach ( const NCXModel::NCXEntry &child_entry, root_entry.children )
    {
        AddEntryToParentItem( child_entry, invisibleRootItem() );
    }
}


void NCXModel::AddEntryToParentItem( const NCXEntry &entry, QStandardItem *parent )
{
    Q_ASSERT( parent );

    QStandardItem *item = new QStandardItem( entry.text );
    // TODO: the target should be processed first
    item->setData( entry.target );
    item->setEditable( false );
    item->setDragEnabled( false );
    item->setDropEnabled( false );

    parent->appendRow( item );
    
    foreach ( const NCXModel::NCXEntry &child_entry, entry.children )
    {
        AddEntryToParentItem( child_entry, item );
    }
}



