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
#include "BookManipulation/Metadata.h"
#include "BookManipulation/XercesCppUse.h"
#include "BookManipulation/XhtmlDoc.h"
#include "Misc/Utility.h"

static const QString PATH_TO_LANGUAGES  = ":/data/languages.csv";
static const QString PATH_TO_BASICMETA  = ":/data/basicmeta.csv";
static const QString PATH_TO_RELATORS   = ":/data/relator.csv";

static const QStringList EVENT_LIST           = QStringList() << "creation" << "publication" << "modification";
static const QStringList MODIFICATION_ALIASES = QStringList() << "modified" << "modification";
static const QStringList CREATION_ALIASES     = QStringList() << "created"  << "creation";
static const QStringList PUBLICATION_ALIASES  = QStringList() << "issued"   << "published" << "publication";
static const QStringList SCHEME_LIST          = QStringList() << "ISBN" << "ISSN" << "DOI" << "CustomID";

QMutex Metadata::s_AccessMutex;
Metadata* Metadata::m_Instance = NULL;

Metadata& Metadata::Instance()
{
    // We use a static local variable
    // to hold our singleton instance; using a pointer member
    // variable creates problems with object destruction;

    QMutexLocker locker( &s_AccessMutex );

    if ( !m_Instance )
    {
        static Metadata meta;
        m_Instance = &meta;
    }

    return *m_Instance;
}

const QMap< QString, QString >& Metadata::GetLanguageMap()
{
    return m_Languages;
}


const QMap< QString, Metadata::MetaInfo >& Metadata::GetRelatorMap()
{
    return m_Relators;
}


const QMap< QString, Metadata::MetaInfo >& Metadata::GetBasicMetaMap()
{
    return m_Basic;
}


const QHash< QString, QString >& Metadata::GetFullRelatorNameHash()
{
    return m_FullRelators;
}


const QHash< QString, QString >& Metadata::GetFullLanguageNameHash()
{
    return m_FullLanguages;
}


Metadata::MetaElement Metadata::MapToBookMetadata( const xc::DOMElement &element )
{
    Metadata::MetaElement meta;
    QString element_name = XhtmlDoc::GetNodeName( element );

    if ( element_name == "meta" )
    {        
        meta.name  = XtoQ( element.getAttribute( QtoX( "name" ) ) );
        meta.value = XtoQ( element.getAttribute( QtoX( "content" ) ) );
        meta.attributes[ "scheme" ] = XtoQ( element.getAttribute( QtoX( "scheme" ) ) );

        if ( ( !meta.name.isEmpty() ) && ( !meta.value.toString().isEmpty() ) ) 
        
            return MapToBookMetadata( meta , false );        
    }

    else
    {
        meta.attributes = XhtmlDoc::GetNodeAttributes( element );
        meta.name = element_name;

        QString element_text = XtoQ( element.getTextContent() );
        meta.value = element_text;

        if ( !element_text.isEmpty() )

            return MapToBookMetadata( meta , true ); 
    }

    return meta;
}


// Maps Dublic Core metadata to internal book meta format
Metadata::MetaElement Metadata::MapToBookMetadata( const Metadata::MetaElement &meta, bool is_dc_element )
{
    QString name = meta.name.toLower();

    if ( !is_dc_element && 
         !name.startsWith( "dc." ) && 
         !name.startsWith( "dcterms." ) )  
    {
        return FreeFormMetadata( meta );
    }

    // Dublin Core

    // Transform HTML based Dublin Core to OPF style meta element
    MetaElement working_copy_meta = is_dc_element ? meta : HtmlToOpfDC( meta );

    name = working_copy_meta.name.toLower();

    if ( ( name == "creator" ) || ( name == "contributor" ) )

        return CreateContribMetadata( working_copy_meta );

    if ( name == "date" )

        return DateMetadata( working_copy_meta );

    if ( name == "identifier" )

        return IdentifierMetadata( working_copy_meta );

    QString value = meta.value.toString();

    if ( name == "language" )
    {
        // We convert ISO 639-1 language code into full language name (e.g. en -> English)
        value = GetFullLanguageNameHash()[ value ];
        // fall through
    }

    MetaElement book_meta;

    if ( ( !name.isEmpty() ) && ( !value.isEmpty() ) )
    {
        book_meta.name  = name[ 0 ].toUpper() + name.mid( 1 );
        book_meta.value = value;
    }

    return book_meta;
}


