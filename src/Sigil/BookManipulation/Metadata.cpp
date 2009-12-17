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
#include "../BookManipulation/Metadata.h"

static const QString PATH_TO_LANGUAGES  = ":/data/languages.csv";
static const QString PATH_TO_BASICMETA  = ":/data/basicmeta.csv";
static const QString PATH_TO_RELATORS   = ":/data/relator.csv";

Metadata & Metadata::Instance()
{
    // We use a static local variable
    // to hold our singleton instance; using a pointer member
    // variable creates problems with object destruction;
    // Sigil is single-threaded so this is ok
    static Metadata meta;

    return meta;
}

const QMap< QString, QString > & Metadata::GetLanguageMap()
{
    return m_Languages;
}


const QMap< QString, Metadata::MetaInfo > & Metadata::GetRelatorMap()
{
    return m_Relators;
}


const QMap< QString, Metadata::MetaInfo > & Metadata::GetBasicMetaMap()
{
    return m_Basic;
}


const QHash< QString, QString > & Metadata::GetFullRelatorNameHash()
{
    return m_FullRelators;
}


const QHash< QString, QString > & Metadata::GetFullLanguageNameHash()
{
    return m_FullLanguages;
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

    QFile file( PATH_TO_LANGUAGES );

    // Check if we can open the file
    if ( !file.open( QFile::ReadOnly | QFile::Text ) )
    {
        QMessageBox::warning(	0,
            QObject::tr( "Sigil" ),
            QObject::tr( "Cannot read file %1:\n%2." )
            .arg( PATH_TO_LANGUAGES )
            .arg( file.errorString() ) 
            );

        return;
    }

    QTextStream in( &file );

    in.setCodec( "UTF-8" );

    // This will automatically switch reading from
    // UTF-8 to UTF-16 if a BOM is detected
    in.setAutoDetectUnicode( true );

    while ( in.atEnd() == false )
    {
        QString line = in.readLine();

        QStringList fields = line.split( "|" );

        m_Languages[ fields[ 0 ] ]      = fields[ 1 ];
        m_FullLanguages[ fields[ 1 ] ]  = fields[ 0 ];
    }	
}


