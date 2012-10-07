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

#include <boost/shared_ptr.hpp>

#include <QtCore/QtCore>
#include <QtCore/QFileInfo>
#include <QtCore/QFutureSynchronizer>
#include <QtGui/QApplication>
#include <QtGui/QProgressDialog>

#include "BookManipulation/Book.h"
#include "BookManipulation/CleanSource.h"
#include "BookManipulation/FolderKeeper.h"
#include "BookManipulation/XercesCppUse.h"
#include "BookManipulation/XhtmlDoc.h"
#include "Misc/TempFolder.h"
#include "Misc/Utility.h"
#include "Misc/HTMLSpellCheck.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/NCXResource.h"
#include "ResourceObjects/OPFResource.h"
#include "sigil_constants.h"
#include "SourceUpdates/AnchorUpdates.h"
#include "SourceUpdates/PerformHTMLUpdates.h"
#include "SourceUpdates/UniversalUpdates.h"

using boost::shared_ptr;
using boost::make_tuple;
using boost::tuple;
using boost::tie;

static const QString FIRST_CSS_NAME   = "Style0001.css";
static const QString FIRST_SVG_NAME   = "Image0001.svg";
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


QList< Metadata::MetaElement > Book::GetMetadata() const
{
    return GetConstOPF().GetDCMetadata();    
}

QList< QVariant > Book::GetMetadataValues( QString text ) const
{
    return GetConstOPF().GetDCMetadataValues( text );    
}

void Book::SetMetadata( const QList< Metadata::MetaElement > metadata )
{
    GetOPF().SetDCMetadata( metadata );
    SetModified( true );
}


HTMLResource& Book::CreateNewHTMLFile()
{
    TempFolder tempfolder;

    QString fullfilepath = tempfolder.GetPath() + "/" + m_Mainfolder.GetUniqueFilenameVersion( FIRST_SECTION_NAME );

    Utility::WriteUnicodeTextFile( PLACEHOLDER_TEXT, fullfilepath );

    HTMLResource &html_resource = *qobject_cast< HTMLResource* >( 
                                        &m_Mainfolder.AddContentFileToFolder( fullfilepath ) );

    SetModified( true );
    return html_resource;
}


HTMLResource& Book::CreateEmptyHTMLFile()
{
    HTMLResource &html_resource = CreateNewHTMLFile();
    html_resource.SetText(EMPTY_HTML_FILE);
    SetModified( true );
    return html_resource;
}


HTMLResource& Book::CreateEmptyHTMLFile( HTMLResource& resource )
{
    HTMLResource &new_resource = CreateNewHTMLFile();
    new_resource.SetText(EMPTY_HTML_FILE);
    if ( &resource != NULL )
    {
        QList< HTMLResource* > html_resources = m_Mainfolder.GetResourceTypeList< HTMLResource >( true );

        int reading_order = GetOPF().GetReadingOrder( resource ) + 1;

        if ( reading_order > 0 )
        {
            html_resources.insert( reading_order, &new_resource );
            GetOPF().UpdateSpineOrder( html_resources );
        }
    }
    SetModified( true );
    return new_resource;
}