Metadata::Metadata()
{
    LoadLanguages();
    LoadBasicMetadata();
    LoadRelatorCodes();
}


// Loads the languages and their codes from disk
void Metadata::LoadLanguages()
{
    // If the languages have already been loaded
    // by a previous Meta Editor, then don't load them again
    if ( !m_Languages.isEmpty() )

        return;

    QStringList file_lines = Utility::ReadUnicodeTextFile( PATH_TO_LANGUAGES ).split( '\n' );

    foreach( QString line, file_lines )
    {
        if ( line.isEmpty() )

            continue;

        QStringList fields = line.split( "|" );

        m_Languages[     fields[ 0 ] ] = fields[ 1 ];
        m_FullLanguages[ fields[ 1 ] ] = fields[ 0 ];
    }	
}


// Loads the basic metadata and their descriptions from disk
void Metadata::LoadBasicMetadata()
{
    // If the basic metadata has already been loaded
    // by a previous Meta Editor, then don't load them again
    if ( !m_Basic.isEmpty() )

        return;

    QStringList file_lines = Utility::ReadUnicodeTextFile( PATH_TO_BASICMETA ).split( '\n' );

    foreach( QString line, file_lines )
    { 
        if ( line.isEmpty() )

            continue;

        QStringList fields = line.split( "|" );

        MetaInfo meta;

        meta.relator_code = "";
        meta.description  = fields[ 1 ];

        m_Basic[ fields[ 0 ] ] = meta;
    }
}


// Loads the relator codes, their full names,
// and their descriptions from disk
void Metadata::LoadRelatorCodes()
{
    // If the relator codes have already been loaded
    // by a previous Meta Editor, then don't load them again
    if ( !m_Relators.isEmpty() )

        return;

    QStringList file_lines = Utility::ReadUnicodeTextFile( PATH_TO_RELATORS ).split( '\n' );

    foreach( QString line, file_lines )
    {
        if ( line.isEmpty() )

            continue;

        QStringList fields = line.split( "|" );

        MetaInfo meta;

        meta.relator_code   = fields[ 1 ];
        meta.description    = fields[ 2 ];

        m_Relators[     fields[ 0 ] ] = meta;
        m_FullRelators[ fields[ 1 ] ] = fields[ 0 ];
    }
}


// Converts HTML sourced Dublin Core metadata to OPF style metadata
Metadata::MetaElement Metadata::HtmlToOpfDC( const Metadata::MetaElement &meta )
{
    // Dublin Core from html file with the original 15 element namespace or
    // expanded DCTerms namespace. Allows qualifiers as refinements
    // prefix.name[.refinement]

    QStringList fields = QString( meta.name.toLower() + ".." ).split( "." );
    QString name       = fields[ 1 ];
    QString refinement = fields[ 2 ];
    
    QString dc_event;

    if ( MODIFICATION_ALIASES.contains( name ) || MODIFICATION_ALIASES.contains( refinement ) )
    {
        name     = "date";
        dc_event = "modification";
    }

    else if ( CREATION_ALIASES.contains( name ) || CREATION_ALIASES.contains( refinement ) ) 
    {
        name     = "date";
        dc_event = "creation";
    }

    else if ( PUBLICATION_ALIASES.contains( name ) || PUBLICATION_ALIASES.contains( refinement ) )
    {
        name     = "date";
        dc_event = "publication";
    }

    QString role   = ( name == "creator" ) || ( name == "contributor" ) ? refinement : QString();
    QString scheme = meta.attributes.value( "scheme" );

    if ( ( name == "identifier" ) && ( scheme.isEmpty() ) )

        scheme = refinement;

    if ( !scheme.isEmpty() )
    {
        if ( SCHEME_LIST.contains( scheme, Qt::CaseInsensitive ) )

            scheme = SCHEME_LIST.filter( scheme, Qt::CaseInsensitive )[ 0 ];
    }

    MetaElement opf_meta;
    opf_meta.name  = name;
    opf_meta.value = meta.value;

    if ( !scheme.isEmpty() )

        opf_meta.attributes[ "scheme" ] = scheme;

    if ( !dc_event.isEmpty() )

        opf_meta.attributes[ "event" ] = dc_event;

    if ( !role.isEmpty() )

        opf_meta.attributes[ "role" ] = role;

    return opf_meta;
}    