// Loads the basic metadata and their descriptions from disk
void Metadata::LoadBasicMetadata()
{
    // If the basic metadata has already been loaded
    // by a previous Meta Editor, then don't load them again
    if ( !m_Basic.isEmpty() )

        return;

    QFile file( PATH_TO_BASICMETA );

    // Check if we can open the file
    if ( !file.open( QFile::ReadOnly | QFile::Text ) )
    {
        QMessageBox::warning(	0,
            QObject::tr( "Sigil" ),
            QObject::tr( "Cannot read file %1:\n%2." )
            .arg( PATH_TO_BASICMETA )
            .arg( file.errorString() ) 
            );

        return;
    }

    QTextStream in( &file );

    in.setCodec( "UTF-8" );

    // This will automatically switch reading from
    // UTF-8 to UTF-16 if a BOM is detected
    in.setAutoDetectUnicode( true );

    while ( in.atEnd() == false )
    {
        QString line = in.readLine();

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

    QFile file( PATH_TO_RELATORS );

    // Check if we can open the file
    if ( !file.open( QFile::ReadOnly | QFile::Text ) )
    {
        QMessageBox::warning(	0,
            QObject::tr( "Sigil" ),
            QObject::tr( "Cannot read file %1:\n%2." )
            .arg( PATH_TO_RELATORS )
            .arg( file.errorString() ) 
            );

        return;
    }

    QTextStream in( &file );

    in.setCodec( "UTF-8" );

    // This will automatically switch reading from
    // UTF-8 to UTF-16 if a BOM is detected
    in.setAutoDetectUnicode( true );

    while ( in.atEnd() == false )
    {
        QString line = in.readLine();

        QStringList fields = line.split( "|" );

        MetaInfo meta;

        meta.relator_code   = fields[ 1 ];
        meta.description    = fields[ 2 ];

        m_Relators[ fields[ 0 ] ]       = meta;
        m_FullRelators[ fields[ 1 ] ]   = fields[ 0 ];
    }
}


// recodes dublin core metadata from html to match metaelement from opf
MetaElement Metadata::RecodeHTMLDC( const MetaElement meta )
{

    MetaElement rmeta;
    QString name = meta.name.toLower();
    QString value = meta.value;

    // handle dublin core from html file with the original 15 element namespace
    // or using the expanded dcterms namespace
    // allow qualifiers as refinements
    QString refinement;

    if ( name.startsWith( "dc." ) ) name = name.mid(3);
    if ( name.startsWith( "dcterms." ) ) name = name.mid(8);

    // handle refinements as qualifiers
    if ( name.contains( "." ) )
    {
         QStringList fields = name.split( "." );
	 name = fields[0];
	 refinement = fields[1];
    }
    
    QString event;

    if ( ( name == "modifed" ) || ( ( name == "date" ) && ( refinement == "modified") ) )
    {
	name = "date";
	event = "modification";
    }
    else if ( ( name == "created" ) || ( ( name == "date" ) && ( refinement == "created") ) )
    {
	name = "date";
	event = "creation";
    }
    else if ( ( name == "issued" ) || ( ( name == "date" ) && ( refinement == "issued") ) )
    {
	name = "date";
	event = "publication";
    }

    QString role;
    if ( ( name == "creator" ) || ( name == "contributor" ) ) role = refinement;

    QString scheme = meta.attributes.value("scheme");
    if ( ( name == "identifier" ) && ( scheme.isEmpty() ) ) scheme = refinement;
    if ( scheme == "isbn" ) scheme = "ISBN";
    else if ( scheme == "issn" ) scheme = "ISSN";
    else if ( scheme == "doi" ) scheme = "DOI"; 
    else if ( scheme == "customid" ) scheme = "CustomID";

    rmeta.name = name;
    rmeta.value = value;
    if ( !scheme.isEmpty() ) rmeta.attributes["scheme"] = scheme;
    if ( !event.isEmpty() ) rmeta.attributes["event"] = event;
    if ( !role.isEmpty() ) rmeta.attributes["role"] = role;

    return rmeta;
}    





// maps meta info to internal book meta format
MetaElement Metadata::MaptoBookMeta( const MetaElement meta, const QString type )
{

    MetaElement bookMeta;

    QString name = meta.name.toLower();
    QString value = meta.value;

    if ( ( type == "HTML" ) &&  ( !name.startsWith( "dc." ) ) && ( !name.startsWith( "dcterms." ) ) )  
    {
        // non - dublin core meta info from html file, if this maps to 
        // one of the metadata basic fields pass it through
	// Author, Title, Publisher, Rights/CopyRight, EISBN/ISBN

        // remap commonly used meta values to match internal names
        if ( name == "copyright" ) name = "Rights";
        else if ( name == "eisbn" ) name = "ISBN";
        else if ( name == "issn" ) name = "ISSN";
        else if ( name == "doi" ) name = "DOI";
	else if ( name == "customid" ) name = "CustomID";
        name = name[ 0 ].toUpper() + name.mid(1);
	if ( m_Basic.contains( name ) || ( name == "Author" ) || ( name == "Title" ) )
	{
	    bookMeta.name = name;
	    bookMeta.value = value;
	}
        return bookMeta;
    }

    // Dublin Core
    // transform html based dublin core to opf style metaelement
    MetaElement wmeta = meta;
    if ( type == "HTML" ) wmeta = RecodeHTMLDC(meta);
    
    name = wmeta.name.toLower();
    value = wmeta.value;

    if ( ( name == "creator" ) || ( name == "contributor" ) )
    {
        QString role = wmeta.attributes.value("role","aut");

        // We convert the role into the new metadata name (e.g. aut -> Author)
        name = GetFullRelatorNameHash()[ role ];

        // If a "file-as" attribute is provided, we use that as the value
        QString file_as = wmeta.attributes.value("file-as");
        if ( !file_as.isEmpty() )  value = file_as;
    }

    else if ( name == "date" )
    {

	QString event = wmeta.attributes.value("event");
	QStringList eventList;
	eventList << "creation" << "publication" << "modification";
        name = "Date of publication";  // default
        if ( eventList.contains( event ) ) name = "Date of " + event;

        // Dates are in YYYY[-MM[-DD]] format
        QStringList date_parts = value.split( "-", QString::SkipEmptyParts );
        if ( date_parts.count() < 1 ) date_parts.append( QString::number( QDate::currentDate().year() ) );
        if ( date_parts.count() < 2 ) date_parts.append("01");
        if ( date_parts.count() < 3 ) date_parts.append("01");

        value = QDate( date_parts[ 0 ].toInt(), date_parts[ 1 ].toInt(), date_parts[ 2 ].toInt() ).toString(Qt::ISODate);
    }

    else if ( name == "identifier" )
    {
        QString scheme = wmeta.attributes.value("scheme");
        QStringList schemeList;
	schemeList << "ISBN" << "ISSN" << "DOI" << "CustomID"; 
        if (schemeList.contains(scheme)) name = scheme;
	else
	{
	  // set name and value to null to ignore identifiers not supported by internal book metadata
	  name = "";
	  value = "";
	}
    }

    else if ( name == "language" )
    {
        // We convert the ISO 639-1 language code into the full language name
        // (e.g. en -> English)
        value = GetFullLanguageNameHash()[ value ];
    }

    if ( ( !name.isEmpty() )  && ( !value.isEmpty() ) )
    {
        bookMeta.name = name[ 0 ].toUpper() + name.mid(1);
        bookMeta.value = value;
    }

    return bookMeta;
}

