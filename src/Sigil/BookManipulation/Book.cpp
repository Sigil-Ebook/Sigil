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
#include "BookManipulation/Book.h"
#include "BookManipulation/XhtmlDoc.h"
#include "Misc/Utility.h"
#include "Misc/TempFolder.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/OPFResource.h"
#include "ResourceObjects/NCXResource.h"
#include "BookManipulation/CleanSource.h"
#include "SourceUpdates/AnchorUpdates.h"
#include "SourceUpdates/PerformHTMLUpdates.h"
#include "SourceUpdates/AnchorUpdates.h"
#include "SourceUpdates/UniversalUpdates.h"
#include "BookManipulation/FolderKeeper.h"
#include "XercesCppUse.h"

static const QString FIRST_CSS_NAME   = "Style0001.css";
static const QString PLACEHOLDER_TEXT = "PLACEHOLDER";
static const QString EMPTY_HTML_FILE  = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                                        "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\"\n"
                                        "    \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n\n"							
                                        "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
                                        "<head>\n"
                                        "<title></title>\n"
                                        "</head>\n"
                                        "<body>\n"

                                        // The "nbsp" is here so that the user starts writing
                                        // inside the <p> element; if it's not here, webkit
                                        // inserts text _outside_ the <p> element
                                        "<p>&nbsp;</p>\n"
                                        "</body>\n"
                                        "</html>";


Book::Book()
    : 
    m_Mainfolder( *new FolderKeeper( this ) ),
    m_IsModified( false )
{
   
}


QUrl Book::GetBaseUrl() const
{
    return QUrl::fromLocalFile( m_Mainfolder.GetFullPathToTextFolder() + "/" );
}



FolderKeeper& Book::GetFolderKeeper()
{
    return m_Mainfolder;
}


const FolderKeeper& Book::GetFolderKeeper() const
{
    return m_Mainfolder;
}



OPFResource& Book::GetOPF()
{
    return m_Mainfolder.GetOPF();
}


const OPFResource& Book::GetConstOPF() const
{
    return m_Mainfolder.GetOPF();
}


NCXResource& Book::GetNCX()
{
    return m_Mainfolder.GetNCX();
}


QString Book::GetPublicationIdentifier() const
{
    return GetConstOPF().GetMainIdentifierValue();
}


QHash< QString, QList< QVariant > > Book::GetMetadata() const
{
    return GetConstOPF().GetDCMetadata();    
}


void Book::SetMetadata( const QHash< QString, QList< QVariant > > metadata )
{
    GetOPF().SetDCMetadata( metadata );
    SetModified( true );
}


HTMLResource& Book::CreateNewHTMLFile()
{
    TempFolder tempfolder;

    QString fullfilepath = tempfolder.GetPath() + "/" + m_Mainfolder.GetUniqueFilenameVersion( FIRST_CHAPTER_NAME );

    Utility::WriteUnicodeTextFile( PLACEHOLDER_TEXT, fullfilepath );

    HTMLResource &html_resource = *qobject_cast< HTMLResource* >( 
                                        &m_Mainfolder.AddContentFileToFolder( fullfilepath ) );

    SetModified( true );
    return html_resource;
}


HTMLResource& Book::CreateEmptyHTMLFile()
{
    HTMLResource &html_resource = CreateNewHTMLFile();
    html_resource.SetDomDocument( XhtmlDoc::LoadTextIntoDocument( EMPTY_HTML_FILE ) );
    SetModified( true );
    return html_resource;
}


CSSResource& Book::CreateEmptyCSSFile()
{
    TempFolder tempfolder;

    QString fullfilepath = tempfolder.GetPath() + "/" + m_Mainfolder.GetUniqueFilenameVersion( FIRST_CSS_NAME );

    Utility::WriteUnicodeTextFile( "", fullfilepath );

    CSSResource &css_resource = *qobject_cast< CSSResource* >(
                                        &m_Mainfolder.AddContentFileToFolder( fullfilepath ) );

    SetModified( true );
    return css_resource;
}


HTMLResource& Book::CreateChapterBreakOriginalResource( const QString &content, HTMLResource& originating_resource )
{
    const QString originating_filename = originating_resource.Filename();

    int reading_order = GetOPF().GetReadingOrder( originating_resource );
    Q_ASSERT( reading_order >= 0 );

    QList< HTMLResource* > html_resources = m_Mainfolder.GetResourceTypeList< HTMLResource >( true );

    originating_resource.RenameTo( m_Mainfolder.GetUniqueFilenameVersion( FIRST_CHAPTER_NAME ) );

    HTMLResource &new_resource = CreateNewHTMLFile();
    new_resource.RenameTo( originating_filename );

    new_resource.SetDomDocument( 
        XhtmlDoc::LoadTextIntoDocument( CleanSource::Clean( content ) ) );

    html_resources.insert( reading_order, &new_resource );

    GetOPF().UpdateSpineOrder( html_resources );

    // Update references between the two new files. Since they used to be one single file we can
    // assume that each id is unique (if they aren't then the references were broken anyway).
    QList< HTMLResource* > new_files;
    new_files.append( &originating_resource );
    new_files.append( &new_resource );
    AnchorUpdates::UpdateAllAnchorsWithIDs( new_files );

    // Remove the original and new files from the list of html resources as we want to scan all
    // the other files for external references to the original file.
    html_resources.removeOne( &originating_resource );
    html_resources.removeOne( &new_resource );

    // Now, update references to the original file that are made in other files.
    // We can't assume that ids are unique in this case, and so need to use a different mechanism.
    AnchorUpdates::UpdateExternalAnchors( html_resources, Utility::URLEncodePath( originating_filename ), new_files );

    SetModified( true );
    return new_resource;
}