// Converts free form metadata into internal book metadata
Metadata::MetaElement Metadata::FreeFormMetadata( const Metadata::MetaElement &meta )
{
    // non - dublin core meta info from html file, if this maps to 
    // one of the metadata basic fields used internally pass it through
    // i.e. Author, Title, Publisher, Rights/CopyRight, EISBN/ISBN

    QString name = meta.name.toLower();

    // Remap commonly used meta values to match internal names
    name =  name == "copyright" ? "Rights"   :
            name == "eisbn"     ? "ISBN"     :
            name == "issn"      ? "ISSN"     :
            name == "doi"       ? "DOI"      :
            name == "customid"  ? "CustomID" :
            name[ 0 ].toUpper() + name.mid( 1 );
    
    MetaElement book_meta;

    if ( GetBasicMetaMap().contains( name ) || 
         name == "Author" ||
         name == "Title" 
       )
    {
        book_meta.name  = name;
        book_meta.value = meta.value;
    }

    return book_meta;
}


// Converts dc:creator and dc:contributor metadata to book internal metadata
Metadata::MetaElement Metadata::CreateContribMetadata( const Metadata::MetaElement &meta )
{
    QString role    = meta.attributes.value( "role", "aut" );

    // We convert the role into the new metadata name (e.g. aut -> Author)
    QString name    = GetFullRelatorNameHash()[ role ];

    // Some epub exporters set incorrect opf:role attributes
    // and we need to handle that. Otherwise, Sigil bugs out on export.
    // Since we can't tell what the role is, just guess author.
    if ( name.isEmpty() )

        name = GetFullRelatorNameHash()[ "aut" ];

    // If a "file-as" attribute is provided, we use that as the value
    QString file_as = meta.attributes.value( "file-as" );

    QString value   = meta.value.toString();

    if ( !file_as.isEmpty() )

        value = file_as;

    name = name[ 0 ].toUpper() + name.mid( 1 );

    MetaElement book_meta;
    book_meta.name  = name;
    book_meta.value = value;

    return book_meta;
}


// Converts dc:date metadata to book internal metadata
Metadata::MetaElement Metadata::DateMetadata( const Metadata::MetaElement &meta )
{
    QString name     = meta.name;
    QString dc_event = meta.attributes.value( "event" );

    // This is the default
    name = "Date of publication";  

    if ( EVENT_LIST.contains( dc_event ) )
    
        name = "Date of " + dc_event;

    // Dates are in YYYY[-MM[-DD]] format
    QStringList date_parts = meta.value.toString().split( "-", QString::SkipEmptyParts );

    if ( date_parts.count() < 1 )

        date_parts.append( QString::number( QDate::currentDate().year() ) );

    if ( date_parts.count() < 2 )

        date_parts.append( "01" );

    if ( date_parts.count() < 3 )

        date_parts.append( "01" );

    QVariant value = QDate( date_parts[ 0 ].toInt(), 
                            date_parts[ 1 ].toInt(), 
                            date_parts[ 2 ].toInt() );

    MetaElement book_meta;
    book_meta.name  = name;
    book_meta.value = value;

    return book_meta;
}


// Converts dc:identifier metadata to book internal metadata
Metadata::MetaElement Metadata::IdentifierMetadata( const Metadata::MetaElement &meta )
{
    QString scheme = meta.attributes.value( "scheme" );

    MetaElement book_meta;

    if ( SCHEME_LIST.contains( scheme, Qt::CaseInsensitive ) )
    {
        book_meta.name = scheme;
        book_meta.value = meta.value;
    }

    return book_meta;
}

