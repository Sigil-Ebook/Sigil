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

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtXml/QXmlStreamReader>

#include "BookManipulation/Metadata.h"
#include "BookManipulation/FolderKeeper.h"
#include "Importers/ImportEPUB.h"
#include "Misc/FontObfuscation.h"
#include "Misc/Utility.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/CSSResource.h"
#include "SourceUpdates/UniversalUpdates.h"
#include "sigil_constants.h"
#include "sigil_exception.h"

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
    if ( !Utility::IsFileReadable( m_FullFilePath ) ) {
        boost_throw(EPUBLoadParseError() << errinfo_epub_load_parse_errors( QString(QObject::tr("Cannot read EPUB: %1")).arg(QDir::toNativeSeparators(m_FullFilePath)).toStdString() ) );
    }

    // These read the EPUB file
    ExtractContainer();

    QHash< QString, QString > encrypted_files = ParseEncryptionXml();
    if ( BookContentEncrypted( encrypted_files ) )

        boost_throw( FileEncryptedWithDrm() );

    // These mutate the m_Book object
    LocateOPF();

    // We're going to check if there is an NCX or if we need to create one
    // as a fall back for poorly (invalid) constructed EPUBs.
    // This causes the OPF to be parsed twice but the OPF is small and the
    // parsers are very fast so refactoring is not a priority.
    QString ncx_id = GetNCXId();

    // These mutate the m_Book object
    ReadOPF();
    AddObfuscatedButUndeclaredFonts( encrypted_files );
    AddNonStandardAppleXML();

    LoadInfrastructureFiles();

    const QHash< QString, QString > &updates = LoadFolderStructure();
    const QList< Resource* > &resources      = m_Book->GetFolderKeeper().GetResourceList();

    const QStringList &load_errors = UniversalUpdates::PerformUniversalUpdates( false, resources, updates );
    if ( !load_errors.isEmpty() ) {
        // Hmmm... we could "possibly" recover from some of these (allowing the user to delete an
        // invalid html file from the EPUB for instance). But then again it could be that the
        // EPUB is so badly kakked that it is totally invalid. Throw an error, the user can
        // analyse the warning messages and fix it externally before attempting another load.
        const QString all_errors = load_errors.join("\n");
        boost_throw( EPUBLoadParseError() << errinfo_epub_load_parse_errors( all_errors.toStdString() ) );
    }

    ProcessFontFiles( resources, updates, encrypted_files );
    m_Book->GetOPF().UpdateNCXLocationInManifest( m_Book->GetNCX() );

    // If spine didn't specify the ncx, recreate the OPF from scratch
    // preserving any important metadata elements and the reading order.
    m_Book->SetModified( false );
    if( ncx_id.isEmpty() )
    {
        QList< Metadata::MetaElement > originalMetadata = m_Book->GetOPF().GetDCMetadata();
        QStringList spineOrder = m_Book->GetOPF().GetSpineOrderFilenames();

        m_Book->GetOPF().AutoFixWellFormedErrors();

        m_Book->GetOPF().SetDCMetadata( originalMetadata );
        m_Book->GetOPF().SetSpineOrderFromFilenames( spineOrder );

        m_Book->SetModified( true );
    }

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
        encryption.readNext(); 

        if ( encryption.isStartElement() )  
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
        const QString error = QString(QObject::tr("Error parsing encryption xml.\nLine: %1 Column %2 - %3"))
                                .arg(encryption.lineNumber())
                                .arg(encryption.columnNumber())
                                .arg(encryption.errorString());
        boost_throw( EPUBLoadParseError() << errinfo_epub_load_parse_errors( error.toStdString() ) );
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


// This is basically a workaround for old versions of InDesign not listing the fonts it
// embedded in the OPF manifest, even though the specs say it has to.
// It does list them in the encryption.xml, so we use that.
void ImportEPUB::AddObfuscatedButUndeclaredFonts( const QHash< QString, QString > &encrypted_files )
{
    if ( encrypted_files.empty() )

        return;

    QDir opf_dir = QFileInfo( m_OPFFilePath ).dir();

    foreach( QString filepath, encrypted_files.keys() )
    {
        if ( !FONT_EXTENSIONS.contains( QFileInfo( filepath ).suffix().toLower() ) )

            continue;
       
        // Only add the path to the manifest if it is not already included.
        QMapIterator< QString, QString > valueSearch( m_Files );
        if ( !valueSearch.findNext( opf_dir.relativeFilePath( filepath ) ) )

            m_Files[ Utility::CreateUUID() ] = opf_dir.relativeFilePath( filepath );
    }
}


// Another workaround for non-standard Apple files
// At present it only handles com.apple.ibooks.display-options.xml, but any
// further iBooks aberrations should be handled here as well.
void ImportEPUB::AddNonStandardAppleXML()
{
    QDir opf_dir = QFileInfo( m_OPFFilePath ).dir();

    QStringList aberrant_Apple_filenames;
    aberrant_Apple_filenames.append( m_ExtractedFolderPath + "/META-INF/com.apple.ibooks.display-options.xml" );

    for( int i = 0 ; i < aberrant_Apple_filenames.size() ; ++i )
    {
        if( QFile::exists( aberrant_Apple_filenames.at( i ) ) )
        {
            m_Files[ Utility::CreateUUID() ]  = opf_dir.relativeFilePath( aberrant_Apple_filenames.at( i ) );
        }
    }
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

    QList< FontResource* > font_resources = m_Book->GetFolderKeeper().GetResourceTypeList< FontResource >();

    if ( font_resources.empty() )

        return;

    QHash< QString, QString > new_font_paths_to_algorithms;

    foreach( QString old_update_path, updates.keys() )
    {
        if ( !FONT_EXTENSIONS.contains( QFileInfo( old_update_path ).suffix().toLower() ) )

            continue;        

        QString new_update_path = updates[ old_update_path ];

        foreach( QString old_encrypted_path, encrypted_files.keys() )
        {
            if ( old_update_path == old_encrypted_path )

                new_font_paths_to_algorithms[ new_update_path ] = encrypted_files[ old_encrypted_path ];
        }        
    }

    foreach( FontResource *font_resource, font_resources )
    {
        QString match_path = "../" + font_resource->GetRelativePathToOEBPS();
        QString algorithm  = new_font_paths_to_algorithms.value( match_path );

        if ( algorithm.isEmpty() )

            continue;

        font_resource->SetObfuscationAlgorithm( algorithm );

        // Actually we are de-obfuscating, but the inverse operations of the obfuscation methods 
        // are the obfuscation methods themselves. For the math oriented, the obfuscation methods
        // are involutary [ f( f( x ) ) = x ].
        if ( algorithm == ADOBE_FONT_ALGO_ID )

            FontObfuscation::ObfuscateFile( font_resource->GetFullPath(), algorithm, m_UuidIdentifierValue );

        else 

            FontObfuscation::ObfuscateFile( font_resource->GetFullPath(), algorithm, m_UniqueIdentifierValue );
    }
}