void Book::CreateNewChapters( const QStringList &new_chapters, HTMLResource &original_resource )
{
    int original_position = GetOPF().GetReadingOrder( original_resource );
    Q_ASSERT( original_position >= 0 );

    QString new_file_prefix = QFileInfo( original_resource.Filename() ).baseName();

    if ( new_chapters.isEmpty() )

        return;

    TempFolder tempfolder;

    QFutureSynchronizer< NewChapterResult > sync;
    QList< HTMLResource* > html_resources = m_Mainfolder.GetResourceTypeList< HTMLResource >( true );

    // A list of all the files that have not been involved in the split.
    // This will be used later when anchors are updated.
    QList< HTMLResource* > other_files = html_resources;
    other_files.removeOne( &original_resource );

    int next_reading_order;
    if ( original_position == -1 )

        next_reading_order = m_Mainfolder.GetHighestReadingOrder() + 1;

    else

        next_reading_order = original_position + 1;

    for ( int i = 0; i < new_chapters.count(); ++i )
    {
        int reading_order = next_reading_order + i;

        NewChapter chapterInfo;
        chapterInfo.source = new_chapters.at( i );
        chapterInfo.reading_order = reading_order;
        chapterInfo.temp_folder_path = tempfolder.GetPath();
        chapterInfo.new_file_prefix = new_file_prefix;
        chapterInfo.file_suffix = i + 1;

        sync.addFuture( 
            QtConcurrent::run( 
                this, 
                &Book::CreateOneNewChapter, 
                chapterInfo ) );
    }	

    sync.waitForFinished();

    QList< HTMLResource* > new_files;
    new_files.append( &original_resource );

    QList< QFuture< NewChapterResult > > futures = sync.futures();
    if ( original_position == -1 )
    {
        // Add new chapters to the end of the book
        for ( int i = 0; i < futures.count(); ++i )
        {
            html_resources.append( futures.at( i ).result().created_chapter );
            new_files.append( futures.at( i ).result().created_chapter );
        }
    }
    else
    {
        // Insert the new files at the correct positions in the list
        // The new files need to be inserted in order from first to last
        for ( int i = 0; i < new_chapters.count(); ++i )
        {
            int reading_order = next_reading_order + i;
            if( futures.at(i).result().reading_order == reading_order )
            {
                html_resources.insert( reading_order , futures.at( i ).result().created_chapter );
                new_files.append( futures.at( i ).result().created_chapter );
            }
            else
            {
                // This is security code to protect against any mangling of the futures list by Qt
                for( int j = 0 ; j < futures.count() ; ++j )
                {
                    if( futures.at(j).result().reading_order == reading_order )
                    {
                        html_resources.insert( reading_order , futures.at( j ).result().created_chapter );
                        new_files.append( futures.at( j ).result().created_chapter );
                        break;
                    }
                }
            }
        }
    }

    // Update anchor references between fragment ids in the new files. Since these all came from one single
    // file it's safe to assume that the fragment ids are all unique (since otherwise the references would be broken).
    AnchorUpdates::UpdateAllAnchorsWithIDs( new_files );

    // Now, update references to the original file that are made in other files.
    // We can't assume that ids are unique in this case, and so need to use a different mechanism.
    AnchorUpdates::UpdateExternalAnchors( other_files, Utility::URLEncodePath( original_resource.Filename() ), new_files );

    GetOPF().UpdateSpineOrder( html_resources ); 

    SetModified( true );
}


void Book::MergeWithPrevious( HTMLResource& html_resource )
{
    const QString defunct_filename = html_resource.Filename();
    QList< HTMLResource* > html_resources = m_Mainfolder.GetResourceTypeList< HTMLResource >( true );
    int previous_file_reading_order = html_resources.indexOf( &html_resource ) - 1;
    Q_ASSERT( previous_file_reading_order >= 0 );
    HTMLResource& previous_html = *html_resources[ previous_file_reading_order ];

    Merge( previous_html, html_resource );
}


