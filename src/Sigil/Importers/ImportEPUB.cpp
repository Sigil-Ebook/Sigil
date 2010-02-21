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
#include "ImportEPUB.h"
#include "../BookManipulation/CleanSource.h"
#include "../Misc/Utility.h"
#include "../Misc/HTMLEncodingResolver.h"
#include "../SourceUpdates/PerformHTMLUpdates.h"
#include "../SourceUpdates/PerformCSSUpdates.h"
#include "../BookManipulation/XHTMLDoc.h"
#include <QDomDocument>
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/CSSResource.h"




// Constructor;
// The parameter is the file to be imported
ImportEPUB::ImportEPUB( const QString &fullfilepath )
    : ImportOEBPS( fullfilepath )
{

}


// Reads and parses the file 
// and returns the created Book
QSharedPointer< Book > ImportEPUB::GetBook()
{
    if ( !Utility::IsFileReadable( m_FullFilePath ) )

        boost_throw( CannotReadFile() << errinfo_file_read( m_FullFilePath.toStdString() ) );

    // These read the EPUB file
    ExtractContainer();
    LocateOPF();
    ReadOPF();

    // These mutate the m_Book object
    LoadMetadata();

    CleanAndUpdateFiles( LoadFolderStructure() );

    return m_Book;
}

void ImportEPUB::CleanAndUpdateFiles( const QHash< QString, QString > &updates )
{
    QHash< QString, QString > html_updates;
    QHash< QString, QString > css_updates;
    tie( html_updates, css_updates ) = PerformHTMLUpdates::SeparateHTMLAndCSSUpdates( updates );

    QList< HTMLResource* > html_resources;
    QList< CSSResource* > css_resources;

    QList< Resource* > all_files = m_Book->mainfolder.GetResourceList();
    int num_files = all_files.count();

    for ( int i = 0; i < num_files; ++i )
    {
        Resource *resource = all_files.at( i );

        if ( resource->Type() == Resource::HTMLResource )

            html_resources.append( qobject_cast< HTMLResource* >( resource ) );

        else if ( resource->Type() == Resource::CSSResource )   

            css_resources.append( qobject_cast< CSSResource* >( resource ) );
    }

    QFutureSynchronizer<void> sync;
    sync.addFuture( QtConcurrent::map( html_resources, boost::bind( CleanAndUpdateOneHTMLFile, _1, html_updates, css_updates ) ) );
    sync.addFuture( QtConcurrent::map( css_resources, boost::bind( UpdateOneCSSFile, _1, css_updates ) ) );
    sync.waitForFinished();
}


// Normally, this would be two functions. But making it
// just one saves us expensive (and unnecessary) loading
// from disk. Here we just load it once, do everything
// and then save back to disk.
void ImportEPUB::CleanAndUpdateOneHTMLFile( HTMLResource* html_resource,
                                            const QHash< QString, QString > &html_updates,
                                            const QHash< QString, QString > &css_updates )
{
    QString source = CleanSource::Clean( HTMLEncodingResolver::ReadHTMLFile( html_resource->GetFullPath() ) );
    html_resource->SetDomDocument( PerformHTMLUpdates( source, html_updates, css_updates )() );
}

void ImportEPUB::UpdateOneCSSFile( CSSResource* css_resource, const QHash< QString, QString > &css_updates )
{
    QString source = Utility::ReadUnicodeTextFile( css_resource->GetFullPath() );
    source = PerformCSSUpdates( source, css_updates )();
    css_resource->SetText( source );
}


// Loads the referenced files into the main folder of the book.
// Returns a hash with keys being old references (URLs) to resources,
// and values being the new references to those resources.
QHash< QString, QString > ImportEPUB::LoadFolderStructure()
{ 
    QList< QString > keys = m_Files.keys();
    int num_files = keys.count();

    QFutureSynchronizer< tuple< QString, QString > > sync;

    for ( int i = 0; i < num_files; ++i )
    {   
        sync.addFuture( QtConcurrent::run( this, &ImportEPUB::LoadOneFile, keys.at( i ) ) );   
    }

    sync.waitForFinished();

    QList< QFuture< tuple< QString, QString > > > futures = sync.futures();
    int num_futures = futures.count();

    QHash< QString, QString > updates;

    for ( int i = 0; i < num_futures; ++i )
    {
        tuple< QString, QString > result = futures.at( i ).result();
        updates[ result.get< 0 >() ] = result.get< 1 >();
    }   

    return updates;
}


tuple< QString, QString > ImportEPUB::LoadOneFile( const QString &key )
{
    QString path         = m_Files.value( key );
    QString fullfilepath = QFileInfo( m_OPFFilePath ).absolutePath() + "/" + path;

    Resource &resource = m_Book->mainfolder.AddContentFileToFolder( fullfilepath, m_ReadingOrderIds.indexOf( key ) );
    QString newpath = "../" + resource.GetRelativePathToOEBPS(); 

    return make_tuple( path, newpath );
}