void Book::MoveResourceAfter( HTMLResource& from_resource, HTMLResource& to_resource )
{
    if ( &from_resource == NULL || &to_resource == NULL )

        return;

    QList< HTMLResource* > html_resources = m_Mainfolder.GetResourceTypeList< HTMLResource >( true );
    int to_after_reading_order = GetOPF().GetReadingOrder( to_resource ) + 1;
    int from_reading_order = GetOPF().GetReadingOrder( from_resource ) ;

    if ( to_after_reading_order > 0 )
    {
        html_resources.move( from_reading_order, to_after_reading_order );
        GetOPF().UpdateSpineOrder( html_resources );
    }
    SetModified( true );
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


SVGResource& Book::CreateEmptySVGFile()
{
    TempFolder tempfolder;

    QString fullfilepath = tempfolder.GetPath() + "/" + m_Mainfolder.GetUniqueFilenameVersion( FIRST_SVG_NAME );

    Utility::WriteUnicodeTextFile( "", fullfilepath );

    SVGResource &svg_resource = *qobject_cast< SVGResource* >(
                                        &m_Mainfolder.AddContentFileToFolder( fullfilepath ) );

    SetModified( true );
    return svg_resource;
}


HTMLResource& Book::CreateSectionBreakOriginalResource( const QString &content, HTMLResource& originating_resource )
{
    const QString originating_filename = originating_resource.Filename();

    int reading_order = GetOPF().GetReadingOrder( originating_resource );
    Q_ASSERT( reading_order >= 0 );

    QList< HTMLResource* > html_resources = m_Mainfolder.GetResourceTypeList< HTMLResource >( true );

    originating_resource.RenameTo( m_Mainfolder.GetUniqueFilenameVersion( FIRST_SECTION_NAME ) );

    HTMLResource &new_resource = CreateNewHTMLFile();
    new_resource.RenameTo( originating_filename );

    new_resource.SetText( CleanSource::Clean( content ) );

    m_Mainfolder.SuspendWatchingResources();
    new_resource.SaveToDisk();
    m_Mainfolder.ResumeWatchingResources();

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

    // Update TOC entries as well
    AnchorUpdates::UpdateTOCEntries(&GetNCX(), Utility::URLEncodePath(originating_filename), new_files);

    SetModified( true );
    return new_resource;
}


void Book::CreateNewSections( const QStringList &new_sections, HTMLResource &original_resource )
{
    int original_position = GetOPF().GetReadingOrder( original_resource );
    Q_ASSERT( original_position >= 0 );

    QString new_file_prefix = QFileInfo( original_resource.Filename() ).baseName();

    if ( new_sections.isEmpty() )

        return;

    TempFolder tempfolder;

    QFutureSynchronizer< NewSectionResult > sync;
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

    for ( int i = 0; i < new_sections.count(); ++i )
    {
        int reading_order = next_reading_order + i;

        NewSection sectionInfo;
        sectionInfo.source = new_sections.at( i );
        sectionInfo.reading_order = reading_order;
        sectionInfo.temp_folder_path = tempfolder.GetPath();
        sectionInfo.new_file_prefix = new_file_prefix;
        sectionInfo.file_suffix = i;

        sync.addFuture( 
            QtConcurrent::run( 
                this, 
                &Book::CreateOneNewSection, 
                sectionInfo ) );

        sectionInfo.file_suffix++;
    }	

    sync.waitForFinished();

    QList< HTMLResource* > new_files;
    new_files.append( &original_resource );

    QList< QFuture< NewSectionResult > > futures = sync.futures();
    if ( original_position == -1 )
    {
        // Add new sections to the end of the book
        for ( int i = 0; i < futures.count(); ++i )
        {
            html_resources.append( futures.at( i ).result().created_section );
            new_files.append( futures.at( i ).result().created_section );
        }
    }
    else
    {
        // Insert the new files at the correct positions in the list
        // The new files need to be inserted in order from first to last
        for ( int i = 0; i < new_sections.count(); ++i )
        {
            int reading_order = next_reading_order + i;
            if( futures.at(i).result().reading_order == reading_order )
            {
                html_resources.insert( reading_order , futures.at( i ).result().created_section );
                new_files.append( futures.at( i ).result().created_section );
            }
            else
            {
                // This is security code to protect against any mangling of the futures list by Qt
                for( int j = 0 ; j < futures.count() ; ++j )
                {
                    if( futures.at(j).result().reading_order == reading_order )
                    {
                        html_resources.insert( reading_order , futures.at( j ).result().created_section );
                        new_files.append( futures.at( j ).result().created_section );
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

    // Update TOC entries as well
    AnchorUpdates::UpdateTOCEntries(&GetNCX(), Utility::URLEncodePath(original_resource.Filename()), new_files);

    GetOPF().UpdateSpineOrder( html_resources ); 

    SetModified( true );
}


bool Book::IsDataWellFormed( HTMLResource& html_resource )
{
    XhtmlDoc::WellFormedError error = XhtmlDoc::WellFormedErrorForSource( html_resource.GetText() );
    return error.line == -1;
}


bool Book::IsDataOnDiskWellFormed( HTMLResource& html_resource )
{
    XhtmlDoc::WellFormedError error = XhtmlDoc::WellFormedErrorForSource( Utility::ReadUnicodeTextFile( html_resource.GetFullPath() ) );
    return error.line == -1;
}


Resource* Book::PreviousResource( Resource *resource )
{
    const QString defunct_filename = resource->Filename();
    QList< HTMLResource* > html_resources = m_Mainfolder.GetResourceTypeList< HTMLResource >( true );

    HTMLResource *html_resource = qobject_cast< HTMLResource* >(resource);

    int previous_file_reading_order = html_resources.indexOf( html_resource ) - 1;
    if ( previous_file_reading_order < 0 )
    {
        previous_file_reading_order = 0;
    }
    HTMLResource& previous_html = *html_resources[ previous_file_reading_order ];

    return qobject_cast< Resource *>( &previous_html );
}

QHash<QString, QStringList> Book::GetIdsInHTMLFiles()
{
    QHash<QString, QStringList> ids_in_html;

    const QList<HTMLResource*> html_resources = m_Mainfolder.GetResourceTypeList< HTMLResource >(false);

    QFuture< boost::tuple<QString, QStringList> > future = QtConcurrent::mapped(html_resources, GetIdsInHTMLFileMapped);

    for (int i = 0; i < future.results().count(); i++) {
        QString filename;
        QStringList ids;
        tie(filename, ids) = future.resultAt(i);

        // Each target entry has a list of filenames that contain it
        ids_in_html[filename] = ids;
    }

    return ids_in_html;
}

tuple<QString, QStringList> Book::GetIdsInHTMLFileMapped( HTMLResource* html_resource )
{
    return make_tuple(html_resource->Filename(), 
        XhtmlDoc::GetAllDescendantIDs(*XhtmlDoc::LoadTextIntoDocument(html_resource->GetText()).get()->getDocumentElement()));
}

QStringList Book::GetIdsInHTMLFile( HTMLResource* html_resource )
{
    return XhtmlDoc::GetAllDescendantIDs(*XhtmlDoc::LoadTextIntoDocument(html_resource->GetText()).get()->getDocumentElement());
}

QStringList Book::GetIdsInHrefs()
{
    QStringList ids;

    QHash<QString, QStringList> hrefs = GetHrefsInHTMLFiles();
    QHashIterator<QString, QStringList> it (hrefs);

    while (it.hasNext()) {
        it.next();
        foreach(QString href, it.value()) {
            if (href.contains('#')) {
                href = href.right(href.length() - href.indexOf("#") - 1);
                if (!href.isEmpty() && !ids.contains(href)) {
                    ids.append(href);
                }
            }
        }
    }

    // Return the list of ids used in hrefs in the book
    return ids;
}


QHash<QString, QStringList> Book::GetHrefsInHTMLFiles()
{
    QHash<QString, QStringList> hrefs_in_html;

    const QList<HTMLResource*> html_resources = m_Mainfolder.GetResourceTypeList< HTMLResource >(false);

    QFuture< boost::tuple<QString, QStringList> > future = QtConcurrent::mapped(html_resources, GetHrefsInHTMLFileMapped);

    for (int i = 0; i < future.results().count(); i++) {
        QString filename;
        QStringList hrefs;
        tie(filename, hrefs) = future.resultAt(i);

        // Each target entry has a list of filenames that contain it
        hrefs_in_html[filename] = hrefs;
    }

    return hrefs_in_html;
}

tuple<QString, QStringList> Book::GetHrefsInHTMLFileMapped( HTMLResource* html_resource )
{
    return make_tuple(html_resource->Filename(), 
        XhtmlDoc::GetAllDescendantHrefs(*XhtmlDoc::LoadTextIntoDocument(html_resource->GetText()).get()->getDocumentElement()));
}

QHash<QString, QStringList> Book::GetClassesInHTMLFiles()
{
    QHash<QString, QStringList> classes_in_html;

    const QList<HTMLResource*> html_resources = m_Mainfolder.GetResourceTypeList< HTMLResource >(false);

    QFuture< tuple<QString, QStringList> > future = QtConcurrent::mapped(html_resources, GetClassesInHTMLFileMapped);

    for (int i = 0; i < future.results().count(); i++) {
        QString filename;
        QStringList class_names;
        tie(filename, class_names) = future.resultAt(i);

        // Each class entry has a list of filenames that contain it
        foreach (QString class_name, class_names) {
            classes_in_html[class_name].append(filename);
        }
    }

    return classes_in_html;
}

tuple<QString, QStringList> Book::GetClassesInHTMLFileMapped(HTMLResource *html_resource)
{
    return make_tuple(html_resource->Filename(), 
            XhtmlDoc::GetAllDescendantClasses(*XhtmlDoc::LoadTextIntoDocument(html_resource->GetText()).get()->getDocumentElement()));
}

QStringList Book::GetClassesInHTMLFile(QString filename)
{
    QList<HTMLResource*> html_resources = m_Mainfolder.GetResourceTypeList< HTMLResource >(true);

    foreach (HTMLResource *html_resource, html_resources) {
        if (html_resource->Filename() == filename) {
            return XhtmlDoc::GetAllDescendantClasses(*XhtmlDoc::LoadTextIntoDocument(html_resource->GetText()).get()->getDocumentElement());
        }
    }

    return QStringList();
}

QHash<QString, QStringList> Book::GetImagesInHTMLFiles()
{
    QHash<QString, QStringList> images_in_html;

    const QList<HTMLResource*> html_resources = m_Mainfolder.GetResourceTypeList< HTMLResource >(false);

    QFuture< tuple<QString, QStringList> > future = QtConcurrent::mapped(html_resources, GetImagesInHTMLFileMapped);

    for (int i = 0; i < future.results().count(); i++) {
        QString filename;
        QStringList images;
        tie(filename, images) = future.resultAt(i);
        images_in_html[filename] = images;
    }

    return images_in_html;
}

QHash<QString, QStringList> Book::GetHTMLFilesUsingImages()
{
    QHash<QString, QStringList> image_html_files;

    const QList<HTMLResource*> html_resources = m_Mainfolder.GetResourceTypeList< HTMLResource >(false);

    QFuture< tuple<QString, QStringList> > future = QtConcurrent::mapped(html_resources, GetImagesInHTMLFileMapped);

    for (int i = 0; i < future.results().count(); i++) {
        QString html_filename;
        QStringList image_filenames;
        tie(html_filename, image_filenames) = future.resultAt(i);

        foreach (QString image_filename, image_filenames) {
            image_html_files[image_filename].append(html_filename);
        }
    }

    return image_html_files;
}


tuple<QString, QStringList> Book::GetImagesInHTMLFileMapped(HTMLResource *html_resource)
{
    return make_tuple(html_resource->Filename(), 
            XhtmlDoc::GetAllImagePathsFromImageChildren(*XhtmlDoc::LoadTextIntoDocument(html_resource->GetText()).get()));
}

QSet<QString> Book::GetWordsInHTMLFiles()
{
    QStringList all_words;

    const QList<HTMLResource*> html_resources = m_Mainfolder.GetResourceTypeList< HTMLResource >(false);

    QFuture<QStringList> future = QtConcurrent::mapped(html_resources, GetWordsInHTMLFileMapped);

    for (int i = 0; i < future.results().count(); i++) {
        QStringList result = future.resultAt(i);
        all_words.append(result);
    }

    return all_words.toSet();
}

QStringList Book::GetWordsInHTMLFileMapped(HTMLResource *html_resource)
{
    return HTMLSpellCheck::GetAllWords(html_resource->GetText());
}

QHash<QString, QStringList> Book::GetStylesheetsInHTMLFiles()
{
    QHash<QString, QStringList> links_in_html;

    const QList<HTMLResource*> html_resources = m_Mainfolder.GetResourceTypeList< HTMLResource >(false);

    QFuture< boost::tuple<QString, QStringList> > future = QtConcurrent::mapped(html_resources, GetStylesheetsInHTMLFileMapped);

    for (int i = 0; i < future.results().count(); i++) {
        QString filename;
        QStringList links;
        tie(filename, links) = future.resultAt(i);
        links_in_html[filename] = links;
    }

    return links_in_html;
}

tuple<QString, QStringList> Book::GetStylesheetsInHTMLFileMapped(HTMLResource *html_resource)
{
    return make_tuple(html_resource->Filename(), 
            XhtmlDoc::GetLinkedStylesheets(html_resource->GetText()));
}

QStringList Book::GetStylesheetsInHTMLFile(HTMLResource *html_resource)
{
    return XhtmlDoc::GetLinkedStylesheets(html_resource->GetText());
}

Resource* Book::MergeResources( QList<Resource *> resources )
{
    QProgressDialog progress(QObject::tr( "Merging Files.." ), 0, 0, resources.count(), QApplication::activeWindow());
    progress.setMinimumDuration(PROGRESS_BAR_MINIMUM_DURATION);
    int progress_value = 0;

    Resource *sink_resource = resources.takeFirst();
    HTMLResource &sink_html_resource = *qobject_cast<HTMLResource *>(sink_resource);
    
    if ( !IsDataWellFormed( sink_html_resource ) ) {
        return sink_resource;
    }

    QList<QString> merged_filenames;
    {
        // Load our DOMDocument just once up front for our sink resource we are merging into
        QWriteLocker sink_locker( &sink_html_resource.GetLock() );
        shared_ptr<xc::DOMDocument> sink_d = XhtmlDoc::LoadTextIntoDocument(sink_html_resource.GetText());
        xc::DOMDocument &sink_dom        = *sink_d.get();
        xc::DOMNodeList &sink_body_nodes = *sink_dom.getElementsByTagName( QtoX( "body" ) );
        xc::DOMNode &sink_body_node      = *sink_body_nodes.item( 0 );

        if ( sink_body_nodes.getLength() != 1 ) {
            return sink_resource;
        }

        Resource *failed_resource = NULL;
        // Now iterate across all the other resources merging them into this resource
        foreach (Resource *source_resource, resources) {
            // Set progress value and ensure dialog has time to display when doing extensive updates
            progress.setValue(progress_value++);
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

            xc::DOMDocumentFragment *body_children_fragment = NULL;
            HTMLResource &source_html_resource = *qobject_cast<HTMLResource *>(source_resource);
            if ( !IsDataWellFormed( source_html_resource ) ) {
                failed_resource = source_resource;
                break;
            }

            // Get the html document for this source resource.
            QWriteLocker source_locker( &source_html_resource.GetLock() );
            shared_ptr<xc::DOMDocument> sd = XhtmlDoc::LoadTextIntoDocument(source_html_resource.GetText());
            const xc::DOMDocument &source_dom  = *sd.get();
            xc::DOMNodeList &source_body_nodes = *source_dom.getElementsByTagName( QtoX( "body" ) );
            if ( source_body_nodes.getLength() != 1 ) {
                failed_resource = source_resource;
                break;
            }

            // Append the html fragment to the body for our sink. 
            xc::DOMNode &source_body_node = *source_body_nodes.item( 0 );
            body_children_fragment        = XhtmlDoc::ConvertToDocumentFragment( *source_body_node.getChildNodes() );
            sink_body_node.appendChild( sink_dom.importNode( body_children_fragment, true ) );

            merged_filenames.append(Utility::URLEncodePath(source_resource->Filename()));
        }

        if (failed_resource != NULL) {
            // Abort the merge process. We will discard whatever we had merged so far
            return failed_resource;
        }

        // Now all fragments have been merged into this sink document, serialize and store it.
        sink_html_resource.SetText(XhtmlDoc::GetDomDocumentAsString(sink_dom));
        // Now safe to do the delete
        foreach (Resource *source_resource, resources) {
            source_resource->Delete();
        }
    }
    progress.setValue(progress_value++);
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

    // It is the user's responsibility to ensure that all ids used across the two merged files are unique.
    // Reconcile all references to the files that were merged.
    QList< HTMLResource* > html_resources = m_Mainfolder.GetResourceTypeList< HTMLResource >( true );
    AnchorUpdates::UpdateAllAnchors( html_resources, merged_filenames, &sink_html_resource );

    SetModified( true );
    return NULL;
}

void Book::SaveAllResourcesToDisk()
{
    QList< Resource* > resources = m_Mainfolder.GetResourceList(); 
    
    m_Mainfolder.SuspendWatchingResources();
    QtConcurrent::blockingMap( resources, SaveOneResourceToDisk );
    m_Mainfolder.ResumeWatchingResources();
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

void Book::ResourceUpdatedFromDisk(Resource &resource)
{
    SetModified(true);
    emit ResourceUpdatedFromDiskRequest(resource);
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


Book::NewSectionResult Book::CreateOneNewSection( NewSection section_info )
{
    return CreateOneNewSection( section_info, QHash< QString, QString >() );
}


Book::NewSectionResult Book::CreateOneNewSection( NewSection section_info,
                                         const QHash<QString, QString> &html_updates )
{
    QString filename = section_info.new_file_prefix % "_" % QString( "%1" ).arg( section_info.file_suffix + 1, 4, 10, QChar( '0' ) ) + ".xhtml";
    QString fullfilepath = section_info.temp_folder_path + "/" + filename;

    Utility::WriteUnicodeTextFile( "PLACEHOLDER", fullfilepath );

    HTMLResource *html_resource = qobject_cast< HTMLResource* >(
        &m_Mainfolder.AddContentFileToFolder( fullfilepath ) );

    Q_ASSERT( html_resource );

    if ( html_updates.isEmpty() )
    {
        html_resource->SetText( CleanSource::Clean( section_info.source ) );
    }

    else
    {
        html_resource->SetText(
            XhtmlDoc::GetDomDocumentAsString( *PerformHTMLUpdates( CleanSource::Clean( section_info.source ),
                                html_updates,
                                QHash< QString, QString >()
                              )().get() ) );
    }

    NewSectionResult section;
    section.created_section = html_resource;
    section.reading_order = section_info.reading_order;

    return section;
}