// Merge selected html files into the first document
void Book::Merge( HTMLResource& html_resource1, HTMLResource& html_resource2 )
{
    const QString defunct_filename = html_resource2.Filename();

    QString html_resource2_fullpath = html_resource2.GetFullPath();
    {
        xc::DOMDocumentFragment *body_children_fragment = NULL;

        {
            QWriteLocker source_locker( &html_resource2.GetLock() );
            const xc::DOMDocument &source_dom  = html_resource2.GetDomDocumentForReading();
            xc::DOMNodeList &source_body_nodes = *source_dom.getElementsByTagName( QtoX( "body" ) );

            if ( source_body_nodes.getLength() != 1 )

                return;

            xc::DOMNode &source_body_node = *source_body_nodes.item( 0 );
            body_children_fragment        = XhtmlDoc::ConvertToDocumentFragment( *source_body_node.getChildNodes() ); 
        }  

        QWriteLocker sink_locker( &html_resource1.GetLock() );
        xc::DOMDocument &sink_dom        = html_resource1.GetDomDocumentForWriting();
        xc::DOMNodeList &sink_body_nodes = *sink_dom.getElementsByTagName( QtoX( "body" ) );

        if ( sink_body_nodes.getLength() != 1 )

            return;

        xc::DOMNode & sink_body_node = *sink_body_nodes.item( 0 );         
        sink_body_node.appendChild( sink_dom.importNode( body_children_fragment, true ) );
        html_resource1.MarkSecondaryCachesAsOld();

        html_resource2.Delete();
    }

    // Reconcile internal references in the merged file. It is the user's responsibility to ensure that 
    // all ids used across the two merged files are unique.
    QList< HTMLResource* > new_file;
    new_file.append( &html_resource1 );
    AnchorUpdates::UpdateAllAnchorsWithIDs( new_file );

    // Reconcile external references to the file that was merged.
    QList< HTMLResource* > html_resources = m_Mainfolder.GetResourceTypeList< HTMLResource >( true );
    html_resources.removeOne( &html_resource1 );
    AnchorUpdates::UpdateExternalAnchors( html_resources, Utility::URLEncodePath( defunct_filename ), new_file );

    // PerformUniversalUpdates accepts generic Resources
    QList< Resource* > resources = m_Mainfolder.GetResourceTypeAsGenericList< HTMLResource >();
    QHash< QString, QString > updates;
    updates[ html_resource2_fullpath ] = "../" + html_resource1.GetRelativePathToOEBPS();

    UniversalUpdates::PerformUniversalUpdates( true, resources, updates );
    SetModified( true );

    return;
}


void Book::SaveAllResourcesToDisk()
{
    QList< Resource* > resources = m_Mainfolder.GetResourceList(); 

    QtConcurrent::blockingMap( resources, SaveOneResourceToDisk );
}


bool Book::IsModified() const
{
    return m_IsModified;
}


bool Book::HasObfuscatedFonts() const
{
    QList< FontResource* > font_resources = m_Mainfolder.GetResourceTypeList< FontResource >();

    if ( font_resources.empty() )

        return false;

    foreach( FontResource *font_resource, font_resources )
    {
        if ( !font_resource->GetObfuscationAlgorithm().isEmpty() )

            return true;
    }

    return false;
}


void Book::SetModified( bool modified )
{
    bool old_modified_state = m_IsModified;
    m_IsModified = modified;

    if ( modified != old_modified_state )

        emit ModifiedStateChanged( m_IsModified );
}


void Book::SaveOneResourceToDisk( Resource *resource )
{
    resource->SaveToDisk( true );
}


Book::NewChapterResult Book::CreateOneNewChapter( NewChapter chapter_info )
{
    return CreateOneNewChapter( chapter_info, QHash< QString, QString >() );
}


Book::NewChapterResult Book::CreateOneNewChapter( NewChapter chapter_info,
                                         const QHash<QString, QString> &html_updates )
{
    QString filename = chapter_info.new_file_prefix % "_" % QString( "%1" ).arg( chapter_info.file_suffix + 1, 4, 10, QChar( '0' ) ) + ".xhtml";
    QString fullfilepath = chapter_info.temp_folder_path + "/" + filename;

    Utility::WriteUnicodeTextFile( "PLACEHOLDER", fullfilepath );

    HTMLResource *html_resource = qobject_cast< HTMLResource* >( 
        &m_Mainfolder.AddContentFileToFolder( fullfilepath ) );

    Q_ASSERT( html_resource );

    if ( html_updates.isEmpty() )
    {
        html_resource->SetDomDocument( 
            XhtmlDoc::LoadTextIntoDocument( CleanSource::Clean( chapter_info.source ) ) );
    }

    else
    {
        html_resource->SetDomDocument( 
            PerformHTMLUpdates( CleanSource::Clean( chapter_info.source ),
                                html_updates, 
                                QHash< QString, QString >() 
                              )() );
    }

    NewChapterResult chapter;
    chapter.created_chapter = html_resource;
    chapter.reading_order = chapter_info.reading_order;

    return chapter;
}
