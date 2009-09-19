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




