/************************************************************************
**
**  Copyright (C) 2009, 2010  Strahinja Markovic
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
#include "ImportSGF.h"
#include "../Misc/Utility.h"
#include "../BookManipulation/CleanSource.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/TextResource.h"


// Constructor;
// The parameter is the file to be imported
ImportSGF::ImportSGF( const QString &fullfilepath )
    : ImportOEBPS( fullfilepath )
{

}


// Reads and parses the file 
// and returns the created Book;
// Overrides;
QSharedPointer< Book > ImportSGF::GetBook()
{
    if ( !Utility::IsFileReadable( m_FullFilePath ) )

        boost_throw( CannotReadFile() << errinfo_file_fullpath( m_FullFilePath.toStdString() ) );

    // These read the EPUB file
    ExtractContainer();
    LocateOPF();
    ReadOPF();

    // These mutate the m_Book object
    LoadMetadata();

    QString source = LoadSource();
    QString header = CreateHeader( CreateStyleResources( source ) );

    // We remove the first and only XHTML resource
    // since we don't want to load that directly.
    // We will chop it up and create new XHTMLs in CreateXHTMLFiles.
    m_Files.remove( m_ReadingOrderIds.at( 0 ) );

    CreateXHTMLFiles( source, header, LoadFolderStructure() );

    return m_Book;
}


// Loads the source code into the Book
QString ImportSGF::LoadSource()
{
    QString fullpath = QFileInfo( m_OPFFilePath ).absolutePath() + "/" + m_Files.values().first();
    return CleanSource::ToValidXHTML( Utility::ReadUnicodeTextFile( fullpath ) ); 
}


// Creates style files from the style tags in the source
// and returns a list of their file paths relative 
// to the OEBPS folder in the FolderKeeper
QList< Resource* > ImportSGF::CreateStyleResources( const QString &source )
{    
    QList< XHTMLDoc::XMLElement > style_tag_nodes = XHTMLDoc::GetTagsInHead( source, "style" );

    QString folderpath = Utility::GetNewTempFolderPath();
    QDir dir( folderpath );
    dir.mkpath( folderpath );

    QFutureSynchronizer< Resource* > sync;

    for ( int i = 0; i < style_tag_nodes.count(); ++i ) 
    {
        sync.addFuture( QtConcurrent::run( 
            this, &ImportSGF::CreateOneStyleFile, style_tag_nodes.at( i ), folderpath, i ) );        
    }

    sync.waitForFinished();

    QtConcurrent::run( Utility::DeleteFolderAndFiles, folderpath );

    QList< QFuture< Resource* > > futures = sync.futures();
    QList< Resource* > style_resources;

    for ( int i = 0; i < futures.count(); ++i )
    {
        style_resources.append( futures.at( i ).result() );
    }    

    return style_resources;
}


Resource* ImportSGF::CreateOneStyleFile( const XHTMLDoc::XMLElement &element, 
                                         const QString &folderpath, 
                                         int index )
{
    QString style_text = element.text;
    style_text = RemoveSigilStyles( style_text );
    style_text = StripCDATA( style_text );
    style_text = style_text.trimmed();  

    if ( style_text.isEmpty() )

        return NULL;
    
    QString extension    = element.attributes.value( "type" ) == "text/css" ? "css" : "xpgt";
    QString filename     = QString( "style" ) + QString( "%1" ).arg( index + 1, 3, 10, QChar( '0' ) ) + "." + extension;
    QString fullfilepath = folderpath + "/" + filename;

    Utility::WriteUnicodeTextFile( style_text, fullfilepath );

    TextResource *text_resource = qobject_cast< TextResource* >( 
                                     &m_Book->GetFolderKeeper().AddContentFileToFolder( fullfilepath ) );

    Q_ASSERT( text_resource );
    text_resource->SetText( style_text );

    return text_resource;
}


// Strips CDATA declarations from the provided source
QString ImportSGF::StripCDATA( const QString &style_source )
{
    QString newsource = style_source;

    newsource.replace( "/*<![CDATA[*/", "" );
    newsource.replace( "/*]]>*/", "" );
    newsource.replace( "/**/", "" );

    newsource.replace( "<![CDATA[", "" );
    newsource.replace( "]]>", "" );

    return newsource;
}


// Removes Sigil styles from the provided source
QString ImportSGF::RemoveSigilStyles( const QString &style_source )
{
    QString newsource = style_source;

    QRegExp chapter_break_style( "hr\\.sigilChapterBreak[^\\}]+\\}" );
    QRegExp sigil_comment( "/\\*SG.*SG\\*/" );

    sigil_comment.setMinimal( true );

    newsource.remove( chapter_break_style );
    newsource.remove( sigil_comment );

    return newsource;
}


// Takes a list of style sheet file names 
// and returns the header for XHTML files
QString ImportSGF::CreateHeader( const QList< Resource* > &style_resources )
{
    QString header = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                     "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"\n"
                     "    \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n\n"							
                     "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
                     "<head>\n";

    foreach( Resource* resource, style_resources )
    {
        if ( resource )
        {
            header += "    <link href=\"../" + resource->GetRelativePathToOEBPS() + 
                      "\" rel=\"stylesheet\" type=\"text/css\" />\n";
        }
    }

    header += "</head>\n";

    return header;
}


// Creates XHTML files from the book source;
// the provided header is used as the header of the created files
void ImportSGF::CreateXHTMLFiles( const QString &source, 
                                  const QString &header,
                                  const QHash< QString, QString > &html_updates )
{    
    const QStringList chapters = XHTMLDoc::GetSGFChapterSplits( source, header );

    m_Book->CreateNewChapters( chapters, html_updates );   
}
