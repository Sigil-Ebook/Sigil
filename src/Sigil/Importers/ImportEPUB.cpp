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
#include "ImportEPUB.h"
#include "Misc/Utility.h"
#include "SourceUpdates/UniversalUpdates.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/CSSResource.h"
#include "Misc/FontObfuscation.h"

const QString ADOBE_FONT_ALGO_ID = "http://ns.adobe.com/pdf/enc#RC";
const QString IDPF_FONT_ALGO_ID  = "http://www.idpf.org/2008/embedding";


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

        boost_throw( CannotReadFile() << errinfo_file_fullpath( m_FullFilePath.toStdString() ) );

    // These read the EPUB file
    ExtractContainer();

    QHash< QString, QString > encrypted_files = ParseEncryptionXml();
    if ( BookContentEncrypted( encrypted_files ) )

        boost_throw( FileEncryptedWithDrm() );

    LocateOPF();
    ReadOPF();

    // These mutate the m_Book object
    LoadMetadata();

    const QHash< QString, QString > &updates = LoadFolderStructure();
    const QList< Resource* > &resources      = m_Book->GetFolderKeeper().GetResourceList();

    UniversalUpdates::PerformUniversalUpdates( false, resources, updates );
    ProcessFontFiles( resources, updates, encrypted_files );

    // Sometimes we load crappy epubs created by Calibre,
    // and this fixes problems with phantom files in the spine.
    m_Book->NormalizeReadingOrders();

    return m_Book;
}


QHash< QString, QString > ImportEPUB::ParseEncryptionXml()
{
    QString encrpytion_xml_path = m_ExtractedFolderPath + "/META-INF/encryption.xml";

    if ( !QFileInfo( encrpytion_xml_path ).exists() )

        return QHash< QString, QString >();

    QXmlStreamReader encryption( Utility::ReadUnicodeTextFile( encrpytion_xml_path ) );

    QHash< QString, QString > encrypted_files;

    QString encryption_algo;
    QString uri;

    while ( !encryption.atEnd() ) 
    {
        // Get the next token from the stream
        if ( encryption.readNext() == QXmlStreamReader::StartElement )  
        {
            if ( encryption.name() == "EncryptionMethod" )
            {
                encryption_algo = encryption.attributes().value( "", "Algorithm" ).toString();                
            }

            else if ( encryption.name() == "CipherReference" )
            {
                uri = m_ExtractedFolderPath + "/" +
                    Utility::URLDecodePath( encryption.attributes().value( "", "URI" ).toString() );
                
                encrypted_files[ uri ] = encryption_algo;
            }
        }
    }

    if ( encryption.hasError() )
    {
        boost_throw( ErrorParsingEncryptionXml() 
                     << errinfo_XML_parsing_error_string( encryption.errorString().toStdString() )
                     << errinfo_XML_parsing_line_number( encryption.lineNumber() )
                     << errinfo_XML_parsing_column_number( encryption.columnNumber() )
                   );
    }

    return encrypted_files;
}


bool ImportEPUB::BookContentEncrypted( const QHash< QString, QString > &encrypted_files )
{
    foreach( QString algorithm, encrypted_files.values() )
    {
        if ( algorithm != ADOBE_FONT_ALGO_ID &&
             algorithm != IDPF_FONT_ALGO_ID )
        {
            return true;
        }
    }

    return false;
}


QString ImportEPUB::MainBookId()
{
    foreach( Metadata::MetaElement meta, m_MetaElements )
    {
        if ( meta.attributes[ "id" ] == m_UniqueIdentifierId )

            return meta.value.toString();
    }

    return QString();
}


QString ImportEPUB::FirstUrnUuid()
{
    foreach( Metadata::MetaElement meta, m_MetaElements )
    {
        QString value = meta.value.toString();

        if ( value.contains( "urn:uuid:" ) )

            return value;
    }

    return QString();
}


// Each resource can provide us with its new path. encrypted_files provides
// a mapping from old resource paths to the obfuscation algorithms. 
// So we use the updates hash which provides a mapping from old paths to new
// paths to match the resources to their algorithms.
void ImportEPUB::ProcessFontFiles( const QList< Resource* > &resources, 
                                   const QHash< QString, QString > &updates, 
                                   const QHash< QString, QString > &encrypted_files )
{
    if ( encrypted_files.empty() )

        return;

    QList< FontResource* > font_resources;

    for ( int i = 0; i < resources.count(); ++i )
    {
        Resource *resource = resources.at( i );

        if ( resource->Type() == Resource::FontResource )

            font_resources.append( qobject_cast< FontResource* >( resource ) );
    }

    if ( font_resources.empty() )

        return;

    QHash< QString, QString > new_font_paths_to_algorithms;

    foreach( QString old_update_path, updates.keys() )
    {
        if ( !old_update_path.endsWith( ".ttf" ) && !old_update_path.endsWith( ".otf" ) )

            continue;

        QString new_update_path = updates[ old_update_path ];

        foreach( QString old_encrypted_path, encrypted_files.keys() )
        {
            if ( old_update_path == old_encrypted_path )

                new_font_paths_to_algorithms[ new_update_path ] = encrypted_files[ old_encrypted_path ];
        }        
    }

    QString main_id  = MainBookId();
    QString urn_uuid = FirstUrnUuid();

    foreach( FontResource *font_resource, font_resources )
    {
        QString match_path = "../" + font_resource->GetRelativePathToOEBPS();
        QString algorithm  = new_font_paths_to_algorithms.value( match_path );

        font_resource->SetObfuscationAlgorithm( algorithm );

        if ( algorithm == ADOBE_FONT_ALGO_ID )

            FontObfuscation::ObfuscateFile( font_resource->GetFullPath(), algorithm, urn_uuid );

        else 

            FontObfuscation::ObfuscateFile( font_resource->GetFullPath(), algorithm, main_id );
    }
}




